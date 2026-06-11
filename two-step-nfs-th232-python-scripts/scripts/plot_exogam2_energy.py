#!/usr/bin/env python3
"""
Fast check of gamma spectra versus neutron energy for Exogam2 ROOT trees.
Exogam2 ROOT tree 中 gamma 能谱随中子能量变化的快速检查脚本。

Purpose:
用途：
- make quick gamma-spectrum checks as a function of neutron-energy gates;
  按中子能量 gate 快速检查 gamma 能谱变化；
- keep only the essential spectra for this check;
  只保留这个检查所需的核心直方图；
- process large runs safely with per-input partial files, checkpointing, resume,
  and final histogram merging.
  对大数据按输入文件生成 partial，支持 checkpoint、续算，以及最后合并直方图。

Notes on branch meaning in this script:
本脚本中 branch 含义说明：
- fEXO is raw crystal/core information from fEXO_ECC_E_*;
  fEXO 是来自 fEXO_ECC_E_* 的原始 crystal/core 信息；
- sEXO is selected single-gamma information, without addback;
  sEXO 是选择后的 single-gamma 信息，不做 addback；
- aEXO is selected addback gamma information.  It is the canonical addback
  branch, so this script does not also keep a duplicated computed sEXO addback.
  aEXO 是选择后的 addback gamma 信息，是标准 addback branch，因此这里不再保留重复的 computed sEXO addback 图。

Example neutron gates:
中子能量 gate 示例：
  python3 plot_exogam2_energy.py -i 0 5 10 15 30 input.root

If -i/--neutron-edges is not supplied, the old default is kept: 0-50 MeV split
into 10 equal 5-MeV bins.
如果不输入 -i/--neutron-edges，则保持原默认设置：0-50 MeV 分成 10 个 5-MeV bin。
"""

from __future__ import annotations

import argparse
import glob
import hashlib
import os
from array import array
from dataclasses import dataclass, replace
from pathlib import Path
from typing import Iterable

import ROOT


DEFAULT_INPUT = "/home/user0/work/IJCLAB/NFS/data_test_out/run_100_r0.root"
DEFAULT_OUTPUT = (
    "/home/user0/work/IJCLAB/NFS/data_ana/tool/run_100_r0_energy_plots.root"
)

NEEDED_BRANCHES = [
    "fEXO_ECC_E_Clover",
    "fEXO_ECC_E_Energy",
    "sEXO_GammaEnergy",
    "sEXO_GammaCoreId",
    "aEXO_GammaEnergy",
    "aEXO_GammaCoreId",
    "fEXO_Neutron_NRJ",
]


