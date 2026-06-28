# NFS Data Processing Workflow Summary / NFS 数据处理流程总结

This note summarizes the current NFS Th-232 / EXOGAM2 data-processing scripts in this repository.  
本文档总结当前仓库中的 NFS Th-232 / EXOGAM2 数据处理脚本和处理逻辑。

The current standard workflow is the modified ADNE workflow under:

```text
/home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne
```

The old two-step Python post-processing workflow is kept only as a historical quick-check tool. It is not the recommended standard analysis path, because it starts from the original ADNE ROOT output and therefore inherits ADNE's default event reconstruction and `fEXO/sEXO/aEXO` decisions, which do not match the current NFS event-definition standard.

旧的两步 Python 后处理流程只作为历史快速检查工具保留；它不再作为推荐标准流程。原因是它以原始 ADNE 生成的 ROOT tree 为输入，会继承 ADNE 默认的事例重建和 `fEXO/sEXO/aEXO` 判断，而这些判断不符合当前 NFS 分析标准。

## 1. Current Standard Workflow / 当前标准流程

### 1.1 Main idea / 核心思路

The current workflow moves the important NFS logic into the ADNE processing step itself:

1. Read the event-built MFM file through ADNE.
2. Keep the original ADNE code structure and output untouched when the new switches are disabled.
3. Add a separate NFS tree output with prefix `nfs_`.
4. Add a separate NFS multiplicity tree output with prefix `mult3_nfs_`.
5. Add NFS-specific spectra into `out/nfs_histoExogam2_1.root`.
6. Use ROOT-only helper macros in `lsy_nfs/` for fitting gamma flash, checking energy peaks, and later fission-candidate analysis.

当前流程把关键 NFS 判断前移到 ADNE 处理阶段：

1. 通过 ADNE 读取已经 event-built 的 MFM 文件。
2. 新开关关闭时，尽量不改变 ADNE 原本输出。
3. 额外生成带 `nfs_` 前缀的 NFS tree。
4. 额外生成带 `mult3_nfs_` 前缀的多重度筛选 tree。
5. 额外生成 NFS 专用谱文件 `out/nfs_histoExogam2_1.root`。
6. 使用 `lsy_nfs/` 下只依赖 ROOT 的宏做 gamma flash 拟合、峰位检查和裂变候选事例分析。

### 1.2 Main source files / 主要源码位置

The NFS-specific modifications are mainly in:

```text
nfs-th232-analysis-adne/Yaml_config_files/config.yaml
nfs-th232-analysis-adne/GUser.C
nfs-th232-analysis-adne/GUser.h
nfs-th232-analysis-adne/TExogam2.cxx
nfs-th232-analysis-adne/TExogam2.h
nfs-th232-analysis-adne/TExogam2Data.h
nfs-th232-analysis-adne/TExogam2Data.cxx
nfs-th232-analysis-adne/lsy_nfs/
```

核心代码逻辑位置：

- `GUser.C`: reads `nfs_exo_ana` switches, creates `nfs_*.root` and `mult3_nfs_*.root`, writes NFS spectra.
- `TExogam2.cxx`: performs crystal-level time conversion, optional per-crystal time correction, NFS spectra filling, and E877-style clover addback.
- `TExogam2Data.h/.cxx`: defines the new tree branches, including `fDeltaT`, `fNeutronTOF`, `fTime`, and `f_E877_Clover_*`.
- `lsy_nfs/*.C`: ROOT helper scripts used after ADNE output is produced.

## 2. Configuration Switches / 配置开关

The current NFS analysis switches are in:

```text
nfs-th232-analysis-adne/Yaml_config_files/config.yaml
```

Current interface:

```yaml
nfs_exo_ana:
  tree: true
  spec: true
  raw_tree: false
  crystal_time_correction: true
  correction_path: "/home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne/out/nfs_histoExogam2_1_deltaT_gamma_flash_fit.txt"
```

