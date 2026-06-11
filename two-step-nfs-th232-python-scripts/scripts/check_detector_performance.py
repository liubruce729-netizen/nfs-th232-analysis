#!/usr/bin/env python3
"""
Quick detector-performance check for Exogam2 ROOT trees.
Exogam2 ROOT tree 的快速探测器性能检查脚本。

This script is meant as a first-pass diagnostic:
这个脚本用于做第一轮快速诊断：
- save all-detector and per-clover spectra for fEXO, sEXO, and aEXO;
  保存所有探测器以及逐 clover 的 fEXO、sEXO、aEXO 能谱；
- fill a clover-clover response matrix with aEXO data;
  用 aEXO 数据填充 clover-clover 两两响应矩阵；
- fill crystal-crystal response matrices with fEXO and sEXO data;
  用 fEXO 和 sEXO 数据填充 crystal-crystal 两两响应矩阵；
- calculate BGO/CSI fire efficiencies per clover and per crystal.
  计算每个 clover 以及每个 crystal 的 BGO/CSI fire 效率。

BGO/CSI efficiency definition:
BGO/CSI 效率定义：
- denominator: one count per event when a clover/crystal has positive gamma
  energy deposition in fEXO_ECC_E_*;
  分母：某个 event 中该 clover/crystal 在 fEXO_ECC_E_* 里有正的 gamma 能量沉积，则计一次；
- numerator: among those denominator events, count once if the matching
  fEXO_ESS_BGO or fEXO_ESS_CSI value is above threshold for the same
  clover/crystal;
  分子：在这些分母事件中，若同一 clover/crystal 对应的 fEXO_ESS_BGO 或
  fEXO_ESS_CSI 大于阈值，则计一次；
- thresholds default to 0, so any non-zero BGO/CSI value is treated as fire.
  阈值默认是 0，也就是任何非零 BGO/CSI 值都认为 fire。
"""

from __future__ import annotations

import argparse
import glob
import os
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

import ROOT


DEFAULT_INPUT = "/home/user0/work/IJCLAB/NFS/data_test_out/run_100_r0.root"
DEFAULT_OUTPUT = "/home/user0/work/IJCLAB/NFS/data_ana/tool/detector_performance_check.root"

NEEDED_BRANCHES = [
    "fEXO_ECC_E_Clover",
    "fEXO_ECC_E_Cristal",
    "fEXO_ECC_E_Energy",
    "sEXO_GammaEnergy",
    "sEXO_GammaCoreId",
    "aEXO_GammaEnergy",
    "aEXO_GammaCoreId",
    "fEXO_ESS_Clover",
    "fEXO_ESS_Cristal",
    "fEXO_ESS_BGO",
    "fEXO_ESS_CSI",
]


@dataclass(frozen=True)
class AnalysisConfig:
    tree_name: str
    output: Path
    max_events: int
    progress: int
    checkpoint_events: int
    clover_count: int
    crystal_count: int
    gamma_bins: int
    gamma_min: float
    gamma_max: float
    bgo_threshold: float
    csi_threshold: float


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Quick Exogam2 detector-performance check / Exogam2 探测器性能快速检查")
    parser.add_argument("inputs", nargs="*", default=[DEFAULT_INPUT], help="input ROOT files or glob patterns / 输入 ROOT 文件或通配符")
    parser.add_argument("-o", "--output", default=DEFAULT_OUTPUT, help="output ROOT file / 输出 ROOT 文件")
    parser.add_argument("--tree", default="TreeMaster", help="tree name / tree 名称")
    parser.add_argument("--max-events", type=int, default=-1, help="maximum events to process; negative means all / 最大处理事件数，负数表示全部")
    parser.add_argument("--progress", type=int, default=200000, help="print progress every N events; 0 disables / 每 N 个 event 打印进度，0 表示关闭")
    parser.add_argument("--checkpoint-events", type=int, default=500000, help="rewrite output every N events; 0 disables / 每 N 个 event 重写一次输出，0 表示关闭")
    parser.add_argument("--clover-count", type=int, default=16, help="number of clover bins / clover bin 数")
    parser.add_argument("--crystal-count", type=int, default=4, help="number of crystal bins per clover / 每个 clover 的 crystal 数")
    parser.add_argument("--gamma-bins", type=int, default=4096, help="gamma histogram bins / gamma 能谱 bin 数")
    parser.add_argument("--gamma-min", type=float, default=0.0, help="gamma lower edge / gamma 能量下限")
    parser.add_argument("--gamma-max", type=float, default=4096.0, help="gamma upper edge / gamma 能量上限")
    parser.add_argument("--bgo-threshold", type=float, default=0.0, help="BGO fire threshold / BGO fire 阈值")
    parser.add_argument("--csi-threshold", type=float, default=0.0, help="CSI fire threshold / CSI fire 阈值")
    return parser.parse_args()


