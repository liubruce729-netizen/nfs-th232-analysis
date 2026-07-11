// Draw RawTree timestamp axes from ADNE raw-tree output.
// 从 ADNE 的 RawTree 输出绘制原始 MFM event/frame 的时间轴。
//
// Usage / 用法:
//   cd /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne
//   source /home/user0/work/IJCLAB/NFS/NFS_env.sh
//   root -l -b -q 'lsy_nfs/draw_raw_timestamp_axis.C("out/nfs_run_23_r0.root")'
//   root -l -b -q 'lsy_nfs/draw_raw_timestamp_axis.C("out/nfs_run_23_r0.root","RawTree",50000,10,"out/raw_ts.root")'
//   root -l -b -q 'lsy_nfs/draw_raw_timestamp_axis.C("out/nfs_run_23_r0.root","RawTree",5000,10,"out/raw_ts_by_event.root",true)'
//
// Arguments / 参数:
//   inputFile, treeName, timeWindowOrMaxEvents, binWidthNs, outputFile, limitByEventCount
//   第三个参数保持原接口位置不变：
//     limitByEventCount=false 时表示从第一个非零 TS 开始读取的时间窗口(ns)，这是默认模式。
//     limitByEventCount=true  时表示最多读取的 RawTree event 数，保留旧行为。
//
// Notes / 说明:
//   - ADNE stores MFM timestamps as absolute TS ticks.
//     ADNE 保存的是 MFM 绝对时间戳 TS tick。
//   - In this analysis chain, 1 TS tick = 10 ns, inferred from ADNE rate calculation:
//       seconds = (TS - TStart) / 1e8
//     在本分析链中，1 个 TS tick = 10 ns；ADNE 计算 rate 时使用 (TS - TStart) / 1e8 得到秒。
//   - By default, the macro subtracts the first non-zero top timestamp and draws a fixed
//     TS time window, which is the useful view for beam/bunch structure.
//     默认模式下，本脚本减去第一个非零顶层 event TS，并绘制固定 TS 时间窗口；
//     这比固定 event 数更适合看束流/束团结构。
//   - RawTree one entry = one top-level MFM event; frame vectors contain the top frame and
//     all nested frames captured from that event.
//     RawTree 的一个 entry 对应一个顶层 MFM event；frame 向量包含顶层 frame 和其中的子 frame。

#include <TBranch.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TNamed.h>
#include <TGraph.h>
#include <TString.h>
#include <TTree.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