Meaning:

- `tree`: create the NFS ROOT tree output, independent of the original ADNE tree.
- `spec`: create NFS quick-check spectra.
- `raw_tree`: copy the already event-built MFM event contents into a raw tree. This is mainly a debugging interface and is normally disabled.
- `crystal_time_correction`: use per-crystal gamma-flash correction loaded from `correction_path`.
- `correction_path`: text table produced by `lsy_nfs/fit_nfs_deltaT_gamma_flash.C`.

Related EXOGAM/NFS parameters are under `Guser.exogam2`:

```yaml
nfs_neutron: true
nfs_distance: 8.6          # meters
nfs_gammaFlashOffset: -700.4 # ns, used only when per-crystal correction is disabled or missing
cloverX_Distance: 145.0    # cm in config comment, used by code as mm-scale clover-distance parameter
activate_ess: true
ess_threshold: 40
SetPromptGate: [0,90000]
```

Note: `nfs_gammaFlashOffset` is the global correction used in the fallback path. When `crystal_time_correction` is enabled and a valid correction exists for a detector, the per-crystal correction replaces this global offset.

注意：`nfs_gammaFlashOffset` 是全局 fallback 修正。若 `crystal_time_correction` 打开且某个 detector 有有效拟合结果，则该 detector 使用单 crystal 修正，不再使用全局 offset。

## 3. Event Source / 事例来源

The input MFM files used here are already event-built before ADNE reads them. ADNE receives one top-level MFM event and processes detector frames inside it. The current NFS code does not rebuild events from scratch by applying a new coincidence window.

这里使用的 MFM 文件在进入 ADNE 之前已经完成 event build。ADNE 读取的是顶层 MFM event，并处理其中的 detector frames。当前 NFS 代码没有重新用新的时间窗从零构建 event。

For details, see:

```text
/home/user0/work/IJCLAB/NFS/nfs-th232-analysis/event build step.md
```

## 4. Crystal-Level Processing / Crystal 级处理

### 4.1 Basic crystal acceptance / 基本 crystal 接收条件

In `TExogam2::IsMFMExo`, an EXOGAM core hit is accepted into the normal crystal-level data when:

```cpp
IsCloverActive(clo) && result && frame->ExoGetInnerM(0) > 10
```

Meaning:

- the clover is activated in `config.yaml`;
- the board/crystal mapping is valid;
- the raw core energy channel `ExoGetInnerM(0)` is above 10.

含义：

- clover 在 `config.yaml` 中被打开；
- board/crystal 映射有效；
- core 原始能量 `ExoGetInnerM(0)` 大于 10。

After that, the calibrated core energy is:

```cpp
valf = Cal(frame->ExoGetInnerM(0), ECoef[MapFinger][0], ECoef[MapFinger][1], ECoef[MapFinger][2]);
```

The detector index follows the ADNE `MapFinger` convention:

```text
detector = clover * 4 + crystal
```

## 5. Time Conversion / 时间转换

### 5.1 Raw DeltaT / 原始 DeltaT

For every accepted EXOGAM crystal fire, ADNE reads:

```cpp
rawDeltaT = frame->ExoGetDeltaT();
```

This is the TDC-like value associated with the crystal fire. In the NFS interpretation, it is reversed and converted to ns:

```cpp
fDeltaT = (65536 - rawDeltaT) * 0.024
```

Here `0.024` is the current hard-coded time gain in ns/channel. It is inherited from the online NFS patch in ADNE. It gives a quick NFS time scale, but it is not yet a full per-crystal gain calibration.

这里 `0.024` 是当前代码中硬编码的时间增益，单位 ns/channel。它来自 ADNE 中面向在线快速检查的 NFS patch，目前还不是逐 crystal 的完整时间增益刻度。

### 5.2 Global time correction / 全局时间修正

When per-crystal correction is disabled or unavailable:

```cpp
Time = fNeutronTOF = fDeltaT + nfs_gammaFlashOffset
```

