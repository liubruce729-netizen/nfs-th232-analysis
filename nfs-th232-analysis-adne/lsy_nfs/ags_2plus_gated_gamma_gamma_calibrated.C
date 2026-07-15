// Calibrated AGS 2+ -> 0+ gated gamma-gamma analysis for NFS fission events.
// NFS 裂变事件的逐 run 刻度、AGS 2+ -> 0+ gate 后 gamma-gamma 分析。
//
// What this macro does / 本宏完成的处理
// -----------------------------------------------------------------------------
// 1. Read one ROOT file, comma-separated ROOT files, or an @file-list.
// 2. Match every input run to the per-crystal energy/time coefficients in the
//    calibration summary produced by calibrate_nfs_crystals.sh.
// 3. Rebuild clover addback after crystal calibration. The time of a clover is
//    taken from its highest-energy crystal, exactly as in the calibrated NFS
//    fission analysis. BGO/CSI veto and the fast-time cut are enabled by default.
// 4. Parse the ASCII GLS/AGS file itself. Gamma records whose initial/final
//    spins are exactly 2+ -> 0+ become gates. Both 2+ -> 0+ and 4+ -> 2+
//    transitions are written to the output metadata tree.
// 5. Test every selected gamma in every event against every 2+ gate. For each
//    matching (gate, gamma-hit) instance, remove that one hit. If at least two
//    gammas remain, fill every residual pair symmetrically.
// 6. Pair each 4+ -> 2+ transition with the 2+ -> 0+ transition sharing the
//    same intermediate 2+ level. Gate both cascade gammas and fill every other
//    gamma into a dedicated one-dimensional spectrum named "E4to2-E2to0".
//
// 1. 输入可以是一个 ROOT、逗号分隔的多个 ROOT，或 @文件列表。
// 2. 按 run 文件名读取 calibrate_nfs_crystals.sh 生成的逐 crystal 能量/时间刻度。
// 3. 刻度后重建 clover addback；clover 时间取最高能量 crystal 的时间。默认执行
//    BGO/CSI veto，并去掉快于 50 MeV 中子允许时间范围的 clover fire。
// 4. 直接解析 ASCII GLS/AGS：初末能级严格为 2+ -> 0+ 的 gamma 作为 gate；
//    2+ -> 0+ 和 4+ -> 2+ 跃迁都会写入输出 metadata tree。
// 5. 每个 event 的每个 gamma 都要经过所有 2+ gate。每个命中的 (gate, gamma)
//    实例单独去掉该 gamma；若剩余 gamma 数量 >= 2，则把剩余 gamma 两两对称填充。
// 6. 将共享同一个中间 2+ level 的 4+ -> 2+ 与 2+ -> 0+ 组成真实级联对；同时
//    gate 住两条级联 gamma，并把其余 gamma 填入名为“E4to2-E2to0”的独立一维谱。
//
// Example / 示例
// -----------------------------------------------------------------------------
// root -l -b -q 'lsy_nfs/ags_2plus_gated_gamma_gamma_calibrated.C(
//   "@mult3_files.txt",
//   "calibration_summary.tsv",
//   "/home/user0/work/IJCLAB/nfs_ana/nfs_ags_no_correction_results/_extracted_ags/th232_18_34to24_34.ags",
//   "ags_2plus_gated.root",
//   "6:8,10:13,17:20",
//   3.0,24.0,20.0,true,-1,4096,0,4096,"offset","fTime")'
//
// neutronWindowsMeV uses low:high pairs separated by commas.
// neutronWindowsMeV 使用逗号分隔的 low:high 区间。
// gateWidthKeV counts consecutive 1-keV gate channels, consistently with
// triple_gamma_gate_fill.C. A width of 3 means center +/-1 keV, inclusive.
// gateWidthKeV 表示连续的 1 keV gate channel 数，与 triple_gamma_gate_fill.C
// 保持一致；宽度 3 表示中心 +/-1 keV，且包含两个端点。

// =============================================================================
// Complete positional interface / 完整位置参数接口
// =============================================================================
// ROOT calls this function with positional arguments:
// ROOT 使用位置参数调用下列函数：
//
// ags_2plus_gated_gamma_gamma_calibrated(
//   [ 1] inputFiles,
//   [ 2] calibrationSummary,
//   [ 3] agsFile = "/home/user0/work/IJCLAB/nfs_ana/nfs_ags_no_correction_results/_extracted_ags/th232_18_34to24_34.ags",
//   [ 4] outputFile = "ags_2plus_gated_gamma_gamma_calibrated.root",
//   [ 5] neutronWindowsMeV = "6:8,10:13,17:20",
//   [ 6] gateWidthKeV = 3.0,
//   [ 7] nfsDistanceMeter = 24.0,
//   [ 8] timeFwhmNs = 20.0,
//   [ 9] useBgoCsiVeto = true,
//   [10] maxEntries = -1,
//   [11] gammaBins = 4096,
//   [12] gammaMinKeV = 0.0,
//   [13] gammaMaxKeV = 4096.0,
//   [14] timeCorrectionMode = "offset",
//   [15] timeBranchLeaf = "fTime")
//
// Only [1] and [2] are required by the C++ signature. The built-in [3] path is
// machine-specific, so explicitly supplying [3] and [4] is strongly recommended.
// C++ 函数签名只强制要求 [1][2]；但 [3] 的内置绝对路径与机器绑定，实际运行时
// 强烈建议总是显式给出 [3] AGS 路径和 [4] 输出路径。
//
// IMPORTANT / 重要：
// ROOT/C++ does not provide named arguments here. To change argument [12], all
// arguments [1] through [11] must also be written. Only trailing arguments may
// be omitted, in which case their defaults above are used.
// 这里不能写“参数名=值”。若要修改第 [12] 项，必须同时写出 [1] 到 [11]；
// 只有末尾连续的参数可以省略，省略后使用上面列出的默认值。
//
// Parameter details / 参数详解
// -----------------------------------------------------------------------------
// [1] inputFiles : const char*
//     Input TreeMaster ROOT files. Three forms are accepted:
//       a) one file:  "/data/mult3_nfs_run_12_r0.root"
//       b) CSV list:  "a.root,b.root,c.root"
//       c) text list: "@mult3_files.txt"
//     An @list contains one ROOT path per line. Empty lines and lines beginning
//     with # or // are ignored. Relative paths are resolved relative to the list
//     file when they are not found from the current directory.
//     输入 TreeMaster ROOT。支持单文件、逗号分隔文件和 @文本列表。@列表每行
//     一个 ROOT 路径；空行、# 和 // 开头行忽略。找不到相对路径时，会相对列表
//     文件所在目录解析。maxEntries 是所有输入文件合计值，不是每个文件各自值。
//
// [2] calibrationSummary : const char*
//     calibration_summary.tsv produced by calibrate_nfs_crystals.sh, or one
//     detailed calibration txt beginning with "# clover ...". Run matching
//     uses the basename and removes the "mult3_" prefix, so
//       mult3_nfs_run_12_r0.root -> nfs_run_12_r0.root.
//     A file with no matching run calibration is skipped; identity calibration
//     is NOT silently substituted. Required values are per-crystal
//     energy_offset, energy_gain, and the requested time correction values.
//     刻度汇总由 calibrate_nfs_crystals.sh 产生。按文件名逐 run 匹配，并自动去掉
//     mult3_ 前缀。没有匹配刻度的输入文件会被跳过，不会偷偷采用单位刻度。
//
// [3] agsFile : const char*
//     ASCII GLS/AGS level-scheme file. Its Level, Band, and Gamma sections are
//     parsed directly. Exact 2+->0+ transitions define single gates. A cascade
//     pair is accepted only when a 4+->2+ final level equals a 2+->0+ initial
//     level and both belong to the same canonical nuclide.
//     ASCII GLS/AGS 能级纲图。代码直接读取 Level/Band/Gamma。所有严格的
//     2+->0+ 作为单 gate；级联还要求 4+->2+ 的终态 level 与 2+->0+ 的初态
//     level 完全相同，并属于同一个核素。建议总是显式填写该路径，不依赖默认绝对路径。
//
// [4] outputFile : const char*
//     Output ROOT path. It is opened with RECREATE, so an existing file with the
//     same name is overwritten. Parent directories must already exist.
//     输出 ROOT 路径，采用 RECREATE；同名文件会被覆盖，父目录必须预先存在。
//
// [5] neutronWindowsMeV : const char*, default "6:8,10:13,17:20"
//     Independent low:high neutron-energy windows in MeV. These are ranges, not
//     a sequence of shared bin edges. The lower edge is included and the upper
//     edge excluded: low <= En < high. Example: "5:8,8:12,17:20".
//     逗号分隔的独立中子能区，单位 MeV，不是连续边界数组。每区间左闭右开。
//
// [6] gateWidthKeV : double, default 3.0
//     Number of consecutive 1-keV gate channels, not a mathematical full width.
//     width=3 means center-1, center, center+1 keV; both endpoints are included.
//     表示连续 1 keV channel 数。3 对应峰位左右各 1 keV，两个端点均包含。
//
// [7] nfsDistanceMeter : double, default 24.0 m
//     Source-to-target neutron flight distance used for the time cut and En
//     conversion. Use the experiment value, for example 23.396 m, when known.
//     中子飞行距离，单位 m，用于快时间 cut 和 TOF->En。应填写实际实验距离。
//
// [8] timeFwhmNs : double, default 20.0 ns
//     Time-resolution FWHM used only by the fast-neutron rejection:
//       sigma_t = timeFwhmNs / 2.355
//       t_min   = TOF(50 MeV, nfsDistanceMeter)
//       t_cut   = t_min - 3*sigma_t
//     A clover with corrected time < t_cut is removed before multiplicity,
//     event-time, gate, and matrix calculations.
//     时间分辨 FWHM，单位 ns。修正时间小于上述 t_cut 的 clover 会在后续所有
//     多重度、事件时间、gate 和矩阵计算前被移除。
//
// [9] useBgoCsiVeto : bool, default true
//     true: remove a clover when its ESS record has BGO>0 or CSI>0.
//     false: ignore BGO and CSI and do not require ESS branches.
//     true 时 BGO>0 或 CSI>0 的 clover 不参与后续处理；false 时完全忽略 veto。
//
// [10] maxEntries : Long64_t, default -1
//     >0 processes at most this many TreeMaster entries across all input files.
//     <=0 processes all entries. Useful values for checks: 1000 or 100000.
//     正数表示所有输入文件合计最多处理多少 event；<=0 表示全部处理。
//
// [11] gammaBins : int, default 4096
// [12] gammaMinKeV : double, default 0 keV
// [13] gammaMaxKeV : double, default 4096 keV
//     X and Y binning of every TH2I gamma-gamma matrix, and X binning of each
//     cascade-gated TH1I. Values outside the range go to ROOT under/overflow.
//     矩阵两轴和级联一维谱的能量分 bin。范围外数据仍进入 ROOT 的 under/overflow。
//     Memory estimate for TH2I matrices:
//       (1 + number_of_neutron_windows) * gammaBins^2 * 4 bytes.
//     Defaults create four 4096x4096 matrices, approximately 256 MiB in RAM,
//     excluding ROOT overhead. Reducing gammaBins to 2048 reduces this by 4x.
//     默认 3 个中子窗加 1 张总图，共约 256 MiB；bin 数减半，矩阵内存约降为 1/4。
//
// [14] timeCorrectionMode : const char*, default "offset"
//     Selects how calibration-summary coefficients are applied:
//       "offset"      : t_corr = t_raw + time_offset
//       "gain"        : t_corr = t_raw * time_gain
//       "offset_gain" : t_corr = (t_raw + time_offset) * time_gain
//       "gain_offset" : t_corr = t_raw * time_gain + time_offset
//     当前逐 run 时间刻度只做平移时使用 "offset"。若汇总文件包含并需要时间 gain，
//     应按照刻度参数的定义选择另外三种顺序之一。
//
// [15] timeBranchLeaf : const char*, default "fTime"
//     Crystal-level raw-to-analysis time branch to calibrate. The resolver also
//     accepts split names ending in this leaf, such as Exogam2.fTime.
//     要读取并刻度的逐 crystal 时间分支叶名；也兼容 Exogam2.fTime 等 split 名称。
//
// Event reconstruction and cuts / 事件重建与选择顺序
// -----------------------------------------------------------------------------
// 1. Reject crystal records with non-positive/non-finite raw energy or time.
// 2. Apply the matched crystal energy and time calibration.
// 3. Sum calibrated crystal energies inside each clover (addback).
// 4. Set clover time to the calibrated time of its highest calibrated-energy
//    crystal.
// 5. Optionally apply BGO/CSI veto, then apply the 50-MeV fast-time cut.
// 6. Require at least three surviving clovers. Event TOF is the minimum surviving
//    clover time; neutron energy is calculated from this event TOF and distance.
// 7. Run every 2+ gate and every physical 4+->2+->0+ cascade pair.
//
// 处理顺序严格为：无效 crystal 清理 -> 逐 crystal 刻度 -> clover addback ->
// 最高能量 crystal 选 clover 时间 -> veto -> 快时间 cut -> 剩余 clover 数>=3 ->
// 最小 clover 时间作为 event TOF -> 单 gate 矩阵与双 gate 级联一维谱。
//
// Output ROOT layout / 输出 ROOT 结构
// -----------------------------------------------------------------------------
// gamma_gamma_matrices/
//   AGS2GateGammaGamma_Total
//   AGS2GateGammaGamma_En<low>_<high>MeV  (one per requested window)
// cascade_gated_spectra/
//   <E4to2>-<E2to0>  (one TH1I per physical cascade pair)
// metadata/
//   AGSTransitions, CascadePairs, GateStatistics, NeutronWindowStatistics,
//   AGS2PlusGateEnergies, AGS2PlusGateMatchedEvents,
//   AGS2GateAnalysisSummary, AGS2GateAnalysisConfig
// diagnostics/
//   AGS2GateAcceptedNeutronEnergy
//
// Practical calls / 常用调用示例
// -----------------------------------------------------------------------------
// A. Explicitly provide all paths, then use trailing defaults /
//    显式给出全部路径，其余末尾参数使用默认值：
// root -l -b -q 'lsy_nfs/ags_2plus_gated_gamma_gamma_calibrated.C(
//   "@mult3_files.txt","calibration_summary.tsv","/abs/path/th232.ags",
//   "ags_gate_result.root")'
//
// B. Actual distance 23.396 m and custom windows / 实际距离和自定义中子窗：
// root -l -b -q 'lsy_nfs/ags_2plus_gated_gamma_gamma_calibrated.C(
//   "@mult3_files.txt","calibration_summary.tsv","/abs/path/th232.ags",
//   "ags_gate_result.root","6:8,10:13,17:20",3,23.396)'
//
// C. Quick 100000-event check with 2048 bins / 快速小样本及低内存矩阵：
// root -l -b -q 'lsy_nfs/ags_2plus_gated_gamma_gamma_calibrated.C(
//   "a.root,b.root","calibration_summary.tsv","/abs/path/th232.ags",
//   "quick.root","6:8,10:13,17:20",3,23.396,20,true,100000,
//   2048,0,4096,"offset","fTime")'
//
// Shell quoting / Shell 引号：outer single quotes protect the complete ROOT
// expression; every C++ string argument inside still uses double quotes.
// 外层单引号保护完整 ROOT 表达式，内部字符串参数仍使用双引号。
// =============================================================================

