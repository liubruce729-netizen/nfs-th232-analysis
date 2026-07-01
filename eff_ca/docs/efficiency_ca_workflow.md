# Eu-152 探测器效率曲线拟合完整流程

本目录用于从标准 `Eu152.spe` 光谱得到 RadWare/EFFIT 的效率参数文件。完整链条是：

```text
ROOT 文件中的 Eu-152 一维能谱
  -> 转成 RadWare .spe
  -> 准备/检查 Eu-152 .sou 标准源表
  -> GF3 的 CA/AUTOCAL 自动寻峰和积分，生成 .sin
  -> EFFIT 读取 .sin 拟合效率曲线，生成 .aef
  -> 用 .aef 对后续实验峰面积做效率修正
```

## 1. 文件说明

- `Eu152.spe`: 标准 Eu-152 源光谱，RadWare GF3 直接读取的 spectrum 文件。
- `eu152.sou`: Eu-152 标准源能量和相对强度表，供 GF3 的 `CA`/AUTOCAL 匹配峰。
- `Eu152.sin`: GF3 `CA` 自动寻峰、积分后生成的 Source INput 文件，是 EFFIT 的输入。
- `Eu152.aca`: GF3 `CA` 给出的自动能量刻度文件。
- `Eu152_fit.aef`: EFFIT 默认拟合结果，固定 `C=0`、`G=15`。
- `Eu152_fit_Gfree.aef`: 额外测试的释放 `G` 的拟合结果，只用于诊断，不建议优先使用。
- `plot_efficiency_fit.py`: 画出 `.sin` 效率点和 `.aef` 曲线，检查拟合形状。
- `Eu152_efficiency_fit.png`: 当前检查图。

## 2. 从 ROOT 到 SPE

RadWare 的 GF3 读取 `.spe`，所以如果一开始拿到的是 ROOT 文件，需要先把 ROOT 里的 Eu-152 一维能谱导出成 RadWare `.spe`。

本地已经看到几个可用入口：

- `/home/user0/work/IJCLAB/test_code/tools/root_projection.py`
- `/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/Utils/root2spe.C`
- `/home/user0/work/IJCLAB/cf252/simple/WriteRWSpec.cxx`

推荐优先用 `root_projection.py`，因为它已经有明确的 `spe` 子命令，且按 ROOT normal bins 写出 RadWare `.spe`：

```bash
python3 /home/user0/work/IJCLAB/test_code/tools/root_projection.py spe \
  -i input.root \
  -n HIST_NAME_IN_ROOT \
  -o Eu152.spe \
  --spe_name Eu152
```

需要替换：

- `input.root`: 你的 Eu-152 源 ROOT 文件。
- `HIST_NAME_IN_ROOT`: ROOT 文件里一维能谱的名字。
- `Eu152.spe`: 输出给 GF3 使用的 RadWare spectrum 文件。

如果 ROOT 文件里是二维矩阵，需要先投影出一维谱：

```bash
python3 /home/user0/work/IJCLAB/test_code/tools/root_projection.py project \
  -i input.root \
  -n HIST2D_NAME_IN_ROOT \
  -o projected.root \
  -p Eu152_proj \
  -a X

python3 /home/user0/work/IJCLAB/test_code/tools/root_projection.py spe \
  -i projected.root \
  -n Eu152_proj \
  -o Eu152.spe \
  --spe_name Eu152
```

也可以用老的通用程序：

```bash
cd /home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/Utils
g++ root2spe.C -o root2spe $(root-config --libs) $(root-config --cflags)
./root2spe
```

`root2spe.C` 是 1D histogram 转 `.spe` 的转换器；如果 ROOT 文件里有多个 TH1，它也支持 `-all` 模式。

转换后建议用 GF3 打开检查：

```bash
source /home/user0/radware.sh
cd /home/user0/work/IJCLAB/rd_work/eff_fit
gf3
```

GF3 中输入：

```text
SP Eu152
DS
```

确认能谱能正常显示，峰位顺序和分辨率大致合理。如果通道轴还不是 keV，后续 `CA` 会根据 `.sou` 做自动能量匹配和 `.aca` 能量刻度，但输入谱本身的峰序必须正确。

## 3. sou 是什么文件

`.sou` 是 RadWare 的 source data file，即标准放射源的能量和相对强度表。GF3 的 `CA` 用它来知道“这个标准源里应该有哪些 gamma 峰”，然后在谱里自动找峰、积分、匹配能量。

本目录的 `eu152.sou` 每行格式大致是：

```text
Energy_keV  dEnergy_keV  Relative_intensity  dIntensity
```

例如：

```text
121.7830  0.0020  13620  160
244.6920  0.0020   3590   60
344.2760  0.0040  12750   90
```

含义：

- 第 1 列：标准源 gamma 能量，单位 keV。
- 第 2 列：能量不确定度。
- 第 3 列：相对强度，不一定归一到 100，RadWare 只需要相对标度一致。
- 第 4 列：相对强度不确定度。

