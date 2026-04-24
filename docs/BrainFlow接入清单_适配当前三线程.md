# BrainFlow 预处理API接入清单（仅算法层，适配你当前三线程）

> 你的目标是：**只用 BrainFlow 的预处理算法 API（DataFilter）**，采集继续用你自己的串口链路。  
> 所以本清单只改线程B（`AlgorithmEngine`），线程A/线程C保持现状。

---

## 1. 线程职责（按你当前项目）

- **线程A `AcquisitionEngine`**：继续你的串口采集、组帧、解析，输出 `RawPacket`
- **线程B `AlgorithmEngine`**：接收 `RawPacket`，调用 BrainFlow `DataFilter` 做预处理与特征
- **线程C `CsvLogWorker`**：继续写 EEG CSV
- **UI线程**：显示曲线与算法结果，不做重计算

---

## 2. 你只需要引入的 BrainFlow 头文件

```cpp
#include "data_filter.h"
```

> 不需要 `board_shim.h`，因为你不替换采集层。

---

## 3. `AlgorithmEngine` 最小接入示例

把下面逻辑放进你的 `AlgorithmEngine::onRawPacket`。

```cpp
void AlgorithmEngine::onRawPacket(const RawPacket &pkt)
{
    // 1) 先入你现有环形缓存
    RawSample s;
    s.raw = pkt.rawUv;
    s.raw_count = pkt.seq;
    s.time = QString::number(pkt.tsMs);
    m_buf.appendRawvalue(s);

    QVector<RawSample> chunk;
    if (!m_buf.tryDequeueRawChunkForAlgo(m_windowSize, chunk))
        return;

    // 2) 转成连续 double 数组给 DataFilter
    std::vector<double> signal;
    signal.reserve(chunk.size());
    for (const auto &it : chunk)
        signal.push_back(it.raw);

    const int fs = 256; // 按你的设备实际采样率改

    // 3) BrainFlow 预处理链（示例）
    DataFilter::detrend(signal.data(), static_cast<int>(signal.size()),
                        DetrendOperations::LINEAR);

    DataFilter::perform_bandpass(signal.data(), static_cast<int>(signal.size()),
                                 fs, 1.0, 40.0, 4,
                                 FilterTypes::BUTTERWORTH, 0.0);

    DataFilter::remove_environmental_noise(signal.data(), static_cast<int>(signal.size()),
                                           fs, NoiseTypes::FIFTY);

    // 4) 发绘图块
    PlotChunk pc;
    pc.seqStart = chunk.first().raw_count;
    pc.seqEnd = chunk.last().raw_count;
    pc.y = QVector<double>(signal.begin(), signal.end());
    emit plotChunkReady(pc);

    // 5) 发算法结果（这里先示例，后续替换为真实算法输出）
    AlgoResult r;
    r.tsMs = QDateTime::currentMSecsSinceEpoch();
    r.seqEnd = pc.seqEnd;
    r.score = pc.y.isEmpty() ? 0.0 : pc.y.last();
    r.label = QStringLiteral("preprocessed");
    emit algoResultReady(r);
}
```

---

## 4. 常用 DataFilter API（你后面会用到）

- 时域预处理
  - `DataFilter::detrend(...)`
  - `DataFilter::perform_lowpass(...)`
  - `DataFilter::perform_highpass(...)`
  - `DataFilter::perform_bandpass(...)`
  - `DataFilter::perform_bandstop(...)`
  - `DataFilter::remove_environmental_noise(...)`
- 频域
  - `DataFilter::get_psd_welch(...)`
  - `DataFilter::get_band_power(...)`
  - `DataFilter::get_avg_band_powers(...)`
- 其他
  - `DataFilter::perform_downsampling(...)`（聚合，不是抗混叠）

---

## 5. 参数建议（先跑通再调）

- 采样率 `fs`：必须与你真实设备一致（如 256 或 512）
- 窗口长度 `m_windowSize`：
  - 实时绘图：128 点
  - 稳定频域：256 或更大
- 工频：
  - 国内一般 `NoiseTypes::FIFTY`
  - 北美常见 `NoiseTypes::SIXTY`

---

## 6. 你当前架构里的调用顺序

1. 线程A产出 `RawPacket`
2. A -> B（QueuedConnection）调用 `onRawPacket`
3. B里做 DataFilter 预处理
4. B发 `plotChunkReady/algoResultReady` 给 UI
5. 同时 `RawPacket` 继续进日志队列，线程C写CSV

