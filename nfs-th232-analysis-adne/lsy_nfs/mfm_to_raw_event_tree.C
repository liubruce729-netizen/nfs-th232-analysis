// Standalone MFM binary -> raw ROOT event-tree converter.
// 独立的 MFM 二进制 -> raw ROOT event tree 转换脚本。
//
// Purpose / 目的
// -----------------------------------------------------------------------------
// One ROOT entry represents one top-level MFM frame read from the input file.
// If that top frame is MERGE_EN or MERGE_TS, its child frames are unfolded
// recursively by default. Detector frames found below it are stored as vectors,
// so one ROOT entry naturally contains all raw detector fires of that MFM event.
//
// ROOT 中一个 entry 对应输入文件中直接读到的一个 top-level MFM frame。
// 如果 top frame 是 MERGE_EN 或 MERGE_TS，默认递归拆解其所有子 frame。
// 同一 top event 内找到的 detector frame 以 vector 形式保存，因此一个 ROOT
// entry 会自然包含该 MFM event 内的全部 raw detector fire。
//
// Strict raw-data contract / 严格 raw-data 约定
// -----------------------------------------------------------------------------
// This macro only decodes fields physically present in MFM headers/payloads.
// It deliberately does NOT apply:
//   * ADNE lookup tables (board/channel -> physical detector mapping),
//   * energy or time calibration,
//   * DeltaT reversal, gamma-flash offset, TOF, or neutron-energy conversion,
//   * timestamp differences, relative time, sorting, or rate calculations,
//   * prompt/threshold/BGO/CSI cuts, addback, multiplicity, or event selection.
//
// 本脚本只解码 MFM header/payload 中真实保存的原始字段，明确不执行：
//   * ADNE LUT 映射（电子学 board/channel -> 物理探测器）；
//   * 能量或时间刻度；
//   * DeltaT 翻转、gamma-flash offset、TOF 或中子能量换算；
//   * TS 差、相对时间、排序或计数率计算；
//   * prompt/阈值/BGO/CSI cut、addback、多重度或 event 筛选。
//
// Input forms / 输入形式
// -----------------------------------------------------------------------------
// 1. One MFM file / 单个 MFM 文件:
//    root -l -b -q 'lsy_nfs/mfm_to_raw_event_tree.C("data/run.dat",1000,"out/raw.root")'
//
// 2. A text list, explicitly prefixed with @ / 用 @ 显式指定文本列表:
//    root -l -b -q 'lsy_nfs/mfm_to_raw_event_tree.C("@mfm_files.txt",-1,"out/raw_all.root")'
//
// 3. A .txt/.list/.lst path is also recognized automatically.
//    .txt/.list/.lst 路径也会自动识别为列表。
//
// List rules / 列表规则:
//   * one input path per line / 每行一个输入文件；
//   * empty lines and lines beginning with # or // are ignored / 忽略空行及注释；
//   * a relative path is tried from the current directory first, then relative
//     to the list file directory / 相对路径先按当前目录解析，再按列表目录解析。
//
// Full function interface / 完整函数接口
// -----------------------------------------------------------------------------
// mfm_to_raw_event_tree(inputSpec,
//                       maxEvents,
//                       outputFile,
//                       startEvent,
//                       unfoldMerge,
//                       progressEvery)
//
// maxEvents    : total number of output events across all files; <0 = all.
//                所有输入文件合计写出的 event 数；<0 表示全部。
// startEvent   : number of top events to skip globally across the input list.
//                在整个输入序列开头全局跳过多少个 top event。
// unfoldMerge  : recursively unpack merge children; default true.
//                是否递归展开 merge 子 frame；默认 true。
// progressEvery: print progress every N written events; <=0 disables it.
//                每写 N 个 event 打印进度；<=0 关闭进度打印。
//
// Output / 输出
// -----------------------------------------------------------------------------
//   RawEventTree : one entry per top-level MFM event / 每个 top event 一个 entry
//   RawInputTree : one entry per input file / 每个输入文件一个 entry
//
// Vector alignment / vector 对齐规则:
//   * all raw_frame_* vectors have the same length: all recursively visited frames;
//   * all raw_exo_* vectors have the same length: EXO2 fires only;
//   * similarly for raw_neda_*, raw_diamant_*, raw_paris_*, ...;
//   * every detector group has raw_<detector>_frame_index, which points back to
//     the corresponding element of raw_frame_*.
//
//   * 所有 raw_frame_* vector 等长，表示递归访问到的全部 frame；
//   * 所有 raw_exo_* vector 等长，只表示 EXO2 fire；
//   * raw_neda_*、raw_diamant_*、raw_paris_* 等同理；
//   * 每类探测器都有 raw_<detector>_frame_index，可回连到 raw_frame_* 中的行号。

R__ADD_INCLUDE_PATH($MFMSYS/include)
R__LOAD_LIBRARY($MFMSYS/lib/libMFM.so)

#include <MFMCommonFrame.h>
#include <MFMDiamantFrame.h>
#include <MFMEbyedatFrame.h>
#include <MFMExogamFrame.h>
#include <MFMMergeFrame.h>
#include <MFMNedaCompFrame.h>
#include <MFMNedaFrame.h>
#include <MFMParisFrame.h>
#include <MFMReaGenericFrame.h>
#include <MFMTypes.h>
#include <MFMVamosICFrame.h>

#include <TFile.h>
#include <TNamed.h>
#include <TString.h>
#include <TSystem.h>
#include <TTree.h>

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

