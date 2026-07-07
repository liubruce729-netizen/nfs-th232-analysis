# eff_ca

`eff_ca` 是一个 RadWare 效率标定流程包，用 Eu-152 标准源谱生成探测器效率曲线。  
`eff_ca` is a RadWare efficiency-calibration workflow package for building detector efficiency curves from Eu-152 source spectra.

默认输入是 ROOT 文件中的一维 TH1 直方图。正式效率曲线推荐用 GF3 手动逐峰拟合，而不是直接依赖 `CA/AUTOCAL`。  
The expected input is a one-dimensional ROOT TH1 histogram. For the final efficiency curve, manual GF3 peak fitting is preferred over direct `CA/AUTOCAL` output.

```text
ROOT TH1 spectrum
  -> RadWare .spe
  -> GF3 manual peak fitting
  -> gf3.sto
  -> manually prepared Eu152.sin
  -> EFFIT
  -> .aef efficiency-parameter file
```

## 内容 / Contents

- `scripts/root_hist_to_spe.py`
  - 中文：把 ROOT 里的一维 TH1 直方图写成 RadWare `.spe`。
  - English: Convert one ROOT TH1 histogram to RadWare `.spe`.
- `scripts/plot_efficiency_fit.py`
  - 中文：把 `.sin` 效率点和 `.aef` 拟合曲线画在一起，用来检查拟合。
  - English: Plot `.sin` efficiency points together with `.aef` fitted curves.
- `examples/eu152.sou`
  - 中文：Eu-152 标准源能量和相对强度表，GF3 手动拟合后整理 `.sin` 时使用。
  - English: Eu-152 source energy/intensity table used when preparing `.sin` after manual GF3 fits.
- `docs/manual_gf3_sto_to_sin_workflow.md`
  - 中文：推荐流程，GF3 手动逐峰拟合、`SA` 保存、`gf3.sto` 整理成 `.sin`、再用 EFFIT。
  - English: Recommended workflow using manual GF3 peak fits, `SA`, `gf3.sto`, manual `.sin`, and EFFIT.
- `docs/efficiency_ca_workflow.md`
  - 中文：早期 CA/AUTOCAL 流程记录和输入输出解释，保留作参考。
  - English: Earlier CA/AUTOCAL workflow notes kept as a reference.

默认不提交 `.root/.spe/.sin/.sto/.aef/.png` 等数据或结果文件。  
ROOT/SPE/SIN/STO/AEF/PNG data products are ignored by default.

## 依赖 / Requirements

`root_hist_to_spe.py`:

```text
Python 3
PyROOT
```

检查方式 / Check:

```bash
python3 -c "import ROOT; print(ROOT.__version__)"
```

`plot_efficiency_fit.py`:

```text
Python 3
numpy
matplotlib
```

检查方式 / Check:

```bash
python3 -c "import numpy, matplotlib"
```

两个脚本不依赖本仓库外的个人 Python 模块。  
Both scripts are standalone apart from the packages listed above.

## 快速使用 / Quick Start

列出 ROOT 文件中的一维直方图：  
List one-dimensional histograms in a ROOT file:

```bash
python3 scripts/root_hist_to_spe.py -i Eu152.root --list
```

把一个 ROOT TH1 转成 RadWare `.spe`：  
Convert one ROOT TH1 to RadWare `.spe`:

```bash
python3 scripts/root_hist_to_spe.py \
  -i Eu152.root \
  -n HIST_NAME_IN_ROOT \
  -o Eu152.spe \
  --spe-name Eu152
```

参数说明 / Arguments:

- `-i Eu152.root`: 输入 ROOT 文件 / input ROOT file.
- `-n HIST_NAME_IN_ROOT`: ROOT 文件中的 TH1 名字或路径 / TH1 name or path inside the ROOT file.
- `-o Eu152.spe`: 输出 RadWare spectrum / output RadWare spectrum.
- `--spe-name Eu152`: 写进 `.spe` header 的 8 字节谱名 / 8-byte internal spectrum name.

如果 ROOT 里的直方图在目录中，例如 `dir/hEu152`，`-n` 直接写完整路径。  
If the histogram is inside a ROOT directory, for example `dir/hEu152`, pass the full path to `-n`.

## GF3 手动拟合 / Manual GF3 Fits

运行 GF3：  
Run GF3:

```bash
source /home/user0/radware.sh
gf3
```

GF3 中的基本循环 / Basic loop inside GF3:

```text
SP Eu152
DS
EX
FT 1
SA
1
```

逐个峰手动选区、拟合，拟合满意后用 `SA` 保存到同一个数据组。全部峰保存后：

```text
SA -1
```

这会写出 `gf3.sto`。然后把 `gf3.sto` 中的 centroid/area 和 `eu152.sou` 中的标准能量/强度手动合并成 `Eu152.sin`。  
Fit each peak by hand, store accepted peaks with `SA`, write `gf3.sto` with `SA -1`, and manually combine `gf3.sto` with `eu152.sou` to prepare `Eu152.sin`.

`CA/AUTOCAL` 只建议作为快速检查，不建议作为正式效率曲线的默认流程。  
`CA/AUTOCAL` is useful for a quick check, but manual fits are preferred for the final efficiency curve.

详细步骤见 / Detailed steps:

```text
docs/manual_gf3_sto_to_sin_workflow.md
```

## EFFIT

运行 EFFIT：  
Run EFFIT:

```bash
effit
```

EFFIT 中输入 / Inside EFFIT:

```text
Eu152.sin
<return>
FT -1
<return>
LP
WP Eu152_fit
ST
1
```

输出 `Eu152_fit.aef`，即后续效率修正要用的参数文件。  
This writes `Eu152_fit.aef`, the efficiency-parameter file used later for correction.

## 画图检查 / Plot Check

```bash
python3 scripts/plot_efficiency_fit.py \
  --sin Eu152.sin \
  --aef Eu152_fit.aef \
  --out Eu152_efficiency_fit.png
```

也可以同时比较多条 `.aef` 曲线：  
Multiple `.aef` curves can be compared:

```bash
python3 scripts/plot_efficiency_fit.py \
  --sin Eu152.sin \
  --aef Eu152_fit.aef Eu152_fit_Gfree.aef \
  --out Eu152_efficiency_fit.png
```

## 文件解释 / File Notes

- `.sou`
  - 中文：标准源表，包含 gamma 能量、能量误差、相对强度、强度误差。
  - English: Source table with gamma energy, energy error, relative intensity, and intensity error.
- `.sto`
  - 中文：GF3 `SA -1` 写出的手动拟合结果，包含已保存峰的 centroid 和 area。
  - English: Manual GF3 fit results written by `SA -1`, containing stored centroids and areas.
- `.sin`
  - 中文：EFFIT 输入文件，手动合并 `.sto` 的实验面积和 `.sou` 的标准源信息。
  - English: EFFIT input file prepared by combining measured areas from `.sto` with source data from `.sou`.
- `.aef`
  - 中文：EFFIT 输出的效率曲线参数，顺序为 `A B C D E F G E1 E2 deff_log`。
  - English: EFFIT efficiency parameters in the order `A B C D E F G E1 E2 deff_log`.

完整中文推荐流程见：  
Recommended Chinese workflow:

```text
docs/manual_gf3_sto_to_sin_workflow.md
```