@dataclass(frozen=True)
class AnalysisConfig:
    tree_name: str
    output: Path
    partial_dir: Path
    max_events: int
    checkpoint_events: int
    progress: int
    clover_id: int
    gamma_bins: int
    gamma_min: float
    gamma_max: float
    neutron_bins: int
    neutron_min: float
    neutron_max: float
    neutron_edges: tuple[float, ...]
    fexo_addback_threshold: float
    force_restart: bool
    merge_only: bool
    append_existing_output: bool


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Fast gamma-spectrum checks versus neutron energy / gamma 能谱随中子能量变化的快速检查"
    )
    parser.add_argument(
        "inputs",
        nargs="*",
        default=[DEFAULT_INPUT],
        help="input ROOT files or glob patterns / 输入 ROOT 文件或通配符",
    )
    parser.add_argument("-o", "--output", default=DEFAULT_OUTPUT, help="merged output ROOT file / 合并后的输出 ROOT 文件")
    parser.add_argument(
        "--partial-dir",
        default=None,
        help="directory for per-input partial ROOT files; default is <output stem>_parts / 每个输入文件 partial ROOT 的目录，默认 <输出名>_parts",
    )
    parser.add_argument("--tree", default="TreeMaster", help="tree name / tree 名称")
    parser.add_argument(
        "--max-events",
        type=int,
        default=-1,
        help="maximum entries to process per input file; negative means all entries / 每个输入文件最大处理 event 数，负数表示全部",
    )
    parser.add_argument(
        "--checkpoint-events",
        type=int,
        default=500000,
        help="write current per-file partial output every N processed entries / 每处理 N 个 event 写一次当前 partial 输出",
    )
    parser.add_argument(
        "--progress",
        type=int,
        default=200000,
        help="print progress every N entries; 0 disables progress output / 每 N 个 event 打印进度，0 表示关闭",
    )
    parser.add_argument("--clover-id", type=int, default=2, help="clover for special plots / 单独检查的 clover 编号")
    parser.add_argument("--gamma-bins", type=int, default=4096, help="gamma histogram bins / gamma 能谱 bin 数")
    parser.add_argument("--gamma-min", type=float, default=0.0, help="gamma lower edge / gamma 能量下限")
    parser.add_argument(
        "--gamma-max",
        type=float,
        default=4096.0,
        help="gamma upper edge, default 4096 channels / about 4 MeV / gamma 能量上限，默认 4096 道约 4 MeV",
    )
    parser.add_argument(
        "--neutron-bins",
        type=int,
        default=10,
        help="default number of neutron gates when -i is not supplied / 未输入 -i 时默认中子能量 gate 数",
    )
    parser.add_argument("--neutron-min", type=float, default=0.0, help="default neutron lower edge in MeV / 默认中子能量下限 MeV")
    parser.add_argument("--neutron-max", type=float, default=50.0, help="default neutron upper edge in MeV / 默认中子能量上限 MeV")
    parser.add_argument(
        "-i",
        "--neutron-edges",
        nargs="+",
        type=float,
        default=None,
        metavar="E",
        help="custom neutron-energy gate edges in MeV, e.g. -i 0 5 10 15 30 / 自定义中子能量 gate 边界 MeV",
    )
    parser.add_argument(
        "--fexo-addback-threshold",
        type=float,
        default=0.0,
        help=(
            "minimum fEXO crystal energy used in computed raw addback; "
            "set to 10 to mimic the framework EnergyAdd threshold / "
            "computed raw addback 使用的 fEXO crystal 能量阈值，可设 10 模拟框架 EnergyAdd 阈值"
        ),
    )
    parser.add_argument(
        "--force-restart",
        action="store_true",
        help="ignore existing partial files and restart processing from entry 0 / 忽略已有 partial，从 entry 0 重新处理",
    )
    parser.add_argument(
        "--merge-only",
        action="store_true",
        help="do not process events; only merge existing per-file partial outputs / 不处理 event，只合并已有 partial 输出",
    )
    return parser.parse_args()


def build_neutron_edges(args: argparse.Namespace) -> tuple[float, ...]:
    if args.neutron_edges is not None:
        edges = tuple(float(edge) for edge in args.neutron_edges)
        if len(edges) < 2:
            raise ValueError("At least two neutron gate edges are required after -i")
        if any(edges[i] >= edges[i + 1] for i in range(len(edges) - 1)):
            raise ValueError("Neutron gate edges must be strictly increasing")
        return edges

    width = (args.neutron_max - args.neutron_min) / args.neutron_bins
    return tuple(args.neutron_min + i * width for i in range(args.neutron_bins + 1))


def make_config(args: argparse.Namespace) -> AnalysisConfig:
    output = Path(args.output)
    partial_dir = Path(args.partial_dir) if args.partial_dir else output.parent / f"{output.stem}_parts"
    neutron_edges = build_neutron_edges(args)
    return AnalysisConfig(
        tree_name=args.tree,
        output=output,
        partial_dir=partial_dir,
        max_events=args.max_events,
        checkpoint_events=args.checkpoint_events,
        progress=args.progress,
        clover_id=args.clover_id,
        gamma_bins=args.gamma_bins,
        gamma_min=args.gamma_min,
        gamma_max=args.gamma_max,
        neutron_bins=args.neutron_bins,
        neutron_min=args.neutron_min,
        neutron_max=args.neutron_max,
        neutron_edges=neutron_edges,
        fexo_addback_threshold=args.fexo_addback_threshold,
        force_restart=args.force_restart,
        merge_only=args.merge_only,
        append_existing_output=False,
    )


