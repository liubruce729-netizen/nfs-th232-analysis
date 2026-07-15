#include <TBranch.h>
#include <TFile.h>
#include <TH1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TNamed.h>
#include <TProfile.h>
#include <TRandom3.h>
#include <TTree.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// EN: Standalone reproduction of the histograms written to nfs_histoExogam2_1.root.
//     It reads the primitive MfmFrameTree produced by mfm_to_raw_root_tree.C and does
//     not link against ADNE, GRU, MFMlib, or yaml-cpp.
// CN: 独立复现 nfs_histoExogam2_1.root 中的直方图。输入是
//     mfm_to_raw_root_tree.C 生成的基础 MfmFrameTree，不链接 ADNE、GRU、MFMlib
//     或 yaml-cpp。

namespace {

constexpr int kClovers = 16;
constexpr int kCrystalsPerClover = 4;
constexpr int kCrystals = kClovers * kCrystalsPerClover;
constexpr double kTimestampTickNs = 10.0;
constexpr double kDefaultTdcGainNs = 0.024;
constexpr double kTdcChannels = 65536.0;
constexpr double kTdcPeriodNs = kTdcChannels * kDefaultTdcGainNs;

std::string Trim(const std::string &value) {
  const auto first = value.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) return "";
  const auto last = value.find_last_not_of(" \t\r\n");
  return value.substr(first, last - first + 1);
}

std::string ToLower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return value;
}

std::string Unquote(std::string value) {
  value = Trim(value);
  if (value.size() >= 2 &&
      ((value.front() == '"' && value.back() == '"') ||
       (value.front() == '\'' && value.back() == '\''))) {
    return value.substr(1, value.size() - 2);
  }
  return value;
}

std::string StripYamlComment(const std::string &line) {
  bool singleQuoted = false;
  bool doubleQuoted = false;
  for (std::size_t i = 0; i < line.size(); ++i) {
    if (line[i] == '\'' && !doubleQuoted) singleQuoted = !singleQuoted;
    if (line[i] == '"' && !singleQuoted) doubleQuoted = !doubleQuoted;
    if (line[i] == '#' && !singleQuoted && !doubleQuoted) return line.substr(0, i);
  }
  return line;
}

bool ParseBool(const std::string &text) {
  const std::string value = ToLower(Trim(text));
  if (value == "true" || value == "yes" || value == "1" || value == "on") return true;
  if (value == "false" || value == "no" || value == "0" || value == "off") return false;
  throw std::runtime_error("invalid boolean value: " + text);
}

std::vector<std::string> Split(const std::string &text, char delimiter) {
  std::vector<std::string> result;
  std::stringstream stream(text);
  std::string token;
  while (std::getline(stream, token, delimiter)) {
    token = Trim(token);
    if (!token.empty()) result.push_back(token);
  }
  return result;
}

struct Coefficients {
  double offset = 0.0;
  double gain = 1.0;
  double gain2 = 0.0;
};

struct RuntimeConfig {
  std::array<bool, kClovers> activeClovers{};
  std::array<bool, kCrystals> enabledCrystals{};
  bool activeCloversConfigured = false;
  bool readCalibrationFile = true;
  bool useDefaultEnergyCalibration = false;
  bool neutronNfs = true;
  bool crystalTimeCorrection = false;
  double gammaFlashOffsetNs = -700.4;
  std::string timeCorrectionPath;

  RuntimeConfig() { enabledCrystals.fill(true); }
};

struct Options {
  std::vector<std::string> inputs;
  std::string output = "nfs_histoExogam2_1.root";
  std::string yamlConfig;
  std::string lutPath;
  std::string energyCalibrationPath;
  std::optional<std::string> timeCalibrationPathOverride;
  std::optional<std::string> activeCloversOverride;
  std::optional<std::string> disabledCrystalsOverride;
  std::optional<double> gammaFlashOffsetOverride;
  std::optional<bool> crystalTimeCorrectionOverride;
  bool dither = true;
  unsigned int randomSeed = 4357;
  Long64_t startEvents = 0;
  Long64_t maxEvents = 0;
  Long64_t progressEntries = 5000000;
};

void PrintUsage(const char *program) {
  std::cout
      << "Usage / 用法:\n"
      << "  " << program << " --input FILE.root --output nfs_histoExogam2_1.root \\\n"
      << "      --config Yaml_config_files/config.yaml \\\n"
      << "      --lut Conf/Exogam2_in_Tree.cfg --energy-cal CalFile/ecc.cal\n\n"
      << "Required / 必需:\n"
      << "  --input FILE              Input ROOT; repeatable. @LIST is accepted.\n"
      << "                            输入 ROOT，可重复；也支持 @文件列表。\n"
      << "  --input-list LIST         One input ROOT path per line. / 每行一个输入。\n"
      << "  --lut FILE                ADNE EXOGAM board-to-crystal LUT.\n"
      << "  --energy-cal FILE         ecc.cal energy coefficients.\n\n"
      << "Main options / 主要选项:\n"
      << "  --output FILE             Output ROOT (default nfs_histoExogam2_1.root).\n"
      << "  --config FILE             Read relevant settings from ADNE config.yaml.\n"
      << "  --time-cal FILE           Override the NFS time-correction ecc.cal.\n"
      << "  --active-clovers EXPR     Example: 2-13 or 2,3,7-10.\n"
      << "  --disabled-crystals EXPR  Example: 7-1,10-3.\n"
      << "  --gamma-flash-offset NS   Override nfs_gammaFlashOffset.\n"
      << "  --time-correction BOOL    Override crystal_time_correction.\n"
      << "  --dither BOOL             ADNE-like +/-0.5 ADC-channel dither (default true).\n"
      << "  --random-seed N           Dither seed (default 4357, ROOT default here).\n"
      << "  --start-events N          Skip N top events before processing.\n"
      << "  --max-events N            Process at most N top events; 0 means all.\n"
      << "  --progress-entries N      Progress interval in MfmFrameTree rows.\n"
      << "  -h, --help                Show this help.\n";
}

void AppendInputList(const std::string &path, std::vector<std::string> &inputs) {
  std::ifstream stream(path);
  if (!stream) throw std::runtime_error("cannot open input list: " + path);
  std::string line;
  const std::filesystem::path listDirectory =
      std::filesystem::absolute(std::filesystem::path(path)).parent_path();
  while (std::getline(stream, line)) {
    line = Trim(StripYamlComment(line));
    if (line.empty()) continue;
    std::filesystem::path item = Unquote(line);
    if (item.is_relative()) item = listDirectory / item;
    inputs.push_back(item.lexically_normal().string());
  }
}

