# Personal NFS Modification Record / NFS 个人修改记录

Last updated: 2026-06-28

This document is a code map for the NFS-specific modifications added on top of the original ADNE analysis framework. It focuses on where the code lives, which cuts and thresholds are used, and what each added feature writes out.

本文档是基于原 ADNE 分析框架所做 NFS 专用修改的个人索引，重点记录代码位置、阈值/判断、输出对象和功能目的。

## 1. Main ADNE Entry Points / ADNE 主入口

### 1.1 NFS switches in YAML

Location:

```text
nfs-th232-analysis-adne/Yaml_config_files/config.yaml
```

Added interface:

```yaml
nfs_exo_ana:
  tree: true
  spec: true
  raw_tree: false
  crystal_time_correction: true
  correction_path: "/absolute/path/to/nfs_histoExogam2_1_deltaT_gamma_flash_fit.txt"
```

Meaning:

- `tree`: write standalone NFS trees with prefix `nfs_` and `mult3_nfs_`.
- `spec`: write NFS diagnostic histograms into `out/nfs_histoExogam2_1.root`.
- `raw_tree`: optional raw event-built MFM copy tree for debugging.
- `crystal_time_correction`: enable per-crystal gamma-flash time offset correction.
- `correction_path`: text table from `lsy_nfs/fit_nfs_deltaT_gamma_flash.C`.

Related NFS parameters:

```yaml
nfs_neutron: true
nfs_distance: 8.6
nfs_gammaFlashOffset: -700.4
nfs_gammaG: 1454.21
activate_ess: true
ess_threshold: 40
SetPromptGate: [0,90000]
```

Notes:

- `nfs_distance` is used in neutron ToF/energy conversion.
- `nfs_gammaFlashOffset` is used only when per-crystal correction is disabled or missing.
- `ess_threshold` and `SetPromptGate` are part of the original ADNE logic; the new NFS E877-style addback does not use the original prompt/AC cut.

### 1.2 YAML reading and output creation

Main locations:

```text
GUser.C:79   reads nfs_exo_ana
GUser.C:87   prints active switches
GUser.C:1315 creates nfs_*.root TreeMaster
GUser.C:1324 creates mult3_nfs_*.root TreeMaster
GUser.C:1393 BuildNfsTreeFileName()
GUser.C:1402 BuildNfsMult3TreeFileName()
GUser.C:1206 writes out/nfs_histoExogam2_1.root
```

Added output files:

```text
out/nfs_<original_output>.root
out/mult3_nfs_<original_output>.root
out/nfs_histoExogam2_1.root
```

Tree fill judgment:

```text
GUser.C:1073  if(NfsTreeFillBool) NfsTree->Fill()
GUser.C:1074  if(NfsMult3TreeFillBool && GetE877CloverVetoMult() >= 3) NfsMult3Tree->Fill()
```

The `mult3_nfs_*.root` tree is therefore an early fission-candidate tree, selected by BGO/CSI-vetoed E877 clover multiplicity `>= 3`.

## 2. Raw Event-Built MFM Tree / RawTree 接口

Location:

```text
GUser.C:1411 InitRawTreeBranches()
GUser.C:1418-1510 raw branch definitions
```

Purpose:

Copy one already event-built top-level MFM event into `RawTree` without adding physics cuts. This is intended for debugging the event-built MFM structure, not for final physics spectra.

Switch:

```yaml
nfs_exo_ana.raw_tree: true/false
```

Current normal setting:

```text
false
```

## 3. Crystal-Level NFS Time and Energy / Crystal 级时间和能量

Main location:

```text
TExogam2.cxx:1020-1130  TExogam2::IsMFMExo()
```

Basic EXOGAM crystal acceptance:

```cpp
IsCloverActive(clo) && result && frame->ExoGetInnerM(0) > 10
```

Location:

```text
TExogam2.cxx around IsMFMExo(), before energy/time filling
```

Meaning:

- clover is active in YAML;
- board/crystal mapping is valid;
- raw core energy channel is above 10.

Energy calibration:

```cpp
valf = Cal(frame->ExoGetInnerM(0), ECoef[MapFinger][0], ECoef[MapFinger][1], ECoef[MapFinger][2]);
```

Time conversion:

```text
TExogam2.cxx:1103 rawDeltaT = frame->ExoGetDeltaT()
TExogam2.cxx:1105 fDeltaT = (65536 - rawDeltaT) * 0.024 ns
```

