# Standalone NFS Histogram Builder / 独立 NFS 直方图处理

This tool rebuilds every histogram currently stored by ADNE in
`nfs_histoExogam2_1.root`, directly from the primitive `MfmFrameTree` produced
by `mfm_to_raw_root_tree.C`.

本工具直接读取 `mfm_to_raw_root_tree.C` 生成的基础 `MfmFrameTree`，重建 ADNE
当前写入 `nfs_histoExogam2_1.root` 的全部直方图。

It links only ROOT. It does not decode MFM again and does not link ADNE, GRU,
MFMlib, or yaml-cpp. Event grouping uses `top_event_index`; therefore all child
EXO2 frames belonging to the same merged top frame remain in one event.

程序只依赖 ROOT，不再解码 MFM，也不链接 ADNE、GRU、MFMlib 或 yaml-cpp。事件
按照 `top_event_index` 分组，因此同一个 merged top frame 的所有 EXO2 子 frame
仍属于同一个 event。

## Build / 编译

```bash
cd /path/to/nfs-th232-analysis/standalone-nfs-histo
./build.sh
```

The executable is built with `-O3` at
`bin/build_nfs_histo_from_raw_root`.

## Recommended Run / 推荐运行

The wrapper automatically uses the ADNE YAML, LUT, and `ecc.cal` from the same
repository:

```bash
./run_nfs_histo_from_raw_root.sh \
  --input /home/user0/work/IJCLAB/nfs_ana/root_tree/run134_0.root \
  --output /home/user0/work/IJCLAB/nfs_ana/root_tree/nfs_histo_run134_0.root
```

Quick test using only the first 100,000 top events:

```bash
./run_nfs_histo_from_raw_root.sh \
  --input /home/user0/work/IJCLAB/nfs_ana/root_tree/run134_0.root \
  --output /tmp/nfs_histo_run134_first100k.root \
  --max-events 100000
```

Multiple input files are accumulated into the same histograms:

```bash
./run_nfs_histo_from_raw_root.sh \
  --input run134_0.root --input run134_1.root \
  --output nfs_histo_run134.root
```

or:

```bash
./run_nfs_histo_from_raw_root.sh \
  --input-list raw_root_files.txt \
  --output nfs_histo_run134.root
```

`raw_root_files.txt` contains one ROOT path per line. Blank lines and lines
starting with `#` are ignored.

## Configuration Equivalence / 配置对齐

The wrapper reads:

- `nfs-th232-analysis-adne/Yaml_config_files/config.yaml`
- `nfs-th232-analysis-adne/Conf/Exogam2_in_Tree.cfg`
- `nfs-th232-analysis-adne/CalFile/ecc.cal`

The standalone YAML reader intentionally extracts only settings that affect the
NFS histograms:

- active `clover0` ... `clover15`;
- `nfs_gammaFlashOffset` and `nfs_neutron`;
- `crystal_time_correction`, `correction_path`, and `disabled_crystals`;
- `readcal_file` and `use_default_cal`.

Command-line values are applied after YAML and therefore override it:

```bash
./run_nfs_histo_from_raw_root.sh \
  --input run.root --output hist.root \
  --lut /actual/run/Conf/Exogam2_in_Tree.cfg \
  --energy-cal /actual/run/CalFile/ecc.cal \
  --active-clovers 2-13 \
  --disabled-crystals 7-1,10-3 \
  --gamma-flash-offset -196.7 \
  --time-correction true \
  --time-cal /actual/run/CalFile/ecc.cal
```

The LUT and calibration files must be the files used for that acquisition.
An old LUT is not interchangeable with a new electronics mapping. Unknown
`(board, half-board)` combinations are skipped exactly as in
`TExogam2::IsMFMExo`, printed at the end, and saved in
`nfs_histo_unknown_lut`.

LUT 与刻度文件必须对应实际 acquisition。旧 LUT 不能替代新的电子学映射。找不到
的 `(board, half-board)` 与 `TExogam2::IsMFMExo` 一样会被跳过，并在终端和
`nfs_histo_unknown_lut` 中列出。

## Reproduced ADNE Logic / 复现的 ADNE 逻辑

For each EXO2 frame:

1. Map `(exo_board_id, exo_lut_halfboard)` to `(clover, crystal)` using the LUT.
2. Require an active clover and `exo_inner6m > 10`.
3. Apply the first 64 `ecc.cal` rows:
   `E = offset + gain*x + gain2*x*x`, with ADNE's optional `+/-0.5` ADC-channel
   dither.
