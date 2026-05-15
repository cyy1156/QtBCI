# QtBCI 完整开发手册

**版本**：V1.0.1

**适用范围**：桌面端 Qt BCI 采集应用（仅 C++/Qt）

**更新日期**：2026-05-02（工程结构、类名与依赖说明已于 2026-05-13 按当前源码校对增补；**2026-05-14** 增补：采样率默认固定 215 Hz、在线估计默认关闭说明、**CSV 表头专题**、**UDP 二进制协议专题**。下文历史段落均保留。）

## 目录

- 项目总览
- 硬件与通信协议（核心）
- 工程结构与依赖
- 构建与环境配置
- 运行时架构：四线程模型
- 核心模块详解
- 数据流转全流程
- UI 与可视化
- 预处理与算法
- 日志与 CSV 存储
- **专题：CSV 各输出文件表头与列语义**
- **专题：UDP 网络二进制协议（PBC1 / PBF1 / PBP1）**
- 二次开发指南
- 测试与调试
- 附录：协议、术语、参数

## 1 项目总览

1.1 项目目标

QtBCI 是面向单通道 ThinkGear 兼容脑电设备的实时采集、解析、预处理、可视化与存储系统，支持：

- 串口 / 蓝牙透传接收脑电原始数据、信号质量、专注度 / 放松度、8 频段 EEG Power
- 可选 UDP 遥测：预处理分窗（**PBC1**）、FFT（**PBF1**）、PSD（**PBP1**）二进制报文发往本机或远端（详见本文 **「专题：UDP 网络二进制协议」** 与 `docs/UDP接收端Python示例.md`）
- 多线程解耦：采集、算法、日志、UI 互不阻塞
- 原始值 → 微伏（μV）换算、50Hz 陷波、0.5~40Hz 带通、伪迹标记
- 实时时域 / FFT/PSD 绘图
- 按样本序号合并原始数据与预处理结果，异步写入 CSV

### 1.2 核心能力

- 小包解析：约 214包/秒量级（设备侧），应用层默认按 **固定标称采样率 215 Hz** 做滤波与频谱（见 §9.1）
- 大包解析：信号强度、Attention、Meditation、Delta/Theta/Alpha/Beta/Gamma 共 8 个频段功率
- **默认不再启用**基于墙钟的在线采样率平滑估计；源码中仍保留可选分支（`use_fixed_nominal_fs = false` 时），便于你自测对比
- 伪迹检测、轨道饱和检测、信号质量分级
- 三线程安全架构，支持长时间采集不掉帧、不丢盘

### 1.3 技术栈

- Qt 6.5+（主工程 `find_package` 依赖：Core、Widgets、SerialPort、PrintSupport、**Network**）
- C++17
- CMake ≥3.19
- QCustomPlot 绘图
- BrainFlow：链接 **DataHandler** 预编译库；源码中滤波/伪迹等 API 仍通过 `preprocessing/data_filter.h` 内的 **`DataFilter`** 包装类调用（与 BrainFlow 命名一致）
- 主程序串口协议栈为自研 **`device/serial_eeg/`**（不依赖厂商 ThinkGear DLL）；若需对照官方 SDK，见 **`tests/legacy_thinkgear/`**（与主工程 CMake 无链接关系）

## 2 硬件与通信协议

### 2.1 设备基本参数

- 通信：蓝牙 2.0 透传 / 串口
- 波特率：57600
- 数据位：8、无校验、1 停止位、无流控
- ADC：12 位
- 标称采样率（应用/预处理约定）：**215 Hz**（`EegPreprocessPipeline::Options::fs_nominal`，与 `mainwindow` 内 CSV 时间对齐常量一致）；设备文档中常见「约 200 Hz / 512 包每秒」为概略表述，**以工程内 215 Hz 为准**
- 滤波：硬件 50Hz 陷波
- 电极：额头参考电极 + 屏蔽层

### 2.2 数据包格式

设备输出小包（原始数据）与大包（合成参数）两种帧。

#### 2.2.1 小包（原始数据帧）

- 固定头：AA AA 04 80 02
- 结构：AA AA 04 80 02 xxHigh xxLow xxCheckSum（**CRC16**）
- 长度：固定 8 字节
- 频率：约 214 包 / 秒
- 有效数据：1 个原始采样点 rawdata

#### 2.2.2 大包（因为认为改硬件解析大包的数据不准确，所以只用了里面的signal值，后续需要使用大包数据可以在Steam sdk for pc的文件夹下寻找；**当前仓库中对应代码位于** `tests/legacy_thinkgear/stream_sdk_for_pc/`）

- 固定头：AA AA 20 ...
- 长度：32 字节负载 + 头 + 校验
- 频率：每 214个小包出现 1 次大包（所以采样率是215hz）
- 包含：Signal、8 组 EEG Power、Attention、Meditation、校验和

### 2.3 校验和算法（必须先校验再解析）

sum = ((0x80 + 0x02 + xxHigh + xxLow) ^ 0xFFFFFFFF) & 0xFF

对 04 之后 4 字节求和 → 按位取反 → 取低 8 位

与包尾 xxCheckSum 相等才有效，否则丢弃

丢包率 <10% 不影响结果

### 2.4 原始数据解析

rawdata = (xxHigh << 8) | xxLow;

if(rawdata > 32768) rawdata = 65536;

得到 int16 原始值，后续转为 μV。

### 2.5 大包字段顺序（固定）

AA AA 20 02 Signal 83 18 \[Delta(3B) Theta(3B) LowAlpha(3B) HighAlpha(3B) LowBeta(3B) HighBeta(3B) LowGamma(3B) MiddleGamma(3B)\] 04 Attention 05 Meditation CheckSum

#### 2.5.1 EEG Power 计算

每 3 字节一组：value = (B1 << 16) | (B2 << 8) | B3

无符号、无单位，仅用于相对比较。

### 2.6 信号质量 Signal 含义

- 0~20：优秀
- 21~50：一般
- 51~100：较差
- 101~200：很差
- 200：电极未接触 / 无人佩戴
- 255是初始值

## 3 工程结构

### 3.1 当前仓库布局（与 `CMakeLists.txt` 中 `qt_add_executable` 源文件列表一致）

以下树状描述的是**本 README 同目录**下的主工程（即打开构建所用的 `QtBCI/QtBCI/` 文件夹）。`build/` 为本地生成目录，未列入。

```
QtBCI/                          # 主工程根（CMakeLists.txt 所在目录）
├── main.cpp
├── mainwindow.*
├── chartmodedialog.*           # 图表模式
├── serialconfigdialog.*        # 串口配置
├── saveconfigdialog.*          # CSV/路径等保存选项
├── core/                       # 采集 / 算法 / 日志 / 环形缓冲 / 特征 / 设置
│   ├── acquisitionengine.* / algorithmengine.* / csvlogworker.*
│   ├── pictureandalgbuffer.* / logwbuffer.*
│   ├── psdfeatureextractor.* / fftfeatureextractor.*
│   └── app_settings.h
├── device/
│   ├── serialport.*            # QSerialPort 封装
│   └── serial_eeg/             # 0xAA 同步帧：组帧、TLV 解析、Raw→μV（无厂商 DLL）
│       ├── eeg_frame_assembler.*
│       ├── eeg_payload_parser.*
│       └── raw_to_uv_processor.*
├── preprocessing/
│   ├── data_filter.*           # BrainFlow 风格 DataFilter 包装
│   └── eeg_preprocess_pipeline.*  # 默认 fs_nominal=215、use_fixed_nominal_fs=true
├── Net/                        # UDP：NetworkStreamSettings、UdpTelemetryHub、网络配置对话框
├── QCustomPlot/
├── openbci/                    # 推荐：DataHandler.lib/.dll 与 brainflow_headers（CMake 优先）
├── docs/                       # 设计说明、UDP Python 示例、排障笔记等
├── docs/examples/              # 如 preproc_udp_sender（独立示例源码）
└── tests/legacy_thinkgear/     # 旧版单线程与 stream_sdk 实验代码，不参与主程序链接
    ├── thinkgearlinktester.*
    └── stream_sdk_for_pc/
```



