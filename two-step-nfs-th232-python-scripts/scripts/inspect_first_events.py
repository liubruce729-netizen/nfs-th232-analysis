#!/usr/bin/env python3
"""
Very small first-look analysis for the Exogam2 event tree.

It reads the first N events from TreeMaster, writes the event content to a text
file, and stores simple histograms for each split branch in an output ROOT file.
"""

from __future__ import annotations

import argparse
import math
from pathlib import Path

import ROOT


DEFAULT_INPUT = (
    "/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/out/run_100_r0.root"
)
DEFAULT_OUT_DIR = "/home/user0/work/IJCLAB/NFS/data_ana"

SKIP_BRANCHES = {"Exogam2", "TObject", "fUniqueID", "fBits"}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Dump the first events of TreeMaster and histogram branch values."
    )
    parser.add_argument("input", nargs="?", default=DEFAULT_INPUT, help="input ROOT file")
    parser.add_argument("-n", "--events", type=int, default=100, help="events to read")
    parser.add_argument("--tree", default="TreeMaster", help="tree name")
    parser.add_argument(
        "--text-out",
        default=str(Path(DEFAULT_OUT_DIR) / "run_100_r0_first100_events.txt"),
        help="text dump output path",
    )
    parser.add_argument(
        "--hist-out",
        default=str(Path(DEFAULT_OUT_DIR) / "run_100_r0_first100_hists.root"),
        help="histogram ROOT output path",
    )
    return parser.parse_args()


def walk_branches(branches):
    """Return terminal branch names from a possibly split TTree branch tree."""
    names = []
    for branch in branches:
        name = branch.GetName()
        children = branch.GetListOfBranches()
        if children and children.GetEntries() > 0:
            names.extend(walk_branches(children))
        elif name not in SKIP_BRANCHES:
            names.append(name)
    return names


def attr_name(branch_name: str) -> str:
    return branch_name.split("[", 1)[0]


def clean_name(name: str) -> str:
    cleaned = []
    for char in name:
        cleaned.append(char if char.isalnum() or char == "_" else "_")
    return "".join(cleaned)


def to_values(obj):
    """Convert ROOT vectors/arrays/scalars to plain Python values."""
    if hasattr(obj, "size") and hasattr(obj, "at"):
        return [obj.at(i) for i in range(int(obj.size()))]

    try:
        return [obj[i] for i in range(len(obj))]
    except TypeError:
        return [obj]


def branch_values(tree, branch_name: str):
    """Read a branch from the current entry.

    Split std::vector branches can be accessed as tree attributes.  Fixed C
    arrays, such as fTimeStampsExogam2[100], must be read through their TLeaf:
    the PyROOT attribute exposes a LowLevelView that does not give reliable
    values without an explicit address.
    """
    base_name = attr_name(branch_name)
    if "[" in branch_name:
        leaf = tree.GetLeaf(base_name)
        if not leaf:
            return []
        type_name = leaf.GetTypeName()
        integer_types = {
            "Bool_t",
            "Char_t",
            "UChar_t",
            "Short_t",
            "UShort_t",
            "Int_t",
            "UInt_t",
            "Long64_t",
            "ULong64_t",
        }
        values = [leaf.GetValue(i) for i in range(int(leaf.GetLen()))]
        if type_name in integer_types:
            return [int(value) for value in values]
        return values

    return to_values(getattr(tree, base_name))


def finite_values(values):
    out = []
    for value in values:
        try:
            number = float(value)
        except (TypeError, ValueError):
            continue
        if math.isfinite(number):
            out.append(number)
    return out


def make_hist(name: str, title: str, values):
    vals = finite_values(values)
    if not vals:
        return None

    lo = min(vals)
    hi = max(vals)
    if lo == hi:
        width = 1.0 if lo == 0 else abs(lo) * 0.05
        lo -= width
        hi += width

    hist = ROOT.TH1F(clean_name(name), title, 100, lo, hi)
    for value in vals:
        hist.Fill(value)
    return hist


def main() -> int:
    args = parse_args()
    ROOT.gROOT.SetBatch(True)

    input_path = Path(args.input)
    text_path = Path(args.text_out)
    hist_path = Path(args.hist_out)
    text_path.parent.mkdir(parents=True, exist_ok=True)
    hist_path.parent.mkdir(parents=True, exist_ok=True)

    root_file = ROOT.TFile.Open(str(input_path), "READ")
    if not root_file or root_file.IsZombie():
        raise RuntimeError(f"Cannot open input file: {input_path}")

    tree = root_file.Get(args.tree)
    if not tree:
        raise RuntimeError(f"Cannot find tree '{args.tree}' in {input_path}")

    branch_names = walk_branches(tree.GetListOfBranches())
    branch_names = [name for name in branch_names if name not in SKIP_BRANCHES]

    tree.SetBranchStatus("*", 0)
    for name in branch_names:
        tree.SetBranchStatus(name, 1)
        base_name = attr_name(name)
        if base_name != name and tree.GetBranch(base_name):
            tree.SetBranchStatus(base_name, 1)

    entries_to_read = min(args.events, int(tree.GetEntries()))
    all_values = {name: [] for name in branch_names}
    all_mults = {name: [] for name in branch_names}

    with text_path.open("w", encoding="utf-8") as text_out:
        text_out.write(f"input: {input_path}\n")
        text_out.write(f"tree: {args.tree}\n")
        text_out.write(f"total_entries: {int(tree.GetEntries())}\n")
        text_out.write(f"events_dumped: {entries_to_read}\n")
        text_out.write(f"branches: {', '.join(branch_names)}\n\n")

        for event_index in range(entries_to_read):
            tree.GetEntry(event_index)
            text_out.write(f"event {event_index}\n")

            for name in branch_names:
                values = branch_values(tree, name)
                all_values[name].extend(values)
                all_mults[name].append(len(values))
                text_out.write(f"  {name}: n={len(values)} values={values}\n")

            text_out.write("\n")

    out_file = ROOT.TFile.Open(str(hist_path), "RECREATE")
    summary = ROOT.TNamed(
        "summary",
        f"First {entries_to_read} events from {input_path.name}; tree={args.tree}",
    )
    summary.Write()

    for name in branch_names:
        value_hist = make_hist(f"h_{name}", f"{name};value;counts", all_values[name])
        if value_hist:
            value_hist.Write()

        mult_hist = ROOT.TH1F(
            clean_name(f"h_mult_{name}"),
            f"{name} multiplicity per event;multiplicity;events",
            max(1, max(all_mults[name], default=0) + 1),
            -0.5,
            max(0.5, max(all_mults[name], default=0) + 0.5),
        )
        for mult in all_mults[name]:
            mult_hist.Fill(mult)
        mult_hist.Write()

    out_file.Close()
    root_file.Close()

    print(f"Wrote text dump: {text_path}")
    print(f"Wrote histograms: {hist_path}")
    print(f"Read events: {entries_to_read}")
    print(f"Branches: {len(branch_names)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
