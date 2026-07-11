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

R__ADD_INCLUDE_PATH($MFMSYS/include)
R__LOAD_LIBRARY($MFMSYS/lib/libMFM.so)

#include <MFMCommonFrame.h>
#include <MFMExogamFrame.h>
#include <MFMMergeFrame.h>
#include <MFMTypes.h>

#include <TCanvas.h>
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

constexpr Long64_t kMaxHistogramBins = 1000000;
constexpr Long64_t kMaxSavedFrameRows = 500000;

struct FrameRow {
  Long64_t topEventIndex = -1;
  Long64_t selectedEventIndex = -1;
  Int_t depth = 0;
  Int_t indexInParent = 0;
  UShort_t frameType = 0;
  UShort_t dataSource = 0;
  UInt_t eventNumber = 0;
  ULong64_t timeStamp = 0;
  Double_t relTimeNs = 0.0;
  Int_t frameSize = 0;
  Int_t headerSize = 0;
  Int_t nbItems = -1;
  Int_t boardId = -1;
  Int_t channelId = -1;
};

TString DefaultOutputName(const char *inputMfmFile)
{
  TString out(inputMfmFile ? inputMfmFile : "mfm");
  const Ssiz_t slash = out.Last('/');
  if (slash != kNPOS) out = out(slash + 1, out.Length() - slash - 1);
  out += "_unmerged_mfm_ts.root";
  return out;
}

bool IsMergeFrame(UShort_t frameType)
{
  return frameType == MFM_MERGE_EN_FRAME_TYPE || frameType == MFM_MERGE_TS_FRAME_TYPE;
}

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

TH1D *MakeReadOrderDeltaHistogram(const std::vector<ULong64_t> &ticks,
                                  const char *name,
                                  const char *title,
                                  Double_t tsTickNs,
                                  Double_t requestedBinWidthNs)
{
  std::vector<Double_t> deltas;
  if (ticks.size() >= 2) {
    deltas.reserve(ticks.size() - 1);
    for (std::size_t i = 1; i < ticks.size(); ++i) {
      const Long64_t current = static_cast<Long64_t>(ticks[i]);
      const Long64_t previous = static_cast<Long64_t>(ticks[i - 1]);
      deltas.push_back(static_cast<Double_t>(current - previous) * tsTickNs);
    }
  }
  return MakeSignedValueHistogram(deltas, name, title, requestedBinWidthNs);
}

void SaveCanvas(TH1 *hist, const char *drawOpt = "hist")
{
  if (!hist) return;
  TCanvas canvas(TString::Format("c_%s", hist->GetName()), hist->GetTitle(), 1100, 750);
  hist->Draw(drawOpt);
  canvas.Write();
}

void CollectFrame(MFMCommonFrame *frame,
                  Long64_t topEventIndex,
                  Long64_t selectedEventIndex,
                  Int_t depth,
                  Int_t indexInParent,
                  bool unfoldMerge,
                  std::vector<ULong64_t> &allFrameTs,
                  std::vector<ULong64_t> &exo2FrameTs,
                  std::vector<FrameRow> &rows,
                  std::map<UShort_t, Long64_t> &typeCounts,
                  std::map<UShort_t, Long64_t> &zeroTsCounts)
{
  if (!frame) return;
  frame->SetAttributs();

  const UShort_t frameType = frame->GetFrameType();
  const ULong64_t ts = frame->GetTimeStamp();
  typeCounts[frameType]++;
  if (ts == 0) zeroTsCounts[frameType]++;
  if (ts > 0) allFrameTs.push_back(ts);
  if (frameType == MFM_EXO2_FRAME_TYPE && ts > 0) exo2FrameTs.push_back(ts);

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
    row.boardId = frame->GetBoardId();
    row.channelId = frame->GetChannelId();
    rows.push_back(row);
  }

  if (!unfoldMerge || !IsMergeFrame(frameType)) return;

  MFMMergeFrame mergeFrame;
  mergeFrame.SetAttributs(frame->GetPointHeader());
  mergeFrame.ResetReadInMem();
  for (Int_t i = 0; i < mergeFrame.GetNbItems(); ++i) {
    MFMCommonFrame innerFrame;
    mergeFrame.ReadInFrame(&innerFrame);
    CollectFrame(&innerFrame, topEventIndex, selectedEventIndex, depth + 1, i,
                 unfoldMerge, allFrameTs, exo2FrameTs, rows, typeCounts, zeroTsCounts);
  }
}

} // namespace

