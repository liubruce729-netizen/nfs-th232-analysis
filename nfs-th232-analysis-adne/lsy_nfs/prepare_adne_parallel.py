#!/usr/bin/env python3
"""Prepare independent ADNE working directories for parallel processing.

CN: 这个脚本把一个已经编译好的 ADNE 目录复制成多个独立 job 目录，
    每个 job 使用自己的 run list 和 out 软链接，data 软链接保持一致。
EN: This script clones a compiled ADNE directory into independent job
    directories. Each job gets its own run list and output symlink, while
    sharing the same read-only data symlink target.

Examples / 运行例子:
  # CN: 在已经编译好的 ADNE 目录中运行；把 data/run_100_r1 到 data/run_100_r180
  #     平均分成 18 份，输出到 /data/out_parallel/par_out001...
  # EN: Run inside a compiled ADNE directory; split data/run_100_r1 to
  #     data/run_100_r180 into 18 jobs, writing to par_out001...
  ./lsy_nfs/prepare_adne_parallel.py \
    --jobs 18 \
    --out-base /data/out_parallel \
    --file-template 'data/run_100_r{n}' \
    --first 1 \
    --last 180 \
    --grusys /home/e877_ana/Analysis/pkg_install/install/GRU \
    --mfmsys /home/e877_ana/Analysis/pkg_install/install/MFMlib

  # CN: 如果已有完整输入列表，也可以直接按列表平均分。
  # EN: An existing full run list can also be split directly.
  ./lsy_nfs/prepare_adne_parallel.py \
    --jobs 18 \
    --out-base /data/out_parallel \
    --input-list ListRun_all.txt

  # CN: 准备完成后启动并行；MAX_PARALLEL 可小于 job 数以控制机器负载。
  # EN: Start the generated jobs; MAX_PARALLEL can be smaller than the job count.
  cd par_run
  MAX_PARALLEL=18 ./run_all_parallel.sh
"""

from __future__ import annotations

import argparse
import glob
import os
import re
import shlex
import shutil
import stat
import sys
from pathlib import Path
from typing import Iterable, List


DEFAULT_GRUSYS = "/home/e877_ana/Analysis/pkg_install/install/GRU"
DEFAULT_MFMSYS = "/home/e877_ana/Analysis/pkg_install/install/MFMlib"


def natural_key(text: str) -> list[object]:
    """CN/EN: Natural sort key, so file.10 is after file.9."""
    return [int(part) if part.isdigit() else part for part in re.split(r"(\d+)", text)]


def read_input_list(path: Path) -> list[str]:
    """CN: 读取用户提供的文件列表；EN: Read an existing user file list."""
    items: list[str] = []
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#") or line.startswith("//"):
            continue
        items.append(line)
    return items


def build_template_list(template: str, first: int, last: int) -> list[str]:
    """Build an integer filename sequence from a template.

    CN: 如果模板含有 {n} 或 {n:04d}，使用 Python format；
        如果没有花括号，就直接把整数接在模板后面。
    EN: If the template contains {n}/{n:04d}, use Python format;
        otherwise append the integer to the template.
    """
    step = 1 if last >= first else -1
    values: list[str] = []
    for n in range(first, last + step, step):
        if "{" in template and "}" in template:
            values.append(template.format(n=n))
        else:
            values.append(f"{template}{n}")
    return values


def split_evenly(items: list[str], jobs: int) -> list[list[str]]:
    """CN: 直接按整数平均切分；EN: Split items evenly by integer counts."""
    chunks: list[list[str]] = []
    total = len(items)
    base = total // jobs
    extra = total % jobs
    pos = 0
    for idx in range(jobs):
        count = base + (1 if idx < extra else 0)
        chunks.append(items[pos : pos + count])
        pos += count
    return chunks


def resolve_symlink_target(source_dir: Path, link_name: str, explicit: str | None) -> Path:
    """Resolve data/out target from an explicit path or an existing symlink."""
    if explicit:
        return Path(explicit).expanduser().resolve()
    link = source_dir / link_name
    if link.is_symlink():
        target = os.readlink(link)
        target_path = Path(target)
        if not target_path.is_absolute():
            target_path = (link.parent / target_path).resolve()
        return target_path
    if link.exists():
        return link.resolve()
    raise SystemExit(f"Cannot resolve {link_name}: pass --{link_name}-target explicitly.")


def ensure_empty_dir(path: Path, allow_non_empty: bool) -> None:
    """CN: 输出目录默认必须为空；EN: Output directory must be empty by default."""
    path.mkdir(parents=True, exist_ok=True)
    entries = list(path.iterdir())
    if entries and not allow_non_empty:
        raise SystemExit(
            f"Output directory is not empty: {path}\n"
            "Use --allow-non-empty-out only if you are sure it is safe."
        )


def replace_symlink(path: Path, target: Path) -> None:
    """Replace path with a symlink, refusing to remove a real directory."""
    if path.is_symlink() or path.is_file():
        path.unlink()
    elif path.exists():
        raise SystemExit(f"Refusing to replace non-symlink directory: {path}")
    path.symlink_to(target, target_is_directory=True)


