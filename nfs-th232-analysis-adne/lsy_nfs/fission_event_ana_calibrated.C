// Calibrated fission-event gamma analysis from NFS mult3 trees.
// 基于 NFS mult3 tree 的逐 run、逐 crystal 刻度裂变候选 gamma 分析。
//
// Usage / 用法:
//   root -l -b -q 'lsy_nfs/fission_event_ana_calibrated.C("@mult3_files.txt","calibration_summary.tsv",24,20,"4,10,20,50","out/fission_cal.root")'
//
// EN: This macro rebuilds clover addback from crystal-level branches so that
//     per-crystal energy/time calibration can be applied before fission cuts.
// CN: 该宏从 crystal 分支重新合并 clover addback，从而在裂变判选前应用逐晶体能量/时间刻度。

#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "TBranch.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLegend.h"
#include "TNamed.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TPaveText.h"
#include "TString.h"
#include "TSystem.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"

namespace {

constexpr int kNClover = 16;
constexpr int kNCrystalPerClover = 4;
constexpr int kNDetector = kNClover * kNCrystalPerClover;
constexpr double kNeutronMassMeV = 939.5654;
constexpr double kLightSpeedMeterPerNs = 0.299792458;

double EnergyMeVToToFNs(double energyMeV, double distanceMeter)
{
  if (energyMeV <= 0.0 || distanceMeter <= 0.0) return std::numeric_limits<double>::infinity();
  const double beta = std::sqrt(2.0 * energyMeV / kNeutronMassMeV);
  return distanceMeter / (kLightSpeedMeterPerNs * beta);
}

std::string Trim(std::string s)
{
  auto notSpace = [](unsigned char c) { return !std::isspace(c); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
  s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
  return s;
}

std::vector<std::string> SplitTab(const std::string &line)
{
  std::vector<std::string> out;
  std::string item;
  std::stringstream ss(line);
  while (std::getline(ss, item, '\t')) out.push_back(item);
  return out;
}

std::vector<std::string> SplitWhitespace(const std::string &line)
{
  std::vector<std::string> out;
  std::stringstream ss(line);
  std::string item;
  while (ss >> item) out.push_back(item);
  return out;
}

bool ParseDouble(const std::string &text, double &value)
{
  if (text.empty() || text == "nan" || text == "NaN" || text == "NAN") return false;
  char *end = nullptr;
  value = std::strtod(text.c_str(), &end);
  return end && *end == '\0' && std::isfinite(value);
}

bool ParseInt(const std::string &text, int &value)
{
  if (text.empty()) return false;
  char *end = nullptr;
  long v = std::strtol(text.c_str(), &end, 10);
  if (!end || *end != '\0') return false;
  value = static_cast<int>(v);
  return true;
}

TString BaseName(const TString &path)
{
  Ssiz_t slash = path.Last('/');
  if (slash == kNPOS) return path;
  return path(slash + 1, path.Length() - slash - 1);
}

TString NormalizeRunKey(TString name)
{
  name = BaseName(name);
  if (name.BeginsWith("mult3_")) name.Remove(0, 6);
  return name;
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
    TString item(Trim(line).c_str());
    if (item.IsNull() || item.BeginsWith("#") || item.BeginsWith("//")) continue;
    items.push_back(item);
  }
  return items;
}

std::vector<TString> SplitCsv(const char *text)
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

std::vector<double> ParseEnergyEdges(const char *energyBinEdgesMeV)
{
  std::vector<double> edges;
  for (const auto &item : SplitCsv(energyBinEdgesMeV)) edges.push_back(item.Atof());
  std::sort(edges.begin(), edges.end());
  edges.erase(std::unique(edges.begin(), edges.end()), edges.end());
  return edges;
}

TString FormatNumberForName(double value)
{
  TString label;
  if (std::fabs(value - std::round(value)) < 1e-6) label.Form("%.0f", value);
  else label.Form("%.3f", value);
  label.ReplaceAll("-", "m");
  label.ReplaceAll(".", "p");
  return label;
}

TString MakeBinTag(double lowEnergyMeV, double highEnergyMeV)
{
  return TString::Format("E%s_%sMeV", FormatNumberForName(lowEnergyMeV).Data(),
                         FormatNumberForName(highEnergyMeV).Data());
}

TString MakeBinTitle(double lowEnergyMeV, double highEnergyMeV, double lowToFNs, double highToFNs)
{
  if (lowEnergyMeV <= 0.0) {
    return TString::Format("E_n %.3g-%.3g MeV, Time >= %.2f ns", lowEnergyMeV, highEnergyMeV, highToFNs);
  }
  return TString::Format("E_n %.3g-%.3g MeV, %.2f <= Time < %.2f ns",
                         lowEnergyMeV, highEnergyMeV, highToFNs, lowToFNs);
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

struct CrystalCalib {
  double energyOffset = 0.0;
  double energyGain = 1.0;
  double timeOffset = 0.0;
  double timeGain = 1.0;
  bool hasEnergy = false;
  bool hasTime = false;
};

struct RunCalib {
  std::array<CrystalCalib, kNDetector> crystal;
  int rows = 0;
};

struct CalibrationDB {
  std::map<TString, RunCalib> byRun;
  RunCalib defaultCalib;
  bool hasDefault = false;
};

std::map<std::string, int> HeaderMap(const std::vector<std::string> &header)
{
  std::map<std::string, int> out;
  for (std::size_t i = 0; i < header.size(); ++i) out[header[i]] = static_cast<int>(i);
  return out;
}

std::string GetColumn(const std::vector<std::string> &cols, const std::map<std::string, int> &header, const std::string &name)
{
  auto it = header.find(name);
  if (it == header.end() || it->second < 0 || static_cast<std::size_t>(it->second) >= cols.size()) return "";
  return cols[it->second];
}

CalibrationDB ReadCalibrationDB(const char *summaryPath)
{
  CalibrationDB db;
  std::ifstream input(summaryPath ? summaryPath : "");
  if (!input.is_open()) {
    std::cerr << "Cannot open calibration summary: " << (summaryPath ? summaryPath : "") << std::endl;
    return db;
  }

  std::vector<std::string> header;
  std::map<std::string, int> hmap;
  bool tabSeparated = true;
  std::string line;
  while (std::getline(input, line)) {
    line = Trim(line);
    if (line.empty()) continue;
    if (line.rfind("# clover", 0) == 0) {
      // EN: Also accept a single detailed calibration txt as a default calibration.
      // CN: 也兼容单个详细刻度 txt，此时作为默认刻度使用。
      line = Trim(line.substr(1));
      header = SplitWhitespace(line);
      hmap = HeaderMap(header);
      tabSeparated = false;
      continue;
    }
    if (line[0] == '#') continue;
    if (header.empty()) {
      tabSeparated = (line.find('\t') != std::string::npos);
      header = tabSeparated ? SplitTab(line) : SplitWhitespace(line);
      hmap = HeaderMap(header);
      continue;
    }

    std::vector<std::string> cols = tabSeparated ? SplitTab(line) : SplitWhitespace(line);
    if (cols.size() < header.size() && tabSeparated) cols = SplitWhitespace(line);

    const std::string status = GetColumn(cols, hmap, "status");
    if (!status.empty() && status != "ok") continue;

    int det = -1;
    if (!ParseInt(GetColumn(cols, hmap, "detector"), det)) {
      int clo = -1, cri = -1;
      if (ParseInt(GetColumn(cols, hmap, "clover"), clo) && ParseInt(GetColumn(cols, hmap, "crystal"), cri)) {
        det = clo * kNCrystalPerClover + cri;
      }
    }
    if (det < 0 || det >= kNDetector) continue;

    TString key = "*";
    std::string runFile = GetColumn(cols, hmap, "run_file");
    if (runFile.empty()) runFile = GetColumn(cols, hmap, "input");
    if (!runFile.empty()) key = NormalizeRunKey(TString(runFile.c_str()));

    RunCalib &run = (key == "*") ? db.defaultCalib : db.byRun[key];
    if (key == "*") db.hasDefault = true;
    CrystalCalib &cal = run.crystal[det];

    double value = 0.0;
    if (ParseDouble(GetColumn(cols, hmap, "energy_offset"), value)) {
      cal.energyOffset = value;
      cal.hasEnergy = true;
    }
    if (ParseDouble(GetColumn(cols, hmap, "energy_gain"), value)) {
      cal.energyGain = value;
      cal.hasEnergy = true;
    }
    // EN: Prefer the new per-crystal standard reference columns, while still
    //     accepting calibration summaries made by the older fixed-442 ns code.
    // CN: 优先读取新的逐 crystal 标准时间列，同时兼容旧版固定 442 ns 列。
    std::string timeOffsetText = GetColumn(cols, hmap, "time_offset_to_reference_ns");
    if (timeOffsetText.empty()) timeOffsetText = GetColumn(cols, hmap, "time_offset_to_442_ns");
    if (ParseDouble(timeOffsetText, value)) {
      cal.timeOffset = value;
      cal.hasTime = true;
    }
    std::string timeGainText = GetColumn(cols, hmap, "time_gain_to_reference");
    if (timeGainText.empty()) timeGainText = GetColumn(cols, hmap, "time_gain_to_442");
    if (ParseDouble(timeGainText, value)) {
      cal.timeGain = value;
      cal.hasTime = true;
    }
    run.rows++;
  }
  return db;
}

const RunCalib *FindRunCalib(const CalibrationDB &db, const TString &inputFile)
{
  TString key = NormalizeRunKey(inputFile);
  auto it = db.byRun.find(key);
  if (it != db.byRun.end()) return &it->second;
  TString exact = BaseName(inputFile);
  it = db.byRun.find(exact);
  if (it != db.byRun.end()) return &it->second;
  if (db.hasDefault) return &db.defaultCalib;
  return nullptr;
}

double ApplyTimeCorrection(double rawTime, const CrystalCalib &cal, const TString &mode)
{
  if (mode.EqualTo("gain", TString::kIgnoreCase)) return rawTime * cal.timeGain;
  if (mode.EqualTo("offset_gain", TString::kIgnoreCase)) return (rawTime + cal.timeOffset) * cal.timeGain;
  if (mode.EqualTo("gain_offset", TString::kIgnoreCase)) return rawTime * cal.timeGain + cal.timeOffset;
  return rawTime + cal.timeOffset;
}

struct EnergyTimeBin {
  double lowEnergyMeV = 0.0;
  double highEnergyMeV = 0.0;
  double lowToFNs = std::numeric_limits<double>::infinity();
  double highToFNs = 0.0;
  TString tag;
  TString title;
  TH1F *spectrum = nullptr;
  TH2F *matrix = nullptr;
  Long64_t selectedEvents = 0;
  Long64_t selectedGammaFires = 0;
};

int FindEnergyTimeBin(const std::vector<EnergyTimeBin> &bins, double eventTimeNs)
{
  for (std::size_t i = 0; i < bins.size(); ++i) {
    const auto &bin = bins[i];
    if (eventTimeNs < bin.highToFNs) continue;
    if (bin.lowEnergyMeV > 0.0 && eventTimeNs >= bin.lowToFNs) continue;
    return static_cast<int>(i);
  }
  return -1;
}

struct CloverHit {
  double energy = 0.0;
  double time = std::numeric_limits<double>::infinity();
  double bestCrystalEnergy = -1.0;
  bool bgo = false;
  bool csi = false;
  int multiplicity = 0;
};

void WriteCanvasForSpectrum(TDirectory *outDir, TH1F *hist, const TString &binTitle)
{
  TCanvas *canvas = new TCanvas(TString::Format("c_%s", hist->GetName()), hist->GetTitle(), 1100, 750);
  hist->Draw("hist");
  TLegend *legend = new TLegend(0.58, 0.76, 0.88, 0.88);
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);
  legend->AddEntry(hist, binTitle, "l");
  legend->Draw();
  outDir->cd();
  canvas->Write();
  delete canvas;
}

void WriteCanvasForMatrix(TDirectory *outDir, TH2F *hist, const TString &binTitle)
{
  TCanvas *canvas = new TCanvas(TString::Format("c_%s", hist->GetName()), hist->GetTitle(), 1100, 900);
  hist->Draw("colz");
  TPaveText *text = new TPaveText(0.12, 0.92, 0.55, 0.98, "NDC");
  text->SetBorderSize(0);
  text->SetFillStyle(0);
  text->AddText(binTitle);
  text->Draw();
  outDir->cd();
  canvas->Write();
  delete canvas;
}

} // namespace