def make_config(args: argparse.Namespace) -> AnalysisConfig:
    return AnalysisConfig(
        tree_name=args.tree,
        output=Path(args.output),
        max_events=args.max_events,
        progress=args.progress,
        checkpoint_events=args.checkpoint_events,
        clover_count=args.clover_count,
        crystal_count=args.crystal_count,
        gamma_bins=args.gamma_bins,
        gamma_min=args.gamma_min,
        gamma_max=args.gamma_max,
        bgo_threshold=args.bgo_threshold,
        csi_threshold=args.csi_threshold,
    )


def expand_inputs(patterns: Iterable[str]) -> list[str]:
    files: list[str] = []
    for pattern in patterns:
        matches = sorted(glob.glob(pattern))
        files.extend(matches if matches else [pattern])

    missing = [path for path in files if not Path(path).is_file()]
    if missing:
        raise FileNotFoundError("Missing input file(s): " + ", ".join(missing))
    return files


def make_h1(name: str, title: str, cfg: AnalysisConfig) -> ROOT.TH1F:
    hist = ROOT.TH1F(name, title, cfg.gamma_bins, cfg.gamma_min, cfg.gamma_max)
    hist.SetDirectory(0)
    return hist


def make_h1_indices(name: str, title: str, bins: int) -> ROOT.TH1F:
    hist = ROOT.TH1F(name, title, bins, 0, bins)
    hist.SetDirectory(0)
    return hist


def make_h2_indices(name: str, title: str, bins: int) -> ROOT.TH2F:
    hist = ROOT.TH2F(name, title, bins, 0, bins, bins, 0, bins)
    hist.SetDirectory(0)
    return hist


def label_clover_axis(axis: ROOT.TAxis, clover_count: int) -> None:
    for index in range(clover_count):
        axis.SetBinLabel(index + 1, str(index))


def label_crystal_axis(axis: ROOT.TAxis, clover_count: int, crystal_count: int) -> None:
    for clover in range(1, clover_count + 1):
        for crystal in range(crystal_count):
            axis_index = (clover - 1) * crystal_count + crystal
            global_crystal = (clover - 1) * crystal_count + crystal + 1
            axis.SetBinLabel(axis_index + 1, f"{clover}-{global_crystal}")


def set_clover_labels(hist: ROOT.TH2, clover_count: int) -> None:
    label_clover_axis(hist.GetXaxis(), clover_count)
    label_clover_axis(hist.GetYaxis(), clover_count)


def set_crystal_labels(hist: ROOT.TH2, clover_count: int, crystal_count: int) -> None:
    label_crystal_axis(hist.GetXaxis(), clover_count, crystal_count)
    label_crystal_axis(hist.GetYaxis(), clover_count, crystal_count)


def vector_values(vector, cast=float) -> list:
    return [cast(vector.at(i)) for i in range(int(vector.size()))]


def clover_from_core_id(core_id: int) -> int:
    return int(core_id) // 4


def crystal_from_core_id(core_id: int) -> int:
    return int(core_id) % 4


def crystal_label(crystal: int) -> str:
    labels = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    if 0 <= int(crystal) < len(labels):
        return labels[int(crystal)]
    return str(crystal)


def crystal_matrix_index(clover: int, crystal: int, cfg: AnalysisConfig) -> int | None:
    # Axis labels start at clover 1, matching labels such as 1-1 ... 2-5.
    # 坐标轴标签从 clover 1 开始，匹配 1-1 ... 2-5 这样的标记方式。
    index = (int(clover) - 1) * cfg.crystal_count + int(crystal)
    if 0 <= index < cfg.clover_count * cfg.crystal_count:
        return index
    return None


