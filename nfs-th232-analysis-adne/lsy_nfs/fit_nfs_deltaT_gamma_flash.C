// Fit gamma-flash peaks in NFS EXOGAM crystal Time/DeltaT spectra.
// 拟合 NFS EXOGAM 每个 crystal Time/DeltaT 谱中的 gamma flash 峰。
//
// Run from nfs-th232-analysis-adne / 建议在 nfs-th232-analysis-adne 目录下运行:
// root -l -b -q 'lsy_nfs/fit_nfs_deltaT_gamma_flash.C("out/nfs_histoExogam2_1.root")'

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "TCanvas.h"
#include "TDirectory.h"
#include "TF1.h"
#include "TFile.h"
#include "TFitResult.h"
#include "TGraph.h"
#include "TH1.h"
#include "TKey.h"
#include "TLegend.h"
#include "TPaveText.h"
#include "TString.h"
#include "TSystem.h"
#include "TTree.h"

namespace {

struct DeltaTHistInfo {
  TString name;
  Int_t clover;
  Int_t crystal;
  Int_t detector;
};

int FindMaximumBinInRange(TH1 *h, double xmin, double xmax)
{
  int firstBin = std::max(1, h->GetXaxis()->FindBin(xmin));
  int lastBin = std::min(h->GetNbinsX(), h->GetXaxis()->FindBin(xmax));
  int bestBin = firstBin;
  double bestCounts = -1.0;
  for (int bin = firstBin; bin <= lastBin; ++bin) {
    const double counts = h->GetBinContent(bin);
    if (counts > bestCounts) {
      bestCounts = counts;
      bestBin = bin;
    }
  }
  return bestBin;
}

TString BuildOutputPath(const char *inputFile, const char *outputFile)
{
  TString explicitOutput(outputFile);
  if (!explicitOutput.IsNull() && explicitOutput.Length() > 0) {
    return explicitOutput;
  }

  TString inputPath(inputFile);
  TString outDir = gSystem->DirName(inputPath);
  TString baseName = gSystem->BaseName(inputPath);
  baseName.ReplaceAll(".root", "");
  return Form("%s/%s_deltaT_gamma_flash_fit.root",
              outDir.Data(), baseName.Data());
}

TString BuildTextOutputPath(const TString &rootOutputPath)
{
  TString textPath(rootOutputPath);
  if (textPath.EndsWith(".root")) {
    textPath.ReplaceAll(".root", ".txt");
  } else {
    textPath += ".txt";
  }
  return textPath;
}

} // namespace

