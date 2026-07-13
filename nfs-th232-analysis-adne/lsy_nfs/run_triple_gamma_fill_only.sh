#!/usr/bin/env bash
set -euo pipefail

# Draw triple-gamma cubes from already calibrated compact ROOT trees only.
# 仅基于已经刻度完成的紧凑 ROOT Tree 绘制三重 gamma cube，不重复预处理。
#
# Each cube is produced by a separate ROOT process. This guarantees that the
# previous dense histogram and ROOT serialization buffer are fully released.
# 每张 cube 使用独立 ROOT 进程；前一张图及其 ROOT 写缓冲会被彻底释放。
#
# Example / 示例:
# ./run_triple_gamma_fill_only.sh \
#   --compact-input-list /data/e877_anaX/recalibrated/july07to10no511/GGG.root.parts/compact_inputs.txt \
#   --output /data/e877_anaX/recalibrated/july07to10no511/GGG.root \
#   --neutron-bins 10,15,20,30,40 \
#   --gamma-bins 512 \
#   --gamma-min 0 \
#   --gamma-max 4096

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MACRO="${SCRIPT_DIR}/triple_gamma_fill_serial.C"
ROOT_CMD="root"
COMPACT_INPUT_LIST=""
OUTPUT=""
MIN_MULT=3
GAMMA_BINS=512
GAMMA_MIN=0
GAMMA_MAX=4096
NEUTRON_BINS="10,15,20,30,40"
MEMORY_GUARD_GIB=12
HIST_STORAGE="auto"

usage() {
  sed -n '3,20p' "$0" | sed 's/^# \{0,1\}//'
  cat <<'EOF'

Required / 必需:
  --compact-input-list FILE   Existing compact_inputs.txt
  --output FILE               Final ROOT output

Optional / 可选:
  --min-mult N                Minimum clover multiplicity, default 3
  --gamma-bins N              Bins per gamma axis, default 512
  --gamma-min KEV             Gamma lower edge, default 0
  --gamma-max KEV             Gamma upper edge, default 4096
  --neutron-bins CSV          Neutron edges, default 10,15,20,30,40
  --memory-guard-gib GIB      Per-process histogram guard, default 12
  --hist-storage MODE         auto, float, or double; default auto
  --macro FILE                Drawing macro path
  --root COMMAND              ROOT executable, default root
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --compact-input-list) COMPACT_INPUT_LIST="$2"; shift 2 ;;
    --output) OUTPUT="$2"; shift 2 ;;
    --min-mult) MIN_MULT="$2"; shift 2 ;;
    --gamma-bins) GAMMA_BINS="$2"; shift 2 ;;
    --gamma-min) GAMMA_MIN="$2"; shift 2 ;;
    --gamma-max) GAMMA_MAX="$2"; shift 2 ;;
    --neutron-bins) NEUTRON_BINS="$2"; shift 2 ;;
    --memory-guard-gib) MEMORY_GUARD_GIB="$2"; shift 2 ;;
    --hist-storage) HIST_STORAGE="$2"; shift 2 ;;
    --macro) MACRO="$2"; shift 2 ;;
    --root) ROOT_CMD="$2"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
  esac
done

if [[ -z "$COMPACT_INPUT_LIST" || -z "$OUTPUT" ]]; then
  echo "--compact-input-list and --output are required." >&2
  usage >&2
  exit 2
fi
if [[ ! -s "$COMPACT_INPUT_LIST" ]]; then
  echo "Compact input list not found or empty: $COMPACT_INPUT_LIST" >&2
  exit 1
fi
if [[ ! -f "$MACRO" ]]; then
  echo "Drawing macro not found: $MACRO" >&2
  exit 1
fi

mkdir -p "$(dirname "$OUTPUT")"
LOG="${OUTPUT}.fill.log"
: > "$LOG"

root_escape() {
  local value="$1"
  value="${value//\\/\\\\}"
  value="${value//\"/\\\"}"
  printf '%s' "$value"
}

IFS=',' read -r -a neutron_edges <<< "$NEUTRON_BINS"
CUBE_COUNT=$(( ${#neutron_edges[@]} + 1 ))

macro_arg="$(root_escape "$MACRO")"
list_arg="$(root_escape "$COMPACT_INPUT_LIST")"
output_arg="$(root_escape "$OUTPUT")"
bins_arg="$(root_escape "$NEUTRON_BINS")"
storage_arg="$(root_escape "$HIST_STORAGE")"

echo "Compact list: $COMPACT_INPUT_LIST" | tee -a "$LOG"
echo "Output: $OUTPUT" | tee -a "$LOG"
echo "Cubes: $CUBE_COUNT, storage: $HIST_STORAGE" | tee -a "$LOG"

for ((cube_index = 0; cube_index < CUBE_COUNT; ++cube_index)); do
  root_call="${macro_arg}(\"@${list_arg}\",\"${output_arg}\",${MIN_MULT},${GAMMA_BINS},${GAMMA_MIN},${GAMMA_MAX},\"${bins_arg}\",${MEMORY_GUARD_GIB},${cube_index},\"${storage_arg}\")"
  echo "Start cube $((cube_index + 1))/${CUBE_COUNT}" | tee -a "$LOG"
  "$ROOT_CMD" -l -b -q "$root_call" 2>&1 | tee -a "$LOG"
done

if [[ ! -s "$OUTPUT" ]]; then
  echo "Drawing completed without a valid output file: $OUTPUT" >&2
  exit 1
fi

echo "All cubes completed: $OUTPUT" | tee -a "$LOG"
echo "Log: $LOG"
