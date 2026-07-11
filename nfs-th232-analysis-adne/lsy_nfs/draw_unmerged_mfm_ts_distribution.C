// Draw timestamp distributions directly from an unmerged MFM data file.
// 直接从未 merged 的 MFM 原始数据文件绘制 TS 事件分布。
//
// Usage / 用法:
//   cd /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne
//   source /home/user0/work/IJCLAB/NFS/NFS_env.sh
//   root -l -b -q 'lsy_nfs/draw_unmerged_mfm_ts_distribution.C("data/run_0023.dat.25-09-23_14h32m42s.rawtest_64MiB",0,5000,10)'
//
// Arguments / 参数:
//   inputMfmFile, startEvent, maxEvents, binWidthNs, outputFile, unfoldMerge, tsTickNs
//   inputMfmFile : raw/unmerged MFM file / 原始未 merged MFM 文件
//   startEvent   : first top-level MFM frame/event to analyse / 起始顶层 MFM event 编号
//   maxEvents    : number of top-level events to analyse; <0 means full file / 读取事件数；<0 表示全文件
//   binWidthNs   : requested time-axis bin width / 时间轴 bin 宽
//   outputFile   : ROOT output path; empty gives <input>_unmerged_mfm_ts.root / 输出 ROOT 文件名
//   unfoldMerge  : recursively unfold MFM merge frames / 是否递归展开 merge frame
//   tsTickNs     : MFM TS tick length in ns; ADNE/NFS convention is 10 ns / TS tick 长度，ADNE/NFS 中为 10 ns
//
// Notes / 说明:
//   - The reader intentionally uses MFMlib only, not the full ADNE detector unpacking.
//     该脚本只使用 MFMlib，不调用完整 ADNE 探测器解包。
//   - A top-level event here means one top-level MFM frame read from the file.
//     这里的顶层 event 指从文件中连续读到的一个顶层 MFM frame。
//   - If a top-level frame is a merge frame, inner frames are unfolded in the same
//     way as ADNE's GUser::CaptureRawFrame() recursion.
//     如果顶层 frame 是 merge frame，则按 ADNE 的 GUser::CaptureRawFrame() 思路递归展开内部 frame。
//   - Sorted Delta-TS histograms sort TS before differencing, so all intervals are positive.
//     排序 Delta-TS 图会先对 TS 排序再相邻相减，因此间隔全为正值。
//   - Read-order Delta-TS histograms keep the original file/unfolding order and may contain negative intervals.
//     未排序 Delta-TS 图保留原始文件/展开读取顺序，因此可能包含负间隔。
//   - Per-crystal EXO2 plots are grouped by raw NUMEXO board id and trigger/crystal channel id.
//     每个 crystal 的 EXO2 图按原始 NUMEXO board id 和 trigger/crystal channel id 分组。

R__ADD_INCLUDE_PATH($MFMSYS/include)
R__LOAD_LIBRARY($MFMSYS/lib/libMFM.so)

#include <MFMCommonFrame.h>
#include <MFMExogamFrame.h>
#include <MFMMergeFrame.h>
#include <MFMTypes.h>

#include <TCanvas.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH1I.h>
#include <TNamed.h>
#include <TString.h>
#include <TTree.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <unistd.h>
#include <vector>