In the current code and plots, this final corrected quantity is called `Time`. It is the neutron-TOF-like analysis time in ns.

当前代码和图中统一把最终修正后的量称为 `Time`，单位 ns。

### 5.3 Per-crystal gamma-flash correction / 逐 crystal gamma flash 修正

When `crystal_time_correction: true`, ADNE reads the text table produced by:

```text
lsy_nfs/fit_nfs_deltaT_gamma_flash.C
```

Text columns:

```text
clover crystal detector gammaMean_ns gammaFwhm_ns gammaSigma_ns fitStatus entries
```

For each valid crystal, ADNE computes:

```cpp
gammaFlightNs = (nfs_distance + cloverDistance) / c
offset[detector] = fittedGammaFlashPeakNs - gammaFlightNs
Time = fNeutronTOF = fDeltaT - offset[detector]
```

where:

```text
c = 0.299792458 m/ns
```

Physical meaning:

- `fDeltaT` is the reversed raw TDC time in ns.
- The fitted gamma-flash peak is the observed prompt gamma time for that crystal.
- `gammaFlightNs` is the expected light-flight time from converter/source geometry to that clover.
- `offset = observed gamma flash - expected light flight` removes per-crystal timing offsets.

物理含义：

- `fDeltaT` 是翻转后的原始 TDC 时间，单位 ns。
- 拟合的 gamma flash 峰位是该 crystal 观测到的 prompt gamma 时间。
- `gammaFlightNs` 是由几何距离估算的光飞行时间。
- `offset = 观测 gamma flash 峰位 - 几何光飞行时间`，用于消除不同 crystal 的时间偏置。

Important limitation:

Only the per-crystal offset is corrected at the moment. The gain is still the hard-coded `0.024 ns/channel`. A later version should replace this with detector-dependent `TCoef` gain and offset or another dedicated time calibration.

重要限制：

当前只修正逐 crystal 偏置，尚未修正逐 crystal 增益；时间增益仍为统一的 `0.024 ns/channel`。后续应考虑使用 `TCoef` 或专门时间刻度来替代。

### 5.4 Neutron energy / 中子能量

The neutron energy is computed from the corrected time:

```cpp
E_n = 0.5 * 939.5654 * pow(nfs_distance / (Time * 0.299), 2)
```

This is the non-relativistic kinetic-energy relation:

```text
E = 1/2 * m_n * v^2
v/c = L / (Time * c)
```

where:

- `m_n = 939.5654 MeV/c^2`;
- `L = nfs_distance`, currently 8.6 m by default;
- `Time` is in ns;
- `c` is approximately `0.299 m/ns`.

## 6. NFS Cuts in ADNE / ADNE 内 NFS cut

### 6.1 Time-valid cut / 有效时间 cut

For NFS time branches and NFS spectra, the current valid-time condition is:

```cpp
rawDeltaT <= 60000 && Time > 0
```

Only if this condition is true does the code fill:

```text
fDeltaT
fNeutronTOF
fTime
NFS crystal Time/Energy spectra
```

If the condition fails, the event may still contain the ordinary `fEXO_ECC_*` crystal information, but its NFS analysis `Time` is stored as zero or not filled into the NFS time vectors.

如果该条件不满足，事件仍可能保留普通 `fEXO_ECC_*` crystal 信息，但 NFS 分析时间不会作为有效时间填入。

### 6.2 Spectrum fill cuts / 谱填充 cut

Crystal-level NFS spectra are filled only when:

```cpp
energy > 0 && Time > 0
```

Clover addback spectra are filled only when:

```cpp
energy > 0
```

The BGO/CSI veto clover spectrum is filled only when:

```cpp
energy > 0 && BGO <= 0 && CSI <= 0
```

This avoids adding artificial zero-energy gamma events to histograms.

这样可以避免把 gamma 能量为 0 的空事例或只有 veto detector 响应的事例填进 gamma 直方图。