Options ParseArguments(int argc, char **argv) {
  Options options;
  auto requireValue = [&](int &index, const std::string &name) -> std::string {
    if (index + 1 >= argc) throw std::runtime_error("missing value after " + name);
    return argv[++index];
  };

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      PrintUsage(argv[0]);
      std::exit(0);
    } else if (arg == "--input") {
      const std::string value = requireValue(i, arg);
      if (!value.empty() && value.front() == '@') AppendInputList(value.substr(1), options.inputs);
      else options.inputs.push_back(value);
    } else if (arg == "--input-list") {
      AppendInputList(requireValue(i, arg), options.inputs);
    } else if (arg == "--output") {
      options.output = requireValue(i, arg);
    } else if (arg == "--config") {
      options.yamlConfig = requireValue(i, arg);
    } else if (arg == "--lut") {
      options.lutPath = requireValue(i, arg);
    } else if (arg == "--energy-cal") {
      options.energyCalibrationPath = requireValue(i, arg);
    } else if (arg == "--time-cal") {
      options.timeCalibrationPathOverride = requireValue(i, arg);
    } else if (arg == "--active-clovers") {
      options.activeCloversOverride = requireValue(i, arg);
    } else if (arg == "--disabled-crystals") {
      options.disabledCrystalsOverride = requireValue(i, arg);
    } else if (arg == "--gamma-flash-offset") {
      options.gammaFlashOffsetOverride = std::stod(requireValue(i, arg));
    } else if (arg == "--time-correction") {
      options.crystalTimeCorrectionOverride = ParseBool(requireValue(i, arg));
    } else if (arg == "--dither") {
      options.dither = ParseBool(requireValue(i, arg));
    } else if (arg == "--random-seed") {
      options.randomSeed = static_cast<unsigned int>(std::stoul(requireValue(i, arg)));
    } else if (arg == "--start-events") {
      options.startEvents = std::stoll(requireValue(i, arg));
    } else if (arg == "--max-events") {
      options.maxEvents = std::stoll(requireValue(i, arg));
    } else if (arg == "--progress-entries") {
      options.progressEntries = std::stoll(requireValue(i, arg));
    } else {
      throw std::runtime_error("unknown option: " + arg);
    }
  }

  if (options.inputs.empty()) throw std::runtime_error("at least one --input is required");
  if (options.lutPath.empty()) throw std::runtime_error("--lut is required");
  if (options.energyCalibrationPath.empty()) throw std::runtime_error("--energy-cal is required");
  if (options.startEvents < 0 || options.maxEvents < 0) {
    throw std::runtime_error("--start-events and --max-events must be non-negative");
  }

  std::vector<std::string> uniqueInputs;
  std::set<std::string> seen;
  for (const auto &input : options.inputs) {
    const std::string normalized = std::filesystem::absolute(input).lexically_normal().string();
    if (seen.insert(normalized).second) uniqueInputs.push_back(normalized);
  }
  options.inputs.swap(uniqueInputs);
  return options;
}

std::pair<std::string, std::string> ParseYamlKeyValue(const std::string &line) {
  const auto colon = line.find(':');
  if (colon == std::string::npos) return {Trim(line), ""};
  return {Trim(line.substr(0, colon)), Trim(line.substr(colon + 1))};
}

void DisableCrystalLabel(const std::string &label, RuntimeConfig &config) {
  const std::string cleaned = Unquote(Trim(label));
  const auto dash = cleaned.find('-');
  if (dash == std::string::npos) return;
  try {
    const int clover = std::stoi(cleaned.substr(0, dash));
    const int crystal = std::stoi(cleaned.substr(dash + 1));
    if (clover >= 0 && clover < kClovers && crystal >= 0 && crystal < kCrystalsPerClover) {
      config.enabledCrystals[clover * kCrystalsPerClover + crystal] = false;
    }
  } catch (...) {
    // EN/CN: Ignore malformed labels exactly as a non-matching YAML item.
  }
}

void ParseDisabledCrystalExpression(std::string expression, RuntimeConfig &config) {
  for (char &c : expression) {
    if (c == '[' || c == ']' || c == '"' || c == '\'') c = ' ';
  }
  for (const auto &item : Split(expression, ',')) DisableCrystalLabel(item, config);
}

void ParseActiveClovers(const std::string &expression, RuntimeConfig &config) {
  config.activeClovers.fill(false);
  for (const auto &item : Split(expression, ',')) {
    const auto dash = item.find('-');
    int first = 0;
    int last = 0;
    if (dash == std::string::npos) {
      first = last = std::stoi(item);
    } else {
      first = std::stoi(item.substr(0, dash));
      last = std::stoi(item.substr(dash + 1));
      if (last < first) std::swap(first, last);
    }
    for (int clover = first; clover <= last; ++clover) {
      if (clover < 0 || clover >= kClovers) {
        throw std::runtime_error("active clover outside 0..15: " + std::to_string(clover));
      }
      config.activeClovers[clover] = true;
    }
  }
  config.activeCloversConfigured = true;
}

void LoadRelevantYaml(const std::string &path, RuntimeConfig &config) {
  if (path.empty()) return;
  std::ifstream stream(path);
  if (!stream) throw std::runtime_error("cannot open YAML config: " + path);

  std::string topSection;
  std::string guserSection;
  bool disabledBlock = false;
  std::string line;
  while (std::getline(stream, line)) {
    line = StripYamlComment(line);
    if (Trim(line).empty()) continue;
    const auto first = line.find_first_not_of(' ');
    const std::size_t indent = first == std::string::npos ? 0 : first;
    const std::string content = Trim(line);
    const auto [key, rawValue] = ParseYamlKeyValue(content);

    if (indent == 0) {
      topSection = key;
      guserSection.clear();
      disabledBlock = false;
      continue;
    }

    if (topSection == "Guser" && indent == 2 && rawValue.empty()) {
      guserSection = key;
      continue;
    }

    if (topSection == "nfs_exo_ana") {
      if (indent == 2) {
        disabledBlock = (key == "disabled_crystals" && rawValue.empty());
        if (key == "crystal_time_correction") config.crystalTimeCorrection = ParseBool(rawValue);
        else if (key == "correction_path") config.timeCorrectionPath = Unquote(rawValue);
        else if (key == "disabled_crystals" && !rawValue.empty()) {
          ParseDisabledCrystalExpression(rawValue, config);
        }
      } else if (disabledBlock && indent >= 4 && content.rfind("-", 0) == 0) {
        DisableCrystalLabel(Trim(content.substr(1)), config);
      }
      continue;
    }

    if (topSection == "Guser" && guserSection == "exogam2" && indent == 4) {
      if (key.rfind("clover", 0) == 0 && key.find("_Distance") == std::string::npos) {
        const std::string suffix = key.substr(6);
        if (!suffix.empty() && std::all_of(suffix.begin(), suffix.end(), ::isdigit)) {
          const int clover = std::stoi(suffix);
          if (clover >= 0 && clover < kClovers) {
            config.activeClovers[clover] = ParseBool(rawValue);
            config.activeCloversConfigured = true;
          }
        }
      } else if (key == "nfs_gammaFlashOffset") {
        config.gammaFlashOffsetNs = std::stod(rawValue);
      } else if (key == "nfs_neutron") {
        config.neutronNfs = ParseBool(rawValue);
      } else if (key == "readcal_file") {
        config.readCalibrationFile = ParseBool(rawValue);
      } else if (key == "use_default_cal") {
        config.useDefaultEnergyCalibration = ParseBool(rawValue);
      }
    }
  }
}

