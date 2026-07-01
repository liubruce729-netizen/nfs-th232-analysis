#!/usr/bin/env python3
"""把 ROOT 文件里的一个一维 TH1 直方图转成 RadWare GF3 可读的 .spe。

依赖:
  - PyROOT

常用:
  python3 root_hist_to_spe.py -i Eu152.root --list
  python3 root_hist_to_spe.py -i Eu152.root -n hEu152 -o Eu152.spe --spe-name Eu152

只写 normal bins，不写 ROOT 的 underflow/overflow。
输出是 little-endian 的 RadWare spectrum 格式，用于 GF3 的 SP 命令。
"""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

import ROOT


def _spe_name_bytes(name: str) -> bytes:
    # RadWare 谱名固定 8 字节；太长截断，不足补空格。
    return name.encode("ascii", "replace")[:8].ljust(8, b" ")


def list_histograms(input_file: str) -> None:
    f_in = ROOT.TFile.Open(input_file, "READ")
    if not f_in or f_in.IsZombie():
        raise RuntimeError(f"Cannot open ROOT file: {input_file}")

    def walk(directory, prefix=""):
        for key in directory.GetListOfKeys():
            obj = key.ReadObj()
            name = f"{prefix}{key.GetName()}"
            if obj.InheritsFrom("TH1") and not obj.InheritsFrom("TH2"):
                print(name)
            elif obj.InheritsFrom("TDirectory"):
                walk(obj, name + "/")

    walk(f_in)
    f_in.Close()


def write_spe(input_file: str, hist_name: str, output_file: str, spe_name: str | None = None) -> None:
    f_in = ROOT.TFile.Open(input_file, "READ")
    if not f_in or f_in.IsZombie():
        raise RuntimeError(f"Cannot open ROOT file: {input_file}")

    hist = f_in.Get(hist_name)
    if not hist:
        f_in.Close()
        raise ValueError(f"Histogram '{hist_name}' not found in {input_file}")
    if not hist.InheritsFrom("TH1") or hist.InheritsFrom("TH2"):
        f_in.Close()
        raise TypeError(f"Object '{hist_name}' is not a one-dimensional TH1")

    nbins = hist.GetNbinsX()
    values = [float(hist.GetBinContent(i)) for i in range(1, nbins + 1)]
    name = spe_name or Path(output_file).stem

    # RadWare .spe 是 Fortran unformatted record: header record + data record。
    header = struct.pack("<8siiii", _spe_name_bytes(name), nbins, 1, 1, 1)
    data = struct.pack("<" + "f" * nbins, *values)

    with open(output_file, "wb") as out:
        out.write(struct.pack("<i", len(header)))
        out.write(header)
        out.write(struct.pack("<i", len(header)))
        out.write(struct.pack("<i", len(data)))
        out.write(data)
        out.write(struct.pack("<i", len(data)))

    f_in.Close()
    print(f"Wrote {output_file} from {input_file}:{hist_name} ({nbins} channels)")


def main() -> None:
    parser = argparse.ArgumentParser(description="Convert one ROOT TH1 to RadWare .spe")
    parser.add_argument("-i", "--input", required=True, help="input ROOT file")
    parser.add_argument("-n", "--hist", help="histogram name/path inside the ROOT file")
    parser.add_argument("-o", "--output", help="output .spe file")
    parser.add_argument("--spe-name", help="8-character RadWare internal spectrum name")
    parser.add_argument("--list", action="store_true", help="list one-dimensional histograms and exit")
    args = parser.parse_args()

    if args.list:
        list_histograms(args.input)
        return

    if not args.hist or not args.output:
        parser.error("--hist and --output are required unless --list is used")

    write_spe(args.input, args.hist, args.output, args.spe_name)


if __name__ == "__main__":
    main()