namespace {

constexpr Int_t kMfmExo2FrameType = 0x10;
constexpr double kTsTickToNs = 10.0;
constexpr Long64_t kMaxTimeAxisHistogramBins = 1000000;

TString BuildDefaultRawTimestampOutputName(const char *inputFile)
{
  TString outName(inputFile);
  if (outName.EndsWith(".root")) {
    outName.ReplaceAll(".root", "_raw_timestamp_axis.root");
  } else {
    outName += "_raw_timestamp_axis.root";
  }
  return outName;
}

bool HasBranch(TTree *tree, const char *branchName)
{
  return tree && tree->GetBranch(branchName) != nullptr;
}

TGraph *BuildCumulativeGraph(const std::vector<double> &times,
                             const char *name,
                             const char *title)
{
  // EN: Use a graph rather than a huge 10 ns-bin histogram for sparse event streams.
  // CN: 对稀疏事件流，用图保存累计计数，避免巨大且几乎全空的 10 ns/bin 直方图。
  std::vector<double> sortedTimes = times;
  std::sort(sortedTimes.begin(), sortedTimes.end());

  TGraph *graph = new TGraph(static_cast<Int_t>(sortedTimes.size()));
  graph->SetName(name);
  graph->SetTitle(title);
  for (size_t i = 0; i < sortedTimes.size(); ++i) {
    graph->SetPoint(static_cast<Int_t>(i), sortedTimes[i], static_cast<double>(i + 1));
  }
  return graph;
}

TH1D *BuildDeltaHistogram(const std::vector<double> &times,
                          const char *name,
                          const char *title,
                          double binWidthNs)
{
  // EN: Inter-event spacing is often the more useful ns-scale view for absolute TS.
  // CN: 对绝对 TS 来说，相邻 event 间隔通常比超细计数谱更有诊断意义。
  if (times.size() < 2) {
    TH1D *empty = new TH1D(name, title, 1, 0.0, binWidthNs);
    return empty;
  }

  std::vector<double> sortedTimes = times;
  std::sort(sortedTimes.begin(), sortedTimes.end());

  std::vector<double> deltas;
  deltas.reserve(sortedTimes.size() - 1);
  double maxDelta = 0.0;
  for (size_t i = 1; i < sortedTimes.size(); ++i) {
    const double dt = sortedTimes[i] - sortedTimes[i - 1];
    if (dt < 0.0) continue;
    deltas.push_back(dt);
    if (dt > maxDelta) maxDelta = dt;
  }

  const double requestedBinsDouble = std::floor(maxDelta / binWidthNs) + 1.0;
  Int_t nbins = 1;
  double actualBinWidthNs = binWidthNs;
  if (requestedBinsDouble > static_cast<double>(kMaxTimeAxisHistogramBins)) {
    nbins = static_cast<Int_t>(kMaxTimeAxisHistogramBins);
    actualBinWidthNs = maxDelta / static_cast<double>(nbins);
  } else {
    nbins = std::max(1, static_cast<Int_t>(requestedBinsDouble));
  }
  TH1D *hist = new TH1D(name, title, nbins, 0.0, nbins * actualBinWidthNs);
  for (double dt : deltas) hist->Fill(dt);
  return hist;
}

} // namespace