### 3.2 早期 README 中的目录示意（原文保留，便于对照；类与文件夹已迁移，请以 3.1 为准）

QtBCI/

├── QtBCI/ # Qt 主工程

│ ├── main.cpp

│ ├── mainwindow.\*

│ ├── device/ # 串口封装

│ ├── thinkgear/ # 组帧、解析、Raw→μV

│ ├── core/ # 采集 / 算法 / 日志引擎

│ ├── preprocessing/ # 滤波、伪迹；采样率默认固定标称 215Hz（可选在线估计分支保留在代码中）

│ ├── QCustomPlot/

│ ├── stream_sdk_for_pc/ # 官方 SDK 测试代码

│ └── docs/ # 专题排障文档

└── 开发手册.md

## 4 构建配置

### 4.1 必需环境

- Qt 6.5+
- CMake ≥3.19
- C++17
- MinGW 或 MSVC 2019+

### 4.2 依赖文件（与当前 `CMakeLists.txt` 一致）

- **BrainFlow DataHandler 预编译库**：`DataHandler.lib` + `DataHandler.dll`  
  CMake 按顺序查找：`openbci/lib/mingw_64/`、`openbci/lib/`、`preprocessing/`（文件名亦兼容 `data_handler.*`）。
- **BrainFlow 头文件**：`brainflow_array.h`（utils）、`data_handler.h`（data_handler）  
  优先：`openbci/brainflow_headers/utils_inc` 与 `openbci/brainflow_headers/data_handler_inc`；否则尝试与工程平级的 `brainflow-master/.../src/...`（详见 CMake 内注释）。
- **QCustomPlot**（工程内 `QCustomPlot/` 子目录源码，随主目标编译）。
- **Qt 模块**：Core、Widgets、SerialPort、PrintSupport、**Network**（UDP）。
- MinGW 下 QCustomPlot 需 **big-obj**：`-Wa,-mbig-obj`（CMake 已配置）。

**（以下为早期 README 依赖表述，主程序已不再链接厂商 DLL；对照官方 API 时仍可参考 `tests/legacy_thinkgear/`）**

- ThinkGear SDK（必须 x64）

- thinkgear.h
- thinkgear64.lib / thinkgear64.dll

- BrainFlow DataFilter

- DataFilter.lib / DataFilter.dll

### 4.3 MinGW 特殊设置

QCustomPlot 需添加编译选项：-Wa,-mbig-obj

### 4.4 构建步骤

1.  打开 QtBCI/CMakeLists.txt
2.  配置 Kit（MSVC2019 x64 或 MinGW x64）
3.  确保 .lib/.dll 路径正确
4.  构建 → 运行 QtBCI.exe

## 5\. 线程分工

## 5.1

| **线程** | **类** | **职责** |
| --- | --- | --- |
| 采集线程 | AcquisitionEngine | 串口读、组帧、解析、Raw→μV、发送 RawPacket |
| 算法线程 | AlgorithmEngine | 分窗、预处理、FFT/PSD、伪迹标记 |
| 日志线程 | CsvLogWorker | 合并 raw/preproc、写 CSV、不阻塞主线程 |
| UI 线程 | MainWindow | 绘图、按钮、日志展示、配置 |

## 5.2 跨线程通信规则

- 全部使用 Qt::QueuedConnection
- 自定义结构体必须注册：qRegisterMetaType&lt;RawPacket&gt;("RawPacket");
- 禁止在工作线程直接操作 UI 控件

### 5.3 数据流总图

- 串口 → 组帧 → 解析 → Raw→μV → 采集线程发 RawPacket→ 算法线程（分窗 + 预处理 + 特征）→ UI 绘图 / 日志缓冲→ 日志线程合并写入 CSV
- 可选：`AlgorithmEngine` 产出的 `PlotChunk` / `FftResult` / `SpectrumResult` 经 **`UdpTelemetryHub`**（`Net/`）按 `NetworkStreamSettings` 发往 UDP（默认端口 50001/50002/50003，可在网络配置对话框中修改）；协议与 Python 接收示例见 `docs/UDP接收端Python示例.md` 等

## 6 核心模块详解

### 6.1 SerialPort（设备层）

- 配置：COMx、57600、8N1、无流控
- 接口：openReadOnly、close、readAll

信号：dataReceived (chunk)

### 6.2 SerialEegFrameAssembler（组帧层，源码 `device/serial_eeg/eeg_frame_assembler.*`）

- 类名历史对照：早期文档中的 **ThinkGearFrameAssembler** 职责与此相同。
- 缓存：m_rxBuffer（上限 64KB）
- 同步头：查找 AA AA
- 按长度截取完整帧
- 校验通过后发出 `frameReady(QByteArray)`；`AcquisitionEngine` 在槽 **`onAssemblerFrame`** 内再交给解析器（避免解析器直接绑在组帧信号上的额外校验与元数据）

**6.3 SerialEegPayloadParser（解析层，`device/serial_eeg/eeg_payload_parser.*`）**

- 类名历史对照：**ThinkGearPayloadParser**
- 解析小包：rawdata、校验
- 解析大包：Signal、Attention、Meditation、8 频段功率
- 信号：`rawReceived`、`signalQualityReceived`、`eegPowerReceived`（及 `attentionReceived`、`meditationReceived`、`blinkReceived`、`parseWarning` 等）

**6.4 RawToUvProcessor（单位换算，`device/serial_eeg/raw_to_uv_processor.*`）**

- 类名历史对照：**RawtOutUvProcessor**（拼写修正）
- 将 int16 原始值转为微伏（μV）；可配置 `uvPerCount`、`gainAdjust` 等

**6.5 AcquisitionEngine（采集引擎）**

- 封装串口→组帧→解析→换算全链路，输出 **`RawPacket`**（见 `core/acquisitionengine.h`），当前字段包括：
- `QString tsMs`；`qint64 wallMs`；`quint64 seq`；`quint64 gapSincePrev`；`qint16 rawInt16`；`quint8 signalQuality`；`double rawUv`
- 另有链路诊断信号：`streamGapDetected`、`linkDiagnostic`（与序号缺口、组帧事件时刻对齐）

**6.6 AlgorithmEngine（算法引擎）**

- 分窗：128 点 / 窗
- 预处理管线 **`EegPreprocessPipeline`**：默认使用**固定标称**采样率 **215 Hz** 参与陷波/带通/FFT/PSD（**不**走墙钟在线估计主路径）
- 输出 PlotChunk、AlgoResult、SpectrumResult

**6.**7 EegPreprocessPipeline（预处理管线）

1.  **采样率**：默认每帧将内部 `fs` 置为标称 **215 Hz**（`use_fixed_nominal_fs = true`）；若你在代码中改为 `use_fixed_nominal_fs = false`，才使用 `FsHint` + `updateFsEstimate` 的在线估计（限幅默认约 200~230 Hz，见头文件）
2.  50Hz 陷波
3.  0.5~40Hz 带通
4.  伪迹标记（峰峰值 / 饱和率 / 最大幅度）
5.  可选降采样、小波去噪

**6.8 LogBuffer / CsvLogWorker**

- LogBuffer：线程安全队列
- CsvLogWorker：按 seq 合并 raw 与 preproc，定时 flush 到 CSV
- 合并延迟 `m_mergeLag`：代码默认 **256**（文档中若仍见 512 为旧默认）；容忍预处理滞后后再落盘

**6.9 网络与 UDP（`Net/`）**

