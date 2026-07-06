#!/usr/bin/env python3
# 中文说明：两个 ROOT histogram 相加/相减工具，用于 delta 和最终矩阵回加。
# -*- coding: utf-8 -*-
"""
subtract_histograms.py

用法示例:
    # A - B
    python subtract_histograms.py \
        -a file1.root H1 \
        -b file2.root H2 \
        -o result.root Hdiff \
        -s -1

    # A + B
    python subtract_histograms.py \
        -a file1.root H1 \
        -b file2.root H2 \
        -o result.root Hsum \
        -s 1
"""

import sys
import argparse
import ROOT

def parse_args():
    p = argparse.ArgumentParser(
        description="对两个 2D 直方图做线性组合 A + s*B（s=±1），输出结果直方图。"
    )
    p.add_argument(
        "-a", "--hist1", nargs=2, required=True, metavar=("ROOT_FILE", "HIST_NAME"),
        help="第一个直方图所在文件及名称（A）"
    )
    p.add_argument(
        "-b", "--hist2", nargs=2, required=True, metavar=("ROOT_FILE", "HIST_NAME"),
        help="第二个直方图所在文件及名称（B）"
    )
    p.add_argument(
        "-o", "--output", nargs=2, required=True, metavar=("OUT_FILE", "OUT_NAME"),
        help="输出 ROOT 文件路径及结果直方图的名称"
    )
    p.add_argument(
        "-s", "--sign", type=int, choices=[1, -1], default=-1,
        help="对第二个直方图的系数 s，取 1 (A+B) 或 -1 (A-B)。默认 -1。"
    )
    return p.parse_args()

def open_hist(file_name, hist_name):
    f = ROOT.TFile.Open(file_name, "READ")
    if not f or f.IsZombie():
        raise RuntimeError(f"无法打开 ROOT 文件 '{file_name}'")
    hist = f.Get(hist_name)
    if not hist:
        raise RuntimeError(f"无法读取文件 '{file_name}' 中的直方图 '{hist_name}'")
    return f, hist

def main():
    args = parse_args()

    # 打开直方图
    f1, h1 = open_hist(*args.hist1)
    f2, h2 = open_hist(*args.hist2)

    # 检查 bin 数是否一致
    if h1.GetNbinsX() != h2.GetNbinsX() or h1.GetNbinsY() != h2.GetNbinsY():
        raise RuntimeError("两个直方图的维度或 bin 数不一致，无法相加/相减。")

    # 克隆 h1，作为结果直方图
    hres = h1.Clone(args.output[1])
    op_sym = "+" if args.sign == 1 else "-"
    hres.SetTitle(f"{args.hist1[1]} {op_sym} {args.hist2[1]}")

    # 执行 A + s*B（s=±1）
    hres.Add(h2, args.sign)

    # —— 禁用误差存储，并清零已有的误差 —— 
    hres.Sumw2(False)
    nbx, nby = hres.GetNbinsX(), hres.GetNbinsY()
    for i in range(1, nbx+1):
        for j in range(1, nby+1):
            hres.SetBinError(i, j, 0.0)

    # 写入输出文件
    fout = ROOT.TFile.Open(args.output[0], "UPDATE")
    if not fout or fout.IsZombie():
        raise RuntimeError(f"无法打开输出文件 '{args.output[0]}' 进行写入")
    fout.cd()
    hres.Write(args.output[1], ROOT.TObject.kOverwrite)
    fout.Close()

    # 关闭输入文件
    f1.Close()
    f2.Close()

    print(f"完成: 已将 {args.hist1[1]} {op_sym} {args.hist2[1]} 写入 {args.output[0]} -> {args.output[1]}")

if __name__ == "__main__":
    main()