namespace mfm_raw_event_detail {

// Return true only for the two MFM event-container formats.
// 只把两种 MFM merge 格式视为可递归展开的 event container。
bool IsMergeType(UShort_t frameType)
{
  return frameType == MFM_MERGE_EN_FRAME_TYPE ||
         frameType == MFM_MERGE_TS_FRAME_TYPE;
}

// EBYEDAT uses three related frame type codes.
// EBYEDAT 使用三种相近的 frame type。
bool IsEbyedatType(UShort_t frameType)
{
  return frameType == MFM_EBY_EN_FRAME_TYPE ||
         frameType == MFM_EBY_TS_FRAME_TYPE ||
         frameType == MFM_EBY_EN_TS_FRAME_TYPE;
}

// These payloads are decoded into typed branches below. Any other frame is
// retained byte-for-byte in the raw_opaque_* branches.
// 以下 payload 会被解码到有类型的分支；其他 frame 则逐字节保存到 raw_opaque_*。
bool HasTypedPayloadDecoder(UShort_t frameType)
{
  return IsMergeType(frameType) || IsEbyedatType(frameType) ||
         frameType == MFM_EXO2_FRAME_TYPE ||
         frameType == MFM_DIAMANT_FRAME_TYPE ||
         frameType == MFM_NEDA_FRAME_TYPE ||
         frameType == MFM_NEDACOMP_FRAME_TYPE ||
         frameType == MFM_PARIS_FRAME_TYPE ||
         frameType == MFM_REA_GENE_FRAME_TYPE ||
         frameType == MFM_VAMOSIC_FRAME_TYPE;
}

// Remove leading/trailing ASCII whitespace without changing the path itself.
// 去掉首尾 ASCII 空白，不修改路径内部字符。
std::string Trim(const std::string &text)
{
  std::size_t first = 0;
  while (first < text.size() &&
         std::isspace(static_cast<unsigned char>(text[first]))) {
    ++first;
  }
  std::size_t last = text.size();
  while (last > first &&
         std::isspace(static_cast<unsigned char>(text[last - 1]))) {
    --last;
  }
  return text.substr(first, last - first);
}

bool IsReadableFile(const TString &path)
{
  return !path.IsNull() && !gSystem->AccessPathName(path, kReadPermission);
}

TString ExpandPath(const TString &path)
{
  TString expanded(path);
  gSystem->ExpandPathName(expanded);
  return expanded;
}

// Resolve a path from a list. Existing paths relative to the current working
// directory win; otherwise the path is interpreted relative to the list file.
// 解析列表中的路径：优先使用相对当前目录已存在的路径，否则相对列表目录解析。
TString ResolveListedPath(const TString &listedPath, const TString &listDir)
{
  TString direct = ExpandPath(listedPath);
  if (IsReadableFile(direct)) return direct;

  if (!gSystem->IsAbsoluteFileName(listedPath)) {
    TString candidate = listDir;
    if (!candidate.EndsWith("/")) candidate += "/";
    candidate += listedPath;
    candidate = ExpandPath(candidate);
    if (IsReadableFile(candidate)) return candidate;
  }
  return "";
}

bool LooksLikeList(const TString &inputSpec)
{
  TString lower(inputSpec);
  lower.ToLower();
  return inputSpec.BeginsWith("@") || lower.EndsWith(".txt") ||
         lower.EndsWith(".list") || lower.EndsWith(".lst");
}

// Parse either one MFM path or a text list into an ordered input vector.
// 把单个 MFM 路径或文本列表统一解析为有序输入 vector。
bool ResolveInputs(const char *inputSpec,
                   std::vector<TString> &inputFiles,
                   TString &normalizedSpec)
{
  inputFiles.clear();
  if (!inputSpec || TString(inputSpec).IsNull()) {
    std::cerr << "Input specification is empty / 输入为空" << std::endl;
    return false;
  }

  TString spec(inputSpec);
  const bool isList = LooksLikeList(spec);
  if (!isList) {
    TString one = ExpandPath(spec);
    if (!IsReadableFile(one)) {
      std::cerr << "Cannot read input MFM file / 无法读取 MFM 文件: "
                << one << std::endl;
      return false;
    }
    inputFiles.push_back(one);
    normalizedSpec = one;
    return true;
  }

  if (spec.BeginsWith("@")) spec.Remove(0, 1);
  spec = ExpandPath(spec);
  normalizedSpec = spec;
  if (!IsReadableFile(spec)) {
    std::cerr << "Cannot read input list / 无法读取输入列表: "
              << spec << std::endl;
    return false;
  }

  std::ifstream list(spec.Data());
  if (!list.good()) {
    std::cerr << "Cannot open input list / 无法打开输入列表: "
              << spec << std::endl;
    return false;
  }

  TString listDir = gSystem->DirName(spec);
  std::string line;
  Long64_t lineNumber = 0;
  while (std::getline(list, line)) {
    ++lineNumber;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    const std::string clean = Trim(line);
    if (clean.empty() || clean[0] == '#' || clean.rfind("//", 0) == 0) {
      continue;
    }

    TString resolved = ResolveListedPath(clean.c_str(), listDir);
    if (resolved.IsNull()) {
      std::cerr << "Missing/unreadable list entry at line " << lineNumber
                << " / 列表中不存在或不可读的路径: " << clean << std::endl;
      return false;
    }
    inputFiles.push_back(resolved);
  }

  if (inputFiles.empty()) {
    std::cerr << "Input list contains no files / 输入列表没有有效文件: "
              << spec << std::endl;
    return false;
  }
  return true;
}

TString DefaultOutputName(const TString &normalizedSpec, bool inputWasList)
{
  TString base = gSystem->BaseName(normalizedSpec);
  if (inputWasList) {
    if (base.EndsWith(".txt", TString::kIgnoreCase)) base.Resize(base.Length() - 4);
    else if (base.EndsWith(".list", TString::kIgnoreCase)) base.Resize(base.Length() - 5);
    else if (base.EndsWith(".lst", TString::kIgnoreCase)) base.Resize(base.Length() - 4);
  }
  base += "_raw_event_tree.root";
  return base;
}

// Read integer words from the original frame byte order and return the logical
// unsigned value in host order. This is a lossless endian decode, not a
// calibration or physical transformation.
// 按 frame 自身字节序读取整数并返回 host-order 逻辑值。这只是无损字节序解码，
// 不是刻度或物理变换。
bool FrameIsBigEndian(const MFMCommonFrame *frame)
{
  return frame && frame->GetFrameEndianness() == MFM_BIG_ENDIAN;
}

UChar_t ReadU8At(const MFMCommonFrame *frame, std::size_t offset)
{
  if (!frame || !frame->GetPointHeader() ||
      offset >= static_cast<std::size_t>(std::max(0, frame->GetFrameSize()))) {
    return 0;
  }
  const UChar_t *bytes =
      reinterpret_cast<const UChar_t *>(frame->GetPointHeader());
  return bytes[offset];
}

UShort_t ReadU16At(const MFMCommonFrame *frame, std::size_t offset)
{
  if (!frame || !frame->GetPointHeader() ||
      offset + 2 > static_cast<std::size_t>(std::max(0, frame->GetFrameSize()))) {
    return 0;
  }
  const UChar_t *b =
      reinterpret_cast<const UChar_t *>(frame->GetPointHeader()) + offset;
  if (FrameIsBigEndian(frame)) {
    return static_cast<UShort_t>((static_cast<UInt_t>(b[0]) << 8) | b[1]);
  }
  return static_cast<UShort_t>(b[0] | (static_cast<UInt_t>(b[1]) << 8));
}

UInt_t ReadU24At(const MFMCommonFrame *frame, std::size_t offset)
{
  if (!frame || !frame->GetPointHeader() ||
      offset + 3 > static_cast<std::size_t>(std::max(0, frame->GetFrameSize()))) {
    return 0;
  }
  const UChar_t *b =
      reinterpret_cast<const UChar_t *>(frame->GetPointHeader()) + offset;
  if (FrameIsBigEndian(frame)) {
    return (static_cast<UInt_t>(b[0]) << 16) |
           (static_cast<UInt_t>(b[1]) << 8) | b[2];
  }
  return b[0] | (static_cast<UInt_t>(b[1]) << 8) |
         (static_cast<UInt_t>(b[2]) << 16);
}

UInt_t ReadU32At(const MFMCommonFrame *frame, std::size_t offset)
{
  if (!frame || !frame->GetPointHeader() ||
      offset + 4 > static_cast<std::size_t>(std::max(0, frame->GetFrameSize()))) {
    return 0;
  }
  const UChar_t *b =
      reinterpret_cast<const UChar_t *>(frame->GetPointHeader()) + offset;
  if (FrameIsBigEndian(frame)) {
    return (static_cast<UInt_t>(b[0]) << 24) |
           (static_cast<UInt_t>(b[1]) << 16) |
           (static_cast<UInt_t>(b[2]) << 8) | b[3];
  }
  return b[0] | (static_cast<UInt_t>(b[1]) << 8) |
         (static_cast<UInt_t>(b[2]) << 16) |
         (static_cast<UInt_t>(b[3]) << 24);
}

struct RawEventRow {
  // Event/file bookkeeping. These fields describe where the raw event came from.
  // event/file 元数据，只记录 raw event 来自哪里。
  ULong64_t event_index = 0;
  UInt_t input_file_index = 0;
  ULong64_t input_event_index = 0;
  Long64_t input_file_offset = -1;
  Int_t input_read_size = 0;

