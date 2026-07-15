// Calibrated event-TOF correlations with clover addback observables.
// 逐 run、逐 crystal 刻度后的 event TOF 与 clover addback 观测量关联图。
//
// Output histograms / 输出直方图
// -----------------------------------------------------------------------------
// 1. TOFVsAddbackGammaMultiplicity
//    X: number of accepted clover addback gamma fires in one event.
//    Y: calibrated event TOF in ns.
// 2. TOFVsAddbackGammaTotalEnergy
//    X: sum of all accepted clover addback gamma energies in one event, in keV.
//    Y: calibrated event TOF in ns.
//
// 1. TOFVsAddbackGammaMultiplicity
//    X：一个 event 中通过选择的 clover addback gamma 数量。
//    Y：刻度后的 event TOF，单位 ns。
// 2. TOFVsAddbackGammaTotalEnergy
//    X：一个 event 中所有通过选择的 clover addback gamma 能量之和，单位 keV。
//    Y：刻度后的 event TOF，单位 ns。
//
// Event TOF definition / Event TOF 定义
// -----------------------------------------------------------------------------
// Each clover time is the calibrated time of its highest-energy crystal. Event
// TOF is the minimum time among the accepted addback clovers. Therefore each
// accepted event contributes exactly one point to each two-dimensional plot.
// 每个 clover 的时间取其“刻度后能量最高的 crystal”的刻度时间；event TOF 再取
// 所有通过选择的 addback clover 时间中的最小值。因此，每个通过的 event 在两张
// 二维图中分别只填一个点。
//
// Selection / 选择条件
// -----------------------------------------------------------------------------
// By default this macro applies the same BGO/CSI veto and 50-MeV fast-neutron
// time cut used by fission_event_ana_calibrated.C. Both choices are explicit
// arguments. Set useBgoCsiVeto=false or applyFastTimeCut=false to disable them.
// 默认采用 fission_event_ana_calibrated.C 相同的 BGO/CSI veto 和 50 MeV 快中子
// 时间 cut；两项均可通过参数关闭。
//
// Input supports one ROOT file, comma-separated ROOT files, or @file-list.
// 输入支持单个 ROOT、逗号分隔的多个 ROOT，或 @文件列表。
//
// Example / 示例
// -----------------------------------------------------------------------------
// root -l -b -q 'lsy_nfs/tof_vs_addback_observables_calibrated.C(
//   "@mult3_files.txt","calibration_summary.tsv","tof_addback.root",
//   true,true,24.0,20.0,-1,1600,0,1600,4096,0,32768,"offset","fTime")'

// Reuse the established per-run calibration parser, branch resolver, physical
// constants, and clover data structure. This keeps the calibration convention
// identical to the existing calibrated fission analysis.
// 复用现有刻度版裂变分析的逐 run 刻度解析、分支解析、物理常数和 clover 结构，
// 保证两份代码的能量、时间和 addback 定义完全一致。
#include "fission_event_ana_calibrated.C"

