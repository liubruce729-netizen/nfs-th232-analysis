# EXOGAM2 tree branch reference

本文档整理 ADNE/EXOGAM2 ROOT tree 中常见 `fEXO_*` 分支的含义，并用一个单 crystal fire 的例子说明如何阅读这些数组。

代码来源主要在：

- `nfs-th232-analysis-adne/TExogam2.cxx`
- `nfs-th232-analysis-adne/TExogam2Data.h`
- `nfs-th232-analysis-adne/TExogam2Data.cxx`

其中 `TExogam2::IsMFMExo()` 负责从 MFM EXOGAM frame 中取出 core energy、core time、segment、BGO/CSI、timestamp 等信息，并填入 `TExogam2Data`。

## 基本阅读规则

ROOT tree 中很多分支是 vector。打印时看到的：

```text
branch_name: n=1 values=[...]
```

意思是该 event 中这个分支有 1 个条目。属于同一组的分支一般按相同 index 对齐。例如：

```text
fEXO_ECC_E_Clover[0]
fEXO_ECC_E_Cristal[0]
fEXO_ECC_E_DetNbr[0]
fEXO_ECC_E_Energy[0]
```

共同描述同一个 crystal fire。

## fEXO_ECC_E_*: core energy information

`ECC` 对应 EXOGAM crystal core，也就是高纯锗 clover 中每个 crystal 的 core 信号。`E` 表示 energy side。

### fEXO_ECC_E_Clover

该条 core energy 属于哪个 clover。

例子：

```text
fEXO_ECC_E_Clover: n=1 values=[7]
```

表示该 event 中有一个 core energy fire，来自 clover 7。

### fEXO_ECC_E_Cristal

该条 core energy 属于 clover 内第几个 crystal。代码中 crystal 编号通常为 `0, 1, 2, 3`。

例子：

```text
fEXO_ECC_E_Cristal: n=1 values=[1]
```

表示 clover 7 的 crystal 1 fire。

### fEXO_ECC_E_DetNbr

全局 crystal 编号，也常称 `MapFinger`。

当前代码中通常采用：

```text
DetNbr = clover * 4 + crystal
```

例子：

```text
7 * 4 + 1 = 29
```

所以：

```text
fEXO_ECC_E_DetNbr: n=1 values=[29]
```

与 clover 7 crystal 1 对应。

### fEXO_ECC_E_Energy

core energy，经过 `CalFile/ecc.cal` 中 `ECoef` 校准后的能量值。单位通常按 keV 使用。

例子：

```text
fEXO_ECC_E_Energy: n=1 values=[439.7926330566406]
```

表示该 crystal 的 core 能量约为 439.79 keV。

### fEXO_ECC_E_T30, fEXO_ECC_E_T60, fEXO_ECC_E_T90

这些是 NUMEXO2/EXOGAM frame 中保存的 pulse-shape/timing 相关量，来自：

```cpp
frame->ExoGetInnerT(0)
frame->ExoGetInnerT(1)
frame->ExoGetInnerT(2)
```

开发者提到它们反映 charge carrier path，可用于进一步改善 timing resolution。当前常规分析中它们主要被保存和画谱，尚未作为 NFS ToF 的主校正量使用。

例子：

```text
T30 = 18
T60 = 36
T90 = 64
```

## fEXO_ECC_T_*: core timing information

`ECC_T` 是 core time 信息，与 `ECC_E` 对应，但记录的是 TDC/time side。

### fEXO_ECC_T_Clover

该条 core time 属于哪个 clover。

例子：

```text
fEXO_ECC_T_Clover: n=1 values=[7]
```

### fEXO_ECC_T_Cristal

该条 core time 属于 clover 内第几个 crystal。

例子：

```text
fEXO_ECC_T_Cristal: n=1 values=[1]
```

### fEXO_ECC_T_Time

core TDC/time 值。代码中由：

```cpp
rawDeltaT = frame->ExoGetDeltaT();
valf2 = Cal(rawDeltaT, TCoef[MapFinger][0], TCoef[MapFinger][1], TCoef[MapFinger][2]);
fExogam2Data->SetECCTTime(valf2);
```

得到。

也就是说，它使用 `CalFile/ecc.cal` 第二段的 `TCoef` 做校准。不过当前 NFS 版本的 `ecc.cal` 第二段多为：

```text
0 1 0
```

因此在这种情况下，`fEXO_ECC_T_Time` 基本就是 raw DeltaT/TDC channel，而不是已经转换好的 ns。

例子：

```text
fEXO_ECC_T_Time: n=1 values=[26464]
```

表示 clover 7 crystal 1 的 core time channel/calibrated time value 为 26464。

### fEXO_ECC_T_TSRequest

原代码中有相关 setter 注释：

```cpp
// fExogam2Data->SetECCTTSRequest(frame->ExoGetTGCristalId());
// is the TS request of the core to the GTS
```

但当前这行没有启用，所以该分支经常为空。

例子：

```text
fEXO_ECC_T_TSRequest: n=0 values=[]
```

表示这个 event 没有写入 TSRequest 信息。