## 7. NFS Tree Outputs / NFS Tree 输出

When `nfs_exo_ana.tree: true`, ADNE writes separate files:

```text
out/nfs_<original_output_name>.root
out/mult3_nfs_<original_output_name>.root
```

These are independent from the original ADNE ROOT tree output.

### 7.1 Main NFS tree / 主 NFS tree

The main NFS tree is named:

```text
TreeMaster
```

It keeps the normal EXOGAM2 branches plus new NFS branches:

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

- `fDeltaT`: reversed raw DeltaT converted to ns: `(65536 - rawDeltaT) * 0.024`.
- `fNeutronTOF`: corrected NFS Time in ns.
- `fTime`: same analysis Time used by later NFS code; if correction is enabled, it is the corrected per-crystal time.
- `f_E877_Clover`: clover number after raw crystal fires are grouped by clover.
- `f_E877_Clover_E`: clover addback gamma energy in keV.
- `f_E877_Clover_T`: clover time; chosen as the Time of the highest-energy crystal fire in that clover.
- `f_E877_Clover_BGO`: summed BGO signal associated with that clover's crystal fires.
- `f_E877_Clover_CSI`: summed CSI signal associated with that clover's crystal fires.

### 7.2 E877-style clover addback / E877 格式 clover 合并

For each event, crystal fires are grouped by clover:

```text
same clover crystal energies -> summed clover energy
clover time -> time of the highest-energy crystal fire in that clover
BGO/CSI -> summed from matching ESS entries
```

This E877-style addback is performed before ADNE's original prompt/addback/anti-Compton decisions. It is meant to be a cleaner NFS-specific representation of clover fires.

该 E877 合并发生在 ADNE 原本 prompt/addback/anti-Compton 判断之前，目标是得到更符合当前 NFS 标准的 clover fire 表示。

### 7.3 Mult3 NFS tree / 多重度筛选 NFS tree

For each event, the code counts clovers with:

```text
BGO <= 0 && CSI <= 0
```

The event is written to the `mult3_nfs_*.root` tree only when:

```text
vetoed E877 clover multiplicity >= 3
```

This is an early fission-candidate tree. Further time and neutron-energy binning is done later by `lsy_nfs/fission_event_ana.C`.

这是初步裂变候选事例 tree。后续更细的时间 cut 和中子能量分 bin 由 `lsy_nfs/fission_event_ana.C` 完成。

## 8. NFS Spectra Produced by ADNE / ADNE 直接生成的 NFS 谱

When `nfs_exo_ana.spec: true`, ADNE writes:

```text
out/nfs_histoExogam2_1.root
```

Main histograms:

```text
nfs_all_crystal_time_vs_energy
nfs_all_crystal_time
nfs_cloverX_crystalY_time
nfs_cloverX_crystalY_energy
nfs_crystal_cross_talk
nfs_crystal_bgo_efficiency
nfs_crystal_csi_efficiency
nfs_cloverX_addback_gamma
nfs_cloverX_addback_gamma_bgo_csi_veto
```

Meaning:

- `nfs_all_crystal_time_vs_energy`: all accepted crystal fires, x = corrected Time in ns, y = gamma energy in keV.
- `nfs_all_crystal_time`: all accepted crystal Times together.
- `nfs_cloverX_crystalY_time`: per-crystal Time spectrum.
- `nfs_cloverX_crystalY_energy`: per-crystal gamma energy spectrum.
- `nfs_crystal_cross_talk`: event-level crystal cross-talk matrix. If one event has two or more positive-energy crystal fires, all off-diagonal crystal pairs are filled symmetrically.
- `nfs_crystal_bgo_efficiency`: per-crystal BGO fire efficiency stored as a `TProfile`; denominator is a positive-energy crystal fire, numerator is BGO `> 0` for that crystal entry.
- `nfs_crystal_csi_efficiency`: per-crystal CSI fire efficiency stored as a `TProfile`; denominator is a positive-energy crystal fire, numerator is CSI `> 0` for that crystal entry.
- `nfs_cloverX_addback_gamma`: per-clover raw addback gamma energy.
- `nfs_cloverX_addback_gamma_bgo_csi_veto`: per-clover raw addback gamma energy with BGO/CSI veto.

