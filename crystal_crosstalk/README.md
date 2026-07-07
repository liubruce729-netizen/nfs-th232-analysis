# Between-Clover Cross-Talk Extraction / 不同 Clover 间串扰提取

This folder contains a small PyROOT utility for extracting between-clover cross-talk from a 2D ROOT histogram.

这个目录包含一个 PyROOT 小工具，用于从二维 ROOT 直方图中提取不同 clover 之间的串扰。

## Input Convention / 输入约定

The input 2D histogram is assumed to use the same bin order on X and Y:

输入二维图的 X/Y 轴假定使用相同的 bin 顺序：

```text
0-0, 0-1, 0-2, 0-3, 1-0, 1-1, ..., 15-3
```

Here `2-0` means clover 2, crystal 0.

其中 `2-0` 表示 clover 2 的 0 号晶体。

## What It Does / 功能

Default behavior:

默认行为：

- Keeps clovers `2..13`, crystals `0..3`.
- 保留 clover `2..13`，每个 clover 的晶体 `0..3`。
- Creates a `48 x 48` output histogram.
- 生成一个 `48 x 48` 的输出图。
- Sets same-clover pairs to zero, such as `2-0 vs 2-1` and `2-1 vs 2-0`.
- 同一个 clover 内的组合置 0，例如 `2-0 vs 2-1` 和 `2-1 vs 2-0`。
- Copies between-clover bin contents and bin errors from the input histogram.
- 不同 clover 之间的 bin content 和 bin error 从原图复制。
- Writes the new histogram back into the same ROOT file in `UPDATE` mode.
- 使用 `UPDATE` 模式把新图直接写回原 ROOT 文件。

Default output name:

默认输出图名：

```text
<input_hist_basename>_btw_clover
```

## Dependency / 依赖

The script needs PyROOT:

脚本需要 PyROOT：

```bash
python3 -c "import ROOT; print(ROOT.__version__)"
```

If `import ROOT` fails, source your ROOT environment first:

如果 `import ROOT` 失败，先 source ROOT 环境：

```bash
source /path/to/root/bin/thisroot.sh
```

## Examples / 示例

### 1. Histogram at ROOT top level / 图在 ROOT 顶层

```bash
python3 crystal_crosstalk/extract_btw_clover.py \
  -i input.root \
  -n HistoName
```

Output:

输出：

```text
input.root:HistoName_btw_clover
```

### 2. Histogram inside a ROOT directory / 图在 ROOT 文件夹里

```bash
python3 crystal_crosstalk/extract_btw_clover.py \
  -i input.root \
  -n folder/HistoName
```

Output:

输出：

```text
input.root:folder/HistoName_btw_clover
```

### 3. Custom clover range / 自定义 clover 范围

```bash
python3 crystal_crosstalk/extract_btw_clover.py \
  -i input.root \
  -n HistoName \
  --first-clover 2 \
  --last-clover 13 \
  --crystals 4
```

### 4. Custom output histogram name / 自定义输出图名

```bash
python3 crystal_crosstalk/extract_btw_clover.py \
  -i input.root \
  -n HistoName \
  -o HistoName_diff_clover
```

### 5. Show help / 查看帮助

```bash
python3 crystal_crosstalk/extract_btw_clover.py --help
```

## Download Only This Folder / 只下载这个文件夹

Use sparse checkout:

使用 sparse checkout：

```bash
git clone --filter=blob:none --sparse https://github.com/liubruce729-netizen/nfs-th232-analysis.git
cd nfs-th232-analysis
git sparse-checkout set crystal_crosstalk
```

SSH version:

SSH 版本：

```bash
git clone --filter=blob:none --sparse git@github.com:liubruce729-netizen/nfs-th232-analysis.git
cd nfs-th232-analysis
git sparse-checkout set crystal_crosstalk
```