---

## 7. 常见坑（只接 DataFilter 时）

- `fs` 填错：滤波频率和 PSD 都会失真
- 窗口太短：频域结果抖动大
- 在 UI线程做 DataFilter：会卡界面（必须放线程B）
- 忘记 `#include "data_filter.h"` 或 CMake 未链接 BrainFlow 对应库

---

## 8. 最小验收

- `plotChunkReady` 曲线明显比原始更平滑（带通+去工频生效）
- CSV 仍正常写（说明线程C未受影响）
- UI 无明显卡顿（说明 DataFilter 在线程B执行）

---

如果你要，我可以下一步给你“只改 `core/algorithmengine.cpp` 的逐行替换版”。

# BrainFlow 接入清单（适配你当前 Qt 三线程工程）

> 适配对象：你当前 `QtBCI` 的三线程结构  
> - 线程A：`AcquisitionEngine`（采集）  
> - 线程B：`AlgorithmEngine`（预处理/算法）  
> - 线程C：`CsvLogWorker`（写盘）  
> - UI线程：`MainWindow`（交互与绘图）

---

## 1. 线程映射（先定职责）

## 线程A（采集线程）接入 `BoardShim`

- 负责：
  - `prepare_session`
  - `start_stream`
  - 周期 `get_current_board_data`
  - `stop_stream`
  - `release_session`
- 输出：`RawPacket` 信号发给线程B / 日志队列

## 线程B（算法线程）接入 `DataFilter`

- 负责：
  - 对 `RawPacket` 做滤波、去工频、降采样、频域特征等
  - 输出 `PlotChunk` 与 `AlgoResult`

## 线程C（日志线程）保持你现有 `CsvLogWorker`

- 负责：
  - 从 `LogBuffer` 批量写 EEG CSV

## UI线程

- 负责：
  - 开始/停止按钮触发 `invokeMethod`
  - 订阅 `plotChunkReady` 更新图
  - 不做采集/滤波重活

---

## 2. 你需要引入的 BrainFlow 头（C++）

```cpp
#include "board_shim.h"
#include "brainflow_input_params.h"
#include "data_filter.h"
```

> CMake 侧记得链接 `brainflow::Brainflow32`（或你的平台对应目标名）。

---

## 3. 线程A：`AcquisitionEngine` 最小改造

## 3.1 关键成员（示例）

```cpp
// acquisitionengine.h
#include "board_shim.h"
#include "brainflow_input_params.h"

class AcquisitionEngine : public QObject {
    Q_OBJECT
public:
    explicit AcquisitionEngine(QObject *parent = nullptr);

public slots:
    void start(const QString &portName, qint32 baudRate);
    void stop();

signals:
    void rawPacketReady(const RawPacket &pkt);
    void warningMessage(const QString &msg);
    void statusMessage(const QString &msg);

private:
    std::unique_ptr<BoardShim> m_board;
    int m_boardId = 0; // 按你的设备改
    QTimer *m_pollTimer = nullptr;
    quint64 m_seq = 0;
};
```

## 3.2 `start()` 示例（最小）

```cpp
void AcquisitionEngine::start(const QString &portName, qint32 baudRate)
{
    try {
        BrainFlowInputParams params;
        params.serial_port = portName.toStdString();
        // 如果设备要求，可设置 params.ip_port / mac_address / serial_number 等

        // 例：board id 按设备改（可放配置）
        m_boardId = static_cast<int>(BoardIds::SYNTHETIC_BOARD);
        m_board.reset(new BoardShim(m_boardId, params));

        m_board->prepare_session();
        m_board->start_stream(45000, "");

        if (!m_pollTimer) {
            m_pollTimer = new QTimer(this);
            m_pollTimer->setInterval(20); // 50Hz 轮询
            connect(m_pollTimer, &QTimer::timeout, this, [this]() {
                if (!m_board) return;
                // 取最近N点，不清空缓冲
                BrainFlowArray<double, 2> data = m_board->get_current_board_data(32);
                // data 维度：[channels][samples]
                const auto eegChannels = BoardShim::get_eeg_channels(m_boardId);
                if (eegChannels.empty()) return;
                const int ch = eegChannels[0];

                const int samples = data.get_size(1);
                for (int i = 0; i < samples; ++i) {
                    RawPacket p;
                    p.tsMs = QDateTime::currentMSecsSinceEpoch();
                    p.seq = ++m_seq;
                    p.rawUv = data.get_address(ch)[i];
                    p.rawInt16 = 0; // BrainFlow多为double物理量，若需原始int需按设备另取
                    emit rawPacketReady(p);
                }
            });
        }
        m_pollTimer->start();
        emit statusMessage(QStringLiteral("BrainFlow采集已启动"));
    } catch (const BrainFlowException &e) {
        emit warningMessage(QStringLiteral("BrainFlow start失败: %1").arg(e.what()));
    }
}
```

