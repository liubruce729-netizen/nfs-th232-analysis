#!/usr/bin/env bash
set -u

# Batch runner for NFS per-crystal energy/time calibration.
# NFS 每个 crystal 能量/时间刻度的批处理脚本。
#
# Examples / 示例:
#   ./calibrate_nfs_crystals.sh \
#     --input-list 1nfs_run_files.txt \
#     --out-dir calibratejuly0910 \
#     --timebranch fTime \
#     --time-reference standard_time_reference.txt \
#     --jobs 8
#
#   ./calibrate_nfs_crystals.sh \
#     --out-dir out/calibration_run23 \
#     --jobs 4 \
#     out/nfs_run_23_r0.root out/nfs_run_23_r1.root
#
# Outputs / 输出:
#   <out-dir>/expanded_root_inputs.txt  : expanded nfs_run ROOT file list / 展开后的 ROOT 文件列表
#   <out-dir>/calibration_outputs.tsv   : one line per input ROOT file / 每个输入文件一行
#   <out-dir>/calibration_summary.tsv   : one line per input+crystal / 每个输入文件、每个 crystal 一行
#   <out-dir>/logs/*.log                : per-run ROOT logs / 每个 run 的 ROOT 日志

usage() {
  cat <<'EOF'
Usage:
  calibrate_nfs_crystals.sh [options] ROOT_FILE [ROOT_FILE ...]
  calibrate_nfs_crystals.sh [options] --input-list FILE

Options:
  --input-list FILE          Text file with one input ROOT file or directory per line.
                             Empty lines and lines beginning with # are ignored.
                             Directory entries are searched recursively for
                             nfs_run_*_r*.root, then naturally sorted and deduplicated.
                             Direct ROOT files are kept for backward compatibility.
  --out-dir DIR             Output directory. Default: nfs_crystal_calibration_batch
  --manifest FILE           Output manifest TSV. Default: <out-dir>/calibration_outputs.tsv
  --summary FILE            Merged calibration TSV. Default: <out-dir>/calibration_summary.tsv
  --jobs N                  Number of ROOT calibration jobs to run in parallel. Default: 1
  --macro FILE              ROOT macro path. Default: calibrate_nfs_crystal_energy_time.C next to this script,
                             or ../nfs-th232-analysis/nfs-th232-analysis-adne/lsy_nfs/calibrate_nfs_crystal_energy_time.C
  --max-entries N           Entries per calibration task. Default: -1
  --time-branch NAME        Time branch leaf name. Default: fTime
  --timebranch NAME         Alias for --time-branch.
  --time-reference FILE     Per-crystal reference table made by
                             fit_nfs_standard_time_reference.C.
  --energy-fit-half-width V Energy peak fit half width. Default: 25
  --time-search-low V       Lower bound for neutron peak search in ns. Default: 400
  --time-search-high V      Upper bound for time peak search in ns. Default: 800
  --root CMD                ROOT executable. Default: root
  -h, --help                Show this help.

Example:
  ./calibrate_nfs_crystals.sh --input-list 1nfs_run_files.txt --out-dir calibratejuly0910 --timebranch fTime --time-reference standard_time_reference.txt --jobs 8
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

expand_input_sources() {
  local unsorted_file="$1"
  shift
  local item found

  : > "$unsorted_file"
  for item in "$@"; do
    if [[ -d "$item" ]]; then
      found=0
      while IFS= read -r file; do
        printf '%s\n' "$file" >> "$unsorted_file"
        found=1
      done < <(find -H "$item" -type f -name 'nfs_run_*_r*.root' | sort -V)
      if [[ "$found" -eq 0 ]]; then
        echo "Warning: no nfs_run_*_r*.root files found under directory: $item" >&2
      fi
    elif [[ -f "$item" ]]; then
      printf '%s\n' "$item" >> "$unsorted_file"
    else
      echo "Warning: input source is neither file nor directory; keeping literal value: $item" >&2
      printf '%s\n' "$item" >> "$unsorted_file"
    fi
  done
}

find_default_macro() {
  local candidate
  for candidate in \
    "${script_dir}/calibrate_nfs_crystal_energy_time.C" \
    "${script_dir}/../nfs-th232-analysis/nfs-th232-analysis-adne/lsy_nfs/calibrate_nfs_crystal_energy_time.C" \
    "${script_dir}/nfs-th232-analysis/nfs-th232-analysis-adne/lsy_nfs/calibrate_nfs_crystal_energy_time.C"; do
    if [[ -f "$candidate" ]]; then
      printf '%s' "$candidate"
      return 0
    fi
  done
  printf '%s' "${script_dir}/calibrate_nfs_crystal_energy_time.C"
}

run_one_calibration() {
  local idx="$1"
  local input="$2"
  local prefix="$3"
  local output_root="$4"
  local output_txt="$5"
  local log_file="$6"
  local status_file="$7"

  local macro_arg input_arg prefix_arg time_branch_arg reference_arg root_call status
  macro_arg="$(root_escape "$macro")"
  input_arg="$(root_escape "$input")"
  prefix_arg="$(root_escape "$prefix")"
  time_branch_arg="$(root_escape "$time_branch")"
  reference_arg="$(root_escape "$time_reference")"
  root_call="${macro_arg}(\"${input_arg}\",\"${prefix_arg}\",${max_entries},\"${time_branch_arg}\",${energy_fit_half_width},${time_search_high_ns},\"${reference_arg}\",${time_search_low_ns})"

  echo "[$idx/$total] Calibrating: $input"
  echo "  Output prefix: $prefix"
  echo "  Log: $log_file"

  if "$root_cmd" -l -b -q "$root_call" > "$log_file" 2>&1; then
    status="ok"
    if [[ ! -s "$output_txt" || ! -s "$output_root" ]]; then
      status="missing_output"
    fi
  else
    status="failed"
  fi

  printf '%s\n' "$status" > "$status_file"
  echo "  Status: $status"
}

write_summary_header_from_txt() {
  local txt="$1"
  local header
  header="$(awk '/^# clover[[:space:]]/{sub(/^# /, ""); gsub(/[[:space:]]+/, "\t"); print; exit}' "$txt")"
  if [[ -z "$header" ]]; then
    return 1
  fi
  printf 'run_index\tinput\trun_file\toutput_prefix\tstatus\t%s\n' "$header" > "$summary"
}

append_summary_from_txt() {
  local number_tag="$1"
  local input="$2"
  local prefix="$3"
  local status="$4"
  local txt="$5"
  local run_file
  run_file="$(basename "$input")"

  awk -v idx="$number_tag" -v input="$input" -v run_file="$run_file" -v prefix="$prefix" -v status="$status" '
    BEGIN { OFS="\t" }
    /^[[:space:]]*$/ { next }
    /^#/ { next }
    {
      printf "%s\t%s\t%s\t%s\t%s", idx, input, run_file, prefix, status
      for (i = 1; i <= NF; ++i) printf "\t%s", $i
      printf "\n"
    }
  ' "$txt" >> "$summary"
}

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
input_list=""
out_dir="nfs_crystal_calibration_batch"
manifest=""
summary=""
macro="$(find_default_macro)"
max_entries="-1"
time_branch="fTime"
time_reference=""
energy_fit_half_width="25"
time_search_low_ns="400"
time_search_high_ns="800"
root_cmd="root"
jobs="1"
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
    --summary)
      [[ $# -ge 2 ]] || { echo "Missing value for --summary" >&2; exit 2; }
      summary="$2"
      shift 2
      ;;
    --jobs|-j)
      [[ $# -ge 2 ]] || { echo "Missing value for --jobs" >&2; exit 2; }
      jobs="$2"
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
    --time-branch|--timebranch)
      [[ $# -ge 2 ]] || { echo "Missing value for $1" >&2; exit 2; }
      time_branch="$2"
      shift 2
      ;;
    --time-reference)
      [[ $# -ge 2 ]] || { echo "Missing value for --time-reference" >&2; exit 2; }
      time_reference="$2"
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
    --time-search-low|--time-search-low-ns)
      [[ $# -ge 2 ]] || { echo "Missing value for --time-search-low" >&2; exit 2; }
      time_search_low_ns="$2"
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

if ! [[ "$jobs" =~ ^[0-9]+$ ]] || [[ "$jobs" -lt 1 ]]; then
  echo "--jobs must be a positive integer: $jobs" >&2
  exit 2
fi

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
  echo "Use --macro /path/to/calibrate_nfs_crystal_energy_time.C" >&2
  exit 1
fi

if [[ -n "$time_reference" && ! -f "$time_reference" ]]; then
  echo "Time-reference table not found: $time_reference" >&2
  exit 1
fi

mkdir -p "$out_dir/logs" "$out_dir/status"
if [[ -z "$manifest" ]]; then
  manifest="${out_dir}/calibration_outputs.tsv"
fi
if [[ -z "$summary" ]]; then
  summary="${out_dir}/calibration_summary.tsv"
fi
expanded_inputs="${out_dir}/expanded_root_inputs.txt"
expanded_inputs_unsorted="${out_dir}/expanded_root_inputs.unsorted.txt"
source_count=${#inputs[@]}
mkdir -p "$(dirname "$manifest")" "$(dirname "$summary")"

expand_input_sources "$expanded_inputs_unsorted" "${inputs[@]}"
sort -V -u "$expanded_inputs_unsorted" > "$expanded_inputs"
mapfile -t inputs < "$expanded_inputs"

if [[ ${#inputs[@]} -eq 0 ]]; then
  echo "No nfs_run ROOT files were found after expanding input sources." >&2
  echo "Expanded list: $expanded_inputs" >&2
  exit 2
fi

printf 'index\tinput\toutput_prefix\toutput_root\toutput_txt\toutput_ecc\tlog\tstatus\n' > "$manifest"
: > "$summary"

total=${#inputs[@]}
failed=0
summary_header_written=0
running=0

number_tags=()
input_values=()
prefix_values=()
output_root_values=()
output_txt_values=()
output_ecc_values=()
log_values=()
status_file_values=()

echo "Input sources: $source_count"
echo "Expanded ROOT inputs: $total"
echo "Expanded input list: $expanded_inputs"
echo "Parallel jobs: $jobs"
echo "Macro: $macro"
echo "Time reference: ${time_reference:-legacy 442 ns}"
echo "Time search: ${time_search_low_ns}-${time_search_high_ns} ns"
echo "Output dir: $out_dir"
echo "Manifest: $manifest"
echo "Summary: $summary"

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
  status_file="${out_dir}/status/${number_tag}_${tag}.status"

  number_tags[$idx]="$number_tag"
  input_values[$idx]="$input"
  prefix_values[$idx]="$prefix"
  output_root_values[$idx]="$output_root"
  output_txt_values[$idx]="$output_txt"
  output_ecc_values[$idx]="$output_ecc"
  log_values[$idx]="$log_file"
  status_file_values[$idx]="$status_file"

  run_one_calibration "$number" "$input" "$prefix" "$output_root" "$output_txt" "$log_file" "$status_file" &
  running=$((running + 1))

  if [[ "$running" -ge "$jobs" ]]; then
    wait -n
    running=$((running - 1))
  fi
done

while [[ "$running" -gt 0 ]]; do
  wait -n
  running=$((running - 1))
done

for ((idx = 0; idx < total; ++idx)); do
  number_tag="${number_tags[$idx]}"
  input="${input_values[$idx]}"
  prefix="${prefix_values[$idx]}"
  output_root="${output_root_values[$idx]}"
  output_txt="${output_txt_values[$idx]}"
  output_ecc="${output_ecc_values[$idx]}"
  log_file="${log_values[$idx]}"
  status_file="${status_file_values[$idx]}"

  if [[ -s "$status_file" ]]; then
    status="$(trim_line "$(<"$status_file")")"
  else
    status="failed"
  fi

  if [[ "$status" != "ok" ]]; then
    failed=$((failed + 1))
  fi

  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
    "$number_tag" "$input" "$prefix" "$output_root" "$output_txt" "$output_ecc" "$log_file" "$status" >> "$manifest"

  if [[ -s "$output_txt" ]]; then
    if [[ "$summary_header_written" -eq 0 ]]; then
      if write_summary_header_from_txt "$output_txt"; then
        summary_header_written=1
      fi
    fi
    if [[ "$summary_header_written" -eq 1 ]]; then
      append_summary_from_txt "$number_tag" "$input" "$prefix" "$status" "$output_txt"
    fi
  fi
done

if [[ "$summary_header_written" -eq 0 ]]; then
  printf 'run_index\tinput\trun_file\toutput_prefix\tstatus\tnote\n' > "$summary"
  printf 'NA\tNA\tNA\tNA\tfailed\tNo readable calibration txt output was produced.\n' >> "$summary"
fi

echo "Manifest: $manifest"
echo "Summary: $summary"
if [[ "$failed" -gt 0 ]]; then
  echo "Calibration finished with ${failed} failed task(s)." >&2
  exit 1
fi

echo "Calibration finished successfully."
