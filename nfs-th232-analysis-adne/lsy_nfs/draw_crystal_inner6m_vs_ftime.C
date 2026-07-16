// ROOT macro: draw raw InnerM6 versus fTime for every EXOGAM crystal.
// ROOT 宏：为每个 EXOGAM crystal 绘制原始 InnerM6 与 fTime 二维图。
//
// No physics cut is applied. Zero InnerM6 and negative fTime are retained.
// 不应用物理 cut；InnerM6=0 和负 fTime 都会保留。
//
// The primitive MfmFrameTree currently has exo_delta_t but no stored fTime.
// In that case this macro uses the same NFS conversion as ADNE:
//   fTime = (65536 - DeltaT) * 0.024 + gammaFlashOffset
// If a scalar fTime branch exists, it is read directly instead.
//
// Example / 示例:
// root -l -b -q \
//   'draw_crystal_inner6m_vs_ftime.C("run134_0.root","inner6m_vs_ftime.root")'

#include <TBranch.h>
#include <TChain.h>
#include <TFile.h>
#include <TH2F.h>
#include <TLeaf.h>
#include <TNamed.h>
#include <TSystem.h>
#include <TTree.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr int kCloverCount = 16;
constexpr int kCrystalCount = 4;
constexpr int kDetectorCount = kCloverCount * kCrystalCount;
constexpr double kDefaultTimeGainNs = 0.024;

struct Detector {
  int clover = -1;
  int crystal = -1;
  bool valid = false;
};

struct TimeCoefficient {
  double relativeOffset = 0.0;
  double relativeGain = 1.0;
  bool valid = false;
};

std::string Trim(const std::string &value) {
  const std::size_t first = value.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) return "";
  const std::size_t last = value.find_last_not_of(" \t\r\n");
  return value.substr(first, last - first + 1);
}

std::string StripComment(const std::string &value) {
  std::size_t pos = value.find('#');
  const std::size_t slash = value.find("//");
  if (slash != std::string::npos) pos = std::min(pos, slash);
  return Trim(value.substr(0, pos));
}

void AddInputToken(const std::string &token, std::vector<std::string> &files) {
  const std::string item = Trim(token);
  if (item.empty() || item[0] == '#') return;

  if (item[0] == '@') {
    std::ifstream list(item.substr(1));
    if (!list) throw std::runtime_error("Cannot open input list: " + item.substr(1));
    std::string line;
    while (std::getline(list, line)) AddInputToken(StripComment(line), files);
    return;
  }

  std::size_t begin = 0;
  while (begin <= item.size()) {
    const std::size_t comma = item.find(',', begin);
    const std::string path = Trim(item.substr(begin, comma - begin));
    if (!path.empty()) files.push_back(path);
    if (comma == std::string::npos) break;
    begin = comma + 1;
  }
}

std::vector<std::string> ExpandInputs(const char *inputSpec) {
  if (!inputSpec || !*inputSpec) throw std::runtime_error("Input ROOT file is empty");
  std::vector<std::string> files;
  AddInputToken(inputSpec, files);
  if (files.empty()) throw std::runtime_error("No input ROOT files were found");
  return files;
}

std::string FindDefaultLut() {
  const std::array<const char *, 4> candidates = {
      "nfs-th232-analysis-adne/Conf/Exogam2_in_Tree.cfg",
      "../nfs-th232-analysis-adne/Conf/Exogam2_in_Tree.cfg",
      "Conf/Exogam2_in_Tree.cfg",
      "Exogam2_in_Tree.cfg"};
  for (const char *candidate : candidates) {
    if (!gSystem->AccessPathName(candidate, kReadPermission)) return candidate;
  }
  throw std::runtime_error(
      "Cannot find Exogam2_in_Tree.cfg; pass its path as the third argument");
}