4. Build Time exactly as the current NFS branch:
   - without crystal correction:
     `(65536-DeltaT)*0.024 + nfs_gammaFlashOffset`;
   - with crystal correction:
     `(65536-DeltaT)*0.024*relative_gain + nfs_gammaFlashOffset + relative_offset`;
   - valid Time additionally requires `rawDeltaT <= 60000` and `Time > 0`.
5. Fill crystal energy/time, BGO/CSI energy, fire/veto, TS-difference, and
   TS-phase-minus-TDC histograms.
6. At the end of each `top_event_index`, combine all enabled positive-energy
   crystal fires by clover. BGO and CSI channels are summed without an extra
   threshold.
7. Fill addback, BGO/CSI-veto addback, event-level crystal cross talk, BGO/CSI
   efficiency profiles, and the ordered-pair no-cut addback gamma-gamma matrix.

The no-cut matrix uses only addback gammas satisfying `BGO <= 0 && CSI <= 0`.
It applies no prompt, Time, TS, neutron-energy, or multiplicity gate.

无 cut 矩阵只使用 `BGO <= 0 && CSI <= 0` 的 addback gamma，不加 prompt、
Time、TS、中子能量或多重度判选。

The matching ADNE source locations are:

- LUT and raw-frame acceptance: `TExogam2.cxx`, `IsMFMExo`;
- NFS histogram definitions: `TExogam2.cxx`, `NfsSpectraConstructor`;
- raw TS diagnostics: `FillNfsTimestampDiagnostics`;
- crystal spectra: `FillNfsSpectra`;
- event cross talk and efficiencies: `FillNfsCrystalEventSpectra`;
- addback and no-cut gamma-gamma: `TExogam2::Treat`.

## Performance / 性能

Only these ten branches are enabled:

`top_event_index`, `is_exo2`, `timestamp`, `exo_board_id`,
`exo_lut_halfboard`, `exo_status3`, `exo_delta_t`, `exo_inner6m`, `exo_bgo`,
and `exo_csi`.

Processing is a single sequential pass. Only one event accumulator is retained,
so memory does not grow with event count. The two largest allocations are the
same fixed-format histograms as ADNE: the 4096x4096 gamma-gamma matrix and the
1600x4000 Time-energy matrix.

## Validation / 验证

Every run performs internal count checks before writing. The checks independently
compare histogram entries with processed-frame/event counters for:

- crystal energy and Time;
- BGO/CSI fire-veto partitions;
- global and per-crystal TS differences;
- BGO/CSI efficiency profile denominators;
- crystal cross talk ordered pairs;
- clover addback and veto addback;
- no-cut gamma-gamma ordered pairs.

`validation: PASS` means all these invariants agree. Processing counters are
also written to `nfs_histo_processing_summary`.

To compare against an ADNE output from the same input and exactly the same
configuration:

```bash
root -l -b -q \
  'compare_nfs_histo.C("standalone.root","adne_nfs_histo.root","comparison.tsv",1e-6)'
```

The energy calibration uses ADNE-style random sub-channel dithering. The tool
also consumes the legacy TDC/GOCCE random calls so that the sequence matches the
current configuration with PSA disabled. If a different ADNE run consumes
additional random numbers elsewhere, entries and spectra remain statistically
equivalent but a few 1-keV addback bins can differ at bin boundaries. Use
`--dither false` on both reference implementations when a deterministic
cell-by-cell test is required.

## Local Full-File Test / 本地完整文件测试

On 2026-07-15 the complete
`/home/user0/work/IJCLAB/nfs_ana/root_tree/run134_0.root` was processed:

- 89,133,292 `MfmFrameTree` rows;
- 45,407,226 top events;
- 72,173,312 EXO2 frames;
- all 16 internal histogram checks passed;
- 39.45 s wall time and about 526 MiB maximum resident memory on this machine;
- 512 ADNE-compatible histograms were written.

The 122 histograms shared with an older local ADNE `nfs_histo` file had identical
names, classes, dimensions, and binning. The remaining 390 histograms are newer
diagnostics present in the current ADNE source but absent from that older output.

This test also found that the repository's 2023 LUT does not describe all
electronics in run134. Boards 112, 114, 115, 120, 131, and 140 account for
18,460,271 skipped frames. This is an input-configuration mismatch, not a
standalone reconstruction failure. Reprocess run134 with the LUT and `ecc.cal`
actually used at the experiment before using the spectra for physics.
