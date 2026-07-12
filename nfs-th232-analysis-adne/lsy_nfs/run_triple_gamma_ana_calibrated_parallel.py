#!/usr/bin/env python3
"""Parallel launcher for calibrated NFS triple-gamma TH3D analysis.

并行执行逐 run、逐 crystal 刻度后的 NFS triple-gamma TH3D 分析。

Example / 示例:
  ./lsy_nfs/run_triple_gamma_ana_calibrated_parallel.py \
    --input /home/e877_ana/analysed_data/July9/run40 \
    --calibration-summary calibratejuly0910/calibration_summary.tsv \
    --output out/triple_gamma_ana_calibrated.root \
    --jobs 8 \
    --distance 24 \
    --neutron-bins 10,15,20,30,40 \
    --gamma-bins 256
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
from pathlib import Path


def natural_key(path: Path) -> list[object]:
    import re

    parts = re.split(r"(\d+)", str(path))
    return [int(p) if p.isdigit() else p for p in parts]


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
        p = Path(source).expanduser()
        if p.is_dir():
            files.extend(p.rglob("mult3_nfs_run_*.root"))
        elif p.is_file():
            files.append(p)
        else:
            print(f"Warning: input source does not exist: {source}", file=sys.stderr)
    unique = {str(f.resolve()): f.resolve() for f in files}
    return sorted(unique.values(), key=natural_key)


def split_evenly(items: list[Path], jobs: int) -> list[list[Path]]:
    jobs = max(1, min(jobs, len(items)))
    chunks: list[list[Path]] = []
    base = len(items) // jobs
    extra = len(items) % jobs
    start = 0
    for idx in range(jobs):
        size = base + (1 if idx < extra else 0)
        chunks.append(items[start : start + size])
        start += size
    return chunks


def root_escape(text: str) -> str:
    return text.replace("\\", "\\\\").replace('"', '\\"')


def build_root_call(
    macro: Path,
    filelist: Path,
    calibration_summary: Path,
    output: Path,
    use_veto: bool,
    max_entries: int,
    min_mult: int,
    gamma_bins: int,
    gamma_min: float,
    gamma_max: float,
    distance: float,
    neutron_bins: str,
    time_mode: str,
    time_branch: str,
    memory_guard_gib: float,
) -> str:
    return (
        f'{root_escape(str(macro))}("@{root_escape(str(filelist))}",'
        f'"{root_escape(str(calibration_summary))}",'
        f'"{root_escape(str(output))}",'
        f'{"true" if use_veto else "false"},'
        f'{max_entries},{min_mult},{gamma_bins},'
        f'{gamma_min:.12g},{gamma_max:.12g},{distance:.12g},'
        f'"{root_escape(neutron_bins)}",'
        f'"{root_escape(time_mode)}",'
        f'"{root_escape(time_branch)}",'
        f'{memory_guard_gib:.12g})'
    )


def main() -> int:
    here = Path(__file__).resolve().parent
    parser = argparse.ArgumentParser(
        description="Run calibrated triple_gamma_ana on mult3 ROOT files in parallel / 并行运行 calibrated triple-gamma 分析",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument("--input", action="append", default=[], help="input file or directory; can be repeated")
    parser.add_argument("--input-list", help="text file containing input files/directories")
    parser.add_argument("--calibration-summary", required=True, help="calibration_summary.tsv from calibrate_nfs_crystals.sh")
    parser.add_argument("--output", required=True, help="final merged ROOT output")
    parser.add_argument("--jobs", type=int, default=1, help="number of parallel ROOT jobs")
    parser.add_argument("--distance", type=float, default=24.0, help="NFS distance in meters")
    parser.add_argument("--neutron-bins", default="10,15,20,30,40", help="neutron energy bin edges in MeV")
    parser.add_argument("--gamma-bins", type=int, default=256, help="TH3D bins per gamma axis")
    parser.add_argument("--gamma-min", type=float, default=0.0, help="gamma energy lower edge in keV")
    parser.add_argument("--gamma-max", type=float, default=4096.0, help="gamma energy upper edge in keV")
    parser.add_argument("--min-mult", type=int, default=3, help="minimum calibrated clover multiplicity")
    parser.add_argument("--no-veto", action="store_true", help="disable BGO/CSI veto")
    parser.add_argument("--max-entries", type=int, default=-1, help="max total entries per ROOT job; default all")
    parser.add_argument("--time-mode", default="offset", choices=["offset", "gain", "offset_gain", "gain_offset"], help="time calibration mode")
    parser.add_argument("--time-branch", default="fTime", help="crystal time branch leaf name")
    parser.add_argument("--memory-guard-gib", type=float, default=12.0, help="skip ROOT job if estimated TH3D storage exceeds this GiB")
    parser.add_argument("--macro", default=str(here / "triple_gamma_ana_calibrated.C"), help="ROOT macro path")
    parser.add_argument("--root", default="root", help="ROOT executable")
    parser.add_argument("--hadd", default="hadd", help="hadd executable")
    parser.add_argument("--work-dir", help="partial output directory; default: <output>.parts")
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

    macro = Path(args.macro).expanduser().resolve()
    calibration_summary = Path(args.calibration_summary).expanduser().resolve()
    output = Path(args.output).expanduser().resolve()
    if not macro.is_file():
        raise SystemExit(f"Macro not found: {macro}")
    if not calibration_summary.is_file():
        raise SystemExit(f"Calibration summary not found: {calibration_summary}")

    files = discover_mult3_files(sources)
    if not files:
        raise SystemExit("No mult3_nfs_run_*.root files found.")

    work_dir = Path(args.work_dir).expanduser().resolve() if args.work_dir else output.with_suffix(output.suffix + ".parts")
    work_dir.mkdir(parents=True, exist_ok=True)
    output.parent.mkdir(parents=True, exist_ok=True)

    expanded_list = work_dir / "mult3_inputs.txt"
    expanded_list.write_text("".join(f"{f}\n" for f in files), encoding="utf-8")

    chunks = split_evenly(files, args.jobs)
    partials: list[Path] = []
    procs: list[tuple[int, subprocess.Popen[bytes], Path, Path]] = []
    for idx, chunk in enumerate(chunks, start=1):
        if not chunk:
            continue
        filelist = work_dir / f"job_{idx:03d}.txt"
        partial = work_dir / f"job_{idx:03d}.root"
        log = work_dir / f"job_{idx:03d}.log"
        filelist.write_text("".join(f"{f}\n" for f in chunk), encoding="utf-8")
        root_call = build_root_call(
            macro,
            filelist,
            calibration_summary,
            partial,
            not args.no_veto,
            args.max_entries,
            args.min_mult,
            args.gamma_bins,
            args.gamma_min,
            args.gamma_max,
            args.distance,
            args.neutron_bins,
            args.time_mode,
            args.time_branch,
            args.memory_guard_gib,
        )
        cmd = [args.root, "-l", "-b", "-q", root_call]
        print(f"Start job {idx}: {len(chunk)} files -> {partial}")
        log_handle = log.open("w", encoding="utf-8")
        log_handle.write("Command:\n  " + " ".join(cmd) + "\n\n")
        log_handle.flush()
        proc = subprocess.Popen(cmd, stdout=log_handle, stderr=subprocess.STDOUT)
        log_handle.close()
        procs.append((idx, proc, partial, log))
        partials.append(partial)

    failed = 0
    for idx, proc, partial, log in procs:
        rc = proc.wait()
        if rc != 0 or not partial.exists() or partial.stat().st_size == 0:
            failed += 1
            print(f"Job {idx} failed rc={rc}; log={log}", file=sys.stderr)
        else:
            print(f"Job {idx} done: {partial}")
    if failed:
        raise SystemExit(f"{failed} job(s) failed; skip merge.")

    if len(partials) == 1:
        os.replace(partials[0], output)
    else:
        cmd = [args.hadd, "-f", str(output)] + [str(p) for p in partials]
        print("Merge:", " ".join(cmd))
        subprocess.run(cmd, check=True)

    manifest = work_dir / "parallel_manifest.tsv"
    with manifest.open("w", encoding="utf-8") as f:
        f.write("job\tpartial\tlog\tnfiles\n")
        for idx, (_job, _proc, partial, log) in enumerate(procs, start=1):
            nfiles = len(chunks[idx - 1]) if idx - 1 < len(chunks) else 0
            f.write(f"{idx:03d}\t{partial}\t{log}\t{nfiles}\n")

    print(f"Expanded input list: {expanded_list}")
    print(f"Manifest: {manifest}")
    print(f"Output: {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
