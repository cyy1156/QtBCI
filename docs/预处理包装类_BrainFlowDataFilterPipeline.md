# 预处理包装类示例（基于 `preprocessing/data_filter.cpp` 的 DataFilter API）

> 目标：把 BrainFlow `DataFilter`（你已放在 `QtBCI/preprocessing/data_filter.*`）封装成一个“**可复用的预处理流水线类**”，按你指定的流程：
>
> **陷波滤波 → 带通滤波 → 降采样 → 检测并标记坏道 → 好通道 ICA 去伪影 → 坏道插值 → 平均重参考**
>
> 同时解决你说的 “采样率在 250 左右但不稳定、没有精确值” 的问题：**在线估计采样率 + 平滑 + 限幅**，保证滤波参数稳定。

---

## 一、重要前提（你当前项目的现实约束）

### 1）单通道 vs 多通道：处理逻辑有本质区别（必须先讲清楚）

单通道设备和多通道帽子的处理逻辑有本质区别。多通道处理的核心是**空间冗余性**（ICA 找共同成分、插值靠邻居、重参考靠全脑平均），而单通道设备完全不具备这种空间信息。

因此，对于**单通道设备**，本 MD 开头列出的“多通道完整流程”里只有一半适用，另一半必须舍弃或寻找替代方案。

#### ❌ 单通道必须删除或跳过的步骤

1. **检测并标记坏道**：只有一个通道，如果坏了就是实验失败，没有其他通道可以替代或插值。只能通过目视/规则检查判断这段数据是否可用。
2. **ICA 去除伪影**：ICA 需要通道数足够（至少 2）。单通道做 ICA 没有数学意义，会导致波形畸变，**单通道严禁做 ICA**。
3. **坏道插值**：没有邻居通道，无法进行空间插值（球面样条等）。
4. **平均重参考**：平均参考对单通道等价于把信号减去自身平均，极易把信号“压扁/趋零”，没有意义，必须跳过。

#### ✅ 单通道设备正确的预处理流程（推荐）

单通道应该简化为**纯时序滤波 + 伪影剔除/回归（可选）**：

- **陷波滤波**：保留（去 50Hz 工频）
- **带通滤波**：保留（去直流漂移和高频肌电噪声）
- **降采样**：可选（节省算力/便于显示）
- **去伪影（替代 ICA）**：改用阈值剔除 / 参考回归 / 小波阈值去噪（见下节）
- **插值/重参考**：跳过

#### 🛠️ 单通道去伪影的替代方案（关键）

既然不能做 ICA，面对眨眼和肌肉伪影，单通道只能用以下方式之一（或组合）：

1. **粗暴剔除（最常用）**
   - 逻辑：用阈值或人工标记，把体动/眨眼的大脉冲时间段**整段剔除**。
   - 后果：数据不连续；功率谱等分析需谨慎（可改为分段统计）。

2. **参考回归（需要参考传感器）**
   - 前提：你同时记录了 EOG/加速度计等参考通道。
   - 逻辑：干净信号 = EEG - \( \beta \times \) 参考通道。

3. **小波阈值去噪（只有纯 EEG 单通道时可用）**
   - 逻辑：离散小波变换，削弱突发尖峰伪影系数，再重构。
   - 风险：可能同时抹掉高频脑电，需严格验证。

### 2）BrainFlow 的 `DataFilter` 是“算法封装”，但坏道检测/插值/重参考并不是全部都有现成函数
`DataFilter` 提供了：
- 滤波：`perform_bandpass / perform_bandstop / remove_environmental_noise`
- 降采样：`perform_downsampling`
- ICA：`perform_ica`

但下面这些通常要你自己做一点逻辑：
- **坏道检测**：按方差、峰峰值、饱和率、异常噪声等指标做规则。
- **坏道插值**：没有电极空间坐标时，只能做简化插值（例如用好通道均值/中值替代）。
- **平均重参考**：实现很简单：每个时刻减去所有好通道的均值。

本 MD 给的是“你能直接用”的工程化包装：`DataFilter` 负责算法核，剩下的由 wrapper 完成。