def config_signature(cfg: AnalysisConfig) -> str:
    parts = [
        cfg.tree_name,
        str(cfg.clover_id),
        str(cfg.gamma_bins),
        repr(cfg.gamma_min),
        repr(cfg.gamma_max),
        "neutron_edges=" + ",".join(repr(edge) for edge in cfg.neutron_edges),
        repr(cfg.fexo_addback_threshold),
    ]
    return "|".join(parts)


def expand_inputs(patterns: Iterable[str]) -> list[str]:
    files: list[str] = []
    for pattern in patterns:
        matches = sorted(glob.glob(pattern))
        if matches:
            files.extend(matches)
        else:
            files.append(pattern)

    missing = [path for path in files if not Path(path).is_file()]
    if missing:
        raise FileNotFoundError("Missing input file(s): " + ", ".join(missing))
    return files


def safe_name(text: str) -> str:
    return "".join(char if char.isalnum() or char == "_" else "_" for char in text)


def partial_path_for(input_file: str, cfg: AnalysisConfig) -> Path:
    input_path = Path(input_file)
    resolved = str(input_path.resolve(strict=False))
    input_digest = hashlib.sha1(resolved.encode("utf-8")).hexdigest()[:10]
    config_digest = hashlib.sha1(config_signature(cfg).encode("utf-8")).hexdigest()[:10]
    return cfg.partial_dir / f"{safe_name(input_path.stem)}__{input_digest}__cfg{config_digest}.root"


def make_h1(name: str, title: str, bins: int, low: float, high: float) -> ROOT.TH1F:
    hist = ROOT.TH1F(name, title, bins, low, high)
    hist.SetDirectory(0)
    return hist


def make_h1_edges(name: str, title: str, edges: tuple[float, ...]) -> ROOT.TH1F:
    hist = ROOT.TH1F(name, title, len(edges) - 1, array("d", edges))
    hist.SetDirectory(0)
    return hist


def make_h2(name: str, title: str, bins: int, low: float, high: float) -> ROOT.TH2F:
    hist = ROOT.TH2F(name, title, bins, low, high, bins, low, high)
    hist.SetDirectory(0)
    return hist


def vector_values(vector, cast=float) -> list:
    return [cast(vector.at(i)) for i in range(int(vector.size()))]


def clover_from_core_id(core_id: int) -> int:
    return int(core_id) // 4


def compute_addback(clovers: list[int], energies: list[float], threshold: float = 0.0) -> dict[int, float]:
    sums: dict[int, float] = {}
    for clover, energy in zip(clovers, energies):
        if energy <= 0 or energy <= threshold:
            continue
        sums[clover] = sums.get(clover, 0.0) + energy
    return sums


def fill_values(hist: ROOT.TH1, values: Iterable[float]) -> None:
    for value in values:
        hist.Fill(value)


def fill_gamma_values(hist: ROOT.TH1, values: Iterable[float]) -> None:
    for value in values:
        if value <= 0:
            continue
        hist.Fill(value)


def neutron_bin_indices(neutron_energies: Iterable[float], cfg: AnalysisConfig) -> set[int]:
    indices: set[int] = set()
    edges = cfg.neutron_edges
    for energy in neutron_energies:
        if energy < edges[0] or energy >= edges[-1]:
            continue
        for index in range(len(edges) - 1):
            if edges[index] <= energy < edges[index + 1]:
                indices.add(index)
                break
    return indices


def fill_ordered_pairs(matrix: ROOT.TH2, energies: list[float]) -> None:
    positive_energies = [energy for energy in energies if energy > 0]
    if len(positive_energies) < 2:
        return
    for i, first in enumerate(positive_energies):
        for second in positive_energies[i + 1 :]:
            matrix.Fill(first, second)
            matrix.Fill(second, first)


