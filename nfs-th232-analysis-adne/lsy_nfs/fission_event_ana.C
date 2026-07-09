// Fission-event gamma analysis from NFS mult3 trees.
// 基于 NFS mult3 tree 的裂变候选事件 gamma 分析。
//
// Usage / 用法:
//   root -l -b -q 'lsy_nfs/fission_event_ana.C("out/mult3_nfs_run_100_r0.root",8.6,20,"2,4,7,12,20,30,50")'
//   root -l -b -q 'lsy_nfs/fission_event_ana.C("out/mult3_nfs_run_100_r0.root,out/mult3_nfs_run_100_r1.root",8.6,20,"2,4,7,12,20,30,50","out/fission_event_ana.root")'
//   root -l -b -q 'lsy_nfs/fission_event_ana.C("@out/mult3_filelist.txt",8.6,20,"2,4,7,12,20,30,50","out/fission_event_ana.root")'
//   root -l -b -q 'lsy_nfs/fission_event_ana.C("@out/mult3_filelist.txt",8.6,20,"4,10,20,50","out/fission_event_ana.root",true,-1,3)'  // force mult>=3
//
// EN: The macro reads f_E877_Clover_* split branches from TreeMaster.
// CN: 该宏读取 TreeMaster 中的 f_E877_Clover_* 拆分分支。

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "TCanvas.h"
#include "TBranch.h"
#include "TChain.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLegend.h"
#include "TNamed.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TPaveText.h"
#include "TString.h"
#include "TSystem.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"

