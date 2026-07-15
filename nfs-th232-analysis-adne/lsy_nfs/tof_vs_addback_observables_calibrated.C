// Calibrated event-TOF correlations with clover addback observables.
// 逐 run、逐 crystal 刻度后的 event TOF 与 clover addback 观测量关联图。
//
// Output histograms / 输出直方图
// -----------------------------------------------------------------------------
// 1. TOFVsAddbackGammaMultiplicity
//    X: number of accepted clover addback gamma fires in one event.
//    Y: calibrated event TOF in ns.
// 2. TOFVsAddbackGammaTotalEnergy
//    X: sum of all accepted clover addback gamma energies in one event, in keV.
//    Y: calibrated event TOF in ns.
//
// 1. TOFVsAddbackGammaMultiplicity
//    X：一个 event 中通过选择的 clover addback gamma 数量。
//    Y：刻度后的 event TOF，单位 ns。
// 2. TOFVsAddbackGammaTotalEnergy
//    X：一个 event 中所有通过选择的 clover addback gamma 能量之和，单位 keV。
//    Y：刻度后的 event TOF，单位 ns。
//
// Event TOF definition / Event TOF 定义
// -----------------------------------------------------------------------------
// Each clover time is the calibrated time of its highest-energy crystal. Event
// TOF is the minimum time among the accepted addback clovers. Therefore each
// accepted event contributes exactly one point to each two-dimensional plot.
// 每个 clover 的时间取其“刻度后能量最高的 crystal”的刻度时间；event TOF 再取
// 所有通过选择的 addback clover 时间中的最小值。因此，每个通过的 event 在两张
// 二维图中分别只填一个点。
//
// Selection / 选择条件
// -----------------------------------------------------------------------------
// By default this macro applies the same BGO/CSI veto and 50-MeV fast-neutron
// time cut used by fission_event_ana_calibrated.C. Both choices are explicit
// arguments. Set useBgoCsiVeto=false or applyFastTimeCut=false to disable them.
// 默认采用 fission_event_ana_calibrated.C 相同的 BGO/CSI veto 和 50 MeV 快中子
// 时间 cut；两项均可通过参数关闭。
//
// Input supports files, recursively scanned directories, CSV mixtures, or an
// @path-list whose lines may themselves be files/directories.
// 输入支持文件、递归目录、CSV 文件/目录混合项，或每行可为文件/目录的 @路径列表。
//
// Example / 示例
// -----------------------------------------------------------------------------
// root -l -b -q 'lsy_nfs/tof_vs_addback_observables_calibrated.C(
//   "@mult3_files.txt","calibration_summary.tsv","tof_addback.root",
//   true,true,24.0,20.0,-1,1600,0,1600,4096,0,32768,"offset","fTime")'