def copy_job_if_needed(source_dir: Path, job_dir: Path) -> None:
    """CN: 已存在则不重新复制；EN: Do not recopy an existing job directory."""
    if job_dir.exists():
        return

    def ignore(_dir: str, names: Iterable[str]) -> set[str]:
        skipped = {
            "par_run",
            ".git",
            "out",
            "data",
        }
        return set(names).intersection(skipped)

    shutil.copytree(source_dir, job_dir, symlinks=True, ignore=ignore)


def set_analysis_filename(config_path: Path, runlist_path: Path) -> None:
    """Update only analysis.filename in config.yaml, preserving other content."""
    lines = config_path.read_text(encoding="utf-8").splitlines(keepends=True)
    out: list[str] = []
    in_analysis = False
    replaced = False

    for line in lines:
        top_key = re.match(r"^([A-Za-z0-9_]+):\s*(?:#.*)?$", line)
        if top_key:
            if in_analysis and not replaced:
                out.append(f'  filename: "{runlist_path}"\n')
                replaced = True
            in_analysis = top_key.group(1) == "analysis"

        if in_analysis and re.match(r"^\s*filename\s*:", line):
            indent = re.match(r"^(\s*)", line).group(1)
            out.append(f'{indent}filename: "{runlist_path}"\n')
            replaced = True
            continue

        out.append(line)

    if in_analysis and not replaced:
        out.append(f'  filename: "{runlist_path}"\n')
        replaced = True

    if not replaced:
        out.append("\nanalysis:\n")
        out.append(f'  filename: "{runlist_path}"\n')

    config_path.write_text("".join(out), encoding="utf-8")


def write_job_runner(job_dir: Path, grusys: str, mfmsys: str) -> None:
    """Write a job-local wrapper that fixes GRU/MFM runtime libraries."""
    script = job_dir / "run_adne_env.sh"
    text = f"""#!/usr/bin/env bash
set -euo pipefail

# CN: 每个并行 job 都通过这个 wrapper 运行 ADNE，避免系统默认 GRU/MFM 覆盖用户版本。
# EN: Each parallel job runs ADNE through this wrapper to force the requested GRU/MFM.
export GRUSYS={shlex.quote(grusys)}
export MFMSYS={shlex.quote(mfmsys)}
export LD_LIBRARY_PATH=\"$GRUSYS/lib:$MFMSYS/lib:${{LD_LIBRARY_PATH:-}}\"

ldd ./AnalysisADNE | grep -E 'GRU|MFM|not found'
exec ./AnalysisADNE \"$@\"
"""
    script.write_text(text, encoding="utf-8")
    mode = script.stat().st_mode
    script.chmod(mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)


