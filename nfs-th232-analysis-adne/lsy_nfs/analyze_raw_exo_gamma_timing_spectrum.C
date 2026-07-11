// Analyze periodic timing components from ADNE RawTree EXO2 gamma frames.
// 从 ADNE RawTree 中筛选 EXO2 且有 gamma core 能量的 frame，分析时间周期成分。
//
// Usage / 用法:
//   cd /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne
//   source /home/user0/work/IJCLAB/NFS/NFS_env.sh
//
//   // Default search: periods 200-50000 ns, step 10 ns.
//   // 默认搜索：周期 200-50000 ns，步长 10 ns。
//   root -l -b -q 'lsy_nfs/analyze_raw_exo_gamma_timing_spectrum.C("out/nfs_run_23_r0.root")'
//
//   // Refined search around a candidate period.
//   // 在候选周期附近精细搜索。
//   root -l -b -q 'lsy_nfs/analyze_raw_exo_gamma_timing_spectrum.C("out/nfs_run_23_r0.root","RawTree",1200,1500,2,"out/exo_gamma_period.root")'
//
// Method / 方法:
//   - Use only raw_frame_type == 0x10 (EXO2 frame).
//     只使用 raw_frame_type == 0x10 的 EXO2 frame。
//   - Require raw_exo_inner_m6 or raw_exo_inner_m20 > gammaThreshold.
//     要求 raw_exo_inner_m6 或 raw_exo_inner_m20 大于 gammaThreshold。
//   - For each trial period, fold selected frame times into phase bins and compute
//     chi-square against a flat phase distribution.
//     对每个候选周期，把时间戳折叠到相位 bin，并计算相对均匀分布的 chi-square。
//   - The strongest periods are folded again using all selected frames.
//     对最强候选周期，用全部选中 frame 再折叠绘图。

#include <TCanvas.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TNamed.h>
#include <TString.h>
#include <TTree.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <numeric>
#include <vector>

