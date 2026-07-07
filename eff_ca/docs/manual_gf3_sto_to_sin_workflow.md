# Eu-152 效率标定：GF3 手动拟合到 EFFIT 的流程

这份流程用于标准 Eu-152 源效率标定。实际使用时，不建议直接依赖 GF3 的 `CA` 自动流程；更稳的做法是逐个峰手动选区、拟合、保存面积，最后把 `gf3.sto` 整理成 EFFIT 可读的 `.sin`。

核心流程：

```text
ROOT 一维直方图
  -> root_hist_to_spe.py 转成 Eu152.spe
  -> GF3 手动逐峰拟合
  -> SA n 暂存每个峰的 centroid 和 area
  -> SA -1 写出 gf3.sto
  -> 手动把 gf3.sto 和 Eu152.sou 合并成 Eu152.sin
  -> EFFIT 读 Eu152.sin 拟合效率曲线
  -> WP 写出 Eu152_fit.aef
```

## 1. ROOT 一维直方图转 SPE

先确认 ROOT 文件中的一维直方图名字：

```bash
python3 scripts/root_hist_to_spe.py -i Eu152.root --list
```

转换成 RadWare `.spe`：

```bash
python3 scripts/root_hist_to_spe.py \
  -i Eu152.root \
  -n HIST_NAME_IN_ROOT \
  -o Eu152.spe \
  --spe-name Eu152
```

`Eu152.spe`、`Eu152.sou` 放在同一个工作目录里。`Eu152.sou` 是标准源表，不是实验谱。

## 2. GF3 中读取谱

```bash
source /home/user0/radware.sh
gf3
```

GF3 中：

```text
SP Eu152
DS
```

`SP Eu152` 读取 `Eu152.spe`。`DS` 显示谱，先确认峰形和峰位顺序正常。

如果需要放大区域：

```text
EX
```

## 3. 逐个峰手动拟合

对每一个 Eu-152 标准峰，手动选定拟合区间和峰位。单峰区域一般使用：

```text
FT 1
```

GF3 会提示：

```text
Limits for fit?  (hit T to type, R to restart, Q to quit)
Peak positions? (hit T to type, R to restart, Q to quit)
Parameters to fix =? (one per line, rtn to end)
Max. no. of iterations = ? (rtn for 50)
```

操作建议：

- 拟合区间不要太宽，只覆盖当前峰和附近本底。
- 尽量一次只拟合一个干净峰；双峰、肩峰、污染峰先不要放进效率拟合。
- 参数不要放太多自由度。统计不够或峰形稳定时，应固定部分 shape 参数。
- 如果某个峰的 `Chisq/d.o.f.` 很大、面积不稳定或 centroid 明显偏，应重新选区或放弃这个峰。
- Eu-152 有 doublet 和弱峰，不是 `.sou` 里每个峰都一定适合使用。

拟合完成后重点看：

```text
position
width
height
area
centroid
energy
Chisq/d.o.f.
```

效率拟合最关心的是 `area` 和它的不确定度；能量最终应以 `.sou` 里的标准 gamma 能量为准。

## 4. 用 SA 保存每个峰

每拟合好一个峰，用 `SA` 保存：

```text
SA
```

GF3 会提示：

```text
N = 1-20: store centroids and areas in one of 20 positions
N = -1:   write stored values to disk file gf3.sto
...N = ?
```

输入一个数据组编号，例如：

```text
1
```

这会把当前拟合的 centroid 和 area 暂存在第 1 组。

继续下一个峰：

```text
DS
EX
FT 1
SA
1
```

重复这个过程，直到所有决定采用的 Eu-152 峰都已经保存。

最后写出 `gf3.sto`：

```text
SA
-1
```

或者直接：

```text
SA -1
```

退出：

```text
ST
Y
```

此时目录里应有：

```text
Eu152.spe
Eu152.sou
gf3.sto
```

## 5. 把 gf3.sto 和 Eu152.sou 合并成 Eu152.sin