void draw_unmerged_mfm_ts_distribution(const char *inputMfmFile,
                                       Long64_t startEvent = 0,
                                       Long64_t maxEvents = 100000,
                                       Double_t binWidthNs = 10.0,
                                       const char *outputFile = "",
                                       bool unfoldMerge = true,
                                       Double_t tsTickNs = 10.0)
{
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

  const int fd = open(inputMfmFile, O_RDONLY);
  if (fd < 0) {
    std::cerr << "Cannot open MFM file / 无法打开 MFM 文件: " << inputMfmFile << std::endl;
    return;
  }

  std::vector<ULong64_t> topEventTs;
  std::vector<ULong64_t> allFrameTs;
  std::vector<ULong64_t> exo2FrameTs;
  std::vector<FrameRow> frameRows;
  std::map<UShort_t, Long64_t> typeCounts;
  std::map<UShort_t, Long64_t> zeroTsCounts;

  MFMCommonFrame topFrame;
  Long64_t topEventIndex = 0;
  Long64_t selectedEvents = 0;
  Long64_t bytesRead = 0;

  while (true) {
    Int_t readSize = topFrame.ReadInFile(const_cast<int *>(&fd));
    if (readSize <= 0) break;
    bytesRead += readSize;

    if (topEventIndex >= startEvent) {
      if (maxEvents >= 0 && selectedEvents >= maxEvents) break;
      topFrame.SetAttributs();
      const ULong64_t topTs = topFrame.GetTimeStamp();
      if (topTs > 0) topEventTs.push_back(topTs);
      CollectFrame(&topFrame, topEventIndex, selectedEvents, 0, 0, unfoldMerge,
                   allFrameTs, exo2FrameTs, frameRows, typeCounts, zeroTsCounts);
      selectedEvents++;
    }
    topEventIndex++;
  }
  close(fd);

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

  TString outName = (outputFile && TString(outputFile).Length() > 0) ? TString(outputFile) : DefaultOutputName(inputMfmFile);
  TFile out(outName, "RECREATE");
  if (!out.IsOpen()) {
    std::cerr << "Cannot create output ROOT / 无法创建输出 ROOT: " << outName << std::endl;
    return;
  }

  TNamed config("unmerged_mfm_ts_config",
                TString::Format("input=%s; start_event=%lld; max_events=%lld; selected_events=%lld; scanned_top_events=%lld; bytes_read=%lld; unfold_merge=%d; ts_tick_ns=%.12g; bin_width_ns=%.12g; base_ts=%llu; saved_frame_rows=%lld",
                                inputMfmFile, startEvent, maxEvents, selectedEvents, topEventIndex,
                                bytesRead, unfoldMerge ? 1 : 0, tsTickNs, binWidthNs,
                                static_cast<unsigned long long>(baseTs),
                                static_cast<Long64_t>(frameRows.size())).Data());
  config.Write();

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
  TH1D *hAllReadOrderDt = MakeReadOrderDeltaHistogram(
      allFrameTs,
      "mfm_all_frame_delta_ts_read_order",
      "ADNE-like Delta TS between consecutive frames in read order;#DeltaT in read order (ns);Pairs / bin",
      tsTickNs, binWidthNs);
  TH1D *hExoReadOrderDt = MakeReadOrderDeltaHistogram(
      exo2FrameTs,
      "mfm_exo2_frame_delta_ts_read_order",
      "ADNE-like Delta TS between consecutive EXO2 frames in read order;#DeltaT in read order (ns);Pairs / bin",
      tsTickNs, binWidthNs);

  hTop->Write();
  hAll->Write();
  hExo->Write();
  hTopDt->Write();
  hAllDt->Write();
  hExoDt->Write();
  hAllReadOrderDt->Write();
  hExoReadOrderDt->Write();

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

  SaveCanvas(hTop);
  SaveCanvas(hAll);
  SaveCanvas(hExo);
  SaveCanvas(hTopDt);
  SaveCanvas(hAllDt);
  SaveCanvas(hExoDt);
  SaveCanvas(hAllReadOrderDt);
  SaveCanvas(hExoReadOrderDt);

  out.Close();

  std::cout << "Input: " << inputMfmFile << std::endl;
  std::cout << "Start top event: " << startEvent << std::endl;
  std::cout << "Selected top events: " << selectedEvents << std::endl;
  std::cout << "Scanned top events: " << topEventIndex << std::endl;
  std::cout << "Base TS: " << baseTs << " ticks" << std::endl;
  std::cout << "Top TS entries: " << topTimesNs.size() << std::endl;
  std::cout << "All frame TS entries: " << allFrameTimesNs.size() << std::endl;
  std::cout << "EXO2 frame TS entries: " << exo2FrameTimesNs.size() << std::endl;
  std::cout << "Saved frame-table rows: " << frameRows.size() << std::endl;
  std::cout << "Output: " << outName << std::endl;
}
