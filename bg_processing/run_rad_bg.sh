#!/usr/bin/env bash
set -euo pipefail

# ===== 新环境通常只需要修改这里 =====
# TOOLSDIR 默认使用本脚本旁边的 tools/；如果你把 tools 放到别处，可改这一行或运行时覆盖。
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLSDIR="${TOOLSDIR:-${SCRIPT_DIR}/tools}"

# ===== 可调参数 =====
T=5
skew_T=5
W=3
skew_W=3

# ===== 输入 ROOT 和 histogram 名称 =====
# 新环境可直接改下面两个默认值，也可以运行时覆盖：
#   INPUT_ROOT=/path/to/input.root INPUT_HIST=EE_T bash run_rad_bg.sh all
INPUT_ROOT="${INPUT_ROOT:-big_win_th.root}"
INPUT_HIST="${INPUT_HIST:-EE_T}"

# ===== 由 INPUT_HIST 派生的 histogram 名称 =====
# subtract2D 使用 suffix _sig1，因此 rad 信号名是 ${INPUT_HIST}_sig1。
# skew_edge.py 的 -o 是输出前缀，会写出 ${DESKEW_PREFIX}_skew 和 ${DESKEW_PREFIX}_no_skew。
RAD_SIG_HIST="${INPUT_HIST}_sig1"
RAD_BG_HIST="${INPUT_HIST}_bg1"
DESKEW_PREFIX="${RAD_SIG_HIST}_noskew"
DESKEW_BG_HIST="${DESKEW_PREFIX}_skew"
DESKEW_SIG_HIST="${DESKEW_PREFIX}_no_skew"

# ===== 派生文件名（都写入当前工作目录）=====
RAD_RE_ROOT="rad_re_mut${T}_${W}.root"
RAD_PROJ_SPE="S_proj.spe"
RAD_BG_SPE="S_proj_bg.spe"
RAD_BG_M4B="rad_bg_mut${T}_${W}.m4b"
FINAL_MAT_WIN80_RAD_M4B="final_matrix_win80_rad_mut${T}_${W}.m4b"

DELTA_ROOT="delta.root"
DELTA_HIST="Delta"

FINAL_MATRIX_ROOT="final_matrix_win80_rad_deskew.root"
FINAL_MATRIX_M4B="final_matrix_win80_rad_deskew.m4b"
FINAL_MATRIX_HIST="S"

START_FROM="${1:-all}"

check_inputs() {
  [[ -d "${TOOLSDIR}" ]] || { echo "ERROR: TOOLSDIR 不存在: ${TOOLSDIR}"; exit 10; }
  [[ -f "${INPUT_ROOT}" ]] || { echo "ERROR: INPUT_ROOT 不存在: ${INPUT_ROOT}"; exit 11; }
}

check_inputs

run_rad_origin() {
  echo "== [rad_origin：normal-bin 投影、本底、rad 矩阵转换] =="

  # 1) 得到 normal-bin 投影 S_proj，不包含 ROOT underflow/overflow。
  python3 "${TOOLSDIR}/root_projection.py" project \
    -i "${INPUT_ROOT}" -n "${INPUT_HIST}" -o "${RAD_RE_ROOT}" -p S_proj

  # 2) 得到一维本底 S_proj_bg。
  python3 "${TOOLSDIR}/root_projection.py" background \
    -i "${RAD_RE_ROOT}" -n S_proj -b S_proj_bg -t "${T}" -w "${W}"

  # 3) 直接保存投影和一维本底为 .spe，避免再用 m4b+slice 取巧投影。
  run_save_rad_projections_spe

  # 4) 构造二维本底和 rad 信号矩阵。
  #    输出名由 suffix 决定：${RAD_BG_HIST} 和 ${RAD_SIG_HIST}。
  python3 "${TOOLSDIR}/root_projection.py" subtract2D \
    -i "${RAD_RE_ROOT}" -n "${INPUT_HIST}" -p S_proj -b S_proj_bg -o "${RAD_RE_ROOT}" \
    --suffix_bg _bg1 --suffix_sig _sig1

  # 5) 仍保存 m4b，兼容需要 m4b 的旧流程。
  run_save_rad_m4b
}

run_save_rad_projections_spe() {
  echo "== [直接保存 S_proj / S_proj_bg 为 spe] =="

  python3 "${TOOLSDIR}/root_projection.py" spe \
    -i "${RAD_RE_ROOT}" -n S_proj -o "${RAD_PROJ_SPE}"
  [[ -f "${RAD_PROJ_SPE}" ]] || { echo "ERROR: 未生成 ${RAD_PROJ_SPE}"; exit 5; }

  python3 "${TOOLSDIR}/root_projection.py" spe \
    -i "${RAD_RE_ROOT}" -n S_proj_bg -o "${RAD_BG_SPE}"
  [[ -f "${RAD_BG_SPE}" ]] || { echo "ERROR: 未生成 ${RAD_BG_SPE}"; exit 6; }
}

