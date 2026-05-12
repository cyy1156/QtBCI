# 网络设置与多路 UDP 发送 — 设计文档

**版本**：v0.2（2026-05-13）  
**范围**：QtBCI 主界面「网络设置」按钮 → 对话框配置协议 / **目标主机 + 各流独立 UDP 端口** / 数据类型开关，运行时经 **`UdpTelemetryHub`** 外发；与 `AlgorithmEngine` 三信号衔接。  
**关联文档**：`docs/UDP本地回环测试接入指南.md`、`docs/UDP与model在线推理结合指南.md`；`model/UDP_Test接入EEGNet推理示例.md`、`model/udp_infer_live.py`（消费端示例）。

**v0.2 变更摘要**：由「**同一 IP + 同一端口**，靠魔数区分多流」改为「**同一 IP + 多端口**」——与 OpenBCI GUI 等「一类数据一个监听端口」的习惯对齐；接收端可开 **多个 `bind` 或多个进程**，互不混包。

---

## 1. 背景与目标

### 1.1 现状

- **预处理窗 UDP**：`Net/udptelemetryhub.cpp` 将 `PlotChunk` 序列化为 **PBC1**，经 `QUdpSocket::writeDatagram` 发出。
- **FFT / PSD 特征**：分别为 **PBF1**、**PBP1** 报文（见 §4）。
- **主窗口**：`mainwindow.cpp` 在 `initThreads()` 中 **`NetworkSettingsStore::load`** 后 **`UdpTelemetryHub::applySettings`**，并以 **`Qt::QueuedConnection`** 连接三信号；用户在「网络设置」中修改并持久化。
- **算法引擎**：`AlgorithmEngine` 每窗依次 `plotChunkReady` / `spectrumReady` / `fftResultReady`（与现有实现一致）。

### 1.2 目标

1. 用户点击 **网络设置**，弹出 `QDialog`（与串口设置一致）。
2. 子页面包含：
   - **传输协议**：首期 **UDP**；其它协议占位即可。
   - **目标主机**：IPv4（如 `127.0.0.1`），三流 **共用同一主机**。
   - **各流独立端口**（`QSpinBox`，1–65535）：
     - **预处理时域窗（PBC1）**；
     - **FFT 频段特征（PBF1）**；
     - **PSD 频段特征（PBP1）**。
   - **数据类型**：三个 `QCheckBox`（是否发送该流）；未勾选则 **不向对应端口** 发送任何报文。
   - **总开关**：「启用网络发送」关闭时，三流均不发。
3. **同一 `QUdpSocket` 即可**（Qt 的 `writeDatagram(data, host, port)` 每次可指向不同端口）；无需强制三 socket，除非日后有绑定本机不同源端口等需求。
4. **PBC1 / PBF1 / PBP1** 载荷格式 **不变**；仅 **目标端口** 按流配置。Python 侧可为每类流单独 `bind` 一个端口，解析器可保持「每端口只一种 magic」，逻辑更简单。

---

## 2. 用户界面设计

### 2.1 入口

- 主窗口 **「网络设置」** 按钮 / 菜单 → `MainWindow::showNetworkConfigDialog()`。

### 2.2 对话框布局（v0.2）

| 区域 | 控件 | 说明 |
|------|------|------|
| 协议 | `QComboBox` | 仅 **UDP** |
| 主机 | `QLineEdit` | IPv4；`QHostAddress` 校验；启用网络时非法则禁用「确定」或提示 |
| 端口 | **三个** `QSpinBox` | 标签建议：「预处理 PBC1 端口」「FFT 特征 PBF1 端口」「PSD 特征 PBP1 端口」；**默认值**建议 `50001` / `50002` / `50003`（与旧版单端口 `50001` 兼容：预处理仍占 50001） |
| 数据流 | 三个 `QCheckBox` | 与三端口一一对应；全不选等价于不发三类包（总开关仍建议单独存在） |
| 总开关 | `QCheckBox`「启用网络发送」 | 关 = 不发任何流 |
| 按钮 | 确定 / 取消 | 确定：校验 + `QSettings` + `UdpTelemetryHub::applySettings` |

### 2.3 校验与提示

- **端口**：三端口 **允许相同**（退化为单端口混流，不推荐）；UI 可提供「三端口相同」时 **非阻塞警告**（黄色提示条或 `QToolTip`），引导用户分开端口以匹配 OpenBCI 风格。
- **端口冲突（本机接收）**：若 Python 在同一进程 `bind` 三个端口，需保证三值互不相同。
- **地址**：首期仅 IPv4 单播；组播/IPv6 另议。

### 2.4 持久化

- 与串口相同 **`QtBCI` / `QtBCI`** 组织与应用名。

---

## 3. 软件架构

### 3.1 分层

```
MainWindow
  └─ NetworkConfigDialog（UI + 校验）
  └─ NetworkStreamSettings（host、port_preproc、port_fft、port_psd、enabled、send_*）
  └─ UdpTelemetryHub
        ├─ sendPreprocPbc1 → writeDatagram(..., host, port_preproc)
        ├─ sendFftPbf1    → writeDatagram(..., host, port_fft)
        └─ sendPsdPbp1    → writeDatagram(..., host, port_psd)
  AlgorithmEngine（不变）
```