struct LutEntry {
  int clover = -1;
  int crystal = -1;
};

class ExogamLut {
 public:
  void Load(const std::string &path) {
    std::ifstream stream(path);
    if (!stream) throw std::runtime_error("cannot open EXOGAM LUT: " + path);
    std::string line;
    while (std::getline(stream, line)) {
      line = Trim(line);
      if (line.empty() || line.rfind("//", 0) == 0 || line.rfind("#", 0) == 0) continue;
      if (line.rfind("END", 0) == 0) break;
      int clover = -1;
      int crystal = -1;
      int board = -1;
      int halfBoard = -1;
      std::istringstream parser(line);
      if (!(parser >> clover >> crystal >> board >> halfBoard)) continue;
      if (clover < 0 || clover >= kClovers || crystal < 0 || crystal >= kCrystalsPerClover ||
          board < 0 || halfBoard < 0 || halfBoard > 1) {
        throw std::runtime_error("invalid LUT row: " + line);
      }
      entries_[{board, halfBoard}] = {clover, crystal};
      mappedClovers_[clover] = true;
    }
    if (entries_.empty()) throw std::runtime_error("EXOGAM LUT contains no usable rows: " + path);
  }

  LutEntry Find(int board, int halfBoard) const {
    const auto it = entries_.find({board, halfBoard});
    return it == entries_.end() ? LutEntry{} : it->second;
  }

  const std::array<bool, kClovers> &MappedClovers() const { return mappedClovers_; }
  std::size_t Size() const { return entries_.size(); }

 private:
  std::map<std::pair<int, int>, LutEntry> entries_;
  std::array<bool, kClovers> mappedClovers_{};
};

std::vector<Coefficients> ReadCoefficientTriples(const std::string &path) {
  std::ifstream stream(path);
  if (!stream) throw std::runtime_error("cannot open calibration file: " + path);
  std::vector<Coefficients> result;
  std::string line;
  while (std::getline(stream, line)) {
    line = Trim(StripYamlComment(line));
    if (line.empty() || line.rfind("//", 0) == 0) continue;
    Coefficients value;
    std::istringstream parser(line);
    if (parser >> value.offset >> value.gain >> value.gain2) result.push_back(value);
  }
  return result;
}

struct Calibration {
  std::array<Coefficients, kCrystals> energy;
  std::array<Coefficients, kCrystals> legacyTime;
  std::array<Coefficients, kCrystals> nfsTime;
  std::array<bool, kCrystals> nfsTimeValid{};
};

Calibration LoadCalibration(const Options &options, const RuntimeConfig &config) {
  Calibration calibration;
  for (auto &value : calibration.energy) value = {0.0, 1.0, 0.0};
  for (auto &value : calibration.legacyTime) value = {0.0, 1.0, 0.0};
  for (auto &value : calibration.nfsTime) value = {0.0, 1.0, 0.0};

  if (config.useDefaultEnergyCalibration) {
    for (auto &value : calibration.energy) value = {0.0, 0.18, 0.0};
  }
  if (config.readCalibrationFile) {
    const auto values = ReadCoefficientTriples(options.energyCalibrationPath);
    if (values.size() < kCrystals) {
      throw std::runtime_error("energy calibration needs at least 64 coefficient rows: " +
                               options.energyCalibrationPath);
    }
    for (int id = 0; id < kCrystals; ++id) calibration.energy[id] = values[id];
    if (values.size() >= 2 * kCrystals) {
      for (int id = 0; id < kCrystals; ++id) calibration.legacyTime[id] = values[kCrystals + id];
    }
  }

  if (config.crystalTimeCorrection) {
    const std::string path = options.timeCalibrationPathOverride.value_or(config.timeCorrectionPath);
    if (path.empty()) throw std::runtime_error("crystal time correction is enabled but no --time-cal/correction_path is set");
    const auto values = ReadCoefficientTriples(path);
    if (values.size() < 2 * kCrystals) {
      throw std::runtime_error("NFS time correction requires 128 coefficient rows: " + path);
    }
    for (int id = 0; id < kCrystals; ++id) {
      calibration.nfsTime[id] = values[kCrystals + id];
      const auto &value = calibration.nfsTime[id];
      calibration.nfsTimeValid[id] = (value.offset != 0.0 || value.gain != 0.0);
    }
  }
  return calibration;
}

double CalibrateWithDither(int raw, const Coefficients &coefficients, TRandom3 &random,
                           bool dither) {
  const double x = static_cast<double>(raw) + (dither ? random.Uniform(1.0) - 0.5 : 0.0);
  return x * x * coefficients.gain2 + x * coefficients.gain + coefficients.offset;
}

struct Counters {
  Long64_t treeRows = 0;
  Long64_t topEventsSeen = 0;
  Long64_t topEventsProcessed = 0;
  Long64_t exo2Frames = 0;
  Long64_t unknownLutFrames = 0;
  Long64_t inactiveCloverFrames = 0;
  Long64_t lowCoreFrames = 0;
  Long64_t acceptedFrames = 0;
  Long64_t positiveEnergyFrames = 0;
  Long64_t validTimePositiveEnergyFrames = 0;
  Long64_t enabledValidTimePositiveEnergyFrames = 0;
  Long64_t bgoPositiveEnergyFrames = 0;
  Long64_t csiPositiveEnergyFrames = 0;
  Long64_t timestampPhaseFrames = 0;
  Long64_t globalTimestampPairs = 0;
  Long64_t perCrystalTimestampPairs = 0;
  Long64_t uniqueCrystalEventFires = 0;
  Long64_t crystalCrossTalkOrderedPairs = 0;
  Long64_t cloverAddbackFires = 0;
  Long64_t vetoCloverAddbackFires = 0;
  Long64_t gammaGammaOrderedPairs = 0;
  Long64_t nonMonotonicTopRows = 0;
};