def book_histograms(cfg: AnalysisConfig) -> dict[str, ROOT.TH1]:
    """Create histograms for quick gamma-vs-neutron checks.
    创建 gamma-vs-neutron 快速检查所需的直方图。
    """
    h: dict[str, ROOT.TH1] = {}
    gb, gmin, gmax = cfg.gamma_bins, cfg.gamma_min, cfg.gamma_max

    # Crystal-level spectra: fEXO is raw crystal energy; sEXO is selected single gamma.
    # Crystal 级别能谱：fEXO 是原始 crystal 能量；sEXO 是选择后的 single gamma。
    h["crystal_fEXO_all"] = make_h1("h_crystal_fEXO_all", "raw fEXO crystal energy;energy;counts", gb, gmin, gmax)
    h["crystal_sEXO_all"] = make_h1("h_crystal_sEXO_all", "selected sEXO crystal energy;energy;counts", gb, gmin, gmax)

    # Addback spectra: aEXO is the framework addback branch; no duplicated computed sEXO-addback is kept.
    # Addback 能谱：aEXO 是框架中的 addback branch；这里不再保留重复计算的 sEXO-addback。
    h["addback_fEXO_all"] = make_h1("h_addback_fEXO_all", "computed raw fEXO addback energy;energy;counts", gb, gmin, gmax)
    h["addback_aEXO_all"] = make_h1("h_addback_aEXO_all", "stored aEXO addback energy;energy;counts", gb, gmin, gmax)

    suffix = f"clover{cfg.clover_id}"
    h[f"crystal_fEXO_{suffix}"] = make_h1(f"h_crystal_fEXO_{suffix}", f"raw fEXO crystal energy {suffix};energy;counts", gb, gmin, gmax)
    h[f"crystal_sEXO_{suffix}"] = make_h1(f"h_crystal_sEXO_{suffix}", f"selected sEXO crystal energy {suffix};energy;counts", gb, gmin, gmax)
    h[f"addback_fEXO_{suffix}"] = make_h1(f"h_addback_fEXO_{suffix}", f"computed raw fEXO addback energy {suffix};energy;counts", gb, gmin, gmax)
    h[f"addback_aEXO_{suffix}"] = make_h1(f"h_addback_aEXO_{suffix}", f"stored aEXO addback energy {suffix};energy;counts", gb, gmin, gmax)

    h["neutron_energy"] = make_h1_edges("h_neutron_energy", "neutron energy;E_{n} [MeV];counts", cfg.neutron_edges)

    for index, (low, high) in enumerate(zip(cfg.neutron_edges[:-1], cfg.neutron_edges[1:])):
        label = safe_name(f"nE_{index:02d}_{low:g}_{high:g}MeV")
        title_gate = f"{low:g} <= E_n < {high:g} MeV"
        h[f"addback_aEXO_{label}"] = make_h1(
            f"h_addback_aEXO_{label}",
            f"stored aEXO addback, {title_gate};energy;counts",
            gb,
            gmin,
            gmax,
        )
        h[f"gg_addback_aEXO_{label}"] = make_h2(
            f"h2_gg_addback_aEXO_{label}",
            f"stored aEXO addback coincidence, {title_gate};E1;E2",
            gb,
            gmin,
            gmax,
        )

    return h


def build_chain(files: list[str], tree_name: str) -> ROOT.TChain:
    chain = ROOT.TChain(tree_name)
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


