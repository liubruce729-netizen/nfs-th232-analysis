#!/usr/bin/env python3
# 中文说明：SNIP/TSpectrum 背景扣除工具，用于生成 SY_*.root。
# -*- coding: utf-8 -*-
"""
bg_subtraction_xyzdiag.py

读取一个 ROOT 文件中的二维频次直方图（TH2），
使用“三方向单向插值再合并”的背景估计算法，
并输出背景图和信号-背景图到指定输出 ROOT 文件。

方向包括：
1. X 方向：      (i-k, j)   与 (i+k, j)
2. Y 方向：      (i, j-k)   与 (i, j+k)
3. 反对角线方向： (i-k, j+k) 与 (i+k, j-k)   [即 x+y=常数, 斜率 -1]

用法(全局最大窗口 m)：
  python bg_subtraction_xyzdiag.py \
    -i input.root -n h2d_hist -o out.root \
    -r fourth -m 5 -s increasing

用法(分段最大窗口；像素(i,j)均 <= M_edge 用 m1，否则用 m2)：
  python bg_subtraction_xyzdiag.py \
    -i input.root -n h2d_hist -o out.root \
    -r fourth -s increasing \
    --M_edge 30 --m1 3 --m2 7

参数:
  -i, --input       输入 ROOT 文件路径
  -n, --histname    要处理的 TH2 名称
  -o, --output      输出 ROOT 文件路径
  -r, --order       保留参数：'second' 或 'fourth'（当前不生效，仅兼容旧接口）
  -m, --window      全局最大窗口 m（默认 5；当未提供 --M_edge/--m1/--m2 时生效）
  -s, --strategy    保留参数：'increasing' 或 'decreasing'（当前不生效，仅兼容旧接口）
  --M_edge          分段阈值（像素索引，i 和 j 均 <= M_edge 视为“边缘区”）
  --m1              边缘区使用的最大窗口（i,j ≤ M_edge）
  --m2              非边缘区使用的最大窗口（其他像素）

算法说明：
对每个点 (i,j)，给定最大窗口 m_local：

1. 遍历 p = 1, 2, ..., m_local
2. 对每个方向，在该 p 下构造 k = 1..p 的插值候选（两端平均）
3. 该方向在该 p 下取“最大值”
4. 再对该方向所有 p 的结果取“最小值”
5. 三个方向 X、Y、D 再取“最小值”
6. 最终背景值为：
       bg(i,j) = min(v0(i,j), interp)

即：
- X = min_p max_k X_k(p)
- Y = min_p max_k Y_k(p)
- D = min_p max_k D_k(p)
- interp = min(X, Y, D)
- bg = min(v0(i,j), interp)

边界处理（第二种）：
- 若某个 p 在某个方向越界，则该方向该 p 跳过
- 若某方向所有 p 都不可用，则该方向不参与最终三方向合并
- 若三个方向都没有任何可用窗口，则返回原值

注意：
- 本算法始终只基于原始矩阵 v0 计算，不是迭代过程
- 参数 order / strategy 当前不参与计算，仅保留接口兼容
"""

import argparse
import warnings
import numpy as np
import ROOT


def root_hist_to_array(hist):
    """将 ROOT TH2 直方图转为 NumPy 数组（按 bin 索引顺序拷贝内容）"""
    nx = hist.GetNbinsX()
    ny = hist.GetNbinsY()
    arr = np.zeros((nx, ny), dtype=float)
    for i in range(1, nx + 1):
        for j in range(1, ny + 1):
            arr[i - 1, j - 1] = hist.GetBinContent(i, j)
    return arr


def array_to_root_hist(arr, ref_hist, name):
    """根据参考直方图的轴设置，创建一个新的 TH2D 并填充数据"""
    nx, ny = arr.shape
    xaxis = ref_hist.GetXaxis()
    yaxis = ref_hist.GetYaxis()

    edges_x = [xaxis.GetBinLowEdge(i) for i in range(1, nx + 2)]
    edges_y = [yaxis.GetBinLowEdge(j) for j in range(1, ny + 2)]

    hist = ROOT.TH2D(
        name, name,
        nx, np.array(edges_x, dtype='d'),
        ny, np.array(edges_y, dtype='d')
    )

    for i in range(nx):
        for j in range(ny):
            hist.SetBinContent(i + 1, j + 1, float(arr[i, j]))

    return hist


