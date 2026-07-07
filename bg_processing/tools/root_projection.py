#!/usr/bin/env python3
# 中文说明：ROOT 投影/一维本底/二维本底构造工具；当前 project 只累加 normal bins，并可直接写 RadWare .spe。
import argparse
import struct
from pathlib import Path

import ROOT


def copy_and_project(input_file: str,
                     hist2d_name: str,
                     output_file: str,
                     proj_name: str,
                     axis: str = 'X') -> None:
    """Copy a 2D histogram and write its normal-bin 1D projection.

    ROOT's default TH2::ProjectionX/Y includes the other axis underflow and
    overflow bins.  RadWare m4b/slice projections only contain normal matrix
    bins, so this routine explicitly projects 1..N on the summed axis.
    """
    f_in = ROOT.TFile.Open(input_file, 'READ')
    if not f_in or f_in.IsZombie():
        raise RuntimeError(f"Cannot open input file '{input_file}'")
    h2 = f_in.Get(hist2d_name)
    if not h2:
        f_in.Close()
        raise ValueError(f"Histogram '{hist2d_name}' not found in {input_file}")

    f_out = ROOT.TFile.Open(output_file, 'UPDATE')
    if not f_out or f_out.IsZombie():
        f_in.Close()
        raise RuntimeError(f"Cannot create output file '{output_file}'")

    output_hist_name = Path(hist2d_name).name
    h2_clone = h2.Clone(output_hist_name)
    f_out.WriteObject(h2_clone, output_hist_name)

    if axis.upper() == 'X':
        h1 = h2.ProjectionX(proj_name, 1, h2.GetNbinsY())
        range_note = f"Y bins 1..{h2.GetNbinsY()}"
    else:
        h1 = h2.ProjectionY(proj_name, 1, h2.GetNbinsX())
        range_note = f"X bins 1..{h2.GetNbinsX()}"
    f_out.WriteObject(h1, proj_name)

    f_out.Close()
    f_in.Close()
    print(f"Projection '{proj_name}' created in '{output_file}' using {range_note}")


def add_background(input_file: str,
                   hist_name: str,
                   bg_name: str,
                   nIter: int = 20,
                   smoothWin: int = 7) -> None:
    f = ROOT.TFile.Open(input_file, 'UPDATE')
    if not f or f.IsZombie():
        raise RuntimeError(f"Cannot open file '{input_file}' in UPDATE mode")
    h = f.Get(hist_name)
    if not h:
        f.Close()
        raise ValueError(f"Histogram '{hist_name}' not found in {input_file}")

    h.GetXaxis().SetRange(1, h.GetNbinsX())
    spec = ROOT.TSpectrum(1000)
    opt = f"BackSmoothing{smoothWin}"
    print('nIter =', nIter, 'smoothWin =', smoothWin, 'opt =', opt)
    hbg = spec.Background(h, nIter, opt)
    print('bg integral =', hbg.Integral())
    if not hbg:
        f.Close()
        raise RuntimeError('TSpectrum failed to compute background')

    hbg.SetName(bg_name)
    hbg.Write(bg_name, ROOT.TObject.kOverwrite)
    f.Close()
    print(f"Background '{bg_name}' written to '{input_file}'")


def subtract_background_2d(input_file: str,
                           hist2d_name: str,
                           proj_name: str,
                           bg1d_name: str,
                           output_file: str,
                           suffix_bg: str = '_bg',
                           suffix_sig: str = '_sig') -> None:
    f_in = ROOT.TFile.Open(input_file, 'READ')
    if not f_in or f_in.IsZombie():
        raise RuntimeError(f"Cannot open input file '{input_file}'")
    h2 = f_in.Get(hist2d_name)
    hP = f_in.Get(proj_name)
    hB1 = f_in.Get(bg1d_name)
    if not all([h2, hP, hB1]):
        f_in.Close()
        missing = [n for n, o in [(hist2d_name, h2), (proj_name, hP), (bg1d_name, hB1)] if not o]
        raise ValueError(f"Missing histograms: {missing}")

    T = hP.Integral()
    if T == 0:
        f_in.Close()
        raise RuntimeError('Projection histogram integral is zero')

    f_out = ROOT.TFile.Open(output_file, 'UPDATE')
    if not f_out or f_out.IsZombie():
        f_in.Close()
        raise RuntimeError(f"Cannot create output file '{output_file}'")

    h2_clone = h2.Clone(hist2d_name)
    f_out.WriteObject(h2_clone, hist2d_name)

    nbx, nby = h2.GetNbinsX(), h2.GetNbinsY()
    hBg2d = h2.Clone(hist2d_name + suffix_bg)
    hBg2d.Reset()
    hSig2d = h2.Clone(hist2d_name + suffix_sig)
    hSig2d.Reset()

    for ix in range(1, nbx + 1):
        Pi = hP.GetBinContent(ix)
        bi = hB1.GetBinContent(ix)
        for iy in range(1, nby + 1):
            Pj = hP.GetBinContent(iy)
            bj = hB1.GetBinContent(iy)
            Bij = (Pi * bj + bi * Pj - bi * bj) / T
            val = h2.GetBinContent(ix, iy)
            hBg2d.SetBinContent(ix, iy, Bij)
            hSig2d.SetBinContent(ix, iy, val - Bij)

    f_out.WriteObject(hBg2d, hBg2d.GetName())
    f_out.WriteObject(hSig2d, hSig2d.GetName())

    f_out.Close()
    f_in.Close()
    print(f"2D background and signal written to '{output_file}' with suffixes '{suffix_bg}', '{suffix_sig}'")