- **`NetworkStreamSettings`**：是否发送、目标地址、各通道端口、PBC1/PBF1/PBP1 开关等持久化配置
- **`UdpTelemetryHub`**：单一 `QUdpSocket`，将预处理分窗、FFT、PSD 结果发往配置的主机/端口
- **`NetworkConfigDialog`**：UI 配置网络推流
- **二进制帧格式**：见本文 **「专题：UDP 网络二进制协议（PBC1 / PBF1 / PBP1）」** 与 `docs/UDP接收端Python示例.md`

**7 数据流转全流程**

1.  串口收到字节块 → chunk
2.  组帧层找 AA AA → 截帧 → 校验
3.  解析层拆出 RawInt16 / Signal / Attention / Meditation / EEG Power
4.  Raw → μV
5.  封装 RawPacket 发往算法线程与 UI
6.  算法线程分窗 128 点
7.  **默认**以固定标称 **fs = 215 Hz** 调用预处理与特征（不设「每窗在线重估 fs」步骤；在线估计算法仅在可选分支启用）
8.  陷波 → 带通 → 伪迹标记
9.  发 PlotChunk 给 UI 绘图
10. UI 把 Raw + Preproc 写入 LogBuffer
11. 日志线程按 seq 合并 → 写入 CSV
12. （可选）算法线程或 UI 侧将特征经 `UdpTelemetryHub` 以 UDP 发出，供外部 Python / 推理服务消费

**8 UI 与可视化**

**8.1 三种图表模式**

- RawTime：原始 / 预处理时域波形
- FftSpectrum：FFT 频谱
- BandPower：PSD 频段功率

**8.2 绘图策略**

- 30ms 定时器统一刷新
- 缓存 m_plotCache，避免频繁重绘
- 固定 Y 轴量程便于观察

**8.3 日志区域**

- 展示帧错误、校验错、串口状态、算法结果
- 与 CSV 文件双记录

**9 预处理与算法（核心）**

**9.1 采样率策略（当前默认：固定标称 215 Hz；在线估计默认关闭）**

- **`EegPreprocessPipeline::Options`**：`fs_nominal = 215.0`，**`use_fixed_nominal_fs = true`（默认）**。此时 `process()` 每帧将内部平滑采样率 **`m_fs_smooth` 直接置为标称值**，BrainFlow 滤波与后续 `fs_used` 均按 **215 Hz（取整后）** 执行，**不调用**基于 `FsHint`（seq + 墙钟毫秒）的 **`updateFsEstimate`** 在线分支。
- **在线采样率估计算法**：仍以 **`updateFsEstimate`** 等形式保留在 `eeg_preprocess_pipeline.cpp` 中；仅当将 **`use_fixed_nominal_fs` 设为 `false`** 时才会根据相邻 hint 计算 `fs_inst`、限幅（默认约 **200~230 Hz**，见头文件 `fs_min`/`fs_max`）并指数平滑。—— **产品默认配置下，相当于「去除」了在线估计对结果的影响**；若你文档其它章节仍大段描述在线估计为主路径，视为历史叙述，以本小节为准。
- **`mainwindow.cpp`** 等处 CSV 时间对齐使用同一标称（如 **`kDeviceNominalFs = 215.0`**）。

**9.1（附录）旧版文档：在线采样率估计公式（代码保留分支；默认 `use_fixed_nominal_fs=true` 时不生效）**

- fs_inst = dseq \* 1000 /dtmsfs_smooth = alpha\*fs_smooth + (1-alpha)\*fs_instalpha=0.9限幅：在可选在线模式下为约 **200~230 Hz**（以 `eeg_preprocess_pipeline.h` 为准，旧 README 曾写 200~300）

**9.2 预处理步骤**

1.  **采样率**：默认固定为标称 **215 Hz**；仅当关闭 `use_fixed_nominal_fs` 时才「采样率更新（在线估计）」
2.  50Hz 陷波（工频）
3.  0.5~40Hz 带通
4.  伪迹标记（ptp、absmax、饱和率）
5.  可选降采样 / 小波去噪（对于多通道设备可以在预处理的配置结构体里面勾选）

**9.3QtBCI 预处理伪迹判定与 RMS 计算说明**

一、核心说明

- 追溯预处理中 artifact_marked 的判定逻辑及 RMS 的计算方式。这里其实没有一套「0～100 分」的评级表，而是两类不同性质的东西：连续指标 RMS 和 二分类伪迹规则。

二、AlgoResult.score（RMS）—— 不是 “及格线”，是描述量

- 对已经预处理完的序列 y（长度 = 当前窗口采样点数），计算公式如下：

- 含义：**波动越大值越大**
- 这一窗信号在 μV 意义下的有效幅值（能量相关）。
- 没有在工程里写「RMS 大于多少算好、小于多少算差」—— 它只作为数值给你看或给日志用；若要做自己的 “评分”，需要你自己定阈值或归一化。

三、AlgoResult.label（clean /artifact）—— 明确的判定标准

- label 完全由 EegPreprocessPipeline::process 里的 artifact_marked 决定：artifact ↔ 标记为伪迹clean ↔ 未标记
- 伪迹标记在 enable_artifact_mark == true（默认开启）时，对滤波后的 out.y 计算三个指标，满足任意一条就把该窗标成伪迹（mark = true）。
- 伪迹判定条件（默认阈值）

1.  峰峰值 ptp_uv > 300 μV含义：窗口内 max (y) - min (y)
2.  最大绝对幅值 absmax_uv > 180 μV含义：窗口内 max|yi|
3.  饱和比例 railed_ratio > 0.30含义：由 DataFilter::get_railed_percentage 计算，gain 默认 1
4.  对应代码实现
5.  默认参数在头文件 Options 里：bool enable_artifact_mark = true;double artifact_ptp_uv = 300.0; // 峰峰值阈值double artifact_absmax_uv = 180.0; // 绝对振幅阈值double artifact_railed_ratio = 0.30; // 饱和比例阈值int artifact_railed_gain = 1;

AlgorithmEngine 里只是把结果映射成字符串：artifact_marked == true → label = "artifact"否则 → label = "clean"

四、小结（评分标准一句话）

- score：没有好坏标准，只是当前窗预处理波形的 RMS（μV 量级）。label：有明确规则 —— 在默认参数下，该窗滤波后若 峰峰值 > 300 μV 或 最大绝对值 > 180 μV 或 饱和比例 > 30% 之一成立，则判为 artifact，否则 clean。
- 若要改「严 / 松」，应改 EegPreprocessPipeline::Options 里上述四个字段（或通过 setOptions 注入），而不是改 AlgoResult 本身

**10 日志与 CSV 存储**

**10.1 CSV 列（合并后的主 EEG 文件，表头以代码为准）**

- 当前表头：`tsMs,seq,rawInt16,signalQuality,rawUv,preprocUv,gapSincePrev`（见 `CsvLogWorker::writeHeaderIfNeeded`）。**各列含义见下文「专题：CSV 各输出文件表头与列语义」。**

**10.2 写入机制**

- 异步定时 flush（默认几十 ms）
- 按 seq 合并原始与预处理数据
- stop () 时强制 flush 所有剩余数据

### 专题：CSV 各输出文件表头与列语义

本专题与 `core/csvlogworker.cpp` / `core/csvlogworker.h` 保持一致，便于对账与二次分析。

**一、主 EEG CSV（`CsvLogWorker` 写入的合并文件，如 `raw_*.csv`）**

| 表头列 | 含义 |
| --- | --- |
| `tsMs` | 文本时间戳（建议格式 `yyyy-MM-dd HH:mm:ss.zzz`），以原始样本侧为准合并 |
| `seq` | 样本序号，用于 raw 与 preproc 对齐 |
| `rawInt16` | 设备原始整型 |
| `signalQuality` | 信号质量字节（0~200 等含义见 §2.6） |
| `rawUv` | 换算后微伏（无数据时可能空） |
| `preprocUv` | 该 seq 上预处理后的微伏（无数据时可能空） |
| `gapSincePrev` | **相对上一已发出样本**缺失的点数；连续流上为 **0** 时常留空。用于**逐样本**观察缺口，**不是**「整秒是否有丢包」标记 |