def fill_index(hist: ROOT.TH1, index: int) -> None:
    hist.Fill(index + 0.5)


def fill_gamma(hist: ROOT.TH1, values: Iterable[float]) -> None:
    for value in values:
        if value <= 0:
            continue
        hist.Fill(value)


def fill_ordered_index_pairs(matrix: ROOT.TH2, indices: Iterable[int]) -> None:
    unique_indices = sorted(set(indices))
    if len(unique_indices) < 2:
        return
    for i, first in enumerate(unique_indices):
        for second in unique_indices[i + 1 :]:
            matrix.Fill(first + 0.5, second + 0.5)
            matrix.Fill(second + 0.5, first + 0.5)


def build_chain(files: list[str], cfg: AnalysisConfig) -> ROOT.TChain:
    chain = ROOT.TChain(cfg.tree_name)
    for path in files:
        added = chain.Add(path)
        if added == 0:
            raise RuntimeError(f"Could not add input file to TChain: {path}")

    chain.SetBranchStatus("*", 0)
    for branch in NEEDED_BRANCHES:
        chain.SetBranchStatus(branch, 1)

    chain.SetCacheSize(64 * 1024 * 1024)
    for branch in NEEDED_BRANCHES:
        chain.AddBranchToCache(branch, True)
    return chain


def book_histograms(cfg: AnalysisConfig) -> dict[str, ROOT.TH1]:
    h: dict[str, ROOT.TH1] = {}

    # Energy spectra: all detectors, each clover, and each crystal.
    # 能谱：所有探测器整体、逐 clover，以及逐 crystal 保存。
    for prefix, title in (
        ("fEXO", "raw fEXO crystal energy"),
        ("sEXO", "selected sEXO crystal energy"),
        ("aEXO", "stored aEXO addback energy"),
    ):
        h[f"{prefix}_all"] = make_h1(f"h_{prefix}_all", f"{title};energy;counts", cfg)
        for clover in range(cfg.clover_count):
            h[f"{prefix}_clover{clover}"] = make_h1(
                f"h_{prefix}_clover{clover}",
                f"{title} clover {clover};energy;counts",
                cfg,
            )
            for crystal in range(cfg.crystal_count):
                label = crystal_label(crystal)
                h[f"{prefix}_crystal_clover{clover}_{label}"] = make_h1(
                    f"h_{prefix}_crystal_clover{clover}_{label}",
                    f"{title} clover {clover} crystal {label};energy;counts",
                    cfg,
                )

    # Detector response matrices.
    # 探测器两两响应矩阵。
    clover_matrix = make_h2_indices(
        "h2_clover_response_aEXO",
        "aEXO clover-clover response;clover;clover",
        cfg.clover_count,
    )
    set_clover_labels(clover_matrix, cfg.clover_count)
    h["clover_response_aEXO"] = clover_matrix

    crystal_bins = cfg.clover_count * cfg.crystal_count
    for prefix, title in (
        ("fEXO", "fEXO crystal-crystal response"),
        ("sEXO", "sEXO crystal-crystal response"),
    ):
        matrix = make_h2_indices(f"h2_crystal_response_{prefix}", f"{title};crystal;crystal", crystal_bins)
        set_crystal_labels(matrix, cfg.clover_count, cfg.crystal_count)
        h[f"crystal_response_{prefix}"] = matrix

    # BGO/CSI efficiency counters and final efficiency histograms.
    # BGO/CSI 效率所需的计数直方图，以及最终效率直方图。
    h["gamma_count_clover"] = make_h1_indices(
        "h_count_gamma_clover",
        "gamma-deposit event count per clover;clover;events",
        cfg.clover_count,
    )
    h["BGO_fire_count_clover"] = make_h1_indices(
        "h_count_BGO_fire_clover",
        "BGO-fire event count per clover;clover;events",
        cfg.clover_count,
    )
    h["CSI_fire_count_clover"] = make_h1_indices(
        "h_count_CSI_fire_clover",
        "CSI-fire event count per clover;clover;events",
        cfg.clover_count,
    )
    h["eff_BGO_clover"] = make_h1_indices(
        "h_eff_BGO_clover",
        "BGO fire efficiency per clover;clover;efficiency",
        cfg.clover_count,
    )
    h["eff_CSI_clover"] = make_h1_indices(
        "h_eff_CSI_clover",
        "CSI fire efficiency per clover;clover;efficiency",
        cfg.clover_count,
    )
    for key in ("gamma_count_clover", "BGO_fire_count_clover", "CSI_fire_count_clover", "eff_BGO_clover", "eff_CSI_clover"):
        label_clover_axis(h[key].GetXaxis(), cfg.clover_count)

    h["gamma_count_crystal"] = make_h1_indices(
        "h_count_gamma_crystal",
        "gamma-deposit event count per crystal;clover-crystal;events",
        crystal_bins,
    )
    h["BGO_fire_count_crystal"] = make_h1_indices(
        "h_count_BGO_fire_crystal",
        "BGO-fire event count per crystal;clover-crystal;events",
        crystal_bins,
    )
    h["CSI_fire_count_crystal"] = make_h1_indices(
        "h_count_CSI_fire_crystal",
        "CSI-fire event count per crystal;clover-crystal;events",
        crystal_bins,
    )
    h["eff_BGO_crystal"] = make_h1_indices(
        "h_eff_BGO_crystal",
        "BGO fire efficiency per crystal;clover-crystal;efficiency",
        crystal_bins,
    )
    h["eff_CSI_crystal"] = make_h1_indices(
        "h_eff_CSI_crystal",
        "CSI fire efficiency per crystal;clover-crystal;efficiency",
        crystal_bins,
    )
    for key in ("gamma_count_crystal", "BGO_fire_count_crystal", "CSI_fire_count_crystal", "eff_BGO_crystal", "eff_CSI_crystal"):
        label_crystal_axis(h[key].GetXaxis(), cfg.clover_count, cfg.crystal_count)

    for key in ("eff_BGO_clover", "eff_CSI_clover", "eff_BGO_crystal", "eff_CSI_crystal"):
        h[key].SetMinimum(0.0)
        h[key].SetMaximum(1.0)

    return h


