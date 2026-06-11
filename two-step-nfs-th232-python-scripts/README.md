# Two-Step NFS Th-232 Python Scripts

Python utilities for a two-step NFS Th-232 / Exogam2 analysis workflow.

## Workflow

1. Run the ADNE analysis framework to produce ROOT files with `TreeMaster`.
2. Run these Python/PyROOT scripts on the produced ROOT files to make quick checks, spectra, neutron-gated plots, detector-performance plots, and text dumps.

The scripts do not replace ADNE. They are lightweight post-processing tools for checking ADNE ROOT outputs.

## Requirements

- Python 3
- ROOT with PyROOT available in the Python environment
- ADNE-produced ROOT files containing `TreeMaster`

Check PyROOT:

```bash
python3 - <<'PY'
import ROOT
print(ROOT.gROOT.GetVersion())
PY
```

## Scripts

### `scripts/inspect_first_events.py`

Dumps the first events from `TreeMaster` to a text file and writes simple branch histograms.

Example:

```bash
python3 scripts/inspect_first_events.py \
  /home/user0/work/IJCLAB/NFS/data_test_out/run_100_r0.root \
  -n 100 \
  --text-out first100_events.txt \
  --hist-out first100_hists.root
```

### `scripts/plot_exogam2_energy.py`

Fast gamma-spectrum checks versus neutron energy. It supports multiple input ROOT files, per-file partial outputs, checkpointing, resume, and final histogram merging.

Default neutron gates are `0-50 MeV` split into 10 equal bins. Custom neutron-energy gates can be passed with `-i`.

Example:

```bash
python3 scripts/plot_exogam2_energy.py \
  /home/user0/work/IJCLAB/NFS/data_test_out/run_100_r0.root \
  /home/user0/work/IJCLAB/NFS/data_test_out/run_100_r1.root \
  -o gamma_neutron_plots.root \
  -i 0 5 10 15 30
```

Main output histograms include:

- raw `fEXO` crystal spectra
- selected `sEXO` crystal spectra
- stored `aEXO` addback spectra
- neutron-energy spectrum
- neutron-gated `aEXO` addback spectra
- neutron-gated `aEXO` gamma-gamma matrices

### `scripts/check_detector_performance.py`

Detector-performance check for Exogam2 ROOT trees.

It writes:

- all-detector `fEXO`, `sEXO`, and `aEXO` spectra
- per-clover spectra
- per-crystal spectra
- `aEXO` clover-clover response matrix
- `fEXO` and `sEXO` crystal-crystal response matrices
- BGO/CSI fire-efficiency histograms per clover and per crystal

BGO/CSI efficiency definition:

- denominator: one count per event when a clover/crystal has positive gamma energy deposition in `fEXO_ECC_E_*`
- numerator: among those denominator events, one count if matching `fEXO_ESS_BGO` or `fEXO_ESS_CSI` is above threshold
- default thresholds are `BGO > 0` and `CSI > 0`

Example:

```bash
python3 scripts/check_detector_performance.py \
  /home/user0/work/IJCLAB/NFS/data_test_out/run_100_r0.root \
  /home/user0/work/IJCLAB/NFS/data_test_out/run_100_r1.root \
  -o detector_performance_check.root
```

Custom BGO/CSI thresholds:

```bash
python3 scripts/check_detector_performance.py input.root \
  -o detector_performance_check.root \
  --bgo-threshold 20 \
  --csi-threshold 20
```

## Example Run Scripts

Two editable examples are provided:

- `examples/run_gamma_neutron.sh`
- `examples/run_detector_performance.sh`

Update the input ROOT paths and output paths for your local analysis directory, then run:

```bash
cd nfs-th232-analysis/two-step-nfs-th232-python-scripts
./examples/run_gamma_neutron.sh
./examples/run_detector_performance.sh
```

## Files Not Tracked

ROOT data/output files, partial ROOT files, PyROOT caches, and build products are intentionally ignored by `.gitignore`.

