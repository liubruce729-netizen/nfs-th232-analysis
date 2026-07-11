// Scan a trial period and fold an existing ROOT TH1 along its x-axis.
// 扫描候选周期，并把已有 ROOT TH1 按横轴周期折叠。
//
// Recommended usage / 推荐用法:
//   root -l -b -q 'lsy_nfs/scan_fold_period.C("out/mfm_ts_diff.root","mfm_exo2_frame_ts_distribution",1000,2000,1,10,"out/period_scan.root")'
//   root -l -b -q 'lsy_nfs/scan_fold_period.C("out/mfm_ts_diff.root","exo2_crystal_ts/mfm_exo2_board111_crystal00_ts_distribution",1000,2000,1,10,"out/period_scan_board111_c00.root")'
//
// Arguments / 参数:
//   inputRoot    : input ROOT file containing a TH1 / 包含 TH1 的输入 ROOT 文件
//   histName     : full TH1 path inside ROOT, directories separated by '/' / ROOT 内部 TH1 完整路径
//   periodMinNs  : first trial period in ns / 最小扫描周期，单位 ns
//   periodMaxNs  : last trial period in ns / 最大扫描周期，单位 ns
//   periodStepNs : period scan step in ns / 周期扫描步长，单位 ns
//   phaseBinNs   : folded phase histogram bin width; only ROOT binning, not physics cut / 折叠后相位图 bin 宽，仅用于直方图分箱
//   outputRoot   : output ROOT path / 输出 ROOT 文件
//   t0Ns         : optional phase origin in ns / 可选相位零点，单位 ns
//   writeFolded1D: write one folded TH1D for each trial period / 是否为每个周期写一张折叠后的一维图

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "TCanvas.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TNamed.h"
#include "TString.h"

namespace {

TString SafeNamePart(Double_t value)
{
  TString s = TString::Format("%.6f", value);
  s.ReplaceAll("-", "m");
  s.ReplaceAll(".", "p");
  return s;
}

TH1D *FoldHistogramByPeriod(const TH1 *input,
                            Double_t periodNs,
                            Double_t phaseBinNs,
                            Double_t t0Ns,
                            const char *name,
                            const char *title)
{
  const Int_t nPhaseBins = std::max<Int_t>(1, static_cast<Int_t>(std::ceil(periodNs / phaseBinNs)));
  const Double_t xMax = nPhaseBins * phaseBinNs;
  TH1D *folded = new TH1D(name, title, nPhaseBins, 0.0, xMax);
  folded->SetDirectory(nullptr);
  folded->Sumw2(false);

  std::vector<Double_t> err2(nPhaseBins + 1, 0.0);
  Double_t totalCounts = 0.0;
  for (Int_t i = 1; i <= input->GetNbinsX(); ++i) {
    const Double_t counts = input->GetBinContent(i);
    if (counts == 0.0) continue;
    totalCounts += counts;

    const Double_t timeNs = input->GetXaxis()->GetBinCenter(i);
    Double_t phaseNs = std::fmod(timeNs - t0Ns, periodNs);
    if (phaseNs < 0.0) phaseNs += periodNs;

    const Int_t phaseBin = folded->FindBin(phaseNs);
    if (phaseBin < 1 || phaseBin > nPhaseBins) continue;

    folded->AddBinContent(phaseBin, counts);
    const Double_t err = input->GetBinError(i);
    err2[phaseBin] += err * err;
  }

  for (Int_t b = 1; b <= nPhaseBins; ++b) {
    folded->SetBinError(b, std::sqrt(err2[b]));
  }
  folded->SetEntries(totalCounts);
  return folded;
}

void ScoreFoldedHistogram(const TH1D *folded,
                          Double_t &chi2OverN,
                          Double_t &peakOverMean,
                          Double_t &rmsOverMean)
{
  const Int_t nBins = folded ? folded->GetNbinsX() : 0;
  chi2OverN = 0.0;
  peakOverMean = 0.0;
  rmsOverMean = 0.0;
  if (nBins <= 0) return;

  Double_t total = 0.0;
  Double_t peak = 0.0;
  for (Int_t b = 1; b <= nBins; ++b) {
    const Double_t value = folded->GetBinContent(b);
    total += value;
    if (value > peak) peak = value;
  }
  if (total <= 0.0) return;

  const Double_t mean = total / static_cast<Double_t>(nBins);
  Double_t sumSquares = 0.0;
  for (Int_t b = 1; b <= nBins; ++b) {
    const Double_t diff = folded->GetBinContent(b) - mean;
    sumSquares += diff * diff;
    chi2OverN += diff * diff / mean;
  }

  chi2OverN /= static_cast<Double_t>(nBins);
  peakOverMean = peak / mean;
  rmsOverMean = std::sqrt(sumSquares / static_cast<Double_t>(nBins)) / mean;
}

void SaveCanvas(TH1 *hist, const char *drawOption)
{
  if (!hist) return;
  TCanvas canvas(TString::Format("c_%s", hist->GetName()), hist->GetTitle(), 1200, 800);
  hist->Draw(drawOption);
  canvas.Write();
}

} // namespace