EFFIT 需要 `.sin` 文件。现在推荐用脚本合并，避免手工复制错列：

```bash
python3 scripts/merge_sto_sou_to_sin.py \
  --sto gf3.sto \
  --sou Eu152.sou \
  --out Eu152.sin \
  --title "Manual GF3 fit, gf3.sto + Eu152.sou"
```

这个脚本完全按 RadWare `SOURCE` 程序的规则逐行合并。`.sin` 的每一行把实验拟合结果和标准源信息放在一起。

推荐 `.sin` 行格式：

```text
set_id  fitted_energy_or_centroid  fitted_energy_err  area  area_err  source_energy  source_energy_err  source_intensity  source_intensity_err
```

例如：

```text
1  122.4459  0.0026  159952  421  121.7830  0.0020  13620  160
```

各列含义：

- 第 1 列：数据组编号，通常写 `1`。
- 第 2 列：GF3 拟合得到的 centroid 或已经刻度后的实验能量。
- 第 3 列：第 2 列的不确定度。
- 第 4 列：GF3 拟合得到的峰面积 `area`。
- 第 5 列：峰面积不确定度。
- 第 6 列：`Eu152.sou` 中的标准 gamma 能量。
- 第 7 列：标准能量不确定度。
- 第 8 列：`Eu152.sou` 中的相对强度。
- 第 9 列：相对强度不确定度。

整理方法：

1. 打开 `gf3.sto`，取每个已保存峰的 centroid、centroid error、area、area error。
2. 打开 `Eu152.sou`，找到对应的标准源 gamma 能量和相对强度。
3. 每个采用的峰写成 `.sin` 的一行。
4. 不可靠的峰不要写入 `.sin`。
5. `.sin` 第一行可以写说明文字，EFFIT 会从后续数据行读取点。

如果 GF3 显示的 `energy` 还不是正确 keV，不要用这个未校准能量替代标准源能量。效率曲线的横坐标应使用 `.sou` 中的标准 gamma 能量。

## 6. EFFIT 拟合效率

运行：

```bash
effit
```

EFFIT 中：

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

说明：

- `Eu152.sin`：读入手工整理后的效率点。
- 固定参数提示处直接 `<return>`：使用默认固定参数，通常 `C=0`、`G` 固定。
- `FT -1`：重新估计初值并拟合。
- `LP`：列出参数。
- `WP Eu152_fit`：写出 `Eu152_fit.aef`。

如果释放 `G`：

```text
FR 7
FT -1
LP
WP Eu152_fit_Gfree
```

这只建议作为检查，不建议默认采用。Eu-152 单独通常不足以稳定约束低能转折。

## 7. 画图检查

```bash
python3 scripts/plot_efficiency_fit.py \
  --sin Eu152.sin \
  --aef Eu152_fit.aef \
  --out Eu152_efficiency_fit.png
```

同时比较固定 `G` 和释放 `G`：

```bash
python3 scripts/plot_efficiency_fit.py \
  --sin Eu152.sin \
  --aef Eu152_fit.aef Eu152_fit_Gfree.aef \
  --out Eu152_efficiency_fit.png
```

检查重点：

- 点是否沿着平滑曲线分布。
- 是否有明显离群点。
- `Chisq/d.o.f.` 是否过大。
- 释放 `G` 后曲线是否发散或不稳定。

## 8. 为什么不优先用 CA

`CA` 可以自动寻峰和积分，但 Eu-152 的谱里有 doublet、弱峰、污染峰和局部本底问题。自动流程容易把不适合的峰也写入 `.sin`，导致 EFFIT 曲线被拉歪。

手动拟合的好处是：

- 可以逐个峰检查拟合区间。
- 可以避免 doublet 和污染峰。
- 可以固定不稳定参数，减少自由度。
- 可以只把可信峰写入 `.sin`。

所以正式效率曲线建议走手动 `FT` + `SA` + `gf3.sto` + 手动 `.sin` 的流程；`CA` 只作为快速检查或初始参考。
