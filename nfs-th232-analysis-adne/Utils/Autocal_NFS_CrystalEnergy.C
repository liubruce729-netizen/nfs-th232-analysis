// NFS EXOGAM crystal energy auto-calibration helper.
// NFS EXOGAM crystal 能量自动刻度辅助脚本。
//
// This macro is adapted from the old Utils/Autocal.cxx idea, but it is
// dedicated to the NFS histogram names:
//   nfs_clover%d_crystal%d_energy
//
// 这个宏借鉴了旧版 Utils/Autocal.cxx 的思路，但专门适配 NFS 输出图名：
//   nfs_clover%d_crystal%d_energy
//
// Run from nfs-th232-analysis-adne / 建议在 nfs-th232-analysis-adne 目录下运行:
//   root -l -b -q 'Utils/Autocal_NFS_CrystalEnergy.C("out/nfs_histoExogam2_1.root","Co60")'
//
// Source options / 源选项:
//   "Co60" or "Co"       : 1173.237, 1332.501 keV
//   "Eu152" or "Eu"      : common Eu-152 lines used in the old Autocal.cxx
//   "Eu152Att" or "EuAtt": Eu-152 without the low 121.783 keV line
//
// Important / 重要:
// The fitted coefficients map the x-axis of the input histograms to true
// gamma energies. If the histograms were already filled with an existing
// ecc.cal, these coefficients are correction coefficients for that histogram
// axis, not automatically composed raw-channel coefficients.
//
// 拟合得到的是“输入直方图横轴 -> 真实 gamma 能量”的刻度系数。如果这些谱已经
// 使用旧 ecc.cal 填充，那么这些系数是针对当前谱横轴的修正系数，并不会自动和
// 原始 ADC/TDC 通道到能量的旧系数做复合。

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "TCanvas.h"
#include "TDirectory.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TH1D.h"
#include "TLegend.h"
#include "TMath.h"
#include "TPaveText.h"
#include "TString.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TTree.h"

