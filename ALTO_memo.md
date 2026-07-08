# ALTO 备忘

## 总谱分析

- 开窗：40 ns
- 条件：Mult >= 3
- 内容：全部 clover 总谱、时间分布、E-E
- 数据文件：`/home/user0/work/IJCLAB/server/total_data.root`

## 单晶体谱

- 方式：逐个 run
- 输出目录：`/home/user0/work/IJCLAB/example/all_out`

## 初步参数

- 峰宽参数：F, G, H = 3.54, 0.00, 0.00
- 效率曲线：暂时使用 `/home/e877_ana/Analysis/nfs-th232-analysis-main/eff_ca/examples/run16/initial_fit` 下的 aef
- aca：使用初始参数，不额外刻度

## 后续流程

1. 先 parallel run + fission ana。
2. 对输出调用 run bg：
   - 路径：`/home/e877_ana/Analysis/fission_ana/nfs-th232-analysis/bg_processing`
   - 目标：得到 m4b 文件。
3. 对 final rad 做投影。
4. 对 rad bg 做投影。
5. 将 bg spe、rad spe 和 m4b 放到：
   - `/data/e877_anaX/rw_work`
6. 在 `/data/e877_anaX/rw_work` 下执行 escl8r。