#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "TBranch.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH1I.h"
#include "TH2I.h"
#include "TNamed.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TString.h"
#include "TSystem.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"

namespace ags_2plus_gate_detail {

constexpr int kNClover = 16;
constexpr int kNCrystalPerClover = 4;
constexpr int kNDetector = kNClover * kNCrystalPerClover;
constexpr double kNeutronMassMeV = 939.5654;
constexpr double kLightSpeedMeterPerNs = 0.299792458;
constexpr double kFastNeutronLimitMeV = 50.0;

std::string Trim(std::string text)
{
  auto notSpace = [](unsigned char c) { return !std::isspace(c); };
  text.erase(text.begin(), std::find_if(text.begin(), text.end(), notSpace));
  text.erase(std::find_if(text.rbegin(), text.rend(), notSpace).base(), text.end());
  return text;
}

std::vector<std::string> SplitWhitespace(const std::string &line)
{
  std::vector<std::string> fields;
  std::stringstream stream(line);
  std::string field;
  while (stream >> field) fields.push_back(field);
  return fields;
}

std::vector<std::string> SplitTab(const std::string &line)
{
  std::vector<std::string> fields;
  std::stringstream stream(line);
  std::string field;
  while (std::getline(stream, field, '\t')) fields.push_back(field);
  return fields;
}

bool ParseDouble(const std::string &text, double &value)
{
  if (text.empty() || text == "nan" || text == "NaN" || text == "NAN") {
    return false;
  }
  char *end = nullptr;
  value = std::strtod(text.c_str(), &end);
  return end && *end == '\0' && std::isfinite(value);
}

bool ParseInt(const std::string &text, int &value)
{
  if (text.empty()) return false;
  char *end = nullptr;
  const long parsed = std::strtol(text.c_str(), &end, 10);
  if (!end || *end != '\0') return false;
  value = static_cast<int>(parsed);
  return true;
}

TString BaseName(const TString &path)
{
  const Ssiz_t slash = path.Last('/');
  if (slash == kNPOS) return path;
  return path(slash + 1, path.Length() - slash - 1);
}

// The calibration summary uses nfs_run_X_rY.root, whereas selected inputs are
// normally called mult3_nfs_run_X_rY.root. Normalize both to the same key.
// 刻度表通常记录 nfs_run_X_rY.root，而输入常为 mult3_nfs_run_X_rY.root；
// 去掉 mult3_ 前缀后即可逐 run 对应。
TString NormalizeRunKey(TString path)
{
  path = BaseName(path);
  if (path.BeginsWith("mult3_")) path.Remove(0, 6);
  return path;
}

std::vector<TString> ReadInputList(const TString &listPath)
{
  std::vector<TString> files;
  std::ifstream input(listPath.Data());
  if (!input.is_open()) {
    std::cerr << "Cannot open input list / 无法打开输入列表: " << listPath
              << std::endl;
    return files;
  }

  const TString listDir = gSystem->DirName(listPath);
  std::string line;
  while (std::getline(input, line)) {
    line = Trim(line);
    if (line.empty() || line[0] == '#' || line.rfind("//", 0) == 0) continue;
    TString path(line.c_str());
    gSystem->ExpandPathName(path);
    if (gSystem->AccessPathName(path, kReadPermission) &&
        !gSystem->IsAbsoluteFileName(path)) {
      TString relative = listDir;
      if (!relative.EndsWith("/")) relative += "/";
      relative += path;
      gSystem->ExpandPathName(relative);
      path = relative;
    }
    files.push_back(path);
  }
  return files;
}

// A single file, comma-separated files, and @list are accepted.
// 支持单文件、逗号分隔多文件和 @list。
std::vector<TString> ResolveInputs(const char *inputSpec)
{
  std::vector<TString> files;
  TString text(inputSpec ? inputSpec : "");
  text = text.Strip(TString::kBoth);
  if (text.BeginsWith("@")) {
    TString listPath = text(1, text.Length() - 1);
    listPath = listPath.Strip(TString::kBoth);
    gSystem->ExpandPathName(listPath);
    return ReadInputList(listPath);
  }

  TObjArray *tokens = text.Tokenize(",");
  for (int i = 0; i < tokens->GetEntriesFast(); ++i) {
    TString path = static_cast<TObjString *>(tokens->At(i))->GetString();
    path = path.Strip(TString::kBoth);
    if (path.IsNull()) continue;
    gSystem->ExpandPathName(path);
    files.push_back(path);
  }
  delete tokens;
  return files;
}

TString ResolveBranch(TTree *tree, const char *leafName)
{
  if (!tree || !leafName) return "";
  std::vector<TString> candidates = {
      TString(leafName), TString::Format("Exogam2.%s", leafName),
      TString::Format("Exogam2_%s", leafName),
      TString::Format("Exogam2/%s", leafName)};
  for (const auto &candidate : candidates) {
    if (tree->GetBranch(candidate)) return candidate;
  }
  TObjArray *branches = tree->GetListOfBranches();
  for (int i = 0; i < branches->GetEntriesFast(); ++i) {
    auto *branch = static_cast<TBranch *>(branches->At(i));
    const TString name = branch->GetName();
    if (name.EndsWith(leafName)) return name;
  }
  return "";
}

// -----------------------------------------------------------------------------
// Per-run calibration database / 逐 run 刻度数据库
// -----------------------------------------------------------------------------

struct CrystalCalibration {
  double energyOffset = 0.0;
  double energyGain = 1.0;
  double timeOffset = 0.0;
  double timeGain = 1.0;
  bool hasEnergyOffset = false;
  bool hasEnergyGain = false;
  bool hasTimeOffset = false;
  bool hasTimeGain = false;
};

struct RunCalibration {
  std::array<CrystalCalibration, kNDetector> crystal;
  int rows = 0;
};

struct CalibrationDatabase {
  std::map<TString, RunCalibration> byRun;
  RunCalibration defaultCalibration;
  bool hasDefault = false;
};

std::map<std::string, int> MakeHeaderMap(
    const std::vector<std::string> &header)
{
  std::map<std::string, int> result;
  for (std::size_t i = 0; i < header.size(); ++i) {
    result[header[i]] = static_cast<int>(i);
  }
  return result;
}

std::string GetColumn(const std::vector<std::string> &columns,
                      const std::map<std::string, int> &header,
                      const std::string &name)
{
  auto found = header.find(name);
  if (found == header.end() || found->second < 0 ||
      static_cast<std::size_t>(found->second) >= columns.size()) {
    return "";
  }
  return columns[found->second];
}

CalibrationDatabase ReadCalibrationDatabase(const char *summaryPath)
{
  CalibrationDatabase database;
  std::ifstream input(summaryPath ? summaryPath : "");
  if (!input.is_open()) {
    std::cerr << "Cannot open calibration summary / 无法打开刻度汇总: "
              << (summaryPath ? summaryPath : "") << std::endl;
    return database;
  }

  std::vector<std::string> header;
  std::map<std::string, int> headerMap;
  bool tabSeparated = true;
  std::string line;
  while (std::getline(input, line)) {
    line = Trim(line);
    if (line.empty()) continue;

    // A per-run detailed calibration txt begins with "# clover ..." and has
    // no run_file column. Treat it as a default calibration for every input.
    // 单 run 的详细 txt 以 "# clover ..." 开头且没有 run_file；将其作为默认刻度。
    if (line.rfind("# clover", 0) == 0) {
      line = Trim(line.substr(1));
      header = SplitWhitespace(line);
      headerMap = MakeHeaderMap(header);
      tabSeparated = false;
      continue;
    }
    if (line[0] == '#') continue;
    if (header.empty()) {
      tabSeparated = line.find('\t') != std::string::npos;
      header = tabSeparated ? SplitTab(line) : SplitWhitespace(line);
      headerMap = MakeHeaderMap(header);
      continue;
    }

    std::vector<std::string> columns =
        tabSeparated ? SplitTab(line) : SplitWhitespace(line);
    if (tabSeparated && columns.size() < header.size()) {
      columns = SplitWhitespace(line);
    }

    const std::string status = GetColumn(columns, headerMap, "status");
    if (!status.empty() && status != "ok") continue;

    int detector = -1;
    if (!ParseInt(GetColumn(columns, headerMap, "detector"), detector)) {
      int clover = -1;
      int crystal = -1;
      if (ParseInt(GetColumn(columns, headerMap, "clover"), clover) &&
          ParseInt(GetColumn(columns, headerMap, "crystal"), crystal)) {
        detector = clover * kNCrystalPerClover + crystal;
      }
    }
    if (detector < 0 || detector >= kNDetector) continue;

    TString key = "*";
    std::string runFile = GetColumn(columns, headerMap, "run_file");
    if (runFile.empty()) runFile = GetColumn(columns, headerMap, "input");
    if (!runFile.empty()) key = NormalizeRunKey(TString(runFile.c_str()));

    RunCalibration &run =
        key == "*" ? database.defaultCalibration : database.byRun[key];
    if (key == "*") database.hasDefault = true;
    CrystalCalibration &calibration = run.crystal[detector];

    double value = 0.0;
    if (ParseDouble(GetColumn(columns, headerMap, "energy_offset"), value)) {
      calibration.energyOffset = value;
      calibration.hasEnergyOffset = true;
    }
    if (ParseDouble(GetColumn(columns, headerMap, "energy_gain"), value)) {
      calibration.energyGain = value;
      calibration.hasEnergyGain = true;
    }

    std::string timeOffset =
        GetColumn(columns, headerMap, "time_offset_to_reference_ns");
    if (timeOffset.empty()) {
      timeOffset = GetColumn(columns, headerMap, "time_offset_to_442_ns");
    }
    if (ParseDouble(timeOffset, value)) {
      calibration.timeOffset = value;
      calibration.hasTimeOffset = true;
    }

    std::string timeGain =
        GetColumn(columns, headerMap, "time_gain_to_reference");
    if (timeGain.empty()) {
      timeGain = GetColumn(columns, headerMap, "time_gain_to_442");
    }
    if (ParseDouble(timeGain, value)) {
      calibration.timeGain = value;
      calibration.hasTimeGain = true;
    }
    run.rows++;
  }
  return database;
}

const RunCalibration *FindRunCalibration(const CalibrationDatabase &database,
                                         const TString &inputPath)
{
  const TString normalized = NormalizeRunKey(inputPath);
  auto found = database.byRun.find(normalized);
  if (found != database.byRun.end()) return &found->second;

  const TString exact = BaseName(inputPath);
  found = database.byRun.find(exact);
  if (found != database.byRun.end()) return &found->second;
  if (database.hasDefault) return &database.defaultCalibration;
  return nullptr;
}

bool HasTimeCalibration(const CrystalCalibration &calibration,
                        const TString &mode)
{
  if (mode.EqualTo("gain", TString::kIgnoreCase)) {
    return calibration.hasTimeGain;
  }
  if (mode.EqualTo("offset_gain", TString::kIgnoreCase) ||
      mode.EqualTo("gain_offset", TString::kIgnoreCase)) {
    return calibration.hasTimeOffset && calibration.hasTimeGain;
  }
  return calibration.hasTimeOffset;
}

double ApplyTimeCalibration(double rawTime,
                            const CrystalCalibration &calibration,
                            const TString &mode)
{
  if (mode.EqualTo("gain", TString::kIgnoreCase)) {
    return rawTime * calibration.timeGain;
  }
  if (mode.EqualTo("offset_gain", TString::kIgnoreCase)) {
    return (rawTime + calibration.timeOffset) * calibration.timeGain;
  }
  if (mode.EqualTo("gain_offset", TString::kIgnoreCase)) {
    return rawTime * calibration.timeGain + calibration.timeOffset;
  }
  return rawTime + calibration.timeOffset;
}

// -----------------------------------------------------------------------------
// ASCII GLS/AGS parser / ASCII GLS/AGS 解析
// -----------------------------------------------------------------------------

struct AgsLevel {
  int index = -1;
  double energyKeV = 0.0;
  double energyErrorKeV = 0.0;
  std::string spinParity;
  int band = -1;
};

struct AgsTransition {
  int gammaIndex = -1;
  double energyKeV = 0.0;
  double energyErrorKeV = 0.0;
  std::string multipolarity;
  int initialLevel = -1;
  int finalLevel = -1;
};

struct SelectedTransition {
  int transitionIndex = -1;
  int gateIndex = -1;
  int gammaIndex = -1;
  std::string kind;
  std::string nuclide;
  std::string initialBandName;
  std::string finalBandName;
  int initialLevel = -1;
  int finalLevel = -1;
  double initialLevelEnergyKeV = 0.0;
  double finalLevelEnergyKeV = 0.0;
  std::string initialSpin;
  std::string finalSpin;
  double gammaEnergyKeV = 0.0;
  double gammaEnergyErrorKeV = 0.0;
  std::string multipolarity;
  bool usedAsGate = false;
};

struct AgsSelection {
  std::vector<SelectedTransition> transitions;
  std::vector<int> gateTransitionIndices;
  int levelCount = 0;
  int bandCount = 0;
  int gammaCount = 0;
};

std::string CanonicalNuclide(std::string bandName)
{
  // GLS authors often append a/b/c to distinguish bands of the same nuclide,
  // for example zr100a. Preserve the raw band name separately and remove only
  // this trailing band suffix from the canonical nuclide label.
  // GLS 常用 a/b/c 表示同一核素的不同 band，例如 zr100a。raw band 名会另存；
  // canonical nuclide 只去掉这种“数字后的一位 a/b/c”后缀。
  if (bandName.size() >= 2) {
    const char last = static_cast<char>(std::tolower(
        static_cast<unsigned char>(bandName.back())));
    const char before = bandName[bandName.size() - 2];
    if ((last == 'a' || last == 'b' || last == 'c') &&
        std::isdigit(static_cast<unsigned char>(before))) {
      bandName.pop_back();
    }
  }
  return bandName;
}

std::string NormalizeSpin(std::string spin)
{
  spin = Trim(spin);
  while (spin.size() >= 2 &&
         ((spin.front() == '(' && spin.back() == ')') ||
          (spin.front() == '[' && spin.back() == ']'))) {
    spin = spin.substr(1, spin.size() - 2);
  }
  return spin;
}

AgsSelection ParseAgsTransitions(const char *agsPath)
{
  AgsSelection result;
  std::ifstream input(agsPath ? agsPath : "");
  if (!input.is_open()) {
    std::cerr << "Cannot open AGS file / 无法打开 AGS 文件: "
              << (agsPath ? agsPath : "") << std::endl;
    return result;
  }

  enum class Section { kNone, kLevels, kBands, kGammas };
  Section section = Section::kNone;
  std::map<int, AgsLevel> levels;
  std::map<int, std::string> bands;
  std::vector<AgsTransition> gammas;

  std::string line;
  while (std::getline(input, line)) {
    line = Trim(line);
    if (line.empty()) continue;
    if (line.rfind("** Level", 0) == 0) {
      section = Section::kLevels;
      continue;
    }
    if (line.rfind("** Band", 0) == 0) {
      section = Section::kBands;
      continue;
    }
    if (line.rfind("** Gamma", 0) == 0) {
      section = Section::kGammas;
      continue;
    }
    if (line.rfind("** Label", 0) == 0) {
      section = Section::kNone;
      continue;
    }
    if (line.rfind("**", 0) == 0 || line.rfind("++", 0) == 0) continue;

    std::stringstream fields(line);
    if (section == Section::kLevels) {
      AgsLevel level;
      int kValue = 0;
      if (fields >> level.index >> level.energyKeV >> level.energyErrorKeV >>
              level.spinParity >> kValue >> level.band) {
        level.spinParity = NormalizeSpin(level.spinParity);
        levels[level.index] = level;
      }
    } else if (section == Section::kBands) {
      int index = -1;
      std::string name;
      if (fields >> index >> name) bands[index] = name;
    } else if (section == Section::kGammas) {
      AgsTransition gamma;
      std::string multipoleType;
      int multipoleOrder = 0;
      if (fields >> gamma.gammaIndex >> gamma.energyKeV >>
              gamma.energyErrorKeV >> multipoleType >> multipoleOrder >>
              gamma.initialLevel >> gamma.finalLevel) {
        gamma.multipolarity = multipoleType + std::to_string(multipoleOrder);
        gammas.push_back(gamma);
      }
    }
  }

  result.levelCount = static_cast<int>(levels.size());
  result.bandCount = static_cast<int>(bands.size());
  result.gammaCount = static_cast<int>(gammas.size());

  int gateIndex = 0;
  for (const auto &gamma : gammas) {
    const auto initialFound = levels.find(gamma.initialLevel);
    const auto finalFound = levels.find(gamma.finalLevel);
    if (initialFound == levels.end() || finalFound == levels.end()) continue;
    const AgsLevel &initial = initialFound->second;
    const AgsLevel &final = finalFound->second;

    const bool isTwoToZero =
        initial.spinParity == "2+" && final.spinParity == "0+";
    const bool isFourToTwo =
        initial.spinParity == "4+" && final.spinParity == "2+";
    if (!isTwoToZero && !isFourToTwo) continue;

    SelectedTransition selected;
    selected.transitionIndex = static_cast<int>(result.transitions.size());
    selected.gateIndex = isTwoToZero ? gateIndex++ : -1;
    selected.gammaIndex = gamma.gammaIndex;
    selected.kind = isTwoToZero ? "2plus_to_0plus" : "4plus_to_2plus";
    selected.initialLevel = initial.index;
    selected.finalLevel = final.index;
    selected.initialLevelEnergyKeV = initial.energyKeV;
    selected.finalLevelEnergyKeV = final.energyKeV;
    selected.initialSpin = initial.spinParity;
    selected.finalSpin = final.spinParity;
    selected.gammaEnergyKeV = gamma.energyKeV;
    selected.gammaEnergyErrorKeV = gamma.energyErrorKeV;
    selected.multipolarity = gamma.multipolarity;

    auto initialBand = bands.find(initial.band);
    auto finalBand = bands.find(final.band);
    selected.initialBandName =
        initialBand == bands.end() ? "unknown" : initialBand->second;
    selected.finalBandName =
        finalBand == bands.end() ? "unknown" : finalBand->second;
    selected.nuclide = CanonicalNuclide(selected.initialBandName);
    selected.usedAsGate = isTwoToZero;

    if (isTwoToZero) {
      result.gateTransitionIndices.push_back(selected.transitionIndex);
    }
    result.transitions.push_back(selected);
  }
  return result;
}

// One cascade pair is an actual 4+ -> 2+ -> 0+ path, not merely two gamma
// energies belonging to the same nuclide. The final level of the first
// transition must be the initial level of the second transition.
// 一个级联对必须是真实的 4+ -> 2+ -> 0+ 路径，而不是把同核素的任意两条 gamma
// 随意组合；第一条跃迁的终态 level 必须等于第二条跃迁的初态 level。
struct CascadePair {
  int pairIndex = -1;
  int fourToTwoTransitionIndex = -1;
  int twoToZeroTransitionIndex = -1;
  int intermediateLevel = -1;
  std::string nuclide;
  TString histogramName;
  TH1I *spectrum = nullptr;
  Long64_t matchedEvents = 0;
  Long64_t matchedGatePairs = 0;
  Long64_t residualGammaFills = 0;
};

// Keep the physical energy readable in the ROOT object name. Three decimals
// retain the precision carried by this AGS file; redundant zeroes are removed.
// ROOT 对象名保留可读的物理能量，最多保留三位小数，并去掉末尾多余的零。
TString CascadeEnergyName(double energyKeV)
{
  TString text = TString::Format("%.3f", energyKeV);
  while (text.EndsWith("0")) text.Chop();
  if (text.EndsWith(".")) text.Chop();
  return text;
}

std::vector<CascadePair> BuildCascadePairs(const AgsSelection &ags)
{
  std::vector<CascadePair> pairs;
  std::map<std::string, int> histogramNameCounts;

  for (const auto &fourToTwo : ags.transitions) {
    if (fourToTwo.kind != "4plus_to_2plus") continue;

    for (const auto &twoToZero : ags.transitions) {
      if (twoToZero.kind != "2plus_to_0plus") continue;

      // Exact cascade requirement / 严格级联条件:
      //   4+ -> [the same 2+ level] -> 0+
      // Level indices are global within one AGS file, so equality identifies
      // the shared physical intermediate state without an energy tolerance.
      // AGS 内 level index 是全局编号，因此直接相等即可确定同一个中间物理态，
      // 不需要再用能级能量容差做模糊匹配。
      if (fourToTwo.finalLevel != twoToZero.initialLevel) continue;

      const std::string intermediateNuclide =
          CanonicalNuclide(fourToTwo.finalBandName);
      if (fourToTwo.nuclide != twoToZero.nuclide ||
          intermediateNuclide != twoToZero.nuclide) {
        continue;
      }

      CascadePair pair;
      pair.pairIndex = static_cast<int>(pairs.size());
      pair.fourToTwoTransitionIndex = fourToTwo.transitionIndex;
      pair.twoToZeroTransitionIndex = twoToZero.transitionIndex;
      pair.intermediateLevel = fourToTwo.finalLevel;
      pair.nuclide = twoToZero.nuclide;

      const TString baseName = TString::Format(
          "%s-%s", CascadeEnergyName(fourToTwo.gammaEnergyKeV).Data(),
          CascadeEnergyName(twoToZero.gammaEnergyKeV).Data());
      const int duplicateIndex = histogramNameCounts[baseName.Data()]++;
      pair.histogramName = duplicateIndex == 0
                               ? baseName
                               : TString::Format("%s_pair%d", baseName.Data(),
                                                 duplicateIndex + 1);
      pairs.push_back(pair);
    }
  }
  return pairs;
}

// -----------------------------------------------------------------------------
// Neutron-energy windows and event reconstruction / 中子能区与事件重建
// -----------------------------------------------------------------------------

struct NeutronWindow {
  double lowMeV = 0.0;
  double highMeV = 0.0;
  TString tag;
  TString title;
  TH2I *matrix = nullptr;
  Long64_t acceptedEvents = 0;
  Long64_t qualifiedGateInstances = 0;
  Long64_t unorderedResidualPairs = 0;
  Long64_t symmetricFills = 0;
};

TString NumberTag(double value)
{
  TString text;
  if (std::fabs(value - std::round(value)) < 1e-9) text.Form("%.0f", value);
  else text.Form("%.3f", value);
  text.ReplaceAll("-", "m");
  text.ReplaceAll(".", "p");
  return text;
}

std::vector<NeutronWindow> ParseNeutronWindows(const char *text)
{
  std::vector<NeutronWindow> windows;
  TString source(text ? text : "");
  TObjArray *tokens = source.Tokenize(",");
  for (int i = 0; i < tokens->GetEntriesFast(); ++i) {
    TString token = static_cast<TObjString *>(tokens->At(i))->GetString();
    token = token.Strip(TString::kBoth);
    token.ReplaceAll(":", " ");
    token.ReplaceAll("-", " ");
    std::stringstream fields(token.Data());
    NeutronWindow window;
    if (!(fields >> window.lowMeV >> window.highMeV) ||
        window.lowMeV < 0.0 || window.highMeV <= window.lowMeV) {
      std::cerr << "Ignore invalid neutron window / 忽略无效中子能区: "
                << token << std::endl;
      continue;
    }
    window.tag = TString::Format("En%s_%sMeV", NumberTag(window.lowMeV).Data(),
                                 NumberTag(window.highMeV).Data());
    window.title = TString::Format("%.6g <= E_{n} < %.6g MeV",
                                   window.lowMeV, window.highMeV);
    windows.push_back(window);
  }
  delete tokens;
  return windows;
}

double EnergyMeVToToFNs(double energyMeV, double distanceMeter)
{
  if (energyMeV <= 0.0 || distanceMeter <= 0.0) {
    return std::numeric_limits<double>::infinity();
  }
  const double beta = std::sqrt(2.0 * energyMeV / kNeutronMassMeV);
  return distanceMeter / (kLightSpeedMeterPerNs * beta);
}

double ToFNsToEnergyMeV(double timeNs, double distanceMeter)
{
  if (timeNs <= 0.0 || distanceMeter <= 0.0) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  const double beta = distanceMeter / (timeNs * kLightSpeedMeterPerNs);
  return 0.5 * kNeutronMassMeV * beta * beta;
}

bool InWindow(double energyMeV, const NeutronWindow &window)
{
  return energyMeV >= window.lowMeV && energyMeV < window.highMeV;
}

// Gate edges are inclusive, matching triple_gamma_gate_fill.C.
// gate 的两个边界都包含在内，与 triple_gamma_gate_fill.C 保持一致。
bool MatchesGate(double energyKeV, double centerKeV, double halfWidthKeV)
{
  return std::isfinite(energyKeV) &&
         energyKeV >= centerKeV - halfWidthKeV &&
         energyKeV <= centerKeV + halfWidthKeV;
}

struct CloverHit {
  double energyKeV = 0.0;
  double timeNs = std::numeric_limits<double>::infinity();
  double bestCrystalEnergyKeV = -1.0;
  bool bgo = false;
  bool csi = false;
  int crystalMultiplicity = 0;
};

void FillResidualPairs(TH2I *matrix,
                       const std::vector<double> &energies,
                       std::size_t removedIndex,
                       Long64_t &unorderedPairs,
                       Long64_t &symmetricFills)
{
  if (!matrix || energies.size() < 3 || removedIndex >= energies.size()) return;
  for (std::size_t first = 0; first < energies.size(); ++first) {
    if (first == removedIndex) continue;
    for (std::size_t second = first + 1; second < energies.size(); ++second) {
      if (second == removedIndex) continue;
      matrix->Fill(energies[first], energies[second]);
      matrix->Fill(energies[second], energies[first]);
      ++unorderedPairs;
      symmetricFills += 2;
    }
  }
}

void WriteTransitionTree(const AgsSelection &ags)
{
  TTree tree("AGSTransitions",
             "AGS 2+->0+ and 4+->2+ transitions used by this analysis");
  Int_t transitionIndex = -1;
  Int_t gateIndex = -1;
  Int_t gammaIndex = -1;
  std::string kind;
  std::string nuclide;
  std::string initialBand;
  std::string finalBand;
  Int_t initialLevel = -1;
  Int_t finalLevel = -1;
  Double_t initialLevelEnergyKeV = 0.0;
  Double_t finalLevelEnergyKeV = 0.0;
  std::string initialSpin;
  std::string finalSpin;
  Double_t gammaEnergyKeV = 0.0;
  Double_t gammaEnergyErrorKeV = 0.0;
  std::string multipolarity;
  Bool_t usedAsGate = false;

  tree.Branch("transition_index", &transitionIndex);
  tree.Branch("gate_index", &gateIndex);
  tree.Branch("gamma_index", &gammaIndex);
  tree.Branch("kind", &kind);
  tree.Branch("nuclide", &nuclide);
  tree.Branch("initial_band", &initialBand);
  tree.Branch("final_band", &finalBand);
  tree.Branch("initial_level", &initialLevel);
  tree.Branch("final_level", &finalLevel);
  tree.Branch("initial_level_energy_keV", &initialLevelEnergyKeV);
  tree.Branch("final_level_energy_keV", &finalLevelEnergyKeV);
  tree.Branch("initial_spin", &initialSpin);
  tree.Branch("final_spin", &finalSpin);
  tree.Branch("gamma_energy_keV", &gammaEnergyKeV);
  tree.Branch("gamma_energy_error_keV", &gammaEnergyErrorKeV);
  tree.Branch("multipolarity", &multipolarity);
  tree.Branch("used_as_gate", &usedAsGate);

  for (const auto &transition : ags.transitions) {
    transitionIndex = transition.transitionIndex;
    gateIndex = transition.gateIndex;
    gammaIndex = transition.gammaIndex;
    kind = transition.kind;
    nuclide = transition.nuclide;
    initialBand = transition.initialBandName;
    finalBand = transition.finalBandName;
    initialLevel = transition.initialLevel;
    finalLevel = transition.finalLevel;
    initialLevelEnergyKeV = transition.initialLevelEnergyKeV;
    finalLevelEnergyKeV = transition.finalLevelEnergyKeV;
    initialSpin = transition.initialSpin;
    finalSpin = transition.finalSpin;
    gammaEnergyKeV = transition.gammaEnergyKeV;
    gammaEnergyErrorKeV = transition.gammaEnergyErrorKeV;
    multipolarity = transition.multipolarity;
    usedAsGate = transition.usedAsGate;
    tree.Fill();
  }
  tree.Write();
}

void WriteCascadePairTree(const std::vector<CascadePair> &pairs,
                          const AgsSelection &ags,
                          double halfGateWidthKeV)
{
  TTree tree("CascadePairs",
             "Matched 4+->2+->0+ AGS cascades and gated-spectrum statistics");
  Int_t pairIndex = -1;
  std::string nuclide;
  Int_t intermediateLevel = -1;
  std::string histogramName;
  Int_t fourToTwoTransitionIndex = -1;
  Int_t fourToTwoGammaIndex = -1;
  Double_t fourToTwoEnergyKeV = 0.0;
  Double_t fourToTwoGateLowKeV = 0.0;
  Double_t fourToTwoGateHighKeV = 0.0;
  Int_t twoToZeroTransitionIndex = -1;
  Int_t twoToZeroGammaIndex = -1;
  Double_t twoToZeroEnergyKeV = 0.0;
  Double_t twoToZeroGateLowKeV = 0.0;
  Double_t twoToZeroGateHighKeV = 0.0;
  Long64_t matchedEvents = 0;
  Long64_t matchedGatePairs = 0;
  Long64_t residualGammaFills = 0;

  tree.Branch("pair_index", &pairIndex);
  tree.Branch("nuclide", &nuclide);
  tree.Branch("intermediate_level", &intermediateLevel);
  tree.Branch("histogram_name", &histogramName);
  tree.Branch("four_to_two_transition_index", &fourToTwoTransitionIndex);
  tree.Branch("four_to_two_gamma_index", &fourToTwoGammaIndex);
  tree.Branch("four_to_two_energy_keV", &fourToTwoEnergyKeV);
  tree.Branch("four_to_two_gate_low_keV", &fourToTwoGateLowKeV);
  tree.Branch("four_to_two_gate_high_keV", &fourToTwoGateHighKeV);
  tree.Branch("two_to_zero_transition_index", &twoToZeroTransitionIndex);
  tree.Branch("two_to_zero_gamma_index", &twoToZeroGammaIndex);
  tree.Branch("two_to_zero_energy_keV", &twoToZeroEnergyKeV);
  tree.Branch("two_to_zero_gate_low_keV", &twoToZeroGateLowKeV);
  tree.Branch("two_to_zero_gate_high_keV", &twoToZeroGateHighKeV);
  tree.Branch("matched_events", &matchedEvents);
  tree.Branch("matched_gate_pairs", &matchedGatePairs);
  tree.Branch("residual_gamma_fills", &residualGammaFills);

  for (const auto &pair : pairs) {
    if (pair.fourToTwoTransitionIndex < 0 ||
        pair.twoToZeroTransitionIndex < 0 ||
        static_cast<std::size_t>(pair.fourToTwoTransitionIndex) >=
            ags.transitions.size() ||
        static_cast<std::size_t>(pair.twoToZeroTransitionIndex) >=
            ags.transitions.size()) {
      continue;
    }
    const SelectedTransition &fourToTwo =
        ags.transitions[pair.fourToTwoTransitionIndex];
    const SelectedTransition &twoToZero =
        ags.transitions[pair.twoToZeroTransitionIndex];

    pairIndex = pair.pairIndex;
    nuclide = pair.nuclide;
    intermediateLevel = pair.intermediateLevel;
    histogramName = pair.histogramName.Data();
    fourToTwoTransitionIndex = pair.fourToTwoTransitionIndex;
    fourToTwoGammaIndex = fourToTwo.gammaIndex;
    fourToTwoEnergyKeV = fourToTwo.gammaEnergyKeV;
    fourToTwoGateLowKeV = fourToTwo.gammaEnergyKeV - halfGateWidthKeV;
    fourToTwoGateHighKeV = fourToTwo.gammaEnergyKeV + halfGateWidthKeV;
    twoToZeroTransitionIndex = pair.twoToZeroTransitionIndex;
    twoToZeroGammaIndex = twoToZero.gammaIndex;
    twoToZeroEnergyKeV = twoToZero.gammaEnergyKeV;
    twoToZeroGateLowKeV = twoToZero.gammaEnergyKeV - halfGateWidthKeV;
    twoToZeroGateHighKeV = twoToZero.gammaEnergyKeV + halfGateWidthKeV;
    matchedEvents = pair.matchedEvents;
    matchedGatePairs = pair.matchedGatePairs;
    residualGammaFills = pair.residualGammaFills;
    tree.Fill();
  }
  tree.Write();
}

} // namespace ags_2plus_gate_detail