class Histograms {
 public:
  explicit Histograms(const RuntimeConfig &config) {
    TH1::AddDirectory(false);
    allTimeVsEnergy = Make<TH2F>("nfs_all_crystal_time_vs_energy",
        "NFS all crystal fires;Time (ns);Energy (keV)", 1600, 0, 1600, 4000, 0, 20000);
    allTime = Make<TH1F>("nfs_all_crystal_time", "NFS all crystal Time;Time (ns);Counts",
                         1600, 0, 1600);
    allTimestampDiff = Make<TH1F>("nfs_all_crystal_ts_diff",
        "NFS all crystal timestamp difference;#DeltaTS in read order (ns);Crystal-frame pairs",
        22000, -10000, 100000);
    allTsMinusTdc = Make<TH1F>("nfs_all_crystal_ts_phase_minus_tdc",
        "NFS all crystal TS phase minus reversed TDC;TS phase - reversed DeltaT (ns);Crystal frames",
        1600, -800, 800);
    crystalCrossTalk = Make<TH2F>("nfs_crystal_cross_talk",
        "NFS crystal cross talk;Crystal (clover-crystal);Crystal (clover-crystal)",
        64, -0.5, 63.5, 64, -0.5, 63.5);
    crystalBgoEfficiency = Make<TProfile>("nfs_crystal_bgo_efficiency",
        "NFS crystal BGO efficiency;Crystal (clover-crystal);BGO fire efficiency",
        64, -0.5, 63.5, 0.0, 1.0);
    crystalCsiEfficiency = Make<TProfile>("nfs_crystal_csi_efficiency",
        "NFS crystal CSI efficiency;Crystal (clover-crystal);CSI fire efficiency",
        64, -0.5, 63.5, 0.0, 1.0);
    gammaGammaNoCut = Make<TH2F>("nfs_all_gamma_gamma_matrix_no_cut",
        "NFS addback gamma-gamma BGO CSI veto no cut;Addback gamma energy (keV);Addback gamma energy (keV)",
        4096, 0, 4096, 4096, 0, 4096);

    for (int id = 0; id < kCrystals; ++id) {
      const std::string label = std::to_string(id / 4) + "-" + std::to_string(id % 4);
      crystalCrossTalk->GetXaxis()->SetBinLabel(id + 1, label.c_str());
      crystalCrossTalk->GetYaxis()->SetBinLabel(id + 1, label.c_str());
      crystalBgoEfficiency->GetXaxis()->SetBinLabel(id + 1, label.c_str());
      crystalCsiEfficiency->GetXaxis()->SetBinLabel(id + 1, label.c_str());
    }

    for (int clover = 0; clover < kClovers; ++clover) {
      if (!config.activeClovers[clover]) continue;
      for (int crystal = 0; crystal < kCrystalsPerClover; ++crystal) {
        const int id = clover * 4 + crystal;
        const std::string prefix = "nfs_clover" + std::to_string(clover) + "_crystal" +
                                   std::to_string(crystal);
        const std::string title = "Clover" + std::to_string(clover) + " Crystal" +
                                  std::to_string(crystal);
        crystalTime[id] = Make<TH1F>((prefix + "_time").c_str(),
            (title + " Time;Time (ns);Counts").c_str(), 1600, 0, 1600);
        crystalTimestampDiff[id] = Make<TH1F>((prefix + "_ts_diff").c_str(),
            (title + " timestamp difference;#DeltaTS for same crystal (ns);Crystal-frame pairs").c_str(),
            22000, -10000, 100000);
        crystalTsMinusTdc[id] = Make<TH1F>((prefix + "_ts_phase_minus_tdc").c_str(),
            (title + " TS phase minus reversed TDC;TS phase - reversed DeltaT (ns);Crystal frames").c_str(),
            1600, -800, 800);
        crystalEnergy[id] = Make<TH1F>((prefix + "_energy").c_str(),
            (title + " Gamma Energy;Energy (keV);Counts").c_str(), 4000, 0, 20000);
        crystalBgoEnergy[id] = Make<TH1F>((prefix + "_bgo_energy").c_str(),
            (title + " BGO Energy;BGO energy (channel);Counts").c_str(), 4000, 0, 16000);
        crystalCsiEnergy[id] = Make<TH1F>((prefix + "_csi_energy").c_str(),
            (title + " CSI Energy;CSI energy (channel);Counts").c_str(), 4000, 0, 16000);
        crystalEnergyBgoFire[id] = Make<TH1F>((prefix + "_gamma_bgo_fire").c_str(),
            (title + " Gamma with BGO fire;Gamma energy (keV);Counts").c_str(), 4000, 0, 20000);
        crystalEnergyBgoVeto[id] = Make<TH1F>((prefix + "_gamma_bgo_veto").c_str(),
            (title + " Gamma with BGO veto;Gamma energy (keV);Counts").c_str(), 4000, 0, 20000);
        crystalEnergyCsiFire[id] = Make<TH1F>((prefix + "_gamma_csi_fire").c_str(),
            (title + " Gamma with CSI fire;Gamma energy (keV);Counts").c_str(), 4000, 0, 20000);
        crystalEnergyCsiVeto[id] = Make<TH1F>((prefix + "_gamma_csi_veto").c_str(),
            (title + " Gamma with CSI veto;Gamma energy (keV);Counts").c_str(), 4000, 0, 20000);
      }

      const std::string prefix = "nfs_clover" + std::to_string(clover);
      const std::string title = "Clover" + std::to_string(clover);
      cloverAddback[clover] = Make<TH1F>((prefix + "_addback_gamma").c_str(),
          (title + " Addback Gamma;Energy (keV);Counts").c_str(), 4096, 0, 4096);
      cloverAddbackVeto[clover] = Make<TH1F>((prefix + "_addback_gamma_bgo_csi_veto").c_str(),
          (title + " Addback Gamma BGO CSI veto;Energy (keV);Counts").c_str(),
          4096, 0, 4096);
    }
  }