void fit_nfs_deltaT_gamma_flash(
    const char *inputFile = "out/nfs_histoExogam2_1.root",
    const char *outputFile = "",
    double offsetLow = 700.0,
    double gammaLow = 790.0,
    double gammaHigh = 850.0,
    double lateSearchHigh = 1050.0,
    double plotPaddingAfterLatePeak = 80.0)
{
  TFile *fin = TFile::Open(inputFile, "READ");
  if (!fin || fin->IsZombie()) {
    std::cerr << "Cannot open input file / 无法打开输入文件: "
              << inputFile << std::endl;
    return;
  }

  std::vector<DeltaTHistInfo> histList;
  TIter nextKey(fin->GetListOfKeys());
  TKey *key = nullptr;
  while ((key = static_cast<TKey *>(nextKey()))) {
    TString name = key->GetName();
    Int_t clover = -1;
    Int_t crystal = -1;
    bool matched = false;
    if (name.EndsWith("_time") &&
        sscanf(name.Data(), "nfs_clover%d_crystal%d_time", &clover, &crystal) == 2) {
      matched = true;
    } else if (name.EndsWith("_deltaT") &&
               sscanf(name.Data(), "nfs_clover%d_crystal%d_deltaT", &clover, &crystal) == 2) {
      matched = true;
    }
    if (matched) {
      DeltaTHistInfo info;
      info.name = name;
      info.clover = clover;
      info.crystal = crystal;
      info.detector = clover * 4 + crystal;
      histList.push_back(info);
    }
  }

  std::sort(histList.begin(), histList.end(),
            [](const DeltaTHistInfo &a, const DeltaTHistInfo &b) {
              return a.detector < b.detector;
            });
  if (histList.empty()) {
    std::cerr << "No crystal time histograms found / 未找到 crystal 时间直方图. "
              << "Expected names like nfs_clover2_crystal0_time "
              << "or legacy nfs_clover2_crystal0_deltaT." << std::endl;
  }

  TString outPath = BuildOutputPath(inputFile, outputFile);
  TString textPath = BuildTextOutputPath(outPath);
  TFile fout(outPath, "RECREATE");
  if (fout.IsZombie()) {
    std::cerr << "Cannot create output file / 无法创建输出文件: "
              << outPath << std::endl;
    fin->Close();
    return;
  }

  std::ofstream textOut(textPath.Data());
  if (!textOut.is_open()) {
    std::cerr << "Cannot create text output / 无法创建文本输出: "
              << textPath << std::endl;
    fin->Close();
    return;
  }
  textOut << "# NFS EXOGAM gamma-flash fit table for Time/DeltaT histograms\n";
  textOut << "# NFS EXOGAM Time/DeltaT gamma flash 拟合结果表\n";
  textOut << "# Columns: clover crystal detector gammaMean_ns gammaFwhm_ns gammaSigma_ns fitStatus entries\n";
  textOut << std::fixed << std::setprecision(6);

  TH1D *hMean = new TH1D("nfs_deltaT_gamma_flash_mean_by_detector",
                         "Gamma-flash mean;Detector number (clover*4+crystal);Mean #mu (ns)",
                         64, -0.5, 63.5);
  TH1D *hFwhm = new TH1D("nfs_deltaT_gamma_flash_fwhm_by_detector",
                         "Gamma-flash FWHM;Detector number (clover*4+crystal);FWHM (ns)",
                         64, -0.5, 63.5);
  TH1D *hStatus = new TH1D("nfs_deltaT_gamma_flash_fit_status_by_detector",
                           "Gamma-flash fit status;Detector number (clover*4+crystal);Fit status",
                           64, -0.5, 63.5);
  TGraph *gMean = new TGraph();
  gMean->SetName("nfs_deltaT_gamma_flash_mean_graph");
  gMean->SetTitle("Gamma-flash mean;Detector number (clover*4+crystal);Mean #mu (ns)");
  TGraph *gFwhm = new TGraph();
  gFwhm->SetName("nfs_deltaT_gamma_flash_fwhm_graph");
  gFwhm->SetTitle("Gamma-flash FWHM;Detector number (clover*4+crystal);FWHM (ns)");

  TTree *tree = new TTree("nfs_deltaT_gamma_flash_fit_table",
                          "Gamma-flash fit results for NFS Time/DeltaT spectra");
  Int_t clover = -1;
  Int_t crystal = -1;
  Int_t detector = -1;
  Int_t fitStatus = -1;
  Double_t entries = 0.0;
  Double_t fitLow = offsetLow;
  Double_t fitHigh = 0.0;
  Double_t gammaSeedMean = 0.0;
  Double_t latePeakPosition = 0.0;
  Double_t gammaMean = 0.0;
  Double_t gammaMeanErr = 0.0;
  Double_t gammaSigma = 0.0;
  Double_t gammaSigmaErr = 0.0;
  Double_t gammaFwhm = 0.0;
  Double_t gammaFwhmErr = 0.0;
  Double_t gammaAmplitude = 0.0;
  Double_t lateAmplitude = 0.0;
  Double_t lateSigma = 0.0;
  TString histName;
  tree->Branch("clover", &clover);
  tree->Branch("crystal", &crystal);
  tree->Branch("detector", &detector);
  tree->Branch("fitStatus", &fitStatus);
  tree->Branch("entries", &entries);
  tree->Branch("fitLow", &fitLow);
  tree->Branch("fitHigh", &fitHigh);
  tree->Branch("gammaSeedMean", &gammaSeedMean);
  tree->Branch("latePeakPosition", &latePeakPosition);
  tree->Branch("gammaMean", &gammaMean);
  tree->Branch("gammaMeanErr", &gammaMeanErr);
  tree->Branch("gammaSigma", &gammaSigma);
  tree->Branch("gammaSigmaErr", &gammaSigmaErr);
  tree->Branch("gammaFwhm", &gammaFwhm);
  tree->Branch("gammaFwhmErr", &gammaFwhmErr);
  tree->Branch("gammaAmplitude", &gammaAmplitude);
  tree->Branch("lateAmplitude", &lateAmplitude);
  tree->Branch("lateSigma", &lateSigma);
  tree->Branch("histName", &histName);

  TDirectory *canvasDir = fout.mkdir("fit_canvases");
  TDirectory *functionDir = fout.mkdir("fit_functions");

  int meanPoint = 0;
  int fwhmPoint = 0;
  for (const auto &info : histList) {
    histName = info.name;
    clover = info.clover;
    crystal = info.crystal;
    detector = info.detector;
    fitStatus = -1;
    entries = 0.0;
    fitLow = offsetLow;
    fitHigh = 0.0;
    gammaSeedMean = 0.0;
    latePeakPosition = 0.0;
    gammaMean = 0.0;
    gammaMeanErr = 0.0;
    gammaSigma = 0.0;
    gammaSigmaErr = 0.0;
    gammaFwhm = 0.0;
    gammaFwhmErr = 0.0;
    gammaAmplitude = 0.0;
    lateAmplitude = 0.0;
    lateSigma = 0.0;

    TH1 *h = dynamic_cast<TH1 *>(fin->Get(histName));
    if (!h || h->GetEntries() <= 0) {
      tree->Fill();
      continue;
    }

    entries = h->GetEntries();
    const double xMin = h->GetXaxis()->GetXmin();
    const double xMax = h->GetXaxis()->GetXmax();
    fitLow = std::max(offsetLow, xMin);

    // Seed the gamma flash with a simple Gaussian fit in the target window.
    // 先在目标窗口内用单高斯给 gamma flash 取初值。
    const int gammaSeedBin = FindMaximumBinInRange(h, gammaLow, gammaHigh);
    const double gammaSeedAmp = h->GetBinContent(gammaSeedBin);
    gammaSeedMean = h->GetBinCenter(gammaSeedBin);
    TF1 seedGamma(Form("seed_%s", histName.Data()), "gaus", gammaLow, gammaHigh);
    seedGamma.SetParameters(gammaSeedAmp, gammaSeedMean, 4.0);
    seedGamma.SetParLimits(0, 0.0, std::max(10.0 * gammaSeedAmp, 1.0));
    seedGamma.SetParLimits(1, gammaLow, gammaHigh);
    seedGamma.SetParLimits(2, 0.5, 30.0);
    h->Fit(&seedGamma, "RQSN0", "", gammaLow, gammaHigh);

    double seedAmp = seedGamma.GetParameter(0);
    double seedMean = seedGamma.GetParameter(1);
    double seedSigma = std::fabs(seedGamma.GetParameter(2));
    if (!std::isfinite(seedAmp) || seedAmp <= 0.0) seedAmp = gammaSeedAmp;
    if (!std::isfinite(seedMean) || seedMean < gammaLow || seedMean > gammaHigh) seedMean = gammaSeedMean;
    if (!std::isfinite(seedSigma) || seedSigma <= 0.0) seedSigma = 4.0;

    // Use the maximum after the gamma-flash region as the nearby late peak cap.
    // 使用 gamma flash 之后、指定搜索上限以内的最大 bin 作为邻近高峰峰顶。
    const double lateSearchUpper = std::min(lateSearchHigh, xMax);
    const int latePeakBin = FindMaximumBinInRange(h, gammaHigh, lateSearchUpper);
    latePeakPosition = h->GetBinCenter(latePeakBin);
    const double latePeakCounts = h->GetBinContent(latePeakBin);
    fitHigh = latePeakPosition;
    if (fitHigh <= gammaHigh) {
      fitHigh = std::min(gammaHigh + 40.0, xMax);
    }

    TF1 totalFit(Form("fit_%s", histName.Data()), "gaus(0)+gaus(3)", fitLow, fitHigh);
    totalFit.SetParNames("gf_amp", "gf_mean", "gf_sigma",
                         "late_amp", "late_mean", "late_sigma");
    totalFit.SetParameters(seedAmp, seedMean, seedSigma,
                           latePeakCounts, latePeakPosition, 35.0);
    totalFit.SetParLimits(0, 0.0, std::max(10.0 * seedAmp, 1.0));
    totalFit.SetParLimits(1, gammaLow, gammaHigh);
    totalFit.SetParLimits(2, 0.3, 30.0);
    totalFit.SetParLimits(3, 0.0, std::max(5.0 * latePeakCounts, 1.0));
    totalFit.FixParameter(4, latePeakPosition);
    totalFit.SetParLimits(5, 3.0, 250.0);

    TFitResultPtr fitResult = h->Fit(&totalFit, "RQSN0", "", fitLow, fitHigh);
    fitStatus = static_cast<int>(fitResult);

    gammaAmplitude = totalFit.GetParameter(0);
    gammaMean = totalFit.GetParameter(1);
    gammaMeanErr = totalFit.GetParError(1);
    gammaSigma = std::fabs(totalFit.GetParameter(2));
    gammaSigmaErr = totalFit.GetParError(2);
    gammaFwhm = 2.35482004503 * gammaSigma;
    gammaFwhmErr = 2.35482004503 * gammaSigmaErr;
    lateAmplitude = totalFit.GetParameter(3);
    lateSigma = std::fabs(totalFit.GetParameter(5));

    if (std::isfinite(gammaMean) && std::isfinite(gammaFwhm) && gammaFwhm > 0.0) {
      hMean->SetBinContent(detector + 1, gammaMean);
      hMean->SetBinError(detector + 1, gammaMeanErr);
      hFwhm->SetBinContent(detector + 1, gammaFwhm);
      hFwhm->SetBinError(detector + 1, gammaFwhmErr);
      hStatus->SetBinContent(detector + 1, fitStatus);
      gMean->SetPoint(meanPoint++, detector, gammaMean);
      gFwhm->SetPoint(fwhmPoint++, detector, gammaFwhm);
    }

    textOut << clover << " "
            << crystal << " "
            << detector << " "
            << gammaMean << " "
            << gammaFwhm << " "
            << gammaSigma << " "
            << fitStatus << " "
            << entries << "\n";

    TH1 *hDraw = dynamic_cast<TH1 *>(h->Clone(Form("%s_fit_input", histName.Data())));
    hDraw->SetDirectory(nullptr);
    hDraw->SetTitle(Form("%s;Time (ns);Counts", histName.Data()));
    hDraw->GetXaxis()->SetRangeUser(fitLow, std::min(xMax, latePeakPosition + plotPaddingAfterLatePeak));
    hDraw->SetLineColor(kBlack);
    hDraw->SetMarkerStyle(1);

    TF1 *drawTotal = dynamic_cast<TF1 *>(totalFit.Clone(Form("%s_total_fit", histName.Data())));
    drawTotal->SetRange(fitLow, fitHigh);
    drawTotal->SetLineColor(kRed + 1);
    drawTotal->SetLineWidth(2);

    TF1 *drawGamma = new TF1(Form("%s_gamma_flash_gaus", histName.Data()),
                             "gaus", fitLow, fitHigh);
    drawGamma->SetParameters(totalFit.GetParameter(0),
                             totalFit.GetParameter(1),
                             totalFit.GetParameter(2));
    drawGamma->SetLineColor(kAzure + 1);
    drawGamma->SetLineWidth(2);

    TF1 *drawLate = new TF1(Form("%s_late_peak_leading_gaus", histName.Data()),
                            "gaus", fitLow, fitHigh);
    drawLate->SetParameters(totalFit.GetParameter(3),
                            totalFit.GetParameter(4),
                            totalFit.GetParameter(5));
    drawLate->SetLineColor(kGreen + 2);
    drawLate->SetLineStyle(2);
    drawLate->SetLineWidth(2);

    TCanvas *canvas = new TCanvas(Form("c_%s_gamma_flash_fit", histName.Data()),
                                  Form("%s gamma flash fit", histName.Data()),
                                  1000, 700);
    hDraw->Draw("hist");
    drawTotal->Draw("same");
    drawGamma->Draw("same");
    drawLate->Draw("same");

    TLegend *legend = new TLegend(0.58, 0.66, 0.88, 0.88);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->AddEntry(hDraw, "Time spectrum", "l");
    legend->AddEntry(drawGamma, "Gamma flash Gaussian", "l");
    legend->AddEntry(drawLate, "Late peak leading Gaussian", "l");
    legend->AddEntry(drawTotal, "Total fit", "l");
    legend->Draw();

    TPaveText *text = new TPaveText(0.14, 0.68, 0.48, 0.88, "NDC");
    text->SetFillStyle(0);
    text->SetBorderSize(0);
    text->SetTextAlign(12);
    text->AddText(Form("detector = %d (clover %d crystal %d)", detector, clover, crystal));
    text->AddText(Form("#mu_{#gamma} = %.3f #pm %.3f ns", gammaMean, gammaMeanErr));
    text->AddText(Form("FWHM_{#gamma} = %.3f #pm %.3f ns", gammaFwhm, gammaFwhmErr));
    text->AddText(Form("fit range = %.1f - %.1f ns", fitLow, fitHigh));
    text->AddText(Form("status = %d", fitStatus));
    text->Draw();

    canvasDir->cd();
    canvas->Write();
    functionDir->cd();
    drawTotal->Write();
    drawGamma->Write();
    drawLate->Write();

    tree->Fill();
    std::cout << histName
              << " detector=" << detector
              << " gammaMean=" << gammaMean
              << " FWHM=" << gammaFwhm
              << " latePeak=" << latePeakPosition
              << " status=" << fitStatus
              << std::endl;
  }

  fout.cd();
  hMean->Write();
  hFwhm->Write();
  hStatus->Write();
  gMean->Write();
  gFwhm->Write();
  tree->Write();
  fout.Close();
  textOut.close();
  fin->Close();

  std::cout << "Wrote / 已写入: " << outPath << std::endl;
  std::cout << "Wrote text / 已写入文本: " << textPath << std::endl;
}