void ags_2plus_gated_gamma_gamma_calibrated(
    const char *inputFiles,
    const char *calibrationSummary,
    const char *agsFile = "/home/user0/work/IJCLAB/nfs_ana/nfs_ags_no_correction_results/_extracted_ags/th232_18_34to24_34.ags",
    const char *outputFile = "ags_2plus_gated_gamma_gamma_calibrated.root",
    const char *neutronWindowsMeV = "6:8,10:13,17:20",
    double gateWidthKeV = 3.0,
    double nfsDistanceMeter = 24.0,
    double timeFwhmNs = 20.0,
    bool useBgoCsiVeto = true,
    Long64_t maxEntries = -1,
    int gammaBins = 4096,
    double gammaMinKeV = 0.0,
    double gammaMaxKeV = 4096.0,
    const char *timeCorrectionMode = "offset",
    const char *timeBranchLeaf = "fTime")
{
  using namespace ags_2plus_gate_detail;

  if (!inputFiles || TString(inputFiles).Strip(TString::kBoth).IsNull() ||
      !calibrationSummary ||
      TString(calibrationSummary).Strip(TString::kBoth).IsNull()) {
    std::cerr << "Input ROOT and calibration summary are required. / "
                 "必须提供输入 ROOT 和刻度汇总。"
              << std::endl;
    return;
  }
  if (gateWidthKeV < 1.0 || nfsDistanceMeter <= 0.0 ||
      timeFwhmNs <= 0.0 || gammaBins <= 0 || gammaMaxKeV <= gammaMinKeV) {
    std::cerr << "Invalid analysis parameters / 分析参数无效" << std::endl;
    return;
  }

  const std::vector<TString> inputs = ResolveInputs(inputFiles);
  if (inputs.empty()) {
    std::cerr << "No input ROOT files / 没有输入 ROOT 文件" << std::endl;
    return;
  }

  CalibrationDatabase calibration =
      ReadCalibrationDatabase(calibrationSummary);
  if (calibration.byRun.empty() && !calibration.hasDefault) {
    std::cerr << "No usable calibration rows / 刻度汇总中没有可用行: "
              << calibrationSummary << std::endl;
    return;
  }

  const AgsSelection ags = ParseAgsTransitions(agsFile);
  if (ags.gateTransitionIndices.empty()) {
    std::cerr << "No 2+ -> 0+ gate found in AGS / AGS 中没有找到 2+ -> 0+"
              << std::endl;
    return;
  }
  std::vector<CascadePair> cascadePairs = BuildCascadePairs(ags);
  if (cascadePairs.empty()) {
    std::cerr << "No shared-level 4+ -> 2+ -> 0+ cascade found / "
                 "没有找到共享中间 level 的 4+ -> 2+ -> 0+ 级联"
              << std::endl;
  }


  std::vector<NeutronWindow> windows =
      ParseNeutronWindows(neutronWindowsMeV);
  if (windows.empty()) {
    std::cerr << "No valid neutron-energy window / 没有有效中子能区"
              << std::endl;
    return;
  }

  TH2I totalMatrix(
      "AGS2GateGammaGamma_Total",
      "All AGS 2+ gates, all accepted neutron energies;Residual calibrated gamma energy (keV);Residual calibrated gamma energy (keV)",
      gammaBins, gammaMinKeV, gammaMaxKeV,
      gammaBins, gammaMinKeV, gammaMaxKeV);
  totalMatrix.SetDirectory(nullptr);

  for (auto &window : windows) {
    window.matrix = new TH2I(
        TString::Format("AGS2GateGammaGamma_%s", window.tag.Data()),
        TString::Format("All AGS 2+ gates, %s;Residual calibrated gamma energy (keV);Residual calibrated gamma energy (keV)",
                        window.title.Data()),
        gammaBins, gammaMinKeV, gammaMaxKeV,
        gammaBins, gammaMinKeV, gammaMaxKeV);
    window.matrix->SetDirectory(nullptr);
  }

  for (auto &pair : cascadePairs) {
    const SelectedTransition &fourToTwo =
        ags.transitions[pair.fourToTwoTransitionIndex];
    const SelectedTransition &twoToZero =
        ags.transitions[pair.twoToZeroTransitionIndex];
    const TString title = TString::Format(
        "%s cascade, %.3f keV (4+ -> 2+) and %.3f keV (2+ -> 0+);Remaining calibrated gamma energy (keV);Counts",
        pair.nuclide.c_str(), fourToTwo.gammaEnergyKeV,
        twoToZero.gammaEnergyKeV);
    pair.spectrum = new TH1I(pair.histogramName, title, gammaBins,
                             gammaMinKeV, gammaMaxKeV);
    pair.spectrum->SetDirectory(nullptr);
    pair.spectrum->GetXaxis()->SetTitle(
        "Remaining calibrated gamma energy (keV)");
    pair.spectrum->GetYaxis()->SetTitle("Counts");
  }

  TH1D neutronEnergy(
      "AGS2GateAcceptedNeutronEnergy",
      "Neutron energy for events with at least one qualified AGS 2+ gate;Calibrated neutron energy (MeV);Events",
      1000, 0.0, 50.0);
  neutronEnergy.SetDirectory(nullptr);

  const std::size_t gateCount = ags.gateTransitionIndices.size();
  std::vector<Long64_t> gateMatchedEvents(gateCount, 0);
  std::vector<Long64_t> gateHitInstances(gateCount, 0);
  std::vector<Long64_t> gateQualifiedInstances(gateCount, 0);
  std::vector<Long64_t> gateUnorderedPairs(gateCount, 0);
  std::vector<Long64_t> gateSymmetricFills(gateCount, 0);

  Long64_t totalEntries = 0;
  Long64_t processedFiles = 0;
  Long64_t filesWithoutCalibration = 0;
  Long64_t malformedEntries = 0;
  Long64_t crystalCalibrationMisses = 0;
  Long64_t eventsWithAtLeastThreeGammas = 0;
  Long64_t eventsMatchingAnyGate = 0;
  Long64_t acceptedEvents = 0;
  Long64_t totalQualifiedGateInstances = 0;
  Long64_t totalUnorderedPairs = 0;
  Long64_t totalSymmetricFills = 0;
  Long64_t eventsMatchingAnyCascade = 0;
  Long64_t totalCascadeMatchedGatePairs = 0;
  Long64_t totalCascadeResidualGammaFills = 0;


  const double tMinNs =
      EnergyMeVToToFNs(kFastNeutronLimitMeV, nfsDistanceMeter);
  const double timeSigmaNs = timeFwhmNs / 2.355;
  const double tCutNs = tMinNs - 3.0 * timeSigmaNs;
  // Width counts 1-keV channels: width=3 gives center-1, center, center+1.
  // 宽度表示 1 keV channel 数：width=3 对应中心 -1、中心、中心 +1。
  const double halfGateWidthKeV = 0.5 * (gateWidthKeV - 1.0);
  const TString timeMode(timeCorrectionMode ? timeCorrectionMode : "offset");

  for (const auto &inputPath : inputs) {
    if (maxEntries > 0 && totalEntries >= maxEntries) break;
    TFile input(inputPath, "READ");
    if (!input.IsOpen() || input.IsZombie()) {
      std::cerr << "Cannot open input ROOT / 无法打开输入 ROOT: "
                << inputPath << std::endl;
      continue;
    }
    TTree *tree = dynamic_cast<TTree *>(input.Get("TreeMaster"));
    if (!tree) {
      std::cerr << "TreeMaster is missing / 缺少 TreeMaster: " << inputPath
                << std::endl;
      continue;
    }

    const RunCalibration *runCalibration =
        FindRunCalibration(calibration, inputPath);
    if (!runCalibration) {
      std::cerr << "Skip file without matching calibration / 跳过无对应刻度文件: "
                << inputPath << std::endl;
      ++filesWithoutCalibration;
      continue;
    }

    const TString cloverBranch = ResolveBranch(tree, "fEXO_ECC_E_Clover");
    const TString crystalBranch = ResolveBranch(tree, "fEXO_ECC_E_Cristal");
    const TString energyBranch = ResolveBranch(tree, "fEXO_ECC_E_Energy");
    const TString timeBranch = ResolveBranch(
        tree, timeBranchLeaf && TString(timeBranchLeaf).Length() > 0
                  ? timeBranchLeaf
                  : "fTime");
    const TString essCloverBranch = ResolveBranch(tree, "fEXO_ESS_Clover");
    const TString essCrystalBranch = ResolveBranch(tree, "fEXO_ESS_Cristal");
    const TString bgoBranch = ResolveBranch(tree, "fEXO_ESS_BGO");
    const TString csiBranch = ResolveBranch(tree, "fEXO_ESS_CSI");

    if (cloverBranch.IsNull() || crystalBranch.IsNull() ||
        energyBranch.IsNull() || timeBranch.IsNull()) {
      std::cerr << "Required crystal branches are missing / 缺少必要 crystal 分支: "
                << inputPath << std::endl;
      continue;
    }
    if (useBgoCsiVeto &&
        (essCloverBranch.IsNull() || essCrystalBranch.IsNull() ||
         bgoBranch.IsNull() || csiBranch.IsNull())) {
      std::cerr << "BGO/CSI veto branches are missing / 缺少 BGO/CSI veto 分支: "
                << inputPath << std::endl;
      continue;
    }

    ++processedFiles;
    TTreeReader reader(tree);
    TTreeReaderArray<UShort_t> clovers(reader, cloverBranch.Data());
    TTreeReaderArray<UShort_t> crystals(reader, crystalBranch.Data());
    TTreeReaderArray<float> rawEnergies(reader, energyBranch.Data());
    TTreeReaderArray<float> rawTimes(reader, timeBranch.Data());
    std::unique_ptr<TTreeReaderArray<UShort_t>> essClovers;
    std::unique_ptr<TTreeReaderArray<UShort_t>> essCrystals;
    std::unique_ptr<TTreeReaderArray<UShort_t>> bgo;
    std::unique_ptr<TTreeReaderArray<UShort_t>> csi;
    if (useBgoCsiVeto) {
      essClovers.reset(
          new TTreeReaderArray<UShort_t>(reader, essCloverBranch.Data()));
      essCrystals.reset(
          new TTreeReaderArray<UShort_t>(reader, essCrystalBranch.Data()));
      bgo.reset(new TTreeReaderArray<UShort_t>(reader, bgoBranch.Data()));
      csi.reset(new TTreeReaderArray<UShort_t>(reader, csiBranch.Data()));
    }

    while (reader.Next()) {
      if (maxEntries > 0 && totalEntries >= maxEntries) break;
      ++totalEntries;

      const std::size_t fireCount =
          static_cast<std::size_t>(rawEnergies.GetSize());
      if (clovers.GetSize() != rawEnergies.GetSize() ||
          crystals.GetSize() != rawEnergies.GetSize() ||
          rawTimes.GetSize() != rawEnergies.GetSize()) {
        ++malformedEntries;
        continue;
      }

      std::array<CloverHit, kNClover> cloverHits;
      if (useBgoCsiVeto) {
        const std::size_t vetoCount = std::min(
            {static_cast<std::size_t>(essClovers->GetSize()),
             static_cast<std::size_t>(essCrystals->GetSize()),
             static_cast<std::size_t>(bgo->GetSize()),
             static_cast<std::size_t>(csi->GetSize())});
        for (std::size_t i = 0; i < vetoCount; ++i) {
          const int clover = (*essClovers)[i];
          if (clover < 0 || clover >= kNClover) continue;
          if ((*bgo)[i] > 0) cloverHits[clover].bgo = true;
          if ((*csi)[i] > 0) cloverHits[clover].csi = true;
        }
      }

      for (std::size_t i = 0; i < fireCount; ++i) {
        const int clover = clovers[i];
        const int crystal = crystals[i];
        if (clover < 0 || clover >= kNClover ||
            crystal < 0 || crystal >= kNCrystalPerClover) {
          continue;
        }
        const int detector = clover * kNCrystalPerClover + crystal;
        const double rawEnergy = rawEnergies[i];
        const double rawTime = rawTimes[i];
        if (rawEnergy <= 0.0 || rawTime <= 0.0 ||
            !std::isfinite(rawEnergy) || !std::isfinite(rawTime)) {
          continue;
        }

        const CrystalCalibration &crystalCal =
            runCalibration->crystal[detector];
        if (!crystalCal.hasEnergyOffset || !crystalCal.hasEnergyGain ||
            !HasTimeCalibration(crystalCal, timeMode)) {
          ++crystalCalibrationMisses;
          continue;
        }
        const double energy =
            crystalCal.energyOffset + crystalCal.energyGain * rawEnergy;
        const double time =
            ApplyTimeCalibration(rawTime, crystalCal, timeMode);
        if (energy <= 0.0 || time <= 0.0 || !std::isfinite(energy) ||
            !std::isfinite(time)) {
          continue;
        }

        CloverHit &hit = cloverHits[clover];
        hit.energyKeV += energy;
        ++hit.crystalMultiplicity;
        if (energy > hit.bestCrystalEnergyKeV) {
          hit.bestCrystalEnergyKeV = energy;
          hit.timeNs = time;
        }
      }

      std::vector<double> gammaEnergies;
      double eventTimeNs = std::numeric_limits<double>::infinity();
      for (const auto &hit : cloverHits) {
        if (hit.crystalMultiplicity <= 0 || hit.energyKeV <= 0.0 ||
            !std::isfinite(hit.timeNs)) {
          continue;
        }
        if (useBgoCsiVeto && (hit.bgo || hit.csi)) continue;
        if (hit.timeNs < tCutNs) continue;
        gammaEnergies.push_back(hit.energyKeV);
        eventTimeNs = std::min(eventTimeNs, hit.timeNs);
      }

      if (gammaEnergies.size() < 3 || !std::isfinite(eventTimeNs)) continue;
      ++eventsWithAtLeastThreeGammas;
      const double eventNeutronEnergyMeV =
          ToFNsToEnergyMeV(eventTimeNs, nfsDistanceMeter);
      if (!std::isfinite(eventNeutronEnergyMeV)) continue;

      // ---------------------------------------------------------------------
      // Double-gated 4+ -> 2+ -> 0+ cascade spectra
      // 4+ -> 2+ -> 0+ 级联双 gate 一维谱
      // ---------------------------------------------------------------------
      // For every physical cascade pair [a,b], a is the 4+->2+ gamma and b is
      // the 2+->0+ gamma. We intentionally preserve the explicit i/j/k loops
      // requested for the analysis so that repeated gamma hits are handled by
      // their event indices rather than collapsed by energy.
      // 对每个真实级联对 [a,b]：a 是 4+->2+ gamma，b 是 2+->0+ gamma。
      // 这里明确保留所要求的 i/j/k 三层循环；即使同一事件有重复能量，也按 fire
      // 下标逐个处理，不会因为能量相同而提前合并。
      bool eventMatchedAnyCascade = false;
      for (auto &cascade : cascadePairs) {
        const SelectedTransition &fourToTwo =
            ags.transitions[cascade.fourToTwoTransitionIndex];
        const SelectedTransition &twoToZero =
            ags.transitions[cascade.twoToZeroTransitionIndex];
        bool cascadeMatchedInEvent = false;

        // Loop i: find every fire compatible with a, the 4+->2+ transition.
        // i 循环：逐个寻找与第一条 a（4+->2+）相符的 fire。
        for (std::size_t i = 0; i < gammaEnergies.size(); ++i) {
          if (!MatchesGate(gammaEnergies[i], fourToTwo.gammaEnergyKeV,
                           halfGateWidthKeV)) {
            continue;
          }

          // Loop j: find b, but never reuse the same physical fire selected
          // by i. This remains important even when a and b have equal energy.
          // j 循环：寻找第二条 b（2+->0+），并强制 j != i，绝不把同一个
          // physical fire 同时当作两条级联 gamma；即使 a、b 能量相同也如此。
          for (std::size_t j = 0; j < gammaEnergies.size(); ++j) {
            if (j == i) continue;
            if (!MatchesGate(gammaEnergies[j], twoToZero.gammaEnergyKeV,
                             halfGateWidthKeV)) {
              continue;
            }

            // Every matched (i,j) is an independent cascade-gate instance.
            // 同一事件若存在多个有效 (i,j)，每一组都独立贡献，避免漏计多重匹配。
            cascadeMatchedInEvent = true;
            eventMatchedAnyCascade = true;
            ++cascade.matchedGatePairs;
            ++totalCascadeMatchedGatePairs;

            // Loop k: fill all remaining gamma fires after excluding exactly
            // the two gate hits i and j. For [a,b,c], this fills c once into
            // the spectrum named a-b. Additional residual fires are each
            // filled once for this particular (i,j) match.
            // k 循环：只排除两个 gate fire（i 和 j），其余 gamma 逐个填入 a-b
            // 一维谱。例如 [a,b,c] 命中后填入 c；若还有 d、e，则对当前这组
            // (i,j) 分别填入 c、d、e 各一次。
            for (std::size_t k = 0; k < gammaEnergies.size(); ++k) {
              if (k == i || k == j) continue;
              if (cascade.spectrum) {
                cascade.spectrum->Fill(gammaEnergies[k]);
              }
              ++cascade.residualGammaFills;
              ++totalCascadeResidualGammaFills;
            }
          }
        }

        // Count an event only once for this cascade, regardless of how many
        // (i,j) combinations it contained.
        // 对当前级联，每个 event 只计数一次，不受其中有效 (i,j) 组数影响。
        if (cascadeMatchedInEvent) ++cascade.matchedEvents;
      }
      if (eventMatchedAnyCascade) ++eventsMatchingAnyCascade;

      bool eventMatchedAnyGate = false;
      bool eventAccepted = false;
      std::vector<bool> gateMatchedInEvent(gateCount, false);
      std::vector<bool> windowAcceptedInEvent(windows.size(), false);

      // Every event is tested against every AGS 2+ gate. If two different
      // gamma hits are gates, each is removed in turn and therefore produces
      // its own residual-pair set, as required for [a,b,c] -> bc/cb + ac/ca.
      // 每个 event 逐一测试全部 2+ gate；若 a、b 都是 gate，就依次去掉 a、b，
      // 因而 [a,b,c] 精确得到 bc/cb 与 ac/ca。
      for (std::size_t gate = 0; gate < gateCount; ++gate) {
        const SelectedTransition &transition =
            ags.transitions[ags.gateTransitionIndices[gate]];

        for (std::size_t gateHit = 0; gateHit < gammaEnergies.size();
             ++gateHit) {
          const double gamma = gammaEnergies[gateHit];
          if (!MatchesGate(gamma, transition.gammaEnergyKeV,
                           halfGateWidthKeV)) continue;
          eventMatchedAnyGate = true;
          gateMatchedInEvent[gate] = true;
          ++gateHitInstances[gate];

          // gammaEnergies.size() >= 3 guarantees at least two residual hits.
          ++gateQualifiedInstances[gate];
          ++totalQualifiedGateInstances;
          eventAccepted = true;

          Long64_t pairsBefore = totalUnorderedPairs;
          Long64_t fillsBefore = totalSymmetricFills;
          FillResidualPairs(&totalMatrix, gammaEnergies, gateHit,
                            totalUnorderedPairs, totalSymmetricFills);
          gateUnorderedPairs[gate] += totalUnorderedPairs - pairsBefore;
          gateSymmetricFills[gate] += totalSymmetricFills - fillsBefore;

          for (std::size_t windowIndex = 0; windowIndex < windows.size();
               ++windowIndex) {
            NeutronWindow &window = windows[windowIndex];
            if (!InWindow(eventNeutronEnergyMeV, window)) continue;
            windowAcceptedInEvent[windowIndex] = true;
            ++window.qualifiedGateInstances;
            FillResidualPairs(window.matrix, gammaEnergies, gateHit,
                              window.unorderedResidualPairs,
                              window.symmetricFills);
          }
        }
      }

      if (eventMatchedAnyGate) ++eventsMatchingAnyGate;
      for (std::size_t gate = 0; gate < gateCount; ++gate) {
        if (gateMatchedInEvent[gate]) ++gateMatchedEvents[gate];
      }
      if (eventAccepted) {
        ++acceptedEvents;
        neutronEnergy.Fill(eventNeutronEnergyMeV);
      }
      for (std::size_t i = 0; i < windows.size(); ++i) {
        if (windowAcceptedInEvent[i]) ++windows[i].acceptedEvents;
      }
    }
    input.Close();
  }

  TFile output(outputFile, "RECREATE");
  if (!output.IsOpen() || output.IsZombie()) {
    std::cerr << "Cannot create output ROOT / 无法创建输出 ROOT: "
              << outputFile << std::endl;
    return;
  }

  TDirectory *metadataDirectory = output.mkdir("metadata");
  TDirectory *matrixDirectory = output.mkdir("gamma_gamma_matrices");
  TDirectory *cascadeDirectory = output.mkdir("cascade_gated_spectra");
  TDirectory *diagnosticDirectory = output.mkdir("diagnostics");

  metadataDirectory->cd();
  const TString configTitle = TString::Format(
          "inputs=%s; calibration=%s; ags=%s; neutron_windows_MeV=%s; gate_width_1keV_channels=%.9g; gate_half_width_keV=%.9g; distance_m=%.9g; time_fwhm_ns=%.9g; t_cut_ns=%.9g; bgo_csi_veto=%d; gamma_bins=%d; gamma_range_keV=%.9g:%.9g; time_mode=%s; time_branch=%s",
          inputFiles, calibrationSummary, agsFile, neutronWindowsMeV,
          gateWidthKeV, halfGateWidthKeV, nfsDistanceMeter, timeFwhmNs, tCutNs,
          useBgoCsiVeto ? 1 : 0, gammaBins, gammaMinKeV, gammaMaxKeV,
          timeCorrectionMode, timeBranchLeaf);
  TNamed config("AGS2GateAnalysisConfig", configTitle.Data());
  config.Write();
  WriteTransitionTree(ags);
  WriteCascadePairTree(cascadePairs, ags, halfGateWidthKeV);

  TTree gateStatistics("GateStatistics",
                       "Per-AGS-2+ gate matching and residual-pair statistics");
  Int_t gateIndex = -1;
  Int_t transitionIndex = -1;
  Int_t gammaIndex = -1;
  std::string nuclide;
  std::string bandName;
  Double_t gateEnergyKeV = 0.0;
  Double_t gateLowKeV = 0.0;
  Double_t gateHighKeV = 0.0;
  Long64_t matchedEvents = 0;
  Long64_t hitInstances = 0;
  Long64_t qualifiedInstances = 0;
  Long64_t unorderedPairs = 0;
  Long64_t symmetricFills = 0;
  gateStatistics.Branch("gate_index", &gateIndex);
  gateStatistics.Branch("transition_index", &transitionIndex);
  gateStatistics.Branch("gamma_index", &gammaIndex);
  gateStatistics.Branch("nuclide", &nuclide);
  gateStatistics.Branch("band_name", &bandName);
  gateStatistics.Branch("gate_energy_keV", &gateEnergyKeV);
  gateStatistics.Branch("gate_low_keV", &gateLowKeV);
  gateStatistics.Branch("gate_high_keV", &gateHighKeV);
  gateStatistics.Branch("matched_events", &matchedEvents);
  gateStatistics.Branch("hit_instances", &hitInstances);
  gateStatistics.Branch("qualified_instances", &qualifiedInstances);
  gateStatistics.Branch("unordered_residual_pairs", &unorderedPairs);
  gateStatistics.Branch("symmetric_matrix_fills", &symmetricFills);
  for (std::size_t gate = 0; gate < gateCount; ++gate) {
    const SelectedTransition &transition =
        ags.transitions[ags.gateTransitionIndices[gate]];
    gateIndex = static_cast<Int_t>(gate);
    transitionIndex = transition.transitionIndex;
    gammaIndex = transition.gammaIndex;
    nuclide = transition.nuclide;
    bandName = transition.initialBandName;
    gateEnergyKeV = transition.gammaEnergyKeV;
    gateLowKeV = gateEnergyKeV - halfGateWidthKeV;
    gateHighKeV = gateEnergyKeV + halfGateWidthKeV;
    matchedEvents = gateMatchedEvents[gate];
    hitInstances = gateHitInstances[gate];
    qualifiedInstances = gateQualifiedInstances[gate];
    unorderedPairs = gateUnorderedPairs[gate];
    symmetricFills = gateSymmetricFills[gate];
    gateStatistics.Fill();
  }
  gateStatistics.Write();

  TTree windowStatistics(
      "NeutronWindowStatistics",
      "Event and matrix-fill statistics for requested neutron-energy windows");
  std::string windowName;
  Double_t lowMeV = 0.0;
  Double_t highMeV = 0.0;
  Long64_t windowEvents = 0;
  Long64_t windowGateInstances = 0;
  Long64_t windowPairs = 0;
  Long64_t windowFills = 0;
  windowStatistics.Branch("window", &windowName);
  windowStatistics.Branch("low_MeV", &lowMeV);
  windowStatistics.Branch("high_MeV", &highMeV);
  windowStatistics.Branch("accepted_events", &windowEvents);
  windowStatistics.Branch("qualified_gate_instances", &windowGateInstances);
  windowStatistics.Branch("unordered_residual_pairs", &windowPairs);
  windowStatistics.Branch("symmetric_matrix_fills", &windowFills);
  for (const auto &window : windows) {
    windowName = window.tag.Data();
    lowMeV = window.lowMeV;
    highMeV = window.highMeV;
    windowEvents = window.acceptedEvents;
    windowGateInstances = window.qualifiedGateInstances;
    windowPairs = window.unorderedResidualPairs;
    windowFills = window.symmetricFills;
    windowStatistics.Fill();
  }
  windowStatistics.Write();

  TH1D gateEnergies(
      "AGS2PlusGateEnergies",
      "AGS 2+ -> 0+ gamma gates;Gate index;Gamma energy (keV)",
      static_cast<int>(gateCount), -0.5, static_cast<double>(gateCount) - 0.5);
  TH1I gateEventCounts(
      "AGS2PlusGateMatchedEvents",
      "Events matching each AGS 2+ gate;Gate index;Events",
      static_cast<int>(gateCount), -0.5, static_cast<double>(gateCount) - 0.5);
  for (std::size_t gate = 0; gate < gateCount; ++gate) {
    const SelectedTransition &transition =
        ags.transitions[ags.gateTransitionIndices[gate]];
    gateEnergies.SetBinContent(static_cast<int>(gate) + 1,
                               transition.gammaEnergyKeV);
    gateEventCounts.SetBinContent(static_cast<int>(gate) + 1,
                                  gateMatchedEvents[gate]);
    const TString label = TString::Format("%s %.3f",
                                          transition.nuclide.c_str(),
                                          transition.gammaEnergyKeV);
    gateEnergies.GetXaxis()->SetBinLabel(static_cast<int>(gate) + 1, label);
    gateEventCounts.GetXaxis()->SetBinLabel(static_cast<int>(gate) + 1, label);
  }
  gateEnergies.Write();
  gateEventCounts.Write();

  TH1D summary("AGS2GateAnalysisSummary",
               "AGS2GateAnalysisSummary;Category;Counts", 16, 0.5, 16.5);
  const char *summaryLabels[16] = {
      "Input entries", "Processed files", "Files no calibration",
      "Malformed entries", "Crystal calibration misses",
      "Events mult>=3", "Events matching gate", "Accepted events",
      "2+ gates", "4+->2+ transitions", "Gate instances",
      "Symmetric matrix fills", "Cascade pairs", "Events matching cascade",
      "Cascade gate pairs", "Cascade residual fills"};
  for (int i = 0; i < 16; ++i) {
    summary.GetXaxis()->SetBinLabel(i + 1, summaryLabels[i]);
  }
  Long64_t fourToTwoCount = 0;
  for (const auto &transition : ags.transitions) {
    if (transition.kind == "4plus_to_2plus") ++fourToTwoCount;
  }
  summary.SetBinContent(1, totalEntries);
  summary.SetBinContent(2, processedFiles);
  summary.SetBinContent(3, filesWithoutCalibration);
  summary.SetBinContent(4, malformedEntries);
  summary.SetBinContent(5, crystalCalibrationMisses);
  summary.SetBinContent(6, eventsWithAtLeastThreeGammas);
  summary.SetBinContent(7, eventsMatchingAnyGate);
  summary.SetBinContent(8, acceptedEvents);
  summary.SetBinContent(9, gateCount);
  summary.SetBinContent(10, fourToTwoCount);
  summary.SetBinContent(11, totalQualifiedGateInstances);
  summary.SetBinContent(12, totalSymmetricFills);
  summary.SetBinContent(13, cascadePairs.size());
  summary.SetBinContent(14, eventsMatchingAnyCascade);
  summary.SetBinContent(15, totalCascadeMatchedGatePairs);
  summary.SetBinContent(16, totalCascadeResidualGammaFills);
  summary.Write();

  matrixDirectory->cd();
  totalMatrix.Write();
  for (auto &window : windows) window.matrix->Write();

  cascadeDirectory->cd();
  for (auto &cascade : cascadePairs) {
    if (cascade.spectrum) cascade.spectrum->Write();
  }

  diagnosticDirectory->cd();
  neutronEnergy.Write();
  output.Close();

  std::cout << "AGS levels parsed / AGS level: " << ags.levelCount << std::endl;
  std::cout << "AGS bands parsed / AGS band: " << ags.bandCount << std::endl;
  std::cout << "AGS gammas parsed / AGS gamma: " << ags.gammaCount << std::endl;
  std::cout << "2+ -> 0+ gates: " << gateCount << std::endl;
  std::cout << "4+ -> 2+ transitions: " << fourToTwoCount << std::endl;
  std::cout << "4+ -> 2+ -> 0+ cascade pairs: " << cascadePairs.size() << std::endl;
  std::cout << "Processed files / 处理文件: " << processedFiles << std::endl;
  std::cout << "Input entries / 输入 event: " << totalEntries << std::endl;
  std::cout << "Events matching any gate / 命中任一 gate: "
            << eventsMatchingAnyGate << std::endl;
  std::cout << "Accepted events / 通过 event: " << acceptedEvents << std::endl;
  std::cout << "Qualified gate instances / 有效 gate 实例: "
            << totalQualifiedGateInstances << std::endl;
  std::cout << "Symmetric matrix fills / 对称填充数: "
            << totalSymmetricFills << std::endl;
  std::cout << "Events matching any cascade / 命中任一级联的 event: "
            << eventsMatchingAnyCascade << std::endl;
  std::cout << "Matched cascade gate pairs / 级联双 gate 组合数: "
            << totalCascadeMatchedGatePairs << std::endl;
  std::cout << "Cascade residual gamma fills / 级联剩余 gamma 填充数: "
            << totalCascadeResidualGammaFills << std::endl;
  for (const auto &window : windows) {
    std::cout << "  " << window.title << ": events=" << window.acceptedEvents
              << " gate_instances=" << window.qualifiedGateInstances
              << " symmetric_fills=" << window.symmetricFills << std::endl;
  }
  std::cout << "Output / 输出: " << outputFile << std::endl;
}