  void FillTimestamp(ULong64_t timestamp, int rawDeltaT, int id, Counters &counters) {
    if (timestamp == 0) return;
    ++counters.timestampPhaseFrames;
    if (previousTimestamp_ > 0) {
      const Long64_t deltaTicks = static_cast<Long64_t>(timestamp) -
                                  static_cast<Long64_t>(previousTimestamp_);
      allTimestampDiff->Fill(static_cast<double>(deltaTicks) * kTimestampTickNs);
      ++counters.globalTimestampPairs;
    }
    previousTimestamp_ = timestamp;

    if (previousTimestampPerCrystal_[id] > 0) {
      const Long64_t deltaTicks = static_cast<Long64_t>(timestamp) -
          static_cast<Long64_t>(previousTimestampPerCrystal_[id]);
      crystalTimestampDiff[id]->Fill(static_cast<double>(deltaTicks) * kTimestampTickNs);
      ++counters.perCrystalTimestampPairs;
    }
    previousTimestampPerCrystal_[id] = timestamp;

    const double reversedDeltaTNs = (kTdcChannels - static_cast<double>(rawDeltaT)) *
                                    kDefaultTdcGainNs;
    const long double timestampNs = static_cast<long double>(timestamp) * kTimestampTickNs;
    double timestampPhaseNs = static_cast<double>(std::fmod(timestampNs,
        static_cast<long double>(kTdcPeriodNs)));
    if (timestampPhaseNs < 0) timestampPhaseNs += kTdcPeriodNs;
    double difference = timestampPhaseNs - reversedDeltaTNs;
    while (difference > 0.5 * kTdcPeriodNs) difference -= kTdcPeriodNs;
    while (difference < -0.5 * kTdcPeriodNs) difference += kTdcPeriodNs;
    allTsMinusTdc->Fill(difference);
    crystalTsMinusTdc[id]->Fill(difference);
  }

  void FillCrystal(int id, double timeNs, double energy, double bgo, double csi,
                   bool fillGlobal, Counters &counters) {
    if (energy <= 0) return;
    crystalEnergy[id]->Fill(energy);
    ++counters.positiveEnergyFrames;
    if (bgo > 0) {
      crystalBgoEnergy[id]->Fill(bgo);
      crystalEnergyBgoFire[id]->Fill(energy);
      ++counters.bgoPositiveEnergyFrames;
    } else {
      crystalEnergyBgoVeto[id]->Fill(energy);
    }
    if (csi > 0) {
      crystalCsiEnergy[id]->Fill(csi);
      crystalEnergyCsiFire[id]->Fill(energy);
      ++counters.csiPositiveEnergyFrames;
    } else {
      crystalEnergyCsiVeto[id]->Fill(energy);
    }
    if (timeNs <= 0) return;
    crystalTime[id]->Fill(timeNs);
    ++counters.validTimePositiveEnergyFrames;
    if (fillGlobal) {
      allTimeVsEnergy->Fill(timeNs, energy);
      allTime->Fill(timeNs);
      ++counters.enabledValidTimePositiveEnergyFrames;
    }
  }

  void FillEvent(const std::array<bool, kCrystals> &crystalFired,
                 const std::array<bool, kCrystals> &bgoFired,
                 const std::array<bool, kCrystals> &csiFired,
                 const std::array<bool, kClovers> &cloverFired,
                 const std::array<double, kClovers> &cloverEnergy,
                 const std::array<double, kClovers> &cloverBgo,
                 const std::array<double, kClovers> &cloverCsi,
                 const RuntimeConfig &config, Counters &counters) {
    std::vector<int> firedIds;
    for (int id = 0; id < kCrystals; ++id) {
      if (!config.enabledCrystals[id] || !crystalFired[id]) continue;
      firedIds.push_back(id);
      crystalBgoEfficiency->Fill(id, bgoFired[id] ? 1.0 : 0.0);
      crystalCsiEfficiency->Fill(id, csiFired[id] ? 1.0 : 0.0);
      ++counters.uniqueCrystalEventFires;
    }
    for (std::size_t i = 0; i < firedIds.size(); ++i) {
      for (std::size_t j = i + 1; j < firedIds.size(); ++j) {
        crystalCrossTalk->Fill(firedIds[i], firedIds[j]);
        crystalCrossTalk->Fill(firedIds[j], firedIds[i]);
        counters.crystalCrossTalkOrderedPairs += 2;
      }
    }

    std::vector<int> vetoClovers;
    for (int clover = 0; clover < kClovers; ++clover) {
      if (!config.activeClovers[clover] || !cloverFired[clover] || cloverEnergy[clover] <= 0) continue;
      cloverAddback[clover]->Fill(cloverEnergy[clover]);
      ++counters.cloverAddbackFires;
      if (cloverBgo[clover] <= 0 && cloverCsi[clover] <= 0) {
        cloverAddbackVeto[clover]->Fill(cloverEnergy[clover]);
        vetoClovers.push_back(clover);
        ++counters.vetoCloverAddbackFires;
      }
    }
    for (int first : vetoClovers) {
      for (int second : vetoClovers) {
        if (first == second) continue;
        gammaGammaNoCut->Fill(cloverEnergy[first], cloverEnergy[second]);
        ++counters.gammaGammaOrderedPairs;
      }
    }
  }

  bool Validate(const Counters &counters) const {
    bool ok = true;
    auto check = [&](const std::string &name, double observed, Long64_t expected) {
      const Long64_t rounded = static_cast<Long64_t>(std::llround(observed));
      const bool pass = rounded == expected;
      std::cout << "  " << std::left << std::setw(42) << name << " observed=" << std::setw(12)
                << rounded << " expected=" << std::setw(12) << expected
                << (pass ? " OK" : " FAIL") << '\n';
      ok = ok && pass;
    };

    double crystalEnergyEntries = 0;
    double crystalTimeEntries = 0;
    double bgoEnergyEntries = 0;
    double csiEnergyEntries = 0;
    double bgoFireEntries = 0;
    double bgoVetoEntries = 0;
    double csiFireEntries = 0;
    double csiVetoEntries = 0;
    double perCrystalTimestampEntries = 0;
    for (int id = 0; id < kCrystals; ++id) {
      if (!crystalEnergy[id]) continue;
      crystalEnergyEntries += crystalEnergy[id]->GetEntries();
      crystalTimeEntries += crystalTime[id]->GetEntries();
      bgoEnergyEntries += crystalBgoEnergy[id]->GetEntries();
      csiEnergyEntries += crystalCsiEnergy[id]->GetEntries();
      bgoFireEntries += crystalEnergyBgoFire[id]->GetEntries();
      bgoVetoEntries += crystalEnergyBgoVeto[id]->GetEntries();
      csiFireEntries += crystalEnergyCsiFire[id]->GetEntries();
      csiVetoEntries += crystalEnergyCsiVeto[id]->GetEntries();
      perCrystalTimestampEntries += crystalTimestampDiff[id]->GetEntries();
    }
    double addbackEntries = 0;
    double addbackVetoEntries = 0;
    for (int clover = 0; clover < kClovers; ++clover) {
      if (!cloverAddback[clover]) continue;
      addbackEntries += cloverAddback[clover]->GetEntries();
      addbackVetoEntries += cloverAddbackVeto[clover]->GetEntries();
    }

    std::cout << "Internal histogram validation / 内部直方图自检:\n";
    check("sum crystal energy entries", crystalEnergyEntries, counters.positiveEnergyFrames);
    check("sum crystal time entries", crystalTimeEntries, counters.validTimePositiveEnergyFrames);
    check("global Time entries", allTime->GetEntries(), counters.enabledValidTimePositiveEnergyFrames);
    check("global Time-vs-energy entries", allTimeVsEnergy->GetEntries(),
          counters.enabledValidTimePositiveEnergyFrames);
    check("BGO energy/fire entries", bgoEnergyEntries, counters.bgoPositiveEnergyFrames);
    check("BGO fire + veto partition", bgoFireEntries + bgoVetoEntries,
          counters.positiveEnergyFrames);
    check("CSI energy/fire entries", csiEnergyEntries, counters.csiPositiveEnergyFrames);
    check("CSI fire + veto partition", csiFireEntries + csiVetoEntries,
          counters.positiveEnergyFrames);
    check("global timestamp-diff entries", allTimestampDiff->GetEntries(),
          counters.globalTimestampPairs);
    check("per-crystal timestamp-diff entries", perCrystalTimestampEntries,
          counters.perCrystalTimestampPairs);
    check("TS phase-minus-TDC entries", allTsMinusTdc->GetEntries(), counters.timestampPhaseFrames);
    check("BGO efficiency profile entries", crystalBgoEfficiency->GetEntries(),
          counters.uniqueCrystalEventFires);
    check("CSI efficiency profile entries", crystalCsiEfficiency->GetEntries(),
          counters.uniqueCrystalEventFires);
    check("crystal cross-talk entries", crystalCrossTalk->GetEntries(),
          counters.crystalCrossTalkOrderedPairs);
    check("clover addback entries", addbackEntries, counters.cloverAddbackFires);
    check("veto clover addback entries", addbackVetoEntries, counters.vetoCloverAddbackFires);
    check("no-cut gamma-gamma entries", gammaGammaNoCut->GetEntries(),
          counters.gammaGammaOrderedPairs);
    return ok;
  }

