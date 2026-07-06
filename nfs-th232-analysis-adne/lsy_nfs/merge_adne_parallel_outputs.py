#!/usr/bin/env python3
"""Merge ROOT outputs produced by prepare_adne_parallel.py jobs.

CN: 合并 par_outXXX 里的两个 tree ROOT 文件组和一个 NFS histo 文件组。
EN: Merge two tree ROOT groups and one NFS histogram ROOT group from par_outXXX.

Examples / 运行例子:
  # CN: 合并 prepare_adne_parallel.py 生成的 par_out001...par_out018。
  # EN: Merge par_out001...par_out018 produced by prepare_adne_parallel.py.
  ./lsy_nfs/merge_adne_parallel_outputs.py \
    --out-base /data/out_parallel \
    --force

  # CN: 自定义合并后的文件名。
  # EN: Use custom merged output names.
  ./lsy_nfs/merge_adne_parallel_outputs.py \
    --out-base /data/out_parallel \
    --nfs-tree-name nfs_tree_all.root \
    --mult3-tree-name mult3_nfs_tree_all.root \
    --histo-name nfs_histoExogam2_all.root \
    --force
"""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from pathlib import Path


def collect_files(out_base: Path, pattern: str) -> list[Path]:
    """CN/EN: Collect files from par_outXXX directories only."""
    return sorted(out_base.glob(f"par_out*/{pattern}"))


def run_hadd(output: Path, inputs: list[Path], force: bool) -> None:
    """Run ROOT hadd to merge trees or histograms."""
    if not inputs:
        raise SystemExit(f"No input files found for {output.name}")
    if output.exists() and not force:
        raise SystemExit(f"Output exists: {output}. Use --force to overwrite.")

    hadd = shutil.which("hadd")
    if not hadd:
        raise SystemExit("Cannot find ROOT hadd in PATH. Source ROOT/ADNE environment first.")

    cmd = [hadd, "-f" if force else "-k", str(output)] + [str(path) for path in inputs]
    print(f"Merging {len(inputs)} files -> {output}")
    subprocess.run(cmd, check=True)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Merge parallel ADNE outputs / 合并 ADNE 并行输出",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=r"""Examples / 运行例子:
  ./lsy_nfs/merge_adne_parallel_outputs.py \
    --out-base /data/out_parallel \
    --force

  ./lsy_nfs/merge_adne_parallel_outputs.py \
    --out-base /data/out_parallel \
    --nfs-tree-name nfs_tree_all.root \
    --mult3-tree-name mult3_nfs_tree_all.root \
    --histo-name nfs_histoExogam2_all.root \
    --force
""",
    )
    parser.add_argument(
        "--out-base",
        required=True,
        help="base output directory containing par_out001, par_out002, ...",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="overwrite merged output files if they already exist",
    )
    parser.add_argument(
        "--nfs-tree-name",
        default="nfs_tree_merged.root",
        help="merged NFS tree output filename",
    )
    parser.add_argument(
        "--mult3-tree-name",
        default="mult3_nfs_tree_merged.root",
        help="merged mult3 NFS tree output filename",
    )
    parser.add_argument(
        "--histo-name",
        default="nfs_histoExogam2_merged.root",
        help="merged NFS histogram output filename",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    out_base = Path(args.out_base).expanduser().resolve()
    if not out_base.exists():
        raise SystemExit(f"Output base does not exist: {out_base}")

    groups = [
        (out_base / args.nfs_tree_name, collect_files(out_base, "nfs_run_*.root")),
        (out_base / args.mult3_tree_name, collect_files(out_base, "mult3_nfs_run_*.root")),
        (out_base / args.histo_name, collect_files(out_base, "nfs_histoExogam2_1.root")),
    ]

    for output, inputs in groups:
        run_hadd(output, inputs, args.force)

    print("Merged outputs:")
    for output, _inputs in groups:
        print(f"  {output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