---

## 二、采样率不稳定：在线估计 + 平滑（强烈建议）

你现在 pipeline 的滤波/陷波都依赖 `fs`（采样率）。如果采样率没定：
- **先用标称值**：250Hz
- **每隔一段时间估计一次真实 fs**（用 `seq` 或时间戳差）
- **做平滑**，避免抖动导致滤波参数频繁变化

估计公式（推荐用 `seq` + `tsMs`）：

\[
fs\_{inst} = \frac{(\Delta seq)\times 1000}{\Delta t\_{ms}}
\]

再平滑：

\[
fs\_{smooth} = \alpha \cdot fs\_{smooth} + (1-\alpha)\cdot fs\_{inst}
\]

最后限幅（比如 200~300）避免异常值污染。

---

## 三、预处理包装类：头文件示例（建议放到 `QtBCI/preprocessing/`）

你可以创建：`preprocessing/eeg_preprocess_pipeline.h`

```cpp
#pragma once

#include <QtGlobal>
#include <QVector>
#include <QString>

#include <vector>

// 你现在项目里已有这个（来自 QtBCI/preprocessing/data_filter.h）
#include "data_filter.h"

// BrainFlow 里用于 ICA 的数组包装
#include "brainflow_array.h"

// BrainFlow 常量枚举（FilterTypes / NoiseTypes / DetrendOperations 等）
#include "brainflow_constants.h"

class EegPreprocessPipeline
{
public:
    struct Options
    {
        // -------- 采样率估计 --------
        double fs_nominal = 250.0;   // 标称值（没有精确值时先用这个）
        double fs_min = 200.0;       // 限幅最小
        double fs_max = 300.0;       // 限幅最大
        double fs_alpha = 0.90;      // 指数平滑系数（越大越稳）

        // -------- 滤波参数 --------
        bool enable_notch = true;
        // 国内一般 50Hz；若你在 60Hz 环境改成 SIXTY
        int env_noise_type = (int)NoiseTypes::FIFTY;

        bool enable_bandpass = true;
        double bp_low_hz = 1.0;
        double bp_high_hz = 40.0;
        int bp_order = 4;
        int bp_filter_type = (int)FilterTypes::BUTTERWORTH_ZERO_PHASE;
        double bp_ripple = 0.0;

        // -------- 降采样 --------
        bool enable_downsample = false;
        int downsample_factor = 2;   // 2 表示 fs/2
        // 0=mean 1=median ... 取决于 BrainFlow 的 AggOperations（若不确定先用 0）
        int downsample_agg = 0;

        // -------- 坏道检测（多通道才有意义；单通道默认关闭）--------
        bool enable_bad_channel_detection = false;
        // 峰峰值阈值（单位：µV），超过视为坏道（可根据你设备调整）
        double bad_ptp_uv = 800.0;
        // 方差阈值（单位：µV^2），超过视为坏道
        double bad_var_uv2 = 200000.0;
        // rail 百分比阈值（0~1），超过视为坏道
        double bad_railed_ratio = 0.30;
        int railed_gain = 1;

        // -------- ICA（多通道才有意义；单通道严禁开启）--------
        bool enable_ica = false;
        // 组件数建议 <= 好通道数
        int ica_num_components = 0; // 0 表示自动（用好通道数）

        // -------- 坏道插值（多通道才有意义；单通道默认关闭）--------
        bool enable_bad_channel_interpolation = false;

        // -------- 平均重参考（多通道才有意义；单通道默认关闭）--------
        bool enable_average_rereference = false;
    };

    struct FsHint
    {
        // 用于估计采样率：序号与毫秒时间戳
        // 你现在 RawPacket 有 seq、tsMs（如果 tsMs 是 QString 也没关系，你可以在调用前转成 ms）
        qint64 seq = -1;
        qint64 ts_ms = -1;
    };

    struct Result
    {
        // 输出：channels x samples（每个通道一条）
        QVector<QVector<double>> data_uv;
        // 坏道标记：true=坏
        QVector<bool> bad_channel;
        // 本次使用的采样率（平滑后）
        double fs_used = 0.0;
        // 文本说明（便于 UI 打印）
        QString debug;
    };

public:
    explicit EegPreprocessPipeline(const Options &opt = Options());

    void setOptions(const Options &opt);
    const Options &options() const;

    // 输入：channels x samples（µV）
    // hint：用于估计采样率（可传无效值，pipeline 会退化为 fs_nominal）
    Result processEpoch(const QVector<QVector<double>> &epoch_uv, const FsHint &hint);

    // 当前平滑采样率
    double fsEstimate() const;

    // 重置内部状态（例如切换设备/重新开始）
    void reset();

private:
    // 采样率估计
    void updateFsEstimate(const FsHint &hint, QString &dbg);
    static double clamp(double v, double lo, double hi);

    // 坏道检测
    QVector<bool> detectBadChannels(const QVector<QVector<double>> &data_uv, int fs, QString &dbg) const;
    static void calcPtpAndVar(const QVector<double> &x, double &ptp, double &var);

    // ICA（只对好通道）
    void runIcaOnGoodChannels(QVector<QVector<double>> &data_uv,
                              const QVector<bool> &bad,
                              QString &dbg) const;

    // 坏道插值（简化：用好通道平均替代）
    void interpolateBadChannels(QVector<QVector<double>> &data_uv,
                                const QVector<bool> &bad,
                                QString &dbg) const;

    // 平均重参考
    void averageRereference(QVector<QVector<double>> &data_uv,
                            const QVector<bool> &bad,
                            QString &dbg) const;

private:
    Options m_opt;
    double m_fs_smooth;
    bool m_has_last;
    FsHint m_last;
};
```