// =============================================================================
// Complete positional interface / 完整位置参数接口
// =============================================================================
// ROOT calls this function with positional arguments:
// ROOT 使用位置参数调用下列函数：
//
// tof_vs_addback_observables_calibrated(
//   [ 1] inputFiles = "out/mult3_nfs_run_100_r0.root",
//   [ 2] calibrationSummary = "calibration_summary.tsv",
//   [ 3] outputFile = "tof_vs_addback_observables_calibrated.root",
//   [ 4] useBgoCsiVeto = true,
//   [ 5] applyFastTimeCut = true,
//   [ 6] nfsDistanceMeter = 24.0,
//   [ 7] timeFwhmNs = 20.0,
//   [ 8] maxEntries = -1,
//   [ 9] tofBins = 1600,
//   [10] tofMinNs = 0.0,
//   [11] tofMaxNs = 1600.0,
//   [12] totalEnergyBins = 4096,
//   [13] totalEnergyMinKeV = 0.0,
//   [14] totalEnergyMaxKeV = 32768.0,
//   [15] timeCorrectionMode = "offset",
//   [16] timeBranchLeaf = "fTime")
//
// IMPORTANT / 重要：
// This ROOT/C++ macro has positional, not named, arguments. To change argument
// [12], arguments [1] through [11] must also be supplied. Only a continuous set
// of trailing arguments may be omitted and replaced by defaults.
// 该 ROOT/C++ 宏使用位置参数而不是命名参数。若修改第 [12] 项，必须先写出
// [1] 到 [11]；只有末尾连续的参数可以省略并采用默认值。
//
// Parameter details / 参数详解
// -----------------------------------------------------------------------------
// [1] inputFiles : const char*
//     File and directory specifications containing TreeMaster. Accepted forms:
//       a) one file:      "/data/run12/nfs_run_12_r0.root"
//       b) one directory: "/data/run12"
//       c) CSV mixture:   "/data/run12,/data/run13/nfs_run_13_r0.root"
//       d) path list:     "@input_paths.txt"
//     Every non-comment @list line may be either a file or directory. Empty
//     lines and lines starting with # or // are ignored. Relative entries are
//     tried from ROOT's current directory, then from the list-file directory.
//
//     Recursive discovery accepts nfs_run_[1-3 digits]_r[1-3 digits].root and
//     the same name with a mult3_ prefix. The underscore after "run" is optional.
//     Explicit files keep backward-compatible arbitrary names. Discovered files
//     are sorted numerically by run and part.
//
//     输入支持单文件、单目录、CSV 文件/目录混合项，以及 @路径列表；列表每行均
//     可为文件或目录。目录递归匹配 nfs_run_<1-3位>_r<1-3位>.root 及其
//     mult3_ 版本。相对路径先按 ROOT 当前目录查找，不存在时再相对列表目录解析。
//
// [2] calibrationSummary : const char*
//     calibration_summary.tsv produced by calibrate_nfs_crystals.sh, or one
//     detailed calibration txt beginning with "# clover ...". Matching is done
//     per input basename after removing the "mult3_" prefix:
//       mult3_nfs_run_12_r0.root -> nfs_run_12_r0.root.
//     Duplicate copies with the same normalized basename are processed once. If
//     both full and mult3 trees are found, the full nfs_run file is preferred.
//     A file without matching calibration is skipped; identity calibration is
//     NOT silently substituted. Each used crystal needs energy and time values:
//       E_cal = energy_offset + energy_gain * E_raw.
//     刻度汇总由 calibrate_nfs_crystals.sh 生成。输入文件按 basename 逐 run 匹配，
//     并去掉 mult3_ 前缀。同名重复副本只处理一次；普通树与 mult3 子集同时存在时
//     优先普通树。无对应刻度的文件跳过。能量按 E_cal=offset+gain*E_raw 计算。
//
// [3] outputFile : const char*
//     Output ROOT path. The file is opened with RECREATE: an existing file with
//     the same path is overwritten. Its parent directory must already exist.
//     输出 ROOT 路径。采用 RECREATE，同名文件会覆盖；父目录必须预先存在。
//
// [4] useBgoCsiVeto : bool, default true
//     true: remove an entire clover addback hit when its ESS record has BGO>0 or
//     CSI>0. The ESS branches are then required.
//     false: ignore BGO/CSI and do not require ESS branches.
//     true 时，只要该 clover 的 ESS 记录中 BGO>0 或 CSI>0，就移除整个 clover
//     addback fire；此时 ESS 分支为必需。false 时完全不使用 BGO/CSI。
//
// [5] applyFastTimeCut : bool, default true
//     true applies the 50-MeV lower-TOF rejection described by [6] and [7].
//     false keeps clovers regardless of this time cut. Other validity checks and
//     optional BGO/CSI veto still apply.
//     true 时应用由 [6][7] 定义的 50 MeV 快时间 cut；false 时关闭这一项，但无效
//     数值清理和可选 BGO/CSI veto 仍然执行。
//
// [6] nfsDistanceMeter : double, default 24.0 m
//     Neutron source-to-target flight distance used to calculate TOF(50 MeV).
//     Use the measured experiment value, for example 23.396 m, when available.
//     中子源到靶的飞行距离，单位 m。已知时应使用实验实测值，例如 23.396 m。
//
// [7] timeFwhmNs : double, default 20.0 ns
//     Time-resolution FWHM used in the optional fast-time cut:
//       sigma_t = timeFwhmNs / 2.355
//       t_min   = TOF(50 MeV, nfsDistanceMeter)
//       t_cut   = t_min - 3*sigma_t
//     When [5] is true, a clover with corrected time < t_cut is removed before
//     event multiplicity, total energy, and event TOF are calculated.
//     时间分辨 FWHM，单位 ns。[5] 为 true 时，修正时间小于 t_cut 的 clover 会在
//     计算多重度、总能量和 event TOF 之前被移除。
//
// [8] maxEntries : Long64_t, default -1
//     >0 processes at most this many TreeMaster entries across ALL input files.
//     <=0 processes every entry. It is a global total, not a per-file limit.
//     正数表示全部输入文件合计最多读取多少 event；<=0 表示全部处理。它不是每个
//     文件分别限额。调试可使用 1000 或 100000。
//
// [9]  tofBins : int, default 1600
// [10] tofMinNs : double, default 0 ns
// [11] tofMaxNs : double, default 1600 ns
//     Y-axis binning and visible range of BOTH two-dimensional histograms.
//     Accepted events outside this visible range are still passed to TH2::Fill
//     and accumulated in ROOT underflow/overflow bins; the summary counts them.
//     两张二维图共同的 Y 轴（event TOF）分 bin 和显示范围。范围外的通过事件仍
//     进入 ROOT underflow/overflow，同时在 summary 中单独计数。
//
// [12] totalEnergyBins : int, default 4096
// [13] totalEnergyMinKeV : double, default 0 keV
// [14] totalEnergyMaxKeV : double, default 32768 keV
//     X-axis binning of TOFVsAddbackGammaTotalEnergy. The default channel width
//     is (32768-0)/4096 = 8 keV. Values outside the range go to under/overflow.
//     TOFVsAddbackGammaTotalEnergy 的 X 轴设置。默认每 bin 为 8 keV；范围外
//     的 event 总能量进入 underflow/overflow。
//     Approximate TH2F bin memory at defaults is
//       totalEnergyBins * tofBins * 4 bytes ~= 25 MiB,
//     plus a much smaller 16*tofBins multiplicity histogram and ROOT overhead.
//     默认总能量二维图约占 25 MiB bin 内存，另有很小的多重度图及 ROOT 开销。
//
// [15] timeCorrectionMode : const char*, default "offset"
//     Defines how per-crystal time coefficients are applied:
//       "offset"      : t_corr = t_raw + time_offset
//       "gain"        : t_corr = t_raw * time_gain
//       "offset_gain" : t_corr = (t_raw + time_offset) * time_gain
//       "gain_offset" : t_corr = t_raw * time_gain + time_offset
//     Unknown strings fall back to the "offset" formula. Use the mode matching
//     how calibration_summary.tsv was constructed.
//     定义逐 crystal 时间系数的应用顺序。未知字符串会按 offset 处理；必须与刻度
//     汇总文件中时间参数的定义保持一致。
//
// [16] timeBranchLeaf : const char*, default "fTime"
//     Crystal-level time branch to calibrate. The branch resolver accepts the
//     exact leaf and split names ending in that leaf, for example Exogam2.fTime.
//     要刻度的逐 crystal 时间分支叶名；也兼容 Exogam2.fTime 等 split 分支名。
//
// Required TreeMaster branches / 必需的 TreeMaster 分支
// -----------------------------------------------------------------------------
// Always required:
//   fEXO_ECC_E_Clover, fEXO_ECC_E_Cristal, fEXO_ECC_E_Energy,
//   and the branch selected by timeBranchLeaf.
// Required only when useBgoCsiVeto=true:
//   fEXO_ESS_Clover, fEXO_ESS_Cristal, fEXO_ESS_BGO, fEXO_ESS_CSI.
// 始终需要 ECC 的 clover、crystal、energy 和所选时间分支；只有启用 veto 时才
// 强制要求四个 ESS 分支。
//
// Event reconstruction and fill order / 事件重建与填图顺序
// -----------------------------------------------------------------------------
// 1. Reject crystal rows with invalid indices, non-positive/non-finite raw energy,
//    or non-positive/non-finite raw time.
// 2. Apply the matched per-crystal energy and time calibration. Invalid corrected
//    values and crystals missing either calibration are removed.
// 3. Add all accepted crystal energies from the same clover (addback).
// 4. Assign each clover the corrected time of its highest calibrated-energy
//    crystal; this is the same addback-time convention as the calibrated fission
//    analysis.
// 5. Apply the optional BGO/CSI veto and optional fast-time cut per clover.
// 6. Require at least ONE surviving clover. There is no additional mult>=2 or
//    mult>=3 requirement in this macro.
// 7. For one accepted event:
//      multiplicity = number of surviving addback clovers;
//      total energy = sum of their calibrated addback energies;
//      event TOF    = minimum of their corrected clover times.
// 8. Fill each TH2 exactly once with TH2::Fill(X,Y):
//      (multiplicity, event TOF) and (total energy, event TOF).
//
// 顺序为：清理无效 crystal -> 逐 crystal 能量/时间刻度 -> 同 clover 能量 addback
// -> 最高刻度能量 crystal 决定 clover 时间 -> 可选 veto -> 可选快时间 cut。
// 至少保留一个 clover 即接受；本宏没有额外 mult>=2 或 mult>=3 判断。随后每个
// event 在两张 TH2 中各填一次，X 分别为 addback 多重度和总能量，Y 均为最小
// clover 时间定义的 event TOF。
//
// Output ROOT layout / 输出 ROOT 结构
// -----------------------------------------------------------------------------
// tof_addback_correlations/
//   TOFVsAddbackGammaMultiplicity  : TH2F, X=multiplicity, Y=event TOF (ns)
//   TOFVsAddbackGammaTotalEnergy   : TH2F, X=total energy (keV), Y=event TOF (ns)
//   TOFVsAddbackSummary            : TH1D diagnostic counters
//   TOFVsAddbackConfig             : TNamed containing the exact run settings
// 所有对象写在 tof_addback_correlations 目录。Summary 记录输入、跳过、veto、
// 快时间 cut、通过 event 和显示范围外 event 数；Config 保存本次调用参数和 t_cut。
//
// Practical calls / 常用调用示例
// -----------------------------------------------------------------------------
// A. Explicit paths and all trailing defaults / 显式路径，其余使用默认值：
// root -l -b -q 'lsy_nfs/tof_vs_addback_observables_calibrated.C(
//   "@mult3_files.txt","calibration_summary.tsv","tof_addback.root")'
//
// B. Use the measured 23.396 m distance / 使用 23.396 m 实际距离：
// root -l -b -q 'lsy_nfs/tof_vs_addback_observables_calibrated.C(
//   "@mult3_files.txt","calibration_summary.tsv","tof_addback.root",
//   true,true,23.396)'
//
// C. Disable both BGO/CSI veto and fast-time cut / 同时关闭 veto 和快时间 cut：
// root -l -b -q 'lsy_nfs/tof_vs_addback_observables_calibrated.C(
//   "a.root,b.root","calibration_summary.tsv","tof_no_cuts.root",
//   false,false)'
//
// D. Quick 100000-event check with custom histogram ranges /
//    处理 10 万 event，并自定义直方图范围：
// root -l -b -q 'lsy_nfs/tof_vs_addback_observables_calibrated.C(
//   "@mult3_files.txt","calibration_summary.tsv","tof_quick.root",
//   true,true,23.396,20,100000,2000,0,2000,2048,0,16384,
//   "offset","fTime")'
//
// E. Recursively scan one directory / 递归遍历一个目录：
// root -l -b -q 'lsy_nfs/tof_vs_addback_observables_calibrated.C(
//   "/data/analysed/run12","calibration_summary.tsv","tof_run12.root")'
//
// F. Read a path list containing directories and files / 输入路径列表：
// input_paths.txt may contain / input_paths.txt 每行可以写：
//   /data/analysed/run12
//   /data/analysed/run13
//   /data/analysed/special/nfs_run_14_r0.root
// root -l -b -q 'lsy_nfs/tof_vs_addback_observables_calibrated.C(
//   "@/abs/path/input_paths.txt","calibration_summary.tsv",
//   "tof_many_runs.root")'
//
// Shell quoting / Shell 引号：the entire ROOT expression is enclosed by single
// quotes; C++ string arguments inside it remain enclosed by double quotes.
// 整个 ROOT 表达式用外层单引号保护，内部 C++ 字符串仍使用双引号。
// =============================================================================

