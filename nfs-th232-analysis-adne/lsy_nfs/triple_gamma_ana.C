// Triple-gamma TH3D cube builder from selected NFS mult trees.
// 基于已经筛选后的 NFS mult tree 构建 triple-gamma 的 TH3D 三维符合矩阵。
//
// Usage / 用法:
//   root -l -b -q 'lsy_nfs/triple_gamma_ana.C("/path/to/output_dir","/path/to/triple_gamma.root")'
//   root -l -b -q 'lsy_nfs/triple_gamma_ana.C("@mult3_files.txt","triple_gamma.root")'
//   root -l -b -q 'lsy_nfs/triple_gamma_ana.C("a.root,b.root","triple_gamma.root",true,-1,3,256,0,4096)'
//
// Important memory note / 重要内存说明:
//   EN: This macro intentionally uses TH3D as requested.  A 4096^3 TH3D is far
//       too large, especially because this macro writes one total cube plus one
//       cube per neutron-energy bin.  The default gamma binning is therefore
//       256 bins over 0-4096 keV.  If a requested binning exceeds the memory
//       guard, the macro exits before allocating histograms.
//   CN: 该宏按要求使用 TH3D。4096^3 的 TH3D 极大，而且这里会同时写一个总 cube
//       和多个中子能量分区 cube。因此默认 gamma 能量分箱为 0-4096 keV 上的 256 bins。
//       如果用户请求的分箱超过内存保护阈值，宏会在创建直方图前退出。
//
// Input / 输入:
//   EN: inputSources can be a directory, a ROOT file, comma-separated items, or
//       "@list.txt". Directories are searched recursively for mult3_nfs_run_*.root.
//   CN: inputSources 可以是目录、ROOT 文件、逗号分隔列表，或 "@list.txt"。
//       对目录会递归查找 mult3_nfs_run_*.root。
//
// Event logic / 事件逻辑:
//   EN: Uses stored E877 clover addback branches:
//       f_E877_Clover_E, f_E877_Clover_T, f_E877_Clover_BGO, f_E877_Clover_CSI.
//       By default BGO/CSI-vetoed clover fires are removed.  Gamma energy must
//       be positive and inside the requested histogram range.  The event time is
//       the earliest accepted clover time, converted to neutron energy with the
//       same non-relativistic TOF formula used by the NFS analysis.
//       For energies [a,b,c], all six permutations are filled.
//   CN: 使用 mult tree 中已经保存的 E877 clover addback 分支：
//       f_E877_Clover_E, f_E877_Clover_T, f_E877_Clover_BGO, f_E877_Clover_CSI。
//       默认去掉有 BGO/CSI 响应的 clover fire。gamma 能量必须为正且落在指定范围内。
//       事件时间取通过 cut 的 clover fire 中最早的时间，并用 NFS 分析相同的非相对论
//       TOF 公式转换成中子能量。若能量为 [a,b,c]，则填充 6 个排列。
//
// Default neutron-energy bins / 默认中子能量分区:
//   0-10, 10-15, 15-20, 20-30, 30-40 MeV, plus Total.
//   0-10、10-15、15-20、20-30、30-40 MeV，另外还有一个总的结果。

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "TBranch.h"
#include "TChain.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH3D.h"
#include "TNamed.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TString.h"
#include "TSystem.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"

