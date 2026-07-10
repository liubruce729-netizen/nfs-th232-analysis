#!/usr/bin/env bash
set -euo pipefail

# Recursively list ADNE NFS tree ROOT files.
# 递归列出 ADNE 生成的 NFS tree ROOT 文件。
#
# The matched file basename is:
# 匹配的文件名格式为：
#   nfs_run_*_r*.root
#
# This intentionally excludes mult3_nfs_run_*.root and nfs_histoExogam2_*.root.
# 这里会刻意排除 mult3_nfs_run_*.root 和 nfs_histoExogam2_*.root。
#
# Examples / 示例:
#   ./lsy_nfs/list_nfs_run_roots.sh /data/e877_anaX/July8beamLsy/run30
#   ./lsy_nfs/list_nfs_run_roots.sh --output out/nfs_run_files.txt /data/e877_anaX/July8beamLsy/run30
#   ./lsy_nfs/list_nfs_run_roots.sh --absolute --output out/nfs_run_files.txt out

usage() {
  cat <<'EOF'
Usage:
  list_nfs_run_roots.sh [options] DIR [DIR ...]

Options:
  --output FILE        Write the file list to FILE instead of stdout.
  --absolute          Convert listed files to absolute paths with realpath.
  --follow-symlinks   Follow all symlinks below the input directories.
                       By default, symlinks passed as input directories are followed,
                       but symlinks found inside them are not.
  -h, --help          Show this help.

Notes:
  Matched basename: nfs_run_*_r*.root
  One file is written per line, sorted with sort -V.
EOF
}

output_file=""
absolute=false
find_mode="-H"
search_dirs=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --output|-o)
      [[ $# -ge 2 ]] || { echo "Missing value for --output" >&2; exit 2; }
      output_file="$2"
      shift 2
      ;;
    --absolute)
      absolute=true
      shift
      ;;
    --follow-symlinks)
      find_mode="-L"
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    --)
      shift
      while [[ $# -gt 0 ]]; do
        search_dirs+=("$1")
        shift
      done
      ;;
    -*)
      echo "Unknown option: $1" >&2
      usage >&2
      exit 2
      ;;
    *)
      search_dirs+=("$1")
      shift
      ;;
  esac
done

if [[ ${#search_dirs[@]} -eq 0 ]]; then
  search_dirs=(".")
fi

tmp_file="$(mktemp)"
trap 'rm -f "$tmp_file"' EXIT

missing=0
for dir in "${search_dirs[@]}"; do
  if [[ ! -e "$dir" ]]; then
    echo "Warning: path does not exist: $dir" >&2
    missing=1
    continue
  fi

  if $absolute; then
    find "$find_mode" "$dir" -type f -name 'nfs_run_*_r*.root' -print0 \
      | while IFS= read -r -d '' file; do
          realpath "$file"
        done >> "$tmp_file"
  else
    find "$find_mode" "$dir" -type f -name 'nfs_run_*_r*.root' -print >> "$tmp_file"
  fi
done

if [[ -n "$output_file" ]]; then
  mkdir -p "$(dirname "$output_file")"
  sort -V -u "$tmp_file" > "$output_file"
  count="$(wc -l < "$output_file" | tr -d '[:space:]')"
  echo "Found ${count} nfs_run ROOT files. Wrote: ${output_file}" >&2
else
  sort -V -u "$tmp_file"
  count="$(sort -V -u "$tmp_file" | wc -l | tr -d '[:space:]')"
  echo "Found ${count} nfs_run ROOT files." >&2
fi

if [[ "$missing" -ne 0 ]]; then
  exit 1
fi
