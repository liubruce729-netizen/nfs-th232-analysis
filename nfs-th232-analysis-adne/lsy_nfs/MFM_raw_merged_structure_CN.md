# MFM Raw / Merged 数据结构与 ADNE 使用说明

本文档说明三个层次的东西：

1. MFM binary 文件里 frame 的真实结构。
2. raw/unmerged data 与 merged data 的区别。
3. ADNE 如何读取这些 frame，并把 EXO2 frame 转成 NFS tree、histo 和后续 addback/mult>=2 事件。

配套脚本：

```bash
lsy_nfs/mfm_to_raw_root_tree.C
lsy_nfs/draw_unmerged_mfm_ts_distribution.C
```

这两个脚本只使用 ROOT + MFMlib，不调用 ADNE 的 detector reconstruction。它们适合做文件结构检查、坏文件定位、raw/unmerged 与 merged 对比。

## 1. 代码来源

主要依据这些源码位置：

```text
MFMlib:
  /home/user0/work/IJCLAB/NFS/pkg/MFMlib/sources/MFMCommonFrame.h
  /home/user0/work/IJCLAB/NFS/pkg/MFMlib/sources/MFMCommonFrame.cc
  /home/user0/work/IJCLAB/NFS/pkg/MFMlib/sources/MFMMergeFrame.h
  /home/user0/work/IJCLAB/NFS/pkg/MFMlib/sources/MFMMergeFrame.cc
  /home/user0/work/IJCLAB/NFS/pkg/MFMlib/sources/MFMExogamFrame.h
  /home/user0/work/IJCLAB/NFS/pkg/MFMlib/sources/MFMExogamFrame.cc
  /home/user0/work/IJCLAB/NFS/pkg/MFMlib/sources/MFMTypes.h

ADNE/NFS:
  /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne/GUser.C
  /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne/GUser.h
  /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne/TExogam2.cxx
  /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne/TExogam2.h
  /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne/TExogam2Data.h
```

## 2. top frame 是什么

`top frame` 是文件层级概念，不是探测器概念。

MFMlib 读取 binary 文件时，调用：

```cpp
MFMCommonFrame::ReadInFile(int *lun)
```

它每次从文件当前位置读取一个完整 MFM frame。这个“直接从文件读出来的最外层 frame”就是本文和脚本里的 `top frame`。

如果 input 是 unmerged/raw 文件，top frame 可能直接就是 EXO2、NEDA、DIAMANT 等 detector frame。

如果 input 是 merged 文件，top frame 通常是：

```text
MFM_MERGE_TS_FRAME_TYPE = 0xff02
```

这个 top merge frame 是一个 event container。它内部连续存放多个 detector frame。ADNE 后续不会重新用时间窗做 event build，而是沿用这个 top merge frame 代表的 event。

脚本里的层级定义：

```text
depth = 0  顶层 frame，即 ReadInFile() 直接读出来的 frame
depth = 1  顶层 merge frame 内部的子 frame
depth = 2  如果 merge 内部还有 merge，则继续递归
```

## 3. MFM common header

MFM 所有 frame 共有 common header。源码在 `MFMCommonFrame.h`：

```cpp
struct MFM_common_header {
  unsigned metaType : 8;
  unsigned frameSize : 24;
  unsigned dataSource : 8;
  unsigned frameType : 16;
  unsigned revision : 8;
};
```

含义：

```text
metaType   : endian、blobness、unit block size 等信息。
frameSize  : 当前 frame 总字节数。读坏文件时，这个值异常会导致 Crazy Frame size。
dataSource : 数据源编号。
frameType  : frame 类型，例如 EXO2=0x10，MERGE_TS=0xff02。
revision   : frame revision。
```

`MFMCommonFrame::SetAttributs()` 会从 header 解析这些字段，并继续解析：

```text
frame type
frame size
event number
timestamp
board/channel location
```

## 4. 常见 frame type

来自 `MFMTypes.h`：

```text
0x10   MFM_EXO2_FRAME_TYPE       EXOGAM2 / NUMEXO2 crystal frame
0x12   MFM_NEDA_FRAME_TYPE       NEDA raw frame
0x13   MFM_NEDACOMP_FRAME_TYPE   NEDA compressed frame
0x16   MFM_DIAMANT_FRAME_TYPE    DIAMANT / CsI frame
0x1c   MFM_REA_GENE_FRAME_TYPE   generic REA frame
0x90   MFM_PARIS_FRAME_TYPE      PARIS frame
0xff00 MFM_HELLO_FRAME_TYPE      hello frame
0xff01 MFM_MERGE_EN_FRAME_TYPE   merge by event number
0xff02 MFM_MERGE_TS_FRAME_TYPE   merge by timestamp
0xff10 MFM_XML_DATA_DESCRIPTION_FRAME_TYPE
0xff11 MFM_XML_FILE_HEADER_FRAME_TYPE
```