`.sou` 不是实验数据，而是标准源核数据表。实验峰面积来自 `.spe`；标准强度来自 `.sou`；二者在 GF3 `CA` 后合并成 `.sin`，再给 EFFIT 拟合效率。

注意：Eu-152 有 doublet 和弱峰，RadWare 文档也提醒复杂源的 `.sou` 通常需要编辑，移除不适合自动积分的峰。`.sou` 里的峰不一定越多越好，坏峰会拉坏效率曲线。

## 4. RadWare 原文依据

本机文档位置：

- `/home/user0/work/IJCLAB/rd_install/rw_current/doc/gf3.html`
- `/home/user0/work/IJCLAB/rd_install/rw_current/doc/gfonline.hlp`
- `/home/user0/work/IJCLAB/rd_install/rw_current/doc/autocal.doc`

文档说明：GF3 的 `CA` 使用 `.sou` 文件定义源中应出现的峰，自动寻峰、定义本底、积分得到 centroid 和 area，并保存为 `.sin`；`.sin` 可作为 `effit` 和 `encal` 的输入。

EFFIT 的输入来自 `.sin` 文件；得到满意拟合后，用 `WP` 写出效率参数文件，供 ENERGY/gf3 等程序进行效率修正。

## 5. GF3 生成 sin

命令：

```bash
source /home/user0/radware.sh
cd /home/user0/work/IJCLAB/rd_work/eff_fit
cp /home/user0/work/IJCLAB/rd_install/rw_current/demo/gfinit.dat .
gf3
```

如果没有图形环境，也可以用：

```bash
gf3_nographics
```

在 GF3 中输入：

```text
<return>
SP Eu152
CA
eu152
<return>
N
ST
1
```

含义：

- `<return>`: 接受 GF3 启动默认设置。
- `SP Eu152`: 读取 `Eu152.spe`。
- `CA`: 执行 AUTOCAL。
- `eu152`: 使用 `eu152.sou`。
- 后面的 `<return>`: 继续自动流程。
- `N`: 不继续额外操作或不保存某些交互选项，具体取决于提示。
- `ST`、`1`: 退出。

这一步会生成：

- `Eu152.sin`: EFFIT 输入文件。
- `Eu152.aca`: GF3 自动能量刻度文件，可选但建议保留。

GF3 `CA` 的关键输出解读：

- `Found ... peaks`: 自动寻峰找到的候选峰数。
- `Matched ... peaks`: 与 `.sou` 成功匹配的标准源峰数。
- `Peak position / Energy fit residual`: 能量刻度残差；残差很大的点可能匹配错峰或峰形有问题。
- `.sin`: 自动积分结果。
- `.aca`: 自动能量刻度结果。

`.sin` 每行把实验峰和标准源信息放在一起。本目录当前例子类似：

```text
1  122.4459  0.0026  159952  421  121.7830  0.0020  13620  160
```

含义：

- 第 1 列：数据组/谱编号。
- 第 2 列：实验峰 centroid 对应能量或刻度后峰位。
- 第 3 列：实验峰位不确定度。
- 第 4 列：GF3 积分得到的实验峰面积 area。
- 第 5 列：area 不确定度。
- 第 6 列：`.sou` 里的标准 gamma 能量。
- 第 7 列：标准能量不确定度。
- 第 8 列：`.sou` 里的相对强度。
- 第 9 列：相对强度不确定度。

EFFIT 实际拟合的是近似相对效率点：

```text
efficiency point ~ experimental area / source relative intensity
```

如果要得到绝对效率，还需要源活度、测量时间、几何位置、死时间等归一化信息；RadWare 这里通常先拟合相对效率曲线，用于相对强度或符合产额修正。

## 6. EFFIT 生成 aef

运行：

```bash
source /home/user0/radware.sh
cd /home/user0/work/IJCLAB/rd_work/eff_fit
effit
```

在 EFFIT 中输入：

```text
<return>
Eu152.sin
<return>
FT -1
<return>
LP
WP Eu152_fit
N
ST
1
```

含义：

- 第一个 `<return>`: 接受启动默认设置。
- `Eu152.sin`: 读入 GF3 自动积分结果。
- 固定参数提示处直接 `<return>`: 使用默认固定参数；通常 `C` 固定为 0，`G` 固定。
- `FT -1`: 重新估计初值并拟合。
- 第二个 `<return>`: 接受默认最大迭代次数。
- `LP`: 列出当前参数。
- `WP Eu152_fit`: 写出 `Eu152_fit.aef`。
- `N`: 不释放 `G` 再拟合。
- `ST`、`1`: 退出。

如果想测试释放 `G`：

```text
FR 7
FT -1
LP
WP Eu152_fit_Gfree
```

