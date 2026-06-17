// Extract NFS EXOGAM crystal energy peak positions.
// 提取 NFS EXOGAM 每个 crystal 能量谱在指定窗口内的峰位。
//
// Run from nfs-th232-analysis-adne / 建议在 nfs-th232-analysis-adne 目录下运行:
// root -l -b -q 'lsy_nfs/extract_nfs_energy_peaks.C("out/nfs_histoExogam2_1.root")'

#include <algorithm>
#include <iostream>

#include "TFile.h"
#include "TGraph.h"
#include "TH1.h"
#include "TString.h"
#include "TSystem.h"
#include "TTree.h"

void extract_nfs_energy_peaks(
    const char *inputFile = "out/nfs_histoExogam2_1.root",
    double emin = 1440.0,
    double emax = 1470.0)
{
  TFile *fin = TFile::Open(inputFile, "READ");
  if (!fin || fin->IsZombie()) {
    std::cerr << "Cannot open input file / 无法打开输入文件: "
              << inputFile << std::endl;
    return;
  }

  TString inputPath(inputFile);
  TString outDir = gSystem->DirName(inputPath);
  TString baseName = gSystem->BaseName(inputPath);
  baseName.ReplaceAll(".root", "");
  TString outputPath = Form("%s/%s_peak_%.0f_%.0f.root",
                            outDir.Data(), baseName.Data(), emin, emax);

  TFile fout(outputPath, "RECREATE");
  if (fout.IsZombie()) {
    std::cerr << "Cannot create output file / 无法创建输出文件: "
              << outputPath << std::endl;
    fin->Close();
    return;
  }

  // Detector number follows ADNE MapFinger: detector = clover * 4 + crystal.
  // 探测器编号沿用 ADNE 的 MapFinger: detector = clover * 4 + crystal。
  TH1D *hPeak = new TH1D("nfs_energy_peak_1440_1470_by_detector",
                         "NFS energy peak in [1440,1470];Detector number (clover*4+crystal);Peak energy (keV)",
                         64, -0.5, 63.5);
  TH1D *hPeakCounts = new TH1D("nfs_energy_peak_counts_1440_1470_by_detector",
                               "NFS peak-bin counts in [1440,1470];Detector number (clover*4+crystal);Peak-bin counts",
                               64, -0.5, 63.5);

  TGraph *gPeak = new TGraph();
  gPeak->SetName("nfs_energy_peak_1440_1470_graph");
  gPeak->SetTitle("NFS energy peak in [1440,1470];Detector number (clover*4+crystal);Peak energy (keV)");

  TTree *tree = new TTree("nfs_energy_peak_1440_1470_table",
                          "NFS energy peak table in [1440,1470]");
  Int_t clover = -1;
  Int_t crystal = -1;
  Int_t detector = -1;
  Double_t peakEnergy = 0.0;
  Double_t peakCounts = 0.0;
  Double_t entries = 0.0;
  tree->Branch("clover", &clover);
  tree->Branch("crystal", &crystal);
  tree->Branch("detector", &detector);
  tree->Branch("peakEnergy", &peakEnergy);
  tree->Branch("peakCounts", &peakCounts);
  tree->Branch("entries", &entries);

  int point = 0;
  for (clover = 0; clover < 16; ++clover) {
    for (crystal = 0; crystal < 4; ++crystal) {
      detector = clover * 4 + crystal;
      TString histName = Form("nfs_clover%d_crystal%d_energy", clover, crystal);
      TH1 *h = dynamic_cast<TH1 *>(fin->Get(histName));
      if (!h) {
        continue;
      }

      entries = h->GetEntries();
      int firstBin = std::max(1, h->GetXaxis()->FindBin(emin));
      int lastBin = std::min(h->GetNbinsX(), h->GetXaxis()->FindBin(emax));

      int bestBin = firstBin;
      peakCounts = -1.0;
      for (int bin = firstBin; bin <= lastBin; ++bin) {
        double counts = h->GetBinContent(bin);
        if (counts > peakCounts) {
          peakCounts = counts;
          bestBin = bin;
        }
      }

      if (peakCounts <= 0.0) {
        peakEnergy = 0.0;
        peakCounts = 0.0;
      } else {
        peakEnergy = h->GetBinCenter(bestBin);
        hPeak->SetBinContent(detector + 1, peakEnergy);
        hPeakCounts->SetBinContent(detector + 1, peakCounts);
        gPeak->SetPoint(point++, detector, peakEnergy);
      }

      tree->Fill();
      std::cout << histName << " detector=" << detector
                << " peakEnergy=" << peakEnergy
                << " peakCounts=" << peakCounts
                << " entries=" << entries << std::endl;
    }
  }

  fout.cd();
  hPeak->Write();
  hPeakCounts->Write();
  gPeak->Write();
  tree->Write();
  fout.Close();
  fin->Close();

  std::cout << "Wrote / 已写入: " << outputPath << std::endl;
}
