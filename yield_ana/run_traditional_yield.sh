#!/usr/bin/env bash
set -euo pipefail

# traditional 产额处理顶层入口。
# 用法：./run_traditional_yield.sh input.ags output_dir

usage() {
  cat <<'EOF'
用法:
  ./run_traditional_yield.sh AGS_FILE OUT_DIR

参数:
  AGS_FILE  输入的 .ags 文件路径
  OUT_DIR   输出目录；不存在会自动创建

输出:
  <stem>_level.csv                         parse_ags.py 生成的能级表
  <stem>_trans.csv                         parse_ags.py 生成的跃迁表
  <stem>_yield.csv                         cal_yield_v3.py 的普通产额结果
  <stem>_yield_raw.csv                     cal_yield_v3.py 的 raw 产额结果
  second_level_transitions.csv             traditional second-level transitions
  long_lived_correction_log.csv            长寿命态修正日志
  <stem>_traditional_corrected_yields.csv  traditional 最终修正产额
EOF
}

if [[ $# -ne 2 ]]; then
  usage >&2
  exit 2
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLS_DIR="${SCRIPT_DIR}/tools"
DATA_DIR="${SCRIPT_DIR}/data"

AGS_FILE="$1"
OUT_DIR="$2"

if [[ ! -f "$AGS_FILE" ]]; then
  echo "[ERR] 输入 .ags 文件不存在: $AGS_FILE" >&2
  exit 1
fi

mkdir -p "$OUT_DIR"
OUT_DIR="$(cd "$OUT_DIR" && pwd)"
AGS_ABS="$(cd "$(dirname "$AGS_FILE")" && pwd)/$(basename "$AGS_FILE")"
STEM="$(basename "$AGS_FILE")"
STEM="${STEM%.ags}"

LEVEL_CSV="${OUT_DIR}/${STEM}_level.csv"
TRANS_CSV="${OUT_DIR}/${STEM}_trans.csv"
YIELD_CSV="${OUT_DIR}/${STEM}_yield.csv"
SECOND_TRANS_CSV="${OUT_DIR}/second_level_transitions.csv"
LONG_LOG_CSV="${OUT_DIR}/long_lived_correction_log.csv"
TRADITION_YIELD_CSV="${OUT_DIR}/${STEM}_traditional_corrected_yields.csv"

CORRECTION_CSV="${DATA_DIR}/correction_coefficients.csv"
SPIN_CORRECTION_CSV="${DATA_DIR}/spin_correction_factors_260501.csv"

for file in \
  "${TOOLS_DIR}/parse_ags.py" \
  "${TOOLS_DIR}/cal_yield_v3.py" \
  "${TOOLS_DIR}/take_trans.py" \
  "${TOOLS_DIR}/gen_yield_tradition.py" \
  "$CORRECTION_CSV" \
  "$SPIN_CORRECTION_CSV"; do
  if [[ ! -f "$file" ]]; then
    echo "[ERR] 打包文件缺失: $file" >&2
    exit 1
  fi
done

echo "[1/4] 解析 AGS -> level/trans"
python3 "${TOOLS_DIR}/parse_ags.py" "$AGS_ABS" "$LEVEL_CSV" "$TRANS_CSV"

echo "[2/4] 计算普通产额 cal_yield_v3"
python3 "${TOOLS_DIR}/cal_yield_v3.py" "$LEVEL_CSV" "$TRANS_CSV" "$CORRECTION_CSV" "$YIELD_CSV"

echo "[3/4] 提取 traditional second-level transitions"
python3 "${TOOLS_DIR}/take_trans.py" \
  --input "$TRANS_CSV" \
  --output "$SECOND_TRANS_CSV" \
  --log "$LONG_LOG_CSV"

echo "[4/4] 计算 traditional 修正产额"
python3 "${TOOLS_DIR}/gen_yield_tradition.py" \
  --transitions "$SECOND_TRANS_CSV" \
  --correction "$SPIN_CORRECTION_CSV" \
  --output "$TRADITION_YIELD_CSV"

echo "[DONE] traditional 产额处理完成"
echo "输出目录: $OUT_DIR"
echo "最终产额: $TRADITION_YIELD_CSV"