## 5. merged frame 结构

源码在 `MFMMergeFrame.h`：

```cpp
#define MERGE_EN_HEADERSIZE MFM_BASIC_DEFAULT_HEADERSIZE + 4
#define MERGE_TS_HEADERSIZE MFM_BASIC_DEFAULT_HEADERSIZE + 6 + 4

struct MFM_Merge_TSeventInfo {
  char     eventTime[6];
  uint32_t deltaTime;
};
```

`MFM_MERGE_TS_FRAME_TYPE` 的 top frame 里有：

```text
common/basic header
6-byte timestamp eventTime
uint32 deltaTime
nItems: 内部子 frame 数量
然后是 nItems 个连续存放的 MFM 子 frame
```

ADNE 对 merge frame 的处理在 `GUser::Unpack()`：

```cpp
if(frame->GetFrameType() == MFM_MERGE_EN_FRAME_TYPE ||
   frame->GetFrameType() == MFM_MERGE_TS_FRAME_TYPE){
  pInceptionMergeFrame[InceptionLayer]->SetAttributs(frame->GetPointHeader());
  pInceptionMergeFrame[InceptionLayer]->ResetReadInMem();
  for (...) {
    pInceptionMergeFrame[InceptionLayer]->ReadInFrame(...);
    Unpack(childFrame, debug);
  }
}
```

这说明 ADNE 的 event 边界来自 merge frame，不是 ADNE 在 `TExogam2` 里重新按时间窗聚类。

## 6. EXO2 frame binary 结构

源码在 `MFMExogamFrame.h`：

```cpp
#define EXO_FRAMESIZE 52

struct MFM_exo_eventInfo {
  unsigned EventIdx  : 32;
  char EventTime[6];
};

struct MFM_exo_data{
  unsigned CristalId : 16;
  unsigned Status1   : 16;
  unsigned Status2   : 16;
  unsigned Status3   : 16;
  unsigned DeltaT    : 16;
  unsigned Inner6M   : 16;
  unsigned Inner20M  : 16;
  unsigned Outer1    : 16;
  unsigned Outer2    : 16;
  unsigned Outer3    : 16;
  unsigned Outer4    : 16;
  unsigned BGO       : 16;
  unsigned Csi       : 16;
  unsigned InnerT30  : 16;
  unsigned InnerT60  : 16;
  unsigned InnerT90  : 16;
  unsigned Padding   : 16;
};
```

EXO2 frame 总大小是 52 bytes：

```text
8 bytes common/blob header
4 bytes EventIdx
6 bytes EventTime / timestamp
34 bytes EXO data = 17 个 uint16 字段
```

### 6.1 CristalId

`CristalId` 是 16-bit 复合字段。

MFMlib 解析函数：

```cpp
ExoGetTGCristalId() = CristalId & 0x001f
ExoGetBoardId()     = (CristalId >> 5) & 0x07ff
```

ADNE 进一步把 `ExoGetTGCristalId()` 压成 LUT 使用的 halfboard：

```cpp
EXO2CrysId = frame->ExoGetTGCristalId();
EXO2BoardId = frame->ExoGetBoardId();
if (EXO2CrysId != 0) EXO2CrysId = 1;
```

然后查：

```cpp
Conf/Exogam2_in_Tree.cfg
```

这个 LUT 格式是：

```text
clover crystal board halfboard
```

例如：

```text
7 0 130 0
7 1 130 1
```

表示 board 130 的 halfboard 0/1 分别映射到 clover 7 的 crystal 0/1。

### 6.2 Inner6M / Inner20M

ADNE 当前 NFS 主要使用：

```cpp
frame->ExoGetInnerM(0)
```

也就是 `Inner6M`。在 `TExogam2::IsMFMExo()` 中先要求：

```cpp
frame->ExoGetInnerM(0) > 10
```

然后用 `ECoef` 能量刻度：

```cpp
valf = Cal(frame->ExoGetInnerM(0), ECoef[MapFinger][0], ECoef[MapFinger][1], ECoef[MapFinger][2]);
```

写入 tree：

```text
fEXO_ECC_E_Energy
```

### 6.3 DeltaT / TDC

EXO2 binary 里的 `DeltaT` 是原始 16-bit TDC 类字段：

```cpp
rawDeltaT = frame->ExoGetDeltaT();
```

普通 ADNE 时间刻度：

```cpp
valf2 = Cal(rawDeltaT, TCoef[MapFinger][0], TCoef[MapFinger][1], TCoef[MapFinger][2]);
fEXO_ECC_T_Time = valf2;
```

