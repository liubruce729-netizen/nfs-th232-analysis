#!/usr/bin/env python3
"""Two-stage launcher for calibrated NFS triple-gamma TH3D analysis.

第一阶段并行读取、快速多重度筛选、逐 crystal 刻度并写紧凑 Tree；
第二阶段串行逐张填充 TH3D，任意时刻内存中只有一张 cube。

Example / 示例:
  ./lsy_nfs/run_triple_gamma_ana_calibrated_parallel.py \
    --input /home/e877_ana/analysed_data/July9/run40 \
    --calibration-summary calibratejuly0910/calibration_summary.tsv \
    --output out/triple_gamma_ana_calibrated.root \
    --jobs 8 \
    --distance 24 \
    --neutron-bins 10,15,20,30,40 \
    --gamma-bins 512 \
    --gamma-min 0 \
    --gamma-max 2048
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


def natural_key(path: Path) -> list[object]:
    import re

    parts = re.split(r"(\d+)", str(path))
    return [int(part) if part.isdigit() else part for part in parts]


def read_input_list(path: Path) -> list[str]:
    items: list[str] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        item = line.strip()
        if not item or item.startswith("#") or item.startswith("//"):
            continue
        items.append(item)
    return items


def discover_mult3_files(sources: list[str]) -> list[Path]:
    files: list[Path] = []
    for source in sources:
        path = Path(source).expanduser()
        if path.is_dir():
            files.extend(path.rglob("mult3_nfs_run_*.root"))
        elif path.is_file():
            files.append(path)
        else:
            print(f"Warning: input source does not exist: {source}", file=sys.stderr)
    unique = {str(path.resolve()): path.resolve() for path in files}
    return sorted(unique.values(), key=natural_key)


def split_evenly(items: list[Path], jobs: int) -> list[list[Path]]:
    jobs = max(1, min(jobs, len(items)))
    chunks: list[list[Path]] = []
    base = len(items) // jobs
    extra = len(items) % jobs
    start = 0
    for index in range(jobs):
        size = base + (1 if index < extra else 0)
        chunks.append(items[start : start + size])
        start += size
    return chunks


def root_escape(text: str) -> str:
    return text.replace("\\", "\\\\").replace('"', '\\"')


def build_prepare_call(
    macro: Path,
    filelist: Path,
    calibration_summary: Path,
    output: Path,
    use_veto: bool,
    max_entries: int,
    min_mult: int,
    gamma_min: float,
    gamma_max: float,
    distance: float,
    time_mode: str,
    time_branch: str,
) -> str:
    return (
        f'{root_escape(str(macro))}("@{root_escape(str(filelist))}",'
        f'"{root_escape(str(calibration_summary))}",'
        f'"{root_escape(str(output))}",'
        f'{"true" if use_veto else "false"},'
        f'{max_entries},{min_mult},'
        f'{gamma_min:.12g},{gamma_max:.12g},{distance:.12g},'
        f'"{root_escape(time_mode)}",'
        f'"{root_escape(time_branch)}")'
    )


def build_fill_call(
    macro: Path,
    compact_filelist: Path,
    output: Path,
    min_mult: int,
    gamma_bins: int,
    gamma_min: float,
    gamma_max: float,
    neutron_bins: str,
    memory_guard_gib: float,
) -> str:
    return (
        f'{root_escape(str(macro))}("@{root_escape(str(compact_filelist))}",'
        f'"{root_escape(str(output))}",'
        f'{min_mult},{gamma_bins},'
        f'{gamma_min:.12g},{gamma_max:.12g},'
        f'"{root_escape(neutron_bins)}",'
        f'{memory_guard_gib:.12g})'
    )


def main() -> int:
    here = Path(__file__).resolve().parent
    parser = argparse.ArgumentParser(
        description=(
            "Parallel calibration/tree preparation followed by serial triple-gamma TH3D filling / "
            "并行刻度写 Tree，串行逐张填 TH3D"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument("--input", action="append", default=[], help="input file or directory; can be repeated")
    parser.add_argument("--input-list", help="text file containing input files/directories")
    parser.add_argument("--calibration-summary", required=True, help="calibration_summary.tsv from calibrate_nfs_crystals.sh")
    parser.add_argument("--output", required=True, help="final serially written ROOT output")
    parser.add_argument("--jobs", type=int, default=1, help="number of parallel preparation ROOT jobs")
    parser.add_argument("--distance", type=float, default=24.0, help="NFS distance in meters")
    parser.add_argument("--neutron-bins", default="10,15,20,30,40", help="neutron energy bin edges in MeV")
    parser.add_argument("--gamma-bins", type=int, default=256, help="TH3D bins per gamma axis")
    parser.add_argument("--gamma-min", type=float, default=0.0, help="gamma energy lower edge in keV")
    parser.add_argument("--gamma-max", type=float, default=4096.0, help="gamma energy upper edge in keV")
    parser.add_argument("--min-mult", type=int, default=3, help="minimum clover multiplicity")
    parser.add_argument("--no-veto", action="store_true", help="disable BGO/CSI veto")
    parser.add_argument("--max-entries", type=int, default=-1, help="max total entries per preparation job; default all")
    parser.add_argument(
        "--time-mode",
        default="offset",
        choices=["offset", "gain", "offset_gain", "gain_offset"],
        help="time calibration mode",
    )
    parser.add_argument("--time-branch", default="fTime", help="crystal time branch leaf name")
    parser.add_argument("--memory-guard-gib", type=float, default=12.0, help="maximum estimated memory for one serial TH3D")
    parser.add_argument("--macro", default=str(here / "triple_gamma_prepare_calibrated.C"), help="parallel compact-tree preparation macro")
    parser.add_argument("--fill-macro", default=str(here / "triple_gamma_fill_serial.C"), help="serial TH3D filling macro")
    parser.add_argument("--root", default="root", help="ROOT executable")
    parser.add_argument("--hadd", default="hadd", help="deprecated compatibility option; no TH3D hadd is performed")
    parser.add_argument("--work-dir", help="intermediate directory; default: <output>.parts")
    args = parser.parse_args()

    sources = list(args.input)
    if args.input_list:
        sources.extend(read_input_list(Path(args.input_list).expanduser()))
    if not sources:
        parser.error("provide --input or --input-list")
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    if args.min_mult < 3:
        parser.error("--min-mult must be >= 3 for triple-gamma")
    if args.gamma_bins < 1 or args.gamma_max <= args.gamma_min:
        parser.error("invalid gamma histogram range or bin count")

    prepare_macro = Path(args.macro).expanduser().resolve()
    fill_macro = Path(args.fill_macro).expanduser().resolve()
    calibration_summary = Path(args.calibration_summary).expanduser().resolve()
    output = Path(args.output).expanduser().resolve()
    if not prepare_macro.is_file():
        raise SystemExit(f"Preparation macro not found: {prepare_macro}")
    if not fill_macro.is_file():
        raise SystemExit(f"Serial fill macro not found: {fill_macro}")
    if not calibration_summary.is_file():
        raise SystemExit(f"Calibration summary not found: {calibration_summary}")

    files = discover_mult3_files(sources)
    if not files:
        raise SystemExit("No mult3_nfs_run_*.root files found.")

    work_dir = Path(args.work_dir).expanduser().resolve() if args.work_dir else output.with_suffix(output.suffix + ".parts")
    work_dir.mkdir(parents=True, exist_ok=True)
    output.parent.mkdir(parents=True, exist_ok=True)

    expanded_list = work_dir / "mult3_inputs.txt"
    expanded_list.write_text("".join(f"{path}\n" for path in files), encoding="utf-8")

    chunks = split_evenly(files, args.jobs)
    compact_outputs: list[Path] = []
    processes: list[tuple[int, subprocess.Popen[bytes], Path, Path]] = []
    for index, chunk in enumerate(chunks, start=1):
        if not chunk:
            continue
        filelist = work_dir / f"job_{index:03d}.txt"
        compact_output = work_dir / f"job_{index:03d}_calibrated.root"
        log = work_dir / f"job_{index:03d}_prepare.log"
        filelist.write_text("".join(f"{path}\n" for path in chunk), encoding="utf-8")

        prepare_call = build_prepare_call(
            prepare_macro,
            filelist,
            calibration_summary,
            compact_output,
            not args.no_veto,
            args.max_entries,
            args.min_mult,
            args.gamma_min,
            args.gamma_max,
            args.distance,
            args.time_mode,
            args.time_branch,
        )
        command = [args.root, "-l", "-b", "-q", prepare_call]
        print(f"Start preparation job {index}: {len(chunk)} files -> {compact_output}")
        log_handle = log.open("w", encoding="utf-8")
        log_handle.write("Command:\n  " + " ".join(command) + "\n\n")
        log_handle.flush()
        process = subprocess.Popen(command, stdout=log_handle, stderr=subprocess.STDOUT)
        log_handle.close()
        processes.append((index, process, compact_output, log))
        compact_outputs.append(compact_output)

    failed = 0
    for index, process, compact_output, log in processes:
        return_code = process.wait()
        if return_code != 0 or not compact_output.exists() or compact_output.stat().st_size == 0:
            failed += 1
            print(f"Preparation job {index} failed rc={return_code}; log={log}", file=sys.stderr)
        else:
            print(f"Preparation job {index} done: {compact_output}")
    if failed:
        raise SystemExit(f"{failed} preparation job(s) failed; skip serial cube filling.")

    compact_list = work_dir / "compact_inputs.txt"
    compact_list.write_text("".join(f"{path}\n" for path in compact_outputs), encoding="utf-8")
    fill_log = work_dir / "serial_fill.log"
    fill_call = build_fill_call(
        fill_macro,
        compact_list,
        output,
        args.min_mult,
        args.gamma_bins,
        args.gamma_min,
        args.gamma_max,
        args.neutron_bins,
        args.memory_guard_gib,
    )
    fill_command = [args.root, "-l", "-b", "-q", fill_call]
    print(f"Start serial cube filling: {len(compact_outputs)} compact files -> {output}")
    if output.exists():
        output.unlink()
    with fill_log.open("w", encoding="utf-8") as log_handle:
        log_handle.write("Command:\n  " + " ".join(fill_command) + "\n\n")
        log_handle.flush()
        fill_result = subprocess.run(fill_command, stdout=log_handle, stderr=subprocess.STDOUT)
    if fill_result.returncode != 0 or not output.exists() or output.stat().st_size == 0:
        raise SystemExit(f"Serial cube filling failed rc={fill_result.returncode}; log={fill_log}")

    manifest = work_dir / "parallel_manifest.tsv"
    with manifest.open("w", encoding="utf-8") as handle:
        handle.write("job\tcompact_tree\tprepare_log\tnfiles\n")
        for index, (_job, _process, compact_output, log) in enumerate(processes, start=1):
            file_count = len(chunks[index - 1]) if index - 1 < len(chunks) else 0
            handle.write(f"{index:03d}\t{compact_output}\t{log}\t{file_count}\n")

    print(f"Expanded input list: {expanded_list}")
    print(f"Compact input list: {compact_list}")
    print(f"Manifest: {manifest}")
    print(f"Serial fill log: {fill_log}")
    print(f"Output: {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