namespace {

constexpr Int_t kMfmExo2FrameType = 0x10;
constexpr double kTsTickToNs = 10.0;

struct PeriodCandidate {
  double periodNs = 0.0;
  double frequencyMHz = 0.0;
  double power = 0.0;
};

TString BuildDefaultTimingSpectrumOutputName(const char *inputFile)
{
  TString outName(inputFile);
  if (outName.EndsWith(".root")) {
    outName.ReplaceAll(".root", "_exo_gamma_timing_spectrum.root");
  } else {
    outName += "_exo_gamma_timing_spectrum.root";
  }
  return outName;
}

bool HasBranch(TTree *tree, const char *branchName)
{
  return tree && tree->GetBranch(branchName) != nullptr;
}

double FrequencyMHzFromPeriodNs(double periodNs)
{
  return periodNs > 0.0 ? 1000.0 / periodNs : 0.0;
}

double EpochFoldingPower(const std::vector<double> &timesNs,
                         double periodNs,
                         int phaseBins,
                         std::vector<double> &workBins)
{
  if (timesNs.empty() || periodNs <= 0.0 || phaseBins <= 1) return 0.0;
  std::fill(workBins.begin(), workBins.end(), 0.0);

  for (double t : timesNs) {
    double phase = std::fmod(t, periodNs);
    if (phase < 0.0) phase += periodNs;
    int bin = static_cast<int>(phase / periodNs * phaseBins);
    if (bin < 0) bin = 0;
    if (bin >= phaseBins) bin = phaseBins - 1;
    workBins[bin] += 1.0;
  }

  const double mean = static_cast<double>(timesNs.size()) / static_cast<double>(phaseBins);
  if (mean <= 0.0) return 0.0;

  double chi2 = 0.0;
  for (double count : workBins) {
    const double diff = count - mean;
    chi2 += diff * diff / mean;
  }
  return chi2;
}

std::vector<double> BuildEvenSample(const std::vector<double> &timesNs, Long64_t maxSearchFrames)
{
  if (maxSearchFrames <= 0 || static_cast<Long64_t>(timesNs.size()) <= maxSearchFrames) {
    return timesNs;
  }

  std::vector<double> sample;
  sample.reserve(static_cast<size_t>(maxSearchFrames));
  const double step = static_cast<double>(timesNs.size()) / static_cast<double>(maxSearchFrames);
  for (Long64_t i = 0; i < maxSearchFrames; ++i) {
    size_t idx = static_cast<size_t>(std::floor(i * step));
    if (idx >= timesNs.size()) idx = timesNs.size() - 1;
    sample.push_back(timesNs[idx]);
  }
  return sample;
}

std::vector<PeriodCandidate> SelectTopCandidates(const std::vector<PeriodCandidate> &allCandidates,
                                                 int nTopPeriods,
                                                 double periodStepNs)
{
  std::vector<PeriodCandidate> sorted = allCandidates;
  std::sort(sorted.begin(), sorted.end(), [](const PeriodCandidate &a, const PeriodCandidate &b) {
    return a.power > b.power;
  });

  std::vector<PeriodCandidate> selected;
  const double minSeparationFloor = std::max(5.0 * periodStepNs, 1.0);
  for (const PeriodCandidate &cand : sorted) {
    bool tooClose = false;
    for (const PeriodCandidate &kept : selected) {
      const double minSeparation = std::max(minSeparationFloor, 0.01 * kept.periodNs);
      if (std::abs(cand.periodNs - kept.periodNs) < minSeparation) {
        tooClose = true;
        break;
      }
    }
    if (tooClose) continue;

    selected.push_back(cand);
    if (static_cast<int>(selected.size()) >= nTopPeriods) break;
  }
  return selected;
}

TH1D *BuildFoldHistogram(const std::vector<double> &timesNs,
                         const PeriodCandidate &candidate,
                         int foldBins,
                         int rank)
{
  TString name;
  name.Form("raw_exo_gamma_fold_period_rank%d", rank);
  TString title;
  title.Form("EXO2 gamma frames folded at %.6g ns (%.6g MHz);Phase in period (ns);EXO2 gamma frames / bin",
             candidate.periodNs, candidate.frequencyMHz);

  TH1D *hist = new TH1D(name, title, foldBins, 0.0, candidate.periodNs);
  for (double t : timesNs) {
    double phase = std::fmod(t, candidate.periodNs);
    if (phase < 0.0) phase += candidate.periodNs;
    hist->Fill(phase);
  }
  return hist;
}

} // namespace