## fEXO_GOCCE_E_*: segment energy information

`GOCCE` 对应 EXOGAM outer/segment 信息，不是 BGO，也不是 CsI。它描述的是同一个 Ge crystal 上 segment/outer contact 的信息。

一个 crystal 通常有 4 个 segment，因此一个 core fire 可能伴随 4 条 GOCCE segment 记录。

### fEXO_GOCCE_E_Clover

segment energy 属于哪个 clover。

例子：

```text
fEXO_GOCCE_E_Clover: n=4 values=[7, 7, 7, 7]
```

表示四条 segment 记录都属于 clover 7。

### fEXO_GOCCE_E_Cristal

segment energy 属于哪个 crystal。

例子：

```text
fEXO_GOCCE_E_Cristal: n=4 values=[1, 1, 1, 1]
```

表示四条 segment 记录都属于 clover 7 crystal 1。

### fEXO_GOCCE_E_Segment

segment 编号。

例子：

```text
fEXO_GOCCE_E_Segment: n=4 values=[0, 1, 2, 3]
```

表示该 crystal 的四个 segment 都被列出。

### fEXO_GOCCE_E_Energy

segment energy。若为 0，通常表示对应 segment 没有有效能量沉积，或者该 event 中 segment 信息只是占位/无有效信号。

例子：

```text
fEXO_GOCCE_E_Energy: n=4 values=[0.0, 0.0, 0.0, 0.0]
```

表示四个 segment 都没有有效 segment energy。

### fEXO_GOCCE_E_Status

segment status/flag。具体含义依赖 EXOGAM frame status 编码。常见 `0` 表示没有特殊状态或没有有效 segment hit。

例子：

```text
fEXO_GOCCE_E_Status: n=4 values=[0, 0, 0, 0]
```

## fEXO_GOCCE_T_*: segment timing information

`GOCCE_T` 是 segment time 信息。当前数据中常为空，说明该 event 没有写入 segment timing。

例子：

```text
fEXO_GOCCE_T_Clover: n=0 values=[]
fEXO_GOCCE_T_Cristal: n=0 values=[]
fEXO_GOCCE_T_Segment: n=0 values=[]
fEXO_GOCCE_T_Time: n=0 values=[]
```

## fEXO_ESS_*: BGO/CSI shield information

`ESS` 分支保存与该 EXOGAM crystal 对应的 BGO/CSI 反符合探测器信息。它不是 Ge segment，而是 shield/anti-Compton 相关量。

### fEXO_ESS_Clover

BGO/CSI 信息对应哪个 clover。

例子：

```text
fEXO_ESS_Clover: n=1 values=[7]
```

### fEXO_ESS_Cristal

BGO/CSI 信息对应哪个 crystal。

例子：

```text
fEXO_ESS_Cristal: n=1 values=[1]
```

### fEXO_ESS_BGO

BGO 信号值。非零通常表示该 crystal/clover 对应的 BGO 有 fire。

例子：

```text
fEXO_ESS_BGO: n=1 values=[826]
```

表示 clover 7 crystal 1 对应 BGO 有明显响应，值为 826。

### fEXO_ESS_CSI

CsI 信号值。非零通常表示 CsI 有 fire。

例子：

```text
fEXO_ESS_CSI: n=1 values=[0]
```

表示该 event 中对应 CsI 没有响应。

## fTimeStampsExogam2[100]

`fTimeStampsExogam2` 是一个固定长度数组，用于保存每个 detector/crystal 对应的 absolute timestamp。

代码中写入方式为：

```cpp
fExogam2Data->SetfTimeStamps(frame->GetTimeStamp(), MapFinger);
```

因此一般只有发生 fire 的 `MapFinger` 位置会被填入非零 timestamp，其余位置为 0。

对于本例：

```text
clover = 7
crystal = 1
MapFinger = DetNbr = 29
```

所以如果查看完整：

```text
fTimeStampsExogam2[100]
```

通常应在 index 29 附近看到该 fire 的 absolute timestamp，其他未响应 detector 位置多为 0。

这个 timestamp 是事件构建/符合判断中更上游使用的绝对时间戳。后续 NFS ToF、中子能量、gamma flash 对齐等分析主要使用 `DeltaT/TDC` 相关信息，而不是直接使用这个 absolute timestamp。

## 本例事件的整体解释

给出的 event 可以读作：

1. 该 event 中 EXOGAM 只有一个 core energy fire。
2. fire 来自 clover 7 crystal 1，全局 detector number 为 29。
3. core energy 为约 439.79 keV。
4. core time/TDC 值为 26464。
5. T30/T60/T90 分别为 18/36/64，可作为 pulse-shape/timing 相关辅助量保存。
6. 该 crystal 的 4 个 Ge segment 被列出，但 segment energy 全为 0，说明没有有效 segment energy。
7. BGO 值为 826，CsI 为 0，说明 BGO 有响应，CsI 没有响应。
8. `fTimeStampsExogam2` 中 detector 29 的位置应保存该 crystal fire 的 absolute timestamp。

