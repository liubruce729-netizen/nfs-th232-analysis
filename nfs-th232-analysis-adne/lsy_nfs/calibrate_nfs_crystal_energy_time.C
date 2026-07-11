// NFS per-crystal energy and time calibration from nfs_run ROOT trees.
// 从 nfs_run ROOT tree 中提取每个 crystal 的能量和时间谱，并拟合校准参数。
//
// Usage / 用法:
//   root -l -b -q 'lsy_nfs/calibrate_nfs_crystal_energy_time.C("out/nfs_run_23_r0.root")'
//   root -l -b -q 'lsy_nfs/calibrate_nfs_crystal_energy_time.C("out/nfs_run_23_r0.root,out/nfs_run_24_r0.root","out/run23_24_cal")'
//   root -l -b -q 'lsy_nfs/calibrate_nfs_crystal_energy_time.C("@out/nfs_run_files.txt","out/run_cal",-1,"fTime")'
//
// EN: Energy spectra are rebuilt with 0.5-channel bins in [0,4096].
//     Peaks are fitted with Gaussian + linear background.
//     Time spectra use fTime by default. The prompt peak width is estimated from
//     its FWHM, then locked with a Gaussian-only fit and refitted with
//     exGaussian + linear background.
// CN: 能量谱重新按 0.5/bin、[0,4096] 建谱；峰形采用高斯 + 直线本底拟合。
//     时间谱默认使用 fTime；先用半高宽估计 prompt 峰宽，再用纯高斯锁定峰位，
//     最后用指数修正高斯 exGaussian + 直线本底联合拟合。

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "TBranch.h"
#include "TCanvas.h"
#include "TChain.h"
#include "TDirectory.h"
#include "TF1.h"
#include "TFile.h"
#include "TFitResult.h"
#include "TGraphErrors.h"
#include "TH1F.h"
#include "TLegend.h"
#include "TMath.h"
#include "TNamed.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TString.h"
#include "TStyle.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"

