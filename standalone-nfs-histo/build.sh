#!/usr/bin/env bash
set -euo pipefail

# EN: Build the ROOT-only processor with optimization. No ADNE/GRU/MFMlib library is linked.
# CN: 以优化模式编译只依赖 ROOT 的处理程序，不链接 ADNE、GRU 或 MFMlib。

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
CXX="${CXX:-g++}"
ROOT_CONFIG="${ROOT_CONFIG:-root-config}"
SOURCE="$SCRIPT_DIR/build_nfs_histo_from_raw_root.cpp"
OUTPUT="$SCRIPT_DIR/bin/build_nfs_histo_from_raw_root"

if ! command -v "$ROOT_CONFIG" >/dev/null 2>&1; then
  echo "root-config not found. Source the ROOT environment first." >&2
  exit 1
fi

mkdir -p "$(dirname -- "$OUTPUT")"
read -r -a ROOT_CFLAGS <<< "$("$ROOT_CONFIG" --cflags)"
read -r -a ROOT_LIBS <<< "$("$ROOT_CONFIG" --libs)"

"$CXX" -O3 -DNDEBUG -Wall -Wextra -Wpedantic \
  "${ROOT_CFLAGS[@]}" "$SOURCE" -o "$OUTPUT" "${ROOT_LIBS[@]}"

echo "Built / 编译完成: $OUTPUT"