  void Write(TFile &output) const {
    output.cd();
    for (const auto &object : objects_) object->Write();
  }

  TH2F *allTimeVsEnergy = nullptr;
  TH1F *allTime = nullptr;
  TH1F *allTimestampDiff = nullptr;
  TH1F *allTsMinusTdc = nullptr;
  TH2F *crystalCrossTalk = nullptr;
  TProfile *crystalBgoEfficiency = nullptr;
  TProfile *crystalCsiEfficiency = nullptr;
  TH2F *gammaGammaNoCut = nullptr;
  std::array<TH1F *, kCrystals> crystalTime{};
  std::array<TH1F *, kCrystals> crystalTimestampDiff{};
  std::array<TH1F *, kCrystals> crystalTsMinusTdc{};
  std::array<TH1F *, kCrystals> crystalEnergy{};
  std::array<TH1F *, kCrystals> crystalBgoEnergy{};
  std::array<TH1F *, kCrystals> crystalCsiEnergy{};
  std::array<TH1F *, kCrystals> crystalEnergyBgoFire{};
  std::array<TH1F *, kCrystals> crystalEnergyBgoVeto{};
  std::array<TH1F *, kCrystals> crystalEnergyCsiFire{};
  std::array<TH1F *, kCrystals> crystalEnergyCsiVeto{};
  std::array<TH1F *, kClovers> cloverAddback{};
  std::array<TH1F *, kClovers> cloverAddbackVeto{};

 private:
  template <typename T, typename... Args>
  T *Make(Args &&...args) {
    auto object = std::make_unique<T>(std::forward<Args>(args)...);
    T *pointer = object.get();
    objects_.push_back(std::move(object));
    return pointer;
  }

  std::vector<std::unique_ptr<TObject>> objects_;
  ULong64_t previousTimestamp_ = 0;
  std::array<ULong64_t, kCrystals> previousTimestampPerCrystal_{};
};

struct EventAccumulator {
  std::array<bool, kCrystals> crystalFired{};
  std::array<bool, kCrystals> bgoFired{};
  std::array<bool, kCrystals> csiFired{};
  std::array<bool, kClovers> cloverFired{};
  std::array<double, kClovers> cloverEnergy{};
  std::array<double, kClovers> cloverBgo{};
  std::array<double, kClovers> cloverCsi{};

  void Clear() {
    crystalFired.fill(false);
    bgoFired.fill(false);
    csiFired.fill(false);
    cloverFired.fill(false);
    cloverEnergy.fill(0.0);
    cloverBgo.fill(0.0);
    cloverCsi.fill(0.0);
  }
};

struct RawBranches {
  Long64_t topEventIndex = -1;
  Bool_t isExo2 = false;
  ULong64_t timestamp = 0;
  Int_t board = -1;
  Int_t halfBoard = -1;
  Int_t status3 = 0;
  Int_t rawDeltaT = -1;
  Int_t inner6m = -1;
  Int_t bgo = -1;
  Int_t csi = -1;

  void Attach(TTree &tree) {
    const std::array<const char *, 10> required = {
        "top_event_index", "is_exo2", "timestamp", "exo_board_id",
        "exo_lut_halfboard", "exo_status3", "exo_delta_t", "exo_inner6m",
        "exo_bgo", "exo_csi"};
    for (const char *name : required) {
      if (!tree.GetBranch(name)) throw std::runtime_error(std::string("missing branch: ") + name);
    }

    tree.SetBranchStatus("*", 0);
    for (const char *name : required) tree.SetBranchStatus(name, 1);
    tree.SetBranchAddress("top_event_index", &topEventIndex);
    tree.SetBranchAddress("is_exo2", &isExo2);
    tree.SetBranchAddress("timestamp", &timestamp);
    tree.SetBranchAddress("exo_board_id", &board);
    tree.SetBranchAddress("exo_lut_halfboard", &halfBoard);
    tree.SetBranchAddress("exo_status3", &status3);
    tree.SetBranchAddress("exo_delta_t", &rawDeltaT);
    tree.SetBranchAddress("exo_inner6m", &inner6m);
    tree.SetBranchAddress("exo_bgo", &bgo);
    tree.SetBranchAddress("exo_csi", &csi);

    tree.SetCacheSize(128LL * 1024LL * 1024LL);
    for (const char *name : required) tree.AddBranchToCache(name, true);
    tree.StopCacheLearningPhase();
  }

};

