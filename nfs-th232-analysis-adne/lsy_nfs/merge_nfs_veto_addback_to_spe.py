#!/usr/bin/env python3
"""Merge NFS clover addback veto spectra from many ADNE output folders.

CN:
  递归遍历一个输出目录下的子目录，寻找 nfs_histoExogam2_1.root，
  提取所有 nfs_cloverX_addback_gamma_bgo_csi_veto 一维谱，并合并成一个总谱。
  如果提供 --spe-tool，则调用已有 SPE 转换工具把总谱转为 RadWare .spe。
  优先兼容 bg_processing/tools/root_projection.py 的 spe 子命令格式；
  同时保留 root_hist_to_spe.py 的旧命令行格式。

EN:
  Recursively scan an output directory for nfs_histoExogam2_1.root files,
  sum all nfs_cloverX_addback_gamma_bgo_csi_veto TH1 spectra into one
  total spectrum, and optionally convert it to RadWare .spe. The preferred
  converter is bg_processing/tools/root_projection.py with its spe subcommand;
  the older root_hist_to_spe.py command-line format is also supported.

Examples / 运行例子:
  # CN: 只生成合并后的 ROOT 文件。
  # EN: Produce only the merged ROOT file.
  ./merge_nfs_veto_addback_to_spe.py \
    --input-dir /home/e877_ana/analysed_data/July6test_lsy/run12

  # CN: 同时转成 .spe；--spe-tool 指向 bg_processing/tools/root_projection.py。
  # EN: Also convert to .spe; --spe-tool points to bg_processing/tools/root_projection.py.
  ./merge_nfs_veto_addback_to_spe.py \
    --input-dir /home/e877_ana/analysed_data/July6test_lsy/run12 \
    --output-root /home/e877_ana/analysed_data/July6test_lsy/run12/run12_veto_addback_sum.root \
    --output-spe /home/e877_ana/analysed_data/July6test_lsy/run12/run12_veto_addback_sum.spe \
    --spe-tool /path/to/bg_processing/tools/root_projection.py
"""

from __future__ import annotations

import argparse
import fnmatch
import re
import subprocess
import sys
from pathlib import Path

import ROOT


DEFAULT_HIST_FILE = "nfs_histoExogam2_1.root"
DEFAULT_HIST_PATTERN = "nfs_clover*_addback_gamma_bgo_csi_veto"
MERGED_HIST_NAME = "nfs_all_clover_addback_gamma_bgo_csi_veto"


def natural_key(path: Path) -> list[object]:
    """CN/EN: Natural sort key, so par_out10 is after par_out9."""
    text = str(path)
    return [int(part) if part.isdigit() else part for part in re.split(r"(\d+)", text)]


def find_histo_files(input_dir: Path, histo_filename: str) -> list[Path]:
    """CN: 递归寻找指定 ROOT 文件；EN: recursively find histogram ROOT files."""
    return sorted(
        (path for path in input_dir.rglob(histo_filename) if path.is_file()),
        key=natural_key,
    )


def walk_root_directory(directory, prefix: str = ""):
    """Yield (path, object) pairs from a ROOT file/directory recursively."""
    for key in directory.GetListOfKeys():
        obj = key.ReadObj()
        name = f"{prefix}{key.GetName()}"
        if obj.InheritsFrom("TDirectory"):
            yield from walk_root_directory(obj, name + "/")
        else:
            yield name, obj


def is_compatible(a, b) -> bool:
    """CN: 合并前检查 binning；EN: check TH1 binning before adding."""
    return (
        a.GetNbinsX() == b.GetNbinsX()
        and abs(a.GetXaxis().GetXmin() - b.GetXaxis().GetXmin()) < 1e-9
        and abs(a.GetXaxis().GetXmax() - b.GetXaxis().GetXmax()) < 1e-9
    )


def merge_veto_spectra(files: list[Path], hist_pattern: str):
    """Return the merged TH1 and a short text summary."""
    merged = None
    matched_histograms = 0
    skipped_incompatible = 0
    summary_lines: list[str] = []

    for root_path in files:
        root_file = ROOT.TFile.Open(str(root_path), "READ")
        if not root_file or root_file.IsZombie():
            summary_lines.append(f"BAD_FILE\t{root_path}")
            continue

        file_matches = 0
        for hist_path, obj in walk_root_directory(root_file):
            hist_name = Path(hist_path).name
            if not fnmatch.fnmatch(hist_name, hist_pattern):
                continue
            if not obj.InheritsFrom("TH1") or obj.InheritsFrom("TH2"):
                continue

            file_matches += 1
            matched_histograms += 1
            if merged is None:
                merged = obj.Clone(MERGED_HIST_NAME)
                merged.SetDirectory(0)
                merged.Reset("ICES")
                merged.SetTitle(
                    "All clover addback gamma with BGO CSI veto;Energy (keV);Counts"
                )

            if is_compatible(merged, obj):
                merged.Add(obj)
            else:
                skipped_incompatible += 1
                summary_lines.append(f"SKIP_INCOMPATIBLE\t{root_path}\t{hist_path}")

        summary_lines.append(f"FILE\t{root_path}\tmatched={file_matches}")
        root_file.Close()

    if merged is None:
        raise RuntimeError(
            f"No compatible histograms matched pattern '{hist_pattern}' in input files."
        )

    summary_lines.append(f"TOTAL_FILES\t{len(files)}")
    summary_lines.append(f"TOTAL_MATCHED_HISTS\t{matched_histograms}")
    summary_lines.append(f"TOTAL_SKIPPED_INCOMPATIBLE\t{skipped_incompatible}")
    summary_lines.append(f"TOTAL_COUNTS\t{merged.Integral()}")
    return merged, summary_lines