def process_event(chain: ROOT.TChain, hist: dict[str, ROOT.TH1], cfg: AnalysisConfig, bgo_csi_available: bool) -> None:
    fexo_clovers = vector_values(chain.fEXO_ECC_E_Clover, int)
    fexo_crystals = vector_values(chain.fEXO_ECC_E_Cristal, int)
    fexo_energies = vector_values(chain.fEXO_ECC_E_Energy, float)

    sexo_energies = vector_values(chain.sEXO_GammaEnergy, float)
    sexo_core_ids = vector_values(chain.sEXO_GammaCoreId, int)
    sexo_clovers = [clover_from_core_id(core_id) for core_id in sexo_core_ids]
    sexo_crystals = [crystal_from_core_id(core_id) for core_id in sexo_core_ids]

    aexo_energies = vector_values(chain.aEXO_GammaEnergy, float)
    aexo_core_ids = vector_values(chain.aEXO_GammaCoreId, int)
    aexo_clovers = [clover_from_core_id(core_id) for core_id in aexo_core_ids]
    aexo_crystals = [crystal_from_core_id(core_id) for core_id in aexo_core_ids]

    fill_gamma(hist["fEXO_all"], fexo_energies)
    fill_gamma(hist["sEXO_all"], sexo_energies)
    fill_gamma(hist["aEXO_all"], aexo_energies)

    for clover, crystal, energy in zip(fexo_clovers, fexo_crystals, fexo_energies):
        if 0 <= clover < cfg.clover_count and 0 <= crystal < cfg.crystal_count and energy > 0:
            label = crystal_label(crystal)
            hist[f"fEXO_clover{clover}"].Fill(energy)
            hist[f"fEXO_crystal_clover{clover}_{label}"].Fill(energy)
    for clover, crystal, energy in zip(sexo_clovers, sexo_crystals, sexo_energies):
        if 0 <= clover < cfg.clover_count and 0 <= crystal < cfg.crystal_count and energy > 0:
            label = crystal_label(crystal)
            hist[f"sEXO_clover{clover}"].Fill(energy)
            hist[f"sEXO_crystal_clover{clover}_{label}"].Fill(energy)
    for clover, crystal, energy in zip(aexo_clovers, aexo_crystals, aexo_energies):
        if 0 <= clover < cfg.clover_count and 0 <= crystal < cfg.crystal_count and energy > 0:
            label = crystal_label(crystal)
            hist[f"aEXO_clover{clover}"].Fill(energy)
            hist[f"aEXO_crystal_clover{clover}_{label}"].Fill(energy)

    aexo_hit_clovers = [clover for clover, energy in zip(aexo_clovers, aexo_energies) if 0 <= clover < cfg.clover_count and energy > 0]
    fill_ordered_index_pairs(hist["clover_response_aEXO"], aexo_hit_clovers)

    fexo_indices = []
    gamma_clovers: set[int] = set()
    gamma_crystals: set[int] = set()
    for clover, crystal, energy in zip(fexo_clovers, fexo_crystals, fexo_energies):
        if energy <= 0:
            continue
        if 0 <= clover < cfg.clover_count:
            gamma_clovers.add(clover)
        index = crystal_matrix_index(clover, crystal, cfg)
        if index is not None:
            fexo_indices.append(index)
            gamma_crystals.add(index)
    fill_ordered_index_pairs(hist["crystal_response_fEXO"], fexo_indices)

    sexo_indices = []
    for clover, crystal, energy in zip(sexo_clovers, sexo_crystals, sexo_energies):
        if energy <= 0:
            continue
        index = crystal_matrix_index(clover, crystal, cfg)
        if index is not None:
            sexo_indices.append(index)
    fill_ordered_index_pairs(hist["crystal_response_sEXO"], sexo_indices)

    for clover in gamma_clovers:
        fill_index(hist["gamma_count_clover"], clover)
    for crystal_index in gamma_crystals:
        fill_index(hist["gamma_count_crystal"], crystal_index)

    if not bgo_csi_available:
        return

    ess_clovers = vector_values(chain.fEXO_ESS_Clover, int)
    ess_crystals = vector_values(chain.fEXO_ESS_Cristal, int)
    ess_bgo = vector_values(chain.fEXO_ESS_BGO, float)
    ess_csi = vector_values(chain.fEXO_ESS_CSI, float)

    bgo_fire_clovers: set[int] = set()
    csi_fire_clovers: set[int] = set()
    bgo_fire_crystals: set[int] = set()
    csi_fire_crystals: set[int] = set()

    for clover, crystal, bgo_value, csi_value in zip(ess_clovers, ess_crystals, ess_bgo, ess_csi):
        index = crystal_matrix_index(clover, crystal, cfg)
        if bgo_value > cfg.bgo_threshold:
            if 0 <= clover < cfg.clover_count:
                bgo_fire_clovers.add(clover)
            if index is not None:
                bgo_fire_crystals.add(index)
        if csi_value > cfg.csi_threshold:
            if 0 <= clover < cfg.clover_count:
                csi_fire_clovers.add(clover)
            if index is not None:
                csi_fire_crystals.add(index)

    # Only denominator-qualified gamma events can contribute to the numerator.
    # 只有已经进入分母的 gamma 事件，才允许进入 BGO/CSI 分子。
    for clover in gamma_clovers:
        if clover in bgo_fire_clovers:
            fill_index(hist["BGO_fire_count_clover"], clover)
        if clover in csi_fire_clovers:
            fill_index(hist["CSI_fire_count_clover"], clover)
    for crystal_index in gamma_crystals:
        if crystal_index in bgo_fire_crystals:
            fill_index(hist["BGO_fire_count_crystal"], crystal_index)
        if crystal_index in csi_fire_crystals:
            fill_index(hist["CSI_fire_count_crystal"], crystal_index)


