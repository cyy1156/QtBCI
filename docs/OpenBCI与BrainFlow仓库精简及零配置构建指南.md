# OpenBCI / BrainFlow 仓库精简与「拉取即可在 Qt Creator 构建」指南

本文基于当前工程中的 `CMakeLists.txt` 与源码引用关系整理：**去掉 NeuroSky ThinkGear 官方 SDK 残余**、**把 BrainFlow 相关依赖收拢到 Qt CMake 工程根目录下的 `openbci/`**（与 `CMakeLists.txt` 同级，便于只推送内层 `QtBCI/` 目录到 GitHub），并说明为便于他人在 GitHub 克隆后用 Qt Creator 二次开发时，建议删除、移动或纳入版本库的内容。

**工程已落实（2026-05）**：主工程不再链接 `thinkgear64`；串口组帧/解析在 `device/serial_eeg/`；`thinkgear.h`、`stream_sdk_for_pc`、`ThinkGearLinkTester` 迁至 `QtBCI/tests/legacy_thinkgear/` 且**不参与**主 CMake；BrainFlow 头与 `DataHandler` 优先从 `openbci/` 探测（仍兼容 `brainflow-master` 与 `preprocessing/`）。

---

## 1. 目标与诚实边界

| 目标 | 说明 |
|------|------|
| 去除 ThinkGear | 指去除 **NeuroSky ThinkGear Stream SDK for PC**（`thinkgear64.lib` / `thinkgear64.dll` 及 `TG_*` API），以及仅服务于该 SDK 的示例代码。 |
| BrainFlow 放在工程内 `openbci/` | 不把完整 `brainflow-master` 与业务源码混放；在 **`CMakeLists.txt` 所在目录**下建立 `openbci/`，集中放置预处理所需头文件与 `DataHandler` 预编译库，便于整夹上传 GitHub。 |
| 克隆后少配置 | 通过**固定相对路径** + **把必需的二进制/头文件纳入仓库或子模块**，使 CMake 不再依赖各人本机上的散落路径。 |

**必须说明**：他人机器上仍需安装 **Qt 6.5+**（含 SerialPort 等组件）与 **CMake**，这是 Qt 桌面开发的正常前提，无法通过纯仓库消除。能做到的是：**不再手动拷贝 BrainFlow 路径、不再单独安装 ThinkGear SDK**。

若采集链路从当前「自研 NeuroSky 流式组帧」改为 **OpenBCI 硬件 + BrainFlow 板级 API**，还需要**替换/重写 `AcquisitionEngine` 及下游假设**（采样率、通道数、包格式等），这不是仅改 CMake 能完成的，见第 7 节。

---

## 2. 当前 CMake 依赖关系（摘要）

根 `QtBCI/CMakeLists.txt` 当前大致分为三块：

1. **Qt6**：`Core` `Widgets` `SerialPort` `PrintSupport` `Network`。
2. **BrainFlow DataHandler 预编译库**：优先 **`${CMAKE_CURRENT_SOURCE_DIR}/openbci/lib/...`**，其次 `preprocessing/`。
3. **BrainFlow 头文件（仅编译期）**：优先 **`${CMAKE_CURRENT_SOURCE_DIR}/openbci/brainflow_headers/...`**，其次 `brainflow-master/.../src/.../inc`。

**已不再包含**：ThinkGear 官方 `thinkgear64.lib` / `thinkgear64.dll`。

---

## 3. ThinkGear：哪些是「厂商 SDK」，哪些是「自研协议栈」

### 3.1 属于 NeuroSky **官方 SDK**（建议整体移除的部分）

| 类别 | 路径或文件 | 依据 |
|------|------------|------|
| 链接库 / 运行库 | `thinkgear64.lib`、`thinkgear64.dll`（及 CMake 中 `THINKGEAR_*` 整段） | `CMakeLists.txt` 强制查找并 `target_link_libraries`。 |
| 厂商头文件 | `thinkgear/thinkgear.h`（声明 `TG_*`） | 与官方 DLL 配套。 |
| 依赖 `thinkgear.h` 与 `TG_*` 的源码 | `stream_sdk_for_pc/` 下：`stream_sdk_for_pc.{h,cpp}`、`origintest.*`、`changetest.*`、`threadteast.*`、`test_stream_*.cpp` 等 | 源码中直接 `#include "thinkgear.h"` 并调用 `TG_Connect` / `TG_ReadPackets` 等。 |