#include <algorithm>
#include <climits>
#include <cctype>
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <sys/stat.h>
#include <vector>

// Reuse the established per-run calibration parser, branch resolver, physical
// constants, and clover data structure. This keeps the calibration convention
// identical to the existing calibrated fission analysis.
// 复用现有刻度版裂变分析的逐 run 刻度解析、分支解析、物理常数和 clover 结构，
// 保证两份代码的能量、时间和 addback 定义完全一致。
#include "fission_event_ana_calibrated.C"

namespace tof_addback_input_detail {

struct NfsRunFileIdentity {
  bool valid = false;
  bool mult3 = false;
  int runNumber = -1;
  int partNumber = -1;
  TString normalizedName;
};

TString BaseName(const TString &path)
{
  const Ssiz_t slash = path.Last('/');
  if (slash == kNPOS) return path;
  return path(slash + 1, path.Length() - slash - 1);
}

bool ParseOneToThreeDigits(const std::string &text, std::size_t &position,
                           int &value)
{
  const std::size_t begin = position;
  while (position < text.size() &&
         std::isdigit(static_cast<unsigned char>(text[position]))) {
    ++position;
  }
  const std::size_t count = position - begin;
  if (count < 1 || count > 3) return false;
  value = std::atoi(text.substr(begin, count).c_str());
  return true;
}

// Directory discovery accepts nfs_run_8_r0.root, nfs_run_100_r12.root, and
// their mult3_nfs_run_... equivalents. Both numeric fields must contain one to
// three digits. The underscore after "run" is optional for compatibility.
// 目录遍历接受普通 nfs_run 和 mult3_nfs_run 文件；run 与 part 编号均必须是
// 1 到 3 位数字，run 后下划线可省略。
NfsRunFileIdentity ParseNfsRunFileIdentity(const TString &path)
{
  NfsRunFileIdentity identity;
  std::string name = BaseName(path).Data();
  const std::string mult3Prefix = "mult3_";
  if (name.compare(0, mult3Prefix.size(), mult3Prefix) == 0) {
    identity.mult3 = true;
    name.erase(0, mult3Prefix.size());
  }
  const std::string prefix = "nfs_run";
  if (name.compare(0, prefix.size(), prefix) != 0) return identity;

  std::size_t position = prefix.size();
  if (position < name.size() && name[position] == '_') ++position;
  if (!ParseOneToThreeDigits(name, position, identity.runNumber)) {
    return NfsRunFileIdentity();
  }
  if (name.compare(position, 2, "_r") != 0) return NfsRunFileIdentity();
  position += 2;
  if (!ParseOneToThreeDigits(name, position, identity.partNumber)) {
    return NfsRunFileIdentity();
  }
  if (name.compare(position, 5, ".root") != 0 ||
      position + 5 != name.size()) {
    return NfsRunFileIdentity();
  }

  identity.valid = true;
  identity.normalizedName = name.c_str();
  return identity;
}

TString CanonicalInputPath(TString path)
{
  path = path.Strip(TString::kBoth);
  if (path.IsNull() || path.Contains("://")) return path;
  gSystem->ExpandPathName(path);
  char resolved[PATH_MAX];
  if (::realpath(path.Data(), resolved)) return TString(resolved);
  return path;
}

bool ReadPathStatus(const TString &path, struct stat &status)
{
  if (path.Contains("://")) return false;
  return ::stat(path.Data(), &status) == 0;
}

// Each non-comment @list line may itself be a ROOT file or a directory. Keep
// the former current-directory behavior first; if it does not exist there,
// resolve the relative path from the list file's directory.
// @list 中每个非注释行都可为 ROOT 文件或目录。先按当前目录解释旧列表；不存在
// 时，再相对 list 文件所在目录解析。
std::vector<TString> ReadInputList(const TString &listPath)
{
  std::vector<TString> specifications;
  std::ifstream input(listPath.Data());
  if (!input.is_open()) {
    std::cerr << "Cannot open input list / 无法打开输入列表: " << listPath
              << std::endl;
    return specifications;
  }

  const TString listDirectory = gSystem->DirName(listPath);
  std::string line;
  while (std::getline(input, line)) {
    line = Trim(line);
    if (line.empty() || line[0] == '#' || line.rfind("//", 0) == 0) continue;
    TString path(line.c_str());
    gSystem->ExpandPathName(path);
    if (gSystem->AccessPathName(path, kReadPermission) &&
        !gSystem->IsAbsoluteFileName(path)) {
      TString relative = listDirectory;
      if (!relative.EndsWith("/")) relative += "/";
      relative += path;
      gSystem->ExpandPathName(relative);
      path = relative;
    }
    specifications.push_back(path);
  }
  return specifications;
}

// Recursive discovery follows directory symlinks, while canonical paths and the
// visited set prevent a symlink cycle from being traversed repeatedly.
// 递归时允许目录软链接；规范化路径和 visited 集合用于阻止软链接循环。
void CollectNfsRunFiles(const TString &directory,
                        std::vector<TString> &candidates,
                        std::set<std::string> &visitedDirectories)
{
  const TString canonicalDirectory = CanonicalInputPath(directory);
  struct stat directoryStatus;
  if (!ReadPathStatus(canonicalDirectory, directoryStatus) ||
      !S_ISDIR(directoryStatus.st_mode)) {
    return;
  }
  if (!visitedDirectories.insert(canonicalDirectory.Data()).second) return;

  DIR *handle = ::opendir(canonicalDirectory.Data());
  if (!handle) {
    std::cerr << "Cannot read input directory / 无法读取输入目录: "
              << canonicalDirectory << std::endl;
    return;
  }

  std::vector<std::string> entries;
  while (dirent *entry = ::readdir(handle)) {
    const std::string name(entry->d_name);
    if (name == "." || name == "..") continue;
    entries.push_back(name);
  }
  ::closedir(handle);
  std::sort(entries.begin(), entries.end());

  for (const auto &entry : entries) {
    TString child = canonicalDirectory;
    if (!child.EndsWith("/")) child += "/";
    child += entry.c_str();
    child = CanonicalInputPath(child);

    struct stat childStatus;
    if (!ReadPathStatus(child, childStatus)) continue;
    if (S_ISDIR(childStatus.st_mode)) {
      CollectNfsRunFiles(child, candidates, visitedDirectories);
      continue;
    }
    if (S_ISREG(childStatus.st_mode) &&
        ParseNfsRunFileIdentity(child).valid) {
      candidates.push_back(child);
    }
  }
}

bool NaturalInputOrder(const TString &left, const TString &right)
{
  const NfsRunFileIdentity leftId = ParseNfsRunFileIdentity(left);
  const NfsRunFileIdentity rightId = ParseNfsRunFileIdentity(right);
  if (leftId.valid && rightId.valid) {
    if (leftId.runNumber != rightId.runNumber) {
      return leftId.runNumber < rightId.runNumber;
    }
    if (leftId.partNumber != rightId.partNumber) {
      return leftId.partNumber < rightId.partNumber;
    }
    if (leftId.normalizedName != rightId.normalizedName) {
      return leftId.normalizedName.CompareTo(rightId.normalizedName) < 0;
    }
    if (leftId.mult3 != rightId.mult3) return !leftId.mult3;
  } else if (leftId.valid != rightId.valid) {
    return leftId.valid;
  }
  return left.CompareTo(right) < 0;
}

// Expand one file/directory, a comma-separated mixture, or an @list whose lines
// are files/directories. Directly named files keep backward-compatible arbitrary
// names; recursive discovery applies the strict NFS filename rule.
// 展开单文件/目录、CSV 混合项或文件/目录路径列表。直接指定的文件保持任意旧名称
// 兼容性；目录内自动发现才使用严格的 NFS 文件名规则。
std::vector<TString> ResolveInputs(const char *inputSpec)
{
  std::vector<TString> specifications;
  TString text(inputSpec ? inputSpec : "");
  text = text.Strip(TString::kBoth);
  if (text.BeginsWith("@")) {
    TString listPath = text(1, text.Length() - 1);
    listPath = CanonicalInputPath(listPath.Strip(TString::kBoth));
    specifications = ReadInputList(listPath);
  } else {
    TObjArray *tokens = text.Tokenize(",");
    for (int index = 0; index < tokens->GetEntriesFast(); ++index) {
      TString path = static_cast<TObjString *>(tokens->At(index))->GetString();
      path = path.Strip(TString::kBoth);
      if (!path.IsNull()) specifications.push_back(path);
    }
    delete tokens;
  }

  std::vector<TString> candidates;
  std::set<std::string> visitedDirectories;
  for (const auto &rawSpecification : specifications) {
    const TString path = CanonicalInputPath(rawSpecification);
    if (path.Contains("://")) {
      candidates.push_back(path);
      continue;
    }
    struct stat status;
    if (!ReadPathStatus(path, status)) {
      std::cerr << "Input path does not exist / 输入路径不存在: " << path
                << std::endl;
      continue;
    }
    if (S_ISDIR(status.st_mode)) {
      CollectNfsRunFiles(path, candidates, visitedDirectories);
    } else if (S_ISREG(status.st_mode)) {
      candidates.push_back(path);
    }
  }

  // Use the same normalized basename as calibration matching. Prefer a full
  // nfs tree over its mult3 subset when both are found, so one run is not counted
  // twice. Duplicate copies in different listed directories are also collapsed.
  // 使用与刻度匹配相同的标准文件名去重；同一 run 同时找到普通树和 mult3 子集时
  // 只保留普通树，不同路径中的重复副本也只处理一次。
  std::map<std::string, TString> selectedByRun;
  std::vector<TString> otherFiles;
  std::set<std::string> seenOtherFiles;
  Long64_t duplicateRunFiles = 0;
  for (const auto &candidate : candidates) {
    const TString path = CanonicalInputPath(candidate);
    const NfsRunFileIdentity identity = ParseNfsRunFileIdentity(path);
    if (!identity.valid) {
      if (seenOtherFiles.insert(path.Data()).second) otherFiles.push_back(path);
      continue;
    }

    const std::string key(identity.normalizedName.Data());
    auto found = selectedByRun.find(key);
    if (found == selectedByRun.end()) {
      selectedByRun[key] = path;
      continue;
    }
    ++duplicateRunFiles;
    const NfsRunFileIdentity existing =
        ParseNfsRunFileIdentity(found->second);
    const bool preferCandidate =
        (existing.mult3 && !identity.mult3) ||
        (existing.mult3 == identity.mult3 &&
         path.CompareTo(found->second) < 0);
    if (preferCandidate) found->second = path;
  }

  std::vector<TString> files;
  for (const auto &entry : selectedByRun) files.push_back(entry.second);
  files.insert(files.end(), otherFiles.begin(), otherFiles.end());
  std::sort(files.begin(), files.end(), NaturalInputOrder);

  std::cout << "Resolved input ROOT files / 解析得到输入 ROOT: "
            << files.size();
  if (duplicateRunFiles > 0) {
    std::cout << " (duplicate run files ignored / 忽略重复 run 文件: "
              << duplicateRunFiles << ")";
  }
  std::cout << std::endl;
  return files;
}

} // namespace tof_addback_input_detail