namespace {

constexpr int kNClover = 16;
constexpr int kNCrystalPerClover = 4;
constexpr int kNDetector = kNClover * kNCrystalPerClover;
constexpr double kTargetTimeNs = 442.0;
constexpr double kEnergyMin = 0.0;
constexpr double kEnergyMax = 4096.0;
constexpr double kEnergyBinWidth = 0.5;
constexpr double kTimeMin = 0.0;
constexpr double kTimeMax = 1600.0;
constexpr double kTimeBinWidth = 0.5;
constexpr double kDefaultEnergyFitHalfWidth = 25.0;
constexpr double kDefaultTimeFitPre = 25.0;
constexpr double kDefaultTimeFitPost = 100.0;

struct PeakRequest {
  TString label;
  double searchLow = 0.0;
  double searchHigh = 0.0;
  double trueEnergy = 0.0;
};

struct PeakFitResult {
  bool ok = false;
  int status = -999;
  double entries = 0.0;
  double seed = std::numeric_limits<double>::quiet_NaN();
  double mean = std::numeric_limits<double>::quiet_NaN();
  double meanErr = std::numeric_limits<double>::quiet_NaN();
  double sigma = std::numeric_limits<double>::quiet_NaN();
  double sigmaErr = std::numeric_limits<double>::quiet_NaN();
  double amplitude = std::numeric_limits<double>::quiet_NaN();
  double chi2ndf = std::numeric_limits<double>::quiet_NaN();
};

struct CalibrationResult {
  bool ok = false;
  int nPoints = 0;
  int status = -999;
  double offset = std::numeric_limits<double>::quiet_NaN();
  double offsetErr = std::numeric_limits<double>::quiet_NaN();
  double gain = std::numeric_limits<double>::quiet_NaN();
  double gainErr = std::numeric_limits<double>::quiet_NaN();
  double chi2ndf = std::numeric_limits<double>::quiet_NaN();
};

Double_t EnergyFitFunction(Double_t *x, Double_t *p)
{
  return p[0] * TMath::Gaus(x[0], p[1], p[2], false) + p[3] + p[4] * x[0];
}

double ExGaussianPdf(double xx, double mu, double sigma, double tau)
{
  // EN: Exponentially modified Gaussian PDF: a Gaussian timing resolution
  //     convolved with a one-sided exponential tail.
  // CN: 指数修正高斯 PDF：高斯时间分辨与单侧指数拖尾卷积。
  if (sigma <= 0.0 || tau <= 0.0) return 0.0;
  const double sqrt2 = TMath::Sqrt(2.0);
  const double arg = (mu + sigma * sigma / tau - xx) / (sqrt2 * sigma);
  const double expo = (mu - xx) / tau + sigma * sigma / (2.0 * tau * tau);
  if (expo > 700.0) return 0.0;
  const double value = 0.5 / tau * TMath::Exp(std::max(-700.0, expo)) * TMath::Erfc(arg);
  return std::isfinite(value) ? value : 0.0;
}

Double_t TimeFitFunction(Double_t *x, Double_t *p)
{
  const double xx = x[0];
  const double mu = p[1];
  double value = p[4] + p[5] * (xx - mu);
  value += p[0] * ExGaussianPdf(xx, mu, p[2], p[3]);
  return value;
}

double ExGaussianMode(double mu, double sigma, double tau, double low, double high)
{
  // EN: The fitted time peak used for calibration is the mode of the
  //     exGaussian component, not its mu parameter.
  // CN: 用于刻度的时间峰位是 exGaussian 组件的峰顶，而不是参数 mu。
  if (high <= low || sigma <= 0.0 || tau <= 0.0) return mu;
  double a = std::max(low, mu - 6.0 * sigma);
  double b = std::min(high, mu + std::max(10.0 * sigma, 10.0 * tau));
  if (b <= a) {
    a = low;
    b = high;
  }

  const double gr = 0.5 * (TMath::Sqrt(5.0) - 1.0);
  double c = b - gr * (b - a);
  double d = a + gr * (b - a);
  for (int i = 0; i < 100; ++i) {
    if (ExGaussianPdf(c, mu, sigma, tau) < ExGaussianPdf(d, mu, sigma, tau)) {
      a = c;
      c = d;
      d = a + gr * (b - a);
    } else {
      b = d;
      d = c;
      c = b - gr * (b - a);
    }
  }
  return 0.5 * (a + b);
}

Double_t TimeGaussianFunction(Double_t *x, Double_t *p)
{
  return p[0] * TMath::Gaus(x[0], p[1], p[2], false);
}

std::vector<TString> ReadFileList(const TString &listPath)
{
  std::vector<TString> items;
  std::ifstream input(listPath.Data());
  if (!input.is_open()) {
    std::cerr << "Cannot open input list: " << listPath << std::endl;
    return items;
  }
  std::string line;
  while (std::getline(input, line)) {
    TString item(line.c_str());
    item = item.Strip(TString::kBoth);
    if (item.IsNull() || item.BeginsWith("#") || item.BeginsWith("//")) continue;
    items.push_back(item);
  }
  return items;
}

std::vector<TString> SplitInputs(const char *text)
{
  std::vector<TString> items;
  TString input(text ? text : "");
  input = input.Strip(TString::kBoth);
  if (input.BeginsWith("@")) {
    TString listPath = input(1, input.Length() - 1);
    listPath = listPath.Strip(TString::kBoth);
    return ReadFileList(listPath);
  }

  TObjArray *tokens = input.Tokenize(",");
  for (int i = 0; i < tokens->GetEntriesFast(); ++i) {
    TString item = static_cast<TObjString *>(tokens->At(i))->GetString();
    item = item.Strip(TString::kBoth);
    if (!item.IsNull()) items.push_back(item);
  }
  delete tokens;
  return items;
}

TString ResolveBranch(TTree *tree, const char *leafName)
{
  std::vector<TString> candidates;
  candidates.emplace_back(leafName);
  candidates.emplace_back(TString::Format("Exogam2.%s", leafName));
  candidates.emplace_back(TString::Format("Exogam2_%s", leafName));
  candidates.emplace_back(TString::Format("Exogam2/%s", leafName));

  for (const auto &candidate : candidates) {
    if (tree->GetBranch(candidate)) return candidate;
  }

  TObjArray *branches = tree->GetListOfBranches();
  for (int i = 0; i < branches->GetEntriesFast(); ++i) {
    auto *branch = static_cast<TBranch *>(branches->At(i));
    TString name = branch->GetName();
    if (name.EndsWith(leafName)) return name;
  }
  return "";
}

TString BuildDefaultPrefix(const char *inputFiles)
{
  auto inputs = SplitInputs(inputFiles);
  TString first = inputs.empty() ? "nfs_crystal_calibration" : inputs.front();
  if (first.Contains("*") || first.Contains("?")) return "nfs_crystal_calibration";
  Ssiz_t slash = first.Last('/');
  TString directory = "";
  TString basename = first;
  if (slash != kNPOS) {
    directory = first(0, slash + 1);
    basename = first(slash + 1, first.Length() - slash - 1);
  }
  if (basename.EndsWith(".root")) basename.Remove(basename.Length() - 5);
  return directory + basename + "_crystal_energy_time_calibration";
}

int FindMaxBinInRange(TH1 *hist, double low, double high)
{
  if (!hist) return -1;
  int b1 = hist->GetXaxis()->FindBin(low);
  int b2 = hist->GetXaxis()->FindBin(high);
  b1 = std::max(1, b1);
  b2 = std::min(hist->GetNbinsX(), b2);
  if (b2 < b1) return -1;

  int bestBin = -1;
  double best = -1.0;
  for (int b = b1; b <= b2; ++b) {
    const double y = hist->GetBinContent(b);
    if (y > best) {
      best = y;
      bestBin = b;
    }
  }
  if (best <= 0.0) return -1;
  return bestBin;
}

double EstimatePeakFwhm(TH1 *hist, int seedBin, double searchLow, double searchHigh)
{
  // EN: Rough FWHM from half-height crossings around the seed maximum.
  // CN: 从峰顶向两侧找半高点，得到粗略 FWHM，用于决定时间拟合窗口。
  if (!hist || seedBin < 1 || seedBin > hist->GetNbinsX()) return 6.0;

  const double seed = hist->GetBinCenter(seedBin);
  const double peak = hist->GetBinContent(seedBin);
  if (peak <= 0.0) return 6.0;

  const int lowBin = std::max(1, hist->GetXaxis()->FindBin(searchLow));
  const int highBin = std::min(hist->GetNbinsX(), hist->GetXaxis()->FindBin(searchHigh));
  const double edgeLow = hist->GetBinContent(lowBin);
  const double edgeHigh = hist->GetBinContent(highBin);
  const double baseline = std::max(0.0, std::min(edgeLow, edgeHigh));
  const double half = baseline + 0.5 * (peak - baseline);

  auto interpolateCrossing = [&](int binNear, int binFar) {
    const double x1 = hist->GetBinCenter(binNear);
    const double y1 = hist->GetBinContent(binNear);
    const double x2 = hist->GetBinCenter(binFar);
    const double y2 = hist->GetBinContent(binFar);
    if (std::fabs(y2 - y1) < 1e-12) return 0.5 * (x1 + x2);
    const double fraction = (half - y1) / (y2 - y1);
    return x1 + fraction * (x2 - x1);
  };

  double left = std::numeric_limits<double>::quiet_NaN();
  for (int b = seedBin; b > lowBin; --b) {
    if (hist->GetBinContent(b) >= half && hist->GetBinContent(b - 1) < half) {
      left = interpolateCrossing(b, b - 1);
      break;
    }
  }

  double right = std::numeric_limits<double>::quiet_NaN();
  for (int b = seedBin; b < highBin; ++b) {
    if (hist->GetBinContent(b) >= half && hist->GetBinContent(b + 1) < half) {
      right = interpolateCrossing(b, b + 1);
      break;
    }
  }

  if (!std::isfinite(left)) left = seed - 6.0;
  if (!std::isfinite(right)) right = seed + 6.0;
  const double fwhm = right - left;
  if (!std::isfinite(fwhm) || fwhm <= 0.0) return 6.0;
  return std::min(120.0, std::max(2.0, fwhm));
}

double AverageBinContentInRange(TH1 *hist, double low, double high)
{
  // EN: Average the bin contents in a stable side-band. For the time fit this
  //     gives the first estimate of the linear background level.
  // CN: 对稳定旁带区间求平均。时间拟合里用它作为直线背景的初始水平。
  if (!hist || high <= low) return 0.0;
  const int b1 = std::max(1, hist->GetXaxis()->FindBin(low));
  const int b2 = std::min(hist->GetNbinsX(), hist->GetXaxis()->FindBin(high));
  if (b2 < b1) return 0.0;

  double sum = 0.0;
  int n = 0;
  for (int b = b1; b <= b2; ++b) {
    const double center = hist->GetBinCenter(b);
    if (center < low || center > high) continue;
    sum += hist->GetBinContent(b);
    ++n;
  }
  if (n <= 0) return 0.0;
  return sum / static_cast<double>(n);
}

PeakFitResult FitEnergyPeak(TH1F *hist, const PeakRequest &request, int detector, double fitHalfWidth)
{
  PeakFitResult result;
  if (!hist || hist->GetEntries() <= 0.0) return result;
  result.entries = hist->GetEntries();

  const int seedBin = FindMaxBinInRange(hist, request.searchLow, request.searchHigh);
  if (seedBin < 0) return result;
  const double seed = hist->GetBinCenter(seedBin);
  const double seedContent = hist->GetBinContent(seedBin);
  result.seed = seed;

  const double fitLow = std::max(request.searchLow, seed - fitHalfWidth);
  const double fitHigh = std::min(request.searchHigh, seed + fitHalfWidth);
  if (fitHigh <= fitLow) return result;

  const double leftBg = hist->GetBinContent(hist->GetXaxis()->FindBin(fitLow));
  const double rightBg = hist->GetBinContent(hist->GetXaxis()->FindBin(fitHigh));
  const double bg = 0.5 * (leftBg + rightBg);
  const double amp = std::max(1.0, seedContent - bg);

  TF1 *fit = new TF1(TString::Format("fit_energy_%s_det%d", request.label.Data(), detector),
                     EnergyFitFunction, fitLow, fitHigh, 5);
  fit->SetParNames("Amp", "Mean", "Sigma", "Bg0", "BgSlope");
  fit->SetParameters(amp, seed, 3.0, bg, 0.0);
  fit->SetParLimits(0, 0.0, std::max(amp * 20.0, seedContent * 20.0 + 10.0));
  fit->SetParLimits(1, fitLow, fitHigh);
  fit->SetParLimits(2, 0.15, fitHalfWidth);
  fit->SetLineColor(kRed + detector % 4);

  TFitResultPtr fitResult = hist->Fit(fit, "QRS0+");
  result.status = int(fitResult);
  result.mean = fit->GetParameter(1);
  result.meanErr = fit->GetParError(1);
  result.sigma = std::fabs(fit->GetParameter(2));
  result.sigmaErr = fit->GetParError(2);
  result.amplitude = fit->GetParameter(0);
  const int ndf = fit->GetNDF();
  if (ndf > 0) result.chi2ndf = fit->GetChisquare() / ndf;
  result.ok = std::isfinite(result.mean) && result.mean >= request.searchLow && result.mean <= request.searchHigh && result.sigma > 0.0;
  return result;
}

PeakFitResult FitTimePeak(TH1F *hist, int detector, double searchHigh, double fitPre, double fitPost)
{
  PeakFitResult result;
  if (!hist || hist->GetEntries() <= 0.0) return result;
  result.entries = hist->GetEntries();

  const int seedBin = FindMaxBinInRange(hist, kTimeMin, searchHigh);
  if (seedBin < 0) return result;
  const double seed = hist->GetBinCenter(seedBin);
  const double seedContent = hist->GetBinContent(seedBin);
  result.seed = seed;

  // EN: Estimate a rough FWHM first. The Gaussian seed window follows the
  //     observed peak width instead of a fixed narrow value.
  // CN: 先根据半高点估计粗略 FWHM；高斯 seed 窗口跟随实际峰宽，而不是固定窄窗口。
  const double roughFwhm = EstimatePeakFwhm(hist, seedBin, kTimeMin, searchHigh);
  const double roughSigma = std::max(0.5, roughFwhm / 2.354820045);
  const double gaussianHalfWidth = std::min(120.0, std::max(4.0, roughFwhm));
  const double gaussianLow = std::max(kTimeMin, seed - gaussianHalfWidth);
  const double gaussianHigh = std::min(searchHigh, seed + gaussianHalfWidth);
  if (gaussianHigh <= gaussianLow) return result;

  TF1 *gaussianFit = new TF1(TString::Format("fit_time_gaussian_seed_det%d", detector),
                             TimeGaussianFunction, gaussianLow, gaussianHigh, 3);
  gaussianFit->SetParNames("GausAmp", "GausMean", "GausSigma");
  gaussianFit->SetParameters(std::max(1.0, seedContent), seed, roughSigma);
  gaussianFit->SetParLimits(0, std::max(0.1, seedContent * 0.05), std::max(10.0, seedContent * 5.0));
  gaussianFit->SetParLimits(1, gaussianLow, gaussianHigh);
  gaussianFit->SetParLimits(2, std::max(0.20, roughSigma * 0.50), std::max(gaussianHalfWidth, roughSigma * 3.0));
  gaussianFit->SetLineColor(kGreen + 2);
  gaussianFit->SetLineStyle(2);

  TFitResultPtr gaussianResult = hist->Fit(gaussianFit, "QRS0+");
  double lockedAmp = gaussianFit->GetParameter(0);
  double lockedMean = gaussianFit->GetParameter(1);
  double lockedSigma = std::fabs(gaussianFit->GetParameter(2));
  double lockedFwhm = 2.354820045 * lockedSigma;
  if (int(gaussianResult) != 0 || !std::isfinite(lockedMean) ||
      lockedMean < gaussianLow || lockedMean > gaussianHigh ||
      !std::isfinite(lockedSigma) || lockedSigma <= 0.0) {
    lockedAmp = std::max(1.0, seedContent);
    lockedMean = seed;
    lockedSigma = std::max(roughFwhm / 2.354820045, 1.0);
    lockedFwhm = 2.354820045 * lockedSigma;
  }

  const double leftWidth = std::min(250.0, std::max({fitPre * 3.0, 1.5 * roughFwhm, 3.0 * lockedSigma, 40.0}));
  const double rightWidth = std::min(500.0, std::max({fitPost * 3.0, 4.0 * roughFwhm, 5.0 * lockedSigma, 120.0}));
  const double fitLow = std::max(kTimeMin, lockedMean - leftWidth);
  const double fitHigh = std::min(kTimeMax, lockedMean + rightWidth);
  if (fitHigh <= fitLow) return result;

  const int lateLowBin = hist->GetXaxis()->FindBin(std::min(fitHigh, lockedMean + std::max(10.0, lockedSigma * 4.0)));
  const int lateHighBin = hist->GetXaxis()->FindBin(fitHigh);
  double lateLevel = 0.0;
  int lateBins = 0;
  for (int b = std::max(1, lateLowBin); b <= std::min(hist->GetNbinsX(), lateHighBin); ++b) {
    const double y = hist->GetBinContent(b);
    if (y > seedContent * 10.0) continue;
    lateLevel += y;
    ++lateBins;
  }
  if (lateBins > 0) lateLevel /= static_cast<double>(lateBins);

  TF1 *fit = new TF1(TString::Format("fit_time_det%d", detector), TimeFitFunction, fitLow, fitHigh, 6);
  fit->SetParNames("Area", "Mu", "Sigma", "Tau", "Bg0", "BgSlope");

  const double sideBandBg = AverageBinContentInRange(hist, 200.0, 250.0);
  const double bgScale = std::max({1.0, sideBandBg, lateLevel});
  const double bgHigh = std::max(10.0, bgScale * 20.0);
  const double slopeLimit = std::max(0.02, bgScale / 40.0);
  const double initialBg = std::min(bgHigh * 0.5, std::max(0.0, sideBandBg));
  const double initialMu = std::max(fitLow, std::min(lockedMean - 0.35 * roughFwhm, fitHigh));
  const double initialSigma = std::max(0.5, std::min(120.0, std::max(lockedSigma, roughSigma)));
  const double initialTau = std::max(2.0, std::min(500.0, roughFwhm * 0.7));
  const double initialArea = std::max(10.0, (seedContent - initialBg) * std::max(10.0, initialSigma + initialTau) * 2.0);

  const double muLow = std::max(kTimeMin, lockedMean - std::max(15.0, 1.2 * roughFwhm));
  const double muHigh = std::min(searchHigh, lockedMean + std::max(10.0, 0.5 * roughFwhm));
  const double sigmaLow = std::max(0.20, roughSigma * 0.35);
  const double sigmaHigh = std::min(150.0, std::max({10.0, roughSigma * 2.5, lockedSigma * 2.5}));
  const double tauLow = 0.5;
  const double tauHigh = std::min(2000.0, std::max({30.0, roughFwhm * 10.0, rightWidth * 2.0}));
  const double areaLow = std::max(1.0, initialArea * 0.02);
  const double areaHigh = std::max(initialArea * 50.0, seedContent * std::max(roughFwhm, 10.0) * 100.0);

  fit->SetParameters(
      std::min(areaHigh * 0.5, std::max(areaLow * 2.0, initialArea)),
      std::min(muHigh, std::max(muLow, initialMu)),
      std::min(sigmaHigh * 0.8, std::max(sigmaLow * 1.2, initialSigma)),
      std::min(tauHigh * 0.5, std::max(tauLow * 2.0, initialTau)),
      initialBg,
      0.0);

  fit->SetParLimits(0, areaLow, areaHigh);
  fit->SetParLimits(1, muLow, muHigh);
  fit->SetParLimits(2, sigmaLow, sigmaHigh);
  fit->SetParLimits(3, tauLow, tauHigh);
  // EN: Linear background. Bg0 is seeded from the 200-250 ns side-band.
  // CN: 直线背景。Bg0 初值来自 200-250 ns 旁带平均计数。
  fit->SetParLimits(4, 0.0, bgHigh);
  fit->SetParLimits(5, -slopeLimit, slopeLimit);
  fit->SetLineColor(kBlue + detector % 4);

  TFitResultPtr fitResult = hist->Fit(fit, "QRS0+");
  result.status = int(fitResult);
  result.mean = ExGaussianMode(fit->GetParameter(1), std::fabs(fit->GetParameter(2)), fit->GetParameter(3), fitLow, fitHigh);
  result.meanErr = fit->GetParError(1);
  result.sigma = std::fabs(fit->GetParameter(2));
  result.sigmaErr = fit->GetParError(2);
  result.amplitude = fit->GetParameter(0);
  const int ndf = fit->GetNDF();
  if (ndf > 0) result.chi2ndf = fit->GetChisquare() / ndf;
  result.ok = std::isfinite(result.mean) && result.mean >= kTimeMin &&
              result.mean <= searchHigh + kTimeBinWidth && result.sigma > 0.0;
  return result;
}

CalibrationResult FitEnergyCalibration(const std::vector<PeakRequest> &requests,
                                       const std::vector<PeakFitResult> &fits)
{
  CalibrationResult result;
  std::vector<double> x;
  std::vector<double> y;
  std::vector<double> ex;
  std::vector<double> ey;
  for (std::size_t i = 0; i < requests.size(); ++i) {
    if (!fits[i].ok || !std::isfinite(fits[i].mean)) continue;
    x.push_back(fits[i].mean);
    y.push_back(requests[i].trueEnergy);
    ex.push_back(std::isfinite(fits[i].meanErr) ? fits[i].meanErr : 0.0);
    ey.push_back(0.0);
  }
  result.nPoints = static_cast<int>(x.size());
  if (x.size() < 2) return result;

  TGraphErrors graph(x.size(), x.data(), y.data(), ex.data(), ey.data());
  TF1 line("energy_linear_calibration", "[0]+[1]*x", 0.0, kEnergyMax);
  line.SetParameters(0.0, 1.0);
  TFitResultPtr fitResult = graph.Fit(&line, "QS0");
  result.status = int(fitResult);
  result.offset = line.GetParameter(0);
  result.offsetErr = line.GetParError(0);
  result.gain = line.GetParameter(1);
  result.gainErr = line.GetParError(1);
  const int ndf = line.GetNDF();
  if (ndf > 0) result.chi2ndf = line.GetChisquare() / ndf;
  result.ok = std::isfinite(result.offset) && std::isfinite(result.gain) && result.gain > 0.0;
  return result;
}

void DrawCrystalCanvas(TDirectory *dir, int det, TH1F *energy, TH1F *time,
                       const std::vector<PeakRequest> &requests,
                       const std::vector<PeakFitResult> &energyFits,
                       const PeakFitResult &timeFit)
{
  if (!dir) return;
  dir->cd();
  const int clo = det / 4;
  const int cri = det % 4;
  TCanvas *canvas = new TCanvas(TString::Format("c_crystal_cal_clover%d_crystal%d", clo, cri),
                                TString::Format("Clover%d Crystal%d calibration", clo, cri),
                                1400, 900);
  canvas->Divide(2, 2);

  auto drawEnergyPanel = [&](int pad, double low, double high, int selectedPeak) {
    canvas->cd(pad);
    if (!energy) return;

    // EN: Each pad gets an independent clone. Otherwise ROOT stores the same
    //     histogram object in all pads and the final SetRangeUser wins.
    // CN: 每个 pad 画独立 clone；否则 ROOT 会让所有 pad 共享同一个 histogram，
    //     最后一次 SetRangeUser 会覆盖前面三个窗口。
    TH1F *copy = static_cast<TH1F *>(energy->Clone(TString::Format("%s_pad%d_det%d", energy->GetName(), pad, det)));
    copy->SetDirectory(nullptr);
    copy->GetListOfFunctions()->Clear();
    if (high > low) copy->GetXaxis()->SetRangeUser(low, high);
    copy->Draw("hist");

    TLegend *legend = new TLegend(0.55, 0.72, 0.90, 0.90);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);

    if (selectedPeak >= 0 && selectedPeak < static_cast<int>(requests.size())) {
      const auto &req = requests[selectedPeak];
      legend->AddEntry(copy, req.label, "l");
      TF1 *fit = energy->GetFunction(TString::Format("fit_energy_%s_det%d", req.label.Data(), det));
      if (fit) fit->DrawCopy("same");
      if (selectedPeak < static_cast<int>(energyFits.size()) && energyFits[selectedPeak].ok) {
        legend->AddEntry((TObject *)nullptr, TString::Format("mean %.3f", energyFits[selectedPeak].mean), "");
      }
    }
    else {
      legend->AddEntry(copy, "Raw energy", "l");
      for (std::size_t i = 0; i < requests.size(); ++i) {
        TF1 *fit = energy->GetFunction(TString::Format("fit_energy_%s_det%d", requests[i].label.Data(), det));
        if (fit) fit->DrawCopy("same");
      }
    }
    legend->Draw();
  };

