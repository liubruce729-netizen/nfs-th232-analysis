// Build per-crystal EXOGAM energy histograms from NFS event trees.
// 从 NFS event tree 生成每个 crystal 的 EXOGAM 能量谱。
//
// CN:
//   Co-60 能量刻度不需要 NFS Time/TOF，因此这个脚本只读取 TreeMaster 里的
//   fEXO_ECC_E_Clover / fEXO_ECC_E_Cristal / fEXO_ECC_E_Energy。
//   它不使用 fNeutronTOF，也不使用 rawDeltaT/time cut。
//
// EN:
//   Co-60 energy calibration does not need NFS Time/TOF. This macro reads only
//   fEXO_ECC_E_Clover / fEXO_ECC_E_Cristal / fEXO_ECC_E_Energy from TreeMaster.
//   It does not use fNeutronTOF, rawDeltaT, or any time cut.
//
// Usage / 用法:
//   root -l -b -q 'lsy_nfs/make_co60_crystal_energy_hists.C("out/nfs_run_8_r0.root")'
//   root -l -b -q 'lsy_nfs/make_co60_crystal_energy_hists.C("out/nfs_run_8_r0.root,out/nfs_run_8_r1.root")'
//   root -l -b -q 'lsy_nfs/make_co60_crystal_energy_hists.C("out/nfs_run_8_r*.root","out/co60_crystal_energy_hists.root")'
//
// Output histogram names / 输出图名:
//   nfs_clover%d_crystal%d_energy

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include "TChain.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TNamed.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TString.h"
#include "TSystem.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"

namespace {

std::vector<TString> SplitInputFiles(const char *inputFiles)
{
  std::vector<TString> files;
  TString input(inputFiles ? inputFiles : "");
  TObjArray *tokens = input.Tokenize(",");
  for (int i = 0; i < tokens->GetEntriesFast(); ++i) {
    TString item = static_cast<TObjString *>(tokens->At(i))->GetString();
    item = item.Strip(TString::kBoth);
    if (!item.IsNull()) files.push_back(item);
  }
  delete tokens;
  return files;
}

TString DefaultOutputName(const char *inputFiles)
{
  std::vector<TString> files = SplitInputFiles(inputFiles);
  if (files.empty()) return "co60_crystal_energy_hists.root";

  TString first = files.front();
  TString outDir = gSystem->DirName(first);
  if (outDir == ".") outDir = "";
  TString prefix = outDir.IsNull() ? "" : outDir + "/";
  return prefix + "co60_crystal_energy_hists.root";
}

Double_t NiceAxisMax(Double_t requestedMax, Double_t observedMax)
{
  // EN: Keep the requested maximum if it already contains the data; otherwise
  //     expand to a rounded upper edge above the observed maximum.
  // CN: 如果用户给定上限已经包含数据，就保持不变；否则扩展到高于最大值的圆整上限。
  if (observedMax <= requestedMax) return requestedMax;

  const Double_t target = observedMax * 1.02;
  if (target <= 0.0) return requestedMax;

  const Double_t decade = std::pow(10.0, std::floor(std::log10(target)));
  const Double_t scaled = target / decade;
  Double_t nice = 10.0;
  if (scaled <= 1.0) nice = 1.0;
  else if (scaled <= 2.0) nice = 2.0;
  else if (scaled <= 5.0) nice = 5.0;

  return nice * decade;
}

} // namespace

