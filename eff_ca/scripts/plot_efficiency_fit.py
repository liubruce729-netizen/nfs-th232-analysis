#!/usr/bin/env python3
"""画 RadWare EFFIT 的效率点和拟合曲线。

依赖:
  - numpy
  - matplotlib

常用:
  python3 plot_efficiency_fit.py --sin Eu152.sin --aef Eu152_fit.aef --out Eu152_efficiency_fit.png
  python3 plot_efficiency_fit.py --sin Eu152.sin --aef Eu152_fit.aef Eu152_fit_Gfree.aef

.sin 里的点来自 GF3 CA/AUTOCAL 的峰面积和 .sou 的源强度。
.aef 里的 10 个数按 RadWare EFFIT 的 A B C D E F G E1 E2 deff_log 读取。
"""

from __future__ import annotations

import argparse
import math
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


def read_sin(path: Path):
    points = []
    for line in path.read_text().splitlines()[1:]:
        fields = line.split()
        if len(fields) < 9:
            continue
        area = float(fields[3])
        area_err = float(fields[4])
        energy = float(fields[5])
        intensity = float(fields[7])
        intensity_err = float(fields[8])
        # EFFIT 拟合的是相对效率；这里按 area / source intensity 画检查点。
        eff = area / intensity
        rel2 = 0.0
        if area > 0:
            rel2 += (area_err / area) ** 2
        if intensity > 0:
            rel2 += (intensity_err / intensity) ** 2
        points.append((energy, eff, eff * math.sqrt(rel2)))
    return np.asarray(points, dtype=float)


def read_aef(path: Path):
    values = []
    for line in path.read_text().splitlines()[1:]:
        values.extend(float(x) for x in line.split())
    if len(values) < 10:
        raise ValueError(f"{path} does not contain the 10 expected AEF values")
    return values[:10]


def effic(energy, pars):
    a, b, c, d, e, f, g, e1, e2, _deff = pars
    x = np.log(energy / e1)
    y = np.log(energy / e2)
    low = a + b * x + c * x * x
    high = d + e * y + f * y * y
    # RadWare EFFIT 使用的低能段/高能段平滑拼接公式。
    return np.exp(((low ** (-g)) + (high ** (-g))) ** (-1.0 / g))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--sin", default="Eu152.sin")
    parser.add_argument("--aef", nargs="+", default=["Eu152_fit.aef", "Eu152_fit_Gfree.aef"])
    parser.add_argument("--out", default="Eu152_efficiency_fit.png")
    args = parser.parse_args()

    sin_path = Path(args.sin)
    points = read_sin(sin_path)
    energies = np.geomspace(points[:, 0].min() * 0.9, points[:, 0].max() * 1.1, 600)

    fig, ax = plt.subplots(figsize=(8.2, 5.2))
    ax.errorbar(points[:, 0], points[:, 1], yerr=points[:, 2], fmt="o", ms=4, capsize=2, label=sin_path.name)

    for aef_name in args.aef:
        aef_path = Path(aef_name)
        if not aef_path.exists():
            continue
        pars = read_aef(aef_path)
        ax.plot(energies, effic(energies, pars), lw=1.8, label=aef_path.name)

    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("Energy (keV)")
    ax.set_ylabel("Relative efficiency (area / source intensity)")
    ax.grid(True, which="both", alpha=0.25)
    ax.legend()
    fig.tight_layout()
    fig.savefig(args.out, dpi=180)


if __name__ == "__main__":
    main()
