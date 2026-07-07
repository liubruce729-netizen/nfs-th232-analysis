#!/usr/bin/env python3
"""把 RadWare GF3 的 .sto 和标准源 .sou 合并成 EFFIT 可读的 .sin。

依赖:
  - Python 3 标准库

常用:
  python3 merge_sto_sou_to_sin.py --sto gf3.sto --sou Eu152.sou --out Eu152.sin \
    --title "Manual GF3 fit, gf3.sto + Eu152.sou"

合并规则按 RadWare SOURCE 程序：
  .sto: no, centroid, dcentroid, area, darea, ...
  .sou: source_energy, dsource_energy, intensity, dintensity
  .sin: no centroid dcentroid area darea source_energy dsource_energy intensity dintensity

.sto 和 .sou 在跳过 .sto 表头后逐行配对。若 .sto 中有空行，会消耗对应
.sou 行但不输出 .sin 行，用来表示这个标准源峰没有采用。
"""

from __future__ import annotations

import argparse
from pathlib import Path


def sto_data_lines(path: Path):
    for raw in path.read_text().splitlines():
        line = raw.strip()
        if not line:
            yield None
            continue
        if "Centroid" in raw or raw.lstrip().startswith("No."):
            continue
        yield raw


def sou_data_lines(path: Path):
    for raw in path.read_text().splitlines():
        line = raw.strip()
        if not line:
            yield None
        else:
            yield raw


def parse_sto_line(raw: str):
    fields = raw.split()
    if len(fields) < 5:
        raise ValueError(f"Bad .sto line, need at least 5 fields: {raw!r}")
    return int(fields[0]), float(fields[1]), float(fields[2]), float(fields[3]), float(fields[4])


def parse_sou_line(raw: str):
    fields = raw.split()
    if len(fields) < 4:
        raise ValueError(f"Bad .sou line, need at least 4 fields: {raw!r}")
    return float(fields[0]), float(fields[1]), float(fields[2]), float(fields[3])


def merge(sto_path: Path, sou_path: Path, out_path: Path, title: str) -> int:
    rows = []
    sto_iter = list(sto_data_lines(sto_path))
    sou_iter = list(sou_data_lines(sou_path))

    if len(sto_iter) != len(sou_iter):
        raise ValueError(
            f"Line count mismatch after headers/blanks: {sto_path.name} has {len(sto_iter)} rows, "
            f"{sou_path.name} has {len(sou_iter)} rows. Edit the files so rows match one-to-one."
        )

    for sto_raw, sou_raw in zip(sto_iter, sou_iter):
        if sou_raw is None:
            continue
        if sto_raw is None:
            continue

        no, centroid, dcentroid, area, darea = parse_sto_line(sto_raw)
        source_energy, dsource_energy, intensity, dintensity = parse_sou_line(sou_raw)

        # RadWare SOURCE 会忽略 centroid 和 area 同时为 0 的行。
        if centroid == 0.0 and area == 0.0:
            continue

        rows.append(
            (no, centroid, dcentroid, area, darea, source_energy, dsource_energy, intensity, dintensity)
        )

    with out_path.open("w") as out:
        out.write(f" {title[:79]:<79}\n")
        for row in rows:
            no, centroid, dcentroid, area, darea, source_energy, dsource_energy, intensity, dintensity = row
            out.write(
                f"{no:4d} {centroid:11.4f} {dcentroid:11.4f} "
                f"{area:11.0f} {darea:11.0f} "
                f"{source_energy:11.4f} {dsource_energy:11.4f} "
                f"{intensity:11.0f} {dintensity:11.0f}\n"
            )
    return len(rows)


def main() -> None:
    parser = argparse.ArgumentParser(description="Merge RadWare gf3 .sto and .sou files into .sin")
    parser.add_argument("--sto", required=True, type=Path, help="input gf3 .sto file")
    parser.add_argument("--sou", required=True, type=Path, help="input source .sou file")
    parser.add_argument("--out", required=True, type=Path, help="output .sin file")
    parser.add_argument("--title", default="Manual GF3 STO + source SOU", help="title line written to .sin")
    args = parser.parse_args()

    n = merge(args.sto, args.sou, args.out, args.title)
    print(f"Wrote {n} rows to {args.out}")


if __name__ == "__main__":
    main()