Current histogram ranges:

```text
Time: 0 to 1600 ns
Crystal energy: 0 to 20000 keV
Clover addback energy: 0 to 4096 keV
Crystal cross talk: 64 x 64 bins, detector = clover * 4 + crystal
BGO/CSI efficiency: 64 profile bins, detector = clover * 4 + crystal
```

## 9. ROOT Helper Scripts in lsy_nfs / lsy_nfs 下的 ROOT 脚本

The test-clone path requested by the user is:

```text
/home/user0/work/IJCLAB/NFS/test/nfs-th232-analysis/nfs-th232-analysis-adne/lsy_nfs
```

The same scripts are intended to live under the ADNE project `lsy_nfs/` directory. They use ROOT directly and do not require Python/PyROOT.

### 9.1 extract_nfs_energy_peaks.C

Purpose:

Find the peak position in each crystal energy spectrum inside a selected energy window.

默认用途：

在每个 crystal 能量谱中，在 `[1440,1470] keV` 区间内找最大 bin，作为该 crystal 的峰位。

Input:

```text
out/nfs_histoExogam2_1.root
```

Main input histograms:

```text
nfs_cloverX_crystalY_energy
```

Default output:

```text
out/nfs_histoExogam2_1_peak_1440_1470.root
```

Detector numbering:

```text
detector = clover * 4 + crystal
```

Example:

```bash
cd /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne
source /home/user0/work/IJCLAB/NFS/NFS_env.sh
root -l -b -q 'lsy_nfs/extract_nfs_energy_peaks.C("out/nfs_histoExogam2_1.root")'
```

### 9.2 fit_nfs_deltaT_gamma_flash.C

Purpose:

Fit the gamma-flash peak in every crystal Time spectrum and write a correction table for ADNE.

功能：

遍历所有 `nfs_cloverX_crystalY_time` 直方图，拟合每个 crystal 的 gamma flash 峰，并输出给 ADNE 使用的逐 crystal 时间修正表。

Compatible input names:

```text
nfs_cloverX_crystalY_time
nfs_cloverX_crystalY_deltaT   # legacy compatibility
```

Default fitting strategy:

1. Search and seed the gamma-flash peak in `790-850 ns`.
2. Search the later high peak in `850-1050 ns`.
3. Fit the histogram from `offsetLow` to the later peak maximum.
4. Use a two-Gaussian model:

```text
gamma flash Gaussian + leading half of late high peak Gaussian
```

This is intended to reduce bias from the strong asymmetric later peak while still accounting for overlap.

默认输出：

```text
out/nfs_histoExogam2_1_deltaT_gamma_flash_fit.root
out/nfs_histoExogam2_1_deltaT_gamma_flash_fit.txt
```

Text output columns:

```text
clover crystal detector gammaMean_ns gammaFwhm_ns gammaSigma_ns fitStatus entries
```

The `.txt` file can be assigned to:

```yaml
nfs_exo_ana:
  correction_path: "/absolute/path/to/nfs_histoExogam2_1_deltaT_gamma_flash_fit.txt"
```

Example:

```bash
cd /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne
source /home/user0/work/IJCLAB/NFS/NFS_env.sh
root -l -b -q 'lsy_nfs/fit_nfs_deltaT_gamma_flash.C("out/nfs_histoExogam2_1.root")'
```

If the gamma-flash search window changes:

```bash
root -l -b -q 'lsy_nfs/fit_nfs_deltaT_gamma_flash.C("out/nfs_histoExogam2_1.root", "", 700, 790, 850, 1050)'
```