但释放 `G` 只是诊断。Eu-152 单独的数据通常不足以稳定约束低能转折，正式曲线优先从固定 `G` 的拟合开始。

## 7. EFFIT 参数形式

EFFIT 文档称可拟合的形状参数为 7 个：`A` 到 `G`，不是 8 个。

低能段：

```text
log(eff) = A + B*log(EG/E1) + C*{log(EG/E1)}**2
```

高能段：

```text
log(eff) = D + E*log(EG/E2) + F*{log(EG/E2)}**2
```

完整形式：

```text
eff = EXP{ [ (A+B*x+C*x*x)**(-G) + (D+E*y+F*y*y)**(-G) ]**(-1/G) }
```

其中 `x=log(EG/E1)`，`y=log(EG/E2)`，`E1=100 keV`，`E2=1000 keV`。

`.aef` 文件实际保存 10 个数：

```text
A B C D E F G E1 E2 deff_log
```

`deff_log` 可换算为效率相对不确定度：

```text
relative uncertainty = exp(deff_log) - 1
```

## 8. 当前自动拟合结果

`Eu152_fit.aef`：

```text
A = 2.1914175
B = 1.5602642
C = 0.0000000
D = 1.5093541
E = -0.7813233
F = -0.0914746
G = 15.0000000
E1 = 100
E2 = 1000
deff_log = 0.0749162
```

对应 `.aef` 文件内容：

```text
EFFIT PARAMETER FILE  28-Jun-26 16:24:39
  2.1914175E+00  1.5602642E+00  0.0000000E+00  1.5093541E+00 -7.8132325E-01
 -9.1474593E-02  1.5000000E+01  1.0000000E+02  1.0000000E+03  7.4916221E-02
```

当前 `deff_log=0.0749162`，对应约 `exp(0.0749162)-1 = 7.8%` 的效率相对不确定度。

`Eu152_fit_Gfree.aef` 释放了 `G`，但 `G` 不稳定，当前不建议优先采用。

## 9. 当前结果的重要警告

这次自动 EFFIT 输出过：

```text
Failed to converge after 100 iterations, Chisq/D.O.F. = 7.569
WARNING - do not believe quoted errors.
```

所以目前的 `Eu152_fit.aef` 是“流程可用的初步曲线”，还不是最终推荐曲线。建议正式使用前人工检查：

- 在 GF3 图形界面里看 `CA` 匹配到的峰和积分区间是否合理。
- Eu-152 有些峰容易受 doublet 或弱峰影响，必要时编辑 `eu152.sou` 删除问题峰。
- 重点看 `Eu152.sin` 中偏离曲线明显的点，必要时在 EFFIT 用 `DE` 删除数据点后重拟合。
- 低能转折如果只靠 Eu-152 不稳定，可以加入 Ba-133 `.sin`；RadWare 文档例子就是 Eu-152 + Ba-133。

## 10. 快速画图检查

```bash
cd /home/user0/work/IJCLAB/rd_work/eff_fit
python3 plot_efficiency_fit.py
```

输出：

```text
Eu152_efficiency_fit.png
```

图上：

- 点：来自 `Eu152.sin` 的实验效率点。
- `Eu152_fit.aef` 曲线：默认固定 `C=0`、`G=15` 的拟合结果，优先参考。
- `Eu152_fit_Gfree.aef` 曲线：释放 `G` 的测试结果，只用于诊断，不建议作为正式效率曲线。

## 11. 推荐的正式操作顺序

1. 从 ROOT 导出 Eu-152 一维谱为 `Eu152.spe`。
2. 检查或编辑 `eu152.sou`，去掉明显 doublet、弱峰或不适合本探测器/本统计量的峰。
3. 在 GF3 中执行 `SP Eu152`、`CA`，生成 `Eu152.sin`。
4. 检查 GF3 `CA` 的峰匹配和能量残差。
5. 在 EFFIT 中读入 `Eu152.sin`，先使用默认固定参数拟合。
6. 看 `Chisq/D.O.F.`、收敛信息和效率图。
7. 如果有离群点，在 EFFIT 用 `DE` 删除坏点后重新 `FT -1`。
8. 满意后用 `WP final_name` 写出最终 `.aef`。
9. 用最终 `.aef` 对实验峰面积做效率修正。

## 12. 常见问题判断

- `CA` 找不到峰：检查 `.spe` 是否写错、通道范围是否合理、峰是否太弱、`.sou` 能量表是否和源一致。
- `CA` 匹配峰很多但残差大：可能能量刻度初值差、匹配错峰、谱里有污染峰。
- EFFIT 不收敛：通常是坏点、doublet、弱峰、本底积分不稳，或者释放了过多参数。
- `Gfree` 拟合飘：Eu-152 单独不足以约束低能转折，建议固定 `G`，或加入 Ba-133 等低能点。
- 曲线形状不平滑：检查 `.sou` 强度、积分区域、离群点。
