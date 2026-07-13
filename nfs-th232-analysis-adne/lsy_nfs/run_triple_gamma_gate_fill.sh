#!/usr/bin/env bash
set -euo pipefail

# Fill gated gamma-gamma matrices from existing calibrated compact ROOT trees.
# 从已经刻度完成的紧凑 ROOT Tree 填充带 gate 的 gamma-gamma 矩阵。
#
# Example / 示例:
# ./run_triple_gamma_gate_fill.sh \
#   --compact-input-list /data/e877_anaX/recalibrated/july07to10no511/GGG.root.parts/compact_inputs.txt \
#   --output /data/e877_anaX/recalibrated/july07to10no511/GGG_gated.root \
#   --gates 502,365,485 \
#   --gate-width 3 \
#   --neutron-bins 10,15,20,30,40 \
#   --gamma-bins 512 \
#   --gamma-min 0 \
#   --gamma-max 4096

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MACRO="${SCRIPT_DIR}/triple_gamma_gate_fill.C"
ROOT_CMD="root"
COMPACT_INPUT_LIST=""
OUTPUT=""
GATES=""
GATE_WIDTH=3
NEUTRON_BINS="10,15,20,30,40"
GAMMA_BINS=512
GAMMA_MIN=0
GAMMA_MAX=4096
MIN_MULT=3
MAX_ENTRIES=-1

usage() {
  cat <<'EOF'
Required / 必需:
  --compact-input-list FILE   Existing compact_inputs.txt
  --output FILE               Output ROOT file
  --gates CSV                 Gate centers in keV, e.g. 502,365,485

Optional / 可选:
  --gate-width KEV            Number of 1-keV gate channels; 3 means center +/-1
  --neutron-bins CSV          Neutron energy edges, default 10,15,20,30,40 MeV
  --gamma-bins N              Matrix bins per axis, default 512
  --gamma-min KEV             Matrix lower edge, default 0
  --gamma-max KEV             Matrix upper edge, default 4096
  --min-mult N                Minimum compact clover multiplicity, default 3
  --max-entries N             Limit compact entries for testing, default all
  --macro FILE                ROOT macro path
  --root COMMAND              ROOT executable, default root
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --compact-input-list|--input-list) COMPACT_INPUT_LIST="$2"; shift 2 ;;
    --output) OUTPUT="$2"; shift 2 ;;
    --gates) GATES="$2"; shift 2 ;;
    --gate-width) GATE_WIDTH="$2"; shift 2 ;;
    --neutron-bins) NEUTRON_BINS="$2"; shift 2 ;;
    --gamma-bins) GAMMA_BINS="$2"; shift 2 ;;
    --gamma-min) GAMMA_MIN="$2"; shift 2 ;;
    --gamma-max) GAMMA_MAX="$2"; shift 2 ;;
    --min-mult) MIN_MULT="$2"; shift 2 ;;
    --max-entries) MAX_ENTRIES="$2"; shift 2 ;;
    --macro) MACRO="$2"; shift 2 ;;
    --root) ROOT_CMD="$2"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
  esac
done

if [[ -z "$COMPACT_INPUT_LIST" || -z "$OUTPUT" || -z "$GATES" ]]; then
  echo "--compact-input-list, --output, and --gates are required." >&2
  usage >&2
  exit 2
fi
if [[ ! -s "$COMPACT_INPUT_LIST" ]]; then
  echo "Compact input list not found or empty: $COMPACT_INPUT_LIST" >&2
  exit 1
fi
if [[ ! -f "$MACRO" ]]; then
  echo "Gate macro not found: $MACRO" >&2
  exit 1
fi

mkdir -p "$(dirname "$OUTPUT")"
LOG="${OUTPUT}.log"

root_escape() {
  local value="$1"
  value="${value//\\/\\\\}"
  value="${value//\"/\\\"}"
  printf '%s' "$value"
}

macro_arg="$(root_escape "$MACRO")"
list_arg="$(root_escape "$COMPACT_INPUT_LIST")"
output_arg="$(root_escape "$OUTPUT")"
gates_arg="$(root_escape "$GATES")"
neutron_arg="$(root_escape "$NEUTRON_BINS")"

root_call="${macro_arg}(\"@${list_arg}\",\"${output_arg}\",\"${gates_arg}\",${GATE_WIDTH},${GAMMA_BINS},${GAMMA_MIN},${GAMMA_MAX},\"${neutron_arg}\",${MIN_MULT},${MAX_ENTRIES})"

echo "Compact list: $COMPACT_INPUT_LIST"
echo "Gates: $GATES; gate width: $GATE_WIDTH keV channels"
echo "Output: $OUTPUT"
"$ROOT_CMD" -l -b -q "$root_call" 2>&1 | tee "$LOG"

if [[ ! -s "$OUTPUT" ]]; then
  echo "Gate analysis completed without a valid output file: $OUTPUT" >&2
  exit 1
fi

echo "Gated matrices: $OUTPUT"
echo "Log: $LOG"
