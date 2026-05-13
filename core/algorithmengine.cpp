#include "algorithmengine.h"
#include <QDateTime>
#include <QTimeZone>
#include "acquisitionengine.h"  // 新增：提供 RawPacket 完整定义
#include "core/psdfeatureextractor.h"
#include "core/fftfeatureextractor.h"
AlgorithmEngine::AlgorithmEngine(QObject *parent) : QObject(parent) {}

void AlgorithmEngine::onRawPacket(const RawPacket &pkt)
{
    if (!m_running)
        return;

    const qint64 msWall = QDateTime::currentMSecsSinceEpoch();
    const QString tsMsFmt = QDateTime::fromMSecsSinceEpoch(msWall, QTimeZone::utc())
                                .toTimeZone(QTimeZone::systemTimeZone())
                                .toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));

    RawSample s;
    s.raw = pkt.rawUv;
    s.raw_count = pkt.seq;
    s.time = tsMsFmt;
    m_buf.appendRawvalue(s);

    QVector<RawSample> chunk;
    if (!m_buf.tryDequeueRawChunkForAlgo(m_windowSize, chunk))
        return;

    if (chunk.isEmpty())
        return;

    // 单通道窗口
    QVector<double> x;
    x.resize(chunk.size());
    for (int i = 0; i < chunk.size(); ++i)
        x[i] = chunk[i].raw;

    EegPreprocessPipeline::FsHint hint;
    hint.seq = (qint64)pkt.seq;
    hint.ts_ms = pkt.wallMs;
    if (hint.ts_ms <= 0 && !pkt.tsMs.isEmpty())
    {
        QDateTime pktTime = QDateTime::fromString(pkt.tsMs, QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
        if (!pktTime.isValid())
            pktTime = QDateTime::fromString(pkt.tsMs, Qt::TextDate);
        if (pktTime.isValid())
            hint.ts_ms = pktTime.toMSecsSinceEpoch();
    }

    const auto res = m_pp.process(x, hint);
    const QVector<double> &y = res.y;

    double sum2 = 0.0;
    for (double v : y)
        sum2 += v * v;
    const double rms = y.isEmpty() ? 0.0 : std::sqrt(sum2 / (double)y.size());


    // 2) 绘图块
    PlotChunk pc;
    pc.y = y;
    pc.seqStart = chunk.first().raw_count;
    pc.seqEnd = chunk.last().raw_count;
    pc.anchorWallMs = pkt.wallMs;
    pc.anchorSeq = pkt.seq;
    emit plotChunkReady(pc);

    // 3) 算法结果（示意）
    AlgoResult r;
    r.tsMs = tsMsFmt;
    r.seqEnd = pc.seqEnd;
    r.score = rms;
    r.label = res.artifact_marked ? QStringLiteral("artifact") : QStringLiteral("clean");
    emit algoResultReady(r);

    //频谱功率结果
    SpectrumResult sp;
    sp.tsMs=r.tsMs;
    sp.wallMs = msWall;
    sp.seqStart = pc.seqStart;
    sp.seqEnd = pc.seqEnd;
    sp.fsUsed = res.fs_used;
    sp.rms = rms;
    sp.label = r.label;

    const int fs=std::max(1,(int)std::lround(res.fs_used));
    //lroud是四舍五入·
    const auto f =m_psd.compute(y,fs);
    if(f.ok)
    {
        sp.delta = f.delta;
        sp.theta = f.theta;
        sp.alpha = f.alpha;
        sp.beta = f.beta;
        sp.gamma = f.gamma;
        sp.alphaBetaRatio = f.alphaBetaRatio;
    }
    emit spectrumReady(sp);

    FftResult fr;
    fr.tsMs = r.tsMs;
    fr.wallMs = msWall;
    fr.seqStart = pc.seqStart;
    fr.seqEnd = pc.seqEnd;
    fr.fsUsed = res.fs_used;
    fr.rms = rms;
    fr.label = r.label;

    const int ffs = std::max(1, (int)std::lround(res.fs_used));
    const auto ff = m_fft.compute(y, ffs);
    if (ff.ok)
    {
        fr.nfft = ff.nfft;

        fr.delta = ff.delta;
        fr.theta = ff.theta;
        fr.alpha = ff.alpha;
        fr.beta  = ff.beta;
        fr.gamma = ff.gamma;
    }

    emit fftResultReady(fr);


}
void AlgorithmEngine::resetState()
{
    m_buf.clear(); // 清环形缓冲与游标
    m_pp.reset();
}