std::map<std::pair<int, int>, Detector> LoadLut(const std::string &path) {
  std::ifstream input(path);
  if (!input) throw std::runtime_error("Cannot open LUT: " + path);

  std::map<std::pair<int, int>, Detector> lut;
  std::string line;
  while (std::getline(input, line)) {
    line = StripComment(line);
    if (line.empty()) continue;
    if (line.rfind("END", 0) == 0) break;

    int clover = -1;
    int crystal = -1;
    int board = -1;
    int halfBoard = -1;
    std::istringstream parser(line);
    if (!(parser >> clover >> crystal >> board >> halfBoard)) continue;
    if (clover < 0 || clover >= kCloverCount ||
        crystal < 0 || crystal >= kCrystalCount ||
        board < 0 || halfBoard < 0 || halfBoard > 1) {
      throw std::runtime_error("Invalid LUT row: " + line);
    }
    lut[{board, halfBoard}] = {clover, crystal, true};
  }
  if (lut.empty()) throw std::runtime_error("No detector mapping found in LUT: " + path);
  return lut;
}

std::array<TimeCoefficient, kDetectorCount> LoadTimeCalibration(
    const std::string &path) {
  std::ifstream input(path);
  if (!input) throw std::runtime_error("Cannot open time calibration file: " + path);

  std::vector<std::array<double, 3>> rows;
  std::string line;
  while (std::getline(input, line)) {
    line = StripComment(line);
    if (line.empty()) continue;
    std::array<double, 3> row{};
    std::istringstream parser(line);
    if (parser >> row[0] >> row[1] >> row[2]) rows.push_back(row);
  }
  if (rows.size() < 2 * kDetectorCount) {
    throw std::runtime_error(
        "Time correction needs at least 128 coefficient rows in ecc.cal");
  }

  std::array<TimeCoefficient, kDetectorCount> result{};
  for (int id = 0; id < kDetectorCount; ++id) {
    const auto &row = rows[kDetectorCount + id];
    result[id].relativeOffset = row[0];
    result[id].relativeGain = row[1];
    result[id].valid = (row[0] != 0.0 || row[1] != 0.0);
  }
  return result;
}

double CalculateFTime(int rawDeltaT, int detectorId, double globalOffset,
                      bool useCorrection,
                      const std::array<TimeCoefficient, kDetectorCount> &calibration) {
  const double reversed = 65536.0 - static_cast<double>(rawDeltaT);
  if (useCorrection && detectorId >= 0 && detectorId < kDetectorCount &&
      calibration[detectorId].valid) {
    return reversed * kDefaultTimeGainNs * calibration[detectorId].relativeGain +
           globalOffset + calibration[detectorId].relativeOffset;
  }
  return reversed * kDefaultTimeGainNs + globalOffset;
}

std::pair<double, double> FindCalculatedTimeRange(
    const std::map<std::pair<int, int>, Detector> &lut, double globalOffset,
    bool useCorrection,
    const std::array<TimeCoefficient, kDetectorCount> &calibration) {
  double minimum = std::numeric_limits<double>::infinity();
  double maximum = -std::numeric_limits<double>::infinity();
  for (const auto &entry : lut) {
    const int id = entry.second.clover * kCrystalCount + entry.second.crystal;
    for (int raw : {0, 65535}) {
      const double value =
          CalculateFTime(raw, id, globalOffset, useCorrection, calibration);
      minimum = std::min(minimum, value);
      maximum = std::max(maximum, value);
    }
  }
  const double span = std::max(1.0, maximum - minimum);
  return {minimum - 0.01 * span, maximum + 0.01 * span};
}

}  // namespace

