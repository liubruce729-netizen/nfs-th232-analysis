#!/usr/bin/env python3
# 中文说明：对矩阵边缘进行 skew/deskew 相关处理的工具。
import ROOT
import argparse
import sys

def denoise_2d_diagonals(input_file: str,
                          hist2d_name: str,
                          out_prefix: str,
                          nIter: int = 20,
                          smoothWin: int = 7) -> None:
    """
    对一个 TH2 直方图的所有斜对角线（x+y=n）进行背景估计，重建一个二维背景图和信号图并写回 ROOT 文件。

    参数：
      input_file : ROOT 文件路径，文件中应已包含 TH2D。
      hist2d_name: 待处理的 TH2D 名称。
      out_prefix : 输出直方图名前缀，生成两个新图：
                   <out_prefix>_skew （背景图）和
                   <out_prefix>_no_skew（去背景信号图）。
      nIter      : TSpectrum.Background() 的迭代次数。
      smoothWin  : TSpectrum.Background() 的平滑窗口大小。
    """
    print(f"[Info] Opening file: {input_file}")
    f = ROOT.TFile.Open(input_file, "UPDATE")
    if not f or f.IsZombie():
        raise RuntimeError(f"Cannot open file '{input_file}' in UPDATE mode")

    print(f"[Info] Retrieving TH2: {hist2d_name}")
    h2 = f.Get(hist2d_name)
    if not h2 or not isinstance(h2, ROOT.TH2):
        f.Close()
        raise ValueError(f"Histogram '{hist2d_name}' not found or not a TH2 in {input_file}")

    spec = ROOT.TSpectrum(1000)
    nx = h2.GetNbinsX()
    ny = h2.GetNbinsY()
    total_diagonals = nx + ny - 1
    print(f"[Info] TH2 dimensions: nx={nx}, ny={ny}, total diagonals={total_diagonals}")

    # 创建背景和信号 TH2
    title = h2.GetTitle()
    hbg2 = ROOT.TH2D(f"{out_prefix}_skew", f"{title} (background)",
                     nx, h2.GetXaxis().GetXmin(), h2.GetXaxis().GetXmax(),
                     ny, h2.GetYaxis().GetXmin(), h2.GetYaxis().GetXmax())
    hsig2 = ROOT.TH2D(f"{out_prefix}_no_skew", f"{title} (signal)",
                      nx, h2.GetXaxis().GetXmin(), h2.GetXaxis().GetXmax(),
                      ny, h2.GetYaxis().GetXmin(), h2.GetYaxis().GetXmax())

    processed = 0
    for n in range(2, nx + ny + 1):
        processed += 1
        do_denoise = (10 <= n <= nx + ny - 10)
        x_min = max(1, n - ny)
        x_max = min(nx, n - 1)
        length = x_max - x_min + 1
        if length <= 0:
            # print(f"[Skip] Diagonal n={n} has no valid bins (length={length})")
            continue

        # print(f"[Info] Diagonal {processed}/{total_diagonals}: n={n}, x in [{x_min},{x_max}], length={length}, denoise={do_denoise}")

        if do_denoise:
            proj = ROOT.TH1D(f"proj_{n}", f"Diagonal x+y={n}", length, 0.5, length + 0.5)
            for ibin, x in enumerate(range(x_min, x_max + 1), start=1):
                y = n - x
                proj.SetBinContent(ibin, h2.GetBinContent(x, y))
            opt = f"BackSmoothing{smoothWin}"
            hbg = spec.Background(proj, nIter, opt)
            if not hbg:
                print(f"[Warning] TSpectrum failed on diagonal n={n}, using zero background")
                continue
            for ibin, x in enumerate(range(x_min, x_max + 1), start=1):
                y = n - x
                bg_val = hbg.GetBinContent(ibin)
                hbg2.SetBinContent(x, y, bg_val)
        else:
            # n < 10 或 n > nx+ny-10: 直接保留背景为0
            pass

    print("[Info] Filling signal matrix")
    for ix in range(1, nx + 1):
        for iy in range(1, ny + 1):
            val_orig = h2.GetBinContent(ix, iy)
            val_bg   = hbg2.GetBinContent(ix, iy)
            hsig2.SetBinContent(ix, iy, val_orig - val_bg)

    print(f"[Info] Writing output histograms: {out_prefix}_skew and {out_prefix}_no_skew")
    f.cd()
    hbg2.Write(hbg2.GetName(), ROOT.TObject.kOverwrite)
    hsig2.Write(hsig2.GetName(), ROOT.TObject.kOverwrite)
    f.Close()
    print(f"[Done] Completed processing and wrote outputs to '{input_file}'")


def main():
    parser = argparse.ArgumentParser(
        description="对 TH2 hist 每条 x+y=n 对角线进行背景降噪并重建 2D 图"
    )
    parser.add_argument("-i", "--input",    required=True,
                        help="输入 ROOT 文件（包含 TH2D）")
    parser.add_argument("-n", "--name",     required=True,
                        help="输入 TH2D 名称")
    parser.add_argument("-o", "--outprefix",required=True,
                        help="输出直方图名前缀")
    parser.add_argument("-t", "--niter",    type=int, default=20,
                        help="TSpectrum 迭代次数（默认 20）")
    parser.add_argument("-s", "--smooth",   type=int, default=7,
                        help="平滑窗口大小（默认 7）")

    args = parser.parse_args()
    try:
        denoise_2d_diagonals(args.input, args.name,
                              args.outprefix, args.niter,
                              args.smooth)
    except Exception as e:
        print(f"[Error] {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
