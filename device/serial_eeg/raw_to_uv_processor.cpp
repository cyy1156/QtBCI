#include "raw_to_uv_processor.h"

#include <QDateTime>

RawToUvProcessor::RawToUvProcessor(QObject *parent)
    : QObject(parent)
{
}

void RawToUvProcessor::onRaw(qint16 rawValue)
{
    const UvSample s = buildSample(rawValue);
    emit uvSampleReady(s);
    emit uvValueReady(s.rawUv);
}

void RawToUvProcessor::onRawWithMeta(qint16 rawValue)
{
    const UvSample s = buildSample(rawValue);
    emit uvSampleReady(s);
    emit uvValueReady(s.rawUv);
}

UvSample RawToUvProcessor::buildSample(qint16 rawValue)
{
    UvSample s;
    s.timestampMs = QDateTime::currentMSecsSinceEpoch();
    s.seq = ++m_seq;
    s.rawInt16 = rawValue;
    s.rawUv = static_cast<double>(rawValue) * m_uvPerCount * m_gainAdjust;
    return s;
}