namespace {

constexpr double kNeutronMassMeV = 939.5654;
constexpr double kLightSpeedMeterPerNs = 0.299792458;

double EnergyMeVToToFNs(double energyMeV, double distanceMeter)
{
  // EN: Invert the ADNE non-relativistic formula:
  //     E = 0.5*m_n*(L/(tof*c))^2.
  // CN: 反解 ADNE 使用的非相对论公式：
  //     E = 0.5*m_n*(L/(tof*c))^2。
  if (energyMeV <= 0.0 || distanceMeter <= 0.0) {
    return std::numeric_limits<double>::infinity();
  }
  const double beta = std::sqrt(2.0 * energyMeV / kNeutronMassMeV);
  return distanceMeter / (kLightSpeedMeterPerNs * beta);
}

double ToFNsToEnergyMeV(double tofNs, double distanceMeter)
{
  if (tofNs <= 0.0 || distanceMeter <= 0.0) return 0.0;
  const double beta = distanceMeter / (tofNs * kLightSpeedMeterPerNs);
  return 0.5 * kNeutronMassMeV * beta * beta;
}

std::vector<TString> ReadFileList(const TString &listPath)
{
  // EN: For long runs, pass inputFiles as "@filelist.txt" to avoid an overlong ROOT command line.
  // CN: 长任务文件很多时，用 "@filelist.txt" 传入，避免 ROOT 命令行过长。
  std::vector<TString> items;
  std::ifstream input(listPath.Data());
  if (!input.is_open()) {
    std::cerr << "Cannot open fission input list: " << listPath << std::endl;
    return items;
  }

  std::string line;
  while (std::getline(input, line)) {
    TString item(line.c_str());
    item = item.Strip(TString::kBoth);
    if (item.IsNull() || item.BeginsWith("#") || item.BeginsWith("//")) continue;
    items.push_back(item);
  }
  return items;
}

std::vector<TString> SplitCsv(const char *text)
{
  std::vector<TString> items;
  TString input(text ? text : "");
  input = input.Strip(TString::kBoth);
  if (input.BeginsWith("@")) {
    TString listPath = input(1, input.Length() - 1);
    listPath = listPath.Strip(TString::kBoth);
    return ReadFileList(listPath);
  }

  TObjArray *tokens = input.Tokenize(",");
  for (int i = 0; i < tokens->GetEntriesFast(); ++i) {
    TString item = static_cast<TObjString *>(tokens->At(i))->GetString();
    item = item.Strip(TString::kBoth);
    if (!item.IsNull()) items.push_back(item);
  }
  delete tokens;
  return items;
}

std::vector<double> ParseEnergyEdges(const char *energyBinEdgesMeV)
{
  std::vector<double> edges;
  for (const auto &item : SplitCsv(energyBinEdgesMeV)) {
    edges.push_back(item.Atof());
  }
  std::sort(edges.begin(), edges.end());
  edges.erase(std::unique(edges.begin(), edges.end()), edges.end());
  return edges;
}

TString FormatNumberForName(double value)
{
  TString label;
  if (std::fabs(value - std::round(value)) < 1e-6) label.Form("%.0f", value);
  else label.Form("%.3f", value);
  label.ReplaceAll("-", "m");
  label.ReplaceAll(".", "p");
  return label;
}

TString MakeBinTag(double lowEnergyMeV, double highEnergyMeV)
{
  return TString::Format("E%s_%sMeV",
                         FormatNumberForName(lowEnergyMeV).Data(),
                         FormatNumberForName(highEnergyMeV).Data());
}

TString MakeBinTitle(double lowEnergyMeV, double highEnergyMeV,
                     double lowToFNs, double highToFNs)
{
  if (lowEnergyMeV <= 0.0) {
    return TString::Format("E_n %.3g-%.3g MeV, Time >= %.2f ns",
                           lowEnergyMeV, highEnergyMeV, highToFNs);
  }
  return TString::Format("E_n %.3g-%.3g MeV, %.2f <= Time < %.2f ns",
                         lowEnergyMeV, highEnergyMeV, highToFNs, lowToFNs);
}

TString ResolveBranch(TTree *tree, const char *leafName)
{
  std::vector<TString> candidates;
  candidates.emplace_back(leafName);
  candidates.emplace_back(TString::Format("Exogam2.%s", leafName));
  candidates.emplace_back(TString::Format("Exogam2_%s", leafName));
  candidates.emplace_back(TString::Format("Exogam2/%s", leafName));

  for (const auto &candidate : candidates) {
    if (tree->GetBranch(candidate)) return candidate;
  }

  TObjArray *branches = tree->GetListOfBranches();
  for (int i = 0; i < branches->GetEntriesFast(); ++i) {
    auto *branch = static_cast<TBranch *>(branches->At(i));
    TString name = branch->GetName();
    if (name.EndsWith(leafName)) return name;
  }
  return "";
}

TString BuildDefaultOutputName(const char *inputFiles)
{
  auto inputs = SplitCsv(inputFiles);
  TString first = inputs.empty() ? "fission_event_ana.root" : inputs.front();
  if (first.Contains("*") || first.Contains("?")) return "fission_event_ana.root";

  Ssiz_t slash = first.Last('/');
  TString directory = "";
  TString basename = first;
  if (slash != kNPOS) {
    directory = first(0, slash + 1);
    basename = first(slash + 1, first.Length() - slash - 1);
  }
  if (basename.EndsWith(".root")) basename.Remove(basename.Length() - 5);
  return directory + basename + "_fission_event_ana.root";
}

struct EnergyTimeBin {
  double lowEnergyMeV = 0.0;
  double highEnergyMeV = 0.0;
  double lowToFNs = std::numeric_limits<double>::infinity();
  double highToFNs = 0.0;
  TString tag;
  TString title;
  TH1F *spectrum = nullptr;
  TH2F *matrix = nullptr;
  Long64_t selectedEvents = 0;
  Long64_t selectedGammaFires = 0;
};

int FindEnergyTimeBin(const std::vector<EnergyTimeBin> &bins, double eventTimeNs)
{
  // EN: Larger neutron energy means shorter flight time.
  // CN: 中子能量越高，飞行时间越短。
  for (std::size_t i = 0; i < bins.size(); ++i) {
    const auto &bin = bins[i];
    if (eventTimeNs < bin.highToFNs) continue;
    if (bin.lowEnergyMeV > 0.0 && eventTimeNs >= bin.lowToFNs) continue;
    return static_cast<int>(i);
  }
  return -1;
}

void WriteCanvasForSpectrum(TDirectory *outDir, TH1F *hist, const TString &binTitle)
{
  TCanvas *canvas = new TCanvas(TString::Format("c_%s", hist->GetName()),
                                hist->GetTitle(), 1100, 750);
  hist->Draw("hist");
  TLegend *legend = new TLegend(0.58, 0.76, 0.88, 0.88);
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);
  legend->AddEntry(hist, binTitle, "l");
  legend->Draw();
  outDir->cd();
  canvas->Write();
}