---

## 四、预处理包装类：实现文件示例

你可以创建：`preprocessing/eeg_preprocess_pipeline.cpp`

> 说明：下面代码刻意写得“工程可用”，但仍然是示例级别：
> - 坏道插值用 **好通道平均值替代**（没有电极坐标就只能先这样）
> - ICA 结果默认直接用 `S`（独立成分）替换原数据并不一定符合你的“去伪影”目标；真实项目需要识别伪影成分并回投。这里给你“先跑通 + 可扩展的骨架”。

```cpp
#include "eeg_preprocess_pipeline.h"

#include <QtMath>

EegPreprocessPipeline::EegPreprocessPipeline(const Options &opt)
    : m_opt(opt),
      m_fs_smooth(opt.fs_nominal),
      m_has_last(false)
{
}

void EegPreprocessPipeline::setOptions(const Options &opt)
{
    m_opt = opt;
    // 不强制覆盖已有估计值，但你也可以选择 reset()
}

const EegPreprocessPipeline::Options &EegPreprocessPipeline::options() const
{
    return m_opt;
}

double EegPreprocessPipeline::fsEstimate() const
{
    return m_fs_smooth;
}

void EegPreprocessPipeline::reset()
{
    m_fs_smooth = m_opt.fs_nominal;
    m_has_last = false;
    m_last = FsHint{};
}

double EegPreprocessPipeline::clamp(double v, double lo, double hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void EegPreprocessPipeline::updateFsEstimate(const FsHint &hint, QString &dbg)
{
    // hint 无效就不更新
    if (hint.seq < 0 || hint.ts_ms <= 0)
        return;

    if (!m_has_last)
    {
        m_last = hint;
        m_has_last = true;
        return;
    }

    const qint64 dseq = hint.seq - m_last.seq;
    const qint64 dtms = hint.ts_ms - m_last.ts_ms;

    m_last = hint;

    if (dseq <= 0 || dtms <= 0)
        return;

    const double fs_inst = (double)dseq * 1000.0 / (double)dtms;
    const double fs_limited = clamp(fs_inst, m_opt.fs_min, m_opt.fs_max);

    m_fs_smooth = m_opt.fs_alpha * m_fs_smooth + (1.0 - m_opt.fs_alpha) * fs_limited;
    dbg += QStringLiteral("fs_inst=%1 fs_use=%2; ").arg(fs_inst, 0, 'f', 2).arg(m_fs_smooth, 0, 'f', 2);
}

void EegPreprocessPipeline::calcPtpAndVar(const QVector<double> &x, double &ptp, double &var)
{
    if (x.isEmpty())
    {
        ptp = 0.0;
        var = 0.0;
        return;
    }

    double mn = x[0], mx = x[0];
    double sum = 0.0;
    for (double v : x)
    {
        if (v < mn) mn = v;
        if (v > mx) mx = v;
        sum += v;
    }
    const double mean = sum / (double)x.size();

    double sum2 = 0.0;
    for (double v : x)
    {
        const double d = v - mean;
        sum2 += d * d;
    }
    ptp = mx - mn;
    var = sum2 / (double)x.size();
}

QVector<bool> EegPreprocessPipeline::detectBadChannels(const QVector<QVector<double>> &data_uv, int fs, QString &dbg) const
{
    QVector<bool> bad;
    bad.resize(data_uv.size());
    bad.fill(false);

    if (!m_opt.enable_bad_channel_detection)
        return bad;

    for (int ch = 0; ch < data_uv.size(); ++ch)
    {
        const QVector<double> &x = data_uv[ch];

        double ptp = 0.0, var = 0.0;
        calcPtpAndVar(x, ptp, var);

        // BrainFlow 的 rail 检测基于 gain 估算饱和比例，这里只是一个参考指标
        double rail_ratio = 0.0;
        if (!x.isEmpty())
        {
            std::vector<double> tmp;
            tmp.reserve(x.size());
            for (double v : x) tmp.push_back(v);

            // 注意：get_railed_percentage 的定义与 gain 有关，你需要按实际设备调整
            rail_ratio = DataFilter::get_railed_percentage(tmp.data(), (int)tmp.size(), m_opt.railed_gain);
        }

        bool is_bad = false;
        if (ptp > m_opt.bad_ptp_uv) is_bad = true;
        if (var > m_opt.bad_var_uv2) is_bad = true;
        if (rail_ratio > m_opt.bad_railed_ratio) is_bad = true;

        bad[ch] = is_bad;
        dbg += QStringLiteral("[badchk ch%1 ptp=%2 var=%3 rail=%4] ")
                   .arg(ch)
                   .arg(ptp, 0, 'f', 1)
                   .arg(var, 0, 'f', 1)
                   .arg(rail_ratio, 0, 'f', 2);
    }
    return bad;
}

void EegPreprocessPipeline::runIcaOnGoodChannels(QVector<QVector<double>> &data_uv,
                                                 const QVector<bool> &bad,
                                                 QString &dbg) const
{
    if (!m_opt.enable_ica)
        return;

    // 统计好通道
    QVector<int> goodIndex;
    for (int ch = 0; ch < bad.size(); ++ch)
        if (!bad[ch])
            goodIndex.push_back(ch);

    // 单通道/好通道太少，ICA 没意义
    if (goodIndex.size() < 2)
    {
        dbg += QStringLiteral("ICA skipped(good_ch<2); ");
        return;
    }

    // 组装 BrainFlowArray<double,2> 需要连续内存：rows=channels, cols=samples
    const int rows = goodIndex.size();
    const int cols = data_uv[goodIndex[0]].size();
    if (cols <= 0)
        return;

    std::vector<double> mat;
    mat.resize((size_t)rows * (size_t)cols);

    for (int r = 0; r < rows; ++r)
    {
        const QVector<double> &src = data_uv[goodIndex[r]];
        if (src.size() != cols)
        {
            dbg += QStringLiteral("ICA skipped(mismatched cols); ");
            return;
        }
        for (int c = 0; c < cols; ++c)
            mat[(size_t)r * (size_t)cols + (size_t)c] = src[c];
    }

    BrainFlowArray<double, 2> bf(mat.data(), rows, cols);

    const int num_comp = (m_opt.ica_num_components > 0)
        ? qMin(m_opt.ica_num_components, rows)
        : rows;

    // BrainFlow 的 ICA 返回 (W,K,A,S)，这里用 S（独立成分）作为“去混合后信号”示意
    // 真正“去伪影”一般是：识别坏成分 -> 置零 -> 通过 A 回投到传感器空间。
    auto t = DataFilter::perform_ica(bf, num_comp);
    BrainFlowArray<double, 2> s = std::get<3>(t); // S: num_comp x cols

    // 简化：如果 num_comp==rows，则 S 维度和 rows 相同，可直接替换好通道
    if ((int)s.get_size(0) == rows && (int)s.get_size(1) == cols)
    {
        for (int r = 0; r < rows; ++r)
        {
            QVector<double> &dst = data_uv[goodIndex[r]];
            for (int c = 0; c < cols; ++c)
                dst[c] = s.at(r, c);
        }
        dbg += QStringLiteral("ICA applied(num_comp=%1); ").arg(num_comp);
    }
    else
    {
        dbg += QStringLiteral("ICA result shape mismatch; ");
    }
}

void EegPreprocessPipeline::interpolateBadChannels(QVector<QVector<double>> &data_uv,
                                                   const QVector<bool> &bad,
                                                   QString &dbg) const
{
    if (!m_opt.enable_bad_channel_interpolation)
        return;

    if (data_uv.isEmpty())
        return;

    // 计算好通道平均，作为“插值”基线
    QVector<int> goodIndex;
    for (int ch = 0; ch < bad.size(); ++ch)
        if (!bad[ch])
            goodIndex.push_back(ch);

    if (goodIndex.isEmpty())
    {
        dbg += QStringLiteral("interp skipped(no good ch); ");
        return;
    }

    const int cols = data_uv[0].size();
    QVector<double> mean(cols, 0.0);
    for (int idx : goodIndex)
    {
        const QVector<double> &x = data_uv[idx];
        if (x.size() != cols)
            return;
        for (int c = 0; c < cols; ++c)
            mean[c] += x[c];
    }
    for (int c = 0; c < cols; ++c)
        mean[c] /= (double)goodIndex.size();

    int replaced = 0;
    for (int ch = 0; ch < bad.size(); ++ch)
    {
        if (!bad[ch]) continue;
        QVector<double> &dst = data_uv[ch];
        if (dst.size() != cols) continue;
        dst = mean;
        replaced++;
    }
    dbg += QStringLiteral("interp bad ch=%1; ").arg(replaced);
}

void EegPreprocessPipeline::averageRereference(QVector<QVector<double>> &data_uv,
                                               const QVector<bool> &bad,
                                               QString &dbg) const
{
    if (!m_opt.enable_average_rereference)
        return;

    if (data_uv.isEmpty())
        return;

    QVector<int> goodIndex;
    for (int ch = 0; ch < bad.size(); ++ch)
        if (!bad[ch])
            goodIndex.push_back(ch);

    if (goodIndex.isEmpty())
    {
        dbg += QStringLiteral("avg_ref skipped(no good ch); ");
        return;
    }

    const int cols = data_uv[0].size();
    QVector<double> mean(cols, 0.0);

    for (int idx : goodIndex)
    {
        const QVector<double> &x = data_uv[idx];
        if (x.size() != cols)
            return;
        for (int c = 0; c < cols; ++c)
            mean[c] += x[c];
    }
    for (int c = 0; c < cols; ++c)
        mean[c] /= (double)goodIndex.size();

    for (int ch = 0; ch < data_uv.size(); ++ch)
    {
        QVector<double> &x = data_uv[ch];
        if (x.size() != cols)
            continue;
        for (int c = 0; c < cols; ++c)
            x[c] -= mean[c];
    }
    dbg += QStringLiteral("avg_ref applied; ");
}

EegPreprocessPipeline::Result EegPreprocessPipeline::processEpoch(const QVector<QVector<double>> &epoch_uv,
                                                                  const FsHint &hint)
{
    Result out;
    out.data_uv = epoch_uv;

    QString dbg;
    updateFsEstimate(hint, dbg);

    // 使用平滑采样率（四舍五入成 int 给 DataFilter）
    const int fs = (int)qRound(m_fs_smooth);
    out.fs_used = (double)fs;

    if (out.data_uv.isEmpty() || out.data_uv[0].isEmpty())
    {
        out.bad_channel = QVector<bool>(out.data_uv.size(), false);
        out.debug = dbg + QStringLiteral("empty epoch; ");
        return out;
    }

    // 0) 可选：先去趋势（你没写在目标步骤里，但在实际工程中通常建议）
    // 如需开启：对每个通道调用 DataFilter::detrend

    // 1) 陷波/工频抑制（Notch）
    if (m_opt.enable_notch)
    {
        for (int ch = 0; ch < out.data_uv.size(); ++ch)
        {
            QVector<double> &x = out.data_uv[ch];
            std::vector<double> tmp;
            tmp.reserve(x.size());
            for (double v : x) tmp.push_back(v);

            DataFilter::remove_environmental_noise(tmp.data(), (int)tmp.size(), fs, m_opt.env_noise_type);

            for (int i = 0; i < x.size(); ++i)
                x[i] = tmp[(size_t)i];
        }
        dbg += QStringLiteral("notch(ok); ");
    }

    // 2) 带通滤波
    if (m_opt.enable_bandpass)
    {
        for (int ch = 0; ch < out.data_uv.size(); ++ch)
        {
            QVector<double> &x = out.data_uv[ch];
            std::vector<double> tmp;
            tmp.reserve(x.size());
            for (double v : x) tmp.push_back(v);

            DataFilter::perform_bandpass(tmp.data(), (int)tmp.size(), fs,
                                         m_opt.bp_low_hz, m_opt.bp_high_hz,
                                         m_opt.bp_order, m_opt.bp_filter_type, m_opt.bp_ripple);

            for (int i = 0; i < x.size(); ++i)
                x[i] = tmp[(size_t)i];
        }
        dbg += QStringLiteral("bandpass(%1-%2Hz ok); ").arg(m_opt.bp_low_hz).arg(m_opt.bp_high_hz);
    }

    // 3) 降采样（注意：降采样会改变 fs）
    if (m_opt.enable_downsample && m_opt.downsample_factor > 1)
    {
        QVector<QVector<double>> ds;
        ds.resize(out.data_uv.size());

        int filtered_size = 0;
        for (int ch = 0; ch < out.data_uv.size(); ++ch)
        {
            const QVector<double> &x = out.data_uv[ch];
            std::vector<double> tmp;
            tmp.reserve(x.size());
            for (double v : x) tmp.push_back(v);

            filtered_size = 0;
            double *filtered = DataFilter::perform_downsampling(tmp.data(), (int)tmp.size(),
                                                                m_opt.downsample_factor,
                                                                m_opt.downsample_agg,
                                                                &filtered_size);
            ds[ch].resize(filtered_size);
            for (int i = 0; i < filtered_size; ++i)
                ds[ch][i] = filtered[i];
            delete[] filtered;
        }

        out.data_uv = ds;
        out.fs_used = out.fs_used / (double)m_opt.downsample_factor;
        dbg += QStringLiteral("downsample(x%1 ok); ").arg(m_opt.downsample_factor);
    }

    // 4) 检测并标记坏道（在降采样后做也可以，成本更低）
    out.bad_channel = detectBadChannels(out.data_uv, (int)qRound(out.fs_used), dbg);

    // 5) ICA 去伪影（只在好通道上做；单通道会自动跳过）
    runIcaOnGoodChannels(out.data_uv, out.bad_channel, dbg);

    // 6) 坏道插值（简化实现：坏道用好通道均值替换）
    interpolateBadChannels(out.data_uv, out.bad_channel, dbg);

    // 7) 平均重参考（所有通道减去好通道均值）
    averageRereference(out.data_uv, out.bad_channel, dbg);

    out.debug = dbg;
    return out;
}
```