namespace {

struct SourceDefinition {
  TString name;
  std::vector<double> energiesKeV;
  double searchHalfWidthKeV;
  double fitHalfWidthKeV;
};

struct Coefficients {
  double offset;
  double gain;
  double gain2;
};

struct PeakFitResult {
  int sourceIndex;
  double expectedKeV;
  double meanKeV;
  double meanErrKeV;
  double sigmaKeV;
  double fwhmKeV;
  double amplitude;
  double backgroundOffset;
  double backgroundSlope;
  double fitLowKeV;
  double fitHighKeV;
  double peakCounts;
  double chi2Ndf;
  int fitStatus;
  bool ok;
};

struct CrystalResult {
  int clover;
  int crystal;
  int detector;
  double entries;
  bool histFound;
  bool calibrationOk;
  Coefficients coeff;
  std::vector<PeakFitResult> peaks;
  double calChi2Ndf;
  int usedPeaks;
  TString status;
};

TString NormalizeSourceName(const TString &sourceName)
{
  TString s(sourceName);
  s.ToLower();
  s.ReplaceAll("-", "");
  s.ReplaceAll("_", "");
  s.ReplaceAll(" ", "");
  return s;
}

SourceDefinition MakeSourceDefinition(const char *sourceName)
{
  const TString s = NormalizeSourceName(sourceName);
  SourceDefinition source;

  if (s == "co" || s == "co60" || s == "60co") {
    source.name = "Co60";
    source.energiesKeV = {1173.237, 1332.501};
    source.searchHalfWidthKeV = 90.0;
    source.fitHalfWidthKeV = 30.0;
    return source;
  }

  if (s == "eu" || s == "eu152" || s == "152eu") {
    source.name = "Eu152";
    source.energiesKeV = {
        121.7830, 244.6920, 344.2760, 778.9030,
        964.1310, 1085.800, 1112.1160, 1408.0110};
    source.searchHalfWidthKeV = 45.0;
    source.fitHalfWidthKeV = 18.0;
    return source;
  }

  if (s == "euatt" || s == "eu152att" || s == "152euatt" ||
      s == "euattenuator" || s == "eu152attenuator") {
    source.name = "Eu152Att";
    source.energiesKeV = {
        244.6920, 344.2760, 778.9030, 964.1310,
        1085.800, 1112.1160, 1408.0110};
    source.searchHalfWidthKeV = 45.0;
    source.fitHalfWidthKeV = 18.0;
    return source;
  }

  std::cerr << "Unknown source / 未知源: " << sourceName
            << ". Fallback to Co60 / 默认改用 Co60." << std::endl;
  return MakeSourceDefinition("Co60");
}

bool ReadEccCal(const char *path,
                std::vector<Coefficients> &energyCoeff,
                std::vector<Coefficients> &timeCoeff)
{
  energyCoeff.clear();
  timeCoeff.clear();

  std::ifstream in(path);
  if (!in.is_open()) return false;

  double a = 0.0;
  double b = 0.0;
  double c = 0.0;
  std::vector<Coefficients> all;
  while (in >> a >> b >> c) {
    all.push_back({a, b, c});
  }

  if (all.size() < 128) return false;
  energyCoeff.assign(all.begin(), all.begin() + 64);
  timeCoeff.assign(all.begin() + 64, all.begin() + 128);
  return true;
}

int FindMaximumBinInRange(TH1 *h, double xmin, double xmax)
{
  if (!h) return -1;
  int firstBin = std::max(1, h->GetXaxis()->FindBin(xmin));
  int lastBin = std::min(h->GetNbinsX(), h->GetXaxis()->FindBin(xmax));
  if (lastBin < firstBin) return -1;

  int bestBin = firstBin;
  double bestCounts = h->GetBinContent(firstBin);
  for (int bin = firstBin + 1; bin <= lastBin; ++bin) {
    const double counts = h->GetBinContent(bin);
    if (counts > bestCounts) {
      bestCounts = counts;
      bestBin = bin;
    }
  }
  return bestBin;
}

PeakFitResult FitOnePeak(TH1 *h,
                         int sourceIndex,
                         double expectedKeV,
                         double searchHalfWidthKeV,
                         double fitHalfWidthKeV,
                         double minPeakCounts)
{
  PeakFitResult result;
  result.sourceIndex = sourceIndex;
  result.expectedKeV = expectedKeV;
  result.meanKeV = 0.0;
  result.meanErrKeV = 0.0;
  result.sigmaKeV = 0.0;
  result.fwhmKeV = 0.0;
  result.amplitude = 0.0;
  result.backgroundOffset = 0.0;
  result.backgroundSlope = 0.0;
  result.fitLowKeV = 0.0;
  result.fitHighKeV = 0.0;
  result.peakCounts = 0.0;
  result.chi2Ndf = 0.0;
  result.fitStatus = -999;
  result.ok = false;

  if (!h || h->GetEntries() <= 0) return result;

  const double searchLow = expectedKeV - searchHalfWidthKeV;
  const double searchHigh = expectedKeV + searchHalfWidthKeV;
  const int maxBin = FindMaximumBinInRange(h, searchLow, searchHigh);
  if (maxBin < 1) return result;

  const double peakX = h->GetBinCenter(maxBin);
  const double peakY = h->GetBinContent(maxBin);
  result.peakCounts = peakY;
  if (peakY < minPeakCounts) return result;

  const double fitLow = std::max(h->GetXaxis()->GetXmin(), peakX - fitHalfWidthKeV);
  const double fitHigh = std::min(h->GetXaxis()->GetXmax(), peakX + fitHalfWidthKeV);
  if (fitHigh <= fitLow) return result;
  result.fitLowKeV = fitLow;
  result.fitHighKeV = fitHigh;

  const double binWidth = h->GetBinWidth(maxBin);
  const double leftY = h->GetBinContent(std::max(1, h->GetXaxis()->FindBin(fitLow)));
  const double rightY = h->GetBinContent(std::min(h->GetNbinsX(), h->GetXaxis()->FindBin(fitHigh)));
  const double bg0 = 0.5 * (leftY + rightY);
  const double amp0 = std::max(peakY - bg0, peakY * 0.5);
  const double sigma0 = std::max(1.2 * binWidth, fitHalfWidthKeV / 5.0);

  TF1 fitFunc(Form("nfs_peak_fit_%d_%.0f", sourceIndex, expectedKeV),
              "gaus(0)+pol1(3)", fitLow, fitHigh);
  fitFunc.SetParameters(amp0, peakX, sigma0, bg0, 0.0);
  fitFunc.SetParLimits(0, 0.0, std::max(peakY * 20.0, 1.0));
  fitFunc.SetParLimits(1, peakX - fitHalfWidthKeV * 0.75, peakX + fitHalfWidthKeV * 0.75);
  fitFunc.SetParLimits(2, std::max(0.2 * binWidth, 0.1), fitHalfWidthKeV);
  fitFunc.SetLineColor(kRed + sourceIndex % 6);
  fitFunc.SetNpx(500);

  // CN: Q 静默，R 限制拟合区间，S 返回 TFitResultPtr。
  // EN: Q is quiet, R restricts the range, S returns TFitResultPtr.
  const int status = h->Fit(&fitFunc, "QRS0");
  result.fitStatus = status;

  const double mean = fitFunc.GetParameter(1);
  const double sigma = std::abs(fitFunc.GetParameter(2));
  const double amp = fitFunc.GetParameter(0);
  const double ndf = fitFunc.GetNDF();
  result.amplitude = amp;
  result.backgroundOffset = fitFunc.GetParameter(3);
  result.backgroundSlope = fitFunc.GetParameter(4);
  result.meanKeV = mean;
  result.meanErrKeV = fitFunc.GetParError(1);
  result.sigmaKeV = sigma;
  result.fwhmKeV = 2.354820045 * sigma;
  result.chi2Ndf = ndf > 0.0 ? fitFunc.GetChisquare() / ndf : 0.0;
  result.ok = (status == 0 || status == 4000) &&
              amp > 0.0 &&
              sigma > 0.0 &&
              mean >= searchLow &&
              mean <= searchHigh;

  return result;
}

bool FitCalibration(const std::vector<PeakFitResult> &peaks,
                    bool useQuadratic,
                    Coefficients &coeff,
                    double &chi2Ndf,
                    int &usedPeaks)
{
  std::vector<double> measured;
  std::vector<double> expected;
  std::vector<double> measuredErr;
  for (const auto &peak : peaks) {
    if (!peak.ok) continue;
    measured.push_back(peak.meanKeV);
    expected.push_back(peak.expectedKeV);
    measuredErr.push_back(std::max(peak.meanErrKeV, 0.001));
  }

  usedPeaks = static_cast<int>(measured.size());
  coeff = {0.0, 0.0, 0.0};
  chi2Ndf = 0.0;

  const int minPeaks = useQuadratic ? 3 : 2;
  if (usedPeaks < minPeaks) return false;

  TGraphErrors graph(usedPeaks);
  for (int i = 0; i < usedPeaks; ++i) {
    graph.SetPoint(i, measured[i], expected[i]);
    graph.SetPointError(i, measuredErr[i], 0.0);
  }

  TF1 calFunc("nfs_energy_calibration_fit",
              useQuadratic ? "pol2" : "pol1",
              0.0, 20000.0);
  const int status = graph.Fit(&calFunc, "QS0");
  if (!(status == 0 || status == 4000)) return false;

  coeff.offset = calFunc.GetParameter(0);
  coeff.gain = calFunc.GetParameter(1);
  coeff.gain2 = useQuadratic ? calFunc.GetParameter(2) : 0.0;
  const double ndf = calFunc.GetNDF();
  chi2Ndf = ndf > 0.0 ? calFunc.GetChisquare() / ndf : 0.0;
  return true;
}

void DrawCrystalFit(TDirectory *canvasDir,
                    TH1 *h,
                    const CrystalResult &result,
                    const SourceDefinition &source)
{
  if (!canvasDir || !h) return;
  canvasDir->cd();

  TCanvas *c = new TCanvas(Form("c_nfs_clover%d_crystal%d_energy_fit",
                                result.clover, result.crystal),
                           Form("Clover%d Crystal%d energy calibration",
                                result.clover, result.crystal),
                           1200, 850);
  h->SetLineColor(kBlack);
  h->SetTitle(Form("Clover%d Crystal%d %s energy fit;Energy (keV);Counts",
                   result.clover, result.crystal, source.name.Data()));
  h->Draw("hist");

  const double ymax = h->GetMaximum();
  for (const auto &peak : result.peaks) {
    TLine *lineExpected = new TLine(peak.expectedKeV, 0.0, peak.expectedKeV, ymax * 0.92);
    lineExpected->SetLineColor(kBlue + 1);
    lineExpected->SetLineStyle(2);
    lineExpected->Write(Form("expected_line_%d", peak.sourceIndex));
    lineExpected->Draw("same");

    if (peak.ok) {
      TF1 *curve = new TF1(Form("fit_curve_clover%d_crystal%d_peak%d",
                                result.clover, result.crystal, peak.sourceIndex),
                           "gaus(0)+pol1(3)",
                           peak.fitLowKeV,
                           peak.fitHighKeV);
      curve->SetParameters(peak.amplitude,
                           peak.meanKeV,
                           peak.sigmaKeV,
                           peak.backgroundOffset,
                           peak.backgroundSlope);
      curve->SetLineColor(kRed + 1 + peak.sourceIndex % 4);
      curve->SetLineWidth(2);
      curve->SetNpx(500);
      curve->Draw("same");

      TLine *lineFit = new TLine(peak.meanKeV, 0.0, peak.meanKeV, ymax * 0.78);
      lineFit->SetLineColor(kRed + 1);
      lineFit->SetLineStyle(1);
      lineFit->Write(Form("fit_line_%d", peak.sourceIndex));
      lineFit->Draw("same");
    }
  }

  TPaveText *text = new TPaveText(0.58, 0.70, 0.89, 0.88, "NDC");
  text->SetFillColor(0);
  text->SetBorderSize(1);
  text->AddText(Form("status: %s", result.status.Data()));
  text->AddText(Form("offset = %.8g", result.coeff.offset));
  text->AddText(Form("gain   = %.8g", result.coeff.gain));
  text->AddText(Form("gain2  = %.8g", result.coeff.gain2));
  text->Draw("same");

  c->Write();
}

TString MakeDefaultOutputPrefix(const char *inputFile, const TString &sourceName)
{
  TString inputPath(inputFile);
  TString outDir = gSystem->DirName(inputPath);
  TString baseName = gSystem->BaseName(inputPath);
  baseName.ReplaceAll(".root", "");
  return Form("%s/%s_%s_energy_autocal",
              outDir.Data(), baseName.Data(), sourceName.Data());
}

} // namespace