void WriteCanvasForMatrix(TDirectory *outDir, TH2F *hist, const TString &binTitle)
{
  TCanvas *canvas = new TCanvas(TString::Format("c_%s", hist->GetName()),
                                hist->GetTitle(), 1100, 900);
  hist->Draw("colz");
  TPaveText *text = new TPaveText(0.12, 0.92, 0.55, 0.98, "NDC");
  text->SetBorderSize(0);
  text->SetFillStyle(0);
  text->AddText(binTitle);
  text->Draw();
  outDir->cd();
  canvas->Write();
}

} // namespace

void fission_event_ana(const char *inputFiles = "out/mult3_nfs_run_100_r0.root",
                       double nfsDistanceMeter = 8.6,
                       double timeFwhmNs = 20.0,
                       const char *energyBinEdgesMeV = "2,4,7,12,20,30,50",
                       const char *outputFile = "",
                       bool useBgoCsiVeto = true,
                       Long64_t maxEntries = -1,
                       int minCloverMultiplicity = 2)
{
  if (minCloverMultiplicity < 1) {
    std::cerr << "Invalid minCloverMultiplicity: " << minCloverMultiplicity << std::endl;
    return;
  }
  if (nfsDistanceMeter <= 0.0) {
    std::cerr << "Invalid nfsDistanceMeter: " << nfsDistanceMeter << std::endl;
    return;
  }

  std::vector<double> edges = ParseEnergyEdges(energyBinEdgesMeV);
  if (edges.empty() || edges.back() <= 0.0) {
    std::cerr << "No valid positive neutron-energy bin edge was provided." << std::endl;
    return;
  }

  // EN: Reject unphysically early clover fires by assuming the fastest relevant neutron is 50 MeV.
  // CN: 认为最高相关中子能量为 50 MeV，用它对应的最短飞行时间去掉过早 fire。
  const double highestEnergyMeV = 50.0;
  const double tMinNs = EnergyMeVToToFNs(highestEnergyMeV, nfsDistanceMeter);
  const double sigmaTimeNs = timeFwhmNs / 2.355;
  const double tCutNs = tMinNs - 3.0 * sigmaTimeNs;

  std::vector<EnergyTimeBin> bins;
  double lowEnergy = 0.0;
  for (double highEnergy : edges) {
    if (highEnergy <= lowEnergy) continue;
    EnergyTimeBin bin;
    bin.lowEnergyMeV = lowEnergy;
    bin.highEnergyMeV = highEnergy;
    bin.lowToFNs = EnergyMeVToToFNs(lowEnergy, nfsDistanceMeter);
    bin.highToFNs = EnergyMeVToToFNs(highEnergy, nfsDistanceMeter);
    bin.tag = MakeBinTag(lowEnergy, highEnergy);
    bin.title = MakeBinTitle(lowEnergy, highEnergy, bin.lowToFNs, bin.highToFNs);

    const TString specName = TString::Format("Mult3GammaSpec_%s", bin.tag.Data());
    const TString specTitle = TString::Format("Mult3GammaSpec %s;Gamma energy (keV);Counts",
                                              bin.title.Data());
    bin.spectrum = new TH1F(specName, specTitle, 4096, 0.0, 4096.0);

    const TString matrixName = TString::Format("Mult3GammaGammaMatrix_%s", bin.tag.Data());
    const TString matrixTitle = TString::Format("Mult3GammaGammaMatrix %s;Gamma energy (keV);Gamma energy (keV)",
                                                bin.title.Data());
    bin.matrix = new TH2F(matrixName, matrixTitle, 4096, 0.0, 4096.0, 4096, 0.0, 4096.0);
    bins.push_back(bin);
    lowEnergy = highEnergy;
  }

  // EN: Filled only after the same fission-candidate cuts as the 1D spectra and matrices.
  // CN: 只在通过与一维谱/符合矩阵相同的裂变候选 cut 后填充。
  const TString multiplicityTitle = TString::Format("Mult>=%d fission candidates", minCloverMultiplicity);
  TH2F *gammaEnergyVsTime = new TH2F(
      "Mult3GammaEnergyVsTime",
      TString::Format("%s;Clover addback gamma energy (keV);Clover Time (ns)", multiplicityTitle.Data()),
      4096, 0.0, 4096.0, 1600, 0.0, 1600.0);

  TChain chain("TreeMaster");
  for (const auto &input : SplitCsv(inputFiles)) {
    chain.Add(input);
  }

  if (chain.GetEntries() <= 0) {
    std::cerr << "No entries found in input: " << inputFiles << std::endl;
    return;
  }

  const TString cloverBranchName = ResolveBranch(&chain, "f_E877_Clover");
  const TString energyBranchName = ResolveBranch(&chain, "f_E877_Clover_E");
  const TString timeBranchName = ResolveBranch(&chain, "f_E877_Clover_T");
  const TString bgoBranchName = ResolveBranch(&chain, "f_E877_Clover_BGO");
  const TString csiBranchName = ResolveBranch(&chain, "f_E877_Clover_CSI");

  if (cloverBranchName.IsNull() || energyBranchName.IsNull() || timeBranchName.IsNull()) {
    std::cerr << "Cannot find required f_E877_Clover, f_E877_Clover_E, or f_E877_Clover_T branches." << std::endl;
    return;
  }
  if (useBgoCsiVeto && (bgoBranchName.IsNull() || csiBranchName.IsNull())) {
    std::cerr << "BGO/CSI veto was requested, but BGO or CSI branches are missing." << std::endl;
    return;
  }

  TTreeReader reader(&chain);
  TTreeReaderArray<UShort_t> clovers(reader, cloverBranchName.Data());
  TTreeReaderArray<float> energies(reader, energyBranchName.Data());
  TTreeReaderArray<float> times(reader, timeBranchName.Data());
  std::unique_ptr<TTreeReaderArray<float>> bgo;
  std::unique_ptr<TTreeReaderArray<float>> csi;
  if (useBgoCsiVeto) {
    bgo.reset(new TTreeReaderArray<float>(reader, bgoBranchName.Data()));
    csi.reset(new TTreeReaderArray<float>(reader, csiBranchName.Data()));
  }

  Long64_t totalEntries = chain.GetEntries();
  if (maxEntries > 0 && maxEntries < totalEntries) totalEntries = maxEntries;

  Long64_t keptAfterTimeCut = 0;
  Long64_t skippedMultiplicity = 0;
  Long64_t skippedNoBin = 0;
  Long64_t skippedMalformed = 0;

  Long64_t entry = 0;
  while (reader.Next()) {
    if (entry >= totalEntries) break;
    entry++;

    const std::size_t nClover = static_cast<std::size_t>(clovers.GetSize());
    const std::size_t nEnergy = static_cast<std::size_t>(energies.GetSize());
    const std::size_t nTime = static_cast<std::size_t>(times.GetSize());
    if (nClover != nEnergy || nClover != nTime) {
      skippedMalformed++;
      continue;
    }
    const std::size_t n = nClover;
    if (n == 0) {
      skippedMultiplicity++;
      continue;
    }

    std::vector<float> selectedEnergies;
    std::vector<float> selectedTimes;
    selectedEnergies.reserve(n);
    selectedTimes.reserve(n);
    double eventTimeNs = std::numeric_limits<double>::infinity();

    for (std::size_t i = 0; i < n; ++i) {
      const float energy = energies[i];
      const float time = times[i];
      // EN: Gamma zero-energy deposits and invalid/nonpositive times are not physical gamma fires.
      // CN: gamma 能量为 0 或时间非正的条目不作为有效 gamma fire。
      if (energy <= 0.0f || time <= 0.0f) continue;
      if (useBgoCsiVeto) {
        const std::size_t nveto = std::min(static_cast<std::size_t>(bgo->GetSize()),
                                           static_cast<std::size_t>(csi->GetSize()));
        if (i >= nveto) continue;
        // EN: Veto threshold is any positive BGO/CSI signal in the stored E877 clover branch.
        // CN: veto 阈值定义为 E877 clover 分支里 BGO/CSI 任意正值响应。
        if ((*bgo)[i] > 0.0f || (*csi)[i] > 0.0f) continue;
      }
      // EN: Remove clover fires earlier than the physical high-energy time cut.
      // CN: 去掉早于最高能中子物理时间 cut 的 clover fire。
      if (time < tCutNs) continue;

      selectedEnergies.push_back(energy);
      selectedTimes.push_back(time);
      if (time < eventTimeNs) eventTimeNs = time;
    }

    // EN: Fission-candidate definition after all per-fire cuts: at least N clover fires remain.
    // CN: 所有单个 fire cut 之后，剩余 clover fire 数 >=N 才作为裂变候选事件。
    if (selectedEnergies.size() < static_cast<std::size_t>(minCloverMultiplicity)) {
      skippedMultiplicity++;
      continue;
    }

    // EN: Event time is the earliest accepted clover time; binning is therefore event-level.
    // CN: 事件时间取通过 cut 的 clover fire 中最早时间，因此能量/时间分 bin 是 event 级。
    const int binIndex = FindEnergyTimeBin(bins, eventTimeNs);
    if (binIndex < 0) {
      skippedNoBin++;
      continue;
    }

    keptAfterTimeCut++;
    bins[binIndex].selectedEvents++;
    bins[binIndex].selectedGammaFires += static_cast<Long64_t>(selectedEnergies.size());
    for (float energy : selectedEnergies) {
      bins[binIndex].spectrum->Fill(energy);
    }
    for (std::size_t i = 0; i < selectedEnergies.size(); ++i) {
      // EN: Same fission-candidate cuts as the spectra; fill one point per accepted clover fire.
      // CN: 使用与裂变候选谱相同的判选条件；每个通过 cut 的 clover fire 填一次。
      gammaEnergyVsTime->Fill(selectedEnergies[i], selectedTimes[i]);
    }
    for (std::size_t i = 0; i < selectedEnergies.size(); ++i) {
      for (std::size_t j = i + 1; j < selectedEnergies.size(); ++j) {
        const float a = selectedEnergies[i];
        const float b = selectedEnergies[j];
        bins[binIndex].matrix->Fill(a, b);
        bins[binIndex].matrix->Fill(b, a);
      }
    }
  }

  TString outName = (outputFile && TString(outputFile).Length() > 0)
                        ? TString(outputFile)
                        : BuildDefaultOutputName(inputFiles);
  TFile out(outName, "RECREATE");
  if (!out.IsOpen()) {
    std::cerr << "Cannot create output file: " << outName << std::endl;
    return;
  }

  TDirectory *histDir = out.mkdir("fission_event_ana");
  histDir->cd();

  const TString configTitle = TString::Format("input=%s; distance_m=%.6g; time_fwhm_ns=%.6g; t_min_50MeV_ns=%.6g; t_cut_ns=%.6g; energy_edges_MeV=%s; use_bgo_csi_veto=%d; min_clover_multiplicity=%d",
                                             inputFiles, nfsDistanceMeter, timeFwhmNs,
                                             tMinNs, tCutNs, energyBinEdgesMeV,
                                             useBgoCsiVeto ? 1 : 0,
                                             minCloverMultiplicity);
  TNamed config("FissionEventAnaConfig", configTitle.Data());
  config.Write();

  Long64_t acceptedGammaFires = 0;
  for (const auto &bin : bins) acceptedGammaFires += bin.selectedGammaFires;

  TH1F summary("FissionEventAnaSummary", "FissionEventAnaSummary;Category;Counts", 6, 0.5, 6.5);
  summary.GetXaxis()->SetBinLabel(1, "Input entries");
  summary.GetXaxis()->SetBinLabel(2, "Accepted events");
  summary.GetXaxis()->SetBinLabel(3, "Accepted clover fires");
  summary.GetXaxis()->SetBinLabel(4, "Rejected mult/time/veto");
  summary.GetXaxis()->SetBinLabel(5, "Rejected no bin");
  summary.GetXaxis()->SetBinLabel(6, "Malformed");
  summary.SetBinContent(1, totalEntries);
  summary.SetBinContent(2, keptAfterTimeCut);
  summary.SetBinContent(3, acceptedGammaFires);
  summary.SetBinContent(4, skippedMultiplicity);
  summary.SetBinContent(5, skippedNoBin);
  summary.SetBinContent(6, skippedMalformed);
  summary.Write();

  TH1F binEventCounts("FissionEventAnaEventsByEnergyBin", "FissionEventAnaEventsByEnergyBin;Neutron energy bin;Events", bins.size(), 0.5, bins.size()+0.5);
  TH1F binGammaCounts("FissionEventAnaGammaFiresByEnergyBin", "FissionEventAnaGammaFiresByEnergyBin;Neutron energy bin;Clover fires", bins.size(), 0.5, bins.size()+0.5);
  for (std::size_t i = 0; i < bins.size(); ++i) {
    binEventCounts.GetXaxis()->SetBinLabel(i+1, bins[i].tag.Data());
    binGammaCounts.GetXaxis()->SetBinLabel(i+1, bins[i].tag.Data());
    binEventCounts.SetBinContent(i+1, bins[i].selectedEvents);
    binGammaCounts.SetBinContent(i+1, bins[i].selectedGammaFires);
  }
  binEventCounts.Write();
  binGammaCounts.Write();

  gammaEnergyVsTime->Write();
  WriteCanvasForMatrix(histDir, gammaEnergyVsTime, multiplicityTitle);

  for (auto &bin : bins) {
    bin.spectrum->Write();
    bin.matrix->Write();
    WriteCanvasForSpectrum(histDir, bin.spectrum, bin.title);
    WriteCanvasForMatrix(histDir, bin.matrix, bin.title);
  }

  out.Close();

  std::cout << "Input entries: " << totalEntries << std::endl;
  std::cout << "t_min(50 MeV): " << tMinNs << " ns" << std::endl;
  std::cout << "t_cut: " << tCutNs << " ns" << std::endl;
  std::cout << "Min clover multiplicity after cuts: " << minCloverMultiplicity << std::endl;
  std::cout << "Accepted events: " << keptAfterTimeCut << std::endl;
  std::cout << "Accepted clover fires: " << acceptedGammaFires << std::endl;
  std::cout << "Rejected by multiplicity/time/veto: " << skippedMultiplicity << std::endl;
  std::cout << "Rejected outside energy bins: " << skippedNoBin << std::endl;
  std::cout << "Malformed entries: " << skippedMalformed << std::endl;
  std::cout << "Output: " << outName << std::endl;
}
