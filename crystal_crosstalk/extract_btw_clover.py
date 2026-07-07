#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Between-clover cross-talk extractor.
不同 clover 之间串扰提取工具。

Purpose / 目的
--------------
Create a new 2D histogram that keeps only between-clover entries.
The input 2D histogram is assumed to contain crystal-pair cross-talk bins
ordered as:

  0-0, 0-1, 0-2, 0-3, 1-0, ..., 15-3

生成一个新的二维直方图，只保留不同 clover 之间的串扰。
输入二维图的 X/Y bin 顺序假定为：

  0-0, 0-1, 0-2, 0-3, 1-0, ..., 15-3

Default behavior / 默认行为
---------------------------
- Keep clovers 2..13, crystals 0..3.
- Set same-clover pairs to zero, for example 2-0 vs 2-1 and 2-1 vs 2-0.
- Write the new histogram back into the same ROOT file in UPDATE mode.
- Output histogram name defaults to <input_basename>_btw_clover.

- 默认保留 clover 2..13，每个 clover 4 个晶体 0..3。
- 同一个 clover 内的组合置 0，例如 2-0 对 2-1、2-1 对 2-0。
- 使用 UPDATE 模式直接写回原 ROOT 文件。
- 输出图名默认为 <输入图基础名>_btw_clover。

Examples / 示例
---------------
1) Histogram at ROOT top level / 图在 ROOT 顶层：

   python3 extract_btw_clover.py -i input.root -n HistoName

   Output / 输出：input.root:HistoName_btw_clover

2) Histogram inside a ROOT directory / 图在 ROOT 文件夹里：

   python3 extract_btw_clover.py -i input.root -n folder/HistoName

   Output / 输出：input.root:folder/HistoName_btw_clover

3) Custom clover range / 自定义 clover 范围：

   python3 extract_btw_clover.py -i input.root -n HistoName \
     --first-clover 2 --last-clover 13 --crystals 4

4) Custom output name / 自定义输出图名：

   python3 extract_btw_clover.py -i input.root -n HistoName -o HistoName_diff_clover

Dependency / 依赖
-----------------
Requires PyROOT. If `import ROOT` fails, source your ROOT environment first,
for example:

   source /path/to/root/bin/thisroot.sh

需要 PyROOT。如果 `import ROOT` 失败，请先 source ROOT 环境，例如：

   source /path/to/root/bin/thisroot.sh
"""

import argparse
from dataclasses import dataclass


@dataclass(frozen=True)
class CrystalID:
    """One detector crystal identifier. / 单个探测器晶体编号。"""

    clover: int
    crystal: int

    @property
    def label(self) -> str:
        return f"{self.clover}-{self.crystal}"


EXAMPLES = """
Examples / 示例:
  # Histogram at ROOT top level / 图在 ROOT 顶层
  python3 extract_btw_clover.py -i input.root -n HistoName

  # Histogram inside a ROOT directory / 图在 ROOT 文件夹里
  python3 extract_btw_clover.py -i input.root -n folder/HistoName

  # Custom clover range / 自定义 clover 范围
  python3 extract_btw_clover.py -i input.root -n HistoName \
    --first-clover 2 --last-clover 13 --crystals 4

  # Custom output name / 自定义输出图名
  python3 extract_btw_clover.py -i input.root -n HistoName -o HistoName_diff_clover