### 3.2 `UdpTelemetryHub`

- 成员：`QHostAddress m_host`；`quint16 m_portPreproc, m_portFft, m_portPsd`。
- `applySettings(const NetworkStreamSettings &)` 同步上述字段与开关标志。
- 仍可用 **单个** `QUdpSocket`；按槽内分支选择 **目标端口**。

### 3.3 主窗口

- `initThreads()`：`load` → `applySettings` → 三信号 **QueuedConnection** 到 Hub（与现实现一致）。
- 对话框 Accepted 后：`save` + `applySettings`；无需因多端口而 `disconnect` 重连（若仍采用「常连槽 + 内部开关」模型）。

### 3.4 线程模型

- 不变：`AlgorithmEngine` 在算法线程发信号，`UdpTelemetryHub` 槽在 GUI 线程、`QUdpSocket` 同线程。

---

## 4. 报文与协议（UDP）

### 4.1 PBC1 / PBF1 / PBP1

- **字节布局与 v0.1 完全一致**；见《UDP本地回环测试接入指南》§A.1 / §A.5。
- **唯一变化**：各流发往 **各自配置端口**，不再依赖「同一端口 + 魔数」区分接收 socket。

### 4.2 多选同时发送

- 每窗仍最多 **3 个 UDP 报文**，但可能发往 **三个不同端口**；接收端可用 **三个监听** 简化解析（每路只收一种格式）。
- 跨流对齐仍可用 **`seqStart` / `seqEnd` / `wallMs`**。

### 4.3 未来 TCP

- 不变；多端口策略主要作用于 UDP。

---

## 5. 配置持久化（QSettings）

| 键 | 类型 | 默认 | 说明 |
|----|------|------|------|
| `network/enabled` | bool | true | 与升级兼容；可按产品改为 false |
| `network/protocol` | string | `udp` | |
| `network/host` | string | `127.0.0.1` | |
| `network/port_preproc` | int | `50001` | PBC1；**兼容**旧键 `network/port`：若存在 `network/port` 且无 `port_preproc`，迁移时读旧值 |
| `network/port_fft` | int | `50002` | PBF1 |
| `network/port_psd` | int | `50003` | PBP1 |
| `network/send_preproc` | bool | true | |
| `network/send_fft` | bool | false | |
| `network/send_psd` | bool | false | |

**废弃**：单键 `network/port` 不再作为唯一端口写入；读取时仅作 **向 `port_preproc` 迁移** 的一次性兼容。

---

## 6. 与 Python / model 的衔接

| 数据 | Qt 目标 | Python 侧（多端口） |
|------|---------|----------------------|
| 预处理窗 | `host:port_preproc` | `udp_infer_live.py --port <port_preproc>` 或单独 `recvfrom` 绑定该端口 |
| FFT 特征 | `host:port_fft` | 独立脚本 `bind(port_fft)` + `parse_pbf1` |
| PSD 特征 | `host:port_psd` | 独立脚本 `bind(port_psd)` + `parse_pbp1` |

也可 **单进程** 内 `select` / 多线程三个 `socket` 各 `bind` 一端口，**无需**再根据魔数选分支（仍建议保留魔数校验防串线）。

---

## 7. 测试与验收

1. 三端口分别监听：仅勾选预处理 → 仅 `port_preproc` 有 PBC1；勾选 FFT/PSD → 对应端口出现 PBF1/PBP1。
2. `udp_infer_live.py`：`--port` 与 **`port_preproc`** 一致即可，**无需改解析**。
3. 持久化：修改三端口后重启 Qt，确认三路目标正确。
4. 总开关关闭：三端口均无流量。
5. 从旧 `network/port=50001` 配置启动：迁移后预处理端口仍为 50001，FFT/PSD 为默认 50002/50003。

---

## 8. 实现清单（相对 v0.1 的增量）

| 序号 | 任务 |
|------|------|
| 1 | `NetworkStreamSettings`：增加 `port_fft`、`port_psd`；`port` 重命名或别名 `port_preproc`；`load/save` + 旧键迁移 |
| 2 | `networkconfigdialog.ui`：单端口改为三 `QSpinBox`；文案与 §2.2 一致 |
| 3 | `UdpTelemetryHub::applySettings` / 三 `send*`：使用对应端口 `writeDatagram` |
| 4 | 设计文档与本节（已完成 v0.2 文档） |
| 5 | （可选）《UDP本地回环测试接入指南》增加一句「多端口为推荐部署，与 OpenBCI GUI 习惯一致」 |

---

## 9. 风险与后续扩展

- **防火墙**：需放行最多三个 UDP 目的端口（本机回环一般无妨）。
- **带宽**：与 v0.1 相同。
- **安全**：不变。

---

## 10. 文档修订记录

| 日期 | 说明 |
|------|------|
| 2026-05-12 | v0.1：单端口 + 魔数区分多流；PBC1/PBF1/PBP1 帧结构 |
| 2026-05-13 | **v0.2**：改为 **同主机、多端口**（预处理 / FFT / PSD 各端口）；QSettings 键扩展与旧 `network/port` 迁移说明；Hub 按流选目的端口 |
