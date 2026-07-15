#!/usr/bin/env bash
set -euo pipefail

# EN: Convenience launcher. The defaults point to the ADNE configuration stored in
#     this repository, while later command-line options can override every setting.
# CN: 便捷运行入口。默认读取本仓库中的 ADNE 配置；放在后面的命令行参数可覆盖默认值。

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd -- "$SCRIPT_DIR/.." && pwd)"
BINARY="$SCRIPT_DIR/bin/build_nfs_histo_from_raw_root"
SOURCE="$SCRIPT_DIR/build_nfs_histo_from_raw_root.cpp"
ADNE_DIR="$REPO_DIR/nfs-th232-analysis-adne"

if [[ ! -x "$BINARY" || "$SOURCE" -nt "$BINARY" ]]; then
  "$SCRIPT_DIR/build.sh"
fi

exec "$BINARY" \
  --config "$ADNE_DIR/Yaml_config_files/config.yaml" \
  --lut "$ADNE_DIR/Conf/Exogam2_in_Tree.cfg" \
  --energy-cal "$ADNE_DIR/CalFile/ecc.cal" \
  "$@"