NFS 专用当前逻辑：

```cpp
reversedDeltaT = 65536 - rawDeltaT;
deltaTNs = reversedDeltaT * 0.024;               // no crystal correction
neutronTOF = deltaTNs + GammaFlashOffset;        // no crystal correction
```

如果打开 crystal time correction，则使用 `ecc.cal` 时间部分给出的相对修正：

```text
deltaTNs = reversedDeltaT * 0.024 * relative_gain
         + GammaFlashOffset
         + relative_offset
```

随后：

```cpp
if(rawDeltaT <= 60000 && neutronTOF > 0){
  validNfsTime = true;
  fDeltaT      = deltaTNs;
  fNeutronTOF  = neutronTOF;
}
```

最终 NFS tree 里 `Time` 保存的是 `neutronTOF`，不是绝对 TS。

### 6.4 BGO / CsI

EXO2 frame 里直接有：

```text
BGO
Csi
```

ADNE 存入：

```text
fEXO_ESS_BGO
fEXO_ESS_CSI
```

NFS addback 和 veto 逻辑里，fire 定义为：

```text
BGO > 0
CSI > 0
```

### 6.5 Outer1-4 与 Status3

`Outer1-4` 对应 GOCCE segment/mirror/net charge 信息。ADNE 用 `Status3` 的 bit 判断每个 segment 是 net charge 还是 mirror：

```cpp
binstatus = (((statusSegment >> (8*EXO2CrysId)) >> seg+2) & 1);
```

当前 NFS fission/addback 主流程主要使用 core energy，不依赖 GOCCE segment 重建。

### 6.6 T30/T60/T90

`InnerT30/60/90` 是 EXOGAM PSA/rise-time 相关字段。ADNE 当前把它们保存到 tree：

```text
fEXO_ECC_E_T30
fEXO_ECC_E_T60
fEXO_ECC_E_T90
```

当前 NFS 主分析没有用 T30/T60/T90 改 TDC，只是保留和绘图。后续如果做 CFD/PSA 时间修正，可以研究这些量和 gamma-flash peak 偏移之间的相关性。

## 7. ADNE 中 raw tree、NFS tree、mult tree 的差别

### 7.1 RawTree

如果 YAML 中：

```yaml
nfs_exo_ana:
  raw_tree: true
```

ADNE 会在 `GUser::CaptureRawEvent()` 中保存 raw event。一个 RawTree entry 对应一个 top-level MFM event/frame。它会保存：

```text
raw_top_frame_type
raw_top_frame_size
raw_top_timestamp
raw_frame_type vector
raw_frame_timestamp vector
raw_exo_delta_t vector
raw_exo_inner_m6 vector
...
```

RawTree 仍然依赖 ADNE 程序，但尽量不做 EXO2 物理判断。

### 7.2 NFS Tree

NFS tree 是处理后的分析 tree。它经历：

```text
MFM frame unpack
LUT board/halfboard -> clover/crystal
active clover 判断
core raw energy > 10 判断
energy calibration
DeltaT/TDC conversion
BGO/CSI 保存
T30/T60/T90 保存
```

输出文件名通常是：

```text
out/nfs_run_XXX_rY.root
```

### 7.3 mult tree

mult tree 文件名暂时保留历史前缀：

```text
out/mult3_nfs_run_XXX_rY.root
```

但当前代码里的判选已经改成 mult>=2。它基于 NFS addback clover fire，带 BGO/CSI veto，多重度满足要求后写入。

## 8. raw/unmerged data 与 merged data 的区别

### raw/unmerged

典型特征：

```text
top frame 可能直接是 detector frame，例如 EXO2=0x10
top frame 不一定代表完整物理 event
需要额外 event builder/merger 才能把不同 detector fire 组合成 event
```

### merged

典型特征：

```text
top frame 通常是 MFM_MERGE_TS_FRAME_TYPE = 0xff02
每个 top merge frame 是一个 event container
内部有 nItems 个 detector frame
ADNE 递归展开内部 frame，并把同一个 top merge frame 的内容作为一个 event 处理
```

如果两个文件大小相同，但 ADNE 处理出来的 event 数差很多，优先比较：

```text
MfmTopTree entries
MfmFrameTree entries
frame_type 分布
MERGE_TS 的 nb_items 分布
EXO2 frame 数量
TS=0 frame 数量
frame_size 是否异常
file_offset 附近是否出现 Crazy Frame size
```

## 9. mfm_to_raw_root_tree.C 用法

基本运行：