### 9.3 fission_event_ana.C

Purpose:

Analyze `mult3_nfs_*.root` files and produce neutron-energy/time-gated gamma spectra and gamma-gamma matrices.

用途：

基于 `mult3_nfs_*.root` 中的 `f_E877_Clover_*` 分支，对裂变候选事例做进一步时间 cut、中子能量分 bin、一维 gamma 谱和二维 gamma-gamma 符合矩阵。

Input branches:

```text
f_E877_Clover
f_E877_Clover_E
f_E877_Clover_T
f_E877_Clover_BGO
f_E877_Clover_CSI
```

Time cut:

```text
T_min = ToF(E_n = 50 MeV, L = nfs_distance)
sigma_time = time_FWHM / 2.355
T_cut = T_min - 3 * sigma_time
```

Per-fire cuts:

```text
energy > 0
time > 0
time >= T_cut
BGO <= 0 && CSI <= 0    # enabled by default
```

Per-event cut after removing failed fires:

```text
remaining clover multiplicity >= 3
```

Event time:

```text
Event_neutron_Time = minimum Time among remaining clover fires
```

Neutron-energy binning:

The user supplies bin upper edges, for example:

```text
2,4,7,12,20,30,50
```

These correspond to neutron-energy bins:

```text
0-2, 2-4, 4-7, 7-12, 12-20, 20-30, 30-50 MeV
```

The macro converts each neutron-energy edge into a ToF edge using the same non-relativistic relation. Higher neutron energy corresponds to shorter ToF.

Outputs per bin:

```text
Mult3GammaSpec_E*_MeV
Mult3GammaGammaMatrix_E*_MeV
Mult3GammaEnergyVsTime
```

`Mult3GammaEnergyVsTime` is a 2D diagnostic histogram filled with the same fission-candidate selection as the spectra and matrices. Its x axis is clover addback gamma energy in keV, and its y axis is clover Time in ns.

The gamma-gamma matrix is filled symmetrically. For one event with clover addback energies `[a,b,c]`, the fill pairs are:

```text
(a,b), (b,a), (a,c), (c,a), (b,c), (c,b)
```

Energy range:

```text
0 to 4096 keV
4096 x 4096 bins for gamma-gamma matrices
```

Example:

```bash
cd /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne
source /home/user0/work/IJCLAB/NFS/NFS_env.sh
root -l -b -q 'lsy_nfs/fission_event_ana.C("out/mult3_nfs_run_100_r0.root",8.6,20,"2,4,7,12,20,30,50")'
```

Multiple inputs:

```bash
root -l -b -q 'lsy_nfs/fission_event_ana.C("out/mult3_nfs_run_100_r0.root,out/mult3_nfs_run_100_r1.root",8.6,20,"2,4,7,12,20,30,50","out/fission_event_ana.root")'
root -l -b -q 'lsy_nfs/fission_event_ana.C("out/mult3_nfs_run_100_r*.root",8.6,20,"2,4,7,12,20,30,50","out/fission_event_ana.root")'
```

## 10. Deprecated Two-Step Python Scripts / 已弃用的两步 Python 脚本

Historical path:

```text
/home/user0/work/IJCLAB/NFS/test/nfs-th232-analysis/two-step-nfs-th232-python-scripts
```

Original idea:

1. Run ADNE first to create ROOT files with `TreeMaster`.
2. Run Python/PyROOT scripts on those ROOT files for quick inspection, gamma spectra, neutron-gated plots, detector-performance checks, and text dumps.

旧流程思路：

1. 先运行 ADNE，生成带 `TreeMaster` 的 ROOT 文件。
2. 再用 Python/PyROOT 脚本读取 ROOT 文件，做快速检查、gamma 谱、中子能量门控、探测器性能检查和文本 dump。

### 10.1 inspect_first_events.py

Purpose:

Dump the first N events from `TreeMaster` to a text file and write simple branch histograms.

用途：

