// Draw RawTree timestamp axes from ADNE raw-tree output.
// 从 ADNE 的 RawTree 输出绘制原始 MFM event/frame 的时间轴。
//
// Usage / 用法:
//   cd /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne
//   source /home/user0/work/IJCLAB/NFS/NFS_env.sh
//   root -l -b -q 'lsy_nfs/draw_raw_timestamp_axis.C("out/nfs_run_23_r0.root")'
//   root -l -b -q 'lsy_nfs/draw_raw_timestamp_axis.C("out/nfs_run_23_r0.root","RawTree",5000,10,"out/raw_ts.root")'
//
// Arguments / 参数:
//   inputFile, treeName, maxEvents, binWidthNs, outputFile, limitByEventCount
//   第三个参数保持原接口位置不变，现在表示最多读取的 RawTree event 数。
//
// Notes / 说明:
//   - ADNE stores MFM timestamps as absolute TS ticks.
//     ADNE 保存的是 MFM 绝对时间戳 TS tick。
//   - In this analysis chain, 1 TS tick = 10 ns, inferred from ADNE rate calculation:
//       seconds = (TS - TStart) / 1e8
//     在本分析链中，1 个 TS tick = 10 ns；ADNE 计算 rate 时使用 (TS - TStart) / 1e8 得到秒。
//   - The macro subtracts the first non-zero top timestamp and draws the first N RawTree events.
//     本脚本减去第一个非零顶层 event TS，并按 RawTree event 数截取前 N 个 event。
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
#include <vector>

namespace {

constexpr Int_t kMfmExo2FrameType = 0x10;
constexpr double kTsTickToNs = 10.0;

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

  const Int_t nbins = std::max(1, static_cast<Int_t>(std::floor(maxDelta / binWidthNs)) + 1);
  TH1D *hist = new TH1D(name, title, nbins, 0.0, nbins * binWidthNs);
  for (double dt : deltas) hist->Fill(dt);
  return hist;
}

} // namespace

