# UDP 本地回环测试接入指南（QtBCI）

本文说明：**不改动现有串口 EEG 组帧逻辑**的前提下，如何用 **UDP + 127.0.0.1** 做「一发一收」打通测试，以及代码建议放在哪、工程要改哪。

---

## 1. 先对齐现有数据流（你应该挂在哪里）

当前样本进入算法的唯一入口是 `RawPacket`，由串口链路在 `AcquisitionEngine::onUvReady()` 里组装并发出信号：

| 环节 | 文件 | 作用 |
|------|------|------|
| 串口字节 → 帧 → 解析 | `core/acquisitionengine.cpp` | `SerialEegFrameAssembler` / `SerialEegPayloadParser` / `RawToUvProcessor` |
| 样本出炉 | 同上 `onUvReady()` | `emit rawPacketReady(p)` |
| 算法消费 | `mainwindow.cpp` | `connect(m_acq, &AcquisitionEngine::rawPacketReady, m_alg, &AlgorithmEngine::onRawPacket, ...)` |

**UDP 回环测试有两种挂法（二选一或并存）：**

### 方案 A：发送端 —— 从现有链路「旁路复制」出去（推荐先做这个）

- **位置**：新建一个小类（例如 `core/udp_raw_sink.{h,cpp}`），在 **`MainWindow` 构造函数**里：
  - `connect(m_acq, &AcquisitionEngine::rawPacketReady, udpSink, &UdpRawSink::sendPacket);`
- **作用**：真机或模拟串口跑起来时，每个 `RawPacket` 序列化成 UDP 报文发往 `127.0.0.1:<端口>`，用 Wireshark / 另一个进程验证「包确实在飞」。
- **优点**：不动 `AcquisitionEngine` 内部；开关可在 UI 或编译期宏控制。

### 方案 B：接收端 —— 把 UDP 还原成 `RawPacket` 再送进算法（推荐第二步）

- **位置**：新建 `core/udp_raw_source.{h,cpp}`（内部 `QUdpSocket`，`bind(QHostAddress::LocalHost, port)`）。
- **挂法**：在 **`MainWindow`** 里增加「UDP 测试模式」：
  - **开启时**：`connect(udpSource, &UdpRawSource::rawPacketReconstructed, m_alg, &AlgorithmEngine::onRawPacket, Qt::QueuedConnection);`
  - **关闭时**：断开上述连接，避免与真实 `m_acq` 重复喂入（同一条 `onRawPacket` 被两个源同时连会乱序/重复）。
- **可选**：若希望 UDP 模式完全不打开串口，则不要 `connect(m_acq → m_alg)`，只连 `udpSource → m_alg`。

**不建议**的第一刀改动：直接把 `QUdpSocket` 塞进 `AcquisitionEngine` 类里。采集引擎已经承担串口与组帧职责，UDP 属于「传输/测试适配层」，单独成类更清晰，也方便以后换成 TCP/LSL。

---

## 2. 工程与依赖（必须改的一处）

项目使用 CMake，`QtBCI/CMakeLists.txt` 里当前是：

```cmake
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets SerialPort PrintSupport)
```

要做 UDP，需要增加 **Network** 组件，并在 `target_link_libraries` 里链接 `Qt::Network`：

1. `find_package` 的 `COMPONENTS` 中加入 **`Network`**。
2. `target_link_libraries(QtBCI PRIVATE ...)` 中加入 **`Qt::Network`**。
3. 把你新建的 `udp_raw_sink.cpp` / `udp_raw_source.cpp` 加到 `qt_add_executable(QtBCI ...)` 的源文件列表中。

新建类头文件中：`#include <QUdpSocket>`（以及按需 `#include <QHostAddress>`）。

---

## 3. 报文格式（本地回环最小可行）

UDP 无连接、可能丢包、双包合一包（应用层需能解析多条或固定一包一样本）。**_loopback 第一阶段建议：一个 UDP _datagram = 一个样本_**。