Current hard-coded time gain:

```text
0.024 ns/channel
```

Valid NFS time cut:

```text
TExogam2.cxx:1123 rawDeltaT <= 60000 && neutronTOF > 0
```

Only if this is true, the code fills:

```text
fDeltaT
fNeutronTOF
fTime
NFS crystal Time/Energy spectra
```

Neutron energy formula:

```cpp
E_n = 0.5 * 939.5654 * pow(distanceTOF / (neutronTOF * 0.299), 2)
```

This is the non-relativistic `E = 1/2 m v^2` relation.

## 4. Per-Crystal Gamma-Flash Time Correction / 逐 crystal 时间修正

Loader location:

```text
TExogam2.cxx:220  SetNfsCrystalTimeCorrection()
```

Input text columns:

```text
clover crystal detector gammaMean_ns gammaFwhm_ns gammaSigma_ns fitStatus entries
```

Correction calculation:

```cpp
gammaFlightNs = (nfs_distance + cloverDistance) / c
offset[detector] = fittedGammaFlashPeakNs - gammaFlightNs
Time = fDeltaT - offset[detector]
```

Fallback if disabled or invalid:

```cpp
Time = fDeltaT + nfs_gammaFlashOffset
```

Important limitation:

Only the per-crystal offset is corrected. The gain is still the unified hard-coded `0.024 ns/channel`.

## 5. Added Tree Branches / 新增 Tree 分支

Main locations:

```text
TExogam2Data.h:73-82 branch data members
TExogam2Data.h:158-165 setters
TExogam2Data.h:187-202 getters and veto multiplicity helper
TExogam2Data.cxx:74-82 and 155-163 clear/reset vectors
```

Added branches:

```text
fDeltaT
fNeutronTOF
fTime
f_E877_Clover
f_E877_Clover_E
f_E877_Clover_T
f_E877_Clover_BGO
f_E877_Clover_CSI
```

Meaning:

- `fDeltaT`: reversed raw DeltaT in ns.
- `fNeutronTOF`: corrected NFS analysis Time in ns.
- `fTime`: current analysis Time used by later NFS processing.
- `f_E877_Clover`: clover number after raw crystal fires are grouped.
- `f_E877_Clover_E`: clover addback energy in keV.
- `f_E877_Clover_T`: clover Time, chosen from the highest-energy crystal in that clover.
- `f_E877_Clover_BGO`: summed BGO value associated with that clover.
- `f_E877_Clover_CSI`: summed CSI value associated with that clover.

Veto multiplicity helper:

```text
TExogam2Data.h:199-202 GetE877CloverVetoMult()
condition: f_E877_Clover_BGO <= 0 && f_E877_Clover_CSI <= 0
```

## 6. E877-Style Clover Addback / E877 格式 clover 合并

Main locations:

```text
TExogam2.cxx:1658-1689 positive-energy raw crystal collection
TExogam2.cxx:1760-1765 branch filling and addback spectra filling
```

Judgment:

```cpp
fExogam2Data->GetECCEEnergy(i) > 0
```

Meaning:

Only positive gamma energy deposits are treated as crystal fires. This excludes zero-energy or empty events from gamma spectra and addback sums.

Clover grouping rule:

```text
same clover crystal energies -> summed clover energy
clover time -> Time of the highest-energy crystal fire in that clover
BGO/CSI -> summed from corresponding ESS entries if present
```

Important distinction:

This E877-style addback is done before the original ADNE prompt/anti-Compton/addback logic. It is the preferred NFS event representation.

## 7. NFS Spectra Written by ADNE / ADNE 输出的 NFS 谱

Constructor and fill locations:

```text
TExogam2.cxx:278  NfsSpectraConstructor()
TExogam2.cxx:342  FillNfsSpectra()
TExogam2.cxx:352  FillNfsCrystalEventSpectra()
TExogam2.cxx:377  FillNfsCloverAddbackSpectra()
```

Output file:

```text
out/nfs_histoExogam2_1.root
```

Crystal Time/Energy spectra:

```text
nfs_all_crystal_time_vs_energy
nfs_all_crystal_time
nfs_cloverX_crystalY_time
nfs_cloverX_crystalY_energy
```

Fill cut:

```cpp
energy > 0 && Time > 0
```

Clover addback spectra:

```text
nfs_cloverX_addback_gamma
nfs_cloverX_addback_gamma_bgo_csi_veto
```

Fill cuts:

```text
addback spectrum: energy > 0
veto spectrum: energy > 0 && BGO <= 0 && CSI <= 0
```

Latest added detector diagnostics:

```text
TExogam2.cxx:294 nfs_crystal_cross_talk
TExogam2.cxx:299 nfs_crystal_bgo_efficiency
TExogam2.cxx:302 nfs_crystal_csi_efficiency
```

Definitions:

- `nfs_crystal_cross_talk`: event-level 64 x 64 matrix. If one event has two or more unique positive-energy crystal fires, fill all off-diagonal pairs symmetrically.
- `nfs_crystal_bgo_efficiency`: `TProfile`; denominator is a positive-energy crystal fire, numerator is BGO `> 0` for the same stored crystal entry.
- `nfs_crystal_csi_efficiency`: `TProfile`; denominator is a positive-energy crystal fire, numerator is CSI `> 0` for the same stored crystal entry.

Detector id:

```text
detector = clover * 4 + crystal
label = clover-crystal
```

Ranges:

```text
Time: 0-1600 ns
Crystal energy: 0-20000 keV
Clover addback energy: 0-4096 keV
Cross talk: 64 x 64 detector bins
Efficiency: 64 TProfile bins, value range 0-1
```

## 8. ROOT Helper Scripts / ROOT 辅助脚本

Location:

```text
nfs-th232-analysis-adne/lsy_nfs
```

### 8.1 extract_nfs_energy_peaks.C

Purpose:

For each crystal energy spectrum, find the maximum bin in a selected energy window.

Default window:

```text
1440-1470 keV
```

Input histograms:

```text
nfs_cloverX_crystalY_energy
```

Output:

```text
<input>_peak_1440_1470.root
```

### 8.2 fit_nfs_deltaT_gamma_flash.C

Purpose:

Fit gamma-flash peak positions for per-crystal time alignment.

Input histograms:

```text
nfs_cloverX_crystalY_time
nfs_cloverX_crystalY_deltaT  # legacy compatibility
```

Default fit settings:

```text
offsetLow = 700 ns
gammaLow = 790 ns
gammaHigh = 850 ns
lateSearchHigh = 1050 ns
plotPaddingAfterLatePeak = 80 ns
```

Fit model:

```text
gamma flash Gaussian + leading half of late high peak Gaussian
```

Outputs:

```text
<input>_deltaT_gamma_flash_fit.root
<input>_deltaT_gamma_flash_fit.txt
```

The `.txt` file is read by `TExogam2::SetNfsCrystalTimeCorrection()`.

### 8.3 fission_event_ana.C

Main locations:

```text
lsy_nfs/fission_event_ana.C:230-233 physical time cut setup
lsy_nfs/fission_event_ana.C:247-255 per-bin 1D/2D histogram creation
lsy_nfs/fission_event_ana.C:263-265 Mult3GammaEnergyVsTime creation
lsy_nfs/fission_event_ana.C:340-351 per-fire cuts
lsy_nfs/fission_event_ana.C:360-367 event multiplicity and bin selection
lsy_nfs/fission_event_ana.C:378-389 spectrum, E-vs-Time, and gamma-gamma matrix filling
```

Input branches:

```text
f_E877_Clover
f_E877_Clover_E
f_E877_Clover_T
f_E877_Clover_BGO
f_E877_Clover_CSI
```

Default arguments:

```text
nfsDistanceMeter = 8.6
timeFwhmNs = 20
energyBinEdgesMeV = "2,4,7,12,20,30,50"
useBgoCsiVeto = true
maxEntries = -1
```

Time cut:

```text
highest neutron energy = 50 MeV
T_min = ToF(50 MeV, nfsDistanceMeter)
sigma_time = timeFwhmNs / 2.355
T_cut = T_min - 3 * sigma_time
```

Per-fire cuts:

```text
energy > 0
time > 0
time >= T_cut
BGO <= 0 && CSI <= 0   # if useBgoCsiVeto is true
```

Per-event cuts:

```text
remaining clover multiplicity >= 3
Event_neutron_Time = minimum Time among remaining clover fires
Event must fall inside one neutron-energy/time bin
```

Output histograms:

```text
Mult3GammaSpec_E*_MeV
Mult3GammaGammaMatrix_E*_MeV
Mult3GammaEnergyVsTime
```