void draw_raw_timestamp_axis(const char *inputFile,
                             const char *treeName = "RawTree",
                             double maxEvents = 5000.0,
                             double binWidthNs = 10.0,
                             const char *outputFile = "",
                             bool limitByEventCount = true)
{
  // EN: Validate user parameters before opening large files.
  // CN: 先检查参数，避免对大文件做无效读取。
  if (maxEvents <= 0.0) {
    std::cerr << "maxEvents must be positive / maxEvents 必须大于 0" << std::endl;
    return;
  }
  if (binWidthNs <= 0.0) {
    std::cerr << "binWidthNs must be positive / binWidthNs 必须大于 0" << std::endl;
    return;
  }

  const Long64_t maxEntriesToRead = static_cast<Long64_t>(std::llround(maxEvents));

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
  std::vector<ULong64_t> *rawFrameTimestamp = nullptr;
  std::vector<Int_t> *rawFrameType = nullptr;

  tree->SetBranchAddress("raw_top_timestamp", &rawTopTimestamp);

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

  // EN: First pass: read the requested number of RawTree entries and keep relative times.
  // CN: 第一遍：读取指定数量的 RawTree entry，保存相对时间。
  std::vector<double> eventTimesNs;
  std::vector<double> frameAllTimesNs;
  std::vector<double> frameExo2TimesNs;

  ULong64_t firstTopTS = 0;
  Long64_t eventsRead = 0;
  Long64_t eventsWithNonZeroTS = 0;
  double maxTimeNs = 0.0;

  const Long64_t nentries = tree->GetEntries();
  const Long64_t entriesToRead = limitByEventCount ? std::min(nentries, maxEntriesToRead) : nentries;

  for (Long64_t ev = 0; ev < entriesToRead; ++ev) {
    tree->GetEntry(ev);
    ++eventsRead;

    if (rawTopTimestamp == 0) continue;
    if (firstTopTS == 0) firstTopTS = rawTopTimestamp;

    const double topTimeNs = static_cast<double>(rawTopTimestamp - firstTopTS) * kTsTickToNs;
    if (topTimeNs >= 0.0) {
      eventTimesNs.push_back(topTimeNs);
      ++eventsWithNonZeroTS;
      if (topTimeNs > maxTimeNs) maxTimeNs = topTimeNs;
    }

    if (!rawFrameTimestamp) continue;
    for (size_t i = 0; i < rawFrameTimestamp->size(); ++i) {
      const ULong64_t frameTS = rawFrameTimestamp->at(i);
      if (frameTS == 0) continue;

      // EN: Avoid unsigned underflow if a nested frame TS is earlier than the first top TS.
      // CN: 如果子 frame TS 早于第一个 top TS，避免无符号整数下溢。
      if (frameTS < firstTopTS) continue;

      const double frameTimeNs = static_cast<double>(frameTS - firstTopTS) * kTsTickToNs;
      frameAllTimesNs.push_back(frameTimeNs);
      if (frameTimeNs > maxTimeNs) maxTimeNs = frameTimeNs;

      if (rawFrameType && i < rawFrameType->size() && rawFrameType->at(i) == kMfmExo2FrameType) {
        frameExo2TimesNs.push_back(frameTimeNs);
      }
    }
  }

  // EN: Build the time axis from the real span of the selected events.
  // CN: 根据选中 event 的实际时间跨度自动建立时间轴。
  const Int_t nbins = std::max(1, static_cast<Int_t>(std::floor(maxTimeNs / binWidthNs)) + 1);
  const double xHigh = nbins * binWidthNs;

  // EN: Event-level axis: one fill per top-level MFM event.
  // CN: event 级时间轴：每个顶层 MFM event 填一次。
  TH1D *hEvent = new TH1D(
      "raw_event_time_axis",
      "Raw event time axis;Time from first raw event (ns);Top-level MFM events",
      nbins, 0.0, xHigh);

  // EN: All-frame axis: top frame plus all nested frames recorded in RawTree vectors.
  // CN: 全部 frame 时间轴：包含顶层 frame 和 RawTree 向量中记录的所有子 frame。
  TH1D *hFrameAll = new TH1D(
      "raw_frame_time_axis_all",
      "Raw frame time axis, all frames;Time from first raw event (ns);Frames",
      nbins, 0.0, xHigh);

  // EN: EXO2-only axis: raw_frame_type == 0x10, useful for EXOGAM crystal-fire timing.
  // CN: 只看 EXO2 frame：raw_frame_type == 0x10，用于查看 EXOGAM crystal fire 时间结构。
  TH1D *hFrameExo2 = new TH1D(
      "raw_frame_time_axis_exo2",
      "Raw frame time axis, EXO2 frames;Time from first raw event (ns);EXO2 frames",
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
  config.Form("input=%s; tree=%s; max_events=%.9g; entries_to_read=%lld; bin_width_ns=%.9g; "
              "time_axis_high_ns=%.9g; ts_tick_to_ns=%.9g; limit_by_event_count=%d; "
              "events_read=%lld; nonzero_top_ts_events=%lld; events_filled=%lld; "
              "all_frames_filled=%lld; exo2_frames_filled=%lld",
              inputFile, treeName, maxEvents,
              static_cast<long long>(entriesToRead),
              binWidthNs, xHigh, kTsTickToNs,
              static_cast<int>(limitByEventCount),
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
  std::cout << "Events read / 读取 event 数: " << eventsRead << std::endl;
  std::cout << "Non-zero top TS events / 非零 top TS event 数: " << eventsWithNonZeroTS << std::endl;
  std::cout << "Events filled / 填充 event 数: " << eventsFilled << std::endl;
  std::cout << "Time axis high / 时间轴上限(ns): " << xHigh << std::endl;
  std::cout << "All frames filled / 填充 frame 数: " << allFramesFilled << std::endl;
  std::cout << "EXO2 frames filled / 填充 EXO2 frame 数: " << exo2FramesFilled << std::endl;
  std::cout << "Cumulative graphs written / 已写出累计曲线: "
            << "raw_event_cumulative_count_vs_time, "
            << "raw_frame_all_cumulative_count_vs_time, "
            << "raw_frame_exo2_cumulative_count_vs_time" << std::endl;
}