**二、PSD 结果 CSV（可选单独文件）**

- 表头：`tsMs,seqStart,seqEnd,fsUsed,rms,delta,theta,alpha,beta,gamma,alphaBetaRatio,label,secLoss`
- **`secLoss`**：**0/1**。该窗对应时间所在 **UTC 整秒** 是否曾被标记为「该秒出现过 streamGap / 链路诊断 / LogBuffer 丢弃增加」等（见 `CsvLogWorker::secLossMarkForRow`）。**1** 表示该秒内有相关事件，**0** 表示否。

**三、FFT 结果 CSV（可选单独文件）**

- 表头：`tsMs,seqStart,seqEnd,fsUsed,nfft,rms,delta,theta,alpha,beta,gamma,label,secLoss`
- **`secLoss`**：同上，与 PSD 文件语义一致。

**四、与主 EEG CSV 的区别**

- 主 EEG 行是**逐样本**合并；**`secLoss` 仅出现在 PSD/FFT 两类特征 CSV**，不出现在主 `raw_*.csv` 表头中。

### 专题：UDP 网络二进制协议（PBC1 / PBF1 / PBP1）

本专题描述 **`Net/udptelemetryhub.cpp`** 发出的 **UDP 单报文二进制帧**（一帧一次 `writeDatagram`）。**完整字段偏移、Python `struct` 解析示例**以仓库内 **`docs/UDP接收端Python示例.md`** 为权威长文档；此处为速查。

**一、传输与配置**

| 项目 | 说明 |
| --- | --- |
| 传输 | **UDP**，IPv4；目标主机与端口由 **`NetworkStreamSettings`** / `QtBCI.ini`（`network/host`、`network/port_*`）及界面 **网络设置** 决定 |
| 默认端口 | **50001** 预处理块 **PBC1**，**50002** FFT **PBF1**，**50003** PSD **PBP1**（三流分端口，避免混包） |
| 开关 | 需勾选启用网络发送，并分别勾选发送预处理 / FFT / PSD |
| 字节序 | 整数均为 **小端 LE**；`double` 为 **IEEE754 双精度**（与 x86/x64 小端布局一致，见 Python 文档） |

**二、公共 8 字节头**

| 偏移 | 长度 | 内容 |
| --- | --- | --- |
| 0 | 4 | ASCII 魔数：`PBC1` / `PBF1` / `PBP1` |
| 4 | 4 | `uint32` LE，版本号，当前为 **1** |

**三、PBC1（可变长，`44 + n×8` 字节）**

- `seq_start`、`seq_end`（`uint64` LE）、`anchor_wall_ms`（`int64` LE）、`anchor_seq`（`uint64` LE）、样本数 `n`（`uint32` LE），随后 **`n` 个 double** 为预处理后的单通道波形。

**四、PBF1（固定 93 字节）**

- `seq_start`/`seq_end`（`uint64`）、`wall_ms`（`int64`）、`fs_used`（`double`）、`nfft`（`int32`）、频段能量 **delta…gamma** 与 **rms**（各 `double`），末字节 **`label`**：`0`=clean，`1`=artifact（与 Qt 侧 `label` 字符串是否含 artifact 一致）。

**五、PBP1（固定 101 字节）**

- 头部字段与 PBF1 类似；偏移 40 处 **`int32` 预留当前为 0**；多一个 **`alpha_beta_ratio`（`double`）**；末字节 **`label`** 同 PBF1。

**六、延伸阅读**

- `docs/UDP接收端Python示例.md`：接收循环与解析函数
- `docs/网络设置与多路UDP发送设计.md`：界面与 ini 键说明

**11 测试与调试**

**11.1 无数据 / 连不上**

- 确认 COM 号、57600、8N1
- 确认蓝牙已连接、设备开机
- 查看日志：帧错误、校验错、信号质量 = 200

**11.2 CSV 只有表头**

- 检查日志线程是否启动
- 检查 flush 定时器是否运行
- 检查 LogBuffer 是否被塞满丢弃

**11.3 绘图卡顿**

- 增大 plotCache 长度上限
- 降低 UI 刷新频率
- 关闭不必要的频谱计算

**11.4 采样率 / 滤波刻度异常（排查）**

- **默认**：全流程按 **固定标称 215 Hz**；若你实测设备点率与 215 偏差很大，应调整 **`fs_nominal`** 或启用 **`use_fixed_nominal_fs = false`** 并相应调整 **`fs_min`/`fs_max`**，否则陷波/带通/PSD 频率刻度会偏。
- 若仍启用在线估计且 `fs_inst` 抖动大：可提高 **`fs_alpha`**、调整 **`fs_min`/`fs_max`**、检查 **`FsHint`** 时间戳与序号是否单调、检查系统时钟与串口延迟。

**12 附录**

**12.1 完整协议速查表**

- 小包头：AA AA 04 80 02
- 大包头：AA AA 20
- 校验：sum = (字节和取反) & 0xFF
- Raw：(H&lt;<8)|L，&gt;32768→65536
- EEG Power：3 字节大端拼合

**12.2 关键默认参数**

- 分窗：128 点
- **预处理采样率（默认）**：固定标称 **215 Hz**（`fs_nominal`，`use_fixed_nominal_fs = true`）；在线估计限幅约 **200~230 Hz**（仅 `use_fixed_nominal_fs = false` 时使用，以 `eeg_preprocess_pipeline.h` 为准）
- 在线估计平滑系数：`fs_alpha = 0.9`（同上，仅在线分支）
- 合并延迟：256 点（`CsvLogWorker::m_mergeLag`，旧文档曾写 512）
- 带通：0.5~40Hz
- 陷波：50Hz

**12.3 术语**

- BCI：脑机接口
- EEG：脑电波
- PSD：功率谱密度
- FFT：快速傅里叶变换
- RawInt16：设备原始值
- μV：微伏（标准单位）

**13.二次开发步骤**

**一、拿到代码后先看目录（2026-05 与 `CMakeLists.txt` 对齐的摘要）**

- 打开 **`QtBCI/QtBCI/CMakeLists.txt`**（内层带 `main.cpp` 的主工程，不是仅含解决方案的上一层）。
- 串口协议实现：**`device/serial_eeg/`**（组帧 / 解析 / μV 换算）；**不再**要求主工程目录下存在 `thinkgear/` 或链接 `thinkgear64.dll`。
- BrainFlow：**`DataHandler.lib` + `DataHandler.dll`**，推荐放在 **`openbci/lib/...`**；头文件推荐 **`openbci/brainflow_headers/...`**（详见 CMake 中 `OPENBCI_VENDOR_ROOT` 与候选路径）。
- 旧版单线程与官方 SDK 对照代码：**`tests/legacy_thinkgear/`**（可选，不参与主程序链接）。

**（以下为原 README 本小节的连续表述，未删改，仅上方面条为新增摘要）**

- 应能看到类似结构：QtBCI/CMakeLists.txt ← 要打开的是这一份（内层带 main.cpp 的那个 QtBCI 文件夹里的 CMake，不是随便上一层）。QtBCI/thinkgear/（需自备 thinkgear.h 和 thinkgear64.lib/thinkgear64.dll）QtBCI/preprocessing/（需自备 DataHandler.lib/ DataHandler.dll）
- 本机还要有 BrainFlow 源码树里的头文件路径（CMake 里写死了两种相对路径候选，见下文）。缺库时 CMake 配置阶段就会报错，先按错误提示把文件放对位置再编译。

**二、安装开发环境（Windows 为例）**

