#ifndef RAW_TO_UV_PROCESSOR_H
#define RAW_TO_UV_PROCESSOR_H

#include <QObject>
#include <QtGlobal>

struct UvSample
{
    qint64 timestampMs = 0;
    quint64 seq = 0;
    qint16 rawInt16 = 0;
    double rawUv = 0.0;
};

/// 自研：RAW 整型到微伏换算（原 RawtOutUvProcessor）。
class RawToUvProcessor : public QObject
{
    Q_OBJECT
public:
    explicit RawToUvProcessor(QObject *parent = nullptr);
    void setUvPerCount(double v) { m_uvPerCount = v; }
    double uvPerCount() const { return m_uvPerCount; }

    void setGainAdjust(double gain) { m_gainAdjust = gain; }
    double gainAdjust() const { return m_gainAdjust; }

    quint64 sampleCount() const { return m_seq; }

    void resetSequence(qint64 start = 0) { m_seq = start; }
public slots:
    void onRaw(qint16 rawValue);
    void onRawWithMeta(qint16 rawValue);

signals:
    void uvSampleReady(const UvSample &sample);
    void uvValueReady(double uv);

private:
    UvSample buildSample(qint16 rawValue);

    quint64 m_seq = 0;
    double m_uvPerCount = (1.8 * 1000000.0) / (4096.0 * 2000.0);
    double m_gainAdjust = 1.0;
};

#endif // RAW_TO_UV_PROCESSOR_H
