// Convert MFM binary frames to a raw ROOT tree using only ROOT + MFMlib.
// 只使用 ROOT + MFMlib，把 MFM 二进制 frame 转换成 raw ROOT tree。
//
// Why this macro exists / 为什么需要这个宏:
//   ADNE is excellent for detector analysis, but it also applies lookup tables,
//   thresholds, calibration, event treatment, and NFS-specific reconstruction.
//   When a file looks damaged, or when raw/unmerged and merged files need to be
//   compared, we need a lower-level reader that answers a simpler question:
//   "What frames are physically present in this binary file?"
//
//   ADNE 很适合做探测器分析，但它会进一步使用 lookup table、阈值、刻度、event
//   treatment 和 NFS 重建逻辑。当文件疑似损坏，或者需要比较 raw/unmerged 与 merged
//   文件时，我们需要一个更底层的 reader，只回答一个问题：这个二进制文件里到底有
//   哪些 MFM frame？
//
// Usage / 用法:
//   cd /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne
//   source /home/user0/work/IJCLAB/NFS/NFS_env.sh
//   root -l -b -q 'lsy_nfs/mfm_to_raw_root_tree.C("data/run_0023.dat.25-09-23_14h32m42s.rawtest_64MiB",2000,"out/mfm_raw_tree.root")'
//
// Full interface / 完整接口:
//   mfm_to_raw_root_tree(inputMfmFile,
//                        maxTopEvents,
//                        outputFile,
//                        unfoldMerge,
//                        maxPrintTopEvents,
//                        tsTickNs,
//                        maxStoredTopBytes)
//
// Parameters / 参数:
//   inputMfmFile       : MFM binary file. It can be raw/unmerged or already merged.
//                        MFM 二进制文件，可以是 raw/unmerged，也可以是 merged。
//   maxTopEvents       : number of top-level frames to read. <0 means full file.
//                        读取多少个顶层 frame；<0 表示全文件。
//   outputFile         : output ROOT file. Empty string gives <input>_mfm_raw_tree.root.
//                        输出 ROOT 文件；空字符串则自动生成。
//   unfoldMerge        : if true, recursively unfold MFM_MERGE_* frames into child frames.
//                        是否递归展开 MFM_MERGE_* frame 内部的子 frame。
//   maxPrintTopEvents  : print this many top-level events to terminal for human inspection.
//                        在终端打印前几个顶层 event 的结构，便于人工检查。
//   tsTickNs           : timestamp tick length. ADNE/NFS convention is 10 ns/tick.
//                        TS tick 长度；ADNE/NFS 约定为 10 ns/tick。
//   maxStoredTopBytes  : if >0, store at most this many raw bytes for each top frame in MfmTopTree.
//                        若 >0，则在 MfmTopTree 中保存每个顶层 frame 的前 N 个原始字节。
//
// Output ROOT structure / 输出 ROOT 结构:
//   1. MfmTopTree
//      One entry per top-level frame read directly from the file.
//      每个 entry 对应文件中直接读到的一个顶层 frame。
//
//   2. MfmFrameTree
//      One entry per frame after optional recursive merge unfolding.
//      If the input is merged, each top merge frame is an event container and
//      its detector frames appear with depth=1.
//      每个 entry 对应一个展开后的 frame。如果输入是 merged 文件，顶层 merge frame
//      是事件容器，内部探测器 frame 通常 depth=1。
//
// Important definitions / 重要定义:
//   top frame:
//      The outermost frame returned by MFMCommonFrame::ReadInFile(). It is a file-level
//      concept, not a detector concept. In a merged MFM file, this is often a
//      MFM_MERGE_TS_FRAME_TYPE event container.
//      top frame 是 MFMCommonFrame::ReadInFile() 从文件当前位置直接读出的最外层 frame。
//      它是文件层级概念，不是探测器概念。在 merged MFM 文件里，它通常是
//      MFM_MERGE_TS_FRAME_TYPE 事件容器。
//
//   EXO2 DeltaT/TDC:
//      For EXOGAM2 frames, ExoGetDeltaT() is the raw 16-bit TDC-like value saved in
//      the binary. ADNE NFS currently converts it as (65536-DeltaT)*0.024 ns before
//      applying offsets/corrections.
//      对 EXOGAM2 frame，ExoGetDeltaT() 是 binary 中保存的原始 16-bit TDC 类值。
//      ADNE NFS 当前先用 (65536-DeltaT)*0.024 ns 转成翻转后的时间，再做 offset/correction。