void make_co60_crystal_energy_hists(
    const char *inputFiles,
    const char *outputFile = "",
    Long64_t maxEntries = -1,
    Int_t nbins = 8192,
    Double_t eminKeV = 0.0,
    Double_t emaxKeV = 4096.0)
{
  std::vector<TString> files = SplitInputFiles(inputFiles);
  if (files.empty()) {
    std::cerr << "No input file / 没有输入文件" << std::endl;
    return;
  }

  TString outName(outputFile ? outputFile : "");
  if (outName.IsNull()) outName = DefaultOutputName(inputFiles);

  TChain chain("TreeMaster");
  for (const auto &file : files) {
    const int added = chain.Add(file);
    std::cout << "Add input / 加入输入: " << file
              << " trees=" << added << std::endl;
  }

  const Long64_t totalEntries = chain.GetEntries();
  if (totalEntries <= 0) {
    std::cerr << "No TreeMaster entries / TreeMaster 没有 entries" << std::endl;
    return;
  }

  Long64_t entriesToProcess = totalEntries;
  if (maxEntries >= 0 && maxEntries < entriesToProcess) entriesToProcess = maxEntries;

  if (emaxKeV <= eminKeV) {
    std::cerr << "Invalid energy range / 能量范围无效: ["
              << eminKeV << ", " << emaxKeV << "]" << std::endl;
    return;
  }

  Double_t maxCrystalEnergy[16][4] = {};
  Double_t globalMaxEnergy = 0.0;
  {
    // EN: First pass: find the largest positive energy per crystal so no Co-60
    //     calibration entries are lost when the current gain is far from final.
    // CN: 第一遍：寻找每个 crystal 的最大正能量，避免当前增益未刻度时谱轴截断 Co-60 数据。
    TTreeReader scanReader(&chain);
    TTreeReaderArray<UShort_t> scanClovers(scanReader, "fEXO_ECC_E_Clover");
    TTreeReaderArray<UShort_t> scanCrystals(scanReader, "fEXO_ECC_E_Cristal");
    TTreeReaderArray<Float_t> scanEnergies(scanReader, "fEXO_ECC_E_Energy");

    Long64_t scanned = 0;
    while (scanReader.Next()) {
      if (scanned >= entriesToProcess) break;
      scanned++;

      const std::size_t nClover = static_cast<std::size_t>(scanClovers.GetSize());
      const std::size_t nCrystal = static_cast<std::size_t>(scanCrystals.GetSize());
      const std::size_t nEnergy = static_cast<std::size_t>(scanEnergies.GetSize());
      if (nClover != nCrystal || nClover != nEnergy) continue;

      for (std::size_t i = 0; i < nEnergy; ++i) {
        const int clo = static_cast<int>(scanClovers[i]);
        const int cri = static_cast<int>(scanCrystals[i]);
        const float energy = scanEnergies[i];
        if (clo < 0 || clo >= 16 || cri < 0 || cri >= 4) continue;
        if (energy <= 0.0f) continue;

        if (energy > maxCrystalEnergy[clo][cri]) maxCrystalEnergy[clo][cri] = energy;
        if (energy > globalMaxEnergy) globalMaxEnergy = energy;
      }

      if (scanned % 1000000 == 0) {
        std::cout << "Scanned / 已扫描 " << scanned << " / "
                  << entriesToProcess << std::endl;
      }
    }
  }

  const Double_t globalAxisMax = NiceAxisMax(emaxKeV, globalMaxEnergy);

  TH1F *hCrystalEnergy[16][4] = {};
  for (int clo = 0; clo < 16; ++clo) {
    for (int cri = 0; cri < 4; ++cri) {
      TString name = Form("nfs_clover%d_crystal%d_energy", clo, cri);
      TString title = Form("Clover%d Crystal%d Gamma Energy;Energy (keV);Counts", clo, cri);
      const Double_t crystalAxisMax = NiceAxisMax(emaxKeV, maxCrystalEnergy[clo][cri]);
      hCrystalEnergy[clo][cri] = new TH1F(name, title, nbins, eminKeV, crystalAxisMax);
      hCrystalEnergy[clo][cri]->Sumw2(false);
    }
  }

  TH1F *hAllCrystalEnergy = new TH1F(
      "nfs_all_crystal_energy",
      "NFS all crystal gamma energy;Energy (keV);Counts",
      nbins, eminKeV, globalAxisMax);
  TH2F *hEnergyVsDetector = new TH2F(
      "nfs_crystal_energy_vs_detector",
      "NFS crystal gamma energy by detector;Detector number (clover*4+crystal);Energy (keV)",
      64, -0.5, 63.5, nbins, eminKeV, globalAxisMax);
  TH1F *hFirePattern = new TH1F(
      "nfs_crystal_fire_pattern",
      "NFS crystal positive-energy fire pattern;Detector number (clover*4+crystal);Counts",
      64, -0.5, 63.5);
  TH1F *hMaxEnergy = new TH1F(
      "nfs_crystal_max_energy_by_detector",
      "NFS observed maximum crystal energy;Detector number (clover*4+crystal);Maximum energy (keV)",
      64, -0.5, 63.5);
  for (int clo = 0; clo < 16; ++clo) {
    for (int cri = 0; cri < 4; ++cri) {
      hMaxEnergy->SetBinContent(clo * 4 + cri + 1, maxCrystalEnergy[clo][cri]);
    }
  }

  TTreeReader reader(&chain);
  TTreeReaderArray<UShort_t> clovers(reader, "fEXO_ECC_E_Clover");
  TTreeReaderArray<UShort_t> crystals(reader, "fEXO_ECC_E_Cristal");
  TTreeReaderArray<Float_t> energies(reader, "fEXO_ECC_E_Energy");

  Long64_t processed = 0;
  Long64_t eventsWithEnergy = 0;
  Long64_t totalCrystalFires = 0;
  Long64_t skippedSizeMismatch = 0;
  Long64_t skippedInvalidId = 0;
  Long64_t skippedNonPositiveEnergy = 0;

  while (reader.Next()) {
    if (processed >= entriesToProcess) break;
    processed++;

    const std::size_t nClover = static_cast<std::size_t>(clovers.GetSize());
    const std::size_t nCrystal = static_cast<std::size_t>(crystals.GetSize());
    const std::size_t nEnergy = static_cast<std::size_t>(energies.GetSize());
    if (nClover != nCrystal || nClover != nEnergy) {
      skippedSizeMismatch++;
      continue;
    }

    bool eventHasEnergy = false;
    for (std::size_t i = 0; i < nEnergy; ++i) {
      const int clo = static_cast<int>(clovers[i]);
      const int cri = static_cast<int>(crystals[i]);
      const float energy = energies[i];

      if (clo < 0 || clo >= 16 || cri < 0 || cri >= 4) {
        skippedInvalidId++;
        continue;
      }
      if (energy <= 0.0f) {
        skippedNonPositiveEnergy++;
        continue;
      }

      const int detector = clo * 4 + cri;
      hCrystalEnergy[clo][cri]->Fill(energy);
      hAllCrystalEnergy->Fill(energy);
      hEnergyVsDetector->Fill(detector, energy);
      hFirePattern->Fill(detector);
      totalCrystalFires++;
      eventHasEnergy = true;
    }

    if (eventHasEnergy) eventsWithEnergy++;
    if (processed % 1000000 == 0) {
      std::cout << "Processed / 已处理 " << processed << " / "
                << entriesToProcess << std::endl;
    }
  }

  TFile fout(outName, "RECREATE");
  if (fout.IsZombie()) {
    std::cerr << "Cannot create output / 无法创建输出文件: "
              << outName << std::endl;
    return;
  }

  for (int clo = 0; clo < 16; ++clo) {
    for (int cri = 0; cri < 4; ++cri) {
      hCrystalEnergy[clo][cri]->Write();
    }
  }
  hAllCrystalEnergy->Write();
  hEnergyVsDetector->Write();
  hFirePattern->Write();
  hMaxEnergy->Write();

  TNamed config("make_co60_crystal_energy_hists_config",
                Form("inputs=%s; maxEntries=%lld; nbins=%d; eminKeV=%g; requestedEmaxKeV=%g; globalAxisMaxKeV=%g",
                     inputFiles, maxEntries, nbins, eminKeV, emaxKeV, globalAxisMax));
  config.Write();
  TNamed summary("make_co60_crystal_energy_hists_summary",
                 Form("treeEntries=%lld; processed=%lld; eventsWithEnergy=%lld; totalCrystalFires=%lld; globalMaxEnergy=%g; skippedSizeMismatch=%lld; skippedInvalidId=%lld; skippedNonPositiveEnergy=%lld",
                      totalEntries, processed, eventsWithEnergy, totalCrystalFires,
                      globalMaxEnergy, skippedSizeMismatch, skippedInvalidId, skippedNonPositiveEnergy));
  summary.Write();

  fout.Close();

  std::cout << "Input tree entries / 输入 tree entries: " << totalEntries << std::endl;
  std::cout << "Processed events / 已处理 events: " << processed << std::endl;
  std::cout << "Events with positive crystal energy / 有正能量 crystal 的 events: "
            << eventsWithEnergy << std::endl;
  std::cout << "Total positive crystal fires / 正能量 crystal fire 总数: "
            << totalCrystalFires << std::endl;
  std::cout << "Observed global max energy / 观测到的全局最大能量: "
            << globalMaxEnergy << " keV" << std::endl;
  std::cout << "Global histogram upper edge / 全局直方图上限: "
            << globalAxisMax << " keV" << std::endl;
  std::cout << "Skipped size mismatch / 跳过长度不一致 events: "
            << skippedSizeMismatch << std::endl;
  std::cout << "Skipped invalid crystal id / 跳过非法 crystal id: "
            << skippedInvalidId << std::endl;
  std::cout << "Skipped non-positive energy / 跳过非正能量: "
            << skippedNonPositiveEnergy << std::endl;
  std::cout << "Wrote / 已写入: " << outName << std::endl;
}

