# Event Build Step

本说明记录当前对 NFS Th-232 数据中 event build 流程的理解，重点回答：

- 原始 `.dat` 文件里的 event 是否已经 build 完成；
- ADNE 后续是否重新 build event；
- 当前看到的 event build 宽度 `80 tick` 从哪里来；
- 源码中哪里提到了 `EventBuilder` / `EventMerger`。

> 说明：本文中的源码路径来自本地工作区 `/home/user0/work/IJCLAB/NFS`。当前 GitHub 仓库只保存分析脚本和说明文档，不包含完整 ADNE / GRU / MFMlib 源码。

## Short Conclusion

当前数据文件：

```text
/home/user0/work/IJCLAB/NFS/data_test/run_0100.dat.26-09-23_10h55m20s
```

顶层 frame 已经是 `MFM_MERGE_TS_FRAME_TYPE`，也就是 **按 timestamp 合并后的 MFM merge event**。

因此，后续 ADNE 的处理逻辑是：

```text
MFM raw file
  -> already-built MFM_MERGE_TS event
  -> ADNE / GRU reads one merge event at a time
  -> recursively unpacks the sub-frames inside the merge event
  -> Exogam2 / CsI / BGO / other detector classes unpack their own frames
  -> ADNE applies prompt gate, anti-Compton, addback, neutron-energy conversion
  -> one processed event is filled into TreeMaster
```

也就是说，**ADNE 没有重新决定哪些 detector hit 属于同一个 event**。ADNE 继承了上游 MFM event builder 已经生成好的 event 边界。

## MFM Merge Event Evidence

MFMlib 对 merge frame 类型的定义在：

```text
/home/user0/work/IJCLAB/NFS/install/MFMlib/include/MFMTypes.h:77
```

相关定义：

```cpp
#define MFM_MERGE_EN_FRAME_TYPE  0xFF01
#define MFM_MERGE_TS_FRAME_TYPE  0xFF02
```

其中 `MFM_MERGE_TS_FRAME_TYPE` 的说明是 timestamp merge：

```text
/home/user0/work/IJCLAB/NFS/install/MFMlib/include/MFMMergeFrame.h:20
```

相关文字定义：

```cpp
#define MFM_MERGE_TS_FRAME_TYPE_SHORT_TXT "MergT"
#define MFM_MERGE_TS_FRAME_TYPE_LONG_TXT "Merge with Timestamp"
```

`MFM_MERGE_TS` header 里包含 event time 和 delta time：

```text
/home/user0/work/IJCLAB/NFS/install/MFMlib/include/MFMMergeFrame.h:27
```

```cpp
struct MFM_Merge_TSeventInfo {
    char     eventTime[6];
    uint32_t deltaTime;
};
```

但是在当前检查过的数据中，merge header 里的 `deltaTime` 没有给出有效的 event-builder 窗口值。因此，下面提到的 `80 tick` 是从当前数据中反推得到的，不是从 header 直接读取到的配置。

## ADNE Reads Existing MFM Events

ADNE 主程序用 `mfm` 模式初始化事件读取：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/AnalysisADNE.C:154
```

```cpp
aa->EventInit("ACTIONS_experiment.CHC_PAR","mfm");
```

然后调用：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/AnalysisADNE.C:263
```

```cpp
aa->DoRun();
```

`mfm` 模式会创建 `GEventMFM`：

```text
/home/user0/work/IJCLAB/NFS/install/GRU/src_bis/GAcq.C:1463
```

```cpp
if (CompareWordsIgnoreCase(type, "mfm")) {
    SetEvent(((GEventBase*) (new GEventMFM(GetDataParameters()))));
}
```

GRU 的主循环逐个从 buffer 中读 event，并调用用户分析：

```text
/home/user0/work/IJCLAB/NFS/install/GRU/src_bis/GAcq.C:342
```

```cpp
iGAcq->GetEvent()->NextEvent(...);
iGAcq->User();
```