namespace {

constexpr double kNeutronMassMeV = 939.5654;
constexpr double kLightSpeedMeterPerNs = 0.299792458;

std::string Trim(std::string s)
{
  const auto notSpace = [](unsigned char c) { return !std::isspace(c); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
  s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
  return s;
}

double ToFNsToEnergyMeV(double tofNs, double distanceMeter)
{
  if (tofNs <= 0.0 || distanceMeter <= 0.0) return -1.0;
  const double beta = distanceMeter / (tofNs * kLightSpeedMeterPerNs);
  return 0.5 * kNeutronMassMeV * beta * beta;
}

std::vector<TString> ReadFileList(const TString &listPath)
{
  std::vector<TString> items;
  std::ifstream input(listPath.Data());
  if (!input.is_open()) {
    std::cerr << "Cannot open input list: " << listPath << std::endl;
    return items;
  }

  std::string line;
  while (std::getline(input, line)) {
    line = Trim(line);
    if (line.empty() || line.rfind("#", 0) == 0 || line.rfind("//", 0) == 0) continue;
    items.emplace_back(line.c_str());
  }
  return items;
}

std::vector<TString> SplitCsvOrList(const char *text)
{
  std::vector<TString> items;
  TString input(text ? text : "");
  input = input.Strip(TString::kBoth);
  if (input.IsNull()) return items;

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

std::vector<double> ParseEdges(const char *text)
{
  std::vector<double> edges;
  for (const auto &item : SplitCsvOrList(text)) {
    const double value = item.Atof();
    if (value > 0.0 && std::isfinite(value)) edges.push_back(value);
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

bool HasMultRootName(const std::filesystem::path &path, const char *filePrefix)
{
  const std::string name = path.filename().string();
  const std::string prefix = filePrefix ? filePrefix : "mult3_nfs_run_";
  return path.extension() == ".root" && name.rfind(prefix, 0) == 0;
}

std::vector<TString> DiscoverInputFiles(const char *inputSources, const char *filePrefix)
{
  // EN: Keep a set to avoid double counting if paths overlap.
  // CN: 用 set 去重，避免目录和文件同时给出时重复计数。
  std::set<std::string> unique;
  std::vector<TString> files;

  for (const auto &source : SplitCsvOrList(inputSources)) {
    TString expanded = source;
    gSystem->ExpandPathName(expanded);
    std::filesystem::path path(expanded.Data());

    std::error_code ec;
    if (std::filesystem::is_directory(path, ec)) {
      for (std::filesystem::recursive_directory_iterator it(path, ec), end; it != end; it.increment(ec)) {
        if (ec) {
          ec.clear();
          continue;
        }
        if (!it->is_regular_file(ec)) continue;
        if (!HasMultRootName(it->path(), filePrefix)) continue;
        const std::string abs = std::filesystem::absolute(it->path(), ec).string();
        if (unique.insert(abs).second) files.emplace_back(abs.c_str());
      }
    } else if (std::filesystem::is_regular_file(path, ec)) {
      const std::string abs = std::filesystem::absolute(path, ec).string();
      if (unique.insert(abs).second) files.emplace_back(abs.c_str());
    } else {
      std::cerr << "Warning: input source does not exist or is not readable: " << source << std::endl;
    }
  }

  std::sort(files.begin(), files.end(), [](const TString &a, const TString &b) {
    return std::string(a.Data()) < std::string(b.Data());
  });
  return files;
}

TString ResolveBranch(TTree *tree, const char *leafName)
{
  // EN: ADNE may store split branches directly or below Exogam2.*.
  // CN: ADNE 可能把分支直接拆开保存，也可能放在 Exogam2.* 下。
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

TString BuildDefaultOutputName(const char *inputSources)
{
  const auto sources = SplitCsvOrList(inputSources);
  if (sources.empty()) return "triple_gamma_ana.root";

  TString first = sources.front();
  if (first.BeginsWith("@")) first.Remove(0, 1);
  Ssiz_t slash = first.Last('/');
  if (slash == kNPOS) return "triple_gamma_ana.root";
  return first(0, slash + 1) + "triple_gamma_ana.root";
}

struct NeutronBin {
  double lowMeV = 0.0;
  double highMeV = 0.0;
  TString tag;
  TString title;
  TH3D *cube = nullptr;
  Long64_t acceptedEvents = 0;
  Long64_t acceptedTriples = 0;
};

std::vector<NeutronBin> MakeNeutronBins(const char *edgesText)
{
  std::vector<NeutronBin> bins;
  double low = 0.0;
  for (double high : ParseEdges(edgesText)) {
    if (high <= low) continue;
    NeutronBin bin;
    bin.lowMeV = low;
    bin.highMeV = high;
    bin.tag = TString::Format("E%s_%sMeV", FormatNumberForName(low).Data(), FormatNumberForName(high).Data());
    bin.title = TString::Format("E_{n} %.6g-%.6g MeV", low, high);
    bins.push_back(bin);
    low = high;
  }
  return bins;
}

int FindNeutronBin(const std::vector<NeutronBin> &bins, double neutronEnergyMeV)
{
  for (std::size_t i = 0; i < bins.size(); ++i) {
    if (neutronEnergyMeV >= bins[i].lowMeV && neutronEnergyMeV < bins[i].highMeV) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

double EstimateTH3DStorageGiB(int energyBins, std::size_t cubeCount)
{
  const double n = static_cast<double>(energyBins) + 2.0;
  const double bytes = n * n * n * static_cast<double>(sizeof(Double_t)) * static_cast<double>(cubeCount);
  return bytes / 1024.0 / 1024.0 / 1024.0;
}

TH3D *MakeCube(const TString &name, const TString &title,
               int energyBins, double energyMinKeV, double energyMaxKeV)
{
  TH3D *hist = new TH3D(name, title,
                        energyBins, energyMinKeV, energyMaxKeV,
                        energyBins, energyMinKeV, energyMaxKeV,
                        energyBins, energyMinKeV, energyMaxKeV);
  hist->GetXaxis()->SetTitle("Gamma energy 1 (keV)");
  hist->GetYaxis()->SetTitle("Gamma energy 2 (keV)");
  hist->GetZaxis()->SetTitle("Gamma energy 3 (keV)");
  return hist;
}

void FillOrderedTriple(TH3D *cube, double a, double b, double c)
{
  // EN: Fill all permutations so projections behave like symmetric coincidence matrices.
  // CN: 填充所有排列，使投影保持与对称符合矩阵一致的习惯。
  cube->Fill(a, b, c);
  cube->Fill(a, c, b);
  cube->Fill(b, a, c);
  cube->Fill(b, c, a);
  cube->Fill(c, a, b);
  cube->Fill(c, b, a);
}

} // namespace

void triple_gamma_ana(const char *inputSources,
                      const char *outputFile = "",
                      bool useBgoCsiVeto = true,
                      Long64_t maxEntries = -1,
                      int minCloverMultiplicity = 3,
                      int energyBins = 256,
                      double energyMinKeV = 0.0,
                      double energyMaxKeV = 4096.0,
                      const char *filePrefix = "mult3_nfs_run_",
                      double nfsDistanceMeter = 24.0,
                      const char *neutronEnergyEdgesMeV = "10,15,20,30,40",
                      double maxCubeMemoryGiB = 12.0)
{
  if (!inputSources || TString(inputSources).Strip(TString::kBoth).IsNull()) {
    std::cerr << "No input source was provided." << std::endl;
    return;
  }
  if (minCloverMultiplicity < 3) {
    std::cerr << "Triple-gamma analysis requires minCloverMultiplicity >= 3." << std::endl;
    return;
  }
  if (energyBins <= 0 || energyMaxKeV <= energyMinKeV) {
    std::cerr << "Invalid energy histogram range." << std::endl;
    return;
  }
  if (nfsDistanceMeter <= 0.0) {
    std::cerr << "Invalid NFS distance: " << nfsDistanceMeter << std::endl;
    return;
  }

  std::vector<NeutronBin> neutronBins = MakeNeutronBins(neutronEnergyEdgesMeV);
  if (neutronBins.empty()) {
    std::cerr << "No valid neutron-energy edges were provided: " << neutronEnergyEdgesMeV << std::endl;
    return;
  }

  // EN: One total cube plus one cube per neutron bin.
  // CN: 一个总 cube，加上每个中子能量分区各一个 cube。
  const std::size_t cubeCount = neutronBins.size() + 1;
  const double estimatedGiB = EstimateTH3DStorageGiB(energyBins, cubeCount);
  if (estimatedGiB > maxCubeMemoryGiB) {
    std::cerr << "Requested TH3D allocation is too large." << std::endl;
    std::cerr << "  energyBins=" << energyBins << " cubes=" << cubeCount
              << " estimated_storage=" << estimatedGiB << " GiB"
              << " limit=" << maxCubeMemoryGiB << " GiB" << std::endl;
    std::cerr << "  Reduce energyBins or increase maxCubeMemoryGiB only if the machine has enough RAM." << std::endl;
    return;
  }

  std::vector<TString> files = DiscoverInputFiles(inputSources, filePrefix);
  if (files.empty()) {
    std::cerr << "No input ROOT files found from: " << inputSources << std::endl;
    std::cerr << "Directory search pattern: " << filePrefix << "*.root" << std::endl;
    return;
  }

  TChain chain("TreeMaster");
  for (const auto &file : files) chain.Add(file);
  if (chain.GetEntries() <= 0) {
    std::cerr << "No TreeMaster entries found in selected inputs." << std::endl;
    return;
  }

  const TString energyBranchName = ResolveBranch(&chain, "f_E877_Clover_E");
  const TString timeBranchName = ResolveBranch(&chain, "f_E877_Clover_T");
  const TString bgoBranchName = ResolveBranch(&chain, "f_E877_Clover_BGO");
  const TString csiBranchName = ResolveBranch(&chain, "f_E877_Clover_CSI");

  if (energyBranchName.IsNull() || timeBranchName.IsNull()) {
    std::cerr << "Cannot find required f_E877_Clover_E or f_E877_Clover_T branch." << std::endl;
    return;
  }
  if (useBgoCsiVeto && (bgoBranchName.IsNull() || csiBranchName.IsNull())) {
    std::cerr << "BGO/CSI veto requested, but f_E877_Clover_BGO or f_E877_Clover_CSI is missing." << std::endl;
    return;
  }

  TH3D *totalCube = MakeCube(
      "TripleGammaCube_Total",
      "TripleGammaCube Total accepted events;Gamma energy 1 (keV);Gamma energy 2 (keV);Gamma energy 3 (keV)",
      energyBins, energyMinKeV, energyMaxKeV);

  for (auto &bin : neutronBins) {
    bin.cube = MakeCube(
        TString::Format("TripleGammaCube_%s", bin.tag.Data()),
        TString::Format("TripleGammaCube %s;Gamma energy 1 (keV);Gamma energy 2 (keV);Gamma energy 3 (keV)",
                        bin.title.Data()),
        energyBins, energyMinKeV, energyMaxKeV);
  }

  TTreeReader reader(&chain);
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

  Long64_t scannedEntries = 0;
  Long64_t acceptedEvents = 0;
  Long64_t acceptedTriples = 0;
  Long64_t neutronBinnedEvents = 0;
  Long64_t skippedMultiplicity = 0;
  Long64_t skippedMalformed = 0;
  Long64_t skippedNoNeutronBin = 0;
  Long64_t vetoedFires = 0;
  Long64_t outOfRangeFires = 0;

  while (reader.Next()) {
    if (scannedEntries >= totalEntries) break;
    scannedEntries++;

    const std::size_t nEnergy = static_cast<std::size_t>(energies.GetSize());
    const std::size_t nTime = static_cast<std::size_t>(times.GetSize());
    if (nTime < nEnergy) {
      skippedMalformed++;
      continue;
    }
    if (useBgoCsiVeto) {
      const std::size_t nBgo = static_cast<std::size_t>(bgo->GetSize());
      const std::size_t nCsi = static_cast<std::size_t>(csi->GetSize());
      if (nBgo < nEnergy || nCsi < nEnergy) {
        skippedMalformed++;
        continue;
      }
    }

    std::vector<double> selectedEnergies;
    selectedEnergies.reserve(nEnergy);
    double eventTimeNs = std::numeric_limits<double>::infinity();

    for (std::size_t i = 0; i < nEnergy; ++i) {
      const double energy = energies[i];
      const double time = times[i];
      if (!std::isfinite(energy) || energy <= 0.0) continue;
      if (!std::isfinite(time) || time <= 0.0) continue;
      if (energy < energyMinKeV || energy >= energyMaxKeV) {
        outOfRangeFires++;
        continue;
      }
      if (useBgoCsiVeto && ((*bgo)[i] > 0.0f || (*csi)[i] > 0.0f)) {
        vetoedFires++;
        continue;
      }
      selectedEnergies.push_back(energy);
      if (time < eventTimeNs) eventTimeNs = time;
    }

    if (selectedEnergies.size() < static_cast<std::size_t>(minCloverMultiplicity)) {
      skippedMultiplicity++;
      continue;
    }

    const double neutronEnergyMeV = ToFNsToEnergyMeV(eventTimeNs, nfsDistanceMeter);
    const int neutronBinIndex = FindNeutronBin(neutronBins, neutronEnergyMeV);

    acceptedEvents++;
    if (neutronBinIndex >= 0) neutronBinnedEvents++;

    for (std::size_t i = 0; i < selectedEnergies.size(); ++i) {
      for (std::size_t j = i + 1; j < selectedEnergies.size(); ++j) {
        for (std::size_t k = j + 1; k < selectedEnergies.size(); ++k) {
          FillOrderedTriple(totalCube, selectedEnergies[i], selectedEnergies[j], selectedEnergies[k]);
          if (neutronBinIndex >= 0) {
            FillOrderedTriple(neutronBins[neutronBinIndex].cube, selectedEnergies[i], selectedEnergies[j], selectedEnergies[k]);
            neutronBins[neutronBinIndex].acceptedTriples++;
          }
          acceptedTriples++;
        }
      }
    }

    if (neutronBinIndex >= 0) {
      neutronBins[neutronBinIndex].acceptedEvents++;
    } else {
      skippedNoNeutronBin++;
    }
  }

  TString outName = (outputFile && TString(outputFile).Length() > 0)
                        ? TString(outputFile)
                        : BuildDefaultOutputName(inputSources);
  TFile out(outName, "RECREATE");
  if (!out.IsOpen()) {
    std::cerr << "Cannot create output file: " << outName << std::endl;
    return;
  }

  TDirectory *dir = out.mkdir("triple_gamma_ana");
  dir->cd();

  TNamed config("TripleGammaAnaConfig",
                TString::Format("input_sources=%s; file_prefix=%s; input_files=%zu; use_bgo_csi_veto=%d; max_entries=%lld; min_clover_multiplicity=%d; energy_bins=%d; energy_min_keV=%.12g; energy_max_keV=%.12g; nfs_distance_m=%.12g; neutron_edges_MeV=%s; estimated_th3d_storage_GiB=%.6g; memory_guard_GiB=%.6g",
                                inputSources, filePrefix, files.size(), useBgoCsiVeto ? 1 : 0,
                                static_cast<long long>(maxEntries), minCloverMultiplicity,
                                energyBins, energyMinKeV, energyMaxKeV, nfsDistanceMeter,
                                neutronEnergyEdgesMeV, estimatedGiB, maxCubeMemoryGiB).Data());
  config.Write();

  TH1F summary("TripleGammaAnaSummary", "TripleGammaAnaSummary;Category;Counts", 9, 0.5, 9.5);
  summary.GetXaxis()->SetBinLabel(1, "Input files");
  summary.GetXaxis()->SetBinLabel(2, "Scanned entries");
  summary.GetXaxis()->SetBinLabel(3, "Accepted events");
  summary.GetXaxis()->SetBinLabel(4, "Neutron-binned events");
  summary.GetXaxis()->SetBinLabel(5, "Accepted unordered triples");
  summary.GetXaxis()->SetBinLabel(6, "Rejected multiplicity");
  summary.GetXaxis()->SetBinLabel(7, "Rejected no neutron bin");
  summary.GetXaxis()->SetBinLabel(8, "Malformed events");
  summary.GetXaxis()->SetBinLabel(9, "Veto/out-of-range fires");
  summary.SetBinContent(1, static_cast<double>(files.size()));
  summary.SetBinContent(2, static_cast<double>(scannedEntries));
  summary.SetBinContent(3, static_cast<double>(acceptedEvents));
  summary.SetBinContent(4, static_cast<double>(neutronBinnedEvents));
  summary.SetBinContent(5, static_cast<double>(acceptedTriples));
  summary.SetBinContent(6, static_cast<double>(skippedMultiplicity));
  summary.SetBinContent(7, static_cast<double>(skippedNoNeutronBin));
  summary.SetBinContent(8, static_cast<double>(skippedMalformed));
  summary.SetBinContent(9, static_cast<double>(vetoedFires + outOfRangeFires));
  summary.Write();

  TH1F neutronBinEvents("TripleGammaEventsByNeutronEnergy",
                        "Triple-gamma accepted events by neutron-energy bin;Neutron energy bin;Events",
                        neutronBins.size(), 0.5, neutronBins.size() + 0.5);
  TH1F neutronBinTriples("TripleGammaTriplesByNeutronEnergy",
                         "Triple-gamma unordered triples by neutron-energy bin;Neutron energy bin;Unordered triples",
                         neutronBins.size(), 0.5, neutronBins.size() + 0.5);
  for (std::size_t i = 0; i < neutronBins.size(); ++i) {
    neutronBinEvents.GetXaxis()->SetBinLabel(i + 1, neutronBins[i].tag.Data());
    neutronBinTriples.GetXaxis()->SetBinLabel(i + 1, neutronBins[i].tag.Data());
    neutronBinEvents.SetBinContent(i + 1, neutronBins[i].acceptedEvents);
    neutronBinTriples.SetBinContent(i + 1, neutronBins[i].acceptedTriples);
  }
  neutronBinEvents.Write();
  neutronBinTriples.Write();

  TH1F inputFileHist("TripleGammaInputFiles", "Input mult ROOT files;File index;Entries in TreeMaster", files.size(), 0.5, files.size() + 0.5);
  for (std::size_t i = 0; i < files.size(); ++i) {
    inputFileHist.GetXaxis()->SetBinLabel(i + 1, gSystem->BaseName(files[i]));
    TFile f(files[i], "READ");
    TTree *t = f.IsOpen() ? dynamic_cast<TTree *>(f.Get("TreeMaster")) : nullptr;
    inputFileHist.SetBinContent(i + 1, t ? t->GetEntries() : 0.0);
  }

  // EN: Re-enter the output directory because opening input ROOT files changes gDirectory.
  // CN: 上面为了统计输入 entries 打开了输入 ROOT 文件，会改变 gDirectory，因此写出前切回输出目录。
  dir->cd();
  inputFileHist.Write();

  totalCube->Write();
  for (auto &bin : neutronBins) {
    if (bin.cube) bin.cube->Write();
  }
  out.Close();

  std::cout << "Input files: " << files.size() << std::endl;
  std::cout << "Scanned entries: " << scannedEntries << std::endl;
  std::cout << "Accepted events: " << acceptedEvents << std::endl;
  std::cout << "Neutron-binned events: " << neutronBinnedEvents << std::endl;
  std::cout << "Accepted unordered triples: " << acceptedTriples << std::endl;
  std::cout << "TH3D filled permutations in total cube: " << acceptedTriples * 6 << std::endl;
  std::cout << "Rejected by multiplicity: " << skippedMultiplicity << std::endl;
  std::cout << "Rejected outside neutron bins: " << skippedNoNeutronBin << std::endl;
  std::cout << "Malformed events: " << skippedMalformed << std::endl;
  std::cout << "Vetoed fires: " << vetoedFires << std::endl;
  std::cout << "Out-of-range fires: " << outOfRangeFires << std::endl;
  std::cout << "Estimated TH3D storage: " << estimatedGiB << " GiB" << std::endl;
  for (const auto &bin : neutronBins) {
    std::cout << "  " << bin.tag << ": events=" << bin.acceptedEvents
              << " triples=" << bin.acceptedTriples << std::endl;
  }
  std::cout << "Output: " << outName << std::endl;
}
