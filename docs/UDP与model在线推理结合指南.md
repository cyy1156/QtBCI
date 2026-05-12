# UDP 预处理流与 `model/` 在线推理结合指南

本文说明：**在已跑通 [UDP本地回环测试接入指南](./UDP本地回环测试接入指南.md)（PBC1 格式、`UDP_Test.py` 收包）的前提下**，如何把每一包里的预处理向量 `y` 接到 **`model/`** 里基于 **`online_infer.py` + EEGNet checkpoint** 的推理流程。  

**本文只给写法与对齐要点，不要求你改 Qt 工程；Python 侧你可新建独立脚本（例如 `model/udp_model_infer.py`）自行粘贴组合。**

---

## 1. 先理清三条「数据含义」

| 来源 | 内容 | 形状直觉 |
|------|------|-----------|
| Qt → UDP（PBC1） | **已在 Qt 里做过 `EegPreprocessPipeline` 的单通道窗口** | 一维 `y`，长度 = `AlgorithmEngine` 的窗口点数（如 128） |
| `model/online_infer.py` 离线仿真 | 原始多通道连续信号 → `EEGStreamSimulator` → `StreamWindowProcessor` → **`EEGPreprocessor.transform_window`** → 模型 | 窗口形状 **`[n_channels, n_samples]`** |
| `checkpoint`（`torch.load`） | 训练时固定的 **`fs`、`n_channels`、`window_samples`（时间点数）**、类别数、`label_names` | 必须与推理张量一致 |

**核心矛盾（你必须二选一或重新训练）：**

1. **预处理是否重复**  
   - UDP 里已是「Qt 预处理后的曲线」。  
   - `utils/preprocessing.py` 里的 **`EEGPreprocessor.transform_window`** 默认还会做占位滤波 + 可选 **逐窗 z-score**（见 `raw_window_zscore`）。  
   - 若训练时用的是「原始 µV → Python 预处理」，则 **UDP 路径不应再套同一套 Python 预处理**，否则分布与训练不一致。  
   - 若你**有意**用 Qt 完全复现训练链路上的最后一步，应在 checkpoint 的 `config["preprocessing"]` 里核对是否启用 z-score，并与 Qt 侧约定「只做一次」。

2. **通道数与窗口长度必须和 checkpoint 一致**  
   - `eegnet.py` 期望输入 **`[batch, 1, n_channels, n_samples]`**（见类注释）。  
   - `online_infer.run_stream_inference` 里典型写法为：  
     `processed_window[None, None, :, :]` → **`[1, 1, C, T]`**。  
   - 你的 UDP 当前是 **单通道** `y`（长度 `T_udp`）。若 checkpoint 里 **`n_channels == 8`**（`config.py` 默认多导联），则不能直接把 `y` 当 8 通道用；需要 **重新训练单通道模型**，或在协议里扩展为多通道再对齐训练格式（本文不展开硬件改造）。

3. **采样率与时间窗**  
   - checkpoint 里的 **`fs`**（如 128）来自训练配置。  
   - Qt 侧标称 **215 Hz**、窗口 **128 点**，与「128 Hz 下 2 秒窗」等训练设定 **不是同一个物理时长**。  
   - **结论**：要么训练时使用与在线相同的 **点数 `window_samples` + 同一预处理语义**，要么只把 UDP 当「长度恰好等于 `window_samples` 的张量」喂给模型（强行对齐形状），**准确率需自行验证**。

---

## 2. `model/` 里你应该复用哪些代码（按文件）

以下路径相对于仓库中的 **`model/`** 目录（与 `online_infer.py` 同级）。

| 文件 | 建议用法 |
|------|-----------|
| **`online_infer.py`** | **`build_model_from_checkpoint(checkpoint, config)`**：构建 `EEGNet` 并 `load_state_dict`；**`build_preprocessor(config, fs)`**：得到 **`EEGPreprocessor`**；**`apply_saved_pipeline_settings(config, saved_config)`**：用 checkpoint 里存的配置覆盖当前 `ProjectConfig`，避免训推不一致。 |
| **`config.py`** | **`build_default_config()`**、`ProjectConfig`；checkpoint 里没有的字段用默认补全。 |
| **`utils/preprocessing.py`** | **`EEGPreprocessor.transform_window(window)`**，输入 **`numpy` 形状 `[C, T]`**。 |
| **`utils/streaming.py`** | 可选：**`PredictionSmoother`**，与 `online_infer.run_stream_inference` 里用法一致，对每帧概率做平滑。 |
| **`UDP_Test.py`**（你已有） | 保留 **`parse_datagram`**（PBC1 解析）；把「打印」换成「推理」即可，不必重复写协议。 |