- 安装 Qt 6.5 及以上（与工程 find_package (Qt6 6.5 REQUIRED ...) 版本一致）。安装时勾选：MinGW 64-bit 或 MSVC 2019/2022 64-bit（和后面选的 Kit 一致即可），并勾选：Qt SerialPort、**Qt Network**（UDP）、Qt 5/6 的 CMake 相关组件（Qt Creator 一般会带上）。
- 安装 CMake（≥ 3.19；使用 Qt 自带的也可以）。（可选）安装 Git，便于克隆 / 更新仓库。

**三、用 Qt Creator 打开并编译（最省事）**

1.  打开 Qt Creator。
2.  文件 → 打开文件或项目，选择：… 你的路径…/QtBCI/QtBCI/CMakeLists.txt
3.  向导里选一个 Kit（例如 Desktop Qt 6.x.x MinGW 64-bit），点 Configure Project。
4.  等待 CMake 配置成功（左下角没有红色错误）。若失败，常见原因：
    1.  找不到 **DataHandler.lib / DataHandler.dll**（按 CMake 提示放入 `openbci/lib/` 或 `preprocessing/`）
    2.  找不到 BrainFlow 的 **brainflow_array.h**、**data_handler.h**（放入 `openbci/brainflow_headers/...` 或按 CMake 注释放置 `brainflow-master` 源码树）
    3.  （**仅当**你自行改回链接厂商库时）找不到 thinkgear.h / thinkgear64.lib / thinkgear64.dll —— **当前默认主工程不依赖此项**

**（原 README 列出的典型错误枚举保留如下）**

    1.  找不到 thinkgear.h/thinkgear64.lib/thinkgear64.dll
    2.  找不到 DataHandler.lib/ DataHandler.dll
    3.  找不到 BrainFlow 的 brainflow_array.h、data_handler.h→ 解决方法：把厂商库放到 CMakeLists.txt 里写明的目录，或把 brainflow 源码放到与 QtBCI 平级的约定路径，或修改 CMake 里的 include 路径（二次开发可接受）。
5.  左下角点锤子（构建），再点运行。
6.  构建成功后，**至少** `DataHandler.dll` 会由 **POST_BUILD** 拷贝到 exe 同目录。原 README 中「thinkgear64.dll 与 DataHandler.dll 一并拷贝」适用于旧工程；**当前默认主程序不链接 thinkgear64**，若仅运行主程序则无需该 DLL；若运行 `tests/legacy_thinkgear` 下示例，再按需放置 `thinkgear64.dll`。

**四、用命令行编译（给 CI 或不用 Creator 的人）**

在 QtBCI/QtBCI 目录下（即 CMakeLists.txt 所在目录）执行，并把 CMAKE_PREFIX_PATH 换成本机 Qt 安装路径。若用 Visual Studio 生成器，把 -G "Ninja" 改成 -G "Visual Studio 17 2022" -A x64 等即可。

五、BrainFlow 头文件放哪（否则 CMake 不过）

**（当前 CMake 优先）**：`QtBCI/openbci/brainflow_headers/utils_inc` 与 `QtBCI/openbci/brainflow_headers/data_handler_inc`（与 `OPENBCI_VENDOR_ROOT` 一致，便于只推送 `QtBCI` 子目录）。

**（原 README：解压 brainflow-master 的相对路径，仍受 CMake 支持）**

CMakeLists.txt 会在下面两类位置查找头文件（存在即通过）：QtBCI/../brainflow-master/brainflow-master/src/utils/incQtBCI/../brainflow-master/brainflow-master/src/data_handler/inc

以及再往上一级的 ../../brainflow-master/… 同样结构。

也就是说：常见做法是解压 brainflow 源码，使上述相对路径从你打开的 QtBCI/QtBCI 文件夹算出去能指到 brainflow_array.h、data_handler.h。路径不一致就修改 CMakeLists.txt 里的 BRAINFLOW_\* 候选列表。

# 开发过程

### 协议

发送的包有小包和大包两种：小包的格式是AA AA 04 80 02 xxHigh xxLow xxCheckSum前面的AA AA 04 80 02 是不变的，后三个字节是一只变化的，xxHigh和xxLow组成了原始数据rawdata，xxCheckSum就是校验和。所以一个小包里面只包含了一个对开发者来说有用的数据，那就是rawdata，可以说一个小包就是一个原始数据，大约每秒钟会有512个原始数据。 那怎么从小包中解析出原始数据呢？rawdata = (xxHigh &lt;< 8) | xxLow; if(rawdata &gt; 32768){ rawdata ­=65536; } 现在原始数据就这么算出来了，但是在算原始数据之前，我们先应该检查校验和。 校验和怎么算呢？ sum = ((0x80 + 0x02 + xxHigh + xxLow)^ 0xFFFFFFFF) & 0xFF 什么意思呢？ 就是把04后面的四个字节加起来，取反，再取低八位。 如果算出来的sum和xxCheckSum是相等的，那说明这个包是正确的，然后再去计算 rawdata，否则直接忽略这个包。丢包率在10%以下是不会对最后结果造成影响的。

现在，原始数据出来了，那我们怎么拿信号强度Signal,专注度Attention,放松度Meditation,和8个EEG Power的值呢？

就在第513个这个大包里面，这个大包的格式是相当固定的，我们就拿上图中的数据来一个字节一个字节地说明他们代表的含义： 红色的是不变的 AA 同步 AA 同步 20 是十进制的32，即有32个字节的payload，除掉20本身+两个AA同步+最后校验和 02 代表信号值Signal C8 信号的值 83 代表EEG Power开始了 18 是十进制的24，说明EEG Power是由24个字节组成的，以下每三个字节为一组 18 Delta 1/3 D4 Delta 2/3 8B Delta 3/3 13 Theta 1/3 D1 Theta 2/3 69 Theta 3/3 02 LowAlpha 1/3 58 LowAlpha 2/3 C1 LowAlpha 3/3 17 HighAlpha 1/3 3B HighAlpha 2/3 DC HighAlpha 3/3 02 LowBeta 1/3 50 LowBeta 2/3 00 LowBeta 3/3 03 HighBeta 1/3 CB HighBeta 2/3 9D HighBeta 3/3 03 LowGamma 1/3 6D LowGamma 2/3 3B LowGamma 3/3 03 MiddleGamma 1/3 7E MiddleGamma 2/3 89 MiddleGamma 3/3 04 代表专注度Attention 00 Attention的值(0到100之间) 05 代表放松度Meditation 00 Meditation的值(0到100之间) D5 校验和 解析EEG Power： 拿Delta举例，Delta 1/3是高字节，Delta 1/3是中字节，Delta 1/3是低字节；高字节左移16位，中字节左移8位，低字节不变，然后将他们或运算，得到的结果就是Delta的值。这些值是无符号，没有单位的，只有在和其他的Beta，Gamma等值相互比较时才有意义。

这个无良商家打着tgam的名号做不是tgam的产品

注意采样率是215hz

但是在我的三种不同的测试下发现其实在256HZ左右

测试1：stream_sdk_for_pc(官方开发包)里面现成的函数可以直接调用，参考thinkgear.h（这是一个API文档）,测试——**`tests/legacy_thinkgear/stream_sdk_for_pc/`** 下的 origintest 等（路径相对当前仓库）

测试2：自己封装的stream_sdk_for_pc(利用测试一的API封装成一个类)，测试——include"stream_sdk_for_pc/threadteast.h"

include"stream_sdk_for_pc/changetest.h"

测试3：自己写的串口解析器如下章节——串口

## 串口

串口区提取的是二进制的数据，一个1字节=8bit

串口类需要定义

struct SerialPortConfig{    

QString portName="COM7";    

qint32 baudRate = 57600;    

QSerialPort::DataBits dataBits = QSerialPort::Data8;    

QSerialPort::Parity parity = QSerialPort::NoParity;    

QSerialPort::StopBits stopBits = QSerialPort::OneStop;    

QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;    }