void scan_fold_period(const char *inputRoot,
                      const char *histName,
                      Double_t periodMinNs,
                      Double_t periodMaxNs,
                      Double_t periodStepNs,
                      Double_t phaseBinNs = 10.0,
                      const char *outputRoot = "period_scan.root",
                      Double_t t0Ns = 0.0,
                      Bool_t writeFolded1D = true)
{
  if (!inputRoot || TString(inputRoot).IsNull() || !histName || TString(histName).IsNull()) {
    std::cerr << "Input ROOT and histogram name are required. / 必须给出输入 ROOT 和图名。" << std::endl;
    return;
  }
  if (periodMinNs <= 0.0 || periodMaxNs < periodMinNs || periodStepNs <= 0.0 || phaseBinNs <= 0.0) {
    std::cerr << "Bad period/bin settings. / 周期或 bin 参数不合法。" << std::endl;
    return;
  }

  TFile inputFile(inputRoot, "READ");
  if (!inputFile.IsOpen() || inputFile.IsZombie()) {
    std::cerr << "Cannot open input ROOT / 无法打开输入 ROOT: " << inputRoot << std::endl;
    return;
  }

  TH1 *inputHist = dynamic_cast<TH1 *>(inputFile.Get(histName));
  if (!inputHist) {
    std::cerr << "Cannot find TH1 / 找不到 TH1: " << histName << std::endl;
    return;
  }

  std::vector<Double_t> periods;
  for (Double_t period = periodMinNs; period <= periodMaxNs + 0.5 * periodStepNs; period += periodStepNs) {
    if (period <= periodMaxNs + 1e-9) periods.push_back(period);
  }
  if (periods.empty()) {
    std::cerr << "No trial periods generated. / 没有生成任何待扫描周期。" << std::endl;
    return;
  }

  const Int_t nPeriods = static_cast<Int_t>(periods.size());
  const Int_t nPhaseBinsMax = std::max<Int_t>(1, static_cast<Int_t>(std::ceil(periodMaxNs / phaseBinNs)));
  const Double_t phaseMax = nPhaseBinsMax * phaseBinNs;
  const Double_t yMin = periods.front() - 0.5 * periodStepNs;
  const Double_t yMax = periods.back() + 0.5 * periodStepNs;

  TFile outputFile(outputRoot, "RECREATE");
  if (!outputFile.IsOpen() || outputFile.IsZombie()) {
    std::cerr << "Cannot create output ROOT / 无法创建输出 ROOT: " << outputRoot << std::endl;
    return;
  }

  const TString configTitle = TString::Format(
      "input=%s; hist=%s; period_min_ns=%.12g; period_max_ns=%.12g; period_step_ns=%.12g; phase_bin_ns=%.12g; t0_ns=%.12g; write_folded_1d=%d",
      inputRoot, histName, periodMinNs, periodMaxNs, periodStepNs, phaseBinNs,
      t0Ns, writeFolded1D ? 1 : 0);
  TNamed config("period_scan_config", configTitle.Data());
  config.Write();

  TH2D hMapRaw("period_fold_map_raw",
               "Period-fold scan raw counts;Time modulo trial period (ns);Trial period (ns);Counts",
               nPhaseBinsMax, 0.0, phaseMax, nPeriods, yMin, yMax);
  TH2D hMapNorm("period_fold_map_norm",
                "Period-fold scan normalized by row mean;Time modulo trial period (ns);Trial period (ns);Counts / row mean",
                nPhaseBinsMax, 0.0, phaseMax, nPeriods, yMin, yMax);
  TH1D hChi2("period_scan_chi2_over_n",
             "Period scan score: folded non-flatness;Trial period (ns);#chi^{2}/N",
             nPeriods, yMin, yMax);
  TH1D hPeakMean("period_scan_peak_over_mean",
                 "Period scan score: peak over mean;Trial period (ns);Peak / mean",
                 nPeriods, yMin, yMax);
  TH1D hRmsMean("period_scan_rms_over_mean",
                "Period scan score: RMS over mean;Trial period (ns);RMS / mean",
                nPeriods, yMin, yMax);

  TDirectory *foldedDir = nullptr;
  if (writeFolded1D) foldedDir = outputFile.mkdir("folded_1d");

  Double_t bestPeriod = periods.front();
  Double_t bestScore = -std::numeric_limits<Double_t>::infinity();
  TH1D *bestFolded = nullptr;

  for (Int_t ip = 0; ip < nPeriods; ++ip) {
    const Double_t periodNs = periods[ip];
    const TString periodTag = SafeNamePart(periodNs);
    const TString foldedName = TString::Format("fold_%s_period_%sns", inputHist->GetName(), periodTag.Data());
    const TString foldedTitle = TString::Format("%s folded at %.6g ns;Time modulo trial period (ns);Counts",
                                                inputHist->GetTitle(), periodNs);
    TH1D *folded = FoldHistogramByPeriod(inputHist, periodNs, phaseBinNs, t0Ns,
                                         foldedName.Data(), foldedTitle.Data());

    Double_t chi2OverN = 0.0;
    Double_t peakOverMean = 0.0;
    Double_t rmsOverMean = 0.0;
    ScoreFoldedHistogram(folded, chi2OverN, peakOverMean, rmsOverMean);
    hChi2.SetBinContent(ip + 1, chi2OverN);
    hPeakMean.SetBinContent(ip + 1, peakOverMean);
    hRmsMean.SetBinContent(ip + 1, rmsOverMean);

    const Double_t rowTotal = folded->Integral(1, folded->GetNbinsX());
    const Double_t rowMean = folded->GetNbinsX() > 0 ? rowTotal / static_cast<Double_t>(folded->GetNbinsX()) : 0.0;
    for (Int_t bx = 1; bx <= folded->GetNbinsX(); ++bx) {
      const Double_t phase = folded->GetXaxis()->GetBinCenter(bx);
      const Int_t mapX = hMapRaw.GetXaxis()->FindBin(phase);
      if (mapX < 1 || mapX > hMapRaw.GetNbinsX()) continue;
      const Double_t value = folded->GetBinContent(bx);
      hMapRaw.SetBinContent(mapX, ip + 1, value);
      if (rowMean > 0.0) hMapNorm.SetBinContent(mapX, ip + 1, value / rowMean);
    }

    if (chi2OverN > bestScore) {
      bestScore = chi2OverN;
      bestPeriod = periodNs;
      delete bestFolded;
      bestFolded = dynamic_cast<TH1D *>(folded->Clone("best_folded_hist"));
      if (bestFolded) {
        bestFolded->SetDirectory(nullptr);
        bestFolded->SetTitle(TString::Format("Best folded histogram at %.6g ns;Time modulo trial period (ns);Counts", periodNs));
      }
    }

    if (writeFolded1D && foldedDir) {
      foldedDir->cd();
      folded->Write();
      outputFile.cd();
    }
    delete folded;
  }

  outputFile.cd();
  hMapRaw.Write();
  hMapNorm.Write();
  hChi2.Write();
  hPeakMean.Write();
  hRmsMean.Write();
  if (bestFolded) bestFolded->Write();

  SaveCanvas(&hMapRaw, "COLZ");
  SaveCanvas(&hMapNorm, "COLZ");
  SaveCanvas(&hChi2, "hist");
  SaveCanvas(&hPeakMean, "hist");
  SaveCanvas(&hRmsMean, "hist");
  if (bestFolded) SaveCanvas(bestFolded, "hist");

  outputFile.Close();
  delete bestFolded;

  std::cout << "Input ROOT: " << inputRoot << std::endl;
  std::cout << "Histogram: " << histName << std::endl;
  std::cout << "Scanned periods: " << nPeriods << std::endl;
  std::cout << "Best period by chi2/N: " << bestPeriod << " ns" << std::endl;
  std::cout << "Best chi2/N: " << bestScore << std::endl;
  std::cout << "Output ROOT: " << outputRoot << std::endl;
}