void draw_crystal_inner6m_vs_ftime(
    const char *inputSpec,
    const char *outputFile = "crystal_inner6m_vs_ftime.root",
    const char *lutFile = "",
    double gammaFlashOffsetNs = -700.4,
    bool crystalTimeCorrection = false,
    const char *correctionFile = "",
    Long64_t maxRows = -1,
    int timeBins = 800,
    int energyBins = 4096,
    double energyMax = 65536.0) {
  try {
    if (!outputFile || !*outputFile) throw std::runtime_error("Output path is empty");
    if (timeBins <= 0 || energyBins <= 0 || energyMax <= 0.0) {
      throw std::runtime_error("Histogram bin counts and energyMax must be positive");
    }

    const std::vector<std::string> inputs = ExpandInputs(inputSpec);
    const std::string lutPath = (lutFile && *lutFile) ? lutFile : FindDefaultLut();
    const auto lut = LoadLut(lutPath);

    std::array<TimeCoefficient, kDetectorCount> timeCalibration{};
    if (crystalTimeCorrection) {
      if (!correctionFile || !*correctionFile) {
        throw std::runtime_error(
            "crystalTimeCorrection=true requires an ecc.cal correction path");
      }
      timeCalibration = LoadTimeCalibration(correctionFile);
    }

    TChain chain("MfmFrameTree");
    for (const std::string &path : inputs) {
      if (chain.Add(path.c_str()) == 0) {
        throw std::runtime_error("Cannot add input ROOT file: " + path);
      }
    }

    const std::array<const char *, 4> alwaysRequired = {
        "is_exo2", "exo_board_id", "exo_lut_halfboard", "exo_inner6m"};
    for (const char *name : alwaysRequired) {
      if (!chain.GetBranch(name)) {
        throw std::runtime_error(std::string("Missing MfmFrameTree branch: ") + name);
      }
    }

    const bool hasStoredFTime = chain.GetBranch("fTime") != nullptr;
    if (!hasStoredFTime && !chain.GetBranch("exo_delta_t")) {
      throw std::runtime_error(
          "Tree has neither scalar fTime nor exo_delta_t for calculating fTime");
    }
    Bool_t isExo2 = false;
    Int_t board = -1;
    Int_t halfBoard = -1;
    Int_t innerM6 = -1;
    Int_t rawDeltaT = -1;
    Float_t storedFTimeFloat = 0.0F;
    Double_t storedFTimeDouble = 0.0;
    bool storedFTimeIsFloat = false;

    chain.SetBranchStatus("*", 0);
    chain.SetBranchStatus("is_exo2", 1);
    chain.SetBranchStatus("exo_board_id", 1);
    chain.SetBranchStatus("exo_lut_halfboard", 1);
    chain.SetBranchStatus("exo_inner6m", 1);
    if (hasStoredFTime) chain.SetBranchStatus("fTime", 1);
    else chain.SetBranchStatus("exo_delta_t", 1);

    chain.SetBranchAddress("is_exo2", &isExo2);
    chain.SetBranchAddress("exo_board_id", &board);
    chain.SetBranchAddress("exo_lut_halfboard", &halfBoard);
    chain.SetBranchAddress("exo_inner6m", &innerM6);
    if (hasStoredFTime) {
      TLeaf *leaf = chain.GetBranch("fTime")->GetLeaf("fTime");
      if (!leaf) throw std::runtime_error("Cannot inspect scalar fTime leaf");
      const std::string type = leaf->GetTypeName();
      if (type == "Float_t" || type == "float") {
        storedFTimeIsFloat = true;
        chain.SetBranchAddress("fTime", &storedFTimeFloat);
      } else if (type == "Double_t" || type == "double") {
        chain.SetBranchAddress("fTime", &storedFTimeDouble);
      } else {
        throw std::runtime_error("Unsupported scalar fTime type: " + type);
      }
    } else {
      chain.SetBranchAddress("exo_delta_t", &rawDeltaT);
    }

    chain.SetCacheSize(128LL * 1024LL * 1024LL);
    chain.AddBranchToCache("*", true);
    chain.StopCacheLearningPhase();

    std::pair<double, double> timeRange;
    if (hasStoredFTime) {
      timeRange = {chain.GetMinimum("fTime"), chain.GetMaximum("fTime")};
      if (!std::isfinite(timeRange.first) || !std::isfinite(timeRange.second) ||
          timeRange.second <= timeRange.first) {
        timeRange = {-1000.0, 2000.0};
      } else {
        const double span = std::max(1.0, timeRange.second - timeRange.first);
        timeRange.first -= 0.01 * span;
        timeRange.second += 0.01 * span;
      }
    } else {
      timeRange = FindCalculatedTimeRange(
          lut, gammaFlashOffsetNs, crystalTimeCorrection, timeCalibration);
    }

    std::array<std::unique_ptr<TH2F>, kDetectorCount> histograms;
    for (const auto &entry : lut) {
      const Detector &detector = entry.second;
      const int id = detector.clover * kCrystalCount + detector.crystal;
      if (histograms[id]) continue;

      const std::string name =
          "nfs_raw_clover" + std::to_string(detector.clover) +
          "_crystal" + std::to_string(detector.crystal) +
          "_inner6m_vs_ftime";
      const std::string title =
          "Clover" + std::to_string(detector.clover) +
          " Crystal" + std::to_string(detector.crystal) +
          " raw InnerM6 vs fTime;fTime (ns);InnerM6 (ADC channel)";
      histograms[id] = std::make_unique<TH2F>(
          name.c_str(), title.c_str(),
          timeBins, timeRange.first, timeRange.second,
          energyBins, 0.0, energyMax);
      histograms[id]->SetDirectory(nullptr);
    }

    Long64_t rowsRead = 0;
    Long64_t exo2Rows = 0;
    Long64_t filledRows = 0;
    Long64_t unknownLutRows = 0;
    Long64_t malformedRows = 0;
    const Long64_t availableRows = chain.GetEntries();
    const Long64_t rowsToRead =
        (maxRows >= 0) ? std::min(maxRows, availableRows) : availableRows;

    for (Long64_t row = 0; row < rowsToRead; ++row) {
      if (chain.GetEntry(row) <= 0) continue;
      ++rowsRead;
      if (!isExo2) continue;
      ++exo2Rows;

      const auto found = lut.find({board, halfBoard});
      if (found == lut.end()) {
        ++unknownLutRows;
        continue;
      }
      if (innerM6 < 0 || (!hasStoredFTime && (rawDeltaT < 0 || rawDeltaT > 65535))) {
        ++malformedRows;
        continue;
      }

      const Detector &detector = found->second;
      const int id = detector.clover * kCrystalCount + detector.crystal;
      const double fTime =
          hasStoredFTime
              ? (storedFTimeIsFloat ? static_cast<double>(storedFTimeFloat)
                                    : storedFTimeDouble)
              : CalculateFTime(rawDeltaT, id, gammaFlashOffsetNs,
                               crystalTimeCorrection, timeCalibration);
      histograms[id]->Fill(fTime, static_cast<double>(innerM6));
      ++filledRows;

      if ((row + 1) % 5000000 == 0) {
        std::cout << "Processed " << (row + 1) << "/" << rowsToRead << " rows\n";
      }
    }

    TFile output(outputFile, "RECREATE", "Raw InnerM6 versus fTime", 4);
    if (output.IsZombie()) throw std::runtime_error("Cannot create output ROOT file");

    output.mkdir("Crystal_InnerM6_vs_fTime");
    output.cd("Crystal_InnerM6_vs_fTime");
    for (auto &histogram : histograms) {
      if (histogram) histogram->Write();
    }

    output.cd();
    TTree summary("processing_summary", "No-cut InnerM6 versus fTime counters");
    summary.Branch("rowsRead", &rowsRead);
    summary.Branch("exo2Rows", &exo2Rows);
    summary.Branch("filledRows", &filledRows);
    summary.Branch("unknownLutRows", &unknownLutRows);
    summary.Branch("malformedRows", &malformedRows);
    summary.Fill();
    summary.Write();

    std::ostringstream configuration;
    configuration << "lut=" << lutPath
                  << ";stored_fTime=" << (hasStoredFTime ? 1 : 0)
                  << ";gamma_flash_offset_ns=" << gammaFlashOffsetNs
                  << ";crystal_time_correction=" << (crystalTimeCorrection ? 1 : 0)
                  << ";time_bins=" << timeBins
                  << ";energy_bins=" << energyBins
                  << ";energy_max=" << energyMax
                  << ";cuts=none";
    TNamed config("configuration", configuration.str().c_str());
    config.Write();
    output.Close();

    std::cout << "\nInput files: " << inputs.size()
              << "\nLUT: " << lutPath
              << "\nfTime source: "
              << (hasStoredFTime ? "stored fTime branch" : "calculated from exo_delta_t")
              << "\nRows read: " << rowsRead
              << "\nEXO2 rows: " << exo2Rows
              << "\nFilled rows: " << filledRows
              << "\nUnknown LUT rows: " << unknownLutRows
              << "\nMalformed rows: " << malformedRows
              << "\nOutput: " << outputFile << '\n';
  } catch (const std::exception &error) {
    std::cerr << "draw_crystal_inner6m_vs_ftime: " << error.what() << '\n';
  }
}