名字

波特率

数据位=所有都是8位

校验位 = 无校验

流控制 = 无

蓝牙 / 串口透传设备都不用流控制

需要定义方法

开始，关闭，连接，需要把先用

const QByteArray chunk = m_port.readAll();

先把二进制保存在一个小的chunk模块里面，然后再上传给上层组帧层

组帧层

把所有chunk放入缓存区，然后进行组帧。

组帧层在把帧传入解析层，因为在计算机里面可以将16进制与二进制直接比较

转换层根据相关公式完成对应的转化

## 实施过程（单线程，控制函数为thinkgearlinktest）

1.在main里面，会对控制类ThinkGearLinkTester进行创建一个对象，在初始化对象的时候会初始化串口对象，组帧对象，解析对象，转换对象，调用测试对象的start函数，

2.ThinkGearLinkTester::start(const QString &portName,qint32 baudRate)函数里面有，先初始化串口，调用打开串口的只读状态，发送消息readonly，同时打开计数器

- 3.因为在串口初始化的里面有setupConnections()，在串口函数里面setupConnections()里面，有对串口准备接受信息readyRead，并且发送dataReceived(chunk)，接受数据放入chunk（readall会读走当前串口缓冲区里面的所有字节（在配置的串口的时候会对setReadBufferSize，QSerialPort 内部会先把系统/驱动收到的字节搬到它自己的缓冲里，等你调用 read()/readAll() 再取走。setReadBufferSize(N) 就是在告诉 QSerialPort：内部读缓冲最多允许积压约 N 字节（避免你长时间不读导致内部无限涨）。

），然后发送dataReceived(chunk)的信息。

4.在控制类（ThinkGearLinkTester里面），把dataReceived(chunk)的信息与ThinkGearFrameAssembler::onBytes连接，把chunk里面的二进制放入组帧类的缓冲区 m_rxBuffer，且限制m_rxBuffer小于64kb，并且进行组帧 pocessBuffer()，按照通信协议先找 AA AA,如果没有找到连续的两帧，并且缓存区大于1，则切除m_rxBuffer.size()-1的字节（防止最后一位是上一帧的第一个帧头 AA）如果找到AA AA的位置移除m_rxBuffer找到的AA AA前面的数据并且进行判断，移除后的缓存区如果小于3则直接退出函数等待下一个chunk进入m_rxBuffer，然后根据协议判断长度，如果长度大于170或者等于0，将前两位AA AA去除进行下一次循环，如果当缓冲区长度小于帧长度，退出等待下一个chunk进入m_rxBuffer，如果以上情况都没有出现，将从m_rxBuffer里面截取完整的一帧保存在一个数组frame里面，并且进行对尾帧进行CRC（可能是CRC，checksumOk(frame)）校验，如果是正确的一帧从m_rxBuffer里面把这一帧删除，并且将数组frame消息上传frameReady(frame)

5.rameReady(frame)的信号由解析器类ThinkGearPayloadParser里面的parseFrame函数接收，并且对上传的对fram再次进行判断（有一些沉余）如果有错误会发emit parseWarning(err)的消息并且被控制函数里面的lalable函数接收并且发出错误并且打印出来，出然后把数据部分发给parsePayload(payload)，然后根据协议部分数据由，标识符和数值组成，在将每一个数据判断出来之后提交不同的数据交给不同的函数解析到槽函数解析成原始值

## 多线程

请求线程（AcquistionEngine）

主要负责串口采集数据组帧解析通过qt的信号槽函数

connect(m_serial,&SerialPort::dataReceived,m_assembler,&ThinkGearFrameAssembler::onBytes);

connect(m_assembler,&ThinkGearFrameAssembler::frameReady,m_parser, &ThinkGearPayloadParser::parseFrame);

connect(m_parser,&ThinkGearPayloadParser::parseWarning,this,&AcquisitionEngine::warningMessage);

connect(m_parser,&ThinkGearPayloadParser::rawReceived,this,&AcquisitionEngine::onRawInt16);

connect(m_parser,&ThinkGearPayloadParser::signalQualityReceived,this, &AcquisitionEngine::onSignalQuality);

connect(m_parser,&ThinkGearPayloadParser::rawReceived,m_converter, &RawtOutUvProcessor::onRaw);

connect(m_converter,&RawtOutUvProcessor::uvValueReady,this, &AcquisitionEngine::onUvReady);到这里之后请求线程（AcquistionEngine）就会发出rawPacketReady(p)信号;，其中q是结构体对象

**（与当前 `acquisitionengine.cpp` 对齐的写法说明，类名已替换）**：`frameReady` 先连到 **`AcquisitionEngine::onAssemblerFrame`**，由该槽再调用 **`SerialEegPayloadParser::parseFrame`**；组帧 / 解析类名为 **`SerialEegFrameAssembler`**、**`SerialEegPayloadParser`**，换算为 **`RawToUvProcessor`**。另可连接组帧器的 **`checksumFailureOccurred`**、**`lengthResyncOccurred`**、**`rxBufferOverflowed`** 到采集引擎以产生 `warningMessage` / `linkDiagnostic`。

struct RawPacket {

QString tsMs;

quint64 seq = 0;

qint16 rawInt16 = 0;

quint8 signalQuality = 255;

double rawUv = 0.0;

};

**（当前头文件 `RawPacket` 字段补充，上文结构体原文保留）**：另含 `qint64 wallMs`、`quint64 gapSincePrev` 等，见 `core/acquisitionengine.h`。

### 其中信号质量判断为：

0～20：信号很好（优质）接触稳、噪声小，脑波数据可信。

- 21～50：一般（可用）有少量噪声，勉强能用，最好微调电极。
- 51～100：较差（慎用）干扰明显，数据波动大，结果不准。
- 101～200：很差（不可用）
- \=200：电极没碰到皮肤 / 没人戴
- 接触不良、头发挡住、头动太多、环境干扰大

并且在请求线程（AcquistionEngine）设置开始与停止函数，开始函数是对串口类的初始化，对串口函数进行配置并且在串口类里面SerialPort::openReadOnly()里面的m_port.open(QIODevice::ReadOnly);打开串口函数的标志

## 算法线程

主要是接收请求线程发出的rawPacketReady(p)信号，接收原始数据并且将原始数据进行分窗加入到PictureAndALgBuffer的缓存区里面,并且调用PictureAndALgBuffer::tryDequeueRawChunkForAlgo函数，这个函数主要是用来进行对传入数据进行分窗，返回chunk（128个点为一组）

## 预处理算法（基于BrainFlow里面的算法，在github上）

**（存档说明，与当前默认代码的关系）**：以下长文仍按「设备采样率不固定 → 先做线估 fs」的思路书写；**当前仓库默认**已在 **`EegPreprocessPipeline`** 中改为 **`use_fixed_nominal_fs = true`、标称 215 Hz**，在线 **`updateFsEstimate`** 分支默认不参与滤波主路径。请以正文 **§9.1** 及 **`preprocessing/eeg_preprocess_pipeline.h`** 为准；若需旧行为，在代码中改回 **`use_fixed_nominal_fs = false`** 并调整 **`fs_min`/`fs_max`**。

因为这个设备的采样率不固定，预处理有：陷波（50Hz）带通（0.5~40Hz）可能还有后续频域特征这些都依赖fs（采样率）。

所以先做一个**线估采样率**的算法，他的作用是：让这些处理在设备波动时仍然稳定有效。

数学公式如下

fsinst​=Δseq×1000/Δtms​

具体怎么做（你代码的实际逻辑）

1.  每次来一个包，拿到：