`GEventMFM::ReadNextEvent()` 会识别当前 frame type。如果 frame type 是 merge frame，就将其作为 `MFMMergeFrame` 读取：

```text
/home/user0/work/IJCLAB/NFS/install/GRU/src_bis/GEventMFM.C:295
```

```cpp
case MFM_MERGE_TS_FRAME_TYPE:
case MFM_MERGE_EN_FRAME_TYPE: {
    pCurrentFrame = pMergeFrame;
    pMergeFrame->SetAttributs((void*) pEventBrut_char);
    fEventNumber = pMergeFrame->GetEventNumber();
    fTimeStamp = pMergeFrame->GetTimeStamp();
    break;
}
```

这说明 ADNE/GRU 在这里是 **读取已有 merge event**，不是生成 merge event。

## ADNE Unpacks Sub-Frames Inside One Event

每个 event 进入 `GUser::User()` 后，ADNE 会先清空各探测器对象，然后拿到当前 MFM frame：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/GUser.C:874
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/GUser.C:896
```

随后调用：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/GUser.C:906
```

```cpp
Unpack(pCommonFrame,debug);
```

`GUser::Unpack()` 中，如果当前 frame 是 merge frame：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/GUser.C:1256
```

```cpp
if(frame->GetFrameType() == MFM_MERGE_EN_FRAME_TYPE ||
   frame->GetFrameType() == MFM_MERGE_TS_FRAME_TYPE) {
```

它会读取 merge event 里面包含多少个 sub-frame：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/GUser.C:1281
```

```cpp
Inceptionnbinsideframe[InceptionLayer] =
    pInceptionMergeFrame[InceptionLayer]->GetNbItems();
```

然后逐个读出内部 frame，并递归 `Unpack()`：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/GUser.C:1293
```

```cpp
for (int i = 0; i < Inceptionnbinsideframe[InceptionLayer]; i++) {
    pInceptionMergeFrame[InceptionLayer]->ReadInFrame(...);
    Unpack(fInceptionInsideframe[InceptionLayer],debug);
}
```

这一步是理解 event 的关键：**ADNE 把一个 `MFM_MERGE_TS` 作为一个 event，然后展开其中所有 detector sub-frame**。

## Exogam2 Handling Inside One Event

当 sub-frame 是 Exogam2 frame 时：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/GUser.C:1304
```

```cpp
else if(frame->GetFrameType() == MFM_EXO2_FRAME_TYPE && frame->GetDataSource()==0) {
```

ADNE 调用：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/GUser.C:1310
```

```cpp
Exogam2b = fExogam2->IsMFMExo(pExogamFrame);
```

`TExogam2::IsMFMExo()` 负责将 Exogam2 raw hit 解包到 `TExogam2Data`，包括 crystal 编号、能量、TDC、BGO、CsI 和 timestamp：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/TExogam2.cxx:929
```

```cpp
fExogam2Data->SetECCEClover(clo);
fExogam2Data->SetECCECristal(cri);
fExogam2Data->SetECCEDetNbr(MapFinger);
fExogam2Data->SetECCEEnergy(valf);
...
fExogam2Data->SetESSTQBGO(frame->ExoGetBGO());
fExogam2Data->SetESSTQCSI(frame->ExoGetCsi());
fExogam2Data->SetfTimeStamps(frame->GetTimeStamp(),MapFinger);
```

随后，`TExogam2::Treat()` 在同一个 event 内做物理筛选和重建：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/TExogam2.cxx:1349
```

其中 prompt gate 逻辑在：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/TExogam2.cxx:1374
```

single gamma branch 写入在：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/TExogam2.cxx:1468
```

addback gamma branch 写入在：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/TExogam2.cxx:1565
```

因此：

- `fEXO`：merge event 内解包出来的原始 Exogam2 crystal 能量；
- `sEXO`：经过 prompt gate 和 anti-Compton 条件后的 single crystal gamma；
- `aEXO`：同一个 clover 内对通过条件的 single gamma 做 addback 后的 gamma。

这些步骤都发生在已经 build 好的 event 内部，不改变 event 的边界。

## Event Width: About 80 Tick

对当前数据文件做只读抽样检查时，前 `1,000,000` 个顶层 merge event 中，内部 Exogam2 sub-frame 的 timestamp 最大跨度为：

```text
max(TS) - min(TS) = 80 tick
```

ADNE 代码中 rate monitor 的写法显示 timestamp tick 约为：

```text
1e8 tick / second
```

对应源码：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/GUser.C:1317
```

所以：

```text
1 tick = 10 ns
80 tick = 800 ns
```

当前最合理的工作结论是：

```text
当前数据的 Exogam2 MFM event build 时间宽度约为 80 tick，即约 800 ns。
```

但需要注意：这是从当前数据结构中反推出来的结果。要严格确认当时 event builder 的配置参数，需要找到生成该 `.dat` 文件时的真实 DAQ / Narval / actor 配置或运行日志。

## Where EventBuilder / EventMerger Are Mentioned

本地 ADNE 目录中提到了 `EventBuilder` 和 `EventMerger`，主要在配置模板里。

`EventBuilder` 配置：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/Utils/zUseful/gen_conf.py:207
```

```text
ActualClass           EventBuilder
BuilderType           TimeStamp 50
KeyIn                 data:psa
KeyOut                event:data:psa
MinFold               1
```

`EventMerger` 配置：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/Utils/zUseful/gen_conf.py:220
```

```text
ActualClass           EventMerger
BuilderType           TimeStamp 114 99
keyIn                 data:ranc1
KeyIn                 event:data:psa
keyOut                event:data
MinFold               2
```

拓扑文件中也有 builder 链条：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/Utils/zUseful/TopologyTotal.conf:16
```

```text
Builder       EventBuilder 500000
Dispatcher    EventMerger
```

以及：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/Utils/zUseful/TopologyTotal.conf:20
```

```text
Builder       EventMerger 1000000
```

`TopologyGlobal.conf` 中也有类似结构：

```text
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/Utils/zUseful/TopologyGlobal.conf:18
/home/user0/work/IJCLAB/NFS/pkg/adne-data-analysis-framework/Utils/zUseful/TopologyGlobal.conf:22
```

但是，在当前本地 `install/GRU` 和 `install/MFMlib` 中没有找到 `EventBuilder::` 或 `EventMerger::` 的 C++ 实现。因此，当前证据支持如下判断：

```text
EventBuilder / EventMerger 是上游 DAQ / Narval / AGATA 采集链中的 actor/class，
用于把 detector frames 按 timestamp 或 event number 合并为 MFM merge event。

ADNE 本身只是读取这些已经生成好的 MFM merge event。
```

## Practical Interpretation For This Analysis

后续如果基于 ADNE 输出的 ROOT tree 做分析，需要记住：

1. 一条 `TreeMaster` entry 对应 ADNE 读取的一个 MFM merge event。
2. 这个 merge event 的边界来自上游 timestamp event builder。
3. 当前数据看起来 event 内 Exogam2 timestamp 跨度约为 `80 tick = 800 ns`。
4. ADNE 的 `prompt gate`、`anti-Compton`、`sEXO`、`aEXO` 是 event 内部筛选和重建，不是 event build。
5. 空 event 可能来自以下情况：
   - merge event 中没有有效 Exogam2 gamma deposit；
   - Exogam2 hit 被 prompt gate 排除；
   - Exogam2 hit 被 BGO / CsI anti-Compton 条件 veto；
   - event 中只有 BGO / CsI / 其他探测器响应；
   - event 本身确实为空或不包含本分析关心的 detector hit。

## Open Checks

为了把结论从“数据反推”变成“配置确认”，后续最好继续寻找：

- 生成该 `.dat` 文件时真实使用的 `EventBuilder` / `EventMerger` 配置；
- DAQ / Narval 运行日志；
- 是否存在与该 run 对应的 topology 文件；
- `BuilderType TimeStamp ...` 中参数和当前 `80 tick` 结果之间的精确关系。