  // Raw top-frame header values.
  // top frame 的原始 header 逻辑值。
  UShort_t raw_top_frame_type = 0;
  UShort_t raw_top_data_source = 0;
  UChar_t raw_top_revision = 0;
  UInt_t raw_top_frame_size = 0;
  UInt_t raw_top_header_size = 0;
  UInt_t raw_top_unit_block_size = 0;
  Int_t raw_top_nb_items = -1;
  UInt_t raw_top_event_number = 0;
  ULong64_t raw_top_timestamp = 0;
  UInt_t raw_top_merge_delta_time = 0;

  // Every recursively visited frame. All vectors in this block are aligned.
  // 递归访问到的每个 frame；本组所有 vector 严格等长。
  std::vector<Int_t> raw_frame_parent_index;
  std::vector<Int_t> raw_frame_depth;
  std::vector<Int_t> raw_frame_index_in_parent;
  std::vector<Long64_t> raw_frame_byte_offset_in_top;
  std::vector<UShort_t> raw_frame_type;
  std::vector<UShort_t> raw_frame_data_source;
  std::vector<UChar_t> raw_frame_revision;
  std::vector<UInt_t> raw_frame_size;
  std::vector<UInt_t> raw_frame_header_size;
  std::vector<UInt_t> raw_frame_unit_block_size;
  std::vector<Int_t> raw_frame_nb_items;
  std::vector<UInt_t> raw_frame_event_number;
  std::vector<ULong64_t> raw_frame_timestamp;
  std::vector<UInt_t> raw_frame_merge_delta_time;

  // EXOGAM2 raw fires: exactly the 17 uint16 payload words.
  // EXOGAM2 raw fire：严格保存 payload 中的 17 个 uint16 字段。
  std::vector<Int_t> raw_exo_frame_index;
  std::vector<UShort_t> raw_exo_cristal_id;
  std::vector<UShort_t> raw_exo_status1;
  std::vector<UShort_t> raw_exo_status2;
  std::vector<UShort_t> raw_exo_status3;
  std::vector<UShort_t> raw_exo_delta_t;
  std::vector<UShort_t> raw_exo_inner_m6;
  std::vector<UShort_t> raw_exo_inner_m20;
  std::vector<UShort_t> raw_exo_outer1;
  std::vector<UShort_t> raw_exo_outer2;
  std::vector<UShort_t> raw_exo_outer3;
  std::vector<UShort_t> raw_exo_outer4;
  std::vector<UShort_t> raw_exo_bgo;
  std::vector<UShort_t> raw_exo_csi;
  std::vector<UShort_t> raw_exo_inner_t30;
  std::vector<UShort_t> raw_exo_inner_t60;
  std::vector<UShort_t> raw_exo_inner_t90;
  std::vector<UShort_t> raw_exo_padding;

  // DIAMANT raw fires.
  // DIAMANT raw fire。
  std::vector<Int_t> raw_diamant_frame_index;
  std::vector<UShort_t> raw_diamant_cristal_id;
  std::vector<UChar_t> raw_diamant_status1;
  std::vector<UChar_t> raw_diamant_status2;
  std::vector<UInt_t> raw_diamant_energy;
  std::vector<UInt_t> raw_diamant_top;
  std::vector<UShort_t> raw_diamant_checksum;

  // NEDA raw fires. Sample words retain the parity bit and are flattened to
  // avoid nested STL dictionaries. offset/count locate each fire's samples.
  // NEDA raw fire。sample word 保留 parity bit，并展平成一个 vector；
  // offset/count 用来定位每个 fire 的 waveform。
  std::vector<Int_t> raw_neda_frame_index;
  std::vector<UShort_t> raw_neda_location_id;
  std::vector<UChar_t> raw_neda_le_interval;
  std::vector<UChar_t> raw_neda_zco_interval;
  std::vector<UShort_t> raw_neda_tdc_value;
  std::vector<UShort_t> raw_neda_free0;
  std::vector<UShort_t> raw_neda_slow_integral;
  std::vector<UShort_t> raw_neda_free1;
  std::vector<UShort_t> raw_neda_fast_integral;
  std::vector<UChar_t> raw_neda_bitfield;
  std::vector<UChar_t> raw_neda_abs_max;
  std::vector<UChar_t> raw_neda_interpol_cfd;
  std::vector<UChar_t> raw_neda_free2;
  std::vector<UInt_t> raw_neda_sample_offset;
  std::vector<UShort_t> raw_neda_sample_count;
  std::vector<UShort_t> raw_neda_sample_word;
  std::vector<UInt_t> raw_neda_end_of_frame;

  // NEDA compressed raw fires.
  // NEDA compressed raw fire。
  std::vector<Int_t> raw_neda_comp_frame_index;
  std::vector<UShort_t> raw_neda_comp_location_id;
  std::vector<UShort_t> raw_neda_comp_energy;
  std::vector<UShort_t> raw_neda_comp_time;
  std::vector<UShort_t> raw_neda_comp_tdc_cor_value;
  std::vector<UInt_t> raw_neda_comp_slow_integral;
  std::vector<UInt_t> raw_neda_comp_fast_integral;
  std::vector<Int_t> raw_neda_comp_int_raise_time;
  std::vector<UShort_t> raw_neda_comp_neural_network;
  std::vector<UChar_t> raw_neda_comp_nb_zero;
  std::vector<UChar_t> raw_neda_comp_neutron_flag;

  // PARIS stores CFD as a raw 24-bit word. We intentionally do not call
  // GetCfd(), because that API adds random smearing and divides by 1000.
  // PARIS CFD 保存为原始 24-bit word；这里故意不调用会加随机展宽并除以
  // 1000 的 GetCfd()。
  std::vector<Int_t> raw_paris_frame_index;
  std::vector<UShort_t> raw_paris_location_id;
  std::vector<UShort_t> raw_paris_qshort;
  std::vector<UShort_t> raw_paris_qlong;
  std::vector<UInt_t> raw_paris_cfd_word;
  std::vector<UChar_t> raw_paris_flags;

  // REA_GENERIC raw fires.
  // REA_GENERIC raw fire。
  std::vector<Int_t> raw_generic_frame_index;
  std::vector<UShort_t> raw_generic_cristal_id;
  std::vector<UShort_t> raw_generic_status1;
  std::vector<UShort_t> raw_generic_status2;
  std::vector<UShort_t> raw_generic_type_tns;
  std::vector<UShort_t> raw_generic_energy;
  std::vector<UShort_t> raw_generic_time;
  std::vector<UShort_t> raw_generic_checksum;

  // VAMOS ionization-chamber raw fires, including reserved words.
  // VAMOS IC raw fire，同时保留两个 reserved/not-used word。
  std::vector<Int_t> raw_vamos_frame_index;
  std::vector<UShort_t> raw_vamos_cristal_id;
  std::vector<UShort_t> raw_vamos_status1;
  std::vector<UShort_t> raw_vamos_status2;
  std::vector<UShort_t> raw_vamos_not_used1;
  std::vector<UShort_t> raw_vamos_energy;
  std::vector<UShort_t> raw_vamos_not_used2;
  std::vector<UShort_t> raw_vamos_checksum;

  // EBYEDAT parameters. Several parameter items may belong to one frame.
  // EBYEDAT 参数；一个 frame 可以包含多个 label/value item。
  std::vector<Int_t> raw_ebye_frame_index;
  std::vector<UShort_t> raw_ebye_label;
  std::vector<UShort_t> raw_ebye_value;