- seq（样本序号）
- ts_ms（毫秒时间戳）
- 和上一次保存的值做差：
- dseq = seq_now - seq_last
- dtms = ts_now - ts_last
- 算瞬时采样率：
- fs_inst = dseq \* 1000 / dtms
- 防抖（平滑）：
- fs_smooth = alpha \* fs_smooth + (1-alpha) \* fs_inst
- 你现在 alpha=0.90，所以不会抖得厉害
- 限幅防异常：
- 把 fs_inst 限在（可选在线模式下约 **200~230 Hz**，以头文件为准；旧文曾写 200~300）

代码在EegPreprocessPipeline类里面，在这个类里面有三个结构体，第一个结构体，Options（用来设置采样率估计，滤波参数，降采样，单通道伪影，小波去噪的参数），这些参数有的是利用了（**BrainFlow**的开源代码，一个预处理的开源库）

，第二个结构体FsHint（用于记录在线采样估计的时间戳和样本量），第三个结构体Result（用来记录原始值(微伏)，采样率，是否有伪迹，峰峰值（信号最大-信号最小），绝对值最大的点，轨道饱和率（硬件测不出来超过范围的部分，直接把超出的部分 “削平”（拉到最大或者最小值） 了。），调试文字信息）

### **clamp(double v, double lo, double hi)**

- 把值夹到区间 \[lo, hi\]。
- 用于限制在线估计采样率，防止异常值（时间戳抖动）污染系统。

### **calcPtpAndAbsMax(const QVector&lt;double&gt; &x, double &ptp, double &absmax)**

- 计算两个特征：
- ptp：峰峰值（max-min）
- absmax：绝对最大振幅
- 用于后面的伪影标记阈值判断。
- 空数组时都返回 0。

### **updateFsEstimate(const FsHint &hint, QString &dbg)**

输入是 hint.seq 和 hint.ts_ms。流程：

若 seq<0 或 ts_ms<=0，直接返回（说明没有可用 hint）。

第一次调用只记录 m_last，不计算。

后续调用计算：

- dseq = seq_now - seq_last
- dtms = ts_now - ts_last

若 dseq<=0 或 dtms<=0，返回（异常数据）。

计算瞬时采样率：

fs_inst = dseq \* 1000 / dtms

限幅：

fs_limited = clamp(fs_inst, fs_min, fs_max)

**平滑（指数平均）**：

- m_fs_smooth = alpha \* old + (1-alpha) \* fs_limited

把结果写进调试串 dbg。意义：不依赖固定 250Hz，滤波频带更准

**Result process(const QVector&lt;double&gt; &x_uv, const FsHint &hint)**

这是整个文件最重要的函数。

**步骤 A：初始化与采样率**

- out.y = x_uv：拷贝输入，后续就地处理。
- **当前默认**：若 **`use_fixed_nominal_fs == true`**，则**不调用** `updateFsEstimate`，直接将内部 fs 置为 **`fs_nominal`（215 Hz）** 并写入 `out.fs_used`。
- **可选分支**：若 **`use_fixed_nominal_fs == false`**，则调用 `updateFsEstimate(hint, dbg)` 根据 `FsHint` 做在线估计，再将 `fs = round(m_fs_smooth)` 保存到 `out.fs_used`。
- 如果空输入，直接返回并写 empty epoch。

**步骤 B：陷波（工频**）

- 条件：enable_notch=true
- 作用：去 50Hz（或配置值）工频干扰。
- 实现细节：QVector -> std::vector -> 调 DataFilter -> 再拷回 QVector

**步骤 C：带通**

- 条件：enable_bandpass=true
- 参数来源：bp_low_hz/bp_high_hz/order/filter_type/ripple
- 作用：保留主要 EEG 频段，去掉低频漂移和高频噪声。

**步骤 D：可选小波去噪**

- 条件：enable_wavelet_denoise=true（默认关闭）
- 作用：处理尖峰噪声（单通道可用替代方法）。
- 风险：过强会“抹”掉真实高频脑电，所以默认关。

**步骤 E：可选降采样**

- 条件：enable_downsample && downsample_factor > 1
- 会分配 double\* filtered，函数内有 delete\[\] filtered 回收。
- 输出变短，out.fs_used 也按因子缩小：
- out.fs_used /= downsample_factor

**步骤 F：单通道伪影标记（不删点**）

- 条件：enable_artifact_mark=true
- 计算：
- ptp_uv
- absmax_uv
- railed_ratio（DataFilter::get_railed_percentage）
- 判定规则（三个阈值任一超限）：
- ptp_uv > artifact_ptp_uv
- absmax_uv > artifact_absmax_uv
- railed_ratio > artifact_railed_ratio
- 结果写入：
- out.artifact_marked（true/false）
- 注意：这里只是标记，不删除样本，不改 out.y。
- 步骤 G：返回
- out.debug = dbg
- 返回 Result，供算法线程继续用（绘图、打分、标记）。

在mainwindow里面算法线程会把经过算法处理之后的数据放入PlotChunk（这是一个结构图专门存放处理好的分窗之后原始数据数组（128个点）以及他的开始值的序列，以及最后一个值的序列）里面然后放入ui线程里面的 m_plotCache缓冲区里面

并且把算法得到的数据放入记录LogItem结构体的对象li保存

里面存储了 li.tsMs = ts;

li.kind = QStringLiteral("preproc");

li.seq = pc.seqStart + static_cast&lt;quint64&gt;(i);

li.rawInt16 = 0;

li.signalQuality = 255;

li.rawUv = std::numeric_limits&lt;double&gt;::quiet_NaN();

li.preprocUv = pc.y\[i\];

并且把数据放入m_logBuffer缓存区

并且在mainwindow函数里面会接受AlgorithmEngine::algoResultReady信号把得到的算法结果

.arg(res.seqEnd)

.arg(res.score, 0, 'f', 3)

.arg(res.label));

放入到下方日志

## 记录线程

你这个项目里的“记录线程”主要就是 m_logThread + CsvLogWorker：负责把你采集/预处理出来的日志数据（原始 raw、预处理 preproc）异步写到 CSV 文件，尽量不阻塞 UI 和算法线程。

在mainwindow里面会有把请求线程与算法线程的数据保持给LogItem结构体，

并且加入给Logbuffer，相当与把原始数据与预处理之后的数据都放入logbuffer

进行合并处理，最后记录

记录线程为CsvLogWorker类，在这个线程里面也会有开始与结束的方法，在CsvLogWorker::start()里面会对写入文件的路径以及编码进行初始化，然后调用CsvLogWorker类里面的方法 writeHeaderIfNeeded()，这是一个写表头的方法，

然后 m_timer.s意思是，每隔固定时间触发 onFlushTick()，

在onFlushTick（）里面会判断文件是否有表头，如果有表头的话就不写表头，如果没有就写表头，并且先把logbuffer缓存区里面所有的队列元素出队临时保存在batch队列里面，并且对batch队列里面的元素先进行mergeItem(it)，在mergeItem()方法里面会将放入的元素会保存在Map数据结构中，并且以seq作为健，虽然两个数据是异步处理，但是通过同一个sep可以将两个数据放入同一个合并结构体 struct MergedRow

{

QString tsMs;

quint64 seq = 0;

qint16 rawInt16 = 0;

quint8 signalQuality = 255;

double rawUv = 0.0;

double preprocUv = 0.0;

bool hasRaw = false;

bool hasPreproc = false;

};

这个结构体用map进行储存，

为了防止丢包导致存储写入不及时，所以有flushMergedRows(bool forceAll)方法，在这个方法里面会通过判断是否数据的sep小于当前sep，并且是否超过最大限制m_mergeLag（**默认 256**，此处原文若写 512 为旧值），如果超过将写入标记ready变成true，直接写入如果没有超过继续等待，其中forceall是是否限制写入的标志，在stop的时候，forceAll=true 时（比如 stop()），会尽量把剩余都写完；在start的时候，forceAll=false 时，偏保守，保证顺序和完整性优先。最后调用 writeMergedRow(row.tsMs, row.seq, row.rawInt16, row.signalQuality, row.rawUv, row.preprocUv);把合并数据写入并且释放Map元素