  drawEnergyPanel(1, kEnergyMin, kEnergyMax, -1);
  for (std::size_t i = 0; i < requests.size() && i < 3; ++i) {
    drawEnergyPanel(static_cast<int>(i) + 2, requests[i].searchLow, requests[i].searchHigh, static_cast<int>(i));
  }
  canvas->Write();
  delete canvas;

  TCanvas *timeCanvas = new TCanvas(TString::Format("c_time_cal_clover%d_crystal%d", clo, cri),
                                    TString::Format("Clover%d Crystal%d time calibration", clo, cri),
                                    1000, 700);
  if (time) {
    TH1F *timeCopy = static_cast<TH1F *>(time->Clone(TString::Format("%s_canvas_det%d", time->GetName(), det)));
    timeCopy->SetDirectory(nullptr);
    timeCopy->GetListOfFunctions()->Clear();
    timeCopy->Draw("hist");

    TF1 *gaussianFit = time->GetFunction(TString::Format("fit_time_gaussian_seed_det%d", det));
    if (gaussianFit) gaussianFit->DrawCopy("same");
    TF1 *finalFit = time->GetFunction(TString::Format("fit_time_det%d", det));
    if (finalFit) finalFit->DrawCopy("same");

    TLegend *legend = new TLegend(0.55, 0.72, 0.90, 0.90);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->AddEntry(timeCopy, "Time spectrum", "l");
    if (gaussianFit) legend->AddEntry(gaussianFit, "Gaussian seed fit", "l");
    if (finalFit) legend->AddEntry(finalFit, "exGaussian + line", "l");
    if (timeFit.ok) {
      legend->AddEntry((TObject *)nullptr, TString::Format("peak %.3f ns", timeFit.mean), "");
      legend->AddEntry((TObject *)nullptr, TString::Format("offset %.3f ns", kTargetTimeNs - timeFit.mean), "");
    }
    legend->Draw();
  }
  timeCanvas->Write();
  delete timeCanvas;
}

