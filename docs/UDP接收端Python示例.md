# QtBCI → Python：UDP 遥测包格式与接收示例

本文说明 **Qt 端 `UdpTelemetryHub` 发出的二进制 UDP 报文**如何在 **Python 3** 侧用 `socket` 接收并解析。实现依据：`Net/udptelemetryhub.cpp`（发送端）、`Net/networkstreamsettings.h`（默认端口与开关）。

---

## 1. 传输概览

| 项目 | 说明 |
|------|------|
| 协议 | **UDP**，一帧一报文（一个 `writeDatagram`） |
| 目标地址 | 由应用内 **网络设置** 写入 `QtBCI.ini`：`network/host`（默认 `127.0.0.1`） |
| 三流三端口 | **预处理曲线**、**FFT 频段**、**PSD 频段** 各占一个端口，避免混包（与 OpenBCI GUI 多端口习惯一致） |
| 字节序 | 整数均为 **小端（little-endian）**；`double` 为 **IEEE754 双精度**，在 x86/x64 上与 **小端** 布局一致，Python 可用 `struct` 的 `<d` / `<q` 等解析 |

默认端口（可在 `QtBCI.ini` 的 `network/port_preproc` 等键修改）：

| 端口（默认） | 魔数（前 4 字节 ASCII） | 含义 |
|--------------|-------------------------|------|
| `50001` | `PBC1` | 预处理后的单通道 **时间序列块**（可变长度） |
| `50002` | `PBF1` | **FFT** 窗口的频段能量 + RMS + 标签 |
| `50003` | `PBP1` | **PSD** 窗口的频段能量 + RMS + α/β 比 + 标签 |

Qt 端需在界面打开 **网络设置**，勾选 **启用 UDP**，并对需要的流勾选 **发送预处理 / 发送 FFT / 发送 PSD**。

---

## 2. 公共报头（8 字节）

三种报文相同：

| 偏移 | 长度 | 类型 | 内容 |
|------|------|------|------|
| 0 | 4 | `bytes` | 魔数：`b'PBC1'` / `b'PBF1'` / `b'PBP1'` |
| 4 | 4 | `uint32` LE | 版本号，当前为 **1** |

---

## 3. `PBC1` — 预处理曲线块（端口默认 50001）

**总长度**：`44 + n * 8` 字节，`n` 为 `double` 样本个数（单通道预处理后的 `y`）。

| 偏移 | 长度 | 类型 | 字段 |
|------|------|------|------|
| 8 | 8 | `uint64` LE | `seq_start` |
| 16 | 8 | `uint64` LE | `seq_end` |
| 24 | 8 | `int64` LE | `anchor_wall_ms`（与触发本窗的最后一个 `RawPacket` 对齐，用于时间轴） |
| 32 | 8 | `uint64` LE | `anchor_seq` |
| 40 | 4 | `uint32` LE | `n`（样本数） |
| 44 | `n*8` | `n` 个 `double` | 预处理后的幅值序列（本机为小端时与 `<n>d` 一致） |

---

## 4. `PBF1` — FFT 结果（端口默认 50002）

**固定长度：93 字节**。

| 偏移 | 长度 | 类型 | 字段 |
|------|------|------|------|
| 8 | 8 | `uint64` LE | `seq_start` |
| 16 | 8 | `uint64` LE | `seq_end` |
| 24 | 8 | `int64` LE | `wall_ms` |
| 32 | 8 | `double` | `fs_used` |
| 40 | 4 | `int32` LE | `nfft` |
| 44 | 8 | `double` | `delta` |
| 52 | 8 | `double` | `theta` |
| 60 | 8 | `double` | `alpha` |
| 68 | 8 | `double` | `beta` |
| 76 | 8 | `double` | `gamma` |
| 84 | 8 | `double` | `rms` |
| 92 | 1 | `uint8` | `label`：`0` = clean，`1` = artifact（含 artifact 字样时为 1） |

---

## 5. `PBP1` — PSD 结果（端口默认 50003）