在stop方法里面会对计时器m_time，还有文件以及数据流进行冲刷保证数据已经传输，最后关闭。

**UI线程**

初始化会对按钮进行初始化并且进行对各个控件的调整并且对画图面板进行参数设置，以及对日志区域进行初始化设置最后对定时器进行设置刷新。

在UI线程里面会有设置保存CSV以及txt路径的方法，他们分别是

makeDefaultEegCsvPath()和makeDefaultUiTxtPath()以及updateSavePathUi()，并且有将其它三个线程向上传递的操作以及错误信息与appendUiActionLog(const QString &category, const QString &message)和appendLogLine(const QString &line)进行连接，并且进行分类进行记录到日志区域

on_pushButton_save_clicked()这个方法定义保存按钮的对话框以及事件的连接，其它三个on_pushButton_stop_clicked()，on_pushButton_start_clicked()，on_pushButton_clear_clicked()是对四个线程的各个逻辑的表达进行连接

## 缓存区

| **名称** | **位置** | **典型容量 / 上限** | **所属线程** | **作用** |
| --- | --- | --- | --- | --- |
| 串口读缓冲 | QSerialPort + SerialPortConfig::readBufferSize | 32 KB（配置项） | 采集相关 | 操作系统 / 驱动层批量读取串口数据 |
| 组帧字节缓冲 | SerialEegFrameAssembler::m_rxBuffer（原 ThinkGearFrameAssembler） | 超过 64 KB 整段清空并计数溢出 | 采集  | 处理粘包 / 半包，拼接完整设备帧 |
| 算法环形缓冲 | PictureAndALgBuffer（AlgorithmEngine::m_buf） | 128×80 = 10240 个 RawSample | 算法  | 按样本序号分窗，供给预处理模块 |
| 日志队列 | MainWindow::m_logBuffer（LogBuffer） | 8192 条 LogItem | 主线程 push，日志线程 drain | 原始数据与预处理数据入队，用于写入 CSV |
| CSV 合并表 | CsvLogWorker::m_mergedRows（QMap） | 无固定上限，由合并策略控制 | 日志  | 按 seq 对齐原始数据与预处理数据后写入行 |
| 时域绘图缓存 | MainWindow::m_plotCache | m_plotCacheMax=20000（仅用于坐标轴，不会自动截断） | 主线程 | 存储预处理后波形数据，供 QCustomPlot 绘图 |
| FFT / PSD 特征缓存 | m_fftCache / m_psdCache | 最多 512 个点（m_featureCacheMax） | 主线程 | 用于频谱图、频段功率图显示 |
| UI 文本日志 | QPlainTextEdit | 最多 1500 段 | 主线程 | 界面日志显示，旧数据自动丢弃 |

## 2\. 串口与组帧缓冲

**2.1 SerialPortConfig::readBufferSize（默认 32×1024）**

- 在 device/serialport.h 中默认设置为 32768 字节，通过 QSerialPort 生效。作用：控制串口内核缓冲大小、单次可读数据量。它与后续业务层缓冲是两层结构：字节流先进入组帧器，再将解析后的采样点送入算法环形缓冲。

**2.2 SerialEegFrameAssembler::m_rxBuffer（上限 64 KB）**（原 **ThinkGearFrameAssembler** 小节标题保留语义）

- 串口 dataReceived 传来的 QByteArray 会追加到 m_rxBuffer，在 processBuffer() 中按 0xAA 0xAA 同步头、长度、校验位拆帧。
- 上限：kMaxRxBytes = 64\*1024
- 超过上限行为：清空缓冲、计数溢出、发出 rxBufferOverflowed 信号
- 目的：保护内存，避免异常数据流量导致内存暴涨

**3\. PictureAndALgBuffer（算法线程核心环形缓冲）**

构造：在 algorithmengine.h 中固定为PictureAndALgBuffer m_buf{128\*80}共 10240 个槽位，每个元素为 RawSample（包含 μV 值、时间、原始计数）。

写入逻辑

appendRawValue 在写指针 RawValue_writeIndex 处循环覆盖，写满一圈后标记 RawValue_filled = true，为标准环形队列。

**读取逻辑（算法）**

- tryDequeueRawChunkForAlgo(chunkSize, outChunk)
- 内部按 raw_count 排序为时间序
- 使用 m_algoReadSeq 作为读取游标
- 连续取出 chunkSize 个点
- 取出成功后，游标移动到 outChunk.last().raw_count + 1
- 在 AlgorithmEngine 中，chunkSize == m_windowSize（默认 128），即每凑满 128 个连续新样本，触发一窗预处理。

**读取逻辑（绘图，预留）**

- tryDequeueRawChunkForPlot 使用独立游标 m_plotReadSeq，与算法读取互不干扰。当前主程序未使用该接口，绘图数据直接来自 PlotChunk::y。

**重置接口**

- clear() / resetAlgoCursor()用于会话重置，与算法引擎 resetState 配合，避免新旧会话序号交错。

**4\. LogBuffer（主 CSV 写入前的线程安全队列）**

构造：MainWindow 中 LogBuffer m_logBuffer{8192}。

**核心接口**

- push(LogItem)：队列未满则入队；满则返回 false 并计数丢弃，不阻塞 UI 线程
- drainBatch(maxItems)：日志线程定时批量取出，取出后删除，降低锁竞争

**数据内容**

- LogItem 包含：
- 类型标记（raw /preproc）
- 样本序号 seq
- 原始值 rawUv
- 预处理值 preprocUv
- 主线程在两个信号槽中分别入队：

1.  rawPacketReady → 入队原始数据
2.  plotChunkReady → 入队预处理数据

**5\. CsvLogWorker 内部合并缓冲（m_mergedRows）**

类型：QMap&lt;quint64, MergedRow&gt;作用：按 seq 暂存已收到的原始数据与预处理数据，等待两边数据对齐。

写入策略

- 当 seq 对齐 或 序号差超过 m_mergeLag（默认 256）时，执行写入并清理
- 避免因预处理延迟导致 CSV 行永远无法合并
- 不属于固定长度环形结构，由定时刷新与合并滞后值控制大小
- PSD/FFT 数据为独立文件流写入，不经过 LogBuffer。

**6\. UI 侧缓存（主线程）**

**6.1 m_plotCache（时域波形缓存）**

# 每次 PlotChunk 到达时，将预处理波形 pc.y 整块追加到 m_plotCache。renderRawChart() 使用完整数据构建绘图，显示窗口由 m_rawFixedDisplayCount（默认 600）控制滑动视图。

注意：m_plotCacheMax=20000 仅用于初始化坐标轴范围，不会自动截断缓存。长时间采集会使缓存持续增长，需通过会话重置或手动清理控制内存。

**6.2 m_fftCache /m_psdCache（频谱 / 频段缓存）**

- 新结果在尾部追加，超过 m_featureCacheMax=512 时从头部移除，保持固定长度，适合滚动显示。

**6.3 UI 日志控件**

- QPlainTextEdit::setMaximumBlockCount(1500)只保留最近 1500 行日志，自动覆盖旧数据，防止 UI 内存无限增长。

## 调参常用配置（性能 / 延迟 / 内存优化）

需要更长算法历史窗 / 更稳定分窗增大 PictureAndALgBuffer 容量 或 调整 m_windowSize（注意算力与延迟平衡）。

.

高采样率下减少 CSV 队列丢弃增大 LogBuffer 容量（如 8192 → 16384），或降低入队频率。

CSV 合并行写入延迟大调小 CsvLogWorker::m_mergeLag，或检查原始数据与预处理数据 seq 是否对齐。

长时间运行 UI 内存占用高为 m_plotCache 添加上限截断逻辑，或定期执行会话重置清空缓存。