run_save_rad_m4b() {
  echo "== [保存 rad 背景和原矩阵为 m4b] =="

  # S_proj_bg2D 是旧 slice 投影流程的辅助矩阵，不是物理对称矩阵。
  root -l -q -b "${TOOLSDIR}/expand1Dto2D.C(\"${RAD_RE_ROOT}\",\"S_proj_bg\",\"S_proj_bg2D\")" | tee expand1D2D.log

  # save2m4b.C 现在显式写 signed int32_t。
  root -l -q -b "${TOOLSDIR}/save2m4b.C(\"${RAD_RE_ROOT}\",\"S_proj_bg2D\",\"${RAD_BG_M4B}\")" | tee save_radbg.log
  [[ -f "${RAD_BG_M4B}" ]] || { echo "ERROR: 未生成 ${RAD_BG_M4B}"; exit 2; }

  root -l -q -b "${TOOLSDIR}/save2m4b.C(\"${INPUT_ROOT}\",\"${INPUT_HIST}\",\"${FINAL_MAT_WIN80_RAD_M4B}\")" | tee save_win80rad.log
  [[ -f "${FINAL_MAT_WIN80_RAD_M4B}" ]] || { echo "ERROR: 未生成 ${FINAL_MAT_WIN80_RAD_M4B}"; exit 3; }
}

run_deskew() {
  echo "== [对 rad 信号结果去 skew] =="

  python3 "${TOOLSDIR}/skew_edge.py" \
    -i "${RAD_RE_ROOT}" -n "${RAD_SIG_HIST}" -o "${DESKEW_PREFIX}" \
    -t "${skew_T}" -s "${skew_W}"
}

run_delta() {
  echo "== [基于 rad 去 skew 结果计算 delta，并回加得到最终矩阵] =="

  # skew_edge.py -o ${DESKEW_PREFIX} 实际写出 ${DESKEW_BG_HIST} 和 ${DESKEW_SIG_HIST}。
  # Delta = 去 skew 后的 rad 信号 - 原 rad 信号。
  python3 "${TOOLSDIR}/sub_fig.py" \
    -a "${RAD_RE_ROOT}" "${DESKEW_SIG_HIST}" \
    -b "${RAD_RE_ROOT}" "${RAD_SIG_HIST}" \
    -o "${DELTA_ROOT}" "${DELTA_HIST}" \
    -s -1

  # Final = 原始矩阵 + Delta。
  python3 "${TOOLSDIR}/sub_fig.py" \
    -a "${RAD_RE_ROOT}" "${INPUT_HIST}" \
    -b "${DELTA_ROOT}" "${DELTA_HIST}" \
    -o "${FINAL_MATRIX_ROOT}" "${FINAL_MATRIX_HIST}" \
    -s 1
}

run_save_final() {
  echo "== [最终矩阵转为 m4b] =="

  root -l -q -b "${TOOLSDIR}/save2m4b.C(\"${FINAL_MATRIX_ROOT}\",\"${FINAL_MATRIX_HIST}\",\"${FINAL_MATRIX_M4B}\")" | tee save_final.log
  [[ -f "${FINAL_MATRIX_M4B}" ]] || { echo "ERROR: 未生成 ${FINAL_MATRIX_M4B}"; exit 4; }
}

print_summary() {
  echo "== 完成（输出均在当前工作目录） =="
  echo "  INPUT_ROOT=${INPUT_ROOT}"
  echo "  INPUT_HIST=${INPUT_HIST}"
  echo "  rad signal hist: ${RAD_SIG_HIST}"
  echo "  deskew signal hist: ${DESKEW_SIG_HIST}"
  echo "  ${RAD_RE_ROOT}"
  echo "  ${RAD_PROJ_SPE}"
  echo "  ${RAD_BG_SPE}"
  echo "  ${RAD_BG_M4B}"
  echo "  ${FINAL_MAT_WIN80_RAD_M4B}"
  echo "  ${DELTA_ROOT}"
  echo "  ${FINAL_MATRIX_ROOT}"
  echo "  ${FINAL_MATRIX_M4B}"
}

case "${START_FROM}" in
  all)
    run_rad_origin
    run_deskew
    run_delta
    run_save_final
    ;;
  rad_origin)
    run_rad_origin
    run_deskew
    run_delta
    run_save_final
    ;;
  deskew)
    run_deskew
    run_delta
    run_save_final
    ;;
  delta)
    run_delta
    run_save_final
    ;;
  save_final)
    run_save_final
    ;;
  convert_only)
    # 已经有 rad_re_mut${T}_${W}.root 和 final root 时，只重做 spe/m4b 转换。
    run_save_rad_projections_spe
    run_save_rad_m4b
    run_save_final
    ;;
  *)
    echo "用法: $0 [all|rad_origin|deskew|delta|save_final|convert_only]"
    exit 1
    ;;
esac

print_summary