字段与 `acquisitionengine.h` 中 `RawPacket` 一致即可：

- `quint64 seq`
- `qint64 wallMs`
- `quint64 gapSincePrev`
- `qint16 rawInt16`
- `quint8 signalQuality`
- `double rawUv`
- `QString tsMs` → 建议 UTF-8 编码，前面加 `quint16` 长度；或使用固定 32 字符不足补 `\0`（实现简单但不优雅）。

**建议在报文头加 4 字节 magic**（例如 `0x52 0x47 0x42 0x31` 表示 `RGB1`）和 **1 字节 version**，避免误把别的程序的 UDP 误解析成脑电样本。

解析侧：`readyRead` → `readDatagram` → 校验 magic/version → `QDataStream` 或手动小端读写 → 组装 `RawPacket` → `emit` 给 `AlgorithmEngine::onRawPacket`。

---

## 4. 本地回环操作要点

| 项目 | 说明 |
|------|------|
| 地址 | 发送目标使用 **`127.0.0.1`**（或 `QHostAddress::LocalHost`），与文档里组帧设计无关，仅是本机 UDP。 |
| 端口 | 接收端 **bind** 固定端口（如 `50000`）；发送端 **不必 bind**，由系统分配 ephemeral 端口即可。 |
| 双进程 | 进程 A 只发、进程 B 只收 bind 同一端口 —— 可行；同一进程内既要「自发自收」也可行（注意不要在同一端口上重复 bind）。 |
| 线程 | `QUdpSocket` 若在子线程使用，需在该线程 `moveToThread` 并保证「在哪个线程创建就在哪个线程读」；最省事是第一版全在 **主线程** 或 **`AcquisitionEngine` 所在线程** 旁路发送小报文（215 Hz 通常可承受）。 |

---

## 5. 验收步骤（建议顺序）

1. **只实现发送（方案 A）**：串口正常采集时，用 netcat / 自写 Python `socket` 收 `127.0.0.1:50000`，打印收到的条数是否与采样率大致一致。
2. **再实现接收（方案 B）**：用 Python 按相同格式往 `50000` 发包，`AlgorithmEngine` 是否仍能累计窗口、出图。
3. **最后**再考虑批量打包、压缩、加密 —— 回环验证不必一上来就做。

---

## 6. 与「准确率」的关系

UDP 只负责把浮点/整型样本送到消费者；**分类准确率**仍在上层（模型输出 vs 标签）统计，与是否 UDP 无关。回环阶段可先打印 `seq` 连续性检测丢包。

---

## 7. 文件清单小结

| 动作 | 路径 |
|------|------|
| 增加 UDP 发送适配类（可选） | `QtBCI/core/udp_raw_sink.h`、`udp_raw_sink.cpp` |
| 增加 UDP 接收适配类（可选） | `QtBCI/core/udp_raw_source.h`、`udp_raw_source.cpp` |
| 挂载 connect / 模式开关 | `QtBCI/mainwindow.cpp`（以及如需 UI：`mainwindow.ui` / `mainwindow.h`） |
| 打开 Qt Network | `QtBCI/CMakeLists.txt` |

按上表实现后，即可在 **不依赖真实网络环境** 的情况下验证：序列化、`RawPacket` 语义、以及 `AlgorithmEngine` 入口是否与你的后续在线模型推送兼容。

---

## 附录：预处理窗口（`PlotChunk`）→ UDP → Python 模型（仅示例，不替代你改工程）

**示例有两种排版**：**§A.2 方案一**为「整段写在单个 `.h`」；**§A.2b 方案二**为「`.h` 声明 + `.cpp` 实现」拆分写法（CMake 里两个文件都要加入 `qt_add_executable`）。**当前工程正式实现**为 **`Net/udptelemetryhub.*`**（含 PBC1/PBF1/PBP1 与「网络设置」对话框），下列 `PreprocChunkUdpSender` 仍可作为最小示例参考。若文件放在 `Net/preprocchunkudpsender.*`，把示例里的 include 路径改成相对你的目录即可。