void tof_vs_addback_observables_calibrated(
    const char *inputFiles = "out/mult3_nfs_run_100_r0.root",
    const char *calibrationSummary = "calibration_summary.tsv",
    const char *outputFile = "tof_vs_addback_observables_calibrated.root",
    bool useBgoCsiVeto = true,
    bool applyFastTimeCut = true,
    double nfsDistanceMeter = 24.0,
    double timeFwhmNs = 20.0,
    Long64_t maxEntries = -1,
    int tofBins = 1600,
    double tofMinNs = 0.0,
    double tofMaxNs = 1600.0,
    int totalEnergyBins = 4096,
    double totalEnergyMinKeV = 0.0,
    double totalEnergyMaxKeV = 32768.0,
    const char *timeCorrectionMode = "offset",
    const char *timeBranchLeaf = "fTime")
{
  if (!inputFiles || TString(inputFiles).Strip(TString::kBoth).IsNull() ||
      !calibrationSummary ||
      TString(calibrationSummary).Strip(TString::kBoth).IsNull() ||
      !outputFile || TString(outputFile).Strip(TString::kBoth).IsNull()) {
    std::cerr << "Input ROOT, calibration summary, and output ROOT are required. / "
                 "必须提供输入 ROOT、刻度汇总和输出 ROOT。"
              << std::endl;
    return;
  }
  if (tofBins <= 0 || tofMaxNs <= tofMinNs || totalEnergyBins <= 0 ||
      totalEnergyMaxKeV <= totalEnergyMinKeV ||
      nfsDistanceMeter <= 0.0 || timeFwhmNs <= 0.0) {
    std::cerr << "Invalid histogram or time-cut parameters. / "
                 "直方图或时间 cut 参数无效。"
              << std::endl;
    return;
  }

  const std::vector<TString> inputs = SplitCsv(inputFiles);
  if (inputs.empty()) {
    std::cerr << "No input ROOT files. / 没有输入 ROOT 文件。" << std::endl;
    return;
  }

  const CalibrationDB calibration = ReadCalibrationDB(calibrationSummary);
  if (calibration.byRun.empty() && !calibration.hasDefault) {
    std::cerr << "No usable calibration rows in: " << calibrationSummary
              << std::endl;
    return;
  }

  const double tMin50MeVNs =
      EnergyMeVToToFNs(50.0, nfsDistanceMeter);
  const double timeSigmaNs = timeFwhmNs / 2.355;
  const double tCutNs = tMin50MeVNs - 3.0 * timeSigmaNs;
  const TString timeMode(timeCorrectionMode ? timeCorrectionMode : "offset");

  TH2F tofVsMultiplicity(
      "TOFVsAddbackGammaMultiplicity",
      "Event TOF vs clover addback gamma multiplicity;Clover addback gamma multiplicity;Calibrated event TOF (ns)",
      kNClover, 0.5, static_cast<double>(kNClover) + 0.5,
      tofBins, tofMinNs, tofMaxNs);
  TH2F tofVsTotalEnergy(
      "TOFVsAddbackGammaTotalEnergy",
      "Event TOF vs total clover addback gamma energy;Total calibrated clover addback gamma energy (keV);Calibrated event TOF (ns)",
      totalEnergyBins, totalEnergyMinKeV, totalEnergyMaxKeV,
      tofBins, tofMinNs, tofMaxNs);
  tofVsMultiplicity.SetDirectory(nullptr);
  tofVsTotalEnergy.SetDirectory(nullptr);

  Long64_t totalEntries = 0;
  Long64_t processedFiles = 0;
  Long64_t filesWithoutCalibration = 0;
  Long64_t filesMissingBranches = 0;
  Long64_t malformedEntries = 0;
  Long64_t missingCrystalCalibration = 0;
  Long64_t eventsWithoutAcceptedAddback = 0;
  Long64_t acceptedEvents = 0;
  Long64_t acceptedCloverFires = 0;
  Long64_t vetoedCloverFires = 0;
  Long64_t fastTimeCutCloverFires = 0;
  Long64_t tofOutsideVisibleRange = 0;
  Long64_t totalEnergyOutsideVisibleRange = 0;

  for (const auto &inputName : inputs) {
    if (maxEntries > 0 && totalEntries >= maxEntries) break;

    TFile input(inputName, "READ");
    if (!input.IsOpen() || input.IsZombie()) {
      std::cerr << "Cannot open input ROOT / 无法打开输入 ROOT: "
                << inputName << std::endl;
      continue;
    }
    TTree *tree = dynamic_cast<TTree *>(input.Get("TreeMaster"));
    if (!tree) {
      std::cerr << "TreeMaster is missing / 缺少 TreeMaster: "
                << inputName << std::endl;
      continue;
    }

    // A missing run calibration is not silently replaced by identity values.
    // 缺少逐 run 刻度时直接跳过该文件，避免把未刻度数据混入最终二维图。
    const RunCalib *runCalibration = FindRunCalib(calibration, inputName);
    if (!runCalibration) {
      std::cerr << "Skip file without matching calibration / 跳过无对应刻度文件: "
                << inputName << std::endl;
      ++filesWithoutCalibration;
      continue;
    }

    const TString cloverBranch = ResolveBranch(tree, "fEXO_ECC_E_Clover");
    const TString crystalBranch = ResolveBranch(tree, "fEXO_ECC_E_Cristal");
    const TString energyBranch = ResolveBranch(tree, "fEXO_ECC_E_Energy");
    const TString timeBranch = ResolveBranch(
        tree, timeBranchLeaf && TString(timeBranchLeaf).Length() > 0
                  ? timeBranchLeaf
                  : "fTime");
    const TString essCloverBranch = ResolveBranch(tree, "fEXO_ESS_Clover");
    const TString essCrystalBranch = ResolveBranch(tree, "fEXO_ESS_Cristal");
    const TString bgoBranch = ResolveBranch(tree, "fEXO_ESS_BGO");
    const TString csiBranch = ResolveBranch(tree, "fEXO_ESS_CSI");

    if (cloverBranch.IsNull() || crystalBranch.IsNull() ||
        energyBranch.IsNull() || timeBranch.IsNull() ||
        (useBgoCsiVeto &&
         (essCloverBranch.IsNull() || essCrystalBranch.IsNull() ||
          bgoBranch.IsNull() || csiBranch.IsNull()))) {
      std::cerr << "Required branches are missing / 缺少必要分支: "
                << inputName << std::endl;
      ++filesMissingBranches;
      continue;
    }

    ++processedFiles;
    TTreeReader reader(tree);
    TTreeReaderArray<UShort_t> clovers(reader, cloverBranch.Data());
    TTreeReaderArray<UShort_t> crystals(reader, crystalBranch.Data());
    TTreeReaderArray<float> rawEnergies(reader, energyBranch.Data());
    TTreeReaderArray<float> rawTimes(reader, timeBranch.Data());
    std::unique_ptr<TTreeReaderArray<UShort_t>> essClovers;
    std::unique_ptr<TTreeReaderArray<UShort_t>> essCrystals;
    std::unique_ptr<TTreeReaderArray<UShort_t>> bgo;
    std::unique_ptr<TTreeReaderArray<UShort_t>> csi;
    if (useBgoCsiVeto) {
      essClovers.reset(
          new TTreeReaderArray<UShort_t>(reader, essCloverBranch.Data()));
      essCrystals.reset(
          new TTreeReaderArray<UShort_t>(reader, essCrystalBranch.Data()));
      bgo.reset(new TTreeReaderArray<UShort_t>(reader, bgoBranch.Data()));
      csi.reset(new TTreeReaderArray<UShort_t>(reader, csiBranch.Data()));
    }

    while (reader.Next()) {
      if (maxEntries > 0 && totalEntries >= maxEntries) break;
      ++totalEntries;

      const std::size_t fireCount =
          static_cast<std::size_t>(rawEnergies.GetSize());
      if (clovers.GetSize() != rawEnergies.GetSize() ||
          crystals.GetSize() != rawEnergies.GetSize() ||
          rawTimes.GetSize() != rawEnergies.GetSize()) {
        ++malformedEntries;
        continue;
      }

      std::array<CloverHit, kNClover> addback;
      if (useBgoCsiVeto) {
        const std::size_t vetoCount = std::min(
            {static_cast<std::size_t>(essClovers->GetSize()),
             static_cast<std::size_t>(essCrystals->GetSize()),
             static_cast<std::size_t>(bgo->GetSize()),
             static_cast<std::size_t>(csi->GetSize())});
        for (std::size_t index = 0; index < vetoCount; ++index) {
          const int clover = (*essClovers)[index];
          if (clover < 0 || clover >= kNClover) continue;
          if ((*bgo)[index] > 0) addback[clover].bgo = true;
          if ((*csi)[index] > 0) addback[clover].csi = true;
        }
      }

      // First calibrate every crystal, then add all crystal energies belonging
      // to the same clover. The clover time follows the highest calibrated
      // crystal energy, exactly as in the standard NFS addback reconstruction.
      // 先逐 crystal 应用能量与时间刻度，再按 clover 累加能量；clover 时间取
      // “刻度后能量最高的 crystal”的时间，与标准 NFS addback 重建一致。
      for (std::size_t index = 0; index < fireCount; ++index) {
        const int clover = clovers[index];
        const int crystal = crystals[index];
        if (clover < 0 || clover >= kNClover || crystal < 0 ||
            crystal >= kNCrystalPerClover) {
          continue;
        }
        const double rawEnergy = rawEnergies[index];
        const double rawTime = rawTimes[index];
        if (rawEnergy <= 0.0 || rawTime <= 0.0 ||
            !std::isfinite(rawEnergy) || !std::isfinite(rawTime)) {
          continue;
        }

        const int detector = clover * kNCrystalPerClover + crystal;
        const CrystalCalib &crystalCalibration =
            runCalibration->crystal[detector];
        if (!crystalCalibration.hasEnergy || !crystalCalibration.hasTime) {
          ++missingCrystalCalibration;
          continue;
        }
        const double energy = crystalCalibration.energyOffset +
                              crystalCalibration.energyGain * rawEnergy;
        const double time = ApplyTimeCorrection(
            rawTime, crystalCalibration, timeMode);
        if (energy <= 0.0 || time <= 0.0 || !std::isfinite(energy) ||
            !std::isfinite(time)) {
          continue;
        }

        CloverHit &hit = addback[clover];
        hit.energy += energy;
        ++hit.multiplicity;
        if (energy > hit.bestCrystalEnergy) {
          hit.bestCrystalEnergy = energy;
          hit.time = time;
        }
      }

      int addbackMultiplicity = 0;
      double addbackTotalEnergyKeV = 0.0;
      double eventToFNs = std::numeric_limits<double>::infinity();
      for (const auto &hit : addback) {
        if (hit.multiplicity <= 0 || hit.energy <= 0.0 ||
            !std::isfinite(hit.time)) {
          continue;
        }
        if (useBgoCsiVeto && (hit.bgo || hit.csi)) {
          ++vetoedCloverFires;
          continue;
        }
        if (applyFastTimeCut && hit.time < tCutNs) {
          ++fastTimeCutCloverFires;
          continue;
        }

        ++addbackMultiplicity;
        addbackTotalEnergyKeV += hit.energy;
        eventToFNs = std::min(eventToFNs, hit.time);
      }

      if (addbackMultiplicity <= 0 || !std::isfinite(eventToFNs)) {
        ++eventsWithoutAcceptedAddback;
        continue;
      }

      ++acceptedEvents;
      acceptedCloverFires += addbackMultiplicity;
      if (eventToFNs < tofMinNs || eventToFNs >= tofMaxNs) {
        ++tofOutsideVisibleRange;
      }
      if (addbackTotalEnergyKeV < totalEnergyMinKeV ||
          addbackTotalEnergyKeV >= totalEnergyMaxKeV) {
        ++totalEnergyOutsideVisibleRange;
      }

      // ROOT TH2::Fill(x,y): multiplicity/total energy is horizontal (X),
      // calibrated event TOF is vertical (Y), as requested.
      // ROOT TH2::Fill(x,y)：多重度/总能量填 X 轴，刻度后的 event TOF 填 Y 轴。
      tofVsMultiplicity.Fill(addbackMultiplicity, eventToFNs);
      tofVsTotalEnergy.Fill(addbackTotalEnergyKeV, eventToFNs);
    }
    input.Close();
  }

  TFile output(outputFile, "RECREATE");
  if (!output.IsOpen() || output.IsZombie()) {
    std::cerr << "Cannot create output ROOT / 无法创建输出 ROOT: "
              << outputFile << std::endl;
    return;
  }
  TDirectory *directory = output.mkdir("tof_addback_correlations");
  directory->cd();

  const TString configTitle = TString::Format(
      "inputs=%s; calibration=%s; event_tof=min_selected_addback_clover_time; use_bgo_csi_veto=%d; apply_fast_time_cut=%d; distance_m=%.9g; time_fwhm_ns=%.9g; t_min_50MeV_ns=%.9g; t_cut_ns=%.9g; tof_bins=%d; tof_range_ns=%.9g:%.9g; total_energy_bins=%d; total_energy_range_keV=%.9g:%.9g; time_mode=%s; time_branch=%s",
      inputFiles, calibrationSummary, useBgoCsiVeto ? 1 : 0,
      applyFastTimeCut ? 1 : 0, nfsDistanceMeter, timeFwhmNs,
      tMin50MeVNs, tCutNs, tofBins, tofMinNs, tofMaxNs, totalEnergyBins,
      totalEnergyMinKeV, totalEnergyMaxKeV, timeCorrectionMode,
      timeBranchLeaf);
  TNamed config("TOFVsAddbackConfig", configTitle.Data());
  config.Write();

  TH1D summary("TOFVsAddbackSummary",
               "TOFVsAddbackSummary;Category;Counts", 13, 0.5, 13.5);
  const char *labels[13] = {
      "Input entries", "Processed files", "Files no calibration",
      "Files missing branches", "Malformed entries", "Crystal calib misses",
      "Events no accepted addback", "Accepted events",
      "Accepted addback clovers", "Vetoed clover fires",
      "Fast-time-cut clover fires", "TOF outside display",
      "Total energy outside display"};
  const double values[13] = {
      static_cast<double>(totalEntries),
      static_cast<double>(processedFiles),
      static_cast<double>(filesWithoutCalibration),
      static_cast<double>(filesMissingBranches),
      static_cast<double>(malformedEntries),
      static_cast<double>(missingCrystalCalibration),
      static_cast<double>(eventsWithoutAcceptedAddback),
      static_cast<double>(acceptedEvents),
      static_cast<double>(acceptedCloverFires),
      static_cast<double>(vetoedCloverFires),
      static_cast<double>(fastTimeCutCloverFires),
      static_cast<double>(tofOutsideVisibleRange),
      static_cast<double>(totalEnergyOutsideVisibleRange)};
  for (int bin = 1; bin <= 13; ++bin) {
    summary.GetXaxis()->SetBinLabel(bin, labels[bin - 1]);
    summary.SetBinContent(bin, values[bin - 1]);
  }

  summary.Write();
  tofVsMultiplicity.Write();
  tofVsTotalEnergy.Write();
  output.Close();

  std::cout << "Processed files / 处理文件: " << processedFiles << std::endl;
  std::cout << "Input entries / 输入 event: " << totalEntries << std::endl;
  std::cout << "Accepted events / 通过 event: " << acceptedEvents << std::endl;
  std::cout << "Accepted addback clovers / 通过 addback clover: "
            << acceptedCloverFires << std::endl;
  std::cout << "t_min(50 MeV): " << tMin50MeVNs << " ns" << std::endl;
  std::cout << "t_cut: " << tCutNs
            << " ns (applied=" << (applyFastTimeCut ? "true" : "false")
            << ")" << std::endl;
  std::cout << "Output / 输出: " << outputFile << std::endl;
}