  // Unsupported/control/XML frame bytes. byte_offset/count delimit each frame
  // in the flat byte vector, preserving information without guessing a schema.
  // 未支持/control/XML frame 的原始字节。offset/count 在展平 byte vector 中
  // 定位每个 frame，避免对未知格式进行猜测。
  std::vector<Int_t> raw_opaque_frame_index;
  std::vector<UInt_t> raw_opaque_byte_offset;
  std::vector<UInt_t> raw_opaque_byte_count;
  std::vector<UChar_t> raw_opaque_bytes;

  void ClearVectors()
  {
    raw_frame_parent_index.clear(); raw_frame_depth.clear();
    raw_frame_index_in_parent.clear(); raw_frame_byte_offset_in_top.clear();
    raw_frame_type.clear(); raw_frame_data_source.clear();
    raw_frame_revision.clear(); raw_frame_size.clear();
    raw_frame_header_size.clear(); raw_frame_unit_block_size.clear();
    raw_frame_nb_items.clear(); raw_frame_event_number.clear();
    raw_frame_timestamp.clear(); raw_frame_merge_delta_time.clear();

    raw_exo_frame_index.clear(); raw_exo_cristal_id.clear();
    raw_exo_status1.clear(); raw_exo_status2.clear(); raw_exo_status3.clear();
    raw_exo_delta_t.clear(); raw_exo_inner_m6.clear(); raw_exo_inner_m20.clear();
    raw_exo_outer1.clear(); raw_exo_outer2.clear(); raw_exo_outer3.clear();
    raw_exo_outer4.clear(); raw_exo_bgo.clear(); raw_exo_csi.clear();
    raw_exo_inner_t30.clear(); raw_exo_inner_t60.clear();
    raw_exo_inner_t90.clear(); raw_exo_padding.clear();

    raw_diamant_frame_index.clear(); raw_diamant_cristal_id.clear();
    raw_diamant_status1.clear(); raw_diamant_status2.clear();
    raw_diamant_energy.clear(); raw_diamant_top.clear();
    raw_diamant_checksum.clear();

    raw_neda_frame_index.clear(); raw_neda_location_id.clear();
    raw_neda_le_interval.clear(); raw_neda_zco_interval.clear();
    raw_neda_tdc_value.clear(); raw_neda_free0.clear();
    raw_neda_slow_integral.clear(); raw_neda_free1.clear();
    raw_neda_fast_integral.clear(); raw_neda_bitfield.clear();
    raw_neda_abs_max.clear(); raw_neda_interpol_cfd.clear();
    raw_neda_free2.clear(); raw_neda_sample_offset.clear();
    raw_neda_sample_count.clear(); raw_neda_sample_word.clear();
    raw_neda_end_of_frame.clear();

    raw_neda_comp_frame_index.clear(); raw_neda_comp_location_id.clear();
    raw_neda_comp_energy.clear(); raw_neda_comp_time.clear();
    raw_neda_comp_tdc_cor_value.clear(); raw_neda_comp_slow_integral.clear();
    raw_neda_comp_fast_integral.clear(); raw_neda_comp_int_raise_time.clear();
    raw_neda_comp_neural_network.clear(); raw_neda_comp_nb_zero.clear();
    raw_neda_comp_neutron_flag.clear();

    raw_paris_frame_index.clear(); raw_paris_location_id.clear();
    raw_paris_qshort.clear(); raw_paris_qlong.clear();
    raw_paris_cfd_word.clear(); raw_paris_flags.clear();

    raw_generic_frame_index.clear(); raw_generic_cristal_id.clear();
    raw_generic_status1.clear(); raw_generic_status2.clear();
    raw_generic_type_tns.clear(); raw_generic_energy.clear();
    raw_generic_time.clear(); raw_generic_checksum.clear();

    raw_vamos_frame_index.clear(); raw_vamos_cristal_id.clear();
    raw_vamos_status1.clear(); raw_vamos_status2.clear();
    raw_vamos_not_used1.clear(); raw_vamos_energy.clear();
    raw_vamos_not_used2.clear(); raw_vamos_checksum.clear();

    raw_ebye_frame_index.clear(); raw_ebye_label.clear(); raw_ebye_value.clear();
    raw_opaque_frame_index.clear(); raw_opaque_byte_offset.clear();
    raw_opaque_byte_count.clear(); raw_opaque_bytes.clear();
  }
};

void MakeEventBranches(TTree &tree, RawEventRow &r)
{
  tree.Branch("event_index", &r.event_index);
  tree.Branch("input_file_index", &r.input_file_index);
  tree.Branch("input_event_index", &r.input_event_index);
  tree.Branch("input_file_offset", &r.input_file_offset);
  tree.Branch("input_read_size", &r.input_read_size);

  tree.Branch("raw_top_frame_type", &r.raw_top_frame_type);
  tree.Branch("raw_top_data_source", &r.raw_top_data_source);
  tree.Branch("raw_top_revision", &r.raw_top_revision);
  tree.Branch("raw_top_frame_size", &r.raw_top_frame_size);
  tree.Branch("raw_top_header_size", &r.raw_top_header_size);
  tree.Branch("raw_top_unit_block_size", &r.raw_top_unit_block_size);
  tree.Branch("raw_top_nb_items", &r.raw_top_nb_items);
  tree.Branch("raw_top_event_number", &r.raw_top_event_number);
  tree.Branch("raw_top_timestamp", &r.raw_top_timestamp);
  tree.Branch("raw_top_merge_delta_time", &r.raw_top_merge_delta_time);

  tree.Branch("raw_frame_parent_index", &r.raw_frame_parent_index);
  tree.Branch("raw_frame_depth", &r.raw_frame_depth);
  tree.Branch("raw_frame_index_in_parent", &r.raw_frame_index_in_parent);
  tree.Branch("raw_frame_byte_offset_in_top", &r.raw_frame_byte_offset_in_top);
  tree.Branch("raw_frame_type", &r.raw_frame_type);
  tree.Branch("raw_frame_data_source", &r.raw_frame_data_source);
  tree.Branch("raw_frame_revision", &r.raw_frame_revision);
  tree.Branch("raw_frame_size", &r.raw_frame_size);
  tree.Branch("raw_frame_header_size", &r.raw_frame_header_size);
  tree.Branch("raw_frame_unit_block_size", &r.raw_frame_unit_block_size);
  tree.Branch("raw_frame_nb_items", &r.raw_frame_nb_items);
  tree.Branch("raw_frame_event_number", &r.raw_frame_event_number);
  tree.Branch("raw_frame_timestamp", &r.raw_frame_timestamp);
  tree.Branch("raw_frame_merge_delta_time", &r.raw_frame_merge_delta_time);

  tree.Branch("raw_exo_frame_index", &r.raw_exo_frame_index);
  tree.Branch("raw_exo_cristal_id", &r.raw_exo_cristal_id);
  tree.Branch("raw_exo_status1", &r.raw_exo_status1);
  tree.Branch("raw_exo_status2", &r.raw_exo_status2);
  tree.Branch("raw_exo_status3", &r.raw_exo_status3);
  tree.Branch("raw_exo_delta_t", &r.raw_exo_delta_t);
  tree.Branch("raw_exo_inner_m6", &r.raw_exo_inner_m6);
  tree.Branch("raw_exo_inner_m20", &r.raw_exo_inner_m20);
  tree.Branch("raw_exo_outer1", &r.raw_exo_outer1);
  tree.Branch("raw_exo_outer2", &r.raw_exo_outer2);
  tree.Branch("raw_exo_outer3", &r.raw_exo_outer3);
  tree.Branch("raw_exo_outer4", &r.raw_exo_outer4);
  tree.Branch("raw_exo_bgo", &r.raw_exo_bgo);
  tree.Branch("raw_exo_csi", &r.raw_exo_csi);
  tree.Branch("raw_exo_inner_t30", &r.raw_exo_inner_t30);
  tree.Branch("raw_exo_inner_t60", &r.raw_exo_inner_t60);
  tree.Branch("raw_exo_inner_t90", &r.raw_exo_inner_t90);
  tree.Branch("raw_exo_padding", &r.raw_exo_padding);

  tree.Branch("raw_diamant_frame_index", &r.raw_diamant_frame_index);
  tree.Branch("raw_diamant_cristal_id", &r.raw_diamant_cristal_id);
  tree.Branch("raw_diamant_status1", &r.raw_diamant_status1);
  tree.Branch("raw_diamant_status2", &r.raw_diamant_status2);
  tree.Branch("raw_diamant_energy", &r.raw_diamant_energy);
  tree.Branch("raw_diamant_top", &r.raw_diamant_top);
  tree.Branch("raw_diamant_checksum", &r.raw_diamant_checksum);

  tree.Branch("raw_neda_frame_index", &r.raw_neda_frame_index);
  tree.Branch("raw_neda_location_id", &r.raw_neda_location_id);
  tree.Branch("raw_neda_le_interval", &r.raw_neda_le_interval);
  tree.Branch("raw_neda_zco_interval", &r.raw_neda_zco_interval);
  tree.Branch("raw_neda_tdc_value", &r.raw_neda_tdc_value);
  tree.Branch("raw_neda_free0", &r.raw_neda_free0);
  tree.Branch("raw_neda_slow_integral", &r.raw_neda_slow_integral);
  tree.Branch("raw_neda_free1", &r.raw_neda_free1);
  tree.Branch("raw_neda_fast_integral", &r.raw_neda_fast_integral);
  tree.Branch("raw_neda_bitfield", &r.raw_neda_bitfield);
  tree.Branch("raw_neda_abs_max", &r.raw_neda_abs_max);
  tree.Branch("raw_neda_interpol_cfd", &r.raw_neda_interpol_cfd);
  tree.Branch("raw_neda_free2", &r.raw_neda_free2);
  tree.Branch("raw_neda_sample_offset", &r.raw_neda_sample_offset);
  tree.Branch("raw_neda_sample_count", &r.raw_neda_sample_count);
  tree.Branch("raw_neda_sample_word", &r.raw_neda_sample_word);
  tree.Branch("raw_neda_end_of_frame", &r.raw_neda_end_of_frame);

  tree.Branch("raw_neda_comp_frame_index", &r.raw_neda_comp_frame_index);
  tree.Branch("raw_neda_comp_location_id", &r.raw_neda_comp_location_id);
  tree.Branch("raw_neda_comp_energy", &r.raw_neda_comp_energy);
  tree.Branch("raw_neda_comp_time", &r.raw_neda_comp_time);
  tree.Branch("raw_neda_comp_tdc_cor_value", &r.raw_neda_comp_tdc_cor_value);
  tree.Branch("raw_neda_comp_slow_integral", &r.raw_neda_comp_slow_integral);
  tree.Branch("raw_neda_comp_fast_integral", &r.raw_neda_comp_fast_integral);
  tree.Branch("raw_neda_comp_int_raise_time", &r.raw_neda_comp_int_raise_time);
  tree.Branch("raw_neda_comp_neural_network", &r.raw_neda_comp_neural_network);
  tree.Branch("raw_neda_comp_nb_zero", &r.raw_neda_comp_nb_zero);
  tree.Branch("raw_neda_comp_neutron_flag", &r.raw_neda_comp_neutron_flag);

  tree.Branch("raw_paris_frame_index", &r.raw_paris_frame_index);
  tree.Branch("raw_paris_location_id", &r.raw_paris_location_id);
  tree.Branch("raw_paris_qshort", &r.raw_paris_qshort);
  tree.Branch("raw_paris_qlong", &r.raw_paris_qlong);
  tree.Branch("raw_paris_cfd_word", &r.raw_paris_cfd_word);
  tree.Branch("raw_paris_flags", &r.raw_paris_flags);

  tree.Branch("raw_generic_frame_index", &r.raw_generic_frame_index);
  tree.Branch("raw_generic_cristal_id", &r.raw_generic_cristal_id);
  tree.Branch("raw_generic_status1", &r.raw_generic_status1);
  tree.Branch("raw_generic_status2", &r.raw_generic_status2);
  tree.Branch("raw_generic_type_tns", &r.raw_generic_type_tns);
  tree.Branch("raw_generic_energy", &r.raw_generic_energy);
  tree.Branch("raw_generic_time", &r.raw_generic_time);
  tree.Branch("raw_generic_checksum", &r.raw_generic_checksum);

  tree.Branch("raw_vamos_frame_index", &r.raw_vamos_frame_index);
  tree.Branch("raw_vamos_cristal_id", &r.raw_vamos_cristal_id);
  tree.Branch("raw_vamos_status1", &r.raw_vamos_status1);
  tree.Branch("raw_vamos_status2", &r.raw_vamos_status2);
  tree.Branch("raw_vamos_not_used1", &r.raw_vamos_not_used1);
  tree.Branch("raw_vamos_energy", &r.raw_vamos_energy);
  tree.Branch("raw_vamos_not_used2", &r.raw_vamos_not_used2);
  tree.Branch("raw_vamos_checksum", &r.raw_vamos_checksum);

  tree.Branch("raw_ebye_frame_index", &r.raw_ebye_frame_index);
  tree.Branch("raw_ebye_label", &r.raw_ebye_label);
  tree.Branch("raw_ebye_value", &r.raw_ebye_value);

  tree.Branch("raw_opaque_frame_index", &r.raw_opaque_frame_index);
  tree.Branch("raw_opaque_byte_offset", &r.raw_opaque_byte_offset);
  tree.Branch("raw_opaque_byte_count", &r.raw_opaque_byte_count);
  tree.Branch("raw_opaque_bytes", &r.raw_opaque_bytes);
}

// Store an unsupported frame byte-for-byte. This preserves XML/control/custom
// payloads while avoiding an invented interpretation.
// 对未知 frame 逐字节保存，保留 XML/control/custom payload，同时不猜测其含义。
void CaptureOpaqueFrame(MFMCommonFrame *frame, Int_t frameIndex, RawEventRow &r)
{
  if (!frame || !frame->GetPointHeader() || frame->GetFrameSize() <= 0) return;
  const UInt_t offset = static_cast<UInt_t>(r.raw_opaque_bytes.size());
  const UInt_t count = static_cast<UInt_t>(frame->GetFrameSize());
  const UChar_t *begin =
      reinterpret_cast<const UChar_t *>(frame->GetPointHeader());
  r.raw_opaque_frame_index.push_back(frameIndex);
  r.raw_opaque_byte_offset.push_back(offset);
  r.raw_opaque_byte_count.push_back(count);
  r.raw_opaque_bytes.insert(r.raw_opaque_bytes.end(), begin, begin + count);
}

// MFMCommonFrame only knows the generic common-header layout. A concrete
// frame class may report a larger detector/merge header, so refresh the last
// common-frame row after the concrete class has interpreted the same bytes.
// MFMCommonFrame 只认识通用公共头；具体探测器/merge 类可能有更长的 header。
// 因此具体类解析同一段 bytes 后，要回填当前公共 frame 行的真实布局参数。
void RefreshCurrentFrameLayout(MFMCommonFrame *concrete, RawEventRow &r)
{
  if (!concrete || r.raw_frame_size.empty()) return;
  r.raw_frame_size.back() =
      static_cast<UInt_t>(std::max(0, concrete->GetFrameSize()));
  r.raw_frame_header_size.back() =
      static_cast<UInt_t>(std::max(0, concrete->GetHeaderSize()));
  r.raw_frame_unit_block_size.back() =
      static_cast<UInt_t>(std::max(0, concrete->GetUnitBlockSize()));
}

// Recursively visit one frame, append its common raw header, then decode its
// detector-specific payload without LUT, cuts, calibration, or derived timing.
// 递归访问一个 frame：先保存公共 raw header，再无 LUT、无 cut、无刻度地解码 payload。
void CaptureFrame(MFMCommonFrame *frame,
                  RawEventRow &r,
                  Int_t depth,
                  Int_t indexInParent,
                  Int_t parentFrameIndex,
                  const UChar_t *topBegin,
                  std::size_t topSize,
                  bool unfoldMerge)
{
  if (!frame) return;
  frame->SetAttributs();

  const Int_t frameIndex = static_cast<Int_t>(r.raw_frame_type.size());
  Long64_t byteOffset = -1;
  const UChar_t *frameBegin =
      reinterpret_cast<const UChar_t *>(frame->GetPointHeader());
  if (topBegin && frameBegin && frameBegin >= topBegin &&
      frameBegin < topBegin + topSize) {
    byteOffset = static_cast<Long64_t>(frameBegin - topBegin);
  }

  r.raw_frame_parent_index.push_back(parentFrameIndex);
  r.raw_frame_depth.push_back(depth);
  r.raw_frame_index_in_parent.push_back(indexInParent);
  r.raw_frame_byte_offset_in_top.push_back(byteOffset);
  r.raw_frame_type.push_back(static_cast<UShort_t>(frame->GetFrameType()));
  r.raw_frame_data_source.push_back(
      static_cast<UShort_t>(frame->GetDataSource()));
  r.raw_frame_revision.push_back(static_cast<UChar_t>(frame->GetRevision()));
  r.raw_frame_size.push_back(static_cast<UInt_t>(std::max(0, frame->GetFrameSize())));
  r.raw_frame_header_size.push_back(static_cast<UInt_t>(std::max(0, frame->GetHeaderSize())));
  r.raw_frame_unit_block_size.push_back(static_cast<UInt_t>(std::max(0, frame->GetUnitBlockSize())));
  r.raw_frame_nb_items.push_back(-1);
  r.raw_frame_event_number.push_back(frame->GetEventNumber());
  r.raw_frame_timestamp.push_back(frame->GetTimeStamp());
  r.raw_frame_merge_delta_time.push_back(0);

  const UShort_t type = static_cast<UShort_t>(frame->GetFrameType());
  if (IsMergeType(type)) {
    MFMMergeFrame merge;
    merge.SetAttributs(frame->GetPointHeader());
    RefreshCurrentFrameLayout(&merge, r);
    const Int_t nbItems = merge.GetNbItems();
    r.raw_frame_nb_items.back() = nbItems;
    r.raw_frame_merge_delta_time.back() = merge.GetDeltaTime();

    if (!unfoldMerge) return;
    if (nbItems < 0 || nbItems > 10000000) {
      std::cerr << "Refuse invalid merge item count / 拒绝异常 merge item 数: "
                << nbItems << " at frame index " << frameIndex << std::endl;
      return;
    }

    merge.ResetReadInMem();
    for (Int_t i = 0; i < nbItems; ++i) {
      MFMCommonFrame child;
      merge.ReadInFrame(&child);
      CaptureFrame(&child, r, depth + 1, i, frameIndex,
                   topBegin, topSize, unfoldMerge);
    }
    return;
  }

  switch (type) {
    case MFM_EXO2_FRAME_TYPE: {
      MFMExogamFrame exo;
      exo.SetAttributs(frame->GetPointHeader());
      RefreshCurrentFrameLayout(&exo, r);
      r.raw_exo_frame_index.push_back(frameIndex);
      r.raw_exo_cristal_id.push_back(exo.ExoGetCristalId());
      r.raw_exo_status1.push_back(exo.ExoGetStatus(0));
      r.raw_exo_status2.push_back(exo.ExoGetStatus(1));
      r.raw_exo_status3.push_back(exo.ExoGetStatus(2));
      r.raw_exo_delta_t.push_back(exo.ExoGetDeltaT());
      r.raw_exo_inner_m6.push_back(exo.ExoGetInnerM(0));
      r.raw_exo_inner_m20.push_back(exo.ExoGetInnerM(1));
      r.raw_exo_outer1.push_back(exo.ExoGetOuter(0));
      r.raw_exo_outer2.push_back(exo.ExoGetOuter(1));
      r.raw_exo_outer3.push_back(exo.ExoGetOuter(2));
      r.raw_exo_outer4.push_back(exo.ExoGetOuter(3));
      r.raw_exo_bgo.push_back(exo.ExoGetBGO());
      r.raw_exo_csi.push_back(exo.ExoGetCsi());
      r.raw_exo_inner_t30.push_back(exo.ExoGetInnerT(0));
      r.raw_exo_inner_t60.push_back(exo.ExoGetInnerT(1));
      r.raw_exo_inner_t90.push_back(exo.ExoGetInnerT(2));
      r.raw_exo_padding.push_back(exo.ExoGetPadding());
      break;
    }

    case MFM_DIAMANT_FRAME_TYPE: {
      MFMDiamantFrame diamant;
      diamant.SetAttributs(frame->GetPointHeader());
      RefreshCurrentFrameLayout(&diamant, r);
      r.raw_diamant_frame_index.push_back(frameIndex);
      r.raw_diamant_cristal_id.push_back(diamant.GetCristalId());
      r.raw_diamant_status1.push_back(static_cast<UChar_t>(diamant.GetStatus(0)));
      r.raw_diamant_status2.push_back(static_cast<UChar_t>(diamant.GetStatus(1)));
      r.raw_diamant_energy.push_back(diamant.GetEnergy());
      r.raw_diamant_top.push_back(diamant.GetTop());
      r.raw_diamant_checksum.push_back(diamant.GetChecksum());
      break;
    }

    case MFM_NEDA_FRAME_TYPE: {
      MFMNedaFrame neda;
      neda.SetAttributs(frame->GetPointHeader());
      RefreshCurrentFrameLayout(&neda, r);
      r.raw_neda_frame_index.push_back(frameIndex);
      r.raw_neda_location_id.push_back(neda.GetLocationId());
      r.raw_neda_le_interval.push_back(neda.GetLeInterval());
      r.raw_neda_zco_interval.push_back(neda.GetZcoInterval());
      r.raw_neda_tdc_value.push_back(neda.GetTdcValue());

      // Offsets 32/36/43 are the packed Free0/Free1/Free2 fields in the
      // fixed 44-byte NEDA header. MFMlib has no public getters for them.
      // 32/36/43 是固定 44-byte NEDA header 中 Free0/Free1/Free2 的偏移；
      // MFMlib 没有对应 public getter，因此按 frame byte order 直接读取。
      r.raw_neda_free0.push_back(ReadU16At(frame, 32));
      r.raw_neda_slow_integral.push_back(neda.GetSlowIntegral());
      r.raw_neda_free1.push_back(ReadU16At(frame, 36));
      r.raw_neda_fast_integral.push_back(neda.GetFastIntegral());
      r.raw_neda_bitfield.push_back(neda.GetBitfield());
      r.raw_neda_abs_max.push_back(neda.GetAbsMax());
      r.raw_neda_interpol_cfd.push_back(neda.GetInterpolCFD());
      r.raw_neda_free2.push_back(ReadU8At(frame, 43));

      const Int_t itemCount = std::max(0, neda.GetNbItems());
      const Int_t itemSize = std::max(0, neda.GetItemSize());
      const std::size_t itemStart =
          static_cast<std::size_t>(std::max(0, neda.GetHeaderSize()));
      r.raw_neda_sample_offset.push_back(
          static_cast<UInt_t>(r.raw_neda_sample_word.size()));
      r.raw_neda_sample_count.push_back(static_cast<UShort_t>(
          std::min(itemCount, static_cast<Int_t>(std::numeric_limits<UShort_t>::max()))));
      for (Int_t i = 0; i < itemCount; ++i) {
        r.raw_neda_sample_word.push_back(
            ReadU16At(frame, itemStart + static_cast<std::size_t>(i) * itemSize));
      }
      r.raw_neda_end_of_frame.push_back(
          ReadU32At(frame, itemStart + static_cast<std::size_t>(itemCount) * itemSize));
      break;
    }

    case MFM_NEDACOMP_FRAME_TYPE: {
      MFMNedaCompFrame neda;
      neda.SetAttributs(frame->GetPointHeader());
      RefreshCurrentFrameLayout(&neda, r);
      r.raw_neda_comp_frame_index.push_back(frameIndex);
      r.raw_neda_comp_location_id.push_back(neda.GetLocationId());
      r.raw_neda_comp_energy.push_back(neda.GetEnergy());
      r.raw_neda_comp_time.push_back(neda.GetTime());
      r.raw_neda_comp_tdc_cor_value.push_back(neda.GetTdcCorValue());
      r.raw_neda_comp_slow_integral.push_back(neda.GetSlowIntegral());
      r.raw_neda_comp_fast_integral.push_back(neda.GetFastIntegral());
      r.raw_neda_comp_int_raise_time.push_back(neda.GetIntRaiseTime());
      r.raw_neda_comp_neural_network.push_back(neda.GetNeuralNetWork());
      r.raw_neda_comp_nb_zero.push_back(neda.GetNbZero());
      // The last byte is stored, rather than the bool-returning getter, so no
      // nonzero bit pattern is collapsed to 1.
      // 保存 frame 最后一个原始 byte，而不是 bool getter，避免把非零 bit pattern 压成 1。
      r.raw_neda_comp_neutron_flag.push_back(
          ReadU8At(frame, static_cast<std::size_t>(std::max(1, frame->GetFrameSize())) - 1));
      break;
    }

    case MFM_PARIS_FRAME_TYPE: {
      MFMParisFrame paris;
      paris.SetAttributs(frame->GetPointHeader());
      RefreshCurrentFrameLayout(&paris, r);
      const std::size_t dataOffset =
          static_cast<std::size_t>(std::max(0, paris.GetHeaderSize()));
      r.raw_paris_frame_index.push_back(frameIndex);
      r.raw_paris_location_id.push_back(paris.GetCristalId());
      r.raw_paris_qshort.push_back(paris.GetQShort());
      r.raw_paris_qlong.push_back(paris.GetQLong());
      r.raw_paris_cfd_word.push_back(ReadU24At(frame, dataOffset + 6));
      r.raw_paris_flags.push_back(ReadU8At(frame, dataOffset + 9));
      break;
    }

    case MFM_REA_GENE_FRAME_TYPE: {
      MFMReaGenericFrame generic;
      generic.SetAttributs(frame->GetPointHeader());
      RefreshCurrentFrameLayout(&generic, r);
      r.raw_generic_frame_index.push_back(frameIndex);
      r.raw_generic_cristal_id.push_back(generic.GetCristalId());
      r.raw_generic_status1.push_back(generic.GetStatus(0));
      r.raw_generic_status2.push_back(generic.GetStatus(1));
      r.raw_generic_type_tns.push_back(
          static_cast<UShort_t>(generic.GetTypeTns()));
      r.raw_generic_energy.push_back(generic.GetEnergy());
      r.raw_generic_time.push_back(generic.GetTime());
      r.raw_generic_checksum.push_back(generic.GetChecksum());
      break;
    }

    case MFM_VAMOSIC_FRAME_TYPE: {
      MFMVamosICFrame vamos;
      vamos.SetAttributs(frame->GetPointHeader());
      RefreshCurrentFrameLayout(&vamos, r);
      const std::size_t dataOffset =
          static_cast<std::size_t>(std::max(0, vamos.GetHeaderSize()));
      r.raw_vamos_frame_index.push_back(frameIndex);
      r.raw_vamos_cristal_id.push_back(vamos.GetCristalId());
      r.raw_vamos_status1.push_back(vamos.GetStatus(0));
      r.raw_vamos_status2.push_back(vamos.GetStatus(1));
      r.raw_vamos_not_used1.push_back(ReadU16At(frame, dataOffset + 6));
      r.raw_vamos_energy.push_back(vamos.GetEnergy());
      r.raw_vamos_not_used2.push_back(ReadU16At(frame, dataOffset + 10));
      r.raw_vamos_checksum.push_back(vamos.GetChecksum());
      break;
    }

    default: {
      if (IsEbyedatType(type)) {
        MFMEbyedatFrame ebye;
        ebye.SetAttributs(frame->GetPointHeader());
        RefreshCurrentFrameLayout(&ebye, r);
        r.raw_frame_nb_items.back() = ebye.GetNbItems();
        for (Int_t i = 0; i < ebye.GetNbItems(); ++i) {
          UShort_t label = 0;
          UShort_t value = 0;
          ebye.EbyedatGetParameters(i, &label, &value);
          r.raw_ebye_frame_index.push_back(frameIndex);
          r.raw_ebye_label.push_back(label);
          r.raw_ebye_value.push_back(value);
        }
      } else if (!HasTypedPayloadDecoder(type)) {
        CaptureOpaqueFrame(frame, frameIndex, r);
      }
      break;
    }
  }
}

struct RawInputRow {
  UInt_t input_file_index = 0;
  std::string input_path;
  ULong64_t top_events_seen = 0;
  ULong64_t events_written = 0;
  ULong64_t bytes_read = 0;
  // 0=normal/processed, 1=not opened because maxEvents was reached,
  // negative=read/open error.
  // 0=正常/已处理，1=达到 maxEvents 因而未打开，负数=读取/打开错误。
  Int_t read_status = 0;
};

void MakeInputBranches(TTree &tree, RawInputRow &r)
{
  tree.Branch("input_file_index", &r.input_file_index);
  tree.Branch("input_path", &r.input_path);
  tree.Branch("top_events_seen", &r.top_events_seen);
  tree.Branch("events_written", &r.events_written);
  tree.Branch("bytes_read", &r.bytes_read);
  tree.Branch("read_status", &r.read_status);
}

} // namespace mfm_raw_event_detail