目标：把 `AlgorithmEngine::plotChunkReady(const PlotChunk &chunk)` 里的 **`chunk.y`（预处理后单通道波形）** 打成 UDP 报文发到 `127.0.0.1`，Python 端 `recvfrom` 解包成 `numpy` 数组，供在线模型试跑。

### A.1 报文格式（小端，一包 = 一个预处理窗口）

| 偏移 | 类型 | 字段 |
|------|------|------|
| 0 | `char[4]` | Magic：`PBC1`（`0x50 0x42 0x43 0x31`） |
| 4 | `uint8` | `version = 1` |
| 5 | `uint8` | `flags`（保留，填 0） |
| 6 | `uint16` | `reserved`（填 0） |
| 8 | `uint64` | `seq_start`（与 `PlotChunk::seqStart` 一致） |
| 16 | `uint64` | `seq_end`（与 `PlotChunk::seqEnd` 一致） |
| 24 | `int64` | `anchor_wall_ms`（与 `PlotChunk::anchorWallMs` 一致） |
| 32 | `uint64` | `anchor_seq`（与 `PlotChunk::anchorSeq` 一致） |
| 40 | `uint32` | `n`（`chunk.y.size()`，即样本个数） |
| 44 | `n × float64` | 连续的 `double`，顺序与 `chunk.y[0]…y[n-1]` 相同 |

**UDP 尺寸**：头 44 字节 + `n×8`。默认窗口 `n=128` 时约 `44+1024=1068` 字节，远小于 UDP 上限。

**线程**：若你在 UI 线程连接 `plotChunkReady`，下面示例在主线程 `writeDatagram` 即可；若以后移到工作线程，请保证 `QUdpSocket` 在该线程创建与使用。

---

### A.2 方案一：`PreprocChunkUdpSender`（仅 `.h`，实现全在头文件）

**`preproc_chunk_udp_sender.h`**

```cpp
#pragma once

#include <QObject>
#include <QByteArray>
#include <QHostAddress>
#include <QUdpSocket>
#include "core/algorithmengine.h"   // PlotChunk

/**
 * 将 plotChunkReady 的预处理向量 chunk.y 发到 UDP（示例：本地 Python 收包测模型）。
 * 使用前：CMake 链接 Qt::Network；在 MainWindow 里 connect plotChunkReady → sendPlotChunk。
 */
class PreprocChunkUdpSender : public QObject
{
    Q_OBJECT
public:
    explicit PreprocChunkUdpSender(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    void setTarget(const QHostAddress &addr, quint16 port)
    {
        m_addr = addr;
        m_port = port;
    }

    /** 供 connect(..., &AlgorithmEngine::plotChunkReady, ...) 使用 */
    void sendPlotChunk(const PlotChunk &chunk)
    {
        const quint32 n = static_cast<quint32>(chunk.y.size());
        QByteArray buf;
        buf.resize(44 + int(n) * 8);

        auto *p = reinterpret_cast<unsigned char *>(buf.data());

        // magic "PBC1"
        p[0] = 'P'; p[1] = 'B'; p[2] = 'C'; p[3] = '1';
        p[4] = 1;   // version
        p[5] = 0;   // flags
        p[6] = 0; p[7] = 0; // reserved u16 LE

        auto w64 = [&](int off, quint64 v) {
            for (int i = 0; i < 8; ++i) p[off + i] = static_cast<unsigned char>((v >> (8 * i)) & 0xFF);
        };
        auto w32 = [&](int off, quint32 v) {
            for (int i = 0; i < 4; ++i) p[off + i] = static_cast<unsigned char>((v >> (8 * i)) & 0xFF);
        };
        auto w64s = [&](int off, qint64 v) { w64(off, static_cast<quint64>(v)); };

        w64(8, chunk.seqStart);
        w64(16, chunk.seqEnd);
        w64s(24, chunk.anchorWallMs);
        w64(32, chunk.anchorSeq);
        w32(40, n);

        // doubles LE (IEEE754)
        int off = 44;
        for (quint32 i = 0; i < n; ++i) {
            union { double d; unsigned char b[8]; } u{};
            u.d = chunk.y[int(i)];
            for (int k = 0; k < 8; ++k) p[off + k] = u.b[k]; // 假设 host 是小端；跨平台可改用 memcpy + 若大端再逐字节交换
            off += 8;
        }

        m_socket.writeDatagram(buf, m_addr, m_port);
    }

private:
    QUdpSocket m_socket;
    QHostAddress m_addr{QHostAddress::LocalHost};
    quint16 m_port = 50001;
};
```

