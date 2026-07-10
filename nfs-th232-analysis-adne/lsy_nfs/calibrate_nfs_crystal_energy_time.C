// NFS per-crystal energy and time calibration from nfs_run ROOT trees.
// 从 nfs_run ROOT tree 中提取每个 crystal 的能量和时间谱，并拟合校准参数。
//
// Usage / 用法:
//   root -l -b -q 'lsy_nfs/calibrate_nfs_crystal_energy_time.C("out/nfs_run_23_r0.root")'
//   root -l -b -q 'lsy_nfs/calibrate_nfs_crystal_energy_time.C("out/nfs_run_23_r0.root,out/nfs_run_24_r0.root","out/run23_24_cal")'
//   root -l -b -q 'lsy_nfs/calibrate_nfs_crystal_energy_time.C("@out/nfs_run_files.txt","out/run_cal",-1,"fDeltaT")'
//
// EN: Energy spectra are rebuilt with 0.5-channel bins in [0,4096].
//     Peaks are fitted with Gaussian + linear background.
//     Time spectra use fDeltaT by default and fit the strongest peak below 800 ns with Gaussian + exponential tail.
// CN: 能量谱重新按 0.5/bin、[0,4096] 建谱；峰形采用高斯 + 直线本底拟合。
//     时间谱默认使用 fDeltaT，在 800 ns 以内找最高峰，采用高斯 + 指数尾部拟合。

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

Double_t TimeFitFunction(Double_t *x, Double_t *p)
{
  const double xx = x[0];
  double value = p[0] * TMath::Gaus(xx, p[1], p[2], false);
  if (xx >= p[1] && p[4] > 0.0) value += p[3] * TMath::Exp(-(xx - p[1]) / p[4]);
  return value;
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

  TFitResultPtr fitResult = hist->Fit(fit, "QRS0");
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

  const double fitLow = std::max(kTimeMin, seed - fitPre);
  const double fitHigh = std::min(kTimeMax, seed + fitPost);
  if (fitHigh <= fitLow) return result;

  TF1 *fit = new TF1(TString::Format("fit_time_det%d", detector), TimeFitFunction, fitLow, fitHigh, 5);
  fit->SetParNames("Amp", "Mean", "Sigma", "TailAmp", "Tau");
  fit->SetParameters(std::max(1.0, seedContent), seed, 4.0, std::max(0.1, seedContent * 0.2), 30.0);
  fit->SetParLimits(0, 0.0, std::max(10.0, seedContent * 50.0));
  fit->SetParLimits(1, std::max(kTimeMin, seed - fitPre), std::min(searchHigh, seed + fitPre));
  fit->SetParLimits(2, 0.1, 40.0);
  fit->SetParLimits(3, 0.0, std::max(10.0, seedContent * 50.0));
  fit->SetParLimits(4, 1.0, 500.0);
  fit->SetLineColor(kBlue + detector % 4);

  TFitResultPtr fitResult = hist->Fit(fit, "QRS0");
  result.status = int(fitResult);
  result.mean = fit->GetParameter(1);
  result.meanErr = fit->GetParError(1);
  result.sigma = std::fabs(fit->GetParameter(2));
  result.sigmaErr = fit->GetParError(2);
  result.amplitude = fit->GetParameter(0);
  const int ndf = fit->GetNDF();
  if (ndf > 0) result.chi2ndf = fit->GetChisquare() / ndf;
  result.ok = std::isfinite(result.mean) && result.mean >= kTimeMin && result.mean <= searchHigh && result.sigma > 0.0;
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

  canvas->cd(1);
  if (energy) {
    energy->Draw("hist");
    for (const auto &obj : *energy->GetListOfFunctions()) obj->Draw("same");
  }

  for (std::size_t i = 0; i < requests.size() && i < 3; ++i) {
    canvas->cd(static_cast<int>(i) + 2);
    if (!energy) continue;
    energy->GetXaxis()->SetRangeUser(requests[i].searchLow, requests[i].searchHigh);
    energy->Draw("hist");
    for (const auto &obj : *energy->GetListOfFunctions()) obj->Draw("same");
    TLegend *legend = new TLegend(0.55, 0.72, 0.90, 0.90);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->AddEntry(energy, requests[i].label, "l");
    if (energyFits[i].ok) legend->AddEntry((TObject *)nullptr, TString::Format("mean %.3f", energyFits[i].mean), "");
    legend->Draw();
  }
  canvas->Write();
  if (energy) energy->GetXaxis()->UnZoom();

  TCanvas *timeCanvas = new TCanvas(TString::Format("c_time_cal_clover%d_crystal%d", clo, cri),
                                    TString::Format("Clover%d Crystal%d time calibration", clo, cri),
                                    1000, 700);
  if (time) {
    time->Draw("hist");
    for (const auto &obj : *time->GetListOfFunctions()) obj->Draw("same");
    TLegend *legend = new TLegend(0.55, 0.72, 0.90, 0.90);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->AddEntry(time, "Time spectrum", "l");
    if (timeFit.ok) {
      legend->AddEntry((TObject *)nullptr, TString::Format("peak %.3f ns", timeFit.mean), "");
      legend->AddEntry((TObject *)nullptr, TString::Format("offset %.3f ns", kTargetTimeNs - timeFit.mean), "");
    }
    legend->Draw();
  }
  timeCanvas->Write();
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
                                       const char *timeBranchLeaf = "fDeltaT",
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
