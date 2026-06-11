#!/usr/bin/env bash
set -euo pipefail

# Example: quick gamma spectra versus neutron-energy gates.
# Edit INPUTS and OUTPUT for your local run.

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

INPUTS=(
  /home/user0/work/IJCLAB/NFS/data_test_out/run_100_r11.root
  /home/user0/work/IJCLAB/NFS/data_test_out/run_100_r12.root
  /home/user0/work/IJCLAB/NFS/data_test_out/run_100_r13.root
)

OUTPUT=/home/user0/work/IJCLAB/NFS/data_ana/tool/multi_runs_energy_plots.root

python3 "${REPO_DIR}/scripts/plot_exogam2_energy.py" \
  "${INPUTS[@]}" \
  -o "${OUTPUT}"

# Example custom neutron gates:
# add: -i 0 5 10 15 30