**在 `MainWindow` 里挂载（示例片段，自行粘贴）**

```cpp
// 成员：PreprocChunkUdpSender *m_preprocUdp = nullptr;

m_preprocUdp = new PreprocChunkUdpSender(this);
m_preprocUdp->setTarget(QHostAddress::LocalHost, 50001);
connect(m_alg, &AlgorithmEngine::plotChunkReady,
        m_preprocUdp, &PreprocChunkUdpSender::sendPlotChunk);
```

> **说明**：上面 `double` 写入在 **Windows / x86_64 小端** 上与 `memcpy` 等价；若需严格跨平台，可对 8 字节显式按小端写入（或用 `QDataStream` 设 `setByteOrder(QDataStream::LittleEndian)` 写 `chunk.y[i]`）。

---

### A.2b 方案二：拆分写法（`preproc_chunk_udp_sender.h` + `preproc_chunk_udp_sender.cpp`）

与方案一行为相同；适合希望编译单元分离、减少头文件依赖的场景。

**CMake**：在 `qt_add_executable(...)` 中同时列出 `.h` 与 `.cpp`；`find_package(Qt6 ... Network)` + `Qt::Network` 不变。启用 `AUTOMOC` 时（`qt_standard_project_setup()` 默认会处理），含 `Q_OBJECT` 的类会在编译该 `.cpp` 时自动生成 `moc_preproc_chunk_udp_sender.cpp`，无需手写 `#include "moc_..."`。

**`preproc_chunk_udp_sender.h`**

```cpp
#pragma once

#include <QHostAddress>
#include <QObject>
#include <QUdpSocket>

#include "core/algorithmengine.h"   // PlotChunk；按你工程路径修改

/**
 * 将 plotChunkReady 的预处理向量 chunk.y 发到 UDP。
 * 使用前：链接 Qt::Network；MainWindow 里 connect plotChunkReady → sendPlotChunk。
 */
class PreprocChunkUdpSender : public QObject
{
    Q_OBJECT
public:
    explicit PreprocChunkUdpSender(QObject *parent = nullptr);

    void setTarget(const QHostAddress &addr, quint16 port);

public slots:
    /** 也可去掉 slots 改为普通 public 成员函数，connect 语法不变（Qt5+） */
    void sendPlotChunk(const PlotChunk &chunk);

private:
    QUdpSocket m_socket;
    QHostAddress m_addr;
    quint16 m_port = 50001;
};
```

**`preproc_chunk_udp_sender.cpp`**