def write_output_root(output_root: Path, hist, summary_lines: list[str]) -> None:
    """CN: 写出总谱和检查信息；EN: write merged spectrum and summary."""
    output_root.parent.mkdir(parents=True, exist_ok=True)
    out = ROOT.TFile(str(output_root), "RECREATE")
    hist.Write()

    canvas = ROOT.TCanvas("c_nfs_all_clover_addback_gamma_bgo_csi_veto", "", 1000, 700)
    hist.SetLineColor(ROOT.kBlue + 1)
    hist.SetLineWidth(2)
    hist.Draw("hist")
    canvas.Write()

    summary = ROOT.TNamed("merge_summary", "\n".join(summary_lines))
    summary.Write()
    out.Close()


def run_spe_converter(spe_tool: Path, output_root: Path, output_spe: Path, spe_name: str) -> None:
    """Call a supported ROOT-histogram to SPE converter.

    CN:
      - bg_processing/tools/root_projection.py 使用 spe 子命令：
        root_projection.py spe -i input.root -n hist -o output.spe --spe_name NAME。
      - 旧的 root_hist_to_spe.py 使用命令行参数 -i/-n/-o/--spe-name。

    EN:
      - bg_processing/tools/root_projection.py uses the spe subcommand:
        root_projection.py spe -i input.root -n hist -o output.spe --spe_name NAME.
      - The older root_hist_to_spe.py uses -i/-n/-o/--spe-name arguments.
    """
    output_spe.parent.mkdir(parents=True, exist_ok=True)

    if spe_tool.name == "root_projection.py":
        cmd = [
            sys.executable,
            str(spe_tool),
            "spe",
            "-i",
            str(output_root),
            "-n",
            MERGED_HIST_NAME,
            "-o",
            str(output_spe),
            "--spe_name",
            spe_name,
        ]
    elif spe_tool.suffix == ".py":
        cmd = [
            sys.executable,
            str(spe_tool),
            "-i",
            str(output_root),
            "-n",
            MERGED_HIST_NAME,
            "-o",
            str(output_spe),
            "--spe-name",
            spe_name,
        ]
    else:
        cmd = [
            str(spe_tool),
            "-i",
            str(output_root),
            "-n",
            MERGED_HIST_NAME,
            "-o",
            str(output_spe),
            "--spe-name",
            spe_name,
        ]

    print("Run SPE converter / 执行 SPE 转换:")
    print("  " + " ".join(cmd))
    sys.stdout.flush()
    subprocess.run(cmd, check=True)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Merge NFS addback BGO/CSI veto spectra and optionally convert to SPE.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("--input-dir", required=True, help="directory to scan recursively")
    parser.add_argument(
        "--histo-filename",
        default=DEFAULT_HIST_FILE,
        help=f"ROOT histogram filename to find, default: {DEFAULT_HIST_FILE}",
    )
    parser.add_argument(
        "--hist-pattern",
        default=DEFAULT_HIST_PATTERN,
        help=f"histogram name pattern, default: {DEFAULT_HIST_PATTERN}",
    )
    parser.add_argument(
        "--output-root",
        help="merged ROOT output, default: <input-dir>/nfs_veto_addback_sum.root",
    )
    parser.add_argument(
        "--output-spe",
        help="output SPE file, default: same path as --output-root with .spe suffix",
    )
    parser.add_argument(
        "--spe-tool",
        help="path to bg_processing/tools/root_projection.py, root_hist_to_spe.py, or compatible converter; omit to skip SPE output",
    )
    parser.add_argument(
        "--spe-name",
        default="VETOADD",
        help="RadWare internal spectrum name, max 8 chars, default: VETOADD",
    )
    return parser.parse_args()


def main() -> int:
    ROOT.gROOT.SetBatch(True)
    args = parse_args()

    input_dir = Path(args.input_dir).expanduser().resolve()
    if not input_dir.exists():
        raise SystemExit(f"Input directory does not exist: {input_dir}")

    output_root = (
        Path(args.output_root).expanduser().resolve()
        if args.output_root
        else input_dir / "nfs_veto_addback_sum.root"
    )
    output_spe = (
        Path(args.output_spe).expanduser().resolve()
        if args.output_spe
        else output_root.with_suffix(".spe")
    )

    files = find_histo_files(input_dir, args.histo_filename)
    if not files:
        raise SystemExit(f"No {args.histo_filename} files found under {input_dir}")

    print(f"Found histogram ROOT files / 找到 ROOT 文件: {len(files)}")
    for path in files:
        print(f"  {path}")

    hist, summary_lines = merge_veto_spectra(files, args.hist_pattern)
    write_output_root(output_root, hist, summary_lines)
    print(f"Wrote merged ROOT / 写出合并 ROOT: {output_root}")
    print(f"Merged histogram / 合并谱名: {MERGED_HIST_NAME}")
    print(f"Total counts / 总计数: {hist.Integral()}")

    if args.spe_tool:
        spe_tool = Path(args.spe_tool).expanduser().resolve()
        if not spe_tool.exists():
            raise SystemExit(f"SPE converter does not exist: {spe_tool}")
        run_spe_converter(spe_tool, output_root, output_spe, args.spe_name)
        print(f"Wrote SPE / 写出 SPE: {output_spe}")
    else:
        print("SPE conversion skipped / 未提供 --spe-tool，跳过 SPE 转换")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