namespace {

// 为了避免长时间运行的原始数据产生过多 bin，时间轴直方图最多保留 1e6 个 bin。
// 如果时间跨度太大，脚本会自动放宽实际 bin 宽，而不是创建超大直方图。
constexpr Long64_t kMaxHistogramBins = 1000000;

// frame table 主要用于抽查前面一段数据结构，不建议把超大文件所有 frame 都写入表中。
constexpr Long64_t kMaxSavedFrameRows = 500000;

// 保存到 mfm_frame_table 的单个 frame 信息。
// 这里的 frame 包括顶层 frame，也包括 merge frame 内部递归展开得到的子 frame。
struct FrameRow {
  Long64_t topEventIndex = -1;      // 原始文件中的顶层 MFM event/frame 序号，从 0 开始。
  Long64_t selectedEventIndex = -1; // 本次分析选中的第几个顶层 event，从 startEvent 后重新编号。
  Int_t depth = 0;                  // frame 嵌套深度：0 是顶层 frame，1 是 merge 内部第一层子 frame。
  Int_t indexInParent = 0;          // 当前 frame 在父 merge frame 中的序号。
  UShort_t frameType = 0;           // MFM frame type，例如 0x10 是 EXO2，0xff02 是 merge TS。
  UShort_t dataSource = 0;          // MFM data source 字段。
  UInt_t eventNumber = 0;           // frame 里保存的 event number；TS merge frame 可能不使用该量。
  ULong64_t timeStamp = 0;          // MFM 原始绝对 TS，单位是 tick。
  Double_t relTimeNs = 0.0;         // 相对第一个非零 TS 的时间，单位 ns。
  Int_t frameSize = 0;              // 当前 frame 字节数，用来检查异常/损坏 frame。
  Int_t headerSize = 0;             // 当前 frame header 字节数。
  Int_t nbItems = -1;               // 如果是 merge frame，这里记录其内部 item/frame 数；否则为 -1。
  Int_t boardId = -1;               // MFMlib 可解析出的 board id；无法解析时为 -1。
  Int_t channelId = -1;             // MFMlib 可解析出的 channel id；无法解析时为 -1。
};

// 根据输入 MFM 文件名生成默认输出 ROOT 文件名。
TString DefaultOutputName(const char *inputMfmFile)
{
  TString out(inputMfmFile ? inputMfmFile : "mfm");
  const Ssiz_t slash = out.Last('/');
  if (slash != kNPOS) out = out(slash + 1, out.Length() - slash - 1);
  out += "_unmerged_mfm_ts.root";
  return out;
}

// 判断一个 frame 是否是 merge frame。
// merge frame 本身是一个容器，里面还包含多个真实探测器 frame，需要递归展开。
bool IsMergeFrame(UShort_t frameType)
{
  return frameType == MFM_MERGE_EN_FRAME_TYPE || frameType == MFM_MERGE_TS_FRAME_TYPE;
}

// 给常见 MFM frame type 一个可读名字，用于 frame type 计数图的轴标签。
const char *FrameTypeName(UShort_t frameType)
{
  switch (frameType) {
    case MFM_EXO2_FRAME_TYPE: return "EXO2";
    case MFM_MERGE_EN_FRAME_TYPE: return "MERGE_EN";
    case MFM_MERGE_TS_FRAME_TYPE: return "MERGE_TS";
    case MFM_HELLO_FRAME_TYPE: return "HELLO";
    case MFM_NEDA_FRAME_TYPE: return "NEDA";
    case MFM_NEDACOMP_FRAME_TYPE: return "NEDACOMP";
    case MFM_DIAMANT_FRAME_TYPE: return "DIAMANT";
    case MFM_PARIS_FRAME_TYPE: return "PARIS";
    case MFM_REA_GENE_FRAME_TYPE: return "REA_GENERIC";
    case MFM_VAMOSIC_FRAME_TYPE: return "VAMOSIC";
    default: return "OTHER";
  }
}

// 用 board id 和 trigger/crystal channel id 组成一个稳定 key。
// 这里不使用 ADNE LUT，因此该编号是原始 MFM/NUMEXO 层面的 crystal 标识。
Int_t CrystalKey(Int_t boardId, Int_t channelId)
{
  return boardId * 1000 + channelId;
}

Int_t CrystalBoardFromKey(Int_t key)
{
  return key / 1000;
}

Int_t CrystalChannelFromKey(Int_t key)
{
  return key % 1000;
}

TString CrystalTag(Int_t key)
{
  return TString::Format("board%03d_crystal%02d", CrystalBoardFromKey(key), CrystalChannelFromKey(key));
}

TString CrystalLabel(Int_t key)
{
  return TString::Format("board %d crystal %d", CrystalBoardFromKey(key), CrystalChannelFromKey(key));
}

// 选择时间零点：优先使用顶层 event 的第一个非零 TS；如果顶层 TS 都为 0，
// 再从所有展开后的 frame TS 中寻找第一个非零 TS。
ULong64_t FirstNonZeroTs(const std::vector<ULong64_t> &a, const std::vector<ULong64_t> &b)
{
  for (ULong64_t ts : a) {
    if (ts > 0) return ts;
  }
  for (ULong64_t ts : b) {
    if (ts > 0) return ts;
  }
  return 0;
}

// 把绝对 TS tick 转为相对时间 ns。
// 只保留非零且不早于 baseTs 的 TS；损坏/无效 TS 不进入时间谱。
std::vector<Double_t> RelativeTimesNs(const std::vector<ULong64_t> &ticks,
                                      ULong64_t baseTs,
                                      Double_t tsTickNs)
{
  std::vector<Double_t> out;
  out.reserve(ticks.size());
  for (ULong64_t ts : ticks) {
    if (ts == 0 || baseTs == 0 || ts < baseTs) continue;
    out.push_back(static_cast<Double_t>(ts - baseTs) * tsTickNs);
  }
  return out;
}

// 根据时间数组创建一维时间分布图。
// 如果时间跨度很大，为避免百万以上的 bin 导致 ROOT 文件巨大，会自动限制最大 bin 数。
TH1D *MakeTimeHistogram(const std::vector<Double_t> &timesNs,
                        const char *name,
                        const char *title,
                        Double_t requestedBinWidthNs)
{
  Double_t maxTime = requestedBinWidthNs;
  for (Double_t t : timesNs) {
    if (std::isfinite(t) && t > maxTime) maxTime = t;
  }
  Long64_t requestedBins = std::max<Long64_t>(1, static_cast<Long64_t>(std::ceil(maxTime / requestedBinWidthNs)));
  Long64_t nbins = std::min<Long64_t>(requestedBins, kMaxHistogramBins);
  Double_t actualBinWidthNs = requestedBinWidthNs;
  if (requestedBins > kMaxHistogramBins) actualBinWidthNs = maxTime / static_cast<Double_t>(nbins);
  Double_t xMax = nbins * actualBinWidthNs;
  if (xMax <= maxTime) xMax = std::nextafter(maxTime, std::numeric_limits<Double_t>::infinity());
  if (xMax <= 0.0) xMax = requestedBinWidthNs;

  TH1D *hist = new TH1D(name, title, static_cast<Int_t>(nbins), 0.0, xMax);
  hist->SetDirectory(nullptr);
  for (Double_t t : timesNs) {
    if (std::isfinite(t) && t >= 0.0) hist->Fill(t);
  }
  return hist;
}

// 按“时间大小排序”后计算相邻 TS 差分。
// 这个图用于观察整体时间间隔分布，但它不是 ADNE 在线谱的填充方式。
TH1D *MakeDeltaHistogram(const std::vector<Double_t> &timesNs,
                         const char *name,
                         const char *title,
                         Double_t requestedBinWidthNs)
{
  if (timesNs.size() < 2) {
    TH1D *empty = new TH1D(name, title, 1, 0.0, requestedBinWidthNs);
    empty->SetDirectory(nullptr);
    return empty;
  }

  std::vector<Double_t> sorted = timesNs;
  std::sort(sorted.begin(), sorted.end());
  std::vector<Double_t> deltas;
  deltas.reserve(sorted.size() - 1);
  for (std::size_t i = 1; i < sorted.size(); ++i) {
    const Double_t dt = sorted[i] - sorted[i - 1];
    if (dt >= 0.0 && std::isfinite(dt)) deltas.push_back(dt);
  }
  return MakeTimeHistogram(deltas, name, title, requestedBinWidthNs);
}

// 为可能带负值的差分量创建直方图。
// 按读取顺序做 TS 差时，如果原始文件不是严格时间排序，差分可能为负，因此不能只从 0 开始画。
TH1D *MakeSignedValueHistogram(const std::vector<Double_t> &values,
                              const char *name,
                              const char *title,
                              Double_t requestedBinWidthNs)
{
  if (values.empty()) {
    TH1D *empty = new TH1D(name, title, 1, -0.5 * requestedBinWidthNs, 0.5 * requestedBinWidthNs);
    empty->SetDirectory(nullptr);
    return empty;
  }

  Double_t minValue = std::numeric_limits<Double_t>::infinity();
  Double_t maxValue = -std::numeric_limits<Double_t>::infinity();
  for (Double_t value : values) {
    if (!std::isfinite(value)) continue;
    if (value < minValue) minValue = value;
    if (value > maxValue) maxValue = value;
  }
  if (!std::isfinite(minValue) || !std::isfinite(maxValue)) {
    minValue = -0.5 * requestedBinWidthNs;
    maxValue = 0.5 * requestedBinWidthNs;
  }

  minValue = std::floor(minValue / requestedBinWidthNs) * requestedBinWidthNs;
  maxValue = std::ceil(maxValue / requestedBinWidthNs) * requestedBinWidthNs;
  if (maxValue <= minValue) maxValue = minValue + requestedBinWidthNs;

  Long64_t requestedBins = std::max<Long64_t>(1, static_cast<Long64_t>(std::ceil((maxValue - minValue) / requestedBinWidthNs)));
  Long64_t nbins = std::min<Long64_t>(requestedBins, kMaxHistogramBins);
  if (requestedBins > kMaxHistogramBins) {
    const Double_t center = 0.5 * (minValue + maxValue);
    const Double_t halfWidth = 0.5 * requestedBinWidthNs * static_cast<Double_t>(kMaxHistogramBins);
    minValue = center - halfWidth;
    maxValue = center + halfWidth;
  }

  maxValue = std::nextafter(maxValue, std::numeric_limits<Double_t>::infinity());
  TH1D *hist = new TH1D(name, title, static_cast<Int_t>(nbins), minValue, maxValue);
  hist->SetDirectory(nullptr);
  for (Double_t value : values) {
    if (std::isfinite(value)) hist->Fill(value);
  }
  return hist;
}

// 先对原始 TS tick 排序，再计算相邻 frame 的 TS 差分。
// 这样得到的间隔全部为正，适合检查束流周期或 frame 间隔分布。
TH1D *MakeSortedTickDeltaHistogram(const std::vector<ULong64_t> &ticks,
                                   const char *name,
                                   const char *title,
                                   Double_t tsTickNs,
                                   Double_t requestedBinWidthNs)
{
  std::vector<ULong64_t> sortedTicks;
  sortedTicks.reserve(ticks.size());
  for (ULong64_t tick : ticks) {
    if (tick > 0) sortedTicks.push_back(tick);
  }
  std::sort(sortedTicks.begin(), sortedTicks.end());

  std::vector<Double_t> deltas;
  if (sortedTicks.size() >= 2) {
    deltas.reserve(sortedTicks.size() - 1);
    for (std::size_t i = 1; i < sortedTicks.size(); ++i) {
      deltas.push_back(static_cast<Double_t>(sortedTicks[i] - sortedTicks[i - 1]) * tsTickNs);
    }
  }
  return MakeSignedValueHistogram(deltas, name, title, requestedBinWidthNs);
}

// 按原始读取/展开顺序计算相邻 TS 差分。
// 这个图保留文件中的顺序信息；如果 frame 顺序不是严格按时间排列，差分可能为负。
TH1D *MakeReadOrderTickDeltaHistogram(const std::vector<ULong64_t> &ticks,
                                      const char *name,
                                      const char *title,
                                      Double_t tsTickNs,
                                      Double_t requestedBinWidthNs)
{
  std::vector<Double_t> deltas;
  ULong64_t previousTick = 0;
  bool hasPrevious = false;
  for (ULong64_t tick : ticks) {
    if (tick == 0) continue;
    if (hasPrevious) {
      const Long64_t current = static_cast<Long64_t>(tick);
      const Long64_t previous = static_cast<Long64_t>(previousTick);
      deltas.push_back(static_cast<Double_t>(current - previous) * tsTickNs);
    }
    previousTick = tick;
    hasPrevious = true;
  }
  return MakeSignedValueHistogram(deltas, name, title, requestedBinWidthNs);
}

// 给每张关键直方图额外保存一个 canvas，方便直接打开 ROOT 文件浏览。
void SaveCanvas(TH1 *hist, const char *drawOpt = "hist")
{
  if (!hist) return;
  TCanvas canvas(TString::Format("c_%s", hist->GetName()), hist->GetTitle(), 1100, 750);
  hist->Draw(drawOpt);
  canvas.Write();
}

// 递归收集一个 MFM frame 的信息。
// 若当前 frame 是 merge frame，并且 unfoldMerge=true，则继续读取其内部子 frame。
// 这样可以同时得到顶层 frame 时间、所有子 frame 时间，以及 EXO2 frame 时间。
void CollectFrame(MFMCommonFrame *frame,
                  Long64_t topEventIndex,
                  Long64_t selectedEventIndex,
                  Int_t depth,
                  Int_t indexInParent,
                  bool unfoldMerge,
                  std::vector<ULong64_t> &allFrameTs,
                  std::vector<ULong64_t> &exo2FrameTs,
                  std::map<Int_t, std::vector<ULong64_t>> &exo2CrystalTs,
                  std::vector<FrameRow> &rows,
                  std::map<UShort_t, Long64_t> &typeCounts,
                  std::map<UShort_t, Long64_t> &zeroTsCounts)
{
  if (!frame) return;

  // SetAttributs 会让 MFMlib 从当前原始字节中解析 frame type、size、TS、event number 等字段。
  frame->SetAttributs();

  const UShort_t frameType = frame->GetFrameType();
  const ULong64_t ts = frame->GetTimeStamp();
  typeCounts[frameType]++;
  if (ts == 0) zeroTsCounts[frameType]++;
  if (ts > 0) allFrameTs.push_back(ts);

  Int_t parsedBoardId = frame->GetBoardId();
  Int_t parsedChannelId = frame->GetChannelId();
  if (frameType == MFM_EXO2_FRAME_TYPE) {
    // EXO2 frame 的 CristalId 字段同时包含 NUMEXO board id 和 trigger/crystal channel id。
    // ADNE 后续会用 LUT 把它映射到 clover/crystal；这里保持原始 board-channel 分组。
    MFMExogamFrame exoFrame;
    exoFrame.SetAttributs(frame->GetPointHeader());
    parsedBoardId = exoFrame.ExoGetBoardId();
    parsedChannelId = exoFrame.ExoGetTGCristalId();

    if (ts > 0) {
      exo2FrameTs.push_back(ts);
      exo2CrystalTs[CrystalKey(parsedBoardId, parsedChannelId)].push_back(ts);
    }
  }

  Int_t nbItems = -1;
  if (IsMergeFrame(frameType)) {
    MFMMergeFrame mergeFrame;
    mergeFrame.SetAttributs(frame->GetPointHeader());
    nbItems = mergeFrame.GetNbItems();
  }

  if (static_cast<Long64_t>(rows.size()) < kMaxSavedFrameRows) {
    FrameRow row;
    row.topEventIndex = topEventIndex;
    row.selectedEventIndex = selectedEventIndex;
    row.depth = depth;
    row.indexInParent = indexInParent;
    row.frameType = frameType;
    row.dataSource = frame->GetDataSource();
    row.eventNumber = frame->GetEventNumber();
    row.timeStamp = ts;
    row.frameSize = frame->GetFrameSize();
    row.headerSize = frame->GetHeaderSize();
    row.nbItems = nbItems;
    row.boardId = parsedBoardId;
    row.channelId = parsedChannelId;
    rows.push_back(row);
  }

  if (!unfoldMerge || !IsMergeFrame(frameType)) return;

  // merge frame 内部是多个连续存放的子 frame。ResetReadInMem 后，ReadInFrame 会逐个取出。
  MFMMergeFrame mergeFrame;
  mergeFrame.SetAttributs(frame->GetPointHeader());
  mergeFrame.ResetReadInMem();
  for (Int_t i = 0; i < mergeFrame.GetNbItems(); ++i) {
    MFMCommonFrame innerFrame;
    mergeFrame.ReadInFrame(&innerFrame);
    CollectFrame(&innerFrame, topEventIndex, selectedEventIndex, depth + 1, i,
                 unfoldMerge, allFrameTs, exo2FrameTs, exo2CrystalTs, rows, typeCounts, zeroTsCounts);
  }
}

} // namespace