void tof_vs_addback_observables_calibrated(
    const char *inputFiles = "out/mult3_nfs_run_100_r0.root",
    const char *calibrationSummary = "calibration_summary.tsv",
    const char *outputFile = "tof_vs_addback_observables_calibrated.root",
    bool useBgoCsiVeto = true,
    bool applyFastTimeCut = true,
    double nfsDistanceMeter = 24.0,
    double timeFwhmNs = 20.0,
    Long64_t maxEntries = -1,
    int tofBins = 1600,
    double tofMinNs = 0.0,
    double tofMaxNs = 1600.0,
    int totalEnergyBins = 4096,
    double totalEnergyMinKeV = 0.0,
    double totalEnergyMaxKeV = 32768.0,
    const char *timeCorrectionMode = "offset",
    const char *timeBranchLeaf = "fTime")
{
  if (!inputFiles || TString(inputFiles).Strip(TString::kBoth).IsNull() ||
      !calibrationSummary ||
      TString(calibrationSummary).Strip(TString::kBoth).IsNull() ||
      !outputFile || TString(outputFile).Strip(TString::kBoth).IsNull()) {
    std::cerr << "Input ROOT, calibration summary, and output ROOT are required. / "
                 "必须提供输入 ROOT、刻度汇总和输出 ROOT。"
              << std::endl;
    return;
  }
  if (tofBins <= 0 || tofMaxNs <= tofMinNs || totalEnergyBins <= 0 ||
      totalEnergyMaxKeV <= totalEnergyMinKeV ||
      nfsDistanceMeter <= 0.0 || timeFwhmNs <= 0.0) {
    std::cerr << "Invalid histogram or time-cut parameters. / "
                 "直方图或时间 cut 参数无效。"
              << std::endl;
    return;
  }

  const std::vector<TString> inputs =
      tof_addback_input_detail::ResolveInputs(inputFiles);
  if (inputs.empty()) {
    std::cerr << "No input ROOT files. / 没有输入 ROOT 文件。" << std::endl;
    return;
  }

  const CalibrationDB calibration = ReadCalibrationDB(calibrationSummary);
  if (calibration.byRun.empty() && !calibration.hasDefault) {
    std::cerr << "No usable calibration rows in: " << calibrationSummary
              << std::endl;
    return;
  }

  const double tMin50MeVNs =
      EnergyMeVToToFNs(50.0, nfsDistanceMeter);
  const double timeSigmaNs = timeFwhmNs / 2.355;
  const double tCutNs = tMin50MeVNs - 3.0 * timeSigmaNs;
  const TString timeMode(timeCorrectionMode ? timeCorrectionMode : "offset");

  TH2F tofVsMultiplicity(
      "TOFVsAddbackGammaMultiplicity",
      "Event TOF vs clover addback gamma multiplicity;Clover addback gamma multiplicity;Calibrated event TOF (ns)",
      kNClover, 0.5, static_cast<double>(kNClover) + 0.5,
      tofBins, tofMinNs, tofMaxNs);
  TH2F tofVsTotalEnergy(
      "TOFVsAddbackGammaTotalEnergy",
      "Event TOF vs total clover addback gamma energy;Total calibrated clover addback gamma energy (keV);Calibrated event TOF (ns)",
      totalEnergyBins, totalEnergyMinKeV, totalEnergyMaxKeV,
      tofBins, tofMinNs, tofMaxNs);
  tofVsMultiplicity.SetDirectory(nullptr);
  tofVsTotalEnergy.SetDirectory(nullptr);

  Long64_t totalEntries = 0;
  Long64_t processedFiles = 0;
  Long64_t filesWithoutCalibration = 0;
  Long64_t filesMissingBranches = 0;
  Long64_t malformedEntries = 0;
  Long64_t missingCrystalCalibration = 0;
  Long64_t eventsWithoutAcceptedAddback = 0;
  Long64_t acceptedEvents = 0;
  Long64_t acceptedCloverFires = 0;
  Long64_t vetoedCloverFires = 0;
  Long64_t fastTimeCutCloverFires = 0;
  Long64_t tofOutsideVisibleRange = 0;
  Long64_t totalEnergyOutsideVisibleRange = 0;

  for (const auto &inputName : inputs) {
    if (maxEntries > 0 && totalEntries >= maxEntries) break;

    TFile input(inputName, "READ");
    if (!input.IsOpen() || input.IsZombie()) {
      std::cerr << "Cannot open input ROOT / 无法打开输入 ROOT: "
                << inputName << std::endl;
      continue;
    }
    TTree *tree = dynamic_cast<TTree *>(input.Get("TreeMaster"));
    if (!tree) {
      std::cerr << "TreeMaster is missing / 缺少 TreeMaster: "
                << inputName << std::endl;
      continue;
    }

    // A missing run calibration is not silently replaced by identity values.
    // 缺少逐 run 刻度时直接跳过该文件，避免把未刻度数据混入最终二维图。
    const RunCalib *runCalibration = FindRunCalib(calibration, inputName);
    if (!runCalibration) {
      std::cerr << "Skip file without matching calibration / 跳过无对应刻度文件: "
                << inputName << std::endl;
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
        energyBranch.IsNull() || timeBranch.IsNull() ||
        (useBgoCsiVeto &&
         (essCloverBranch.IsNull() || essCrystalBranch.IsNull() ||
          bgoBranch.IsNull() || csiBranch.IsNull()))) {
      std::cerr << "Required branches are missing / 缺少必要分支: "
                << inputName << std::endl;
      ++filesMissingBranches;
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

      std::array<CloverHit, kNClover> addback;
      if (useBgoCsiVeto) {
        const std::size_t vetoCount = std::min(
            {static_cast<std::size_t>(essClovers->GetSize()),
             static_cast<std::size_t>(essCrystals->GetSize()),
             static_cast<std::size_t>(bgo->GetSize()),
             static_cast<std::size_t>(csi->GetSize())});
        for (std::size_t index = 0; index < vetoCount; ++index) {
          const int clover = (*essClovers)[index];
          if (clover < 0 || clover >= kNClover) continue;
          if ((*bgo)[index] > 0) addback[clover].bgo = true;
          if ((*csi)[index] > 0) addback[clover].csi = true;
        }
      }

      // First calibrate every crystal, then add all crystal energies belonging
      // to the same clover. The clover time follows the highest calibrated
      // crystal energy, exactly as in the standard NFS addback reconstruction.
      // 先逐 crystal 应用能量与时间刻度，再按 clover 累加能量；clover 时间取
      // “刻度后能量最高的 crystal”的时间，与标准 NFS addback 重建一致。
      for (std::size_t index = 0; index < fireCount; ++index) {
        const int clover = clovers[index];
        const int crystal = crystals[index];
        if (clover < 0 || clover >= kNClover || crystal < 0 ||
            crystal >= kNCrystalPerClover) {
          continue;
        }
        const double rawEnergy = rawEnergies[index];
        const double rawTime = rawTimes[index];
        if (rawEnergy <= 0.0 || rawTime <= 0.0 ||
            !std::isfinite(rawEnergy) || !std::isfinite(rawTime)) {
          continue;
        }

        const int detector = clover * kNCrystalPerClover + crystal;
        const CrystalCalib &crystalCalibration =
            runCalibration->crystal[detector];
        if (!crystalCalibration.hasEnergy || !crystalCalibration.hasTime) {
          ++missingCrystalCalibration;
          continue;
        }
        const double energy = crystalCalibration.energyOffset +
                              crystalCalibration.energyGain * rawEnergy;
        const double time = ApplyTimeCorrection(
            rawTime, crystalCalibration, timeMode);
        if (energy <= 0.0 || time <= 0.0 || !std::isfinite(energy) ||
            !std::isfinite(time)) {
          continue;
        }

        CloverHit &hit = addback[clover];
        hit.energy += energy;
        ++hit.multiplicity;
        if (energy > hit.bestCrystalEnergy) {
          hit.bestCrystalEnergy = energy;
          hit.time = time;
        }
      }

      int addbackMultiplicity = 0;
      double addbackTotalEnergyKeV = 0.0;
      double eventToFNs = std::numeric_limits<double>::infinity();
      for (const auto &hit : addback) {
        if (hit.multiplicity <= 0 || hit.energy <= 0.0 ||
            !std::isfinite(hit.time)) {
          continue;
        }
        if (useBgoCsiVeto && (hit.bgo || hit.csi)) {
          ++vetoedCloverFires;
          continue;
        }
        if (applyFastTimeCut && hit.time < tCutNs) {
          ++fastTimeCutCloverFires;
          continue;
        }

        ++addbackMultiplicity;
        addbackTotalEnergyKeV += hit.energy;
        eventToFNs = std::min(eventToFNs, hit.time);
      }

      if (addbackMultiplicity <= 0 || !std::isfinite(eventToFNs)) {
        ++eventsWithoutAcceptedAddback;
        continue;
      }

      ++acceptedEvents;
      acceptedCloverFires += addbackMultiplicity;
      if (eventToFNs < tofMinNs || eventToFNs >= tofMaxNs) {
        ++tofOutsideVisibleRange;
      }
      if (addbackTotalEnergyKeV < totalEnergyMinKeV ||
          addbackTotalEnergyKeV >= totalEnergyMaxKeV) {
        ++totalEnergyOutsideVisibleRange;
      }

      // ROOT TH2::Fill(x,y): multiplicity/total energy is horizontal (X),
      // calibrated event TOF is vertical (Y), as requested.
      // ROOT TH2::Fill(x,y)：多重度/总能量填 X 轴，刻度后的 event TOF 填 Y 轴。
      tofVsMultiplicity.Fill(addbackMultiplicity, eventToFNs);
      tofVsTotalEnergy.Fill(addbackTotalEnergyKeV, eventToFNs);
    }
    input.Close();
  }

  TFile output(outputFile, "RECREATE");
  if (!output.IsOpen() || output.IsZombie()) {
    std::cerr << "Cannot create output ROOT / 无法创建输出 ROOT: "
              << outputFile << std::endl;
    return;
  }
  TDirectory *directory = output.mkdir("tof_addback_correlations");
  directory->cd();

  const TString configTitle = TString::Format(
      "inputs=%s; calibration=%s; event_tof=min_selected_addback_clover_time; use_bgo_csi_veto=%d; apply_fast_time_cut=%d; distance_m=%.9g; time_fwhm_ns=%.9g; t_min_50MeV_ns=%.9g; t_cut_ns=%.9g; tof_bins=%d; tof_range_ns=%.9g:%.9g; total_energy_bins=%d; total_energy_range_keV=%.9g:%.9g; time_mode=%s; time_branch=%s",
      inputFiles, calibrationSummary, useBgoCsiVeto ? 1 : 0,
      applyFastTimeCut ? 1 : 0, nfsDistanceMeter, timeFwhmNs,
      tMin50MeVNs, tCutNs, tofBins, tofMinNs, tofMaxNs, totalEnergyBins,
      totalEnergyMinKeV, totalEnergyMaxKeV, timeCorrectionMode,
      timeBranchLeaf);
  TNamed config("TOFVsAddbackConfig", configTitle.Data());
  config.Write();

  TH1D summary("TOFVsAddbackSummary",
               "TOFVsAddbackSummary;Category;Counts", 13, 0.5, 13.5);
  const char *labels[13] = {
      "Input entries", "Processed files", "Files no calibration",
      "Files missing branches", "Malformed entries", "Crystal calib misses",
      "Events no accepted addback", "Accepted events",
      "Accepted addback clovers", "Vetoed clover fires",
      "Fast-time-cut clover fires", "TOF outside display",
      "Total energy outside display"};
  const double values[13] = {
      static_cast<double>(totalEntries),
      static_cast<double>(processedFiles),
      static_cast<double>(filesWithoutCalibration),
      static_cast<double>(filesMissingBranches),
      static_cast<double>(malformedEntries),
      static_cast<double>(missingCrystalCalibration),
      static_cast<double>(eventsWithoutAcceptedAddback),
      static_cast<double>(acceptedEvents),
      static_cast<double>(acceptedCloverFires),
      static_cast<double>(vetoedCloverFires),
      static_cast<double>(fastTimeCutCloverFires),
      static_cast<double>(tofOutsideVisibleRange),
      static_cast<double>(totalEnergyOutsideVisibleRange)};
  for (int bin = 1; bin <= 13; ++bin) {
    summary.GetXaxis()->SetBinLabel(bin, labels[bin - 1]);
    summary.SetBinContent(bin, values[bin - 1]);
  }

  summary.Write();
  tofVsMultiplicity.Write();
  tofVsTotalEnergy.Write();
  output.Close();

  std::cout << "Processed files / 处理文件: " << processedFiles << std::endl;
  std::cout << "Input entries / 输入 event: " << totalEntries << std::endl;
  std::cout << "Accepted events / 通过 event: " << acceptedEvents << std::endl;
  std::cout << "Accepted addback clovers / 通过 addback clover: "
            << acceptedCloverFires << std::endl;
  std::cout << "t_min(50 MeV): " << tMin50MeVNs << " ns" << std::endl;
  std::cout << "t_cut: " << tCutNs
            << " ns (applied=" << (applyFastTimeCut ? "true" : "false")
            << ")" << std::endl;
  std::cout << "Output / 输出: " << outputFile << std::endl;
}