void mfm_to_raw_event_tree(const char *inputSpec,
                           Long64_t maxEvents = -1,
                           const char *outputFile = "",
                           Long64_t startEvent = 0,
                           bool unfoldMerge = true,
                           Long64_t progressEvery = 100000)
{
  using namespace mfm_raw_event_detail;

  if (startEvent < 0) {
    std::cerr << "startEvent must be >= 0 / startEvent 必须 >= 0" << std::endl;
    return;
  }

  std::vector<TString> inputFiles;
  TString normalizedSpec;
  if (!ResolveInputs(inputSpec, inputFiles, normalizedSpec)) return;
  const bool inputWasList = LooksLikeList(TString(inputSpec));

  TString outputName = (outputFile && TString(outputFile).Length() > 0)
                           ? TString(outputFile)
                           : DefaultOutputName(normalizedSpec, inputWasList);

  TFile output(outputName, "RECREATE");
  if (!output.IsOpen() || output.IsZombie()) {
    std::cerr << "Cannot create output ROOT / 无法创建输出 ROOT: "
              << outputName << std::endl;
    return;
  }

  TTree eventTree("RawEventTree",
                  "One raw ROOT entry per top-level MFM event");
  TTree inputTree("RawInputTree",
                  "Input-file manifest and raw conversion counters");
  RawEventRow event;
  RawInputRow input;
  MakeEventBranches(eventTree, event);
  MakeInputBranches(inputTree, input);

  // Flush baskets periodically so a long conversion does not retain all data
  // in memory. This changes only ROOT I/O buffering, never event content.
  // 定期 flush ROOT basket，避免长任务把全部数据留在内存；只影响 I/O 缓冲，
  // 不改变任何 event 内容。
  eventTree.SetAutoFlush(-50000000);

  ULong64_t globalTopSeen = 0;
  ULong64_t globalWritten = 0;
  ULong64_t globalBytesRead = 0;
  bool reachedLimit = false;

  for (std::size_t fileIndex = 0; fileIndex < inputFiles.size(); ++fileIndex) {
    input = RawInputRow();
    input.input_file_index = static_cast<UInt_t>(fileIndex);
    input.input_path = inputFiles[fileIndex].Data();

    // Keep every requested list item in RawInputTree, even when a global
    // maxEvents limit prevents later files from being opened.
    // 即使全局 maxEvents 使后续文件不再打开，也把每个清单项保留在 RawInputTree。
    if (reachedLimit ||
        (maxEvents >= 0 &&
         globalWritten >= static_cast<ULong64_t>(maxEvents))) {
      reachedLimit = true;
      input.read_status = 1;
      inputTree.Fill();
      continue;
    }

    int fd = open(inputFiles[fileIndex].Data(), O_RDONLY);
    if (fd < 0) {
      input.read_status = -errno;
      inputTree.Fill();
      std::cerr << "Cannot open MFM input / 无法打开 MFM 输入: "
                << inputFiles[fileIndex] << " errno=" << errno << std::endl;
      continue;
    }

    std::cout << "Input " << (fileIndex + 1) << "/" << inputFiles.size()
              << " / 输入: " << inputFiles[fileIndex] << std::endl;

    MFMCommonFrame topFrame;
    ULong64_t fileEventIndex = 0;
    while (true) {
      if (maxEvents >= 0 &&
          globalWritten >= static_cast<ULong64_t>(maxEvents)) {
        reachedLimit = true;
        break;
      }

      const Long64_t fileOffset = static_cast<Long64_t>(lseek(fd, 0, SEEK_CUR));
      const Int_t readSize = topFrame.ReadInFile(&fd);
      if (readSize == 0) break;
      if (readSize < 0) {
        input.read_status = readSize;
        std::cerr << "MFM read stopped with error / MFM 读取因错误停止: file="
                  << inputFiles[fileIndex] << " event=" << fileEventIndex
                  << " status=" << readSize << std::endl;
        break;
      }

      input.bytes_read += static_cast<ULong64_t>(readSize);
      globalBytesRead += static_cast<ULong64_t>(readSize);
      ++input.top_events_seen;
      ++globalTopSeen;

      if (globalTopSeen <= static_cast<ULong64_t>(startEvent)) {
        ++fileEventIndex;
        continue;
      }

      topFrame.SetAttributs();
      event.ClearVectors();
      event.event_index = globalWritten;
      event.input_file_index = static_cast<UInt_t>(fileIndex);
      event.input_event_index = fileEventIndex;
      event.input_file_offset = fileOffset;
      event.input_read_size = readSize;
      event.raw_top_frame_type = static_cast<UShort_t>(topFrame.GetFrameType());
      event.raw_top_data_source =
          static_cast<UShort_t>(topFrame.GetDataSource());
      event.raw_top_revision = static_cast<UChar_t>(topFrame.GetRevision());
      event.raw_top_frame_size =
          static_cast<UInt_t>(std::max(0, topFrame.GetFrameSize()));
      event.raw_top_header_size =
          static_cast<UInt_t>(std::max(0, topFrame.GetHeaderSize()));
      event.raw_top_unit_block_size =
          static_cast<UInt_t>(std::max(0, topFrame.GetUnitBlockSize()));
      event.raw_top_nb_items = -1;
      event.raw_top_event_number = topFrame.GetEventNumber();
      event.raw_top_timestamp = topFrame.GetTimeStamp();
      event.raw_top_merge_delta_time = 0;

      if (IsMergeType(event.raw_top_frame_type)) {
        MFMMergeFrame merge;
        merge.SetAttributs(topFrame.GetPointHeader());
        event.raw_top_nb_items = merge.GetNbItems();
        event.raw_top_merge_delta_time = merge.GetDeltaTime();
      }

      const UChar_t *topBegin =
          reinterpret_cast<const UChar_t *>(topFrame.GetPointHeader());
      CaptureFrame(&topFrame, event, 0, 0, -1, topBegin,
                   static_cast<std::size_t>(event.raw_top_frame_size),
                   unfoldMerge);

      // The first recursively stored frame is the top frame. Copy its
      // concrete-class layout back to the scalar top-frame fields.
      // 递归表第 0 行就是 top frame；将具体类识别出的真实布局回填到标量字段。
      if (!event.raw_frame_size.empty()) {
        event.raw_top_frame_size = event.raw_frame_size.front();
        event.raw_top_header_size = event.raw_frame_header_size.front();
        event.raw_top_unit_block_size = event.raw_frame_unit_block_size.front();
        event.raw_top_nb_items = event.raw_frame_nb_items.front();
        event.raw_top_merge_delta_time =
            event.raw_frame_merge_delta_time.front();
      }
      eventTree.Fill();

      ++input.events_written;
      ++globalWritten;
      ++fileEventIndex;

      if (progressEvery > 0 &&
          globalWritten % static_cast<ULong64_t>(progressEvery) == 0) {
        std::cout << "Written / 已写入: " << globalWritten
                  << " raw events" << std::endl;
      }
    }

    close(fd);
    inputTree.Fill();
  }

  const TString configTitle = TString::Format(
      "input_spec=%s; input_files=%zu; start_event=%lld; max_events=%lld; "
      "unfold_merge=%d; events_written=%llu; top_events_seen=%llu; "
      "bytes_read=%llu; raw_only=1",
      inputSpec ? inputSpec : "", inputFiles.size(), startEvent, maxEvents,
      unfoldMerge ? 1 : 0,
      static_cast<unsigned long long>(globalWritten),
      static_cast<unsigned long long>(globalTopSeen),
      static_cast<unsigned long long>(globalBytesRead));
  TNamed config(TString("mfm_to_raw_event_tree_config"), configTitle);

  output.cd();
  config.Write();
  inputTree.Write();
  eventTree.Write();
  output.Close();

  std::cout << "Output / 输出: " << outputName << std::endl;
  std::cout << "Input files / 输入文件数: " << inputFiles.size() << std::endl;
  std::cout << "Top events seen / 读到的 top event: " << globalTopSeen << std::endl;
  std::cout << "Raw events written / 写出的 raw event: " << globalWritten << std::endl;
  std::cout << "Bytes read / 读取字节: " << globalBytesRead << std::endl;
}