```bash
cd /home/user0/work/IJCLAB/NFS/nfs-th232-analysis/nfs-th232-analysis-adne
source /home/user0/work/IJCLAB/NFS/NFS_env.sh
root -l -b -q 'lsy_nfs/mfm_to_raw_root_tree.C("data/run_0023.dat.25-09-23_14h32m42s.rawtest_64MiB",2000,"out/mfm_raw_tree.root")'
```

完整参数：

```cpp
mfm_to_raw_root_tree(inputMfmFile,
                     maxTopEvents,
                     outputFile,
                     unfoldMerge,
                     maxPrintTopEvents,
                     tsTickNs,
                     maxStoredTopBytes)
```

推荐调试：

```bash
root -l -b -q 'lsy_nfs/mfm_to_raw_root_tree.C("data/file.dat",100,"out/check.root",true,20,10.0,64)'
```

这里：

```text
100   : 只读前 100 个 top frame
true  : 展开 merge frame
20    : 终端打印前 20 个 top event 的层级结构
10.0  : 1 TS tick = 10 ns
64    : 每个 top frame 保存前 64 bytes 原始 binary，方便抽查 header
```

## 10. mfm_to_raw_root_tree.C 输出怎么看

打开 ROOT：

```bash
root -l out/mfm_raw_tree.root
```

查看顶层 frame：

```cpp
MfmTopTree->Scan("selected_top_index:file_offset:frame_type:frame_type_name:frame_size:timestamp:nb_items", "", "", 30)
```

如果是 merged 文件，你应该看到很多：

```text
frame_type_name = MERGE_TS
nb_items > 0
```

查看展开后的 frame：

```cpp
MfmFrameTree->Scan("selected_top_index:depth:index_in_parent:frame_type_name:timestamp:exo_board_id:exo_tg_crystal_id:exo_delta_t:exo_inner6m:exo_bgo:exo_csi", "", "", 50)
```

解释：

```text
depth=0  top frame
depth=1  merge frame 内部 detector frame
EXO2 frame 才会有 exo_* 字段；非 EXO2 frame 的 exo_* 为 -1
```

画 frame type 分布：

```cpp
MfmFrameTree->Draw("frame_type")
```

只看 EXO2 原始能量：

```cpp
MfmFrameTree->Draw("exo_inner6m", "is_exo2")
```

只看 EXO2 raw DeltaT：

```cpp
MfmFrameTree->Draw("exo_delta_t", "is_exo2")
```

看 TS 和 TDC 的相位差诊断：

```cpp
MfmFrameTree->Draw("exo_ts_phase_minus_tdc_ns", "is_exo2")
```

比较两个文件时，建议对两个输出 ROOT 都跑：

```cpp
MfmTopTree->GetEntries()
MfmFrameTree->GetEntries()
MfmFrameTree->Draw("frame_type")
MfmFrameTree->Draw("nb_items", "is_merge")
MfmFrameTree->Draw("exo_board_id", "is_exo2")
MfmFrameTree->Draw("timestamp", "timestamp==0")
```

## 11. draw_unmerged_mfm_ts_distribution.C 新增输出

这个脚本原本已经输出：

```text
mfm_top_event_ts_distribution
mfm_all_frame_ts_distribution
mfm_exo2_frame_ts_distribution
mfm_exo2_frame_delta_ts
mfm_exo2_frame_delta_ts_read_order
exo2_crystal_ts/*_delta_ts_sorted
exo2_crystal_ts/*_delta_ts_read_order
```

现在额外输出：

```text
mfm_exo2_ts_phase_minus_reversed_tdc
exo2_crystal_ts/mfm_exo2_boardXXX_crystalYY_ts_phase_minus_reversed_tdc
```

`mfm_frame_table` 也新增：

```text
exo_delta_t
exo_reversed_delta_t_ns
exo_ts_phase_minus_tdc_ns
```

这些量和 ADNE 新增的 NFS histo 定义一致。

## 12. ADNE 新增 NFS histo

ADNE NFS histo 文件：

```text
out/nfs_histoExogam2_1.root
```

新增：

```text
nfs_all_crystal_ts_diff
nfs_all_crystal_ts_phase_minus_tdc
```

含义：

```text
nfs_all_crystal_ts_diff:
  已接受 EXO2 crystal frame 的读取顺序 TS 差分。
  TS tick 按 10 ns 转换。

nfs_all_crystal_ts_phase_minus_tdc:
  对每个已接受 EXO2 crystal frame，计算：
  TS 相位 - (65536-DeltaT)*0.024 ns
  TS 相位是把绝对 TS 按 65536*0.024 ns 的 TDC 周期折叠后得到。
```

注意：这两个图只用于诊断 TS/TDC 关系，不参与 NFS addback、mult>=2、fission event selection。
