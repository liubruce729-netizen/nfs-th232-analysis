// Calibrated triple-gamma TH3D cube builder from selected NFS mult trees.
// 基于筛选后的 NFS mult tree，应用逐 run/逐 crystal 刻度后构建 triple-gamma TH3D。
//
// Usage / 用法:
//   root -l -b -q 'lsy_nfs/triple_gamma_ana_calibrated.C("@mult3_files.txt","calibration_summary.tsv","triple_gamma.root")'
//   root -l -b -q 'lsy_nfs/triple_gamma_ana_calibrated.C("a.root,b.root","calibration_summary.tsv","triple_gamma.root",true,-1,3,256,0,4096,24,"10,15,20,30,40")'
//
// EN: This macro intentionally writes only TH3D triple-gamma cubes: one total cube
//     and one cube for each neutron-energy bin.  It rebuilds clover addback from
//     crystal branches so that per-run/per-crystal energy and time calibration can
//     be applied before the triple-gamma selection.
// CN: 该宏只写出 TH3D 三重 gamma 图：一个总 cube，以及每个中子能量分区一个 cube。
//     它从 crystal 分支重新合并 clover addback，从而在三重 gamma 判选前应用逐 run、
//     逐 crystal 的能量和时间刻度。
//
// Default neutron-energy bins / 默认中子能量分区:
//   0-10, 10-15, 15-20, 20-30, 30-40 MeV, plus Total.
//   0-10、10-15、15-20、20-30、30-40 MeV，另有 Total 总结果。

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
#include "TFile.h"
#include "TH3D.h"
#include "TObjArray.h"
#include "TObjString.h"
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
  for (const auto &item : SplitCsv(energyBinEdgesMeV)) {
    const double edge = item.Atof();
    if (edge > 0.0 && std::isfinite(edge)) edges.push_back(edge);
  }
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

double ToFNsToEnergyMeV(double tofNs, double distanceMeter)
{
  if (tofNs <= 0.0 || distanceMeter <= 0.0) return -1.0;
  const double beta = distanceMeter / (tofNs * kLightSpeedMeterPerNs);
  return 0.5 * kNeutronMassMeV * beta * beta;
}

struct CloverHit {
  double energy = 0.0;
  double time = std::numeric_limits<double>::infinity();
  double bestCrystalEnergy = -1.0;
  bool bgo = false;
  bool csi = false;
  int multiplicity = 0;
};

struct NeutronBin {
  double lowMeV = 0.0;
  double highMeV = 0.0;
  TString tag;
  TH3D *cube = nullptr;
  Long64_t acceptedEvents = 0;
  Long64_t acceptedTriples = 0;
};

std::vector<NeutronBin> MakeNeutronBins(const char *edgesText)
{
  std::vector<NeutronBin> bins;
  double low = 0.0;
  for (double high : ParseEnergyEdges(edgesText)) {
    if (high <= low) continue;
    NeutronBin bin;
    bin.lowMeV = low;
    bin.highMeV = high;
    bin.tag = TString::Format("E%s_%sMeV", FormatNumberForName(low).Data(), FormatNumberForName(high).Data());
    bins.push_back(bin);
    low = high;
  }
  return bins;
}

int FindNeutronBin(const std::vector<NeutronBin> &bins, double neutronEnergyMeV)
{
  for (std::size_t i = 0; i < bins.size(); ++i) {
    if (neutronEnergyMeV >= bins[i].lowMeV && neutronEnergyMeV < bins[i].highMeV) return static_cast<int>(i);
  }
  return -1;
}

double EstimateTH3DStorageGiB(int energyBins, std::size_t cubeCount)
{
  const double n = static_cast<double>(energyBins) + 2.0;
  const double bytes = n * n * n * static_cast<double>(sizeof(Double_t)) * static_cast<double>(cubeCount);
  return bytes / 1024.0 / 1024.0 / 1024.0;
}

TH3D *MakeCube(const TString &name, const TString &title, int bins, double emin, double emax)
{
  TH3D *hist = new TH3D(name, title, bins, emin, emax, bins, emin, emax, bins, emin, emax);
  hist->GetXaxis()->SetTitle("Calibrated gamma energy 1 (keV)");
  hist->GetYaxis()->SetTitle("Calibrated gamma energy 2 (keV)");
  hist->GetZaxis()->SetTitle("Calibrated gamma energy 3 (keV)");
  return hist;
}