// 主入口函数。
// 注意：输入必须是原始 MFM/dat 文件，不是 ADNE 生成的 ROOT 文件。
void draw_unmerged_mfm_ts_distribution(const char *inputMfmFile,
                                       Long64_t startEvent = 0,
                                       Long64_t maxEvents = 100000,
                                       Double_t binWidthNs = 10.0,
                                       const char *outputFile = "",
                                       bool unfoldMerge = true,
                                       Double_t tsTickNs = 10.0)
{
  // 1. 参数检查。
  if (!inputMfmFile || TString(inputMfmFile).IsNull()) {
    std::cerr << "Input MFM file is empty / 输入 MFM 文件名为空" << std::endl;
    return;
  }
  if (startEvent < 0) {
    std::cerr << "startEvent must be >= 0 / startEvent 必须 >= 0" << std::endl;
    return;
  }
  if (binWidthNs <= 0.0 || tsTickNs <= 0.0) {
    std::cerr << "binWidthNs and tsTickNs must be positive / binWidthNs 和 tsTickNs 必须为正" << std::endl;
    return;
  }

  // 2. 以只读方式打开原始 MFM 文件。脚本不会修改输入文件。
  const int fd = open(inputMfmFile, O_RDONLY);
  if (fd < 0) {
    std::cerr << "Cannot open MFM file / 无法打开 MFM 文件: " << inputMfmFile << std::endl;
    return;
  }

  // 3. 保存不同层级的 TS。
  // topEventTs: 顶层 frame/event 的 TS。
  // allFrameTs: 顶层 frame 加上递归展开的所有子 frame 的 TS。
  // exo2FrameTs: 只保留 EXO2 frame 的 TS，用来对照 ADNE 的 Exogam 时间谱。
  std::vector<ULong64_t> topEventTs;
  std::vector<ULong64_t> allFrameTs;
  std::vector<ULong64_t> exo2FrameTs;
  std::map<Int_t, std::vector<ULong64_t>> exo2CrystalTs;
  std::vector<FrameRow> frameRows;
  std::map<UShort_t, Long64_t> typeCounts;
  std::map<UShort_t, Long64_t> zeroTsCounts;

  MFMCommonFrame topFrame;
  Long64_t topEventIndex = 0;
  Long64_t selectedEvents = 0;
  Long64_t bytesRead = 0;

  // 4. 顺序读取顶层 MFM frame。
  // ReadInFile 每次从文件当前位置读取一个完整 MFM frame，并由 MFMlib 解析 header。
  while (true) {
    Int_t readSize = topFrame.ReadInFile(const_cast<int *>(&fd));
    if (readSize <= 0) break;
    bytesRead += readSize;

    // 跳过 startEvent 之前的顶层 event，从指定位置开始统计。
    if (topEventIndex >= startEvent) {
      if (maxEvents >= 0 && selectedEvents >= maxEvents) break;
      topFrame.SetAttributs();
      const ULong64_t topTs = topFrame.GetTimeStamp();
      if (topTs > 0) topEventTs.push_back(topTs);
      CollectFrame(&topFrame, topEventIndex, selectedEvents, 0, 0, unfoldMerge,
                   allFrameTs, exo2FrameTs, exo2CrystalTs, frameRows, typeCounts, zeroTsCounts);
      selectedEvents++;
    }
    topEventIndex++;
  }
  close(fd);

  // 5. 选取第一个非零 TS 作为时间零点，把绝对 TS 转为相对 ns。
  const ULong64_t baseTs = FirstNonZeroTs(topEventTs, allFrameTs);
  std::vector<Double_t> topTimesNs = RelativeTimesNs(topEventTs, baseTs, tsTickNs);
  std::vector<Double_t> allFrameTimesNs = RelativeTimesNs(allFrameTs, baseTs, tsTickNs);
  std::vector<Double_t> exo2FrameTimesNs = RelativeTimesNs(exo2FrameTs, baseTs, tsTickNs);

  for (auto &row : frameRows) {
    if (baseTs > 0 && row.timeStamp >= baseTs) {
      row.relTimeNs = static_cast<Double_t>(row.timeStamp - baseTs) * tsTickNs;
    } else {
      row.relTimeNs = -1.0;
    }
  }

  // 6. 创建输出 ROOT 文件。
  TString outName = (outputFile && TString(outputFile).Length() > 0) ? TString(outputFile) : DefaultOutputName(inputMfmFile);
  TFile out(outName, "RECREATE");
  if (!out.IsOpen()) {
    std::cerr << "Cannot create output ROOT / 无法创建输出 ROOT: " << outName << std::endl;
    return;
  }

  // 7. 写入配置记录，方便之后确认该 ROOT 文件由哪个输入和参数生成。
  TNamed config("unmerged_mfm_ts_config",
                TString::Format("input=%s; start_event=%lld; max_events=%lld; selected_events=%lld; scanned_top_events=%lld; bytes_read=%lld; unfold_merge=%d; ts_tick_ns=%.12g; bin_width_ns=%.12g; base_ts=%llu; saved_frame_rows=%lld",
                                inputMfmFile, startEvent, maxEvents, selectedEvents, topEventIndex,
                                bytesRead, unfoldMerge ? 1 : 0, tsTickNs, binWidthNs,
                                static_cast<unsigned long long>(baseTs),
                                static_cast<Long64_t>(frameRows.size())).Data());
  config.Write();

  // 8. 时间分布图：横轴是相对第一个非零 TS 的时间。
  TH1D *hTop = MakeTimeHistogram(topTimesNs,
                                 "mfm_top_event_ts_distribution",
                                 "Top-level MFM event TS distribution;Time from first selected TS (ns);Top events / bin",
                                 binWidthNs);
  TH1D *hAll = MakeTimeHistogram(allFrameTimesNs,
                                 "mfm_all_frame_ts_distribution",
                                 "All MFM frame TS distribution;Time from first selected TS (ns);Frames / bin",
                                 binWidthNs);
  TH1D *hExo = MakeTimeHistogram(exo2FrameTimesNs,
                                 "mfm_exo2_frame_ts_distribution",
                                 "EXO2 frame TS distribution;Time from first selected TS (ns);EXO2 frames / bin",
                                 binWidthNs);
  // 9. 按 TS 排序后的相邻间隔分布。
  // 这些图回答“所有时间点之间的间隔分布是什么”，但不保留读取顺序。
  TH1D *hTopDt = MakeDeltaHistogram(topTimesNs,
                                    "mfm_top_event_delta_ts",
                                    "Delta TS between consecutive top-level events;#DeltaT (ns);Pairs / bin",
                                    binWidthNs);
  TH1D *hAllDt = MakeDeltaHistogram(allFrameTimesNs,
                                    "mfm_all_frame_delta_ts",
                                    "Delta TS between consecutive frames;#DeltaT (ns);Pairs / bin",
                                    binWidthNs);
  TH1D *hExoDt = MakeDeltaHistogram(exo2FrameTimesNs,
                                    "mfm_exo2_frame_delta_ts",
                                    "Delta TS between consecutive EXO2 frames;#DeltaT sorted by TS (ns);Pairs / bin",
                                    binWidthNs);
  // 10. 按原始读取/展开顺序计算相邻 frame 间隔。
  // 这保留文件中的顺序信息；如果原始 frame 没有按 TS 严格排序，图中可能出现负值。
  TH1D *hTopReadOrderDt = MakeReadOrderTickDeltaHistogram(
      topEventTs,
      "mfm_top_event_delta_ts_read_order",
      "Read-order Delta TS between consecutive top-level events;#DeltaT in read order (ns);Pairs / bin",
      tsTickNs, binWidthNs);
  TH1D *hAllReadOrderDt = MakeReadOrderTickDeltaHistogram(
      allFrameTs,
      "mfm_all_frame_delta_ts_read_order",
      "Read-order Delta TS between consecutive frames;#DeltaT in read order (ns);Pairs / bin",
      tsTickNs, binWidthNs);
  TH1D *hExoReadOrderDt = MakeReadOrderTickDeltaHistogram(
      exo2FrameTs,
      "mfm_exo2_frame_delta_ts_read_order",
      "Read-order Delta TS between consecutive EXO2 frames;#DeltaT in read order (ns);Pairs / bin",
      tsTickNs, binWidthNs);

  hTop->Write();
  hAll->Write();
  hExo->Write();
  hTopDt->Write();
  hAllDt->Write();
  hExoDt->Write();
  hTopReadOrderDt->Write();
  hAllReadOrderDt->Write();
  hExoReadOrderDt->Write();

  // 11. frame type 计数和 TS=0 的 frame type 计数。
  // 这可以快速检查文件里是否混有非 EXO2 frame、hello frame、merge frame 或异常 zero-TS frame。
  TH1I hType("mfm_frame_type_counts", "MFM frame type counts;Frame type;Frames", 65536, 0, 65536);
  TH1I hZero("mfm_zero_ts_frame_type_counts", "MFM zero-TS frame type counts;Frame type;Frames with TS=0", 65536, 0, 65536);
  for (const auto &kv : typeCounts) {
    hType.SetBinContent(kv.first + 1, kv.second);
    hType.GetXaxis()->SetBinLabel(kv.first + 1, FrameTypeName(kv.first));
  }
  Long64_t totalTypeEntries = 0;
  Long64_t totalZeroEntries = 0;
  for (const auto &kv : typeCounts) totalTypeEntries += kv.second;
  for (const auto &kv : zeroTsCounts) {
    hZero.SetBinContent(kv.first + 1, kv.second);
    hZero.GetXaxis()->SetBinLabel(kv.first + 1, FrameTypeName(kv.first));
    totalZeroEntries += kv.second;
  }
  hType.SetEntries(totalTypeEntries);
  hZero.SetEntries(totalZeroEntries);
  hType.Write();
  hZero.Write();

  // 12. 每个 EXO2 crystal 的 TS 分布和相邻 TS 间隔分布。
  // 这些图按原始 board-channel 分组，方便在不加载 ADNE LUT 的情况下定位某个 crystal 的异常计数率或时间间隔。
  TDirectory *crystalDir = out.mkdir("exo2_crystal_ts");
  if (crystalDir) {
    crystalDir->cd();
    TH1D hCrystalCounts("mfm_exo2_crystal_frame_counts",
                        "EXO2 crystal frame counts;Raw EXO2 crystal;Frames",
                        std::max<Int_t>(1, static_cast<Int_t>(exo2CrystalTs.size())),
                        0.5, std::max<Double_t>(1.5, static_cast<Double_t>(exo2CrystalTs.size()) + 0.5));

    Int_t bin = 1;
    for (const auto &kv : exo2CrystalTs) {
      const Int_t key = kv.first;
      const TString tag = CrystalTag(key);
      const TString label = CrystalLabel(key);
      hCrystalCounts.GetXaxis()->SetBinLabel(bin, label.Data());
      hCrystalCounts.SetBinContent(bin, static_cast<Double_t>(kv.second.size()));
      bin++;

      std::vector<Double_t> crystalTimesNs = RelativeTimesNs(kv.second, baseTs, tsTickNs);
      TH1D *hCrystalTs = MakeTimeHistogram(
          crystalTimesNs,
          TString::Format("mfm_exo2_%s_ts_distribution", tag.Data()),
          TString::Format("EXO2 %s TS distribution;Time from first selected TS (ns);Frames / bin", label.Data()),
          binWidthNs);
      TH1D *hCrystalDelta = MakeSortedTickDeltaHistogram(
          kv.second,
          TString::Format("mfm_exo2_%s_delta_ts_sorted", tag.Data()),
          TString::Format("EXO2 %s sorted Delta TS;#DeltaT after TS sorting (ns);Pairs / bin", label.Data()),
          tsTickNs, binWidthNs);
      TH1D *hCrystalReadOrderDelta = MakeReadOrderTickDeltaHistogram(
          kv.second,
          TString::Format("mfm_exo2_%s_delta_ts_read_order", tag.Data()),
          TString::Format("EXO2 %s read-order Delta TS;#DeltaT in read order (ns);Pairs / bin", label.Data()),
          tsTickNs, binWidthNs);

      hCrystalTs->Write();
      hCrystalDelta->Write();
      hCrystalReadOrderDelta->Write();
      SaveCanvas(hCrystalTs);
      SaveCanvas(hCrystalDelta);
      SaveCanvas(hCrystalReadOrderDelta);
    }
    hCrystalCounts.SetEntries(exo2FrameTs.size());
    hCrystalCounts.Write();
    SaveCanvas(&hCrystalCounts);
    out.cd();
  }

  // 13. 写出 frame table。
  // 它不是后续大规模分析的主数据，而是用来抽查原始 MFM 结构和定位坏 frame。
  TTree frameTable("mfm_frame_table", "Unmerged MFM frame table");
  FrameRow row;
  frameTable.Branch("top_event_index", &row.topEventIndex);
  frameTable.Branch("selected_event_index", &row.selectedEventIndex);
  frameTable.Branch("depth", &row.depth);
  frameTable.Branch("index_in_parent", &row.indexInParent);
  frameTable.Branch("frame_type", &row.frameType);
  frameTable.Branch("data_source", &row.dataSource);
  frameTable.Branch("event_number", &row.eventNumber);
  frameTable.Branch("timestamp", &row.timeStamp);
  frameTable.Branch("rel_time_ns", &row.relTimeNs);
  frameTable.Branch("frame_size", &row.frameSize);
  frameTable.Branch("header_size", &row.headerSize);
  frameTable.Branch("nb_items", &row.nbItems);
  frameTable.Branch("board_id", &row.boardId);
  frameTable.Branch("channel_id", &row.channelId);
  for (const auto &saved : frameRows) {
    row = saved;
    frameTable.Fill();
  }
  frameTable.Write();

  // 14. 保存 canvas，方便用 TBrowser 或 ROOT GUI 直接查看。
  SaveCanvas(hTop);
  SaveCanvas(hAll);
  SaveCanvas(hExo);
  SaveCanvas(hTopDt);
  SaveCanvas(hAllDt);
  SaveCanvas(hExoDt);
  SaveCanvas(hTopReadOrderDt);
  SaveCanvas(hAllReadOrderDt);
  SaveCanvas(hExoReadOrderDt);

  out.Close();

  // 15. 终端打印简要统计，便于批处理日志中快速检查。
  std::cout << "Input: " << inputMfmFile << std::endl;
  std::cout << "Start top event: " << startEvent << std::endl;
  std::cout << "Selected top events: " << selectedEvents << std::endl;
  std::cout << "Scanned top events: " << topEventIndex << std::endl;
  std::cout << "Base TS: " << baseTs << " ticks" << std::endl;
  std::cout << "Top TS entries: " << topTimesNs.size() << std::endl;
  std::cout << "All frame TS entries: " << allFrameTimesNs.size() << std::endl;
  std::cout << "EXO2 frame TS entries: " << exo2FrameTimesNs.size() << std::endl;
  std::cout << "EXO2 crystal groups: " << exo2CrystalTs.size() << std::endl;
  std::cout << "Saved frame-table rows: " << frameRows.size() << std::endl;
  std::cout << "Output: " << outName << std::endl;
}