void fission_event_ana_calibrated(const char *inputFiles = "out/mult3_nfs_run_100_r0.root",
                                  const char *calibrationSummary = "calibration_summary.tsv",
                                  double nfsDistanceMeter = 8.6,
                                  double timeFwhmNs = 20.0,
                                  const char *energyBinEdgesMeV = "2,4,7,12,20,30,50",
                                  const char *outputFile = "",
                                  bool useBgoCsiVeto = true,
                                  Long64_t maxEntries = -1,
                                  int minCloverMultiplicity = 2,
                                  const char *timeCorrectionMode = "offset",
                                  const char *timeBranchLeaf = "fTime")
{
  if (minCloverMultiplicity < 1 || nfsDistanceMeter <= 0.0) {
    std::cerr << "Invalid fission analysis parameters." << std::endl;
    return;
  }

  const std::vector<TString> inputs = SplitCsv(inputFiles);
  if (inputs.empty()) {
    std::cerr << "No mult3 input files were provided." << std::endl;
    return;
  }

  CalibrationDB calibDB = ReadCalibrationDB(calibrationSummary);
  if (calibDB.byRun.empty() && !calibDB.hasDefault) {
    std::cerr << "No usable calibration rows found in: " << calibrationSummary << std::endl;
    return;
  }

  std::vector<double> edges = ParseEnergyEdges(energyBinEdgesMeV);
  if (edges.empty() || edges.back() <= 0.0) {
    std::cerr << "No valid positive neutron-energy bin edge was provided." << std::endl;
    return;
  }

  const double highestEnergyMeV = 50.0;
  const double tMinNs = EnergyMeVToToFNs(highestEnergyMeV, nfsDistanceMeter);
  const double sigmaTimeNs = timeFwhmNs / 2.355;
  const double tCutNs = tMinNs - 3.0 * sigmaTimeNs;

  std::vector<EnergyTimeBin> bins;
  double lowEnergy = 0.0;
  for (double highEnergy : edges) {
    if (highEnergy <= lowEnergy) continue;
    EnergyTimeBin bin;
    bin.lowEnergyMeV = lowEnergy;
    bin.highEnergyMeV = highEnergy;
    bin.lowToFNs = EnergyMeVToToFNs(lowEnergy, nfsDistanceMeter);
    bin.highToFNs = EnergyMeVToToFNs(highEnergy, nfsDistanceMeter);
    bin.tag = MakeBinTag(lowEnergy, highEnergy);
    bin.title = MakeBinTitle(lowEnergy, highEnergy, bin.lowToFNs, bin.highToFNs);
    bin.spectrum = new TH1F(TString::Format("Mult3GammaSpec_%s", bin.tag.Data()),
                            TString::Format("Mult3GammaSpec %s;Calibrated gamma energy (keV);Counts", bin.title.Data()),
                            4096, 0.0, 4096.0);
    bin.matrix = new TH2F(TString::Format("Mult3GammaGammaMatrix_%s", bin.tag.Data()),
                          TString::Format("Mult3GammaGammaMatrix %s;Calibrated gamma energy (keV);Calibrated gamma energy (keV)", bin.title.Data()),
                          4096, 0.0, 4096.0, 4096, 0.0, 4096.0);
    bins.push_back(bin);
    lowEnergy = highEnergy;
  }

  const TString multiplicityTitle = TString::Format("Mult>=%d calibrated fission candidates", minCloverMultiplicity);
  TH2F *gammaEnergyVsTime = new TH2F(
      "Mult3GammaEnergyVsTime",
      TString::Format("%s;Calibrated clover addback gamma energy (keV);Calibrated clover Time (ns)", multiplicityTitle.Data()),
      4096, 0.0, 4096.0, 1600, 0.0, 1600.0);

  Long64_t totalEntries = 0;
  Long64_t keptAfterTimeCut = 0;
  Long64_t skippedMultiplicity = 0;
  Long64_t skippedNoBin = 0;
  Long64_t skippedMalformed = 0;
  Long64_t missingCalibrationFiles = 0;
  Long64_t missingCrystalCalibration = 0;
  Long64_t processedFiles = 0;

  TString timeMode(timeCorrectionMode ? timeCorrectionMode : "offset");

  for (const auto &inputName : inputs) {
    TFile inputFile(inputName, "READ");
    if (!inputFile.IsOpen()) {
      std::cerr << "Cannot open input file: " << inputName << std::endl;
      continue;
    }
    TTree *tree = dynamic_cast<TTree *>(inputFile.Get("TreeMaster"));
    if (!tree) {
      std::cerr << "Cannot find TreeMaster in: " << inputName << std::endl;
      continue;
    }
    processedFiles++;

    const RunCalib *runCal = FindRunCalib(calibDB, inputName);
    if (!runCal) {
      std::cerr << "Warning: no calibration found for " << inputName << "; using identity calibration." << std::endl;
      missingCalibrationFiles++;
    }
    RunCalib identityCal;
    const RunCalib &calib = runCal ? *runCal : identityCal;

    const TString cloverBranchName = ResolveBranch(tree, "fEXO_ECC_E_Clover");
    const TString crystalBranchName = ResolveBranch(tree, "fEXO_ECC_E_Cristal");
    const TString energyBranchName = ResolveBranch(tree, "fEXO_ECC_E_Energy");
    const TString timeBranchName = ResolveBranch(tree, timeBranchLeaf && TString(timeBranchLeaf).Length() > 0 ? timeBranchLeaf : "fTime");
    const TString essCloverBranchName = ResolveBranch(tree, "fEXO_ESS_Clover");
    const TString essCrystalBranchName = ResolveBranch(tree, "fEXO_ESS_Cristal");
    const TString bgoBranchName = ResolveBranch(tree, "fEXO_ESS_BGO");
    const TString csiBranchName = ResolveBranch(tree, "fEXO_ESS_CSI");

    if (cloverBranchName.IsNull() || crystalBranchName.IsNull() || energyBranchName.IsNull() || timeBranchName.IsNull()) {
      std::cerr << "Missing required crystal branches in: " << inputName << std::endl;
      continue;
    }
    if (useBgoCsiVeto && (essCloverBranchName.IsNull() || essCrystalBranchName.IsNull() || bgoBranchName.IsNull() || csiBranchName.IsNull())) {
      std::cerr << "BGO/CSI veto requested but ESS branches are missing in: " << inputName << std::endl;
      continue;
    }

    TTreeReader reader(tree);
    TTreeReaderArray<UShort_t> clovers(reader, cloverBranchName.Data());
    TTreeReaderArray<UShort_t> crystals(reader, crystalBranchName.Data());
    TTreeReaderArray<float> energies(reader, energyBranchName.Data());
    TTreeReaderArray<float> times(reader, timeBranchName.Data());
    std::unique_ptr<TTreeReaderArray<UShort_t>> essClovers;
    std::unique_ptr<TTreeReaderArray<UShort_t>> essCrystals;
    std::unique_ptr<TTreeReaderArray<UShort_t>> bgo;
    std::unique_ptr<TTreeReaderArray<UShort_t>> csi;
    if (useBgoCsiVeto) {
      essClovers.reset(new TTreeReaderArray<UShort_t>(reader, essCloverBranchName.Data()));
      essCrystals.reset(new TTreeReaderArray<UShort_t>(reader, essCrystalBranchName.Data()));
      bgo.reset(new TTreeReaderArray<UShort_t>(reader, bgoBranchName.Data()));
      csi.reset(new TTreeReaderArray<UShort_t>(reader, csiBranchName.Data()));
    }

    Long64_t fileEntry = 0;
    while (reader.Next()) {
      if (maxEntries > 0 && totalEntries >= maxEntries) break;
      totalEntries++;
      fileEntry++;

      const std::size_t n = static_cast<std::size_t>(energies.GetSize());
      if (clovers.GetSize() != energies.GetSize() || crystals.GetSize() != energies.GetSize() || times.GetSize() != energies.GetSize()) {
        skippedMalformed++;
        continue;
      }

      std::array<CloverHit, kNClover> hit;
      if (useBgoCsiVeto) {
        const std::size_t nv = std::min({static_cast<std::size_t>(essClovers->GetSize()),
                                         static_cast<std::size_t>(essCrystals->GetSize()),
                                         static_cast<std::size_t>(bgo->GetSize()),
                                         static_cast<std::size_t>(csi->GetSize())});
        for (std::size_t iv = 0; iv < nv; ++iv) {
          const int clo = (*essClovers)[iv];
          if (clo < 0 || clo >= kNClover) continue;
          if ((*bgo)[iv] > 0) hit[clo].bgo = true;
          if ((*csi)[iv] > 0) hit[clo].csi = true;
        }
      }

      for (std::size_t i = 0; i < n; ++i) {
        const int clo = clovers[i];
        const int cri = crystals[i];
        if (clo < 0 || clo >= kNClover || cri < 0 || cri >= kNCrystalPerClover) continue;
        const int det = clo * kNCrystalPerClover + cri;
        const float rawEnergy = energies[i];
        const float rawTime = times[i];
        if (rawEnergy <= 0.0f || rawTime <= 0.0f) continue;

        const CrystalCalib &cc = calib.crystal[det];
        if (!cc.hasEnergy || !cc.hasTime) missingCrystalCalibration++;
        const double energy = cc.energyOffset + cc.energyGain * rawEnergy;
        const double time = ApplyTimeCorrection(rawTime, cc, timeMode);
        if (energy <= 0.0 || time <= 0.0 || !std::isfinite(energy) || !std::isfinite(time)) continue;

        CloverHit &h = hit[clo];
        h.energy += energy;
        h.multiplicity++;
        if (energy > h.bestCrystalEnergy) {
          h.bestCrystalEnergy = energy;
          h.time = time;
        }
      }

      std::vector<float> selectedEnergies;
      std::vector<float> selectedTimes;
      double eventTimeNs = std::numeric_limits<double>::infinity();
      for (int clo = 0; clo < kNClover; ++clo) {
        const CloverHit &h = hit[clo];
        if (h.multiplicity <= 0 || h.energy <= 0.0 || !std::isfinite(h.time)) continue;
        if (useBgoCsiVeto && (h.bgo || h.csi)) continue;
        if (h.time < tCutNs) continue;
        selectedEnergies.push_back(static_cast<float>(h.energy));
        selectedTimes.push_back(static_cast<float>(h.time));
        if (h.time < eventTimeNs) eventTimeNs = h.time;
      }

      if (selectedEnergies.size() < static_cast<std::size_t>(minCloverMultiplicity)) {
        skippedMultiplicity++;
        continue;
      }

      const int binIndex = FindEnergyTimeBin(bins, eventTimeNs);
      if (binIndex < 0) {
        skippedNoBin++;
        continue;
      }

      keptAfterTimeCut++;
      bins[binIndex].selectedEvents++;
      bins[binIndex].selectedGammaFires += static_cast<Long64_t>(selectedEnergies.size());
      for (std::size_t i = 0; i < selectedEnergies.size(); ++i) {
        bins[binIndex].spectrum->Fill(selectedEnergies[i]);
        gammaEnergyVsTime->Fill(selectedEnergies[i], selectedTimes[i]);
      }
      for (std::size_t i = 0; i < selectedEnergies.size(); ++i) {
        for (std::size_t j = i + 1; j < selectedEnergies.size(); ++j) {
          const float a = selectedEnergies[i];
          const float b = selectedEnergies[j];
          bins[binIndex].matrix->Fill(a, b);
          bins[binIndex].matrix->Fill(b, a);
        }
      }
    }
    inputFile.Close();
    if (maxEntries > 0 && totalEntries >= maxEntries) break;
  }

  TString outName = (outputFile && TString(outputFile).Length() > 0) ? TString(outputFile) : TString("fission_event_ana_calibrated.root");
  TFile out(outName, "RECREATE");
  if (!out.IsOpen()) {
    std::cerr << "Cannot create output file: " << outName << std::endl;
    return;
  }

  TDirectory *histDir = out.mkdir("fission_event_ana_calibrated");
  histDir->cd();
  TNamed config("FissionEventAnaCalibratedConfig",
                TString::Format("inputs=%s; calibration=%s; distance_m=%.6g; time_fwhm_ns=%.6g; t_cut_ns=%.6g; energy_edges_MeV=%s; use_bgo_csi_veto=%d; min_clover_multiplicity=%d; time_mode=%s; time_branch=%s",
                                inputFiles, calibrationSummary, nfsDistanceMeter, timeFwhmNs, tCutNs,
                                energyBinEdgesMeV, useBgoCsiVeto ? 1 : 0, minCloverMultiplicity,
                                timeCorrectionMode, timeBranchLeaf).Data());
  config.Write();

  Long64_t acceptedGammaFires = 0;
  for (const auto &bin : bins) acceptedGammaFires += bin.selectedGammaFires;

  TH1F summaryHist("FissionEventAnaSummary", "FissionEventAnaSummary;Category;Counts", 8, 0.5, 8.5);
  summaryHist.GetXaxis()->SetBinLabel(1, "Input entries");
  summaryHist.GetXaxis()->SetBinLabel(2, "Accepted events");
  summaryHist.GetXaxis()->SetBinLabel(3, "Accepted clover fires");
  summaryHist.GetXaxis()->SetBinLabel(4, "Rejected mult/time/veto");
  summaryHist.GetXaxis()->SetBinLabel(5, "Rejected no bin");
  summaryHist.GetXaxis()->SetBinLabel(6, "Malformed");
  summaryHist.GetXaxis()->SetBinLabel(7, "Files without calibration");
  summaryHist.GetXaxis()->SetBinLabel(8, "Crystal calib misses");
  summaryHist.SetBinContent(1, totalEntries);
  summaryHist.SetBinContent(2, keptAfterTimeCut);
  summaryHist.SetBinContent(3, acceptedGammaFires);
  summaryHist.SetBinContent(4, skippedMultiplicity);
  summaryHist.SetBinContent(5, skippedNoBin);
  summaryHist.SetBinContent(6, skippedMalformed);
  summaryHist.SetBinContent(7, missingCalibrationFiles);
  summaryHist.SetBinContent(8, missingCrystalCalibration);
  summaryHist.Write();

  TH1F binEventCounts("FissionEventAnaEventsByEnergyBin", "FissionEventAnaEventsByEnergyBin;Neutron energy bin;Events", bins.size(), 0.5, bins.size() + 0.5);
  TH1F binGammaCounts("FissionEventAnaGammaFiresByEnergyBin", "FissionEventAnaGammaFiresByEnergyBin;Neutron energy bin;Clover fires", bins.size(), 0.5, bins.size() + 0.5);
  for (std::size_t i = 0; i < bins.size(); ++i) {
    binEventCounts.GetXaxis()->SetBinLabel(i + 1, bins[i].tag.Data());
    binGammaCounts.GetXaxis()->SetBinLabel(i + 1, bins[i].tag.Data());
    binEventCounts.SetBinContent(i + 1, bins[i].selectedEvents);
    binGammaCounts.SetBinContent(i + 1, bins[i].selectedGammaFires);
  }
  binEventCounts.Write();
  binGammaCounts.Write();

  gammaEnergyVsTime->Write();
  WriteCanvasForMatrix(histDir, gammaEnergyVsTime, multiplicityTitle);
  for (auto &bin : bins) {
    bin.spectrum->Write();
    bin.matrix->Write();
    WriteCanvasForSpectrum(histDir, bin.spectrum, bin.title);
    WriteCanvasForMatrix(histDir, bin.matrix, bin.title);
  }

  out.Close();

  std::cout << "Processed files: " << processedFiles << std::endl;
  std::cout << "Input entries: " << totalEntries << std::endl;
  std::cout << "t_min(50 MeV): " << tMinNs << " ns" << std::endl;
  std::cout << "t_cut: " << tCutNs << " ns" << std::endl;
  std::cout << "Accepted events: " << keptAfterTimeCut << std::endl;
  std::cout << "Accepted clover fires: " << acceptedGammaFires << std::endl;
  std::cout << "Rejected by multiplicity/time/veto: " << skippedMultiplicity << std::endl;
  std::cout << "Rejected outside energy bins: " << skippedNoBin << std::endl;
  std::cout << "Malformed entries: " << skippedMalformed << std::endl;
  std::cout << "Files without calibration: " << missingCalibrationFiles << std::endl;
  std::cout << "Crystal calibration misses: " << missingCrystalCalibration << std::endl;
  std::cout << "Output: " << outName << std::endl;
}