def _interp_value_xyzdiag(v0: np.ndarray, i: int, j: int, m: int) -> float:
    """
    对点 (i,j) 在最大窗口 m 下做三方向插值背景估计。

    正确逻辑：
    1. 对每个方向（X, Y, D），收集该方向在 p=1..m 下、k=1..p 的所有有效插值候选
    2. 该方向所有候选取最小值，得到 a_x / a_y / a_d
    3. 三个方向结果取最大值，得到 b
    4. 最终背景值为 min(v0[i,j], b)

    边界处理（第二种）：
    - 某个 p 或某个 k 越界，则该候选跳过
    - 若某个方向没有任何有效候选，则该方向不参与最终 max
    - 若三个方向都没有任何有效候选，则返回原值
    """
    h, w = v0.shape

    x_candidates_all = []
    y_candidates_all = []
    d_candidates_all = []

    for p in range(1, m + 1):
        # X 方向: (i-k, j), (i+k, j)
        for k in range(1, p + 1):
            if i - k >= 0 and i + k < h:
                val = 0.5 * (v0[i - k, j] + v0[i + k, j])
                x_candidates_all.append(val)

        # Y 方向: (i, j-k), (i, j+k)
        for k in range(1, p + 1):
            if j - k >= 0 and j + k < w:
                val = 0.5 * (v0[i, j - k] + v0[i, j + k])
                y_candidates_all.append(val)

        # 反对角线方向: (i-k, j+k), (i+k, j-k)
        for k in range(1, p + 1):
            if (i - k >= 0) and (i + k < h) and (j + k < w) and (j - k >= 0):
                val = 0.5 * (v0[i - k, j + k] + v0[i + k, j - k])
                d_candidates_all.append(val)

    available_dirs = []
    if x_candidates_all:
        available_dirs.append(min(x_candidates_all))
    if y_candidates_all:
        available_dirs.append(min(y_candidates_all))
    if d_candidates_all:
        available_dirs.append(min(d_candidates_all))

    if not available_dirs:
        return float(v0[i, j])

    b = max(available_dirs)
    return float(min(v0[i, j], b))


def clipping_filter(v0: np.ndarray, m: int, increasing: bool) -> np.ndarray:
    """
    全局最大窗口模式。
    increasing 参数保留但当前不生效，仅为接口兼容。
    """
    h, w = v0.shape
    bg = np.zeros_like(v0, dtype=float)

    for i in range(h):
        for j in range(w):
            bg[i, j] = _interp_value_xyzdiag(v0, i, j, m)

    return bg


def clipping_filter_variable(v0: np.ndarray,
                             m1: int, m2: int, M_edge: int,
                             increasing: bool) -> np.ndarray:
    """
    分段最大窗口模式：
    m_local(i,j) = m1 if (i+1) <= M_edge and (j+1) <= M_edge else m2

    increasing 参数保留但当前不生效，仅为接口兼容。
    """
    h, w = v0.shape
    bg = np.zeros_like(v0, dtype=float)

    for i in range(h):
        for j in range(w):
            m_local = m1 if ((i + 1) <= M_edge and (j + 1) <= M_edge) else m2
            bg[i, j] = _interp_value_xyzdiag(v0, i, j, m_local)

    return bg


def main():
    parser = argparse.ArgumentParser(description="二维直方图背景估计与扣背景（三方向单向插值再合并）")
    parser.add_argument('-i', '--input', required=True, help='输入 ROOT 文件')
    parser.add_argument('-n', '--histname', required=True, help='TH2 直方图名')
    parser.add_argument('-o', '--output', required=True, help='输出 ROOT 文件')
    parser.add_argument('-r', '--order', choices=['second', 'fourth'], default='second',
                        help='保留参数：滤波阶数（当前不生效，仅兼容旧接口）')
    parser.add_argument('-m', '--window', type=int, default=5,
                        help='全局最大窗口 m（默认 5）')
    parser.add_argument('-s', '--strategy', choices=['increasing', 'decreasing'], default='increasing',
                        help='保留参数：窗口序列（当前不生效，仅兼容旧接口）')

    parser.add_argument('--M_edge', type=int, default=None,
                        help='分段阈值（像素索引，i,j <= M_edge 视为边缘区）')
    parser.add_argument('--m1', type=int, default=None,
                        help='边缘区最大窗口')
    parser.add_argument('--m2', type=int, default=None,
                        help='非边缘区最大窗口')

    args = parser.parse_args()

    warnings.warn("参数 --order 当前保留但不生效；本版本统一使用三方向单向插值再合并算法。")
    warnings.warn("参数 --strategy 当前保留但不生效；本版本不做 increasing/decreasing 迭代。")

    segmented = (args.M_edge is not None) and (args.m1 is not None) and (args.m2 is not None)

    if segmented:
        if args.m1 <= 0 or args.m2 <= 0:
            raise ValueError("分段模式下 m1、m2 必须为正整数。")
        if args.M_edge <= 0:
            raise ValueError("M_edge 必须为正整数（以像素索引计）。")
    else:
        if args.window <= 0:
            raise ValueError("全局窗口 m 必须为正整数。")

    fin = ROOT.TFile.Open(args.input)
    if not fin or fin.IsZombie():
        raise RuntimeError(f"无法打开输入文件: {args.input}")

    h2 = fin.Get(args.histname)
    if not h2:
        raise RuntimeError(f"在文件中找不到直方图: {args.histname}")
    if not isinstance(h2, ROOT.TH2):
        raise RuntimeError(f"{args.histname} 不是 TH2 类型")

    arr = root_hist_to_array(h2)

    if segmented:
        bg = clipping_filter_variable(
            arr,
            m1=args.m1,
            m2=args.m2,
            M_edge=args.M_edge,
            increasing=(args.strategy == 'increasing')
        )
    else:
        bg = clipping_filter(
            arr,
            m=args.window,
            increasing=(args.strategy == 'increasing')
        )

    sig = arr - bg

    fout = ROOT.TFile.Open(args.output, 'RECREATE')
    if not fout or fout.IsZombie():
        raise RuntimeError(f"无法创建输出文件: {args.output}")

    h_bg = array_to_root_hist(bg, h2, args.histname + '_bg')
    h_sig = array_to_root_hist(sig, h2, args.histname + '_sig')

    h_bg.Write()
    h_sig.Write()

    fout.Close()
    fin.Close()


if __name__ == '__main__':
    main()