**不必**在 UDP 场景里调用 **`EEGStreamSimulator`** / **`StreamWindowProcessor`**，因为 **Qt 已经按窗打好包**；它们是离线长序列仿真用的。

---

## 3. 推荐整合步骤（你自己写脚本）

### 步骤 A：加载 checkpoint 与配置

逻辑与 **`online_infer.run_online_inference`** 前半段一致（可抄 **`torch.load` + `apply_saved_pipeline_settings` + `config.data.fs = checkpoint["fs"]`**）。

你需要从 `checkpoint` 读出至少：

- `checkpoint["n_channels"]`、`checkpoint["window_samples"]`、`checkpoint["num_classes"]`
- `checkpoint["label_names"]`
- `checkpoint.get("fs")`
- `checkpoint.get("config", {})` 用于 **`apply_saved_pipeline_settings`**

然后用 **`build_model_from_checkpoint(checkpoint, config)`** 得到 **`model`, `device`**。

### 步骤 B：每收到一包 UDP，检查形状

解析得到 **`meta`, `y`** 后：

1. `assert y.ndim == 1`  
2. `len(y)` 应等于 **`checkpoint["window_samples"]`**；若不等，说明 Qt 窗口长度与训练不一致，需要改 Qt 的 `setWindowSize` 或重训模型，而不是强行 pad（除非你清楚自己在做什么）。

### 步骤 C：组成 `[C, T]` 再 preprocessing

- 若 **`n_channels == 1`**（且训练也是单通道）：  
  `window_2d = y.astype(np.float32).reshape(1, -1)`
- 若 checkpoint **`n_channels > 1`** 且你仍只有一条 UDP 曲线：**不能**在未重训的情况下随意复制成多通道；应与训练数据格式一致后再推理。

然后：

```python
processed = preprocessor.transform_window(window_2d)  # [C, T]
```

**若确定 Qt 输出已是最终输入、不想再 z-score**：有两种做法（二选一，保持与训练一致即可）：

- 临时在内存里把 `config.preprocessing.raw_window_zscore` 设为 `False` 再 `build_preprocessor`；或  
- 跳过 `transform_window`，直接用 `window_2d`（**仅当**训练脚本也是 Raw 且未用该项）。

### 步骤 D：前向与 Softmax

与 **`online_infer.run_stream_inference`** 中一致：

```python
import torch

with torch.no_grad():
    x = torch.from_numpy(processed[None, None, :, :].astype(np.float32)).to(device)
    logits = model(x)
    probs = torch.softmax(logits, dim=1).cpu().numpy()[0]
```

`label_names` 用 checkpoint 中的列表 **`pred = label_names[int(np.argmax(probs))]`**。

### 步骤 E（可选）：平滑

```python
from utils.streaming import PredictionSmoother

smoother = PredictionSmoother(mode="probability_mean", window_size=5)  # 与线上一致可自行改
smoothed = smoother.update(probs)
```

---

## 4. 最小「伪代码骨架」（新建文件自行填空）

下面展示如何把 **`recvfrom` 循环** 与 **推理** 拼在一起；**不要**与项目里的文件自动合并，仅作模板。

