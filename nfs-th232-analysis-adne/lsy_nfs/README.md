# LSY NFS ROOT Helpers

这个目录存放只依赖 ROOT 的 NFS 快速检查脚本。
This directory contains ROOT-only helper macros for NFS quick checks.

## Energy Peak Check / 能量峰位检查

在每个 crystal 能量谱中，在给定能量窗口内取最大 bin 作为峰位。
For each crystal energy spectrum, the maximum bin in the selected energy window is used as the peak position.

```bash
cd /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne
source /home/user0/work/IJCLAB/NFS/NFS_env.sh
root -l -b -q 'lsy_nfs/extract_nfs_energy_peaks.C("out/nfs_histoExogam2_1.root")'
```

默认窗口是 `[1440,1470] keV`。
The default window is `[1440,1470] keV`.

输出文件名类似：
Output file name:

```text
out/nfs_histoExogam2_1_peak_1440_1470.root
```

## DeltaT Gamma-Flash Fit / DeltaT Gamma Flash 拟合

自动遍历输入 ROOT 文件中的 `nfs_clover*_crystal*_time` 直方图；同时兼容旧文件中的 `nfs_clover*_crystal*_deltaT` 命名。
The macro automatically scans `nfs_clover*_crystal*_time` histograms in the input ROOT file, with legacy `nfs_clover*_crystal*_deltaT` compatibility.

拟合模型：
Fit model:

```text
gamma flash Gaussian + leading half of late high peak Gaussian
```

其中 gamma flash 默认在 `790-850 ns` 内寻找和拟合；后面的高峰默认在 `850-1050 ns` 内取最大位置作为峰顶，只拟合从 offset 起点到该峰顶之间的数据。
The gamma flash is searched and fitted in `790-850 ns` by default. The nearby late high peak maximum is searched in `850-1050 ns` by default and treated as its peak cap; only the region from the offset start to that maximum is fitted.

```bash
cd /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne
source /home/user0/work/IJCLAB/NFS/NFS_env.sh
root -l -b -q 'lsy_nfs/fit_nfs_deltaT_gamma_flash.C("out/nfs_histoExogam2_1.root")'
```

如果 `nfs_gammaFlashOffset` 或后峰位置改变，可以显式给出 offset 起点、gamma flash 搜索窗口、后峰搜索上限：
If `nfs_gammaFlashOffset` or the late peak position changes, the offset start, gamma-flash window, and late-peak search upper limit can be passed explicitly:

```bash
root -l -b -q 'lsy_nfs/fit_nfs_deltaT_gamma_flash.C("out/nfs_histoExogam2_1.root", "", 700, 790, 850, 1050)'
```

输出文件名类似：
Output file name:

```text
out/nfs_histoExogam2_1_deltaT_gamma_flash_fit.root
out/nfs_histoExogam2_1_deltaT_gamma_flash_fit.txt
```

文本文件列格式：
Text-file columns:

```text
clover crystal detector gammaMean_ns gammaFwhm_ns gammaSigma_ns fitStatus entries
```

这个 `.txt` 文件用于检查 gamma-flash 拟合质量；当前 ADNE 运行时的 `nfs_exo_ana.correction_path` 应指向 `ecc.cal`。后 64 行时间刻度中，`offset` 是相对全局 gamma-flash offset 的差值，`gain` 是相对 `0.024 ns/channel` 的比例，`gain2` 暂时预留不用。
This `.txt` file is used to inspect the gamma-flash fit quality; the current ADNE runtime `nfs_exo_ana.correction_path` should point to `ecc.cal`. In the second block of 64 lines, `offset` is relative to the global gamma-flash offset, `gain` is the ratio to `0.024 ns/channel`, and `gain2` is reserved for now.

主要输出对象：
Main output objects:

- `fit_canvases/`: one canvas per crystal, with histogram and fit curves / 每个 crystal 一张叠加拟合曲线的图
- `nfs_deltaT_gamma_flash_fit_table`: fit result table / 拟合结果表
- `nfs_deltaT_gamma_flash_mean_by_detector`: detector number vs gamma-flash mean / 探测器编号-峰位
- `nfs_deltaT_gamma_flash_fwhm_by_detector`: detector number vs FWHM / 探测器编号-半高宽

探测器编号定义沿用 ADNE `MapFinger`：
Detector number follows ADNE `MapFinger`:

```text
detector = clover * 4 + crystal
```


## Crystal Energy/Time Calibration / Crystal 能量和时间刻度