void WriteDetailedText(const TString &path,
                       const std::vector<PeakRequest> &requests,
                       const std::vector<std::vector<PeakFitResult>> &energyFits,
                       const std::vector<CalibrationResult> &energyCal,
                       const std::vector<PeakFitResult> &timeFits)
{
  std::ofstream out(path.Data());
  out << "# NFS crystal energy/time calibration summary\n";
  out << "# Energy calibration relation: E_cal_keV = energy_offset + energy_gain * E_raw\n";
  out << "# Time suggestions: time_offset_to_442_ns = 442 - fitted_time_peak_ns; time_gain_to_442 = 442 / fitted_time_peak_ns\n";
  out << "# clover crystal detector energy_entries";
  for (const auto &req : requests) {
    out << " " << req.label << "_raw " << req.label << "_raw_err " << req.label << "_sigma " << req.label << "_status";
  }
  out << " energy_offset energy_gain energy_gain2 energy_fit_points energy_fit_status energy_chi2_ndf";
  out << " time_entries time_peak_ns time_peak_err_ns time_sigma_ns time_fwhm_ns time_fit_status time_chi2_ndf time_offset_to_442_ns time_gain_to_442\n";
  out << std::setprecision(10);

  for (int det = 0; det < kNDetector; ++det) {
    const int clo = det / 4;
    const int cri = det % 4;
    out << clo << " " << cri << " " << det;
    const double energyEntries = energyFits[det].empty() ? 0.0 : energyFits[det][0].entries;
    out << " " << energyEntries;
    for (std::size_t i = 0; i < requests.size(); ++i) {
      const auto &fit = energyFits[det][i];
      out << " " << fit.mean << " " << fit.meanErr << " " << fit.sigma << " " << fit.status;
    }
    const auto &cal = energyCal[det];
    out << " " << cal.offset << " " << cal.gain << " 0";
    out << " " << cal.nPoints << " " << cal.status << " " << cal.chi2ndf;
    const auto &tf = timeFits[det];
    const double fwhm = tf.sigma * 2.354820045;
    const double timeOffset = tf.ok ? (kTargetTimeNs - tf.mean) : std::numeric_limits<double>::quiet_NaN();
    const double timeGain = (tf.ok && tf.mean > 0.0) ? (kTargetTimeNs / tf.mean) : std::numeric_limits<double>::quiet_NaN();
    out << " " << tf.entries << " " << tf.mean << " " << tf.meanErr << " " << tf.sigma << " " << fwhm;
    out << " " << tf.status << " " << tf.chi2ndf << " " << timeOffset << " " << timeGain << "\n";
  }
}