R__ADD_INCLUDE_PATH($MFMSYS/include)
R__LOAD_LIBRARY($MFMSYS/lib/libMFM.so)

#include <MFMCommonFrame.h>
#include <MFMExogamFrame.h>
#include <MFMMergeFrame.h>
#include <MFMTypes.h>

#include <TFile.h>
#include <TNamed.h>
#include <TString.h>
#include <TTree.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

namespace {

// ADNE/NFS 当前用于快速 TDC->ns 的默认 gain。
// 注意：这里不是精细 time calibration，只是为了把 raw DeltaT 显示到 ns 量级。
constexpr Double_t kNfsDefaultTdcGainNs = 0.024;
constexpr Double_t kNfsTdcChannels = 65536.0;

// 给常见 MFM frame type 一个可读名字。数字本身仍然保存在 frame_type 分支里。
std::string FrameTypeName(UShort_t frameType)
{
  switch (frameType) {
    case MFM_COMMON_FRAME_TYPE: return "COMMON";
    case MFM_EXO2_FRAME_TYPE: return "EXO2";
    case MFM_NEDA_FRAME_TYPE: return "NEDA";
    case MFM_NEDACOMP_FRAME_TYPE: return "NEDACOMP";
    case MFM_DIAMANT_FRAME_TYPE: return "DIAMANT";
    case MFM_PARIS_FRAME_TYPE: return "PARIS";
    case MFM_REA_GENE_FRAME_TYPE: return "REA_GENERIC";
    case MFM_VAMOSIC_FRAME_TYPE: return "VAMOSIC";
    case MFM_HELLO_FRAME_TYPE: return "HELLO";
    case MFM_MERGE_EN_FRAME_TYPE: return "MERGE_EN";
    case MFM_MERGE_TS_FRAME_TYPE: return "MERGE_TS";
    case MFM_XML_DATA_DESCRIPTION_FRAME_TYPE: return "XML_DESCRIPTION";
    case MFM_XML_FILE_HEADER_FRAME_TYPE: return "XML_FILE_HEADER";
    default: return "OTHER";
  }
}

// 判断一个 frame 是否是 merge/event 容器。
bool IsMergeFrame(UShort_t frameType)
{
  return frameType == MFM_MERGE_EN_FRAME_TYPE || frameType == MFM_MERGE_TS_FRAME_TYPE;
}

// 根据输入文件名生成默认 ROOT 输出路径。
TString DefaultOutputName(const char *inputMfmFile)
{
  TString out(inputMfmFile ? inputMfmFile : "mfm");
  const Ssiz_t slash = out.Last('/');
  if (slash != kNPOS) out = out(slash + 1, out.Length() - slash - 1);
  out += "_mfm_raw_tree.root";
  return out;
}

// 把 EXO2 的 raw DeltaT 翻转成 ns。这个量对应 ADNE NFS 里最基础的 TDC 时间。
Double_t ReversedDeltaTNs(UShort_t rawDeltaT)
{
  return (kNfsTdcChannels - static_cast<Double_t>(rawDeltaT)) * kNfsDefaultTdcGainNs;
}

// 把绝对 TS 折叠到一个 TDC 周期里，再和同一 frame 的 reversed DeltaT 比较。
// 这是诊断量：看 TS 相位和 TDC 是否有稳定关系，不是物理 TOF。
Double_t TsPhaseMinusTdcNs(ULong64_t timeStamp, UShort_t rawDeltaT, Double_t tsTickNs)
{
  if (timeStamp == 0 || tsTickNs <= 0.0) return -999999.0;
  const Double_t tdcPeriodNs = kNfsTdcChannels * kNfsDefaultTdcGainNs;
  const Double_t reversedDeltaT = ReversedDeltaTNs(rawDeltaT);
  const long double tsNs = static_cast<long double>(timeStamp) * static_cast<long double>(tsTickNs);
  Double_t tsPhaseNs = static_cast<Double_t>(std::fmod(tsNs, static_cast<long double>(tdcPeriodNs)));
  if (tsPhaseNs < 0.0) tsPhaseNs += tdcPeriodNs;
  Double_t diffNs = tsPhaseNs - reversedDeltaT;
  while (diffNs > 0.5 * tdcPeriodNs) diffNs -= tdcPeriodNs;
  while (diffNs < -0.5 * tdcPeriodNs) diffNs += tdcPeriodNs;
  return diffNs;
}

// MfmFrameTree 的一行。所有字段都初始化为“无效值”，只有对应 frame 类型存在时才填。
struct FrameRow {
  Long64_t global_frame_index = -1;       // 展开后的全局 frame 行号，从 0 开始。
  Long64_t top_event_index = -1;          // 顶层 frame 序号，从文件开头开始数。
  Long64_t selected_top_index = -1;       // 本次处理选中的顶层 frame 序号，从 0 开始。
  Int_t depth = 0;                        // 0=top frame；1=top merge 内部子 frame；2=嵌套 merge 的更深层。
  Int_t index_in_parent = 0;              // 当前 frame 在父 merge frame 中的序号。
  Long64_t parent_global_frame_index = -1;// 父 frame 的 global_frame_index；top frame 为 -1。
  Long64_t top_file_offset = -1;          // 顶层 frame 在文件中的字节偏移。
  Long64_t byte_offset_in_top = -1;       // 当前 frame 相对顶层 frame 起始位置的字节偏移。
  UShort_t frame_type = 0;                // MFM frame type 数值，例如 EXO2=0x10, MERGE_TS=0xff02。
  std::string frame_type_name;            // frame_type 的可读标签。
  UShort_t data_source = 0;               // MFM common header 里的 data source。
  UShort_t revision = 0;                  // MFM common header 里的 revision。
  Int_t frame_size = 0;                   // 当前 frame 总字节数。
  Int_t header_size = 0;                  // 当前 frame header 字节数。
  Int_t unit_block_size = 0;              // MFM unit block size。
  Int_t nb_items = -1;                    // merge frame 内部 item 数；非 merge 为 -1。
  UInt_t event_number = 0;                // frame event number；MERGE_TS 通常为 0。
  ULong64_t timestamp = 0;                // 原始绝对 timestamp，单位 tick。
  Double_t time_from_first_ns = -1.0;     // 相对第一个非零 TS 的时间，单位 ns。
  Double_t time_from_top_ns = -1.0;       // 相对本 top frame TS 的时间，单位 ns。
  Bool_t is_merge = false;                // 是否 merge/event 容器。
  Bool_t is_exo2 = false;                 // 是否 EXO2 frame。