"""


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Extract between-clover cross-talk from a 2D ROOT histogram. "
            "从二维 ROOT 图中提取不同 clover 之间的串扰。"
        ),
        epilog=EXAMPLES,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "-i",
        "--input",
        required=True,
        help="ROOT file to update in place / 要直接写回更新的 ROOT 文件",
    )
    parser.add_argument(
        "-n",
        "--hist",
        required=True,
        help="input TH2 name/path, e.g. HistoName or folder/HistoName / 输入 TH2 名或路径",
    )
    parser.add_argument(
        "-o",
        "--output",
        help=(
            "output TH2 name only; default is basename(input_hist)_btw_clover / "
            "输出 TH2 名，默认是 输入图基础名_btw_clover"
        ),
    )
    parser.add_argument(
        "--first-clover",
        type=int,
        default=2,
        help="first clover to keep / 保留的第一个 clover，默认 2",
    )
    parser.add_argument(
        "--last-clover",
        type=int,
        default=13,
        help="last clover to keep / 保留的最后一个 clover，默认 13",
    )
    parser.add_argument(
        "--crystals",
        type=int,
        default=4,
        help="number of crystals per clover / 每个 clover 的晶体数，默认 4",
    )
    return parser.parse_args()


def basename(root_path: str) -> str:
    """Return object basename from a ROOT object path. / 从 ROOT 对象路径取基础名。"""
    return root_path.rstrip("/").split("/")[-1]


def dirname(root_path: str) -> str:
    """Return directory part from a ROOT object path. / 从 ROOT 对象路径取目录名。"""
    parts = root_path.rstrip("/").split("/")
    return "/".join(parts[:-1])


def make_ids(first_clover: int, last_clover: int, crystals: int) -> list[CrystalID]:
    """Build kept crystal IDs in input bin order. / 按输入 bin 顺序生成保留的晶体编号。"""
    return [
        CrystalID(clover, crystal)
        for clover in range(first_clover, last_clover + 1)
        for crystal in range(crystals)
    ]


def copy_axis_labels(hist, ids: list[CrystalID]) -> None:
    """Label output X/Y axes as clover-crystal. / 把输出图 X/Y 轴标成 clover-crystal。"""
    for idx, crystal_id in enumerate(ids, start=1):
        hist.GetXaxis().SetBinLabel(idx, crystal_id.label)
        hist.GetYaxis().SetBinLabel(idx, crystal_id.label)


def main() -> int:
    args = parse_args()

    import ROOT

    if args.first_clover < 0:
        raise ValueError("--first-clover must be >= 0")
    if args.last_clover < args.first_clover:
        raise ValueError("--last-clover must be >= --first-clover")
    if args.crystals <= 0:
        raise ValueError("--crystals must be > 0")

    root_file = ROOT.TFile.Open(args.input, "UPDATE")
    if not root_file or root_file.IsZombie():
        raise RuntimeError(f"Cannot open ROOT file in UPDATE mode: {args.input}")

    hist = root_file.Get(args.hist)
    if not hist:
        root_file.Close()
        raise RuntimeError(f"Cannot find histogram: {args.hist}")
    if not hist.InheritsFrom("TH2"):
        root_file.Close()
        raise RuntimeError(f"{args.hist} is not a TH2 histogram")

    ids = make_ids(args.first_clover, args.last_clover, args.crystals)
    n_keep = len(ids)
    min_bins = (args.last_clover + 1) * args.crystals
    if hist.GetNbinsX() < min_bins:
        root_file.Close()
        raise RuntimeError("Input histogram has too few X bins for the requested clover range")
    if hist.GetNbinsY() < min_bins:
        root_file.Close()
        raise RuntimeError("Input histogram has too few Y bins for the requested clover range")

    out_name = args.output or f"{basename(args.hist)}_btw_clover"
    out_dir_name = dirname(args.hist)
    out = ROOT.TH2D(out_name, out_name, n_keep, 0, n_keep, n_keep, 0, n_keep)

    copy_axis_labels(out, ids)

    for out_x, x_id in enumerate(ids, start=1):
        # Convert clover-crystal ID to original 1-based ROOT bin index.
        # 将 clover-crystal 编号转换为原图中 ROOT 的 1-based bin 编号。
        in_x = x_id.clover * args.crystals + x_id.crystal + 1
        for out_y, y_id in enumerate(ids, start=1):
            in_y = y_id.clover * args.crystals + y_id.crystal + 1

            # Same-clover cross talk is suppressed; between-clover entries are copied.
            # 同一个 clover 内的组合置 0；不同 clover 之间的 bin 保留原值。
            if x_id.clover == y_id.clover:
                out.SetBinContent(out_x, out_y, 0.0)
                out.SetBinError(out_x, out_y, 0.0)
                continue
            out.SetBinContent(out_x, out_y, hist.GetBinContent(in_x, in_y))
            out.SetBinError(out_x, out_y, hist.GetBinError(in_x, in_y))

    # If the input histogram is in a ROOT directory, write the output there too.
    # 如果输入图在 ROOT 文件夹里，输出图也写回同一个文件夹。
    if out_dir_name:
        out_dir = root_file.Get(out_dir_name)
        if not out_dir:
            root_file.Close()
            raise RuntimeError(f"Cannot find output directory: {out_dir_name}")
        out_dir.cd()
        written_path = f"{out_dir_name}/{out_name}"
    else:
        root_file.cd()
        written_path = out_name
    out.Write(out_name, ROOT.TObject.kOverwrite)
    root_file.Close()

    print(f"Wrote {args.input}:{written_path}")
    print(f"Kept clovers {args.first_clover}..{args.last_clover}; same-clover pairs set to 0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