def process_event(chain: ROOT.TChain, hist: dict[str, ROOT.TH1], cfg: AnalysisConfig) -> None:
    target = cfg.clover_id
    suffix = f"clover{target}"

    fexo_clovers = vector_values(chain.fEXO_ECC_E_Clover, int)
    fexo_energies = vector_values(chain.fEXO_ECC_E_Energy, float)

    sexo_energies = vector_values(chain.sEXO_GammaEnergy, float)
    sexo_core_ids = vector_values(chain.sEXO_GammaCoreId, int)
    sexo_clovers = [clover_from_core_id(core_id) for core_id in sexo_core_ids]

    aexo_energies = vector_values(chain.aEXO_GammaEnergy, float)
    aexo_core_ids = vector_values(chain.aEXO_GammaCoreId, int)
    aexo_clovers = [clover_from_core_id(core_id) for core_id in aexo_core_ids]

    neutron_energies = vector_values(chain.fEXO_Neutron_NRJ, float)

    fill_gamma_values(hist["crystal_fEXO_all"], fexo_energies)
    fill_gamma_values(hist["crystal_sEXO_all"], sexo_energies)
    fill_gamma_values(
        hist[f"crystal_fEXO_{suffix}"],
        (energy for clover, energy in zip(fexo_clovers, fexo_energies) if clover == target),
    )
    fill_gamma_values(
        hist[f"crystal_sEXO_{suffix}"],
        (energy for clover, energy in zip(sexo_clovers, sexo_energies) if clover == target),
    )

    fexo_addback = compute_addback(fexo_clovers, fexo_energies, cfg.fexo_addback_threshold)
    fill_gamma_values(hist["addback_fEXO_all"], fexo_addback.values())
    fill_gamma_values(hist["addback_aEXO_all"], aexo_energies)

    if target in fexo_addback:
        hist[f"addback_fEXO_{suffix}"].Fill(fexo_addback[target])
    fill_gamma_values(
        hist[f"addback_aEXO_{suffix}"],
        (energy for clover, energy in zip(aexo_clovers, aexo_energies) if clover == target),
    )

    fill_values(hist["neutron_energy"], neutron_energies)

    for bin_index in neutron_bin_indices(neutron_energies, cfg):
        low = cfg.neutron_edges[bin_index]
        high = cfg.neutron_edges[bin_index + 1]
        label = safe_name(f"nE_{bin_index:02d}_{low:g}_{high:g}MeV")
        fill_gamma_values(hist[f"addback_aEXO_{label}"], aexo_energies)
        fill_ordered_pairs(hist[f"gg_addback_aEXO_{label}"], aexo_energies)


def named_value(root_file: ROOT.TFile, key: str, default: str = "") -> str:
    obj = root_file.Get(key)
    if not obj:
        return default
    return str(obj.GetTitle())


def write_hist_file(
    hist: dict[str, ROOT.TH1],
    output_path: Path,
    input_files: list[str],
    processed_entries: int,
    requested_entries: int,
    total_entries: int,
    complete: bool,
    cfg: AnalysisConfig,
    kind: str,
) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    tmp_path = output_path.with_name(output_path.name + ".tmp")
    out = ROOT.TFile.Open(str(tmp_path), "RECREATE")
    if not out or out.IsZombie():
        raise RuntimeError(f"Cannot create output file: {tmp_path}")

    ROOT.TNamed("kind", kind).Write()
    ROOT.TNamed("input_files", "\n".join(input_files)).Write()
    ROOT.TNamed("processed_entries", str(processed_entries)).Write()
    ROOT.TNamed("requested_entries", str(requested_entries)).Write()
    ROOT.TNamed("total_entries", str(total_entries)).Write()
    ROOT.TNamed("complete", "1" if complete else "0").Write()
    ROOT.TNamed("config_signature", config_signature(cfg)).Write()
    ROOT.TNamed("script_note", "Fast gamma-spectrum check versus neutron-energy gates").Write()
    ROOT.TNamed("neutron_edges", " ".join(str(edge) for edge in cfg.neutron_edges)).Write()

    for obj in hist.values():
        obj.Write()

    out.Close()
    os.replace(tmp_path, output_path)


def load_partial_histograms(hist: dict[str, ROOT.TH1], path: Path, cfg: AnalysisConfig) -> tuple[int, bool]:
    if not path.exists() or cfg.force_restart:
        return 0, False

    root_file = ROOT.TFile.Open(str(path), "READ")
    if not root_file or root_file.IsZombie():
        raise RuntimeError(f"Existing partial file is not readable: {path}")

    signature = named_value(root_file, "config_signature")
    if signature and signature != config_signature(cfg):
        root_file.Close()
        raise RuntimeError(
            f"Partial file config does not match current options: {path}. "
            "Use --force-restart or choose a different --partial-dir."
        )

    processed_entries = int(named_value(root_file, "processed_entries", "0"))
    complete = named_value(root_file, "complete", "0") == "1"

    for obj in hist.values():
        existing = root_file.Get(obj.GetName())
        if existing:
            obj.Add(existing)

    root_file.Close()
    return processed_entries, complete