  Int_t exo_raw_cristal_id = -1;          // EXO2 CristalId 原始 16-bit 复合字段。
  Int_t exo_board_id = -1;                // CristalId 高位解析出的 NUMEXO board id。
  Int_t exo_tg_crystal_id = -1;           // CristalId 低 5 bit，ADNE 后续会压成 half-board 0/1。
  Int_t exo_lut_halfboard = -1;           // ADNE LUT 使用的 halfboard：tg id 为 0 -> 0，否则 -> 1。
  Int_t exo_status1 = -1;                 // EXO2 Status1 原始值。
  Int_t exo_status2 = -1;                 // EXO2 Status2 原始值。
  Int_t exo_status3 = -1;                 // EXO2 Status3 原始值，ADNE 用它判断 GOCCE segment net/mirror。
  Int_t exo_delta_t = -1;                 // EXO2 DeltaT/TDC 原始 16-bit 值。
  Double_t exo_reversed_delta_t_ns = -1.0;// (65536-DeltaT)*0.024 ns。
  Double_t exo_ts_phase_minus_tdc_ns = -999999.0; // TS phase - reversed DeltaT 的循环差。
  Int_t exo_inner6m = -1;                 // Core energy, 6 MeV range；ADNE 用 ECoef 刻度为 ECCE energy。
  Int_t exo_inner20m = -1;                // Core energy, 20 MeV range；当前 NFS 主要不用。
  Int_t exo_outer1 = -1;                  // Segment/outer raw value 1。
  Int_t exo_outer2 = -1;                  // Segment/outer raw value 2。
  Int_t exo_outer3 = -1;                  // Segment/outer raw value 3。
  Int_t exo_outer4 = -1;                  // Segment/outer raw value 4。
  Int_t exo_bgo = -1;                     // EXO2 frame 中保存的 BGO 值。
  Int_t exo_csi = -1;                     // EXO2 frame 中保存的 CsI 值。
  Int_t exo_t30 = -1;                     // PSA/rise-time marker T30。
  Int_t exo_t60 = -1;                     // PSA/rise-time marker T60。
  Int_t exo_t90 = -1;                     // PSA/rise-time marker T90。
  Int_t exo_padding = -1;                 // EXO2 padding 字段。
};

// MfmTopTree 的一行。它描述文件中直接读出的 top frame。
struct TopRow {
  Long64_t top_event_index = -1;          // 文件中的顶层 frame 序号。
  Long64_t selected_top_index = -1;       // 本次处理选中的顶层 frame 序号。
  Long64_t file_offset = -1;              // 顶层 frame 起始字节偏移，可用于定位坏 frame。
  Int_t read_size = 0;                    // ReadInFile 返回的读取字节数。
  UShort_t frame_type = 0;                // 顶层 frame type。
  std::string frame_type_name;            // 顶层 frame type 名称。
  Int_t frame_size = 0;                   // 顶层 frame 大小。
  Int_t nb_items = -1;                    // 若顶层是 merge frame，则为内部 item 数。
  ULong64_t timestamp = 0;                // 顶层 frame TS。
  UInt_t event_number = 0;                // 顶层 frame event number。
  std::vector<UChar_t> stored_top_bytes;  // 可选：顶层 frame 前 N 字节，默认不保存。
};

// 给 MfmFrameTree 创建分支。把这部分集中写，主函数会清楚很多。
void MakeFrameBranches(TTree &tree, FrameRow &r)
{
  tree.Branch("global_frame_index", &r.global_frame_index);
  tree.Branch("top_event_index", &r.top_event_index);
  tree.Branch("selected_top_index", &r.selected_top_index);
  tree.Branch("depth", &r.depth);
  tree.Branch("index_in_parent", &r.index_in_parent);
  tree.Branch("parent_global_frame_index", &r.parent_global_frame_index);
  tree.Branch("top_file_offset", &r.top_file_offset);
  tree.Branch("byte_offset_in_top", &r.byte_offset_in_top);
  tree.Branch("frame_type", &r.frame_type);
  tree.Branch("frame_type_name", &r.frame_type_name);
  tree.Branch("data_source", &r.data_source);
  tree.Branch("revision", &r.revision);
  tree.Branch("frame_size", &r.frame_size);
  tree.Branch("header_size", &r.header_size);
  tree.Branch("unit_block_size", &r.unit_block_size);
  tree.Branch("nb_items", &r.nb_items);
  tree.Branch("event_number", &r.event_number);
  tree.Branch("timestamp", &r.timestamp);
  tree.Branch("time_from_first_ns", &r.time_from_first_ns);
  tree.Branch("time_from_top_ns", &r.time_from_top_ns);
  tree.Branch("is_merge", &r.is_merge);
  tree.Branch("is_exo2", &r.is_exo2);
  tree.Branch("exo_raw_cristal_id", &r.exo_raw_cristal_id);
  tree.Branch("exo_board_id", &r.exo_board_id);
  tree.Branch("exo_tg_crystal_id", &r.exo_tg_crystal_id);
  tree.Branch("exo_lut_halfboard", &r.exo_lut_halfboard);
  tree.Branch("exo_status1", &r.exo_status1);
  tree.Branch("exo_status2", &r.exo_status2);
  tree.Branch("exo_status3", &r.exo_status3);
  tree.Branch("exo_delta_t", &r.exo_delta_t);
  tree.Branch("exo_reversed_delta_t_ns", &r.exo_reversed_delta_t_ns);
  tree.Branch("exo_ts_phase_minus_tdc_ns", &r.exo_ts_phase_minus_tdc_ns);
  tree.Branch("exo_inner6m", &r.exo_inner6m);
  tree.Branch("exo_inner20m", &r.exo_inner20m);
  tree.Branch("exo_outer1", &r.exo_outer1);
  tree.Branch("exo_outer2", &r.exo_outer2);
  tree.Branch("exo_outer3", &r.exo_outer3);
  tree.Branch("exo_outer4", &r.exo_outer4);
  tree.Branch("exo_bgo", &r.exo_bgo);
  tree.Branch("exo_csi", &r.exo_csi);
  tree.Branch("exo_t30", &r.exo_t30);
  tree.Branch("exo_t60", &r.exo_t60);
  tree.Branch("exo_t90", &r.exo_t90);
  tree.Branch("exo_padding", &r.exo_padding);
}

// 给 MfmTopTree 创建分支。
void MakeTopBranches(TTree &tree, TopRow &r)
{
  tree.Branch("top_event_index", &r.top_event_index);
  tree.Branch("selected_top_index", &r.selected_top_index);
  tree.Branch("file_offset", &r.file_offset);
  tree.Branch("read_size", &r.read_size);
  tree.Branch("frame_type", &r.frame_type);
  tree.Branch("frame_type_name", &r.frame_type_name);
  tree.Branch("frame_size", &r.frame_size);
  tree.Branch("nb_items", &r.nb_items);
  tree.Branch("timestamp", &r.timestamp);
  tree.Branch("event_number", &r.event_number);
  tree.Branch("stored_top_bytes", &r.stored_top_bytes);
}

// 打印缩进，帮助终端上看清楚 merge 层级。
std::string Indent(Int_t depth)
{
  return std::string(static_cast<std::size_t>(std::max(0, depth)) * 2, ' ');
}

// 递归填充一个 frame。返回当前 frame 的 global_frame_index。
Long64_t FillFrameRecursive(MFMCommonFrame *frame,
                            TTree &frameTree,
                            FrameRow &row,
                            Long64_t &nextGlobalIndex,
                            Long64_t topEventIndex,
                            Long64_t selectedTopIndex,
                            Int_t depth,
                            Int_t indexInParent,
                            Long64_t parentGlobalIndex,
                            Long64_t topFileOffset,
                            const char *topBasePtr,
                            ULong64_t firstNonZeroTs,
                            ULong64_t topTs,
                            bool unfoldMerge,
                            Double_t tsTickNs,
                            bool printThisTop)
{
  if (!frame) return -1;

  // MFMlib 用 SetAttributs() 从当前 frame 的二进制 header 解析 type/size/TS/event number 等。
  frame->SetAttributs();

  // 每个 frame 行都从默认无效值开始，避免非 EXO2 frame 继承上一个 EXO2 的字段。
  row = FrameRow();
  row.global_frame_index = nextGlobalIndex++;
  row.top_event_index = topEventIndex;
  row.selected_top_index = selectedTopIndex;
  row.depth = depth;
  row.index_in_parent = indexInParent;
  row.parent_global_frame_index = parentGlobalIndex;
  row.top_file_offset = topFileOffset;

  // 如果 frame 指针仍指向顶层 frame 内存，计算它在 top frame 内部的相对字节偏移。
  // 对 MFMlib 的 ReadInMem/ReadInFrame 来说，内部 frame 通常仍指向 merge buffer 内部。
  const char *thisPtr = reinterpret_cast<const char *>(frame->GetPointHeader());
  if (topBasePtr && thisPtr >= topBasePtr) row.byte_offset_in_top = thisPtr - topBasePtr;

  row.frame_type = frame->GetFrameType();
  row.frame_type_name = FrameTypeName(row.frame_type);
  row.data_source = frame->GetDataSource();
  row.revision = frame->GetRevision();
  row.frame_size = frame->GetFrameSize();
  row.header_size = frame->GetHeaderSize();
  row.unit_block_size = frame->GetUnitBlockSize();
  row.event_number = frame->GetEventNumber();
  row.timestamp = frame->GetTimeStamp();
  row.is_merge = IsMergeFrame(row.frame_type);
  row.is_exo2 = (row.frame_type == MFM_EXO2_FRAME_TYPE);

  if (firstNonZeroTs > 0 && row.timestamp >= firstNonZeroTs) row.time_from_first_ns = static_cast<Double_t>(row.timestamp - firstNonZeroTs) * tsTickNs;
  if (topTs > 0 && row.timestamp >= topTs) row.time_from_top_ns = static_cast<Double_t>(row.timestamp - topTs) * tsTickNs;

  if (row.is_merge) {
    MFMMergeFrame mergeFrame;
    mergeFrame.SetAttributs(frame->GetPointHeader());
    row.nb_items = mergeFrame.GetNbItems();
  }

  if (row.is_exo2) {
    MFMExogamFrame exoFrame;
    exoFrame.SetAttributs(frame->GetPointHeader());
    row.exo_raw_cristal_id = exoFrame.ExoGetCristalId();
    row.exo_board_id = exoFrame.ExoGetBoardId();
    row.exo_tg_crystal_id = exoFrame.ExoGetTGCristalId();
    row.exo_lut_halfboard = (row.exo_tg_crystal_id == 0) ? 0 : 1;
    row.exo_status1 = exoFrame.ExoGetStatus(0);
    row.exo_status2 = exoFrame.ExoGetStatus(1);
    row.exo_status3 = exoFrame.ExoGetStatus(2);
    row.exo_delta_t = exoFrame.ExoGetDeltaT();
    row.exo_reversed_delta_t_ns = ReversedDeltaTNs(static_cast<UShort_t>(row.exo_delta_t));
    row.exo_ts_phase_minus_tdc_ns = TsPhaseMinusTdcNs(row.timestamp, static_cast<UShort_t>(row.exo_delta_t), tsTickNs);
    row.exo_inner6m = exoFrame.ExoGetInnerM(0);
    row.exo_inner20m = exoFrame.ExoGetInnerM(1);
    row.exo_outer1 = exoFrame.ExoGetOuter(0);
    row.exo_outer2 = exoFrame.ExoGetOuter(1);
    row.exo_outer3 = exoFrame.ExoGetOuter(2);
    row.exo_outer4 = exoFrame.ExoGetOuter(3);
    row.exo_bgo = exoFrame.ExoGetBGO();
    row.exo_csi = exoFrame.ExoGetCsi();
    row.exo_t30 = exoFrame.ExoGetInnerT(0);
    row.exo_t60 = exoFrame.ExoGetInnerT(1);
    row.exo_t90 = exoFrame.ExoGetInnerT(2);
    row.exo_padding = exoFrame.ExoGetPadding();
  }

  if (printThisTop) {
    std::cout << Indent(depth)
              << "frame#" << row.global_frame_index
              << " depth=" << row.depth
              << " idx=" << row.index_in_parent
              << " type=0x" << std::hex << row.frame_type << std::dec
              << "(" << row.frame_type_name << ")"
              << " size=" << row.frame_size
              << " TS=" << row.timestamp;
    if (row.is_merge) std::cout << " nb_items=" << row.nb_items;
    if (row.is_exo2) {
      std::cout << " EXO2(board=" << row.exo_board_id
                << ", tg=" << row.exo_tg_crystal_id
                << ", half=" << row.exo_lut_halfboard
                << ", dT=" << row.exo_delta_t
                << ", revDTns=" << row.exo_reversed_delta_t_ns
                << ", E6=" << row.exo_inner6m
                << ", BGO=" << row.exo_bgo
                << ", CSI=" << row.exo_csi
                << ")";
    }
    std::cout << std::endl;
  }

  const Long64_t thisGlobalIndex = row.global_frame_index;
  frameTree.Fill();

  if (!unfoldMerge || !row.is_merge) return thisGlobalIndex;

  MFMMergeFrame mergeFrame;
  mergeFrame.SetAttributs(frame->GetPointHeader());
  mergeFrame.ResetReadInMem();
  for (Int_t i = 0; i < mergeFrame.GetNbItems(); ++i) {
    MFMCommonFrame innerFrame;
    mergeFrame.ReadInFrame(&innerFrame);
    FillFrameRecursive(&innerFrame, frameTree, row, nextGlobalIndex,
                       topEventIndex, selectedTopIndex,
                       depth + 1, i, thisGlobalIndex,
                       topFileOffset, topBasePtr,
                       firstNonZeroTs, topTs,
                       unfoldMerge, tsTickNs, printThisTop);
  }
  return thisGlobalIndex;
}

} // namespace

