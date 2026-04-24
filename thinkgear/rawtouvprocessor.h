#ifndef RAWTOUVPROCESSOR_H
#define RAWTOUVPROCESSOR_H

#include <QObject>
#include <QtGlobal>

struct UvSample
{
    qint64 timestampMs = 0;      // 采样时刻（毫秒）
    quint64 seq = 0;             // 单调递增样本序号
    qint16 rawInt16 = 0;         // 原始值（保留用于追溯）
    double rawUv = 0.0;          // 换算后的微伏


};

class RawtOutUvProcessor : public QObject
{
    Q_OBJECT
public:
    explicit RawtOutUvProcessor(QObject *parent = nullptr);
    // 可调参数：uV/Count 比例，默认官方值
    void setUvPerCount(double v) { m_uvPerCount = v; }
    double uvPerCount() const { return m_uvPerCount; }

    // 可选增益微调（比如标定后 ±5%）
    void setGainAdjust(double gain) { m_gainAdjust = gain; } // 1.0 表示不调整
    double gainAdjust() const { return m_gainAdjust; }

    quint64 sampleCount() const { return m_seq; }

    void resetSequence(qint64 start =0){m_seq=start;}
public slots:
    // 最简入口：只有 raw
    void onRaw(qint16 rawValue);

    // 扩展入口：携带质量字段，便于后续 CSV / Python 同步使用
    void onRawWithMeta(qint16 rawValue);

signals:
    // 输出统一结构（推荐）
    void uvSampleReady(const UvSample &sample);

    // 兼容简单消费者（只要 uV）
    void uvValueReady(double uv);

private:
    UvSample buildSample(qint16 rawValue);

    quint64 m_seq = 0;
    double m_uvPerCount = (1.8 * 1000000.0) / (4096.0 * 2000.0); // 0.2197265625
    double m_gainAdjust = 1.0; // 标定系数

};

#endif // RAWTOUVPROCESSOR_H