def process_one_file(input_file: str, cfg: AnalysisConfig) -> Path:
    part_path = partial_path_for(input_file, cfg)
    hist = book_histograms(cfg)
    chain = build_chain([input_file], cfg.tree_name)
    total_entries = int(chain.GetEntries())
    requested_entries = total_entries if cfg.max_events < 0 else min(total_entries, cfg.max_events)

    processed_entries, complete = load_partial_histograms(hist, part_path, cfg)
    if processed_entries > requested_entries:
        processed_entries = requested_entries
    if complete and processed_entries >= requested_entries:
        print(f"Skip complete partial: {part_path} ({processed_entries}/{requested_entries})")
        return part_path

    if cfg.force_restart:
        write_hist_file(hist, part_path, [input_file], 0, requested_entries, total_entries, False, cfg, "partial")

    print(f"Input: {input_file}")
    print(f"Partial output: {part_path}")
    print(f"Total entries: {total_entries}")
    print(f"Entries to process: {requested_entries}")
    print(f"Starting entry: {processed_entries}")

    for entry in range(processed_entries, requested_entries):
        chain.GetEntry(entry)
        process_event(chain, hist, cfg)
        done = entry + 1

        if cfg.progress > 0 and done % cfg.progress == 0:
            print(f"Processed {done}/{requested_entries} in {Path(input_file).name}")

        if cfg.checkpoint_events > 0 and done % cfg.checkpoint_events == 0:
            write_hist_file(hist, part_path, [input_file], done, requested_entries, total_entries, False, cfg, "partial")
            print(f"Checkpoint written: {part_path} ({done}/{requested_entries})")

    write_hist_file(hist, part_path, [input_file], requested_entries, requested_entries, total_entries, True, cfg, "partial")
    print(f"Completed partial: {part_path}")
    return part_path


def axes_match(first, second) -> bool:
    return (
        first.GetNbins() == second.GetNbins()
        and abs(first.GetXmin() - second.GetXmin()) < 1e-12
        and abs(first.GetXmax() - second.GetXmax()) < 1e-12
    )


def histograms_compatible(first: ROOT.TH1, second: ROOT.TH1) -> bool:
    if first.GetDimension() != second.GetDimension():
        return False
    if not axes_match(first.GetXaxis(), second.GetXaxis()):
        return False
    if first.GetDimension() >= 2 and not axes_match(first.GetYaxis(), second.GetYaxis()):
        return False
    return True


def ordered_unique(items: Iterable[str]) -> list[str]:
    seen: set[str] = set()
    result: list[str] = []
    for item in items:
        if not item or item in seen:
            continue
        seen.add(item)
        result.append(item)
    return result


def next_new_output_path(output_path: Path) -> Path:
    for index in range(1, 1000):
        suffix = "_new" if index == 1 else f"_new{index}"
        candidate = output_path.with_name(f"{output_path.stem}{suffix}{output_path.suffix}")
        if not candidate.exists():
            return candidate
    raise RuntimeError(f"Could not find a free fallback output path near {output_path}")


def confirm_append_existing_output(output_path: Path) -> bool:
    print(f"Output file already exists: {output_path}")
    print("Choose y to append the new merged histograms to the existing histograms.")
    print("Choose n to stop now without processing or modifying the existing output file.")
    print("Warning: choosing y with data already included in that output will double count it.")
    while True:
        try:
            answer = input("Append current results to existing output? [y/n]: ").strip().lower()
        except EOFError as exc:
            raise RuntimeError("Confirmation required, but stdin is not available.") from exc
        if answer in {"y", "yes"}:
            return True
        if answer in {"n", "no"}:
            return False
        print("Please type y or n.")