从一个或多个 `nfs_run_*.root` 的 `TreeMaster` 中读取 `fEXO_ECC_E_Clover`、`fEXO_ECC_E_Cristal`、`fEXO_ECC_E_Energy` 和时间分支，重新构建每个 crystal 的能量谱和时间谱。
The macro reads `fEXO_ECC_E_Clover`, `fEXO_ECC_E_Cristal`, `fEXO_ECC_E_Energy`, and a selected time branch from one or more `nfs_run_*.root` files, then rebuilds per-crystal energy and time spectra.

能量谱设置：`0.5/bin`，范围 `[0,4096]`。
Energy histogram setting: `0.5/bin` in `[0,4096]`.

默认拟合三个能量窗口中的最强峰：
Default energy peak windows:

```text
[440,560]    -> 511 keV
[850,950]    -> 911.2 keV
[2400,2800]  -> 2614.511 keV
```

每个峰用 `Gaussian + linear background` 拟合，然后用 `E_cal = offset + gain * E_raw` 做线性刻度。
Each peak is fitted with `Gaussian + linear background`, then a linear calibration `E_cal = offset + gain * E_raw` is produced.

时间谱默认使用 `fTime`。在 `0-800 ns` 内找最高峰，先从半高点估计粗略 FWHM，用该宽度做纯高斯 seed 拟合；最终联合拟合范围采用约 `2*FWHM`。随后用 `Gaussian + radioactive build-up` 联合拟合。build-up 项起点可移动，但限制在高斯峰心 `±15 ns` 内；起点之后按近似常数生成率同时衰变：`A*(1-exp(-(t-t0)/tau))`。输出中同时给出 `time_offset_to_442_ns = 442 - peak` 和 `time_gain_to_442 = 442 / peak`。
The default time branch is `fTime`. The strongest peak below `800 ns` is first given a rough FWHM estimate from half-height crossings, and that width defines the Gaussian seed fit. The final combined fit range is about `2*FWHM`. It is then refitted with `Gaussian + radioactive build-up`. The build-up start time is allowed to move, but is constrained within `±15 ns` of the Gaussian mean; after that start it follows constant production with decay: `A*(1-exp(-(t-t0)/tau))`. The output includes both `time_offset_to_442_ns = 442 - peak` and `time_gain_to_442 = 442 / peak`.

```bash
cd /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne
source /home/user0/work/IJCLAB/NFS/NFS_env.sh
root -l -b -q 'lsy_nfs/calibrate_nfs_crystal_energy_time.C("out/nfs_run_23_r0.root")'
```

多文件输入可以逗号分隔，或者使用 `@filelist.txt`：
Multiple files can be comma-separated or passed via `@filelist.txt`:

```bash
root -l -b -q 'lsy_nfs/calibrate_nfs_crystal_energy_time.C("out/nfs_run_23_r0.root,out/nfs_run_24_r0.root","out/run23_24_cal")'
root -l -b -q 'lsy_nfs/calibrate_nfs_crystal_energy_time.C("@out/nfs_run_files.txt","out/run_cal",-1,"fTime")'
```

参数顺序：
Arguments:

```text
inputFiles, outputPrefix, maxEntries, timeBranchLeaf, energyFitHalfWidth, timeSearchHighNs
```

主要输出：
Main outputs:

```text
<prefix>.root
<prefix>.txt
<prefix>_ecc_cal_candidate.txt
```

- `<prefix>.root`: rebuilt spectra and fit canvases / 重建谱和拟合画布
- `<prefix>.txt`: detailed peak, energy-calibration, and time-fit table / 详细峰位、能量刻度和时间拟合表
- `<prefix>_ecc_cal_candidate.txt`: 64 energy lines plus 64 time lines in `offset gain gain2` format / 接近 `ecc.cal` 的候选格式，前 64 行能量，后 64 行时间

批量执行多个 ROOT 文件刻度时，可以使用外层脚本。每个输入会独立调用一次 ROOT 宏，并写出一个 TSV 列表，记录每个输入对应的 `.root`、`.txt` 和 `_ecc_cal_candidate.txt`。
For batch calibration, use the wrapper script below. Each input is calibrated by one ROOT macro call, and a TSV manifest records the corresponding `.root`, `.txt`, and `_ecc_cal_candidate.txt` outputs.

```bash
cd /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne
source /home/user0/work/IJCLAB/NFS/NFS_env.sh

./lsy_nfs/batch_calibrate_nfs_crystals.sh \
  --out-dir out/calibration_run23 \
  out/nfs_run_23_r0.root out/nfs_run_23_r1.root
```