```python
# 新建例如 model/udp_model_infer.py，在 model 目录下运行：
#   python udp_model_infer.py --checkpoint artifacts/checkpoints/xxx.pt --port 50001

import argparse
import socket
import sys

import numpy as np
import torch

from online_infer import (
    apply_saved_pipeline_settings,
    build_model_from_checkpoint,
    build_preprocessor,
    build_default_config,
)
from config import ProjectConfig  # 若需类型注解

# 把 UDP_Test.py 里的 parse_datagram、MAGIC、HEADER_SIZE 复制过来，或：
# from UDP_Test import parse_datagram


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=50001)
    ap.add_argument("--checkpoint", required=True)
    args = ap.parse_args()

    ckpt = torch.load(args.checkpoint, map_location="cpu", weights_only=False)
    config = build_default_config()
    apply_saved_pipeline_settings(config, ckpt.get("config", {}))
    if ckpt.get("fs") is not None:
        config.data.fs = int(ckpt["fs"])

    model, device = build_model_from_checkpoint(ckpt, config)
    preprocessor = build_preprocessor(config, int(config.data.fs))

    label_names = [str(x) for x in ckpt["label_names"]]
    n_ch = int(ckpt["n_channels"])
    win = int(ckpt["window_samples"])

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((args.host, args.port))
    print("listening", args.host, args.port, "expect window_samples=", win, "n_channels=", n_ch)

    try:
        while True:
            data, addr = sock.recvfrom(65535)
            parsed = parse_datagram(data)  # 来自 UDP_Test / 指南附录
            if parsed is None:
                continue
            meta, y = parsed
            if int(y.size) != win:
                print("skip: len", y.size, "!= checkpoint window_samples", win)
                continue
            # 单通道训练：window_2d 形状 [1, win]。若 n_ch>1 必须由真实多通道数据构造，不能强行 reshape 一维 y。
            if n_ch != 1:
                print("skip: checkpoint expects n_channels=%d but UDP sends 1 channel; train a 1-ch model or extend UDP." % n_ch)
                continue
            window_2d = y.astype(np.float32).reshape(1, -1)

            processed = preprocessor.transform_window(window_2d)
            x = torch.from_numpy(processed[None, None, :, :].astype(np.float32)).to(device)
            with torch.no_grad():
                logits = model(x)
                probs = torch.softmax(logits, dim=1).cpu().numpy()[0]
            pred = label_names[int(np.argmax(probs))]
            print(meta.get("seq_end"), pred, probs)
    finally:
        sock.close()


if __name__ == "__main__":
    main()
```

**注意**：示例里已用 **`if n_ch != 1: continue`** 拒绝与「单通道 UDP」不匹配的 checkpoint；若你的 checkpoint 是多通道，必须改为 **多通道 UDP 协议** 或 **重新训练单通道模型** 后再推理。模板顶部需自行 **`from UDP_Test import parse_datagram`**（或复制该函数），否则会 **NameError**。

---

## 5. 与《UDP本地回环测试接入指南》的衔接点

1. **二进制格式**：严格使用指南附录 **PBC1 v1**（`parse_datagram`、`"<4sBBHQQqQI"`、小端 double）。  
2. **端口**：Qt `setTarget(LocalHost, port)` 与 Python `bind` **同一端口**。  
3. **退出时释放端口**：脚本里用 **`with socket.socket(...) as sock`** 或 **`try/finally: sock.close()`**，避免 WinError 10048（见前文对话）。  
4. **打印每组数值**：调试时用指南里的 **`np.printoptions`**；接模型后改为打印 **`pred` / `probs`** 即可。

---

## 6. 自检清单（接模型前逐项确认）

- [ ] `len(y) == checkpoint["window_samples"]`  
- [ ] `y`  reshape 后的 **`n_channels`** 与 **`checkpoint["n_channels"]`** 一致  
- [ ] **预处理只做一次**：Qt 与 Python **`EEGPreprocessor`** 是否与训练流程一致  
- [ ] **`fs`**：训练 checkpoint 的 `fs` 与 Qt 物理采样率不必相等，但 **窗口语义**（几秒、多少点）要与训练约定一致  
- [ ] **`label_names`** 顺序与训练时类别索引一致（不要用错映射）

---

## 7. 延伸阅读（仓库内）

- 离线整条推理仿真：**`model/online_infer.py`** 中的 **`run_stream_inference`**  
- 串口步态联动示例：**`model/eeg_walker_online_control.py`**（从文件仿真改为 UDP 时，替换数据源即可）  

完成上述对齐后，你就实现了：**Qt 预处理窗 → UDP → Python 解析 → EEGNet → 类别概率**，与文档中的协议描述一致。