void draw_raw_timestamp_axis(const char *inputFile,
                             const char *treeName = "RawTree",
                             double timeWindowOrMaxEvents = 50000.0,
                             double binWidthNs = 10.0,
                             const char *outputFile = "",
                             bool limitByEventCount = false)
{
  // EN: Validate user parameters before opening large files.
  // CN: 先检查参数，避免对大文件做无效读取。
  if (timeWindowOrMaxEvents <= 0.0) {
    std::cerr << "timeWindowOrMaxEvents must be positive / timeWindowOrMaxEvents 必须大于 0"
              << std::endl;
    return;
  }
  if (binWidthNs <= 0.0) {
    std::cerr << "binWidthNs must be positive / binWidthNs 必须大于 0" << std::endl;
    return;
  }

  const double requestedTimeWindowNs = timeWindowOrMaxEvents;
  const Long64_t maxEntriesToRead = static_cast<Long64_t>(std::llround(timeWindowOrMaxEvents));

  TFile *fin = TFile::Open(inputFile, "READ");
  if (!fin || fin->IsZombie()) {
    std::cerr << "Cannot open input file / 无法打开输入文件: " << inputFile << std::endl;
    return;
  }

  TTree *tree = dynamic_cast<TTree *>(fin->Get(treeName));
  if (!tree) {
    std::cerr << "Cannot find tree / 找不到 tree: " << treeName << std::endl;
    fin->Close();
    return;
  }

  if (!HasBranch(tree, "raw_top_timestamp")) {
    std::cerr << "Missing branch / 缺少分支: raw_top_timestamp" << std::endl;
    fin->Close();
    return;
  }

  ULong64_t rawTopTimestamp = 0;
  ULong64_t rawEventIndex = 0;
  Int_t rawTopFrameType = -1;
  Int_t rawTopNbItems = -1;
  std::vector<ULong64_t> *rawFrameTimestamp = nullptr;
  std::vector<Int_t> *rawFrameType = nullptr;

  tree->SetBranchAddress("raw_top_timestamp", &rawTopTimestamp);
  if (HasBranch(tree, "raw_event_index")) {
    tree->SetBranchAddress("raw_event_index", &rawEventIndex);
  }
  if (HasBranch(tree, "raw_top_frame_type")) {
    tree->SetBranchAddress("raw_top_frame_type", &rawTopFrameType);
  }
  if (HasBranch(tree, "raw_top_nb_items")) {
    tree->SetBranchAddress("raw_top_nb_items", &rawTopNbItems);
  }

  const bool hasFrameTimestamp = HasBranch(tree, "raw_frame_timestamp");
  const bool hasFrameType = HasBranch(tree, "raw_frame_type");

  if (hasFrameTimestamp) {
    tree->SetBranchAddress("raw_frame_timestamp", &rawFrameTimestamp);
  } else {
    std::cerr << "Warning: raw_frame_timestamp not found; frame histograms will be empty."
              << std::endl;
  }
  if (hasFrameType) {
    tree->SetBranchAddress("raw_frame_type", &rawFrameType);
  } else {
    std::cerr << "Warning: raw_frame_type not found; EXO2-only histogram will be empty."
              << std::endl;
  }

  // EN: First pass: keep relative times either inside a fixed TS window or for N entries.
  // CN: 第一遍：按固定 TS 时间窗口或固定 entry 数保存相对时间。
  std::vector<double> eventTimesNs;
  std::vector<double> frameAllTimesNs;
  std::vector<double> frameExo2TimesNs;

  ULong64_t firstTopTS = 0;
  ULong64_t lastTopTS = 0;
  ULong64_t previousTopTS = 0;
  Long64_t eventsRead = 0;
  Long64_t eventsWithNonZeroTS = 0;
  bool stoppedByTimeWindow = false;
  double maxTimeNs = 0.0;
  ULong64_t minPositiveEventDeltaTicks = std::numeric_limits<ULong64_t>::max();
  ULong64_t maxEventDeltaTicks = 0;

  // EN: Diagnostic tree stores the first raw timestamps exactly as read.
  // CN: 诊断 tree 原样保存前若干个 raw timestamp，方便确认是否真的读到了变化的 TS。
  TTree *debugTree = new TTree("raw_timestamp_axis_debug",
                               "First raw timestamps used by draw_raw_timestamp_axis");
  debugTree->SetDirectory(nullptr);
  Long64_t debugEntry = 0;
  ULong64_t debugRawEventIndex = 0;
  Int_t debugTopFrameType = -1;
  Int_t debugTopNbItems = -1;
  ULong64_t debugTopTimestamp = 0;
  Long64_t debugDeltaFromFirstTicks = 0;
  Long64_t debugDeltaFromPreviousTicks = 0;
  double debugDeltaFromFirstNs = 0.0;
  double debugDeltaFromPreviousNs = 0.0;
  Int_t debugNFrames = 0;
  Int_t debugFirstFrameType = -1;
  Long64_t debugFirstFrameDeltaTicks = 0;
  Int_t debugExo2FrameCount = 0;
  debugTree->Branch("entry", &debugEntry, "entry/L");
  debugTree->Branch("raw_event_index", &debugRawEventIndex, "raw_event_index/l");
  debugTree->Branch("raw_top_frame_type", &debugTopFrameType, "raw_top_frame_type/I");
  debugTree->Branch("raw_top_nb_items", &debugTopNbItems, "raw_top_nb_items/I");
  debugTree->Branch("raw_top_timestamp", &debugTopTimestamp, "raw_top_timestamp/l");
  debugTree->Branch("delta_from_first_ticks", &debugDeltaFromFirstTicks, "delta_from_first_ticks/L");
  debugTree->Branch("delta_from_previous_ticks", &debugDeltaFromPreviousTicks, "delta_from_previous_ticks/L");
  debugTree->Branch("delta_from_first_ns", &debugDeltaFromFirstNs, "delta_from_first_ns/D");
  debugTree->Branch("delta_from_previous_ns", &debugDeltaFromPreviousNs, "delta_from_previous_ns/D");
  debugTree->Branch("raw_frame_count", &debugNFrames, "raw_frame_count/I");
  debugTree->Branch("first_frame_type", &debugFirstFrameType, "first_frame_type/I");
  debugTree->Branch("first_frame_delta_ticks", &debugFirstFrameDeltaTicks, "first_frame_delta_ticks/L");
  debugTree->Branch("exo2_frame_count", &debugExo2FrameCount, "exo2_frame_count/I");

  const Long64_t nentries = tree->GetEntries();
  const Long64_t entriesToRead = limitByEventCount ? std::min(nentries, maxEntriesToRead) : nentries;

  for (Long64_t ev = 0; ev < entriesToRead; ++ev) {
    tree->GetEntry(ev);
    ++eventsRead;

    if (rawTopTimestamp == 0) continue;
    if (firstTopTS == 0) firstTopTS = rawTopTimestamp;

    const double topTimeNs = static_cast<double>(rawTopTimestamp - firstTopTS) * kTsTickToNs;
    if (!limitByEventCount && topTimeNs > requestedTimeWindowNs) {
      stoppedByTimeWindow = true;
      break;
    }

    if (topTimeNs >= 0.0) {
      eventTimesNs.push_back(topTimeNs);
      ++eventsWithNonZeroTS;
      if (topTimeNs > maxTimeNs) maxTimeNs = topTimeNs;
    }

    if (previousTopTS > 0 && rawTopTimestamp >= previousTopTS) {
      const ULong64_t dtTicks = rawTopTimestamp - previousTopTS;
      if (dtTicks > 0 && dtTicks < minPositiveEventDeltaTicks) {
        minPositiveEventDeltaTicks = dtTicks;
      }
      if (dtTicks > maxEventDeltaTicks) maxEventDeltaTicks = dtTicks;
    }
    lastTopTS = rawTopTimestamp;

    if (eventsRead <= 200) {
      debugEntry = ev;
      debugRawEventIndex = rawEventIndex;
      debugTopFrameType = rawTopFrameType;
      debugTopNbItems = rawTopNbItems;
      debugTopTimestamp = rawTopTimestamp;
      debugDeltaFromFirstTicks = static_cast<Long64_t>(rawTopTimestamp - firstTopTS);
      debugDeltaFromPreviousTicks = previousTopTS > 0
                                      ? static_cast<Long64_t>(rawTopTimestamp - previousTopTS)
                                      : 0;
      debugDeltaFromFirstNs = debugDeltaFromFirstTicks * kTsTickToNs;
      debugDeltaFromPreviousNs = debugDeltaFromPreviousTicks * kTsTickToNs;
      debugNFrames = rawFrameTimestamp ? static_cast<Int_t>(rawFrameTimestamp->size()) : 0;
      debugFirstFrameType = (rawFrameType && !rawFrameType->empty()) ? rawFrameType->at(0) : -1;
      debugFirstFrameDeltaTicks = 0;
      debugExo2FrameCount = 0;
      if (rawFrameTimestamp && !rawFrameTimestamp->empty()) {
        debugFirstFrameDeltaTicks = static_cast<Long64_t>(rawFrameTimestamp->at(0) - rawTopTimestamp);
      }
      if (rawFrameType) {
        for (Int_t frameTypeValue : *rawFrameType) {
          if (frameTypeValue == kMfmExo2FrameType) ++debugExo2FrameCount;
        }
      }
      debugTree->Fill();
    }
    previousTopTS = rawTopTimestamp;

    if (!rawFrameTimestamp) continue;
    for (size_t i = 0; i < rawFrameTimestamp->size(); ++i) {
      const ULong64_t frameTS = rawFrameTimestamp->at(i);
      if (frameTS == 0) continue;

      // EN: Avoid unsigned underflow if a nested frame TS is earlier than the first top TS.
      // CN: 如果子 frame TS 早于第一个 top TS，避免无符号整数下溢。
      if (frameTS < firstTopTS) continue;

      const double frameTimeNs = static_cast<double>(frameTS - firstTopTS) * kTsTickToNs;
      if (!limitByEventCount && frameTimeNs > requestedTimeWindowNs) continue;

      frameAllTimesNs.push_back(frameTimeNs);
      if (frameTimeNs > maxTimeNs) maxTimeNs = frameTimeNs;

      if (rawFrameType && i < rawFrameType->size() && rawFrameType->at(i) == kMfmExo2FrameType) {
        frameExo2TimesNs.push_back(frameTimeNs);
      }
    }
  }

  // EN: Build the time axis from the real span of the selected events.
  // CN: 根据选中 event 的实际时间跨度自动建立时间轴。
  // EN: ROOT histograms use Int_t bin counts; cap huge ns-bin requests to avoid overflow.
  // CN: ROOT 直方图 bin 数是 Int_t；对过大的 ns-bin 请求做上限保护，避免溢出成 0-10 ns。
  const double requestedAxisHighNs = limitByEventCount ? maxTimeNs : requestedTimeWindowNs;
  const double requestedTimeBinsDouble = std::floor(requestedAxisHighNs / binWidthNs) + 1.0;
  Int_t nbins = 1;
  double actualTimeBinWidthNs = binWidthNs;
  bool timeAxisBinsCapped = false;
  if (requestedTimeBinsDouble > static_cast<double>(kMaxTimeAxisHistogramBins)) {
    nbins = static_cast<Int_t>(kMaxTimeAxisHistogramBins);
    actualTimeBinWidthNs = requestedAxisHighNs > 0.0
                             ? requestedAxisHighNs / static_cast<double>(nbins)
                             : binWidthNs;
    timeAxisBinsCapped = true;
  } else {
    nbins = std::max(1, static_cast<Int_t>(requestedTimeBinsDouble));
  }
  const double xHigh = nbins * actualTimeBinWidthNs;

  // EN: Event-level axis: one fill per top-level MFM event.
  // CN: event 级时间轴：每个顶层 MFM event 填一次。
  TH1D *hEvent = new TH1D(
      "raw_event_time_axis",
      "Raw event counts vs TS time;Time from first raw event (ns);Top-level MFM events / bin",
      nbins, 0.0, xHigh);

  // EN: All-frame axis: top frame plus all nested frames recorded in RawTree vectors.
  // CN: 全部 frame 时间轴：包含顶层 frame 和 RawTree 向量中记录的所有子 frame。
  TH1D *hFrameAll = new TH1D(
      "raw_frame_time_axis_all",
      "Raw frame counts vs TS time, all frames;Time from first raw event (ns);Frames / bin",
      nbins, 0.0, xHigh);

  // EN: EXO2-only axis: raw_frame_type == 0x10, useful for EXOGAM crystal-fire timing.
  // CN: 只看 EXO2 frame：raw_frame_type == 0x10，用于查看 EXOGAM crystal fire 时间结构。
  TH1D *hFrameExo2 = new TH1D(
      "raw_frame_time_axis_exo2",
      "Raw EXO2 frame counts vs TS time;Time from first raw event (ns);EXO2 frames / bin",
      nbins, 0.0, xHigh);

  for (double t : eventTimesNs) hEvent->Fill(t);
  for (double t : frameAllTimesNs) hFrameAll->Fill(t);
  for (double t : frameExo2TimesNs) hFrameExo2->Fill(t);

  TGraph *gEventCumulative = BuildCumulativeGraph(
      eventTimesNs,
      "raw_event_cumulative_count_vs_time",
      "Raw event cumulative count;Time from first raw event (ns);Cumulative top-level MFM events");
  TGraph *gFrameAllCumulative = BuildCumulativeGraph(
      frameAllTimesNs,
      "raw_frame_all_cumulative_count_vs_time",
      "Raw all-frame cumulative count;Time from first raw event (ns);Cumulative frames");
  TGraph *gFrameExo2Cumulative = BuildCumulativeGraph(
      frameExo2TimesNs,
      "raw_frame_exo2_cumulative_count_vs_time",
      "Raw EXO2-frame cumulative count;Time from first raw event (ns);Cumulative EXO2 frames");

  TH1D *hEventDelta = BuildDeltaHistogram(
      eventTimesNs,
      "raw_event_delta_time_ns",
      "Raw event delta time;#DeltaT between consecutive top-level MFM events (ns);Pairs",
      binWidthNs);
  TH1D *hFrameExo2Delta = BuildDeltaHistogram(
      frameExo2TimesNs,
      "raw_frame_exo2_delta_time_ns",
      "Raw EXO2 frame delta time;#DeltaT between consecutive EXO2 frames (ns);Pairs",
      binWidthNs);

  const Long64_t eventsFilled = static_cast<Long64_t>(eventTimesNs.size());
  const Long64_t allFramesFilled = static_cast<Long64_t>(frameAllTimesNs.size());
  const Long64_t exo2FramesFilled = static_cast<Long64_t>(frameExo2TimesNs.size());

  TString outName = outputFile;
  if (outName.Length() == 0) outName = BuildDefaultRawTimestampOutputName(inputFile);

  TFile *fout = TFile::Open(outName, "RECREATE");
  if (!fout || fout->IsZombie()) {
    std::cerr << "Cannot create output file / 无法创建输出文件: " << outName << std::endl;
    fin->Close();
    return;
  }

  TString config;
  config.Form("input=%s; tree=%s; mode=%s; time_window_or_max_events=%.9g; "
              "time_window_ns=%.9g; entries_to_read=%lld; bin_width_ns=%.9g; "
              "time_axis_high_ns=%.9g; requested_bin_width_ns=%.9g; actual_bin_width_ns=%.9g; "
              "requested_time_bins=%.9g; hist_bins=%d; bins_capped=%d; "
              "ts_tick_to_ns=%.9g; limit_by_event_count=%d; stopped_by_time_window=%d; "
              "events_read=%lld; nonzero_top_ts_events=%lld; events_filled=%lld; "
              "all_frames_filled=%lld; exo2_frames_filled=%lld",
              inputFile, treeName, limitByEventCount ? "entry_count" : "time_window",
              timeWindowOrMaxEvents, requestedTimeWindowNs,
              static_cast<long long>(entriesToRead),
              binWidthNs, xHigh, binWidthNs, actualTimeBinWidthNs,
              requestedTimeBinsDouble, nbins, static_cast<int>(timeAxisBinsCapped),
              kTsTickToNs, static_cast<int>(limitByEventCount),
              static_cast<int>(stoppedByTimeWindow),
              static_cast<long long>(eventsRead),
              static_cast<long long>(eventsWithNonZeroTS),
              static_cast<long long>(eventsFilled),
              static_cast<long long>(allFramesFilled),
              static_cast<long long>(exo2FramesFilled));
  TNamed runInfo("raw_timestamp_axis_config", config.Data());

  hEvent->Write();
  hFrameAll->Write();
  hFrameExo2->Write();
  hEventDelta->Write();
  hFrameExo2Delta->Write();
  gEventCumulative->Write();
  gFrameAllCumulative->Write();
  gFrameExo2Cumulative->Write();
  debugTree->Write();
  runInfo.Write();

  TCanvas *canvas = new TCanvas("c_raw_timestamp_axis", "Raw timestamp axis", 1200, 900);
  canvas->Divide(1, 3);
  canvas->cd(1);
  hEvent->Draw("hist");
  canvas->cd(2);
  hFrameAll->Draw("hist");
  canvas->cd(3);
  hFrameExo2->Draw("hist");
  canvas->Write();

  TCanvas *cumulativeCanvas = new TCanvas("c_raw_timestamp_cumulative", "Raw cumulative timestamp axis", 1200, 900);
  cumulativeCanvas->Divide(1, 3);
  cumulativeCanvas->cd(1);
  gEventCumulative->Draw("AL");
  cumulativeCanvas->cd(2);
  gFrameAllCumulative->Draw("AL");
  cumulativeCanvas->cd(3);
  gFrameExo2Cumulative->Draw("AL");
  cumulativeCanvas->Write();

  TCanvas *deltaCanvas = new TCanvas("c_raw_timestamp_delta", "Raw timestamp delta", 1200, 700);
  deltaCanvas->Divide(1, 2);
  deltaCanvas->cd(1);
  hEventDelta->Draw("hist");
  deltaCanvas->cd(2);
  hFrameExo2Delta->Draw("hist");
  deltaCanvas->Write();

  fout->Close();
  fin->Close();

  std::cout << "Wrote / 写出: " << outName << std::endl;
  std::cout << "Read mode / 读取模式: "
            << (limitByEventCount ? "entry count / 固定 event 数"
                                  : "time window / 固定 TS 时间窗口")
            << std::endl;
  if (!limitByEventCount) {
    std::cout << "Requested time window / 请求时间窗口(ns): "
              << requestedTimeWindowNs << std::endl;
  }
  std::cout << "Events read / 读取 event 数: " << eventsRead << std::endl;
  std::cout << "Non-zero top TS events / 非零 top TS event 数: " << eventsWithNonZeroTS << std::endl;
  std::cout << "Events filled / 填充 event 数: " << eventsFilled << std::endl;
  std::cout << "Time axis high / 时间轴上限(ns): " << xHigh << std::endl;
  std::cout << "Requested time bins / 请求时间轴 bin 数: " << requestedTimeBinsDouble << std::endl;
  std::cout << "Histogram bins / 实际直方图 bin 数: " << nbins << std::endl;
  std::cout << "Requested bin width / 请求 bin 宽(ns): " << binWidthNs << std::endl;
  std::cout << "Actual bin width / 实际 bin 宽(ns): " << actualTimeBinWidthNs << std::endl;
  if (timeAxisBinsCapped) {
    std::cout << "Warning / 注意: requested ns-bin histogram was too large; "
              << "histogram bin count was capped, while cumulative graphs keep exact timestamps."
              << std::endl;
  }
  std::cout << "First top TS / 第一个 top TS: " << firstTopTS << std::endl;
  std::cout << "Last top TS / 最后一个 top TS: " << lastTopTS << std::endl;
  if (minPositiveEventDeltaTicks == std::numeric_limits<ULong64_t>::max()) {
    std::cout << "Min positive event delta / 最小非零 event 间隔: none" << std::endl;
  } else {
    std::cout << "Min positive event delta / 最小非零 event 间隔: "
              << minPositiveEventDeltaTicks << " ticks = "
              << minPositiveEventDeltaTicks * kTsTickToNs << " ns" << std::endl;
  }
  std::cout << "Max event delta / 最大 event 间隔: "
            << maxEventDeltaTicks << " ticks = "
            << maxEventDeltaTicks * kTsTickToNs << " ns" << std::endl;
  std::cout << "All frames filled / 填充 frame 数: " << allFramesFilled << std::endl;
  std::cout << "EXO2 frames filled / 填充 EXO2 frame 数: " << exo2FramesFilled << std::endl;
  std::cout << "Cumulative graphs written / 已写出累计曲线: "
            << "raw_event_cumulative_count_vs_time, "
            << "raw_frame_all_cumulative_count_vs_time, "
            << "raw_frame_exo2_cumulative_count_vs_time" << std::endl;
}