def load_existing_output_for_append(
    hist: dict[str, ROOT.TH1], cfg: AnalysisConfig
) -> tuple[int, int, int, list[str], Path]:
    if not cfg.output.exists() or not cfg.append_existing_output:
        return 0, 0, 0, [], cfg.output

    fallback_output = next_new_output_path(cfg.output)
    root_file = ROOT.TFile.Open(str(cfg.output), "READ")
    if not root_file or root_file.IsZombie():
        print(f"Existing output is not readable: {cfg.output}")
        print(f"Current results will be written to a new file instead: {fallback_output}")
        return 0, 0, 0, [], fallback_output

    signature = named_value(root_file, "config_signature")
    if signature and signature != config_signature(cfg):
        root_file.Close()
        print(f"Existing output config does not match current options: {cfg.output}")
        print(f"Current results will be written to a new file instead: {fallback_output}")
        return 0, 0, 0, [], fallback_output

    existing_pairs = []
    for obj in hist.values():
        existing = root_file.Get(obj.GetName())
        if not existing:
            continue
        if not histograms_compatible(obj, existing):
            root_file.Close()
            print(f"Existing histogram has incompatible binning: {obj.GetName()} in {cfg.output}")
            print(f"Current results will be written to a new file instead: {fallback_output}")
            return 0, 0, 0, [], fallback_output
        existing_pairs.append((obj, existing))

    for obj, existing in existing_pairs:
        obj.Add(existing)

    old_processed = int(named_value(root_file, "processed_entries", "0"))
    old_requested = int(named_value(root_file, "requested_entries", "0"))
    old_total = int(named_value(root_file, "total_entries", "0"))
    old_inputs = named_value(root_file, "input_files", "").splitlines()
    root_file.Close()
    return old_processed, old_requested, old_total, old_inputs, cfg.output


def merge_partials(part_paths: list[Path], cfg: AnalysisConfig, input_files: list[str]) -> None:
    merged = book_histograms(cfg)
    processed_total = 0
    requested_total = 0
    entries_total = 0
    all_complete = True

    for part_path in part_paths:
        if not part_path.exists():
            raise FileNotFoundError(f"Missing partial file for merge: {part_path}")
        root_file = ROOT.TFile.Open(str(part_path), "READ")
        if not root_file or root_file.IsZombie():
            raise RuntimeError(f"Cannot read partial file for merge: {part_path}")

        signature = named_value(root_file, "config_signature")
        if signature and signature != config_signature(cfg):
            root_file.Close()
            raise RuntimeError(f"Partial file config does not match current options: {part_path}")

        processed_total += int(named_value(root_file, "processed_entries", "0"))
        requested_total += int(named_value(root_file, "requested_entries", "0"))
        entries_total += int(named_value(root_file, "total_entries", "0"))
        all_complete = all_complete and named_value(root_file, "complete", "0") == "1"

        for obj in merged.values():
            existing = root_file.Get(obj.GetName())
            if existing:
                obj.Add(existing)

        root_file.Close()

    old_processed, old_requested, old_entries, old_inputs, final_output = load_existing_output_for_append(merged, cfg)
    processed_total += old_processed
    requested_total += old_requested
    entries_total += old_entries
    merged_input_files = ordered_unique(old_inputs + input_files)

    write_hist_file(
        merged,
        final_output,
        merged_input_files,
        processed_total,
        requested_total,
        entries_total,
        all_complete,
        cfg,
        "merged",
    )
    print(f"Merged output written: {final_output}")


def main() -> int:
    args = parse_args()
    cfg = make_config(args)
    files = expand_inputs(args.inputs)

    if cfg.output.exists():
        append_existing = confirm_append_existing_output(cfg.output)
        if not append_existing:
            raise SystemExit("Stopped before processing. Existing output file was not modified.")
        cfg = replace(cfg, append_existing_output=True)

    ROOT.gROOT.SetBatch(True)

    part_paths = [partial_path_for(path, cfg) for path in files]
    print(f"Input files: {len(files)}")
    print(f"Tree: {cfg.tree_name}")
    print(f"Partial dir: {cfg.partial_dir}")
    print(f"Checkpoint entries: {cfg.checkpoint_events}")
    print(f"Neutron edges: {' '.join(str(edge) for edge in cfg.neutron_edges)}")
    print(f"Merged output: {cfg.output}")

    if not cfg.merge_only:
        part_paths = []
        for input_file in files:
            part_paths.append(process_one_file(input_file, cfg))

    merge_partials(part_paths, cfg, files)
    print("Done")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
