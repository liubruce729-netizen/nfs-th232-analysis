// Build per-crystal NFS neutron-time references from standard double-peak data.
// 从带有双时间峰的标准数据中，建立逐 crystal 的 NFS 中子时间参考。
//
// Input / 输入:
//   1. One nfs_histo ROOT file / 单个 nfs_histo ROOT 文件
//   2. A directory, searched recursively for nfs_histoExogam2_*.root
//      一个目录，递归搜索所有 nfs_histoExogam2_*.root
//   3. A comma-separated mixture or @list containing files/directories
//      文件/目录混合的逗号列表，或 @文本列表
//
// Examples / 示例:
//   root -l -b -q 'lsy_nfs/fit_nfs_standard_time_reference.C("out/nfs_histoExogam2_1.root")'
//   root -l -b -q 'lsy_nfs/fit_nfs_standard_time_reference.C("/data/standard_runs","out/standard_time_reference")'
//   root -l -b -q 'lsy_nfs/fit_nfs_standard_time_reference.C("@standard_inputs.txt","out/standard_time_reference")'
//
// Method / 方法:
//   measured gamma peak: Gaussian + linear background in 170-350 ns
//   gamma offset       : 78.04 ns - measured gamma peak
//   measured neutron   : exGaussian + linear background in 400-800 ns
//   neutron reference  : measured neutron peak + gamma offset
//
// Multiple input histograms are summed crystal by crystal before fitting.
// 多个输入文件先按 crystal 合并同名时间谱，再进行一次统一拟合。

#include "calibrate_nfs_crystal_energy_time.C"

#include <array>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <set>
#include <sys/stat.h>

#include "TKey.h"
#include "TLine.h"
#include "TSystem.h"

