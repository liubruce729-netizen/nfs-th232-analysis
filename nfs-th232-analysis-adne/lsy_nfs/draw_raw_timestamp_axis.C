// Draw RawTree timestamp axes from ADNE raw-tree output.
// 从 ADNE 的 RawTree 输出绘制原始 MFM event/frame 的时间轴。
//
// Usage / 用法:
//   cd /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne
//   source /home/user0/work/IJCLAB/NFS/NFS_env.sh
//   root -l -b -q 'lsy_nfs/draw_raw_timestamp_axis.C("out/nfs_run_23_r0.root")'
//   root -l -b -q 'lsy_nfs/draw_raw_timestamp_axis.C("out/nfs_run_23_r0.root","RawTree",5000,10,"out/raw_ts.root")'
//
// Notes / 说明:
//   - ADNE stores MFM timestamps as absolute TS ticks.
//     ADNE 保存的是 MFM 绝对时间戳 TS tick。
//   - In this analysis chain, 1 TS tick = 10 ns, inferred from ADNE rate calculation:
//       seconds = (TS - TStart) / 1e8
//     在本分析链中，1 个 TS tick = 10 ns；ADNE 计算 rate 时使用 (TS - TStart) / 1e8 得到秒。
//   - The macro subtracts the first non-zero top timestamp and draws only a short window.
//     本脚本减去第一个非零顶层 event TS，只画开头一小段时间窗口。
//   - RawTree one entry = one top-level MFM event; frame vectors contain the top frame and
//     all nested frames captured from that event.
//     RawTree 的一个 entry 对应一个顶层 MFM event；frame 向量包含顶层 frame 和其中的子 frame。

#include <TBranch.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TNamed.h>
#include <TString.h>
#include <TTree.h>

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

} // namespace

void draw_raw_timestamp_axis(const char *inputFile,
                             const char *treeName = "RawTree",
                             double windowNs = 5000.0,
                             double binWidthNs = 10.0,
                             const char *outputFile = "",
                             bool stopAfterWindow = true)
{
  // EN: Validate user parameters before opening large files.
  // CN: 先检查参数，避免对大文件做无效读取。
  if (windowNs <= 0.0) {
    std::cerr << "windowNs must be positive / windowNs 必须大于 0" << std::endl;
    return;
  }
  if (binWidthNs <= 0.0) {
    std::cerr << "binWidthNs must be positive / binWidthNs 必须大于 0" << std::endl;
    return;
  }

  const Int_t nbins = std::max(1, static_cast<Int_t>(std::ceil(windowNs / binWidthNs)));
  const double xHigh = nbins * binWidthNs;

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

  ULong64_t firstTopTS = 0;
  Long64_t eventsRead = 0;
  Long64_t eventsFilled = 0;
  Long64_t allFramesFilled = 0;
  Long64_t exo2FramesFilled = 0;

  const Long64_t nentries = tree->GetEntries();
  for (Long64_t ev = 0; ev < nentries; ++ev) {
    tree->GetEntry(ev);
    ++eventsRead;

    if (rawTopTimestamp == 0) continue;
    if (firstTopTS == 0) firstTopTS = rawTopTimestamp;

    const double topTimeNs = static_cast<double>(rawTopTimestamp - firstTopTS) * kTsTickToNs;

    // EN: RawTree is expected to be time ordered; stop early to avoid scanning large files.
    // CN: RawTree 通常按时间顺序写入；超过窗口后提前停止，避免扫描大文件。
    if (stopAfterWindow && topTimeNs > windowNs) break;

    if (topTimeNs >= 0.0 && topTimeNs <= windowNs) {
      hEvent->Fill(topTimeNs);
      ++eventsFilled;
    }

    if (!rawFrameTimestamp) continue;
    for (size_t i = 0; i < rawFrameTimestamp->size(); ++i) {
      const ULong64_t frameTS = rawFrameTimestamp->at(i);
      if (frameTS == 0) continue;

      const double frameTimeNs = static_cast<double>(frameTS - firstTopTS) * kTsTickToNs;
      if (frameTimeNs < 0.0 || frameTimeNs > windowNs) continue;

      hFrameAll->Fill(frameTimeNs);
      ++allFramesFilled;

      if (rawFrameType && i < rawFrameType->size() && rawFrameType->at(i) == kMfmExo2FrameType) {
        hFrameExo2->Fill(frameTimeNs);
        ++exo2FramesFilled;
      }
    }
  }

  TString outName = outputFile;
  if (outName.Length() == 0) outName = BuildDefaultRawTimestampOutputName(inputFile);

  TFile *fout = TFile::Open(outName, "RECREATE");
  if (!fout || fout->IsZombie()) {
    std::cerr << "Cannot create output file / 无法创建输出文件: " << outName << std::endl;
    fin->Close();
    return;
  }

  TString config;
  config.Form("input=%s; tree=%s; window_ns=%.9g; bin_width_ns=%.9g; ts_tick_to_ns=%.9g; "
              "stop_after_window=%d; events_read=%lld; events_filled=%lld; "
              "all_frames_filled=%lld; exo2_frames_filled=%lld",
              inputFile, treeName, windowNs, binWidthNs, kTsTickToNs,
              static_cast<int>(stopAfterWindow),
              static_cast<long long>(eventsRead),
              static_cast<long long>(eventsFilled),
              static_cast<long long>(allFramesFilled),
              static_cast<long long>(exo2FramesFilled));
  TNamed runInfo("raw_timestamp_axis_config", config.Data());

  hEvent->Write();
  hFrameAll->Write();
  hFrameExo2->Write();
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

  fout->Close();
  fin->Close();

  std::cout << "Wrote / 写出: " << outName << std::endl;
  std::cout << "Events read / 读取 event 数: " << eventsRead << std::endl;
  std::cout << "Events filled / 填充 event 数: " << eventsFilled << std::endl;
  std::cout << "All frames filled / 填充 frame 数: " << allFramesFilled << std::endl;
  std::cout << "EXO2 frames filled / 填充 EXO2 frame 数: " << exo2FramesFilled << std::endl;
}