```cpp
#include "preproc_chunk_udp_sender.h"

#include <QByteArray>

PreprocChunkUdpSender::PreprocChunkUdpSender(QObject *parent)
    : QObject(parent)
    , m_addr(QHostAddress::LocalHost)
{
}

void PreprocChunkUdpSender::setTarget(const QHostAddress &addr, quint16 port)
{
    m_addr = addr;
    m_port = port;
}

void PreprocChunkUdpSender::sendPlotChunk(const PlotChunk &chunk)
{
    const quint32 n = static_cast<quint32>(chunk.y.size());
    QByteArray buf;
    buf.resize(44 + int(n) * 8);

    auto *p = reinterpret_cast<unsigned char *>(buf.data());

    p[0] = 'P';
    p[1] = 'B';
    p[2] = 'C';
    p[3] = '1';
    p[4] = 1;
    p[5] = 0;
    p[6] = 0;
    p[7] = 0;

    auto w64 = [&](int off, quint64 v) {
        for (int i = 0; i < 8; ++i)
            p[off + i] = static_cast<unsigned char>((v >> (8 * i)) & 0xFF);
    };
    auto w32 = [&](int off, quint32 v) {
        for (int i = 0; i < 4; ++i)
            p[off + i] = static_cast<unsigned char>((v >> (8 * i)) & 0xFF);
    };
    auto w64s = [&](int off, qint64 v) { w64(off, static_cast<quint64>(v)); };

    w64(8, chunk.seqStart);
    w64(16, chunk.seqEnd);
    w64s(24, chunk.anchorWallMs);
    w64(32, chunk.anchorSeq);
    w32(40, n);

    int off = 44;
    for (quint32 i = 0; i < n; ++i) {
        union {
            double d;
            unsigned char b[8];
        } u{};
        u.d = chunk.y[int(i)];
        for (int k = 0; k < 8; ++k)
            p[off + k] = u.b[k];
        off += 8;
    }

    m_socket.writeDatagram(buf, m_addr, m_port);
}
```

**`MainWindow` 挂载**与 §A.2 末尾片段相同（`new PreprocChunkUdpSender`、`setTarget`、`connect plotChunkReady`）。

---

### A.3 Python 示例：收包并打成 `numpy` 窗口（给 PyTorch / sklearn 等）

依赖：`numpy`（可选 `torch`）。默认监听 `127.0.0.1:50001`。

**`recv_preproc_udp.py`**

```python
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Receive Qt PreprocChunk UDP packets (format PBC1 v1) -> numpy window."""

import argparse
import socket
import struct
import sys
from typing import Dict, Optional, Tuple

import numpy as np

HEADER_SIZE = 44
MAGIC = b"PBC1"


def parse_datagram(data: bytes) -> Optional[Tuple[Dict, np.ndarray]]:
    if len(data) < HEADER_SIZE:
        return None
    magic, ver, flags, res, seq_s, seq_e, anchor_ms, anchor_seq, n = struct.unpack(
        "<4sBBHQQqQI", data[:HEADER_SIZE]
    )
    if magic != MAGIC or ver != 1:
        return None
    need = HEADER_SIZE + int(n) * 8
    if len(data) < need:
        return None
    meta = {
        "seq_start": seq_s,
        "seq_end": seq_e,
        "anchor_wall_ms": anchor_ms,
        "anchor_seq": anchor_seq,
        "n": n,
        "flags": flags,
    }
    y = np.frombuffer(data, dtype="<f8", count=int(n), offset=HEADER_SIZE).copy()
    return meta, y


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=50001)
    ap.add_argument("--max", type=int, default=0, help="0 = unlimited chunks")
    ap.add_argument(
        "--print-mode",
        choices=("values", "stats", "both"),
        default="values",
        help="values=打印本组全部采样；stats=仅 mean/std；both=都要",
    )
    ap.add_argument(
        "--precision",
        type=int,
        default=8,
        help="打印浮点小数位数（values/both 时生效）",
    )
    args = ap.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((args.host, args.port))
    print(f"listening udp://{args.host}:{args.port}  (expect magic {MAGIC!r})", flush=True)

    count = 0
    while True:
        data, addr = sock.recvfrom(65535)
        parsed = parse_datagram(data)
        if parsed is None:
            print("skip bad packet", len(data), "from", addr, flush=True)
            continue
        meta, y = parsed
        count += 1
        head = (
            f"chunk#{count} seq=[{meta['seq_start']},{meta['seq_end']}] "
            f"n={meta['n']} anchor_wall_ms={meta['anchor_wall_ms']}"
        )
        print(head, flush=True)

        if args.print_mode in ("stats", "both"):
            print(
                f"  mean={float(y.mean()):.{args.precision}g} "
                f"std={float(y.std()):.{args.precision}g}",
                flush=True,
            )
        if args.print_mode in ("values", "both"):
            # 每一组的全部预处理值（整段窗口）
            with np.printoptions(
                precision=args.precision,
                suppress=True,
                linewidth=160,
                threshold=np.inf,  # 不省略为 ...；窗口很大时终端会很长
            ):
                print(y, flush=True)
            # 若更喜欢「一行一个数」或 CSV，可改用下面两行之一：
            # for i, v in enumerate(y):
            #     print(f"  i={meta['seq_start'] + i}\t{v:.{args.precision}f}", flush=True)
            # print(",".join(f"{v:.{args.precision}f}" for v in y), flush=True)

        # --- 在这里喂你的模型，例如 ---
        # x = torch.from_numpy(y).float().unsqueeze(0).unsqueeze(0)  # [1,1,T]
        # with torch.no_grad():
        #     logits = model(x)
        if args.max > 0 and count >= args.max:
            break

    sock.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
```