def branch_exists(chain: ROOT.TChain, branch_name: str) -> bool:
    return bool(chain.GetBranch(branch_name))


def update_ratio_hist(eff_hist: ROOT.TH1, numerator: ROOT.TH1, denominator: ROOT.TH1) -> None:
    for bin_index in range(1, denominator.GetNbinsX() + 1):
        den = denominator.GetBinContent(bin_index)
        num = numerator.GetBinContent(bin_index)
        if den <= 0:
            eff_hist.SetBinContent(bin_index, 0.0)
            eff_hist.SetBinError(bin_index, 0.0)
            continue
        ratio = num / den
        eff_hist.SetBinContent(bin_index, ratio)
        if 0.0 <= ratio <= 1.0:
            eff_hist.SetBinError(bin_index, (ratio * (1.0 - ratio) / den) ** 0.5)
        else:
            eff_hist.SetBinError(bin_index, 0.0)


def update_efficiency_histograms(hist: dict[str, ROOT.TH1]) -> None:
    update_ratio_hist(hist["eff_BGO_clover"], hist["BGO_fire_count_clover"], hist["gamma_count_clover"])
    update_ratio_hist(hist["eff_CSI_clover"], hist["CSI_fire_count_clover"], hist["gamma_count_clover"])
    update_ratio_hist(hist["eff_BGO_crystal"], hist["BGO_fire_count_crystal"], hist["gamma_count_crystal"])
    update_ratio_hist(hist["eff_CSI_crystal"], hist["CSI_fire_count_crystal"], hist["gamma_count_crystal"])