namespace {

bool IsDirectoryPath(const TString &path)
{
  struct stat info {};
  return ::stat(path.Data(), &info) == 0 && S_ISDIR(info.st_mode);
}

bool IsRegularFilePath(const TString &path)
{
  struct stat info {};
  return ::stat(path.Data(), &info) == 0 && S_ISREG(info.st_mode);
}

TString PathBaseName(const TString &path)
{
  const Ssiz_t slash = path.Last('/');
  return slash == kNPOS ? path : path(slash + 1, path.Length() - slash - 1);
}

bool IsNfsHistogramRoot(const TString &path)
{
  const TString name = PathBaseName(path);
  return name.BeginsWith("nfs_histoExogam2_") && name.EndsWith(".root");
}

void CollectHistogramRootsRecursive(const TString &directory, std::vector<TString> &files)
{
  DIR *dir = ::opendir(directory.Data());
  if (!dir) {
    std::cerr << "Cannot open directory: " << directory << " (" << std::strerror(errno) << ")" << std::endl;
    return;
  }

  while (dirent *entry = ::readdir(dir)) {
    TString name(entry->d_name);
    if (name == "." || name == "..") continue;
    TString child = directory;
    if (!child.EndsWith("/")) child += "/";
    child += name;
    if (IsDirectoryPath(child)) CollectHistogramRootsRecursive(child, files);
    else if (IsRegularFilePath(child) && IsNfsHistogramRoot(child)) files.push_back(child);
  }
  ::closedir(dir);
}

std::vector<TString> ExpandStandardInputs(const char *inputSources)
{
  std::vector<TString> files;
  for (const TString &source : SplitInputs(inputSources)) {
    if (IsDirectoryPath(source)) {
      CollectHistogramRootsRecursive(source, files);
    }
    else if (IsRegularFilePath(source)) {
      if (source.EndsWith(".root")) files.push_back(source);
      else {
        std::cerr << "Skip non-ROOT standard input: " << source << std::endl;
      }
    }
    else {
      std::cerr << "Standard input does not exist: " << source << std::endl;
    }
  }

  std::sort(files.begin(), files.end(), [](const TString &a, const TString &b) {
    return std::string(a.Data()) < std::string(b.Data());
  });
  files.erase(std::unique(files.begin(), files.end(), [](const TString &a, const TString &b) {
    return a == b;
  }), files.end());
  return files;
}

TString DefaultStandardPrefix(const char *inputSources)
{
  const auto sources = SplitInputs(inputSources);
  if (sources.size() == 1 && IsDirectoryPath(sources.front())) {
    TString prefix = sources.front();
    if (!prefix.EndsWith("/")) prefix += "/";
    return prefix + "nfs_standard_time_reference";
  }
  if (sources.size() == 1 && sources.front().EndsWith(".root")) {
    TString prefix = sources.front();
    prefix.Remove(prefix.Length() - 5);
    return prefix + "_standard_time_reference";
  }
  return "nfs_standard_time_reference";
}

PeakFitResult FitStandardGammaPeak(TH1F *hist, int detector, double fitLow, double fitHigh)
{
  PeakFitResult result;
  if (!hist || hist->GetEntries() <= 0.0 || fitHigh <= fitLow) return result;
  result.entries = hist->GetEntries();

  const int seedBin = FindMaxBinInRange(hist, fitLow, fitHigh);
  if (seedBin < 0) return result;
  const double seed = hist->GetBinCenter(seedBin);
  const double seedContent = hist->GetBinContent(seedBin);
  result.seed = seed;

  const double left = hist->GetBinContent(hist->GetXaxis()->FindBin(fitLow));
  const double right = hist->GetBinContent(hist->GetXaxis()->FindBin(fitHigh));
  const double background = std::max(0.0, 0.5 * (left + right));
  const double amplitude = std::max(1.0, seedContent - background);
  const double roughFwhm = EstimatePeakFwhm(hist, seedBin, fitLow, fitHigh);
  const double roughSigma = std::max(0.5, roughFwhm / 2.354820045);

  TF1 *fit = new TF1(TString::Format("fit_standard_gamma_det%d", detector),
                     EnergyFitFunction, fitLow, fitHigh, 5);
  fit->SetParNames("Amp", "Mean", "Sigma", "Bg0", "BgSlope");
  fit->SetParameters(amplitude, seed, roughSigma, background, 0.0);
  fit->SetParLimits(0, 0.0, std::max(10.0, seedContent * 20.0));
  fit->SetParLimits(1, fitLow, fitHigh);
  fit->SetParLimits(2, 0.2, std::max(5.0, 0.25 * (fitHigh - fitLow)));
  fit->SetLineColor(kRed + 1);

  TFitResultPtr fitResult = hist->Fit(fit, "QRS0+");
  result.status = int(fitResult);
  result.mean = fit->GetParameter(1);
  result.meanErr = fit->GetParError(1);
  result.sigma = std::fabs(fit->GetParameter(2));
  result.sigmaErr = fit->GetParError(2);
  result.amplitude = fit->GetParameter(0);
  if (fit->GetNDF() > 0) result.chi2ndf = fit->GetChisquare() / fit->GetNDF();
  result.ok = std::isfinite(result.mean) && result.mean >= fitLow &&
              result.mean <= fitHigh && result.sigma > 0.0;
  return result;
}

void WriteStandardReferenceText(const TString &path,
                                const std::array<TH1F *, kNDetector> &histograms,
                                const std::array<PeakFitResult, kNDetector> &gammaFits,
                                const std::array<PeakFitResult, kNDetector> &neutronFits,
                                double gammaTrueTimeNs)
{
  std::ofstream out(path.Data());
  out << "# NFS per-crystal standard neutron-time reference\n";
  out << "# gamma_offset_ns = gamma_true_ns - gamma_peak_ns\n";
  out << "# neutron_reference_ns = neutron_peak_ns + gamma_offset_ns\n";
  out << "# clover crystal detector time_entries gamma_peak_ns gamma_peak_err_ns gamma_sigma_ns gamma_fit_status gamma_true_ns gamma_offset_ns neutron_peak_ns neutron_peak_err_ns neutron_sigma_ns neutron_fit_status neutron_reference_ns reference_valid\n";
  out << std::setprecision(12);

  for (int detector = 0; detector < kNDetector; ++detector) {
    const auto &gamma = gammaFits[detector];
    const auto &neutron = neutronFits[detector];
    const bool valid = gamma.ok && neutron.ok;
    const double offset = gamma.ok ? gammaTrueTimeNs - gamma.mean : std::numeric_limits<double>::quiet_NaN();
    const double reference = valid ? neutron.mean + offset : std::numeric_limits<double>::quiet_NaN();
    out << detector / 4 << " " << detector % 4 << " " << detector << " "
        << (histograms[detector] ? histograms[detector]->GetEntries() : 0.0) << " "
        << gamma.mean << " " << gamma.meanErr << " " << gamma.sigma << " " << gamma.status << " "
        << gammaTrueTimeNs << " " << offset << " "
        << neutron.mean << " " << neutron.meanErr << " " << neutron.sigma << " "
        << neutron.status << " " << reference << " " << (valid ? 1 : 0) << "\n";
  }
}

} // namespace