查看 ROOT tree 里前若干个 event，每个 event 的分支内容和值；早期用于确认 ROOT 文件里是 tree 还是 histograms，以及排查 branch 读取问题。

Status:

Historical/debug only.

### 10.2 plot_exogam2_energy.py

Purpose:

Quick gamma-spectrum checks versus neutron energy using ADNE-produced branches.

Main inputs:

```text
fEXO_ECC_E_*
sEXO_Gamma*
aEXO_Gamma*
fEXO_Neutron_NRJ
```

Main outputs included:

- raw `fEXO` crystal spectra;
- selected `sEXO` crystal spectra;
- stored `aEXO` addback spectra;
- neutron-energy spectrum;
- neutron-gated `aEXO` addback spectra;
- neutron-gated `aEXO` gamma-gamma matrices.

Important cuts:

- gamma energies `<= 0` are skipped;
- default gamma range was `0-4096 keV`;
- neutron gates were either default `0-50 MeV` split into 10 bins or user-provided by `-i`.

Large-file strategy:

- partial output per input file;
- checkpoints every 500000 events;
- resume support;
- final merge step;
- optional append into existing output with user confirmation.

Status:

Not used in the current standard workflow because it relies on ADNE's original `fEXO/sEXO/aEXO` event decisions.

### 10.3 check_detector_performance.py

Purpose:

Detector-performance checks from ADNE ROOT trees.

Outputs included:

- all-detector `fEXO`, `sEXO`, and `aEXO` spectra;
- per-clover spectra;
- per-crystal spectra;
- `aEXO` clover-clover response matrix;
- `fEXO` and `sEXO` crystal-crystal response matrices;
- BGO/CSI fire-efficiency histograms per clover and per crystal.

BGO/CSI efficiency definition:

```text
denominator = event where the clover/crystal has positive fEXO gamma energy deposition
numerator   = denominator event with matching BGO or CSI above threshold
```

Default thresholds:

```text
BGO > 0
CSI > 0
```

Status:

Useful as an early detector quick-check reference, but not part of the current standard NFS analysis.

## 11. Recommended Practical Sequence / 推荐实际运行顺序

One practical analysis iteration is:

1. Run ADNE once with `nfs_gammaFlashOffset` chosen so the uncorrected Time spectra keep the gamma flash in the visible region.
2. Fit gamma flash:

```bash
root -l -b -q 'lsy_nfs/fit_nfs_deltaT_gamma_flash.C("out/nfs_histoExogam2_1.root")'
```

3. Set:

```yaml
nfs_exo_ana:
  crystal_time_correction: true
  correction_path: "/absolute/path/to/nfs_histoExogam2_1_deltaT_gamma_flash_fit.txt"
```

4. Re-run ADNE to produce corrected:

```text
out/nfs_histoExogam2_1.root
out/nfs_*.root
out/mult3_nfs_*.root
```

5. Run `fission_event_ana.C` on `mult3_nfs_*.root` to produce neutron-energy/time-binned spectra, matrices, and the selected clover gamma-energy versus Time map.

## 12. Known Caveats / 已知注意事项

- The upstream MFM event build is still assumed. The current code does not redefine the event-building time window.
- The current per-crystal time correction corrects offsets only, not gains.
- The hard-coded time gain is still `0.024 ns/channel`.
- The neutron-energy formula is non-relativistic and was inherited from the ADNE NFS patch.
- `mult3_nfs_*.root` applies an early BGO/CSI veto multiplicity selection. `fission_event_ana.C` can re-apply BGO/CSI veto and adds a stricter physical time cut.
- ADNE crystal BGO/CSI efficiencies use a simple fire definition: BGO `> 0` or CSI `> 0`; no additional amplitude threshold is applied in these new NFS diagnostic profiles.
- The old Python two-step workflow should not be used for final standard results because it depends on the original ADNE post-processed `fEXO/sEXO/aEXO` logic.