void FillOrderedTriple(TH3D *cube, double a, double b, double c)
{
  cube->Fill(a, b, c);
  cube->Fill(a, c, b);
  cube->Fill(b, a, c);
  cube->Fill(b, c, a);
  cube->Fill(c, a, b);
  cube->Fill(c, b, a);
}

void FillAllTriples(TH3D *cube, const std::vector<double> &energies)
{
  for (std::size_t i = 0; i < energies.size(); ++i) {
    for (std::size_t j = i + 1; j < energies.size(); ++j) {
      for (std::size_t k = j + 1; k < energies.size(); ++k) {
        FillOrderedTriple(cube, energies[i], energies[j], energies[k]);
      }
    }
  }
}

Long64_t CountUnorderedTriples(std::size_t n)
{
  if (n < 3) return 0;
  return static_cast<Long64_t>(n * (n - 1) * (n - 2) / 6);
}

} // namespace

void triple_gamma_ana_calibrated(const char *inputFiles = "out/mult3_nfs_run_100_r0.root",
                                 const char *calibrationSummary = "calibration_summary.tsv",
                                 const char *outputFile = "triple_gamma_ana_calibrated.root",
                                 bool useBgoCsiVeto = true,
                                 Long64_t maxEntries = -1,
                                 int minCloverMultiplicity = 3,
                                 int energyBins = 256,
                                 double energyMinKeV = 0.0,
                                 double energyMaxKeV = 4096.0,
                                 double nfsDistanceMeter = 24.0,
                                 const char *neutronEnergyEdgesMeV = "10,15,20,30,40",
                                 const char *timeCorrectionMode = "offset",
                                 const char *timeBranchLeaf = "fTime",
                                 double maxCubeMemoryGiB = 12.0)
{
  if (minCloverMultiplicity < 3 || energyBins <= 0 || energyMaxKeV <= energyMinKeV || nfsDistanceMeter <= 0.0) {
    std::cerr << "Invalid triple-gamma calibrated analysis parameters." << std::endl;
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

  std::vector<NeutronBin> neutronBins = MakeNeutronBins(neutronEnergyEdgesMeV);
  if (neutronBins.empty()) {
    std::cerr << "No valid neutron-energy edges were provided: " << neutronEnergyEdgesMeV << std::endl;
    return;
  }

  const std::size_t cubeCount = neutronBins.size() + 1;
  const double estimatedGiB = EstimateTH3DStorageGiB(energyBins, cubeCount);
  if (estimatedGiB > maxCubeMemoryGiB) {
    std::cerr << "Requested TH3D allocation is too large." << std::endl;
    std::cerr << "  energyBins=" << energyBins << " cubes=" << cubeCount
              << " estimated_storage=" << estimatedGiB << " GiB"
              << " limit=" << maxCubeMemoryGiB << " GiB" << std::endl;
    return;
  }

  TH3D *totalCube = MakeCube("TripleGammaCube_Total",
                             "Calibrated triple-gamma cube Total;Calibrated gamma energy 1 (keV);Calibrated gamma energy 2 (keV);Calibrated gamma energy 3 (keV)",
                             energyBins, energyMinKeV, energyMaxKeV);
  for (auto &bin : neutronBins) {
    bin.cube = MakeCube(TString::Format("TripleGammaCube_%s", bin.tag.Data()),
                        TString::Format("Calibrated triple-gamma cube %s;Calibrated gamma energy 1 (keV);Calibrated gamma energy 2 (keV);Calibrated gamma energy 3 (keV)", bin.tag.Data()),
                        energyBins, energyMinKeV, energyMaxKeV);
  }

  Long64_t totalEntries = 0;
  Long64_t processedFiles = 0;
  Long64_t acceptedEvents = 0;
  Long64_t neutronBinnedEvents = 0;
  Long64_t acceptedTriples = 0;
  Long64_t skippedMultiplicity = 0;
  Long64_t skippedMalformed = 0;
  Long64_t skippedNoNeutronBin = 0;
  Long64_t missingCalibrationFiles = 0;
  Long64_t missingCrystalCalibration = 0;
  Long64_t vetoedFires = 0;
  Long64_t outOfRangeFires = 0;
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

    while (reader.Next()) {
      if (maxEntries > 0 && totalEntries >= maxEntries) break;
      totalEntries++;

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

      std::vector<double> selectedEnergies;
      selectedEnergies.reserve(kNClover);
      double eventTimeNs = std::numeric_limits<double>::infinity();
      for (int clo = 0; clo < kNClover; ++clo) {
        const CloverHit &h = hit[clo];
        if (h.multiplicity <= 0 || h.energy <= 0.0 || !std::isfinite(h.time)) continue;
        if (useBgoCsiVeto && (h.bgo || h.csi)) {
          vetoedFires++;
          continue;
        }
        if (h.energy < energyMinKeV || h.energy >= energyMaxKeV) {
          outOfRangeFires++;
          continue;
        }
        selectedEnergies.push_back(h.energy);
        if (h.time < eventTimeNs) eventTimeNs = h.time;
      }

      if (selectedEnergies.size() < static_cast<std::size_t>(minCloverMultiplicity)) {
        skippedMultiplicity++;
        continue;
      }

      const double neutronEnergyMeV = ToFNsToEnergyMeV(eventTimeNs, nfsDistanceMeter);
      const int neutronBinIndex = FindNeutronBin(neutronBins, neutronEnergyMeV);
      const Long64_t triplesThisEvent = CountUnorderedTriples(selectedEnergies.size());

      FillAllTriples(totalCube, selectedEnergies);
      acceptedEvents++;
      acceptedTriples += triplesThisEvent;

      if (neutronBinIndex >= 0) {
        FillAllTriples(neutronBins[neutronBinIndex].cube, selectedEnergies);
        neutronBins[neutronBinIndex].acceptedEvents++;
        neutronBins[neutronBinIndex].acceptedTriples += triplesThisEvent;
        neutronBinnedEvents++;
      } else {
        skippedNoNeutronBin++;
      }
    }
    inputFile.Close();
    if (maxEntries > 0 && totalEntries >= maxEntries) break;
  }

  TString outName = outputFile && TString(outputFile).Length() > 0 ? TString(outputFile) : TString("triple_gamma_ana_calibrated.root");
  TFile out(outName, "RECREATE");
  if (!out.IsOpen()) {
    std::cerr << "Cannot create output file: " << outName << std::endl;
    return;
  }
  TDirectory *dir = out.mkdir("triple_gamma_ana_calibrated");
  dir->cd();

  // EN/CN: Deliberately write only the requested triple-gamma TH3D cubes.
  totalCube->Write();
  for (auto &bin : neutronBins) {
    if (bin.cube) bin.cube->Write();
  }
  out.Close();

  std::cout << "Processed files: " << processedFiles << std::endl;
  std::cout << "Input entries: " << totalEntries << std::endl;
  std::cout << "Accepted events: " << acceptedEvents << std::endl;
  std::cout << "Neutron-binned events: " << neutronBinnedEvents << std::endl;
  std::cout << "Accepted unordered triples: " << acceptedTriples << std::endl;
  std::cout << "TH3D filled permutations in total cube: " << acceptedTriples * 6 << std::endl;
  std::cout << "Rejected by multiplicity: " << skippedMultiplicity << std::endl;
  std::cout << "Rejected outside neutron bins: " << skippedNoNeutronBin << std::endl;
  std::cout << "Malformed entries: " << skippedMalformed << std::endl;
  std::cout << "Files without calibration: " << missingCalibrationFiles << std::endl;
  std::cout << "Crystal calibration misses: " << missingCrystalCalibration << std::endl;
  std::cout << "Vetoed clover fires: " << vetoedFires << std::endl;
  std::cout << "Out-of-range clover fires: " << outOfRangeFires << std::endl;
  std::cout << "Estimated TH3D storage: " << estimatedGiB << " GiB" << std::endl;
  for (const auto &bin : neutronBins) {
    std::cout << "  " << bin.tag << ": events=" << bin.acceptedEvents
              << " triples=" << bin.acceptedTriples << std::endl;
  }
  std::cout << "Output: " << outName << std::endl;
}