void analyze_raw_exo_gamma_timing_spectrum(const char *inputFile,
                                           const char *treeName = "RawTree",
                                           double minPeriodNs = 200.0,
                                           double maxPeriodNs = 50000.0,
                                           double periodStepNs = 10.0,
                                           const char *outputFile = "",
                                           double gammaThreshold = 10.0,
                                           Long64_t maxSearchFrames = 100000,
                                           int searchPhaseBins = 64,
                                           int nTopPeriods = 6,
                                           int foldBins = 512,
                                           double timeStartNs = 0.0,
                                           double timeStopNs = -1.0)
{
  if (minPeriodNs <= 0.0 || maxPeriodNs <= minPeriodNs || periodStepNs <= 0.0) {
    std::cerr << "Bad period range / 周期范围错误" << std::endl;
    return;
  }
  if (gammaThreshold < 0.0) {
    std::cerr << "gammaThreshold must be non-negative / gammaThreshold 必须大于等于 0" << std::endl;
    return;
  }
  if (searchPhaseBins < 4 || foldBins < 4 || nTopPeriods < 1) {
    std::cerr << "Bad bin/top settings / bin 或候选数设置错误" << std::endl;
    return;
  }
  if (timeStartNs < 0.0 || (timeStopNs >= 0.0 && timeStopNs <= timeStartNs)) {
    std::cerr << "Bad time selection / 时间选择区间错误" << std::endl;
    return;
  }

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

  const char *requiredBranches[] = {
      "raw_top_timestamp",
      "raw_frame_timestamp",
      "raw_frame_type",
      "raw_exo_inner_m6",
      "raw_exo_inner_m20"};
  for (const char *branchName : requiredBranches) {
    if (!HasBranch(tree, branchName)) {
      std::cerr << "Missing branch / 缺少分支: " << branchName << std::endl;
      fin->Close();
      return;
    }
  }

  ULong64_t rawTopTimestamp = 0;
  std::vector<ULong64_t> *rawFrameTimestamp = nullptr;
  std::vector<Int_t> *rawFrameType = nullptr;
  std::vector<Int_t> *rawExoInnerM6 = nullptr;
  std::vector<Int_t> *rawExoInnerM20 = nullptr;

  tree->SetBranchAddress("raw_top_timestamp", &rawTopTimestamp);
  tree->SetBranchAddress("raw_frame_timestamp", &rawFrameTimestamp);
  tree->SetBranchAddress("raw_frame_type", &rawFrameType);
  tree->SetBranchAddress("raw_exo_inner_m6", &rawExoInnerM6);
  tree->SetBranchAddress("raw_exo_inner_m20", &rawExoInnerM20);

  std::vector<double> exoGammaTimesNs;
  ULong64_t firstTopTS = 0;
  Long64_t entriesRead = 0;
  Long64_t exo2FramesSeen = 0;
  Long64_t exo2FramesWithGamma = 0;
  Long64_t exo2FramesInTimeRange = 0;

  const Long64_t nentries = tree->GetEntries();
  for (Long64_t ev = 0; ev < nentries; ++ev) {
    tree->GetEntry(ev);
    ++entriesRead;

    if (rawTopTimestamp == 0) continue;
    if (firstTopTS == 0) firstTopTS = rawTopTimestamp;
    if (!rawFrameTimestamp || !rawFrameType || !rawExoInnerM6 || !rawExoInnerM20) continue;

    const size_t nframes = rawFrameTimestamp->size();
    for (size_t i = 0; i < nframes; ++i) {
      if (i >= rawFrameType->size()) continue;
      if (rawFrameType->at(i) != kMfmExo2FrameType) continue;
      ++exo2FramesSeen;

      const bool hasGammaEnergy =
          (i < rawExoInnerM6->size() && rawExoInnerM6->at(i) > gammaThreshold) ||
          (i < rawExoInnerM20->size() && rawExoInnerM20->at(i) > gammaThreshold);
      if (!hasGammaEnergy) continue;
      ++exo2FramesWithGamma;

      if (i >= rawFrameTimestamp->size()) continue;
      const ULong64_t frameTS = rawFrameTimestamp->at(i);
      if (frameTS == 0 || frameTS < firstTopTS) continue;

      const double tNs = static_cast<double>(frameTS - firstTopTS) * kTsTickToNs;
      if (tNs < timeStartNs) continue;
      if (timeStopNs >= 0.0 && tNs > timeStopNs) continue;

      ++exo2FramesInTimeRange;
      exoGammaTimesNs.push_back(tNs);
    }
  }

  if (exoGammaTimesNs.empty()) {
    std::cerr << "No EXO2 gamma frames found / 没有找到 EXO2 gamma frame" << std::endl;
    fin->Close();
    return;
  }

  std::sort(exoGammaTimesNs.begin(), exoGammaTimesNs.end());
  const double firstTimeNs = exoGammaTimesNs.front();
  const double lastTimeNs = exoGammaTimesNs.back();
  const double spanNs = lastTimeNs - firstTimeNs;

  std::vector<double> searchTimesNs = BuildEvenSample(exoGammaTimesNs, maxSearchFrames);
  // EN: Shift search times to improve numerical stability in long runs.
  // CN: 长时间运行时先平移时间，提高 fmod 数值稳定性。
  for (double &t : searchTimesNs) t -= firstTimeNs;

  const int nPeriodBins = static_cast<int>(std::floor((maxPeriodNs - minPeriodNs) / periodStepNs)) + 1;
  TH1D *hPeriodPower = new TH1D(
      "raw_exo_gamma_periodogram_period_ns",
      "EXO2 gamma epoch-folding periodogram;Trial period (ns);Epoch-folding #chi^{2}",
      nPeriodBins,
      minPeriodNs - 0.5 * periodStepNs,
      minPeriodNs + (nPeriodBins - 0.5) * periodStepNs);

  std::vector<PeriodCandidate> allCandidates;
  allCandidates.reserve(static_cast<size_t>(nPeriodBins));
  std::vector<double> workBins(static_cast<size_t>(searchPhaseBins), 0.0);

  for (int ip = 0; ip < nPeriodBins; ++ip) {
    const double periodNs = minPeriodNs + ip * periodStepNs;
    const double power = EpochFoldingPower(searchTimesNs, periodNs, searchPhaseBins, workBins);
    hPeriodPower->SetBinContent(ip + 1, power);
    allCandidates.push_back({periodNs, FrequencyMHzFromPeriodNs(periodNs), power});
  }

  std::vector<PeriodCandidate> topCandidates =
      SelectTopCandidates(allCandidates, nTopPeriods, periodStepNs);

  TGraph *gFrequencyPower = new TGraph(nPeriodBins);
  gFrequencyPower->SetName("raw_exo_gamma_periodogram_frequency_mhz");
  gFrequencyPower->SetTitle("EXO2 gamma epoch-folding periodogram;Frequency (MHz);Epoch-folding #chi^{2}");
  for (int ip = 0; ip < nPeriodBins; ++ip) {
    const int src = nPeriodBins - 1 - ip;
    gFrequencyPower->SetPoint(ip, allCandidates[src].frequencyMHz, allCandidates[src].power);
  }

  std::vector<TH1D *> foldHists;
  foldHists.reserve(topCandidates.size());
  std::vector<double> shiftedAllTimesNs = exoGammaTimesNs;
  for (double &t : shiftedAllTimesNs) t -= firstTimeNs;
  for (size_t i = 0; i < topCandidates.size(); ++i) {
    foldHists.push_back(BuildFoldHistogram(shiftedAllTimesNs,
                                           topCandidates[i],
                                           foldBins,
                                           static_cast<int>(i + 1)));
  }

  TString outName = outputFile;
  if (outName.Length() == 0) outName = BuildDefaultTimingSpectrumOutputName(inputFile);

  TFile *fout = TFile::Open(outName, "RECREATE");
  if (!fout || fout->IsZombie()) {
    std::cerr << "Cannot create output file / 无法创建输出文件: " << outName << std::endl;
    fin->Close();
    return;
  }

  TTree *candidateTree = new TTree("raw_exo_gamma_period_candidates",
                                   "Strongest EXO2 gamma timing period candidates");
  Int_t rank = 0;
  Double_t candidatePeriodNs = 0.0;
  Double_t candidateFrequencyMHz = 0.0;
  Double_t candidatePower = 0.0;
  candidateTree->Branch("rank", &rank, "rank/I");
  candidateTree->Branch("period_ns", &candidatePeriodNs, "period_ns/D");
  candidateTree->Branch("frequency_mhz", &candidateFrequencyMHz, "frequency_mhz/D");
  candidateTree->Branch("power", &candidatePower, "power/D");
  for (size_t i = 0; i < topCandidates.size(); ++i) {
    rank = static_cast<Int_t>(i + 1);
    candidatePeriodNs = topCandidates[i].periodNs;
    candidateFrequencyMHz = topCandidates[i].frequencyMHz;
    candidatePower = topCandidates[i].power;
    candidateTree->Fill();
  }

  TString config;
  config.Form("input=%s; tree=%s; min_period_ns=%.9g; max_period_ns=%.9g; "
              "period_step_ns=%.9g; gamma_threshold=%.9g; max_search_frames=%lld; "
              "search_phase_bins=%d; fold_bins=%d; time_start_ns=%.9g; time_stop_ns=%.9g; "
              "entries_read=%lld; exo2_frames_seen=%lld; exo2_frames_with_gamma=%lld; "
              "exo2_frames_in_time_range=%lld; selected_times=%lld; search_times=%lld; "
              "first_selected_time_ns=%.9g; last_selected_time_ns=%.9g; span_ns=%.9g",
              inputFile, treeName, minPeriodNs, maxPeriodNs,
              periodStepNs, gammaThreshold, static_cast<long long>(maxSearchFrames),
              searchPhaseBins, foldBins, timeStartNs, timeStopNs,
              static_cast<long long>(entriesRead),
              static_cast<long long>(exo2FramesSeen),
              static_cast<long long>(exo2FramesWithGamma),
              static_cast<long long>(exo2FramesInTimeRange),
              static_cast<long long>(exoGammaTimesNs.size()),
              static_cast<long long>(searchTimesNs.size()),
              firstTimeNs, lastTimeNs, spanNs);
  TNamed runInfo("raw_exo_gamma_timing_spectrum_config", config.Data());

  hPeriodPower->Write();
  gFrequencyPower->Write();
  candidateTree->Write();
  runInfo.Write();
  for (TH1D *hist : foldHists) hist->Write();

  TCanvas *periodCanvas = new TCanvas("c_raw_exo_gamma_periodogram", "EXO2 gamma periodogram", 1200, 700);
  hPeriodPower->Draw("hist");
  periodCanvas->Write();

  TCanvas *frequencyCanvas = new TCanvas("c_raw_exo_gamma_frequency_spectrum", "EXO2 gamma frequency spectrum", 1200, 700);
  gFrequencyPower->Draw("AL");
  frequencyCanvas->Write();

  TCanvas *foldCanvas = new TCanvas("c_raw_exo_gamma_fold_top_periods", "EXO2 gamma folded top periods", 1200, 900);
  const int nPads = std::max(1, static_cast<int>(foldHists.size()));
  const int nCols = nPads > 3 ? 2 : 1;
  const int nRows = (nPads + nCols - 1) / nCols;
  foldCanvas->Divide(nCols, nRows);
  for (size_t i = 0; i < foldHists.size(); ++i) {
    foldCanvas->cd(static_cast<int>(i + 1));
    foldHists[i]->Draw("hist");
  }
  foldCanvas->Write();

  fout->Close();
  fin->Close();

  std::cout << "Wrote / 写出: " << outName << std::endl;
  std::cout << "EXO2 frames seen / 看到的 EXO2 frame 数: " << exo2FramesSeen << std::endl;
  std::cout << "EXO2 frames with gamma core raw energy / 有 gamma core 原始能量的 EXO2 frame 数: "
            << exo2FramesWithGamma << std::endl;
  std::cout << "Selected EXO2 gamma frames / 选中 EXO2 gamma frame 数: "
            << exoGammaTimesNs.size() << std::endl;
  std::cout << "Search sample size / 谱分析抽样数: " << searchTimesNs.size() << std::endl;
  std::cout << "Selected time span / 选中时间跨度(ns): " << spanNs << std::endl;
  std::cout << "Top period candidates / 最强周期候选:" << std::endl;
  for (size_t i = 0; i < topCandidates.size(); ++i) {
    std::cout << "  rank " << (i + 1)
              << ": period=" << topCandidates[i].periodNs << " ns"
              << ", frequency=" << topCandidates[i].frequencyMHz << " MHz"
              << ", power=" << topCandidates[i].power << std::endl;
  }
}