---

## 五、如何在你当前 `AlgorithmEngine` 里使用（示例）

你现在 `AlgorithmEngine::onRawPacket()` 里是单通道 `signal`。要接入这个 pipeline，有两种方式：

### 方式 A：单通道也走 pipeline（默认只做：陷波/带通/降采样；严禁 ICA/坏道/插值/平均参考）

```cpp
// 假设你在 AlgorithmEngine 里有成员：
// EegPreprocessPipeline m_pp;
// qint64 m_lastSeq, m_lastTsMs ...

QVector<QVector<double>> epoch;
epoch.resize(1);
epoch[0].resize((int)signal.size());
for (int i = 0; i < (int)signal.size(); ++i)
    epoch[0][i] = signal[(size_t)i];

EegPreprocessPipeline::FsHint hint;
hint.seq = pkt.seq;
hint.ts_ms = pkt.tsMs_ms; // 你需要保证这里是 ms（qint64）

// 单通道务必使用“单通道安全配置”（本 MD 上面 Options 默认值已按单通道关闭 ICA/坏道/插值/重参考）
auto res = m_pp.processEpoch(epoch, hint);

// res.data_uv[0] 就是处理后的单通道（仅时序滤波链）
```

#### 单通道“去伪影”的推荐接法（替代 ICA）

