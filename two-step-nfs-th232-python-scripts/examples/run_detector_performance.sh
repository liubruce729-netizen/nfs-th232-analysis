#!/usr/bin/env bash
set -euo pipefail

# Example: Exogam2 detector-performance checks.
# Edit INPUTS and OUTPUT for your local run.

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

INPUTS=(
  /home/user0/work/IJCLAB/NFS/data_test_out/run_100_r11.root
  /home/user0/work/IJCLAB/NFS/data_test_out/run_100_r12.root
  /home/user0/work/IJCLAB/NFS/data_test_out/run_100_r13.root
)

OUTPUT=/home/user0/work/IJCLAB/NFS/data_ana/tool/detector_performance_check.root

python3 "${REPO_DIR}/scripts/check_detector_performance.py" \
  "${INPUTS[@]}" \
  -o "${OUTPUT}"

# Example custom BGO/CSI thresholds:
# add: --bgo-threshold 20 --csi-threshold 20