void FillSummaryTree(TFile &output, Counters &counters) {
  output.cd();
  TTree summary("nfs_histo_processing_summary", "Standalone NFS histogram processing counters");
#define BRANCH_COUNTER(name) summary.Branch(#name, &counters.name)
  BRANCH_COUNTER(treeRows);
  BRANCH_COUNTER(topEventsSeen);
  BRANCH_COUNTER(topEventsProcessed);
  BRANCH_COUNTER(exo2Frames);
  BRANCH_COUNTER(unknownLutFrames);
  BRANCH_COUNTER(inactiveCloverFrames);
  BRANCH_COUNTER(lowCoreFrames);
  BRANCH_COUNTER(acceptedFrames);
  BRANCH_COUNTER(positiveEnergyFrames);
  BRANCH_COUNTER(validTimePositiveEnergyFrames);
  BRANCH_COUNTER(enabledValidTimePositiveEnergyFrames);
  BRANCH_COUNTER(bgoPositiveEnergyFrames);
  BRANCH_COUNTER(csiPositiveEnergyFrames);
  BRANCH_COUNTER(timestampPhaseFrames);
  BRANCH_COUNTER(globalTimestampPairs);
  BRANCH_COUNTER(perCrystalTimestampPairs);
  BRANCH_COUNTER(uniqueCrystalEventFires);
  BRANCH_COUNTER(crystalCrossTalkOrderedPairs);
  BRANCH_COUNTER(cloverAddbackFires);
  BRANCH_COUNTER(vetoCloverAddbackFires);
  BRANCH_COUNTER(gammaGammaOrderedPairs);
  BRANCH_COUNTER(nonMonotonicTopRows);
#undef BRANCH_COUNTER
  summary.Fill();
  summary.Write();
}

void FillUnknownLutTree(TFile &output,
                        const std::map<std::pair<int, int>, Long64_t> &unknownCounts) {
  output.cd();
  Int_t board = -1;
  Int_t halfBoard = -1;
  Long64_t frames = 0;
  TTree tree("nfs_histo_unknown_lut",
             "EXO2 board/half-board combinations skipped because they are absent from the LUT");
  tree.Branch("board", &board);
  tree.Branch("half_board", &halfBoard);
  tree.Branch("frames", &frames);
  for (const auto &[key, count] : unknownCounts) {
    board = key.first;
    halfBoard = key.second;
    frames = count;
    tree.Fill();
  }
  tree.Write();
}

std::string BuildMetadata(const Options &options, const RuntimeConfig &config,
                          const ExogamLut &lut, const Counters &counters,
                          bool validationOk) {
  std::ostringstream text;
  text << "producer=standalone-nfs-histo;version=1;tree=MfmFrameTree;"
       << "event_key=top_event_index;lut=" << options.lutPath
       << ";lut_rows=" << lut.Size()
       << ";energy_cal=" << options.energyCalibrationPath
       << ";time_correction=" << (config.crystalTimeCorrection ? 1 : 0)
       << ";time_cal=" << options.timeCalibrationPathOverride.value_or(config.timeCorrectionPath)
       << ";gamma_flash_offset_ns=" << config.gammaFlashOffsetNs
       << ";dither=" << (options.dither ? 1 : 0)
       << ";random_seed=" << options.randomSeed
       << ";validation=" << (validationOk ? "PASS" : "FAIL")
       << ";processed_top_events=" << counters.topEventsProcessed
       << ";inputs=";
  for (std::size_t i = 0; i < options.inputs.size(); ++i) {
    if (i) text << ',';
    text << options.inputs[i];
  }
  return text.str();
}