void fit_nfs_standard_time_reference(const char *inputSources,
                                     const char *outputPrefix = "",
                                     double gammaTrueTimeNs = 78.04,
                                     double gammaFitLowNs = 170.0,
                                     double gammaFitHighNs = 350.0,
                                     double neutronFitLowNs = 400.0,
                                     double neutronFitHighNs = 800.0)
{
  if (!inputSources || TString(inputSources).Strip(TString::kBoth).IsNull()) {
    std::cerr << "No standard nfs_histo input was provided." << std::endl;
    return;
  }
  if (gammaFitHighNs <= gammaFitLowNs || neutronFitHighNs <= neutronFitLowNs) {
    std::cerr << "Invalid gamma or neutron fit interval." << std::endl;
    return;
  }

  const auto inputFiles = ExpandStandardInputs(inputSources);
  if (inputFiles.empty()) {
    std::cerr << "No nfs_histo ROOT files found in: " << inputSources << std::endl;
    return;
  }

  const TString prefix = (outputPrefix && TString(outputPrefix).Length() > 0)
                             ? TString(outputPrefix)
                             : DefaultStandardPrefix(inputSources);
  const TString outputDirectory = gSystem->DirName(prefix);
  if (!outputDirectory.IsNull() && outputDirectory != "." &&
      gSystem->mkdir(outputDirectory, true) != 0 && !IsDirectoryPath(outputDirectory)) {
    std::cerr << "Cannot create output directory: " << outputDirectory << std::endl;
    return;
  }
  const TString rootOutput = prefix + ".root";
  const TString textOutput = prefix + ".txt";

  std::array<TH1F *, kNDetector> merged {};
  int usableFiles = 0;
  for (const TString &path : inputFiles) {
    std::unique_ptr<TFile> input(TFile::Open(path, "READ"));
    if (!input || input->IsZombie()) {
      std::cerr << "Cannot open ROOT input: " << path << std::endl;
      continue;
    }

    bool usedThisFile = false;
    for (int detector = 0; detector < kNDetector; ++detector) {
      const int clover = detector / 4;
      const int crystal = detector % 4;
      const TString histogramName = TString::Format("nfs_clover%d_crystal%d_time", clover, crystal);
      TH1 *source = dynamic_cast<TH1 *>(input->Get(histogramName));
      if (!source) continue;

      if (!merged[detector]) {
        merged[detector] = new TH1F(
            TString::Format("standard_clover%d_crystal%d_time", clover, crystal),
            TString::Format("Clover%d Crystal%d merged standard time;Time (ns);Counts", clover, crystal),
            source->GetNbinsX(), source->GetXaxis()->GetXmin(), source->GetXaxis()->GetXmax());
        merged[detector]->SetDirectory(nullptr);
        merged[detector]->Sumw2();
      }
      if (merged[detector]->GetNbinsX() != source->GetNbinsX() ||
          std::fabs(merged[detector]->GetXaxis()->GetXmin() - source->GetXaxis()->GetXmin()) > 1e-9 ||
          std::fabs(merged[detector]->GetXaxis()->GetXmax() - source->GetXaxis()->GetXmax()) > 1e-9) {
        std::cerr << "Skip incompatible histogram " << histogramName << " in " << path << std::endl;
        continue;
      }
      merged[detector]->Add(source);
      usedThisFile = true;
    }
    if (usedThisFile) usableFiles++;
  }

  if (usableFiles <= 0) {
    std::cerr << "Input ROOT files contain no nfs_cloverX_crystalY_time histograms." << std::endl;
    return;
  }

  std::array<PeakFitResult, kNDetector> gammaFits {};
  std::array<PeakFitResult, kNDetector> neutronFits {};
  std::array<double, kNDetector> references {};
  references.fill(std::numeric_limits<double>::quiet_NaN());

  TFile output(rootOutput, "RECREATE");
  if (output.IsZombie()) {
    std::cerr << "Cannot create output ROOT: " << rootOutput << std::endl;
    return;
  }
  TDirectory *spectraDirectory = output.mkdir("merged_time_spectra");
  TDirectory *canvasDirectory = output.mkdir("fit_canvases");
  TDirectory *summaryDirectory = output.mkdir("summary");

  for (int detector = 0; detector < kNDetector; ++detector) {
    TH1F *histogram = merged[detector];
    if (!histogram) continue;
    gammaFits[detector] = FitStandardGammaPeak(histogram, detector, gammaFitLowNs, gammaFitHighNs);
    neutronFits[detector] = FitTimePeak(histogram, detector, neutronFitLowNs, neutronFitHighNs,
                                        kDefaultTimeFitPre, kDefaultTimeFitPost);
    if (gammaFits[detector].ok && neutronFits[detector].ok) {
      references[detector] = neutronFits[detector].mean + gammaTrueTimeNs - gammaFits[detector].mean;
    }

    spectraDirectory->cd();
    histogram->Write();

    canvasDirectory->cd();
    const int clover = detector / 4;
    const int crystal = detector % 4;
    TCanvas canvas(TString::Format("c_standard_time_clover%d_crystal%d", clover, crystal),
                   TString::Format("Clover%d Crystal%d standard time fits", clover, crystal),
                   1200, 600);
    canvas.Divide(2, 1);

    canvas.cd(1);
    TH1F *gammaCopy = static_cast<TH1F *>(histogram->Clone(
        TString::Format("standard_gamma_canvas_det%d", detector)));
    gammaCopy->SetDirectory(nullptr);
    gammaCopy->GetListOfFunctions()->Clear();
    gammaCopy->GetXaxis()->SetRangeUser(gammaFitLowNs, gammaFitHighNs);
    gammaCopy->Draw("hist");
    if (TF1 *fit = histogram->GetFunction(TString::Format("fit_standard_gamma_det%d", detector))) {
      fit->DrawCopy("same");
    }

    canvas.cd(2);
    TH1F *neutronCopy = static_cast<TH1F *>(histogram->Clone(
        TString::Format("standard_neutron_canvas_det%d", detector)));
    neutronCopy->SetDirectory(nullptr);
    neutronCopy->GetListOfFunctions()->Clear();
    neutronCopy->GetXaxis()->SetRangeUser(neutronFitLowNs, neutronFitHighNs);
    neutronCopy->Draw("hist");
    if (TF1 *seed = histogram->GetFunction(TString::Format("fit_time_gaussian_seed_det%d", detector))) {
      seed->DrawCopy("same");
    }
    if (TF1 *fit = histogram->GetFunction(TString::Format("fit_time_det%d", detector))) {
      fit->DrawCopy("same");
    }
    canvas.Write();
    delete gammaCopy;
    delete neutronCopy;
  }

  summaryDirectory->cd();
  TH1F gammaSummary("StandardGammaPeakByCrystal",
                    "Measured standard gamma peak;Clover-Crystal;Time (ns)",
                    kNDetector, -0.5, kNDetector - 0.5);
  TH1F offsetSummary("StandardGammaOffsetByCrystal",
                     "Gamma time offset;Clover-Crystal;Offset (ns)",
                     kNDetector, -0.5, kNDetector - 0.5);
  TH1F neutronSummary("MeasuredNeutronPeakByCrystal",
                      "Measured neutron peak;Clover-Crystal;Time (ns)",
                      kNDetector, -0.5, kNDetector - 0.5);
  TH1F referenceSummary("NeutronReferenceByCrystal",
                        "Per-crystal neutron time reference;Clover-Crystal;Time (ns)",
                        kNDetector, -0.5, kNDetector - 0.5);
  int validReferences = 0;
  for (int detector = 0; detector < kNDetector; ++detector) {
    const TString label = TString::Format("%d-%d", detector / 4, detector % 4);
    for (TH1F *summary : {&gammaSummary, &offsetSummary, &neutronSummary, &referenceSummary}) {
      summary->GetXaxis()->SetBinLabel(detector + 1, label);
    }
    if (gammaFits[detector].ok) {
      gammaSummary.SetBinContent(detector + 1, gammaFits[detector].mean);
      gammaSummary.SetBinError(detector + 1, gammaFits[detector].meanErr);
      offsetSummary.SetBinContent(detector + 1, gammaTrueTimeNs - gammaFits[detector].mean);
      offsetSummary.SetBinError(detector + 1, gammaFits[detector].meanErr);
    }
    if (neutronFits[detector].ok) {
      neutronSummary.SetBinContent(detector + 1, neutronFits[detector].mean);
      neutronSummary.SetBinError(detector + 1, neutronFits[detector].meanErr);
    }
    if (std::isfinite(references[detector])) {
      referenceSummary.SetBinContent(detector + 1, references[detector]);
      referenceSummary.SetBinError(
          detector + 1,
          std::hypot(gammaFits[detector].meanErr, neutronFits[detector].meanErr));
      validReferences++;
    }
  }
  gammaSummary.Write();
  offsetSummary.Write();
  neutronSummary.Write();
  referenceSummary.Write();

  const TString configText = TString::Format(
      "inputs=%s; discovered=%zu; usable=%d; gamma_true_ns=%.6f; gamma_fit_ns=%.3f:%.3f; neutron_fit_ns=%.3f:%.3f",
      inputSources, inputFiles.size(), usableFiles, gammaTrueTimeNs,
      gammaFitLowNs, gammaFitHighNs, neutronFitLowNs, neutronFitHighNs);
  TNamed config("StandardTimeReferenceConfig", configText.Data());
  config.Write();
  output.Close();

  WriteStandardReferenceText(textOutput, merged, gammaFits, neutronFits, gammaTrueTimeNs);
  for (TH1F *histogram : merged) delete histogram;

  std::cout << "Discovered nfs_histo ROOT files: " << inputFiles.size() << std::endl;
  std::cout << "Usable nfs_histo ROOT files: " << usableFiles << std::endl;
  std::cout << "Valid crystal references: " << validReferences << "/" << kNDetector << std::endl;
  std::cout << "Output ROOT: " << rootOutput << std::endl;
  std::cout << "Reference text: " << textOutput << std::endl;
}