def _spe_name_bytes(name: str) -> bytes:
    return name.encode('ascii', 'replace')[:8].ljust(8, b' ')


def write_spe(input_file: str,
              hist_name: str,
              output_file: str,
              spe_name: str | None = None) -> None:
    """Write a TH1 normal-bin spectrum as a RadWare gf3 .spe file."""
    f_in = ROOT.TFile.Open(input_file, 'READ')
    if not f_in or f_in.IsZombie():
        raise RuntimeError(f"Cannot open input file '{input_file}'")
    h1 = f_in.Get(hist_name)
    if not h1:
        f_in.Close()
        raise ValueError(f"Histogram '{hist_name}' not found in {input_file}")

    nbins = h1.GetNbinsX()
    values = [float(h1.GetBinContent(i)) for i in range(1, nbins + 1)]
    name = spe_name or Path(output_file).stem
    header = struct.pack('<8siiii', _spe_name_bytes(name), nbins, 1, 1, 1)
    data = struct.pack('<' + 'f' * nbins, *values)

    with open(output_file, 'wb') as out:
        out.write(struct.pack('<i', len(header)))
        out.write(header)
        out.write(struct.pack('<i', len(header)))
        out.write(struct.pack('<i', len(data)))
        out.write(data)
        out.write(struct.pack('<i', len(data)))

    f_in.Close()
    print(f"SPE '{output_file}' written from '{input_file}:{hist_name}' ({nbins} channels)")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='ROOT histogram utilities: project, background, subtract2D, spe'
    )
    subparsers = parser.add_subparsers(dest='command', required=True)

    p = subparsers.add_parser('project', help='Copy 2D hist and create normal-bin 1D projection')
    p.add_argument('-i', '--input_file', required=True)
    p.add_argument('-n', '--hist2d_name', required=True)
    p.add_argument('-o', '--output_file', required=True)
    p.add_argument('-p', '--proj_name', required=True)
    p.add_argument('-a', '--axis', choices=['X', 'Y'], default='X')

    b = subparsers.add_parser('background', help='Compute 1D background via TSpectrum')
    b.add_argument('-i', '--input_file', required=True)
    b.add_argument('-n', '--hist_name', required=True)
    b.add_argument('-b', '--bg_name', required=True)
    b.add_argument('-t', '--nIter', type=int, default=20)
    b.add_argument('-w', '--smoothWin', type=int, default=7)

    s = subparsers.add_parser('subtract2D', help='Construct 2D background and signal')
    s.add_argument('-i', '--input_file', required=True)
    s.add_argument('-n', '--hist2d_name', required=True)
    s.add_argument('-p', '--proj_name', required=True)
    s.add_argument('-b', '--bg1d_name', required=True)
    s.add_argument('-o', '--output_file', required=True)
    s.add_argument('--suffix_bg', default='_bg')
    s.add_argument('--suffix_sig', default='_sig')

    sp = subparsers.add_parser('spe', help='Write a TH1 normal-bin spectrum as RadWare .spe')
    sp.add_argument('-i', '--input_file', required=True)
    sp.add_argument('-n', '--hist_name', required=True)
    sp.add_argument('-o', '--output_file', required=True)
    sp.add_argument('--spe_name')

    args = parser.parse_args()
    if args.command == 'project':
        copy_and_project(args.input_file, args.hist2d_name, args.output_file, args.proj_name, args.axis)
    elif args.command == 'background':
        add_background(args.input_file, args.hist_name, args.bg_name, args.nIter, args.smoothWin)
    elif args.command == 'subtract2D':
        subtract_background_2d(args.input_file, args.hist2d_name, args.proj_name, args.bg1d_name,
                               args.output_file, args.suffix_bg, args.suffix_sig)
    elif args.command == 'spe':
        write_spe(args.input_file, args.hist_name, args.output_file, args.spe_name)