**固定长度：101 字节**。

| 偏移 | 长度 | 类型 | 字段 |
|------|------|------|------|
| 8 | 8 | `uint64` LE | `seq_start` |
| 16 | 8 | `uint64` LE | `seq_end` |
| 24 | 8 | `int64` LE | `wall_ms` |
| 32 | 8 | `double` | `fs_used` |
| 40 | 4 | `int32` LE | 预留，当前为 **0** |
| 44–91 | 6×8 | `double` | `delta` … `rms`（与 PBF1 相同顺序） |
| 92 | 8 | `double` | `alpha_beta_ratio` |
| 100 | 1 | `uint8` | `label`（同 PBF1） |

---

## 6. Python 最小接收示例（同步 `socket`）

以下示例在 **本机** 监听三个端口；若 Qt 里 `network/host` 指向另一台机器，把 `bind` 地址改为 `0.0.0.0` 并在该机上运行脚本即可。

```python
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""接收 QtBCI UdpTelemetryHub 发出的 PBC1 / PBF1 / PBP1 UDP 报文。"""

import socket
import struct
from typing import Tuple, List

HOST = "127.0.0.1"  # 与 QtBCI.ini 中 network/host 一致；接收端 bind 本机
PORT_PREPROC = 50001
PORT_FFT = 50002
PORT_PSD = 50003


def make_socket(port: int) -> socket.socket:
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, port))
    s.settimeout(1.0)  # 便于 Ctrl+C 退出；生产可改为 None
    return s


def parse_pbc1(data: bytes) -> dict:
    if len(data) < 44:
        raise ValueError(f"PBC1 too short: {len(data)}")
    magic = data[0:4]
    if magic != b"PBC1":
        raise ValueError(f"bad magic {magic!r}")
    ver = struct.unpack_from("<I", data, 4)[0]
    seq_start, seq_end, anchor_wall_ms, anchor_seq, n = struct.unpack_from("<QQqQI", data, 8)
    need = 44 + n * 8
    if len(data) < need:
        raise ValueError(f"PBC1 len {len(data)} need {need} for n={n}")
    y = list(struct.unpack_from("<" + "d" * n, data, 44))
    return {
        "kind": "PBC1",
        "version": ver,
        "seq_start": seq_start,
        "seq_end": seq_end,
        "anchor_wall_ms": anchor_wall_ms,
        "anchor_seq": anchor_seq,
        "n": n,
        "y": y,
    }


def parse_pbf1(data: bytes) -> dict:
    if len(data) != 93:
        raise ValueError(f"PBF1 expect 93 bytes got {len(data)}")
    if data[0:4] != b"PBF1":
        raise ValueError("bad magic")
    ver = struct.unpack_from("<I", data, 4)[0]
    seq_start, seq_end, wall_ms = struct.unpack_from("<QQq", data, 8)
    fs_used, nfft = struct.unpack_from("<di", data, 32)
    delta, theta, alpha, beta, gamma, rms = struct.unpack_from("<6d", data, 44)
    label = data[92]
    return {
        "kind": "PBF1",
        "version": ver,
        "seq_start": seq_start,
        "seq_end": seq_end,
        "wall_ms": wall_ms,
        "fs_used": fs_used,
        "nfft": nfft,
        "bands": {"delta": delta, "theta": theta, "alpha": alpha, "beta": beta, "gamma": gamma},
        "rms": rms,
        "label": "artifact" if label == 1 else "clean",
    }


def parse_pbp1(data: bytes) -> dict:
    if len(data) != 101:
        raise ValueError(f"PBP1 expect 101 bytes got {len(data)}")
    if data[0:4] != b"PBP1":
        raise ValueError("bad magic")
    ver = struct.unpack_from("<I", data, 4)[0]
    seq_start, seq_end, wall_ms = struct.unpack_from("<QQq", data, 8)
    fs_used, _reserved = struct.unpack_from("<di", data, 32)
    delta, theta, alpha, beta, gamma, rms, ab_ratio = struct.unpack_from("<7d", data, 44)
    label = data[100]
    return {
        "kind": "PBP1",
        "version": ver,
        "seq_start": seq_start,
        "seq_end": seq_end,
        "wall_ms": wall_ms,
        "fs_used": fs_used,
        "bands": {"delta": delta, "theta": theta, "alpha": alpha, "beta": beta, "gamma": gamma},
        "rms": rms,
        "alpha_beta_ratio": ab_ratio,
        "label": "artifact" if label == 1 else "clean",
    }


def dispatch(data: bytes) -> dict:
    if len(data) < 4:
        return {"kind": "unknown", "raw_len": len(data)}
    m = data[0:4]
    if m == b"PBC1":
        return parse_pbc1(data)
    if m == b"PBF1":
        return parse_pbf1(data)
    if m == b"PBP1":
        return parse_pbp1(data)
    return {"kind": "unknown", "magic": m, "len": len(data)}


def main() -> None:
    socks: List[Tuple[socket.socket, str]] = [
        (make_socket(PORT_PREPROC), "preproc"),
        (make_socket(PORT_FFT), "fft"),
        (make_socket(PORT_PSD), "psd"),
    ]
    print("Listening:", [(HOST, PORT_PREPROC), (HOST, PORT_FFT), (HOST, PORT_PSD)])
    print("Start QtBCI acquisition with UDP enabled…")
    try:
        while True:
            for s, name in socks:
                try:
                    data, addr = s.recvfrom(65535)
                except socket.timeout:
                    continue
                pkt = dispatch(data)
                pkt["from_addr"] = addr
                pkt["socket"] = name
                # 按需打印或写入队列 / asyncio / numpy 环缓冲
                if pkt.get("kind") == "PBC1":
                    print(f"[{name}] PBC1 n={pkt['n']} seq {pkt['seq_start']}-{pkt['seq_end']}")
                else:
                    print(f"[{name}] {pkt['kind']} seq {pkt['seq_start']}-{pkt['seq_end']}")
    except KeyboardInterrupt:
        print("exit")
    finally:
        for s, _ in socks:
            s.close()


if __name__ == "__main__":
    main()
```