Gamma-gamma matrix rule:

For energies `[a,b,c]`, fill:

```text
(a,b), (b,a), (a,c), (c,a), (b,c), (c,b)
```

Histogram ranges:

```text
1D gamma spectra: 4096 bins, 0-4096 keV
2D gamma-gamma matrices: 4096 x 4096 bins, 0-4096 keV on both axes
Mult3GammaEnergyVsTime: x = 0-4096 keV, y = 0-1600 ns
```

## 9. Deprecated Two-Step Python Scripts / 已弃用两步 Python 脚本

Location:

```text
two-step-nfs-th232-python-scripts
```

Purpose when created:

- inspect first events from ADNE ROOT tree;
- plot `fEXO/sEXO/aEXO` energy spectra;
- plot neutron-gated spectra and gamma-gamma matrices;
- check detector response, clover/crystal matrices, BGO/CSI efficiencies.

Current status:

This workflow is kept as a historical quick-check tool only. It is not used as the final NFS analysis standard because it starts from ADNE's original reconstructed ROOT tree and therefore inherits ADNE default `fEXO/sEXO/aEXO` judgments.

## 10. Threshold and Cut Summary / 阈值和判断汇总

| Cut or threshold | Value | Code location | Meaning |
| --- | --- | --- | --- |
| Raw core energy acceptance | `ExoGetInnerM(0) > 10` | `TExogam2::IsMFMExo()` | accept EXOGAM crystal frame |
| Time gain | `0.024 ns/channel` | `TExogam2.cxx:1105` | convert reversed raw DeltaT to ns |
| Raw DeltaT validity | `rawDeltaT <= 60000` | `TExogam2.cxx:1123` | reject invalid/unphysical TDC entries |
| Corrected time validity | `neutronTOF > 0` | `TExogam2.cxx:1123` | only positive Time is filled into NFS time branches/spectra |
| Crystal gamma fire | `energy > 0` | `TExogam2.cxx:1662`; `fission_event_ana.C:340` | reject empty or zero-energy gamma deposits |
| Crystal spectra fill | `energy > 0 && Time > 0` | `TExogam2.cxx:342-347` | fill NFS crystal Time/Energy spectra |
| Clover veto spectrum | `BGO <= 0 && CSI <= 0` | `TExogam2.cxx:377-382` | fill vetoed clover addback spectrum |
| `mult3_nfs` tree | vetoed clover multiplicity `>= 3` | `GUser.C:1074` | early fission-candidate tree |
| Crystal BGO/CSI efficiency | BGO or CSI `> 0` | `TExogam2.cxx:352-370` | numerator for efficiency profile |
| Fission highest neutron energy | `50 MeV` | `fission_event_ana.C:230` | defines earliest allowed physical time |
| Fission time width | default `20 ns` | `fission_event_ana.C:213` | converted to sigma by `/2.355` |
| Fission time cut | `T_cut = T_min - 3*sigma` | `fission_event_ana.C:233` | remove too-early clover fires |
| Fission per-fire veto | `BGO <= 0 && CSI <= 0` | `fission_event_ana.C:341-348` | optional, default enabled |
| Fission final multiplicity | selected clovers `>= 3` | `fission_event_ana.C:360` | final event-level fission candidate |
| Gamma flash fit window | `790-850 ns` | `fit_nfs_deltaT_gamma_flash.C` | seed and fit target gamma flash |
| Late peak search | `850-1050 ns` | `fit_nfs_deltaT_gamma_flash.C` | model leading side of late high peak |
| Energy peak search | `1440-1470 keV` | `extract_nfs_energy_peaks.C` | quick per-crystal peak check |

## 11. Practical Output Map / 实际输出索引

ADNE run outputs:

```text
out/nfs_histoExogam2_1.root
out/nfs_*.root
out/mult3_nfs_*.root
```

Gamma flash fit outputs:

```text
out/nfs_histoExogam2_1_deltaT_gamma_flash_fit.root
out/nfs_histoExogam2_1_deltaT_gamma_flash_fit.txt
```

Fission-event macro output example:

```text
out/mult3_nfs_run_100_r0_fission_event_ana.root
```

Personal rule of thumb:

Use the ADNE-produced NFS tree and spectra as the standard path. Use the old Python two-step scripts only for historical comparison or quick debugging.