void mfm_to_raw_root_tree(const char *inputMfmFile,
                          Long64_t maxTopEvents = 1000,
                          const char *outputFile = "",
                          bool unfoldMerge = true,
                          Long64_t maxPrintTopEvents = 10,
                          Double_t tsTickNs = 10.0,
                          Long64_t maxStoredTopBytes = 0)
{
  if (!inputMfmFile || TString(inputMfmFile).IsNull()) {
    std::cerr << "Input MFM file is empty / 输入 MFM 文件名为空" << std::endl;
    return;
  }
  if (tsTickNs <= 0.0) {
    std::cerr << "tsTickNs must be positive / tsTickNs 必须为正" << std::endl;
    return;
  }

  const int fd = open(inputMfmFile, O_RDONLY);
  if (fd < 0) {
    std::cerr << "Cannot open MFM file / 无法打开 MFM 文件: " << inputMfmFile << std::endl;
    return;
  }

  TString outName = (outputFile && TString(outputFile).Length() > 0) ? TString(outputFile) : DefaultOutputName(inputMfmFile);
  TFile out(outName, "RECREATE");
  if (!out.IsOpen()) {
    std::cerr << "Cannot create output ROOT / 无法创建输出 ROOT: " << outName << std::endl;
    close(fd);
    return;
  }

  TTree topTree("MfmTopTree", "Top-level MFM frames read directly from binary file");
  TTree frameTree("MfmFrameTree", "MFM frames after optional merge-frame unfolding");
  TopRow topRow;
  FrameRow frameRow;
  MakeTopBranches(topTree, topRow);
  MakeFrameBranches(frameTree, frameRow);

  MFMCommonFrame topFrame;
  Long64_t topEventIndex = 0;
  Long64_t selectedTopIndex = 0;
  Long64_t nextGlobalFrameIndex = 0;
  Long64_t totalBytesRead = 0;
  ULong64_t firstNonZeroTs = 0;

  while (true) {
    if (maxTopEvents >= 0 && selectedTopIndex >= maxTopEvents) break;
    const Long64_t fileOffset = static_cast<Long64_t>(lseek(fd, 0, SEEK_CUR));
    const Int_t readSize = topFrame.ReadInFile(const_cast<int *>(&fd));
    if (readSize <= 0) break;
    totalBytesRead += readSize;

    topFrame.SetAttributs();
    const ULong64_t topTs = topFrame.GetTimeStamp();
    if (firstNonZeroTs == 0 && topTs > 0) firstNonZeroTs = topTs;

    topRow = TopRow();
    topRow.top_event_index = topEventIndex;
    topRow.selected_top_index = selectedTopIndex;
    topRow.file_offset = fileOffset;
    topRow.read_size = readSize;
    topRow.frame_type = topFrame.GetFrameType();
    topRow.frame_type_name = FrameTypeName(topRow.frame_type);
    topRow.frame_size = topFrame.GetFrameSize();
    topRow.timestamp = topTs;
    topRow.event_number = topFrame.GetEventNumber();
    if (IsMergeFrame(topRow.frame_type)) {
      MFMMergeFrame mergeFrame;
      mergeFrame.SetAttributs(topFrame.GetPointHeader());
      topRow.nb_items = mergeFrame.GetNbItems();
    }
    if (maxStoredTopBytes > 0 && topFrame.GetPointHeader() != nullptr && topRow.frame_size > 0) {
      const Long64_t nStore = std::min<Long64_t>(maxStoredTopBytes, topRow.frame_size);
      const UChar_t *begin = reinterpret_cast<const UChar_t *>(topFrame.GetPointHeader());
      topRow.stored_top_bytes.assign(begin, begin + nStore);
    }
    topTree.Fill();

    const bool printThisTop = (maxPrintTopEvents > 0 && selectedTopIndex < maxPrintTopEvents);
    if (printThisTop) {
      std::cout << "top event " << selectedTopIndex
                << " file_index=" << topEventIndex
                << " offset=" << fileOffset
                << " type=0x" << std::hex << topRow.frame_type << std::dec
                << "(" << topRow.frame_type_name << ")"
                << " size=" << topRow.frame_size
                << " TS=" << topRow.timestamp
                << " nb_items=" << topRow.nb_items
                << std::endl;
    }

    FillFrameRecursive(&topFrame, frameTree, frameRow, nextGlobalFrameIndex,
                       topEventIndex, selectedTopIndex,
                       0, 0, -1,
                       fileOffset,
                       reinterpret_cast<const char *>(topFrame.GetPointHeader()),
                       firstNonZeroTs,
                       topTs,
                       unfoldMerge,
                       tsTickNs,
                       printThisTop);

    selectedTopIndex++;
    topEventIndex++;
  }

  close(fd);

  TNamed config("mfm_to_raw_root_tree_config",
                TString::Format("input=%s; max_top_events=%lld; selected_top_events=%lld; total_top_frames_seen=%lld; total_frames_written=%lld; unfold_merge=%d; ts_tick_ns=%.12g; max_print_top_events=%lld; max_stored_top_bytes=%lld; total_bytes_read=%lld",
                                inputMfmFile,
                                maxTopEvents,
                                selectedTopIndex,
                                topEventIndex,
                                nextGlobalFrameIndex,
                                unfoldMerge ? 1 : 0,
                                tsTickNs,
                                maxPrintTopEvents,
                                maxStoredTopBytes,
                                totalBytesRead).Data());
  config.Write();
  topTree.Write();
  frameTree.Write();
  out.Close();

  std::cout << "Input / 输入: " << inputMfmFile << std::endl;
  std::cout << "Output / 输出: " << outName << std::endl;
  std::cout << "Top events written / 写入顶层 event: " << selectedTopIndex << std::endl;
  std::cout << "Frames written / 写入展开 frame: " << nextGlobalFrameIndex << std::endl;
  std::cout << "Bytes read / 读取字节: " << totalBytesRead << std::endl;
}
