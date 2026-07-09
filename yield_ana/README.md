# traditional 产额处理工具

这个目录把原来分散在个人路径中的 traditional 产额处理流程打包成一个可移动工具。运行时只需要给一个 `.ags` 文件和一个输出目录。

## 使用方法

```bash
cd yield_ana
./run_traditional_yield.sh /path/to/input.ags /path/to/output_dir
```

## 流程说明

1. `tools/parse_ags.py` 将 `.ags` 解析成 `<stem>_level.csv` 和 `<stem>_trans.csv`。
2. `tools/cal_yield_v3.py` 使用 `data/correction_coefficients.csv` 生成普通产额 `<stem>_yield.csv` 和 `<stem>_yield_raw.csv`。
3. `tools/take_trans.py` 从 transition CSV 中提取 traditional 方法需要的 `second_level_transitions.csv`，并输出 `long_lived_correction_log.csv`。
4. `tools/gen_yield_tradition.py` 使用 `data/spin_correction_factors_260501.csv` 生成最终产额 `<stem>_traditional_corrected_yields.csv`。

## 打包数据

- `data/correction_coefficients.csv`: `cal_yield_v3.py` 使用的解析修正系数。
- `data/spin_correction_factors_260501.csv`: traditional 最终产额使用的 spin 修正因子。
- `data/NDC.dat`: 随工具一起保存，供后续与 NDC 数据比较时使用。

## 依赖

需要 Python 3，以及 Python 包 `numpy`、`pandas`。