int Run(const Options &options) {
  RuntimeConfig config;
  LoadRelevantYaml(options.yamlConfig, config);
  if (options.activeCloversOverride) ParseActiveClovers(*options.activeCloversOverride, config);
  if (options.disabledCrystalsOverride) {
    config.enabledCrystals.fill(true);
    ParseDisabledCrystalExpression(*options.disabledCrystalsOverride, config);
  }
  if (options.gammaFlashOffsetOverride) config.gammaFlashOffsetNs = *options.gammaFlashOffsetOverride;
  if (options.crystalTimeCorrectionOverride) {
    config.crystalTimeCorrection = *options.crystalTimeCorrectionOverride;
  }

  ExogamLut lut;
  lut.Load(options.lutPath);
  if (!config.activeCloversConfigured) config.activeClovers = lut.MappedClovers();
  if (options.timeCalibrationPathOverride) config.timeCorrectionPath = *options.timeCalibrationPathOverride;
  const Calibration calibration = LoadCalibration(options, config);

  std::cout << "Standalone NFS histogram builder / 独立 NFS 直方图处理\n"
            << "  input files: " << options.inputs.size() << '\n'
            << "  output: " << options.output << '\n'
            << "  LUT: " << options.lutPath << " (" << lut.Size() << " rows)\n"
            << "  energy calibration: " << options.energyCalibrationPath << '\n'
            << "  gamma-flash offset: " << config.gammaFlashOffsetNs << " ns\n"
            << "  crystal time correction: " << (config.crystalTimeCorrection ? "true" : "false") << '\n'
            << "  ADC dither: " << (options.dither ? "true" : "false")
            << " seed=" << options.randomSeed << '\n'
            << "  active clovers:";
  for (int clover = 0; clover < kClovers; ++clover) if (config.activeClovers[clover]) std::cout << ' ' << clover;
  std::cout << "\n  disabled crystals:";
  bool anyDisabled = false;
  for (int id = 0; id < kCrystals; ++id) {
    if (!config.enabledCrystals[id]) {
      std::cout << ' ' << id / 4 << '-' << id % 4;
      anyDisabled = true;
    }
  }
  if (!anyDisabled) std::cout << " none";
  std::cout << '\n';

  Histograms histograms(config);
  Counters counters;
  std::map<std::pair<int, int>, Long64_t> unknownLutCounts;
  EventAccumulator event;
  event.Clear();
  TRandom3 random(options.randomSeed);
  Long64_t selectedEventsTotal = 0;
  Long64_t seenEventsTotal = 0;
  bool stop = false;

  for (const std::string &inputPath : options.inputs) {
    if (stop) break;
    std::unique_ptr<TFile> input(TFile::Open(inputPath.c_str(), "READ"));
    if (!input || input->IsZombie()) throw std::runtime_error("cannot open input ROOT: " + inputPath);
    auto *tree = dynamic_cast<TTree *>(input->Get("MfmFrameTree"));
    if (!tree) throw std::runtime_error("MfmFrameTree not found in: " + inputPath);
    RawBranches branches;
    branches.Attach(*tree);
    const Long64_t entries = tree->GetEntries();
    std::cout << "Process / 处理: " << inputPath << " rows=" << entries << '\n';

    Long64_t currentTop = std::numeric_limits<Long64_t>::min();
    bool currentSelected = false;
    bool haveCurrent = false;

    auto finalizeCurrent = [&]() {
      if (!haveCurrent || !currentSelected) return;
      histograms.FillEvent(event.crystalFired, event.bgoFired, event.csiFired,
                           event.cloverFired, event.cloverEnergy, event.cloverBgo,
                           event.cloverCsi, config, counters);
      ++counters.topEventsProcessed;
      ++selectedEventsTotal;
    };

    for (Long64_t entry = 0; entry < entries; ++entry) {
      tree->GetEntry(entry);
      ++counters.treeRows;
      if (!haveCurrent || branches.topEventIndex != currentTop) {
        if (haveCurrent && branches.topEventIndex < currentTop) ++counters.nonMonotonicTopRows;
        finalizeCurrent();
        if (options.maxEvents > 0 && selectedEventsTotal >= options.maxEvents) {
          stop = true;
          break;
        }
        event.Clear();
        currentTop = branches.topEventIndex;
        haveCurrent = true;
        currentSelected = seenEventsTotal >= options.startEvents;
        ++seenEventsTotal;
        ++counters.topEventsSeen;
      }
      if (options.progressEntries > 0 && counters.treeRows % options.progressEntries == 0) {
        std::cout << "  rows=" << counters.treeRows
                  << " events=" << counters.topEventsProcessed
                  << " accepted EXO2=" << counters.acceptedFrames << '\n';
      }
      if (!currentSelected || !branches.isExo2) continue;

      ++counters.exo2Frames;
      const LutEntry mapped = lut.Find(branches.board, branches.halfBoard);
      if (mapped.clover < 0) {
        ++counters.unknownLutFrames;
        ++unknownLutCounts[{branches.board, branches.halfBoard}];
        continue;
      }
      if (!config.activeClovers[mapped.clover]) {
        ++counters.inactiveCloverFrames;
        continue;
      }
      if (branches.inner6m <= 10) {
        ++counters.lowCoreFrames;
        continue;
      }
      ++counters.acceptedFrames;

      const int id = mapped.clover * kCrystalsPerClover + mapped.crystal;
      const bool enabled = config.enabledCrystals[id];
      if (enabled) histograms.FillTimestamp(branches.timestamp, branches.rawDeltaT, id, counters);

      // EN: Match TExogam2::Cal: one random sub-channel dither is applied before
      //     the quadratic energy calibration. The following unused random calls
      //     reproduce ADNE's legacy TDC and four GOCCE segment calibrations, keeping
      //     the random sequence aligned from frame to frame when PSA is disabled.
      // CN: 对齐 TExogam2::Cal：能量二次刻度前先加一个随机的半道展宽。随后额外
      //     消耗传统 TDC 与四个 GOCCE segment 刻度所用的随机数，使关闭 PSA 时相邻
      //     frame 的随机序列尽可能与 ADNE 一致。
      const double energy = CalibrateWithDither(branches.inner6m, calibration.energy[id],
                                                 random, options.dither);
      (void)CalibrateWithDither(branches.rawDeltaT, calibration.legacyTime[id],
                                random, options.dither);
      for (int segment = 0; segment < 4; ++segment) {
        const int mirror = ((branches.status3 >> (8 * branches.halfBoard)) >> (segment + 2)) & 1;
        if (mirror == 0 && options.dither) (void)random.Uniform(1.0);
      }

      double neutronTimeNs = 0.0;
      if (config.neutronNfs) {
        const double reversed = kTdcChannels - static_cast<double>(branches.rawDeltaT);
        const bool useCorrection = config.crystalTimeCorrection && calibration.nfsTimeValid[id];
        if (useCorrection) {
          neutronTimeNs = reversed * kDefaultTdcGainNs * calibration.nfsTime[id].gain +
                          config.gammaFlashOffsetNs + calibration.nfsTime[id].offset;
        } else {
          neutronTimeNs = reversed * kDefaultTdcGainNs + config.gammaFlashOffsetNs;
        }
        if (branches.rawDeltaT > 60000 || neutronTimeNs <= 0) neutronTimeNs = 0.0;
      }

      histograms.FillCrystal(id, neutronTimeNs, energy, branches.bgo, branches.csi,
                             enabled, counters);
      if (enabled && energy > 0) {
        event.crystalFired[id] = true;
        if (branches.bgo > 0) event.bgoFired[id] = true;
        if (branches.csi > 0) event.csiFired[id] = true;
        event.cloverFired[mapped.clover] = true;
        event.cloverEnergy[mapped.clover] += energy;
        event.cloverBgo[mapped.clover] += branches.bgo;
        event.cloverCsi[mapped.clover] += branches.csi;
      }

    }
    if (!stop) finalizeCurrent();
    input->Close();
  }

  const bool validationOk = histograms.Validate(counters);
  if (counters.nonMonotonicTopRows > 0) {
    std::cerr << "WARNING: non-contiguous/decreasing top_event_index rows: "
              << counters.nonMonotonicTopRows << '\n';
  }
  if (!unknownLutCounts.empty()) {
    std::cerr << "WARNING: EXO2 board/half-board pairs absent from LUT / LUT 中缺失的板卡组合:\n";
    for (const auto &[key, count] : unknownLutCounts) {
      std::cerr << "  board=" << key.first << " half=" << key.second
                << " frames=" << count << '\n';
    }
  }

  const std::filesystem::path outputPath = options.output;
  if (outputPath.has_parent_path()) std::filesystem::create_directories(outputPath.parent_path());
  TFile output(options.output.c_str(), "RECREATE", "Standalone ADNE-equivalent NFS spectra", 4);
  if (output.IsZombie()) throw std::runtime_error("cannot create output ROOT: " + options.output);
  histograms.Write(output);
  FillSummaryTree(output, counters);
  FillUnknownLutTree(output, unknownLutCounts);
  const std::string metadata = BuildMetadata(options, config, lut, counters, validationOk);
  TNamed metadataObject("nfs_histo_standalone_config", metadata.c_str());
  metadataObject.Write();
  output.Write();
  output.Close();

  std::cout << "Summary / 汇总:\n"
            << "  top events processed: " << counters.topEventsProcessed << '\n'
            << "  EXO2 frames read: " << counters.exo2Frames << '\n'
            << "  accepted EXO2 frames: " << counters.acceptedFrames << '\n'
            << "  unknown LUT frames: " << counters.unknownLutFrames << '\n'
            << "  positive gamma frames: " << counters.positiveEnergyFrames << '\n'
            << "  clover addback fires: " << counters.cloverAddbackFires << '\n'
            << "  no-cut gamma-gamma ordered pairs: " << counters.gammaGammaOrderedPairs << '\n'
            << "  validation: " << (validationOk ? "PASS" : "FAIL") << '\n'
            << "  output: " << options.output << '\n';
  return validationOk ? 0 : 3;
}

}  // namespace

int main(int argc, char **argv) {
  try {
    return Run(ParseArguments(argc, argv));
  } catch (const std::exception &error) {
    std::cerr << "ERROR: " << error.what() << '\n';
    return 1;
  }
}