void Autocal_NFS_CrystalEnergy(
    const char *inputFile = "out/nfs_histoExogam2_1.root",
    const char *sourceName = "Co60",
    const char *outputPrefix = "",
    const char *existingEccCal = "CalFile/ecc.cal",
    double minPeakCounts = 20.0,
    bool useQuadratic = false)
{
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);

  const SourceDefinition source = MakeSourceDefinition(sourceName);
  TString prefix(outputPrefix);
  if (prefix.Length() == 0) {
    prefix = MakeDefaultOutputPrefix(inputFile, source.name);
  }

  const TString outputRoot = prefix + ".root";
  const TString outputTable = prefix + "_table.txt";
  const TString outputEnergyBlock = prefix + "_energy_block.cal";
  const TString outputCandidateEcc = prefix + "_ecc_candidate.cal";

  TFile *fin = TFile::Open(inputFile, "READ");
  if (!fin || fin->IsZombie()) {
    std::cerr << "Cannot open input file / 无法打开输入文件: "
              << inputFile << std::endl;
    return;
  }

  std::vector<Coefficients> oldEnergyCoeff;
  std::vector<Coefficients> oldTimeCoeff;
  const bool haveOldEcc = ReadEccCal(existingEccCal, oldEnergyCoeff, oldTimeCoeff);
  if (!haveOldEcc) {
    std::cerr << "Warning / 警告: cannot read existing ecc.cal from "
              << existingEccCal
              << ". The full ecc candidate file will use zero time rows."
              << std::endl;
    oldEnergyCoeff.assign(64, {0.0, 0.0, 0.0});
    oldTimeCoeff.assign(64, {0.0, 0.0, 0.0});
  }

  TFile fout(outputRoot, "RECREATE");
  if (fout.IsZombie()) {
    std::cerr << "Cannot create output ROOT file / 无法创建输出 ROOT 文件: "
              << outputRoot << std::endl;
    fin->Close();
    return;
  }

  TDirectory *canvasDir = fout.mkdir("fit_canvases");

  TH1D *hOffset = new TH1D("nfs_energy_cal_offset_by_detector",
                           "NFS crystal energy calibration offset;Detector number (clover*4+crystal);Offset (keV)",
                           64, -0.5, 63.5);
  TH1D *hGain = new TH1D("nfs_energy_cal_gain_by_detector",
                         "NFS crystal energy calibration gain;Detector number (clover*4+crystal);Gain",
                         64, -0.5, 63.5);
  TH1D *hGain2 = new TH1D("nfs_energy_cal_gain2_by_detector",
                          "NFS crystal energy calibration gain2;Detector number (clover*4+crystal);Gain2 (1/keV)",
                          64, -0.5, 63.5);
  TH1D *hStatus = new TH1D("nfs_energy_cal_status_by_detector",
                           "NFS crystal energy calibration status;Detector number (clover*4+crystal);Status code",
                           64, -0.5, 63.5);
  hStatus->GetYaxis()->SetTitle("1=ok, 0=failed, -1=missing");

  TTree table("nfs_energy_autocal_table",
              "NFS crystal energy auto-calibration table");
  Int_t clover = -1;
  Int_t crystal = -1;
  Int_t detector = -1;
  Double_t entries = 0.0;
  Int_t histFound = 0;
  Int_t calibrationOk = 0;
  Int_t usedPeaks = 0;
  Double_t offset = 0.0;
  Double_t gain = 0.0;
  Double_t gain2 = 0.0;
  Double_t calChi2Ndf = 0.0;
  TString status;
  table.Branch("clover", &clover);
  table.Branch("crystal", &crystal);
  table.Branch("detector", &detector);
  table.Branch("entries", &entries);
  table.Branch("histFound", &histFound);
  table.Branch("calibrationOk", &calibrationOk);
  table.Branch("usedPeaks", &usedPeaks);
  table.Branch("offset", &offset);
  table.Branch("gain", &gain);
  table.Branch("gain2", &gain2);
  table.Branch("calChi2Ndf", &calChi2Ndf);
  table.Branch("status", &status);

  std::vector<CrystalResult> results(64);

  for (clover = 0; clover < 16; ++clover) {
    for (crystal = 0; crystal < 4; ++crystal) {
      detector = clover * 4 + crystal;

      CrystalResult result;
      result.clover = clover;
      result.crystal = crystal;
      result.detector = detector;
      result.entries = 0.0;
      result.histFound = false;
      result.calibrationOk = false;
      result.coeff = {0.0, 0.0, 0.0};
      result.calChi2Ndf = 0.0;
      result.usedPeaks = 0;
      result.status = "missing";

      const TString histName = Form("nfs_clover%d_crystal%d_energy", clover, crystal);
      TH1 *h = dynamic_cast<TH1 *>(fin->Get(histName));
      if (h) {
        result.histFound = true;
        result.entries = h->GetEntries();

        if (result.entries > 0.0) {
          for (std::size_t i = 0; i < source.energiesKeV.size(); ++i) {
            result.peaks.push_back(FitOnePeak(h,
                                              static_cast<int>(i),
                                              source.energiesKeV[i],
                                              source.searchHalfWidthKeV,
                                              source.fitHalfWidthKeV,
                                              minPeakCounts));
          }

          result.calibrationOk = FitCalibration(result.peaks,
                                                useQuadratic,
                                                result.coeff,
                                                result.calChi2Ndf,
                                                result.usedPeaks);
          result.status = result.calibrationOk ? "ok" : "fit_failed";
          DrawCrystalFit(canvasDir, h, result, source);
        } else {
          result.status = "empty";
        }
      }

      results[detector] = result;

      entries = result.entries;
      histFound = result.histFound ? 1 : 0;
      calibrationOk = result.calibrationOk ? 1 : 0;
      usedPeaks = result.usedPeaks;
      offset = result.coeff.offset;
      gain = result.coeff.gain;
      gain2 = result.coeff.gain2;
      calChi2Ndf = result.calChi2Ndf;
      status = result.status;
      table.Fill();

      if (result.histFound) hStatus->SetBinContent(detector + 1, 0.0);
      else hStatus->SetBinContent(detector + 1, -1.0);

      if (result.calibrationOk) {
        hStatus->SetBinContent(detector + 1, 1.0);
        hOffset->SetBinContent(detector + 1, result.coeff.offset);
        hGain->SetBinContent(detector + 1, result.coeff.gain);
        hGain2->SetBinContent(detector + 1, result.coeff.gain2);
      }
    }
  }

  // CN: 人可读表，包含每个 crystal 的每个峰位和最终系数。
  // EN: Human-readable table with peak positions and final coefficients.
  std::ofstream textOut(outputTable.Data());
  textOut << "# NFS crystal energy auto-calibration\n";
  textOut << "# NFS crystal 能量自动刻度结果\n";
  textOut << "# input_file " << inputFile << "\n";
  textOut << "# source " << source.name << "\n";
  textOut << "# Columns:\n";
  textOut << "# clover crystal detector entries status usedPeaks offset gain gain2 calChi2Ndf";
  for (std::size_t i = 0; i < source.energiesKeV.size(); ++i) {
    textOut << " peak" << i << "_expected peak" << i << "_fit peak" << i << "_fwhm peak" << i << "_status";
  }
  textOut << "\n";
  textOut << std::setprecision(10);

  for (const auto &result : results) {
    textOut << result.clover << " "
            << result.crystal << " "
            << result.detector << " "
            << result.entries << " "
            << result.status << " "
            << result.usedPeaks << " "
            << result.coeff.offset << " "
            << result.coeff.gain << " "
            << result.coeff.gain2 << " "
            << result.calChi2Ndf;
    for (std::size_t i = 0; i < source.energiesKeV.size(); ++i) {
      if (i < result.peaks.size()) {
        const auto &peak = result.peaks[i];
        textOut << " " << peak.expectedKeV
                << " " << peak.meanKeV
                << " " << peak.fwhmKeV
                << " " << (peak.ok ? 1 : 0);
      } else {
        textOut << " " << source.energiesKeV[i] << " 0 0 0";
      }
    }
    textOut << "\n";
  }
  textOut.close();

  // CN: 纯 64 行能量 block，顺序为 clover0 crystal0-3, clover1 crystal0-3, ...
  // EN: Plain 64-line energy block ordered as clover0 crystal0-3, clover1 crystal0-3, ...
  std::ofstream blockOut(outputEnergyBlock.Data());
  blockOut << std::fixed << std::setprecision(8);
  for (const auto &result : results) {
    blockOut << result.coeff.offset << " "
             << result.coeff.gain << " "
             << result.coeff.gain2 << "\n";
  }
  blockOut.close();

  // CN: 候选 ecc.cal。成功拟合的 crystal 写入新能量系数；失败的 crystal 保留旧能量系数；
  //     时间 64 行完整保留旧 ecc.cal 的时间 block。
  // EN: Candidate ecc.cal. Successful crystals use new energy coefficients; failed ones keep old
  //     energy coefficients; the old 64-line time block is preserved.
  std::ofstream eccOut(outputCandidateEcc.Data());
  eccOut << std::fixed << std::setprecision(8);
  for (int i = 0; i < 64; ++i) {
    const Coefficients coeffToWrite = results[i].calibrationOk ? results[i].coeff : oldEnergyCoeff[i];
    eccOut << coeffToWrite.offset << " "
           << coeffToWrite.gain << " "
           << coeffToWrite.gain2 << "\n";
  }
  for (int i = 0; i < 64; ++i) {
    eccOut << oldTimeCoeff[i].offset << " "
           << oldTimeCoeff[i].gain << " "
           << oldTimeCoeff[i].gain2 << "\n";
  }
  eccOut.close();

  fout.cd();
  hOffset->Write();
  hGain->Write();
  hGain2->Write();
  hStatus->Write();
  table.Write();
  fout.Close();
  fin->Close();

  int okCount = 0;
  int foundCount = 0;
  for (const auto &result : results) {
    if (result.histFound) foundCount++;
    if (result.calibrationOk) okCount++;
  }

  std::cout << "Input / 输入: " << inputFile << std::endl;
  std::cout << "Source / 源: " << source.name << std::endl;
  std::cout << "Histograms found / 找到直方图: " << foundCount << "/64" << std::endl;
  std::cout << "Calibrations ok / 成功刻度: " << okCount << "/64" << std::endl;
  std::cout << "Wrote ROOT / 已写入 ROOT: " << outputRoot << std::endl;
  std::cout << "Wrote table / 已写入表格: " << outputTable << std::endl;
  std::cout << "Wrote energy block / 已写入能量 block: " << outputEnergyBlock << std::endl;
  std::cout << "Wrote ecc candidate / 已写入 ecc 候选: " << outputCandidateEcc << std::endl;
}

