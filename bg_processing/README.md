# BG 处理工具包

这个目录把当前 BG 处理流程需要的主脚本和依赖脚本放在一起，目标是在新环境中只改工具目录和输入 ROOT 路径即可运行。

## 目录结构

```text
bg_processing/
├── run_bg.sh          # SNIP + RadWare 公式二维背景 + delta 回加流程
├── run_rad_bg.sh      # RadWare 公式二维背景 + deskew + delta 回加流程
└── tools/
    ├── root_projection.py  # normal-bin 投影、一维本底、二维背景、直接写 spe
    ├── bg_sub_SY_v2.py     # SNIP 背景扣除，生成 *_bg / *_sig
    ├── sub_fig.py          # ROOT histogram 加减，用于 delta 和 final
    ├── skew_edge.py        # run_rad_bg.sh 使用的 deskew 工具
    ├── save2m4b.C          # ROOT TH2 -> RadWare m4b，signed int32
    └── expand1Dto2D.C      # 一维谱展开成二维辅助矩阵，兼容旧 m4b 流程
```

## 依赖

需要系统里已有：

```text
bash
python3
ROOT / PyROOT，包含 TSpectrum
numpy
```

如果 ROOT/RadWare 环境需要初始化，运行前先 source 对应环境脚本，例如：

```bash
source /home/user0/radware.sh
```

## 新环境需要改哪里

两个主脚本顶部都有同样的配置块：

```bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLSDIR="${TOOLSDIR:-${SCRIPT_DIR}/tools}"

INPUT_ROOT="${INPUT_ROOT:-big_win_th.root}"
INPUT_HIST="${INPUT_HIST:-EE_T}"
```

如果保持 `tools/` 和脚本在同一个目录，`TOOLSDIR` 不需要改。
如果把工具脚本放到别处，修改 `TOOLSDIR`，或运行时覆盖：

```bash
TOOLSDIR=/path/to/tools INPUT_ROOT=/path/to/input.root INPUT_HIST=EE_T bash run_bg.sh all
```

最常用方式是进入一个空的输出目录，再用绝对路径指定输入 ROOT：

```bash
source /home/user0/radware.sh
mkdir -p /path/to/output_dir
cd /path/to/output_dir
INPUT_ROOT=/path/to/input.root INPUT_HIST=EE_T bash /path/to/bg_processing/run_bg.sh all
```

## run_bg.sh 流程

`run_bg.sh` 用于 SNIP + RadWare 公式二维背景的组合流程：

1. 从输入二维矩阵做 normal-bin `ProjectionX`，不包含 ROOT underflow/overflow。
2. 用 `TSpectrum.Background` 估计一维本底 `S_proj_bg`。
3. 直接写出 `S_proj.spe` 和 `S_proj_bg.spe`。
4. 用 RadWare 官方等价公式构造二维本底和 rad 信号：

   ```text
   B_ij = (P_i P_j - p_i p_j) / T
   ```

5. 用 `bg_sub_SY_v2.py` 做 SNIP 背景扣除。
6. 计算 `Delta = SNIP_signal - rad_signal`。
7. 回加得到 `Final = 原始矩阵 + Delta`。
8. 保存最终 ROOT 和 m4b。

可用入口：

```bash
bash run_bg.sh all
bash run_bg.sh rad_origin
bash run_bg.sh snip
bash run_bg.sh delta
bash run_bg.sh save_final
bash run_bg.sh convert_only
```

主要输出：

```text
rad_re_mut${T}_${W}.root
S_proj.spe
S_proj_bg.spe
rad_bg_mut${T}_${W}.m4b
final_matrix_win80_rad_mut${T}_${W}.m4b
SY_${SNIP_M}.root
delta.root
final_matrix_win80_sy_${SNIP_M}.root
final_matrix_win80_sy_${SNIP_M}.m4b
```

## run_rad_bg.sh 流程

`run_rad_bg.sh` 是不跑 SNIP 的 rad/deskew 版本：

1. 构造 rad 二维背景和 rad 信号。
2. 对 rad 信号做 deskew。
3. 计算 `Delta = deskew_rad_signal - rad_signal`。
4. 回加到原始矩阵。
5. 保存 ROOT 和 m4b。

可用入口：

```bash
bash run_rad_bg.sh all
bash run_rad_bg.sh rad_origin
bash run_rad_bg.sh deskew
bash run_rad_bg.sh delta
bash run_rad_bg.sh save_final
bash run_rad_bg.sh convert_only
```

主要输出：

```text
rad_re_mut${T}_${W}.root
S_proj.spe
S_proj_bg.spe
rad_bg_mut${T}_${W}.m4b
final_matrix_win80_rad_mut${T}_${W}.m4b
delta.root
final_matrix_win80_rad_deskew.root
final_matrix_win80_rad_deskew.m4b
```

## 注意

- 所有输出都写入当前工作目录，所以建议先 `cd` 到专门的输出目录再运行。
- `root_projection.py project` 只累加 normal bins，不把 ROOT underflow/overflow 加入投影。
- `.spe` 由 `root_projection.py spe` 直接写出，不再通过二维 m4b + `slice` 间接投影。
- `save2m4b.C` 使用 signed int32 写 m4b；浮点 bin 会向 0 截断。
- `rad_bg_mut*.m4b` 是由一维本底展开的辅助矩阵，用于兼容旧流程，不是物理二维对称背景。
