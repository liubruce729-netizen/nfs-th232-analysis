#!/usr/bin/env bash
set -u

# Batch runner for NFS per-crystal energy/time calibration.
# NFS 每个 crystal 能量/时间刻度的批处理脚本。
#
# Examples / 示例:
#   ./lsy_nfs/batch_calibrate_nfs_crystals.sh \
#     --out-dir out/calibration_run23 \
#     out/nfs_run_23_r0.root out/nfs_run_23_r1.root
#
#   ./lsy_nfs/batch_calibrate_nfs_crystals.sh \
#     --input-list out/nfs_run_files.txt \
#     --out-dir out/calibration_all \
#     --max-entries -1 \
#     --time-branch fTime
#
# The manifest TSV records all output files for downstream reading.
# manifest TSV 会记录每个输入对应的输出文件，方便后续读取。

usage() {
  cat <<'EOF'
Usage:
  batch_calibrate_nfs_crystals.sh [options] ROOT_FILE [ROOT_FILE ...]
  batch_calibrate_nfs_crystals.sh [options] --input-list FILE

Options:
  --input-list FILE          Text file with one input ROOT file/spec per line.
                             Empty lines and lines beginning with # are ignored.
                             Each line may also be a comma list or @filelist
                             accepted by calibrate_nfs_crystal_energy_time.C.
  --out-dir DIR             Output directory. Default: nfs_crystal_calibration_batch
  --manifest FILE           Output manifest TSV. Default: <out-dir>/calibration_outputs.tsv
  --macro FILE              ROOT macro path. Default: this_dir/calibrate_nfs_crystal_energy_time.C
  --max-entries N           Entries per calibration task. Default: -1
  --time-branch NAME        Time branch leaf name. Default: fTime
  --energy-fit-half-width V Energy peak fit half width. Default: 25
  --time-search-high V      Upper bound for time peak search in ns. Default: 800
  --root CMD                ROOT executable. Default: root
  -h, --help                Show this help.
EOF
}

trim_line() {
  local s="$1"
  s="${s#"${s%%[![:space:]]*}"}"
  s="${s%"${s##*[![:space:]]}"}"
  printf '%s' "$s"
}

root_escape() {
  local s="$1"
  s="${s//\\/\\\\}"
  s="${s//\"/\\\"}"
  printf '%s' "$s"
}

sanitize_tag() {
  local s="$1"
  s="${s%.root}"
  printf '%s' "$s" | tr -c 'A-Za-z0-9._-' '_'
}

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
input_list=""
out_dir="nfs_crystal_calibration_batch"
manifest=""
macro="${script_dir}/calibrate_nfs_crystal_energy_time.C"
max_entries="-1"
time_branch="fTime"
energy_fit_half_width="25"
time_search_high_ns="800"
root_cmd="root"
inputs=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --input-list)
      [[ $# -ge 2 ]] || { echo "Missing value for --input-list" >&2; exit 2; }
      input_list="$2"
      shift 2
      ;;
    --out-dir)
      [[ $# -ge 2 ]] || { echo "Missing value for --out-dir" >&2; exit 2; }
      out_dir="$2"
      shift 2
      ;;
    --manifest)
      [[ $# -ge 2 ]] || { echo "Missing value for --manifest" >&2; exit 2; }
      manifest="$2"
      shift 2
      ;;
    --macro)
      [[ $# -ge 2 ]] || { echo "Missing value for --macro" >&2; exit 2; }
      macro="$2"
      shift 2
      ;;
    --max-entries)
      [[ $# -ge 2 ]] || { echo "Missing value for --max-entries" >&2; exit 2; }
      max_entries="$2"
      shift 2
      ;;
    --time-branch)
      [[ $# -ge 2 ]] || { echo "Missing value for --time-branch" >&2; exit 2; }
      time_branch="$2"
      shift 2
      ;;
    --energy-fit-half-width)
      [[ $# -ge 2 ]] || { echo "Missing value for --energy-fit-half-width" >&2; exit 2; }
      energy_fit_half_width="$2"
      shift 2
      ;;
    --time-search-high|--time-search-high-ns)
      [[ $# -ge 2 ]] || { echo "Missing value for --time-search-high" >&2; exit 2; }
      time_search_high_ns="$2"
      shift 2
      ;;
    --root)
      [[ $# -ge 2 ]] || { echo "Missing value for --root" >&2; exit 2; }
      root_cmd="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    --)
      shift
      while [[ $# -gt 0 ]]; do
        inputs+=("$1")
        shift
      done
      ;;
    -*)
      echo "Unknown option: $1" >&2
      usage >&2
      exit 2
      ;;
    *)
      inputs+=("$1")
      shift
      ;;
  esac
done

if [[ -n "$input_list" ]]; then
  if [[ ! -f "$input_list" ]]; then
    echo "Input list not found: $input_list" >&2
    exit 1
  fi
  while IFS= read -r line || [[ -n "$line" ]]; do
    item="$(trim_line "$line")"
    [[ -z "$item" || "${item:0:1}" == "#" ]] && continue
    inputs+=("$item")
  done < "$input_list"
fi

if [[ ${#inputs[@]} -eq 0 ]]; then
  echo "No calibration inputs were provided." >&2
  usage >&2
  exit 2
fi

if [[ ! -f "$macro" ]]; then
  echo "Calibration macro not found: $macro" >&2
  exit 1
fi

mkdir -p "$out_dir/logs"
if [[ -z "$manifest" ]]; then
  manifest="${out_dir}/calibration_outputs.tsv"
fi
mkdir -p "$(dirname "$manifest")"

printf 'index\tinput\toutput_prefix\toutput_root\toutput_txt\toutput_ecc\tlog\tstatus\n' > "$manifest"

total=${#inputs[@]}
failed=0

for ((idx = 0; idx < total; ++idx)); do
  input="${inputs[$idx]}"
  number=$((idx + 1))
  printf -v number_tag '%03d' "$number"

  base="$(basename "$input")"
  tag="$(sanitize_tag "$base")"
  prefix="${out_dir}/${number_tag}_${tag}"
  output_root="${prefix}.root"
  output_txt="${prefix}.txt"
  output_ecc="${prefix}_ecc_cal_candidate.txt"
  log_file="${out_dir}/logs/${number_tag}_${tag}.log"

  macro_arg="$(root_escape "$macro")"
  input_arg="$(root_escape "$input")"
  prefix_arg="$(root_escape "$prefix")"
  time_branch_arg="$(root_escape "$time_branch")"
  root_call="${macro_arg}(\"${input_arg}\",\"${prefix_arg}\",${max_entries},\"${time_branch_arg}\",${energy_fit_half_width},${time_search_high_ns})"

  echo "[$number/$total] Calibrating: $input"
  echo "  Output prefix: $prefix"
  echo "  Log: $log_file"

  if "$root_cmd" -l -b -q "$root_call" > "$log_file" 2>&1; then
    status="ok"
    if [[ ! -s "$output_txt" || ! -s "$output_root" ]]; then
      status="missing_output"
      failed=$((failed + 1))
    fi
  else
    status="failed"
    failed=$((failed + 1))
  fi

  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
    "$number_tag" "$input" "$prefix" "$output_root" "$output_txt" "$output_ecc" "$log_file" "$status" >> "$manifest"

  echo "  Status: $status"
done

echo "Manifest: $manifest"
if [[ "$failed" -gt 0 ]]; then
  echo "Calibration finished with ${failed} failed task(s)." >&2
  exit 1
fi

echo "Calibration finished successfully."