**只用一路数据时**：只 `bind` 对应端口即可（例如只要预处理，只监听 `50001`）。

---

## 7. 常见注意点

1. **先起 Python 监听，再开 Qt 发流**（或无所谓顺序，但 UDP 无连接，丢包无重传，接收端晚启动会丢掉之前的包）。  
2. **端口被占用**：换 Qt 里网络设置端口，或改脚本 `PORT_*`。  
3. **远程机器**：Qt 里填对方 IP；Python 在对方机器上 `bind('0.0.0.0', port)` 或具体网卡 IP；注意防火墙放行 UDP。  
4. **大包**：`PBC1` 随 `n` 增大；若 `n` 极大，注意未分片以太网 MTU（一般 **≤ 65507** 应用层上限，实际建议远小于 1500 字节以免分片）。当前算法窗长默认较小，一般无虞。  
5. **`double` 与 ARM**：若未来在 **大端** 或特殊平台接收，需与 Qt 发送端一致地处理浮点字节序；当前 Windows x64 ↔ Python 3 小端无需额外处理。

---

## 8. 与配置文件的对应关系

`QtBCI.ini` 中与 UDP 相关的键（节选）：

- `network/enabled`、`network/protocol`（`udp`）  
- `network/host`  
- `network/port_preproc`、`network/port_fft`、`network/port_psd`  
- `network/send_preproc`、`network/send_fft`、`network/send_psd`  

Python 侧端口应与上述 **一致**。

---

## 9. 小结

- 三种报文以 **魔数 + 版本** 区分；**PBC1 变长**，**PBF1 / PBP1 定长**。  
- 使用 **`struct.unpack_from` + 小端格式符 `<`** 即可稳定解析。  
- 将本文中的 `dispatch` / `parse_*` 嵌入你的在线推理或绘图流程，即可消费 QtBCI 实时外发的数据。