void WriteEccCandidate(const TString &path,
                       const std::vector<CalibrationResult> &energyCal,
                       const std::vector<PeakFitResult> &timeFits)
{
  std::ofstream out(path.Data());
  out << "# Candidate ecc.cal coefficients from calibrate_nfs_crystal_energy_time.C\n";
  out << "# First 64 lines: energy offset gain gain2. Next 64 lines: time offset gain gain2.\n";
  out << "# Time block uses offset-only correction to place the fitted time peak at 442 ns; gain is kept at 1.\n";
  out << std::setprecision(10);
  for (int det = 0; det < kNDetector; ++det) {
    if (energyCal[det].ok) out << energyCal[det].offset << " " << energyCal[det].gain << " 0\n";
    else out << "0 1 0\n";
  }
  for (int det = 0; det < kNDetector; ++det) {
    if (timeFits[det].ok) out << (kTargetTimeNs - timeFits[det].mean) << " 1 0\n";
    else out << "0 1 0\n";
  }
}

} // namespace

void calibrate_nfs_crystal_energy_time(const char *inputFiles,
                                       const char *outputPrefix = "",
                                       Long64_t maxEntries = -1,
                                       const char *timeBranchLeaf = "fTime",
                                       double energyFitHalfWidth = kDefaultEnergyFitHalfWidth,
                                       double timeSearchHighNs = 800.0)
{
  if (!inputFiles || TString(inputFiles).Strip(TString::kBoth).IsNull()) {
    std::cerr << "No input ROOT file was provided." << std::endl;
    return;
  }

  std::vector<PeakRequest> peakRequests = {
      {"peak511", 440.0, 560.0, 511.0},
      {"peak911", 850.0, 950.0, 911.2},
      {"peak2614", 2400.0, 2800.0, 2614.511},
  };

  const TString prefix = (outputPrefix && TString(outputPrefix).Length() > 0)
                             ? TString(outputPrefix)
                             : BuildDefaultPrefix(inputFiles);
  const TString rootOutPath = prefix + ".root";
  const TString txtOutPath = prefix + ".txt";
  const TString eccOutPath = prefix + "_ecc_cal_candidate.txt";

  TChain chain("TreeMaster");
  int nAdded = 0;
  for (const auto &input : SplitInputs(inputFiles)) {
    nAdded += chain.Add(input);
  }
  if (nAdded <= 0 || chain.GetEntries() <= 0) {
    std::cerr << "No TreeMaster entries found in input: " << inputFiles << std::endl;
    return;
  }

  const TString cloverBranch = ResolveBranch(&chain, "fEXO_ECC_E_Clover");
  const TString crystalBranch = ResolveBranch(&chain, "fEXO_ECC_E_Cristal");
  const TString energyBranch = ResolveBranch(&chain, "fEXO_ECC_E_Energy");
  const TString timeBranch = ResolveBranch(&chain, timeBranchLeaf);

  if (cloverBranch.IsNull() || crystalBranch.IsNull() || energyBranch.IsNull()) {
    std::cerr << "Cannot find fEXO_ECC_E_Clover/Cristal/Energy branches in TreeMaster." << std::endl;
    return;
  }
  if (timeBranch.IsNull()) {
    std::cerr << "Cannot find requested time branch: " << timeBranchLeaf << std::endl;
    return;
  }

  const int energyBins = static_cast<int>((kEnergyMax - kEnergyMin) / kEnergyBinWidth);
  const int timeBins = static_cast<int>((kTimeMax - kTimeMin) / kTimeBinWidth);
  std::vector<TH1F *> energyHists(kNDetector, nullptr);
  std::vector<TH1F *> timeHists(kNDetector, nullptr);

  for (int det = 0; det < kNDetector; ++det) {
    const int clo = det / 4;
    const int cri = det % 4;
    energyHists[det] = new TH1F(TString::Format("cal_clover%d_crystal%d_energy_raw", clo, cri),
                                TString::Format("Clover%d Crystal%d raw energy;Raw energy;Counts", clo, cri),
                                energyBins, kEnergyMin, kEnergyMax);
    timeHists[det] = new TH1F(TString::Format("cal_clover%d_crystal%d_time", clo, cri),
                              TString::Format("Clover%d Crystal%d %s;Time (ns);Counts", clo, cri, timeBranchLeaf),
                              timeBins, kTimeMin, kTimeMax);
  }

  TTreeReader reader(&chain);
  TTreeReaderArray<UShort_t> clovers(reader, cloverBranch.Data());
  TTreeReaderArray<UShort_t> crystals(reader, crystalBranch.Data());
  TTreeReaderArray<float> energies(reader, energyBranch.Data());
  TTreeReaderArray<float> times(reader, timeBranch.Data());

  Long64_t totalEntries = chain.GetEntries();
  if (maxEntries > 0 && maxEntries < totalEntries) totalEntries = maxEntries;

  Long64_t entry = 0;
  Long64_t filledEnergy = 0;
  Long64_t filledTime = 0;
  while (reader.Next()) {
    if (entry >= totalEntries) break;
    entry++;
    const std::size_t n = std::min({static_cast<std::size_t>(clovers.GetSize()),
                                    static_cast<std::size_t>(crystals.GetSize()),
                                    static_cast<std::size_t>(energies.GetSize())});
    const std::size_t nt = std::min(n, static_cast<std::size_t>(times.GetSize()));
    for (std::size_t i = 0; i < n; ++i) {
      const int clo = clovers[i];
      const int cri = crystals[i];
      if (clo < 0 || clo >= kNClover || cri < 0 || cri >= kNCrystalPerClover) continue;
      const int det = clo * 4 + cri;
      const float energy = energies[i];
      if (energy > kEnergyMin && energy < kEnergyMax) {
        energyHists[det]->Fill(energy);
        filledEnergy++;
      }
      if (i < nt) {
        const float time = times[i];
        if (time > kTimeMin && time < kTimeMax) {
          timeHists[det]->Fill(time);
          filledTime++;
        }
      }
    }
  }

  std::vector<std::vector<PeakFitResult>> energyFits(kNDetector, std::vector<PeakFitResult>(peakRequests.size()));
  std::vector<CalibrationResult> energyCal(kNDetector);
  std::vector<PeakFitResult> timeFits(kNDetector);

  TFile out(rootOutPath, "RECREATE");
  if (!out.IsOpen()) {
    std::cerr << "Cannot create output file: " << rootOutPath << std::endl;
    return;
  }
  TDirectory *spectraDir = out.mkdir("spectra");
  TDirectory *canvasDir = out.mkdir("fit_canvases");
  TDirectory *summaryDir = out.mkdir("summary");

  for (int det = 0; det < kNDetector; ++det) {
    for (std::size_t i = 0; i < peakRequests.size(); ++i) {
      energyFits[det][i] = FitEnergyPeak(energyHists[det], peakRequests[i], det, energyFitHalfWidth);
    }
    energyCal[det] = FitEnergyCalibration(peakRequests, energyFits[det]);
    timeFits[det] = FitTimePeak(timeHists[det], det, timeSearchHighNs, kDefaultTimeFitPre, kDefaultTimeFitPost);

    spectraDir->cd();
    energyHists[det]->Write();
    timeHists[det]->Write();
    DrawCrystalCanvas(canvasDir, det, energyHists[det], timeHists[det], peakRequests, energyFits[det], timeFits[det]);
  }

  summaryDir->cd();
  const TString configTitle = TString::Format("input=%s; entries=%lld; time_branch=%s; energy_bin_width=%.3f; time_target_ns=%.3f; energy_fit_half_width=%.3f; time_search_high_ns=%.3f",
                                             inputFiles, totalEntries, timeBranch.Data(), kEnergyBinWidth,
                                             kTargetTimeNs, energyFitHalfWidth, timeSearchHighNs);
  TNamed config("CalibrationConfig", configTitle.Data());
  config.Write();
  TH1F summary("CalibrationSummary", "CalibrationSummary;Category;Counts", 5, 0.5, 5.5);
  summary.GetXaxis()->SetBinLabel(1, "Input entries");
  summary.GetXaxis()->SetBinLabel(2, "Filled energy");
  summary.GetXaxis()->SetBinLabel(3, "Filled time");
  summary.GetXaxis()->SetBinLabel(4, "Energy cal ok");
  summary.GetXaxis()->SetBinLabel(5, "Time fit ok");
  summary.SetBinContent(1, totalEntries);
  summary.SetBinContent(2, filledEnergy);
  summary.SetBinContent(3, filledTime);
  int energyOk = 0;
  int timeOk = 0;
  for (int det = 0; det < kNDetector; ++det) {
    if (energyCal[det].ok && energyCal[det].nPoints >= 3) energyOk++;
    if (timeFits[det].ok) timeOk++;
  }
  summary.SetBinContent(4, energyOk);
  summary.SetBinContent(5, timeOk);
  summary.Write();
  out.Close();

  WriteDetailedText(txtOutPath, peakRequests, energyFits, energyCal, timeFits);
  WriteEccCandidate(eccOutPath, energyCal, timeFits);

  std::cout << "Input files: " << inputFiles << std::endl;
  std::cout << "Tree entries processed: " << totalEntries << std::endl;
  std::cout << "Filled energy counts: " << filledEnergy << std::endl;
  std::cout << "Filled time counts: " << filledTime << std::endl;
  std::cout << "Energy calibrations with 3 peaks: " << energyOk << "/" << kNDetector << std::endl;
  std::cout << "Time fits: " << timeOk << "/" << kNDetector << std::endl;
  std::cout << "Output ROOT: " << rootOutPath << std::endl;
  std::cout << "Output text: " << txtOutPath << std::endl;
  std::cout << "ECC candidate: " << eccOutPath << std::endl;
}