也可以把输入 ROOT 文件逐行写入列表：
Inputs can also be passed via a text list:

```bash
./lsy_nfs/batch_calibrate_nfs_crystals.sh \
  --input-list out/nfs_run_files.txt \
  --out-dir out/calibration_all \
  --manifest out/calibration_all/calibration_outputs.tsv \
  --max-entries -1 \
  --time-branch fTime
```

默认 manifest 列格式：
Default manifest columns:

```text
index input output_prefix output_root output_txt output_ecc log status
```

如果需要先从一个输出目录及其子目录自动生成 `nfs_run_*_r*.root` 输入列表，可以使用：
To generate an input list from an output directory recursively:

```bash
./lsy_nfs/list_nfs_run_roots.sh \
  --absolute \
  --output out/nfs_run_files.txt \
  /data/e877_anaX/July8beamLsy/run30
```

默认只匹配 `nfs_run_*_r*.root`，不会包含 `mult3_nfs_run_*.root` 或 `nfs_histoExogam2_*.root`。
By default it only matches `nfs_run_*_r*.root`, excluding `mult3_nfs_run_*.root` and `nfs_histoExogam2_*.root`.

## RawTree EXO Gamma Timing Spectrum / RawTree EXO Gamma 时间谱分析

`analyze_raw_exo_gamma_timing_spectrum.C` 用于在不知道束流周期时，从 RawTree 中的 EXO2 gamma frame 搜索周期成分。
`analyze_raw_exo_gamma_timing_spectrum.C` searches periodic timing components from RawTree EXO2 gamma frames when the beam period is unknown.

筛选条件：
Selection:

```text
raw_frame_type == 0x10
and (raw_exo_inner_m6 > gammaThreshold or raw_exo_inner_m20 > gammaThreshold)
```

也就是说，它只分析 EXO2 frame，并要求有 core gamma 原始能量；不会把 merge/top frame 或只有 BGO/CSI 的 frame 放进周期分析。
It uses only EXO2 frames with positive core gamma raw energy; merge/top frames and BGO/CSI-only frames are excluded from the period search.

默认搜索周期范围是 `200-50000 ns`，步长 `10 ns`，使用 epoch-folding periodogram：对每个候选周期折叠相位，然后计算相位分布相对均匀分布的 `chi-square` 强度。
The default period range is `200-50000 ns` with `10 ns` steps. The method is an epoch-folding periodogram: each trial period is used to fold event phases, and the phase distribution is scored with `chi-square` against a flat distribution.

```bash
cd /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne
source /home/user0/work/IJCLAB/NFS/NFS_env.sh
root -l -b -q 'lsy_nfs/analyze_raw_exo_gamma_timing_spectrum.C("out/nfs_run_23_r0.root")'
```

如果看到某个候选周期，可以缩小范围再精扫，例如：
After a candidate period is found, refine the scan in a narrower range:

```bash
root -l -b -q 'lsy_nfs/analyze_raw_exo_gamma_timing_spectrum.C("out/nfs_run_23_r0.root","RawTree",1200,1500,2,"out/exo_gamma_period_1200_1500.root")'
```

参数顺序：
Arguments:

```text
inputFile, treeName, minPeriodNs, maxPeriodNs, periodStepNs, outputFile,
gammaThreshold, maxSearchFrames, searchPhaseBins, nTopPeriods, foldBins,
timeStartNs, timeStopNs
```

- `gammaThreshold`: default `10`; raw EXO core energy threshold / 默认 `10`，EXO core 原始能量阈值
- `maxSearchFrames`: default `100000`; evenly sampled frames used in the period scan, `0` means use all / 默认 `100000`，周期扫描抽样数，`0` 表示全部使用
- `searchPhaseBins`: default `64`; phase bins used during period search / 默认 `64`，搜索时相位 bin 数
- `nTopPeriods`: default `6`; number of strongest candidates to fold / 默认 `6`，输出折叠图的最强候选数
- `foldBins`: default `512`; phase bins in final folded histograms / 默认 `512`，最终折叠图相位 bin 数
- `timeStartNs`, `timeStopNs`: optional time selection after subtracting first non-zero top TS / 可选时间选择，单位 ns，已减去第一个非零 top TS

主要输出对象：
Main output objects:

- `raw_exo_gamma_periodogram_period_ns`: trial period vs epoch-folding power / 候选周期-谱强度
- `raw_exo_gamma_periodogram_frequency_mhz`: frequency vs epoch-folding power / 频率-谱强度
- `raw_exo_gamma_period_candidates`: strongest candidate table / 最强候选周期表
- `raw_exo_gamma_fold_period_rank*`: selected frames folded by the strongest periods / 按最强候选周期折叠后的相位图
- `raw_exo_gamma_timing_spectrum_config`: run settings and selection counts / 参数和筛选统计

## Fission Event Analysis / 裂变事件分析

基于 ADNE 新产生的 `mult3_nfs_*.root` 文件，读取 `TreeMaster` 中的 `f_E877_Clover_*` 分支，对 veto 后且时间 cut 后仍满足 clover 多重度默认为 `>=2` 的事件做分 bin gamma 谱、gamma-gamma 符合矩阵，以及 clover gamma 能量-时间二维图。
The macro reads `f_E877_Clover_*` branches from ADNE `mult3_nfs_*.root` files and builds gamma spectra, gamma-gamma matrices, plus a clover gamma-energy versus Time map for events that still have clover multiplicity `>=2` by default after veto and time cuts.

时间 cut 使用 ADNE 同源公式反推最高能中子飞行时间：
The time cut uses the same neutron TOF relation as ADNE:

```text
E = 0.5 * m_n * (L / (tof * c))^2
T_min = tof(E = 50 MeV, L = nfs distance)
T_cut = T_min - 3 * (time_FWHM / 2.355)
```

示例：
Example:

```bash
cd /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne
source /home/user0/work/IJCLAB/NFS/NFS_env.sh
root -l -b -q 'lsy_nfs/fission_event_ana.C("out/mult3_nfs_run_100_r0.root",8.6,20,"2,4,7,12,20,30,50")'
```

多文件输入可以用逗号分隔，也可以传 ROOT wildcard：
Multiple input files can be comma-separated or passed with a ROOT wildcard:

```bash
root -l -b -q 'lsy_nfs/fission_event_ana.C("out/mult3_nfs_run_100_r0.root,out/mult3_nfs_run_100_r1.root",8.6,20,"2,4,7,12,20,30,50","out/fission_event_ana.root")'
root -l -b -q 'lsy_nfs/fission_event_ana.C("out/mult3_nfs_run_100_r*.root",8.6,20,"2,4,7,12,20,30,50","out/fission_event_ana.root")'
```

参数顺序：
Arguments:

```text
inputFiles, nfsDistanceMeter, timeFwhmNs, energyBinEdgesMeV, outputFile, useBgoCsiVeto, maxEntries, minCloverMultiplicity
```

- `inputFiles`: input ROOT file(s) / 输入 ROOT 文件
- `nfsDistanceMeter`: NFS flight distance in meters / NFS 飞行距离，单位 m
- `timeFwhmNs`: time FWHM in ns / 时间半高宽，单位 ns
- `energyBinEdgesMeV`: neutron-energy bin upper edges, e.g. `2,4,7,12,20,30,50` / 中子能量分 bin 边界，单位 MeV
- `outputFile`: optional output ROOT file / 可选输出文件
- `useBgoCsiVeto`: default `true`; re-apply BGO/CSI veto to clover fires / 默认 `true`，对 clover fire 再应用 BGO/CSI veto
- `maxEntries`: default `-1`; process all entries / 默认 `-1`，处理全部 entry
- `minCloverMultiplicity`: default `2`; required clover fire multiplicity after cuts / 默认 `2`，cut 后要求的 clover fire 多重度

主要输出对象：
Main output objects:

- `Mult3GammaSpec_E*_MeV`: 1D gamma spectrum for each neutron-energy/time bin / 每个中子能量或时间 bin 的一维 gamma 谱
- `Mult3GammaGammaMatrix_E*_MeV`: 2D gamma-gamma coincidence matrix / 每个 bin 的二维 gamma-gamma 符合矩阵
- `Mult3GammaEnergyVsTime`: selected clover addback gamma energy versus clover Time / 通过裂变候选 cut 的 clover addback gamma 能量-时间二维图
- `c_Mult3GammaSpec_*`, `c_Mult3GammaGammaMatrix_*`, `c_Mult3GammaEnergyVsTime`: canvases with labels / 带标注的画布
- `FissionEventAnaSummary`: processing summary / 处理统计摘要
- `FissionEventAnaConfig`: input parameters / 输入参数记录