## 3.3 `stop()` 示例

```cpp
void AcquisitionEngine::stop()
{
    try {
        if (m_pollTimer && m_pollTimer->isActive())
            m_pollTimer->stop();
        if (m_board) {
            m_board->stop_stream();
            m_board->release_session();
            m_board.reset();
        }
        emit statusMessage(QStringLiteral("BrainFlow采集已停止"));
    } catch (const BrainFlowException &e) {
        emit warningMessage(QStringLiteral("BrainFlow stop失败: %1").arg(e.what()));
    }
}
```

---

## 4. 线程B：`AlgorithmEngine` 接入 `DataFilter`

你现有 `AlgorithmEngine::onRawPacket` 里可以直接插入 BrainFlow 预处理。

```cpp
void AlgorithmEngine::onRawPacket(const RawPacket &pkt)
{
    RawSample s;
    s.raw = pkt.rawUv;
    s.raw_count = pkt.seq;
    s.time = QString::number(pkt.tsMs);
    m_buf.appendRawvalue(s);

    QVector<RawSample> chunk;
    if (!m_buf.tryDequeueRawChunkForAlgo(m_windowSize, chunk))
        return;

    std::vector<double> signal;
    signal.reserve(chunk.size());
    for (const auto &it : chunk)
        signal.push_back(it.raw);

    // 例1：去趋势 + 带通 + 工频
    DataFilter::detrend(signal.data(), static_cast<int>(signal.size()), DetrendOperations::LINEAR);
    DataFilter::perform_bandpass(signal.data(), static_cast<int>(signal.size()),
                                 256, 1.0, 40.0, 4,
                                 FilterTypes::BUTTERWORTH, 0.0);
    DataFilter::remove_environmental_noise(signal.data(), static_cast<int>(signal.size()),
                                           256, NoiseTypes::FIFTY);

    // 转回绘图块
    PlotChunk pc;
    pc.seqStart = chunk.first().raw_count;
    pc.seqEnd = chunk.last().raw_count;
    pc.y = QVector<double>(signal.begin(), signal.end());
    emit plotChunkReady(pc);

    // 算法结果（示例）
    AlgoResult r;
    r.tsMs = QDateTime::currentMSecsSinceEpoch();
    r.seqEnd = pc.seqEnd;
    r.score = pc.y.isEmpty() ? 0.0 : pc.y.last();
    r.label = QStringLiteral("bf_preprocessed");
    emit algoResultReady(r);
}
```

---

## 5. 线程C：日志线程不用改架构

你现有 `CsvLogWorker` 已可用，建议只修两点：

1. 表头统一：`tsMs`（现在是 `tsMS`）  
2. `LogItem.tsMs` 类型与写入一致（建议 `qint64` 毫秒）

---

## 6. MainWindow 启停顺序（你当前框架可直接套）

## 开始

1. 确定 CSV 路径（若启用）
2. `invoke(m_csvWorker, stop->setOutputPath->start)`
3. `invoke(m_acq, start(port,57600))`

## 停止

1. `invoke(m_acq, stop)`
2. `invoke(m_csvWorker, stop)`

---

## 7. 常见坑（接 BrainFlow 时）

- 忘记 `release_session()`：下次启动可能报已有会话  
- `get_board_data()` 会清空缓冲，与你当前窗口逻辑可能冲突  
- 降采样前建议先低通（`perform_downsampling` 只是聚合）  
- 不同 `board_id` 的 EEG 通道索引不同，要通过 `BoardShim::get_eeg_channels(board_id)` 获取  

---

## 8. 最小验收

1. 点击开始后，`rawPacketReady` 持续触发  
2. `AlgorithmEngine` 能收到并输出 `plotChunkReady`  
3. CSV 连续写行，不只表头  
4. 点击停止后会话释放，二次开始无异常  

---

## 9. 你下一步可以做什么

- 我可以再给你一份“**按你当前文件逐行改动清单**”（`acquisitionengine.*` / `algorithmengine.*` / `mainwindow.cpp`），减少你手动拼接成本。  