上面 `processEpoch` 只负责**滤波与降采样**。对于单通道眨眼/体动伪影，建议你在 pipeline 外再加一个“段落剔除/标记”步骤（最稳、最常用）：

- **阈值剔除**：若窗口内峰峰值 `ptp > 阈值` 或 rail 比例 `> 阈值`，则把这个窗口标记为坏段（不要用于后续特征/分类），但仍可用于 UI 绘图对比。
- **小波阈值去噪（可选）**：如果你确认伪影是短促尖峰且不关心高频脑电，可用 `DataFilter::perform_wavelet_denoising` 做去噪；否则建议先不要上小波，避免过度滤波。

> 注意：如果你现在 `RawPacket.tsMs` 是 `QString`，建议在采集线程直接保留 `qint64` 毫秒时间戳，
> 或者在这里把 `QString` 解析成 `QDateTime` 再取 `toMSecsSinceEpoch()`。

### 方式 B：未来多通道时，直接传 `channels x samples`
当你将来接入多通道设备（或你自己扩展 ThinkGear 多路），只要把每个通道的波形组成 `epoch_uv[ch][i]` 即可，wrapper 会自动做坏道/ICA/重参考。

---

## 六、你需要重点理解的两个“真实工程点”

### 1）ICA “去伪影”不是“做了 ICA 就结束”
正确流程一般是：
- ICA 分解得到独立成分 \(S\)
- **识别伪影成分**（眼电/肌电/工频/心电等），方法可能是：
  - 频带能量、峰峰值、峭度、与 EOG 通道相关性等
- 将伪影成分置零或衰减
- 用混合矩阵回投到传感器空间得到清理后的 EEG

本示例只提供“能跑通 + 可扩展骨架”。如果你后续要“真正的 ICA 伪影剔除”，我建议你先把 **EOG 通道** 或至少多通道接入，再做成分识别策略。

### 2）坏道插值需要电极空间信息才更合理
没有电极坐标时，只能做简化（均值替换/相邻替换）。要更科学，你需要：
- 头皮电极位置（10-20 system）或自定义坐标
- 用球面样条插值（spherical spline）或基于邻接图插值

---

## 七、下一步你要我继续做什么（建议）

如果你确认要把这套 wrapper 真正编译进项目，我可以继续帮你做两件事：
- **把 `eeg_preprocess_pipeline.h/.cpp` 真正加入 `CMakeLists.txt` 并修到能编译**（可能需要你现在的 `RawPacket.tsMs` 类型统一成 `qint64` 毫秒）
- 在 `AlgorithmEngine` 里把现有的 `DataFilter` 直调替换成 `EegPreprocessPipeline`，并把 `res.debug` 打到 UI/TXT，方便你观察每一步效果