def write_output(
    hist: dict[str, ROOT.TH1],
    cfg: AnalysisConfig,
    input_files: list[str],
    processed_entries: int,
    total_entries: int,
    bgo_csi_available: bool,
) -> None:
    update_efficiency_histograms(hist)
    cfg.output.parent.mkdir(parents=True, exist_ok=True)
    tmp_output = cfg.output.with_name(cfg.output.name + ".tmp")
    out = ROOT.TFile.Open(str(tmp_output), "RECREATE")
    if not out or out.IsZombie():
        raise RuntimeError(f"Cannot create output file: {tmp_output}")

    ROOT.TNamed("kind", "detector_performance_check").Write()
    ROOT.TNamed("input_files", "\n".join(input_files)).Write()
    ROOT.TNamed("processed_entries", str(processed_entries)).Write()
    ROOT.TNamed("total_entries", str(total_entries)).Write()
    ROOT.TNamed(
        "bgo_csi_mapping",
        (
            "available: fEXO_ESS_Clover/fEXO_ESS_Cristal/fEXO_ESS_BGO/fEXO_ESS_CSI"
            if bgo_csi_available
            else "not available in this tree"
        ),
    ).Write()
    ROOT.TNamed(
        "bgo_csi_efficiency_definition",
        (
            "denominator=fEXO positive gamma-deposit events per clover/crystal; "
            "numerator=same events with matching BGO/CSI above threshold"
        ),
    ).Write()
    ROOT.TNamed("bgo_threshold", str(cfg.bgo_threshold)).Write()
    ROOT.TNamed("csi_threshold", str(cfg.csi_threshold)).Write()

    for obj in hist.values():
        obj.Write()
    out.Close()
    os.replace(tmp_output, cfg.output)


def main() -> int:
    args = parse_args()
    cfg = make_config(args)
    files = expand_inputs(args.inputs)
    ROOT.gROOT.SetBatch(True)

    chain = build_chain(files, cfg)
    total_entries = int(chain.GetEntries())
    requested_entries = total_entries if cfg.max_events < 0 else min(total_entries, cfg.max_events)
    hist = book_histograms(cfg)

    bgo_csi_available = all(branch_exists(chain, branch) for branch in ("fEXO_ESS_Clover", "fEXO_ESS_Cristal", "fEXO_ESS_BGO", "fEXO_ESS_CSI"))

    print(f"Input files: {len(files)}")
    print(f"Tree: {cfg.tree_name}")
    print(f"Total entries: {total_entries}")
    print(f"Entries to process: {requested_entries}")
    print(f"Output: {cfg.output}")
    print(f"BGO/CSI mapping branches: {'available' if bgo_csi_available else 'not available'}")
    print(f"BGO/CSI thresholds: BGO>{cfg.bgo_threshold}, CSI>{cfg.csi_threshold}")

    for entry in range(requested_entries):
        chain.GetEntry(entry)
        process_event(chain, hist, cfg, bgo_csi_available)
        done = entry + 1
        if cfg.progress > 0 and done % cfg.progress == 0:
            print(f"Processed {done}/{requested_entries}")
        if cfg.checkpoint_events > 0 and done % cfg.checkpoint_events == 0:
            write_output(hist, cfg, files, done, total_entries, bgo_csi_available)
            print(f"Checkpoint written: {cfg.output} ({done}/{requested_entries})")

    write_output(hist, cfg, files, requested_entries, total_entries, bgo_csi_available)
    print("Done")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