**运行顺序**

1. 先起 Python：`python recv_preproc_udp.py --port 50001`（默认 **打印每组全部采样**；仅要看 mean/std 时用 `--print-mode stats`；两者都要用 `--print-mode both`）
2. 再运行 QtBCI，串口采集正常后，每次预处理滑窗完成会打一 UDP 包；终端先打印一行 `chunk#...` 元数据，下一行起为该窗口全部 `double`。

---

### A.4 与模型输入对齐的小提示

- 你当前管道是 **单通道** 向量 `y`；若训练时用 `[B, C, T]`，在 Python 里 `y.reshape(1, 1, -1)` 或按训练时的预处理再归一化。
- **采样率**：标称 215 Hz 仍由 Qt 侧滑窗与 `seq` 定义；Python 端若要做频域特征，请与训练时 `fs` 一致。
- **丢包**：UDP 不保证送达；联调通过后可统计 `seq_start` 是否连续跳变；正式在线推理可再换 TCP/LSL。

---

### A.5 附录：PBF1 / PBP1（频段特征 UDP，与主程序「网络设置」一致）

主程序 `UdpTelemetryHub` 在勾选相应选项时，除 **PBC1** 预处理窗外，还可发送：

#### PBF1（FFT 路径频段功率，`FftResult`）

| 偏移 | 长度 | 类型 | 含义 |
|------|------|------|------|
| 0–3 | 4 | ASCII | 魔数 `PBF1` |
| 4 | 1 | u8 | version = 1 |
| 5–7 | 3 | u8 | reserved = 0 |
| 8–15 | 8 | u64 LE | seqStart |
| 16–23 | 8 | u64 LE | seqEnd |
| 24–31 | 8 | i64 LE | wallMs |
| 32–39 | 8 | f64 本机字节序 | fsUsed |
| 40–43 | 4 | i32 LE | nfft |
| 44–51 | 8 | f64 | delta |
| 52–59 | 8 | f64 | theta |
| 60–67 | 8 | f64 | alpha |
| 68–75 | 8 | f64 | beta |
| 76–83 | 8 | f64 | gamma |
| 84–91 | 8 | f64 | rms |
| 92 | 1 | u8 | label：0=clean，1=artifact |

**总长** 93 字节。整型/小端与 PBC1 头相同；`double` 与 PBC1 载荷相同（本机 IEEE754 字节序，Windows x64 为小端）。

#### PBP1（PSD 路径频段功率，`SpectrumResult`）

头部与 PBF1 相同至偏移 40（其中 **40–43 的 i32 固定为 0**，占位与 PBF1 对齐；无 nfft 语义）。随后在 **44–91** 仍为 delta…rms 六个 `double`，**92–99** 为 **alphaBetaRatio**（`double`），**100** 为 label 字节（同 PBF1）。

**总长** 101 字节。

配置入口见 **`docs/网络设置与多路UDP发送设计.md`**。