def write_run_all_script(par_dir: Path, jobs: int) -> None:
    """Generate a small launcher with bounded parallelism."""
    script = par_dir / "run_all_parallel.sh"
    text = f"""#!/usr/bin/env bash
set -euo pipefail

# CN: 默认同时跑 {jobs} 个任务；可用 MAX_PARALLEL=4 ./run_all_parallel.sh 限制并发数。
# EN: Runs {jobs} jobs by default; set MAX_PARALLEL=4 to limit concurrency.
SCRIPT_DIR=\"$(cd \"$(dirname \"${{BASH_SOURCE[0]}}\")\" && pwd)\"
MANIFEST=\"$SCRIPT_DIR/jobs.tsv\"
MAX_PARALLEL=\"${{MAX_PARALLEL:-{jobs}}}\"

if [[ ! -f \"$MANIFEST\" ]]; then
  echo \"Missing manifest: $MANIFEST\" >&2
  exit 1
fi

running=0
status=0
tail -n +2 \"$MANIFEST\" | while IFS=$'\\t' read -r job_id job_dir runlist out_dir nfiles; do
  if [[ \"$nfiles\" == \"0\" ]]; then
    echo \"Skip job $job_id: empty run list\"
    continue
  fi

  mkdir -p \"$out_dir\"
  log=\"$out_dir/AnalysisADNE.log\"
  echo \"Start job $job_id: $job_dir  log=$log\"
  (
    cd \"$job_dir\"
    ./run_adne_env.sh > \"$log\" 2>&1
  ) &

  running=$((running + 1))
  if (( running >= MAX_PARALLEL )); then
    if ! wait -n; then
      status=1
    fi
    running=$((running - 1))
  fi
done

while (( running > 0 )); do
  if ! wait -n; then
    status=1
  fi
  running=$((running - 1))
done

exit \"$status\"
"""
    script.write_text(text, encoding="utf-8")
    mode = script.stat().st_mode
    script.chmod(mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Prepare parallel ADNE job directories / 准备 ADNE 并行任务目录",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=r"""Examples / 运行例子:
  ./lsy_nfs/prepare_adne_parallel.py \
    --jobs 18 \
    --out-base /data/out_parallel \
    --file-template 'data/run_100_r{n}' \
    --first 1 \
    --last 180 \
    --grusys /home/e877_ana/Analysis/pkg_install/install/GRU \
    --mfmsys /home/e877_ana/Analysis/pkg_install/install/MFMlib

  ./lsy_nfs/prepare_adne_parallel.py \
    --jobs 18 \
    --out-base /data/out_parallel \
    --input-list ListRun_all.txt

  cd par_run
  MAX_PARALLEL=18 ./run_all_parallel.sh
""",
    )
    parser.add_argument(
        "--source",
        default=".",
        help="compiled ADNE source/template directory, default: current directory",
    )
    parser.add_argument("--jobs", type=int, default=18, help="number of parallel jobs")
    parser.add_argument(
        "--out-base",
        required=True,
        help="base output directory; par_out001... will be created inside it",
    )
    parser.add_argument(
        "--data-target",
        default=None,
        help="target directory for each job's data symlink; default: source/data target",
    )
    parser.add_argument(
        "--grusys",
        default=DEFAULT_GRUSYS,
        help=f"GRUSYS path written into each job wrapper, default: {DEFAULT_GRUSYS}",
    )
    parser.add_argument(
        "--mfmsys",
        default=DEFAULT_MFMSYS,
        help=f"MFMSYS path written into each job wrapper, default: {DEFAULT_MFMSYS}",
    )
    parser.add_argument(
        "--allow-non-empty-out",
        action="store_true",
        help="allow existing non-empty par_out directories",
    )

    input_group = parser.add_mutually_exclusive_group(required=True)
    input_group.add_argument("--input-list", help="existing file containing all input files")
    input_group.add_argument("--glob", help="glob pattern for input files, quoted")
    input_group.add_argument(
        "--file-template",
        help="integer sequence template, e.g. 'data/run_100_r{n}.dat' or 'data/file.'",
    )
    parser.add_argument("--first", type=int, help="first integer for --file-template")
    parser.add_argument("--last", type=int, help="last integer for --file-template")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.jobs <= 0:
        raise SystemExit("--jobs must be positive")

    source_dir = Path(args.source).expanduser().resolve()
    if not (source_dir / "AnalysisADNE").exists():
        raise SystemExit(f"AnalysisADNE not found in source directory: {source_dir}")

    if args.input_list:
        files = read_input_list(Path(args.input_list).expanduser())
    elif args.glob:
        files = sorted(glob.glob(args.glob), key=natural_key)
    else:
        if args.first is None or args.last is None:
            raise SystemExit("--file-template requires --first and --last")
        files = build_template_list(args.file_template, args.first, args.last)

    if not files:
        raise SystemExit("No input files were selected.")

    data_target = resolve_symlink_target(source_dir, "data", args.data_target)
    out_base = Path(args.out_base).expanduser().resolve()
    par_dir = source_dir / "par_run"
    par_dir.mkdir(parents=True, exist_ok=True)
    out_base.mkdir(parents=True, exist_ok=True)

    chunks = split_evenly(files, args.jobs)
    manifest_path = par_dir / "jobs.tsv"

    manifest_lines = ["job_id\tjob_dir\trunlist\tout_dir\tnfiles\n"]
    for index, chunk in enumerate(chunks, start=1):
        job_name = f"job_{index:03d}"
        runlist = par_dir / f"ListRun_par_{index:03d}.txt"
        out_dir = out_base / f"par_out{index:03d}"
        job_dir = par_dir / job_name

        runlist.write_text("".join(f"{item}\n" for item in chunk), encoding="utf-8")
        ensure_empty_dir(out_dir, args.allow_non_empty_out)

        copy_job_if_needed(source_dir, job_dir)
        replace_symlink(job_dir / "data", data_target)
        replace_symlink(job_dir / "out", out_dir)
        set_analysis_filename(job_dir / "Yaml_config_files" / "config.yaml", runlist)
        write_job_runner(job_dir, args.grusys, args.mfmsys)

        manifest_lines.append(
            f"{index}\t{job_dir}\t{runlist}\t{out_dir}\t{len(chunk)}\n"
        )

    manifest_path.write_text("".join(manifest_lines), encoding="utf-8")
    write_run_all_script(par_dir, args.jobs)

    empty_jobs = sum(1 for chunk in chunks if not chunk)
    print(f"Source ADNE directory: {source_dir}")
    print(f"Parallel directory:    {par_dir}")
    print(f"Data target:           {data_target}")
    print(f"Output base:           {out_base}")
    print(f"GRUSYS:                {args.grusys}")
    print(f"MFMSYS:                {args.mfmsys}")
    print(f"Input files:           {len(files)}")
    print(f"Jobs:                  {args.jobs}")
    if empty_jobs:
        print(f"Empty jobs:            {empty_jobs} (run_all_parallel.sh will skip them)")
    print(f"Manifest:              {manifest_path}")
    print(f"Launcher:              {par_dir / 'run_all_parallel.sh'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