以上移除后，应从 `qt_add_executable` 源文件列表中**删掉**对应条目，并删除 CMake 中 ThinkGear 的 `include_directories`、`target_link_libraries` 与 `POST_BUILD` 里对 `thinkgear64.dll` 的拷贝。

### 3.2 名为 ThinkGear、实为**自研组帧/解析**（与官方 DLL 无直接符号依赖）

以下类实现 NeuroSky **设备流式包格式**的组帧与解析，**不**调用 `TG_*`，但若你转向 OpenBCI，其**串口包格式与 Cyton/Ganglion 不同**，通常不能原样沿用，需整体替换为 BrainFlow 采集或 OpenBCI 二进制协议解析：

| 组件 | 路径 |
|------|------|
| `ThinkGearFrameAssembler` / `ThinkGearPayloadParser` / `RawtOutUvProcessor` | `thinkgear/thinkgearframeassembler.*`、`thinkgearpayloadparser.*`、`rawtouvprocessor.*` |
| `ThinkGearLinkTester` | `thinkgear/thinkgearlinktester.*`（与主窗口可选连接的旧链路） |
| 主采集引擎 | `core/acquisitionengine.*`（include 与成员均绑定上述类） |

**结论**：  
- 若仅目标是「去掉 ThinkGear **官方 SDK** 与残余示例」：删除第 3.1 节内容后，若工程中**没有任何**翻译单元再引用 `TG_*`，即可从链接行去掉 `thinkgear64.lib`。  
- 若目标是「设备改为 OpenBCI」：第 3.2 节属于**旧设备协议栈**，应在采集侧重写后删除或改名迁移，避免与 OpenBCI 混用。

---

## 4. BrainFlow：`openbci/` 放在 Qt 工程根目录（与 `CMakeLists.txt` 同级）

若你**只把内层 `QtBCI/` 推送到 GitHub**，推荐在本机保持如下结构（`openbci` 与 `main.cpp` 并列）：

```text
<Qt CMake 工程根>/              # 含 CMakeLists.txt（你推送 GitHub 的这一层）
  CMakeLists.txt
  main.cpp
  device/
  openbci/
    brainflow_headers/
      utils_inc/
      data_handler_inc/
    lib/
      mingw_64/
        DataHandler.lib
        DataHandler.dll
    # 可选：git submodule 放置完整 brainflow 源码，例如 openbci/brainflow/
```

### 4.1 CMake（当前工程已采用）

- `OPENBCI_VENDOR_ROOT` = `${CMAKE_CURRENT_SOURCE_DIR}/openbci`
- `_BRAINFLOW_*` 与 `_DATAHANDLER_*` 候选路径的首项均为 **`${CMAKE_CURRENT_SOURCE_DIR}/openbci/...`**

这样只要 **`openbci/` 与 `CMakeLists.txt` 一并提交**，克隆后无需再指向上级目录。

### 4.2 头文件来源说明

当前 `preprocessing/data_filter.h` 包含：

- `brainflow_array.h`、`brainflow_constants.h`、`brainflow_exception.h`
- `data_handler.h`

这些文件在官方 BrainFlow 源码树中的位置即 CMake 里已写明的 `src/utils/inc` 与 `src/data_handler/inc`。迁入 `openbci/brainflow_headers/` 时，应保持**与所链接的 `DataHandler.lib` 相同的 BrainFlow 版本**，否则可能出现 ABI/枚举不一致。

---

## 5. `preprocessing/data_handler/` 大块源码：是否「残余」

`QtBCI/preprocessing/data_handler/` 下包含 **BrainFlow data_handler 的源码副本**（含 kissfft、DSPFilters、wavelib 等）及 `build.cmake`。**当前根 `CMakeLists.txt` 并未 `add_subdirectory` 该目录**，工程实际链接的是预编译 **`DataHandler.lib`**（优先 **`openbci/lib/...`**，否则 `preprocessing/`）。

因此：

- 若策略是「**只使用预编译 DataHandler**」：该目录可作为**冗余**从仓库删除（或移出 Git，仅本地保留），以减小体积；务必在文档中写明 **DataHandler 的官方构建方式**与版本号。  
- 若策略是「**从源码编 DataHandler 再链接**」：应新增独立 CMake 目标或脚本，而不是静默保留一份不参与构建的源码树。

---

## 6. 建议从 Git 仓库排除或不要提交的内容（减轻克隆负担）

