// Parallel preparation stage for calibrated triple-gamma analysis.
// 三重 gamma 分析的并行预处理阶段：先做快速多重度判断，再刻度并写紧凑 Tree。
//
// This file reuses the calibration/branch helpers from triple_gamma_ana_calibrated.C.
// It does not allocate any TH3D histogram. Each parallel process writes one compact
// ROOT file, which is consumed later by triple_gamma_fill_serial.C.
// 本文件复用 triple_gamma_ana_calibrated.C 中的刻度和分支解析函数，不创建 TH3D。
// 每个并行进程只写一个紧凑 ROOT 文件，之后由 triple_gamma_fill_serial.C 串行填图。

// Direct usage / 直接调用示例:
// root -l -b -q 'triple_gamma_prepare_calibrated.C("@mult3_files.txt","calibration_summary.tsv","compact.root",true,-1,3,0,2048,24,"offset","fTime")'

#include "triple_gamma_ana_calibrated.C"

bool PrepareTripleGammaCalibratedTree(const char *inputFiles,
                                      const char *calibrationSummary,
                                      const char *outputFile,
                                      bool useBgoCsiVeto,
                                      Long64_t maxEntries,
                                      int minCloverMultiplicity,
                                      double energyMinKeV,
                                      double energyMaxKeV,
                                      double nfsDistanceMeter,
                                      const char *timeCorrectionMode,
                                      const char *timeBranchLeaf)
{
  if (minCloverMultiplicity < 3 || energyMaxKeV <= energyMinKeV || nfsDistanceMeter <= 0.0) {
    std::cerr << "Invalid compact-tree preparation parameters." << std::endl;
    return false;
  }

  const std::vector<TString> inputs = SplitCsv(inputFiles);
  if (inputs.empty()) {
    std::cerr << "No mult input files were provided." << std::endl;
    return false;
  }

  const CalibrationDB calibDB = ReadCalibrationDB(calibrationSummary);
  if (calibDB.byRun.empty() && !calibDB.hasDefault) {
    std::cerr << "No usable calibration rows found in: " << calibrationSummary << std::endl;
    return false;
  }

  TString outName = outputFile && TString(outputFile).Length() > 0
                      ? TString(outputFile)
                      : TString("triple_gamma_compact.root");
  TFile out(outName, "RECREATE");
  if (!out.IsOpen()) {
    std::cerr << "Cannot create compact output: " << outName << std::endl;
    return false;
  }
  out.SetCompressionLevel(4);

  Double_t eventNeutronEnergyMeV = -1.0;
  Double_t eventNeutronTimeNs = -1.0;
  std::vector<UShort_t> compactCloverId;
  std::vector<float> compactCloverEnergyKeV;

  TTree compact("TripleGammaCompact", "Calibrated multiplicity-selected triple-gamma events");
  compact.Branch("EventNeutronEnergyMeV", &eventNeutronEnergyMeV, "EventNeutronEnergyMeV/D");
  compact.Branch("EventNeutronTimeNs", &eventNeutronTimeNs, "EventNeutronTimeNs/D");
  compact.Branch("CloverID", &compactCloverId);
  compact.Branch("CloverEnergyKeV", &compactCloverEnergyKeV);
  compact.SetAutoFlush(-50LL * 1024LL * 1024LL);

  Long64_t inputEntries = 0;
  Long64_t processedFiles = 0;
  Long64_t preCalibrationMultiplicityRejects = 0;
  Long64_t postCalibrationMultiplicityRejects = 0;
  Long64_t malformedEntries = 0;
  Long64_t writtenEntries = 0;
  Long64_t missingCalibrationFiles = 0;
  Long64_t missingCrystalCalibration = 0;
  Long64_t vetoedCloverFires = 0;
  Long64_t outOfRangeCloverFires = 0;
  TString timeMode(timeCorrectionMode ? timeCorrectionMode : "offset");

  for (const auto &inputName : inputs) {
    TFile inputFile(inputName, "READ");
    if (!inputFile.IsOpen()) {
      std::cerr << "Cannot open input file: " << inputName << std::endl;
      continue;
    }
    TTree *tree = dynamic_cast<TTree *>(inputFile.Get("TreeMaster"));
    if (!tree) {
      std::cerr << "Cannot find TreeMaster in: " << inputName << std::endl;
      continue;
    }
    processedFiles++;

    const RunCalib *runCal = FindRunCalib(calibDB, inputName);
    if (!runCal) {
      std::cerr << "Warning: no calibration found for " << inputName
                << "; using identity calibration." << std::endl;
      missingCalibrationFiles++;
    }
    RunCalib identityCal;
    const RunCalib &calib = runCal ? *runCal : identityCal;

    const TString cloverBranchName = ResolveBranch(tree, "fEXO_ECC_E_Clover");
    const TString crystalBranchName = ResolveBranch(tree, "fEXO_ECC_E_Cristal");
    const TString energyBranchName = ResolveBranch(tree, "fEXO_ECC_E_Energy");
    const TString requestedTimeBranch = timeBranchLeaf && TString(timeBranchLeaf).Length() > 0
                                          ? TString(timeBranchLeaf)
                                          : TString("fTime");
    const TString timeBranchName = ResolveBranch(tree, requestedTimeBranch.Data());
    const TString essCloverBranchName = ResolveBranch(tree, "fEXO_ESS_Clover");
    const TString essCrystalBranchName = ResolveBranch(tree, "fEXO_ESS_Cristal");
    const TString bgoBranchName = ResolveBranch(tree, "fEXO_ESS_BGO");
    const TString csiBranchName = ResolveBranch(tree, "fEXO_ESS_CSI");

    if (cloverBranchName.IsNull() || crystalBranchName.IsNull() ||
        energyBranchName.IsNull() || timeBranchName.IsNull()) {
      std::cerr << "Missing required crystal branches in: " << inputName << std::endl;
      continue;
    }
    if (useBgoCsiVeto && (essCloverBranchName.IsNull() || essCrystalBranchName.IsNull() ||
                          bgoBranchName.IsNull() || csiBranchName.IsNull())) {
      std::cerr << "BGO/CSI veto requested but ESS branches are missing in: " << inputName << std::endl;
      continue;
    }

    TTreeReader reader(tree);
    TTreeReaderArray<UShort_t> clovers(reader, cloverBranchName.Data());
    TTreeReaderArray<UShort_t> crystals(reader, crystalBranchName.Data());
    TTreeReaderArray<float> energies(reader, energyBranchName.Data());
    TTreeReaderArray<float> times(reader, timeBranchName.Data());
    std::unique_ptr<TTreeReaderArray<UShort_t>> essClovers;
    std::unique_ptr<TTreeReaderArray<UShort_t>> essCrystals;
    std::unique_ptr<TTreeReaderArray<UShort_t>> bgo;
    std::unique_ptr<TTreeReaderArray<UShort_t>> csi;
    if (useBgoCsiVeto) {
      essClovers.reset(new TTreeReaderArray<UShort_t>(reader, essCloverBranchName.Data()));
      essCrystals.reset(new TTreeReaderArray<UShort_t>(reader, essCrystalBranchName.Data()));
      bgo.reset(new TTreeReaderArray<UShort_t>(reader, bgoBranchName.Data()));
      csi.reset(new TTreeReaderArray<UShort_t>(reader, csiBranchName.Data()));
    }

    while (reader.Next()) {
      if (maxEntries > 0 && inputEntries >= maxEntries) break;
      inputEntries++;

      const std::size_t n = static_cast<std::size_t>(energies.GetSize());
      if (clovers.GetSize() != energies.GetSize() || crystals.GetSize() != energies.GetSize() ||
          times.GetSize() != energies.GetSize()) {
        malformedEntries++;
        continue;
      }

      std::array<CloverHit, kNClover> hit;
      if (useBgoCsiVeto) {
        const std::size_t nv = std::min({static_cast<std::size_t>(essClovers->GetSize()),
                                         static_cast<std::size_t>(essCrystals->GetSize()),
                                         static_cast<std::size_t>(bgo->GetSize()),
                                         static_cast<std::size_t>(csi->GetSize())});
        for (std::size_t iv = 0; iv < nv; ++iv) {
          const int clo = (*essClovers)[iv];
          if (clo < 0 || clo >= kNClover) continue;
          if ((*bgo)[iv] > 0) hit[clo].bgo = true;
          if ((*csi)[iv] > 0) hit[clo].csi = true;
        }
      }

      // Fast preselection before calibration: count unique clovers with a valid raw
      // crystal hit. Vetoed clovers are excluded here as they cannot enter the cube.
      // 刻度前快速预筛：只统计具有有效原始 crystal fire 的不同 clover；被 veto 的
      // clover 最终不可能进入矩阵，因此此处直接排除。
      std::array<bool, kNClover> rawCloverPresent{};
      for (std::size_t i = 0; i < n; ++i) {
        const int clo = clovers[i];
        const int cri = crystals[i];
        if (clo < 0 || clo >= kNClover || cri < 0 || cri >= kNCrystalPerClover) continue;
        const double rawEnergy = energies[i];
        const double rawTime = times[i];
        if (rawEnergy <= 0.0 || rawTime <= 0.0 || !std::isfinite(rawEnergy) || !std::isfinite(rawTime)) continue;
        if (useBgoCsiVeto && (hit[clo].bgo || hit[clo].csi)) continue;
        rawCloverPresent[clo] = true;
      }
      const int rawMultiplicity = static_cast<int>(std::count(rawCloverPresent.begin(), rawCloverPresent.end(), true));
      if (rawMultiplicity < minCloverMultiplicity) {
        preCalibrationMultiplicityRejects++;
        continue;
      }

      // Only events surviving the cheap raw-multiplicity check reach calibration.
      // 只有通过快速原始多重度判断的事件才执行逐 crystal 刻度。
      for (std::size_t i = 0; i < n; ++i) {
        const int clo = clovers[i];
        const int cri = crystals[i];
        if (clo < 0 || clo >= kNClover || cri < 0 || cri >= kNCrystalPerClover) continue;
        const double rawEnergy = energies[i];
        const double rawTime = times[i];
        if (rawEnergy <= 0.0 || rawTime <= 0.0 || !std::isfinite(rawEnergy) || !std::isfinite(rawTime)) continue;
        if (useBgoCsiVeto && (hit[clo].bgo || hit[clo].csi)) continue;

        const int det = clo * kNCrystalPerClover + cri;
        const CrystalCalib &cc = calib.crystal[det];
        if (!cc.hasEnergy || !cc.hasTime) missingCrystalCalibration++;
        const double energy = cc.energyOffset + cc.energyGain * rawEnergy;
        const double time = ApplyTimeCorrection(rawTime, cc, timeMode);
        if (energy <= 0.0 || time <= 0.0 || !std::isfinite(energy) || !std::isfinite(time)) continue;

        CloverHit &h = hit[clo];
        h.energy += energy;
        h.multiplicity++;
        if (energy > h.bestCrystalEnergy) {
          h.bestCrystalEnergy = energy;
          h.time = time;
        }
      }

      compactCloverId.clear();
      compactCloverEnergyKeV.clear();
      eventNeutronTimeNs = std::numeric_limits<double>::infinity();
      for (int clo = 0; clo < kNClover; ++clo) {
        const CloverHit &h = hit[clo];
        if (h.multiplicity <= 0 || h.energy <= 0.0 || !std::isfinite(h.time)) continue;
        if (useBgoCsiVeto && (h.bgo || h.csi)) {
          vetoedCloverFires++;
          continue;
        }
        if (h.energy < energyMinKeV || h.energy >= energyMaxKeV) {
          outOfRangeCloverFires++;
          continue;
        }
        compactCloverId.push_back(static_cast<UShort_t>(clo));
        compactCloverEnergyKeV.push_back(static_cast<float>(h.energy));
        if (h.time < eventNeutronTimeNs) eventNeutronTimeNs = h.time;
      }

      // Recheck after calibration because invalid/out-of-range calibrated clovers may
      // reduce the final multiplicity.
      // 刻度后再次检查：无效或超出能量范围的 clover 可能降低最终多重度。
      if (compactCloverEnergyKeV.size() < static_cast<std::size_t>(minCloverMultiplicity)) {
        postCalibrationMultiplicityRejects++;
        continue;
      }

      eventNeutronEnergyMeV = ToFNsToEnergyMeV(eventNeutronTimeNs, nfsDistanceMeter);
      compact.Fill();
      writtenEntries++;
    }
    inputFile.Close();
    if (maxEntries > 0 && inputEntries >= maxEntries) break;
  }

  out.cd();
  compact.Write();
  out.Close();

  std::cout << "Compact preparation files: " << processedFiles << std::endl;
  std::cout << "Compact preparation input entries: " << inputEntries << std::endl;
  std::cout << "Pre-calibration multiplicity rejects: " << preCalibrationMultiplicityRejects << std::endl;
  std::cout << "Post-calibration multiplicity rejects: " << postCalibrationMultiplicityRejects << std::endl;
  std::cout << "Malformed entries: " << malformedEntries << std::endl;
  std::cout << "Compact entries written: " << writtenEntries << std::endl;
  std::cout << "Files without calibration: " << missingCalibrationFiles << std::endl;
  std::cout << "Crystal calibration misses: " << missingCrystalCalibration << std::endl;
  std::cout << "Vetoed clover fires: " << vetoedCloverFires << std::endl;
  std::cout << "Out-of-range clover fires: " << outOfRangeCloverFires << std::endl;
  std::cout << "Compact output: " << outName << std::endl;
  return processedFiles > 0;
}

void triple_gamma_prepare_calibrated(const char *inputFiles = "@mult3_files.txt",
                                     const char *calibrationSummary = "calibration_summary.tsv",
                                     const char *outputFile = "triple_gamma_compact.root",
                                     bool useBgoCsiVeto = true,
                                     Long64_t maxEntries = -1,
                                     int minCloverMultiplicity = 3,
                                     double energyMinKeV = 0.0,
                                     double energyMaxKeV = 4096.0,
                                     double nfsDistanceMeter = 24.0,
                                     const char *timeCorrectionMode = "offset",
                                     const char *timeBranchLeaf = "fTime")
{
  if (!PrepareTripleGammaCalibratedTree(inputFiles, calibrationSummary, outputFile,
                                        useBgoCsiVeto, maxEntries, minCloverMultiplicity,
                                        energyMinKeV, energyMaxKeV, nfsDistanceMeter,
                                        timeCorrectionMode, timeBranchLeaf)) {
    gSystem->Exit(2);
  }
}