| 路径/类型 | 说明 |
|-----------|------|
| `build/`、`**/build/` | Qt Creator 生成目录，应 `.gitignore`。 |
| `QtBCI - 副本/` 等重复工程 | 非必要不要进远程仓库。 |
| 仓库根下完整 `.venv/`、`PythonProject/.venv/` | 与 C++ 主工程无关且体积巨大。 |
| 完整 `brainflow-master/`（若已用 `openbci` 精简头文件 + 子模块策略） | 避免与 `openbci` 重复；可改为 `git submodule`。 |
| 个人 IDE 缓存 | `.idea/` 等按需忽略。 |

**许可注意**：`thinkgear64.dll` 等 NeuroSky 二进制通常**不可随意再分发**；去掉 ThinkGear 官方 SDK 也有利于仓库合规。

---

## 7. 转向 OpenBCI 时的代码层工作（CMake 之外）

当前主链为 `MainWindow` → `AcquisitionEngine` → `SerialPort` + **NeuroSky 流式组帧**。OpenBCI Cyton/Daisy 等使用**不同**串口帧格式与控制命令。

可选方向：

1. **BrainFlow C++ API**（`BoardShim` 等）：在工程中增加对 BrainFlow **BoardController** 的链接与线程封装，用其拉取时间序列，再接入现有预处理/绘图管道。  
2. **自研 OpenBCI 串口解析**：不引入完整 BoardController，仅实现与硬件文档一致的二进制解析（维护成本高）。

无论哪种，都需统一 **通道数、采样率、电压换算** 等与 UI/算法相关的假设。

---

## 8. 推荐实施顺序（ checklist ）

1. 在 **Qt 工程根目录**（与 `CMakeLists.txt` 同级）维护 `openbci/`，拷贝与 `DataHandler.lib` **版本一致**的 `utils/inc` 与 `data_handler/inc` 头文件到 `openbci/brainflow_headers/`。  
2. 将 `DataHandler.lib` / `DataHandler.dll` 放入 `openbci/lib/<工具链>/`（CMake 已优先探测）；可选删除对 `brainflow-master/...` 的依赖以强制自包含。
3. 从 `qt_add_executable` 与源码树中移除 `stream_sdk_for_pc` 等仅服务官方 SDK 的单元；删除 CMake 中 ThinkGear 相关段落及 `POST_BUILD` 中对 `thinkgear64.dll` 的拷贝。  
4. 确认全工程 `grep` 无 `TG_`、`thinkgear64`、`#include "thinkgear.h"`（若保留自研组帧，可暂时保留类名，但应计划与 OpenBCI 替换方案衔接）。  
5. 按需删除或 submodule 化 `preprocessing/data_handler/` 源码副本。  
6. 更新根目录 `README.md` / `开发手册.md`：写明用 Qt Creator 打开 **`QtBCI/CMakeLists.txt`**、所需 Qt 版本、64 位 Kit，以及 `openbci/` 目录约定。  
7. 若正式支持 OpenBCI：实现新采集后端并替换 `AcquisitionEngine` 后再删除第 3.2 节中的旧协议类。

---

## 9. 小结表

| 项目 | 建议操作 |
|------|----------|
| ThinkGear 官方 SDK（lib/dll + `thinkgear.h`） | 移除 CMake 与仓库中的二进制/头文件依赖（注意许可）。 |
| `stream_sdk_for_pc` 等 TG_* 示例 | 从目标与仓库删除或移入独立「历史」分支。 |
| BrainFlow 头路径 | 固定到 **`${CMAKE_CURRENT_SOURCE_DIR}/openbci/brainflow_headers/...`**（与 `CMakeLists.txt` 同级）。 |
| DataHandler 预编译库 | 固定到 **`${CMAKE_CURRENT_SOURCE_DIR}/openbci/lib/<工具链>/`**。 |
| `preprocessing/data_handler/` 源码树 | 未参与当前顶层构建 → 视为冗余或改为显式子工程。 |
| `thinkgear/` 下旧自研组帧（已迁移） | 已迁至 `device/serial_eeg/`；若文档仍写 `thinkgear/` 请改为新路径。 |

按上述整理后，协作者克隆仓库、用 Qt Creator 打开 `QtBCI/CMakeLists.txt` 并选择匹配工具链的 64 位 Kit，即可在**不单独配置 BrainFlow 本机路径**的前提下完成配置与构建（仍须本机已安装 Qt）。
