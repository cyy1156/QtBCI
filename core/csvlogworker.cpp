#include "csvlogworker.h"
#include "logwbuffer.h" // 需要完整类型：LogBuffer / LogItem
#include <core/psdfeatureextractor.h>
#include <QDateTime>
#include <QStringConverter>
#include <QThread>
#include <core/algorithmengine.h>
CsvLogWorker::CsvLogWorker(LogBuffer *buffer,QObject *parent)
    : QObject{parent}
    ,m_buf(buffer)
{
    m_timer.setParent(this); // 关键：让 timer 归属于 worker 对象
    m_timer.setTimerType(Qt::CoarseTimer);
    connect(&m_timer,&QTimer::timeout,this,&CsvLogWorker::onFlushTick);

}
void CsvLogWorker::start()
{
    if (QThread::currentThread() != thread()) {
        emit workerError(QStringLiteral("CsvLogWorker::start 线程不一致"));
        return;}
    if(!m_buf)
    {
        emit workerError(QStringLiteral("CsvLogWorker: buffer is null"));
        return;
    }
    if(m_csvPath.isEmpty())
    {
        //运行时会自动生成类似这样的文件名：
        //raw_20251225_153025.csv
        m_csvPath=QString("raw_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    }
    if(m_file.isOpen())m_file.close();
    m_mergedRows.clear();
    m_maxSeenSeq = 0;
    m_lossUtcSeconds.clear();
    m_dropMarkWatermark = 0;

    m_file.setFileName(m_csvPath);
    //以「只写、追加、文本」模式尝试打开文件
    if(!m_file.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text)){
        emit workerError(QStringLiteral("CsvLogWorker:open failed:%1").arg(m_file.errorString()));
        return;
    }
    m_out.setDevice(&m_file);//设置输出流对象为m_file
    m_out.setEncoding(QStringConverter::Utf8);

    m_headerWritten=false;
    writeHeaderIfNeeded();

    m_timer.start(m_flushIntervalMs);
    emit workerInfo(QStringLiteral("CsvWorkLoger started,path=%1").arg(m_csvPath));

}
void CsvLogWorker::stop()
{
    if(m_timer.isActive())
        m_timer.stop();
     // 最后再冲一次，把残余队列尽量写完
    onFlushTick();
    flushMergedRows(true);
    if(m_file.isOpen()){
        m_out.flush();
        m_file.flush();
        m_file.close();
    }
    if (m_spectrumFile.isOpen()) {
        m_spectrumOut.flush();
        m_spectrumFile.flush();
        m_spectrumFile.close();
    }
    m_spectrumHeaderWritten = false;

    if (m_fftFile.isOpen()) {
        m_fftOut.flush();
        m_fftFile.flush();
        m_fftFile.close();
    }
    m_fftHeaderWritten = false;
    emit workerInfo(QStringLiteral("CsvLogWorker stopped"));

}

void CsvLogWorker::noteLossEventWallMs(qint64 wallMs)
{
    if (wallMs >= 0)
        m_lossUtcSeconds.insert(wallMs / 1000LL);
}

void CsvLogWorker::onAcquisitionStreamGap(quint64 missed, quint64 prevSeq, quint64 curSeq,
                                          qint64 eventWallMs)
{
    Q_UNUSED(missed);
    Q_UNUSED(prevSeq);
    Q_UNUSED(curSeq);
    noteLossEventWallMs(eventWallMs);
}

void CsvLogWorker::onAcquisitionLinkDiag(const QString &category, const QString &message,
                                           qint64 eventWallMs)
{
    Q_UNUSED(category);
    Q_UNUSED(message);
    noteLossEventWallMs(eventWallMs);
}

int CsvLogWorker::secLossMarkForRow(qint64 wallMs, const QString &tsMs) const
{
    qint64 sec = -1;
    if (wallMs >= 0)
        sec = wallMs / 1000LL;
    else if (!tsMs.isEmpty()) {
        QDateTime dt = QDateTime::fromString(tsMs, QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
        if (!dt.isValid())
            dt = QDateTime::fromString(tsMs, Qt::TextDate);
        if (dt.isValid())
            sec = dt.toMSecsSinceEpoch() / 1000LL;
    }
    if (sec < 0)
        return 0;
    return m_lossUtcSeconds.contains(sec) ? 1 : 0;
}

//写顶头
void CsvLogWorker::writeHeaderIfNeeded()
{
    if(!m_file.isOpen())
    {
        return;
    }
    if(m_headerWritten)
        return;
    if(m_file.size()==0)
    {
        m_out << "tsMs,seq,rawInt16,signalQuality,rawUv,preprocUv,gapSincePrev\n";
        m_out.flush();
    }
    m_headerWritten=true;
}
void CsvLogWorker::writeMergedRow(const QString &tsMs, quint64 seq, qint16 rawInt16, quint8 signalQuality,
                                  double rawUv, double preprocUv, quint64 gapSincePrev)
{
    const QString rawUvStr = qIsNaN(rawUv) ? QString() : QString::number(rawUv, 'f', 6);
    const QString preprocUvStr = qIsNaN(preprocUv) ? QString() : QString::number(preprocUv, 'f', 6);
    const QString gapStr = (gapSincePrev > 0) ? QString::number(gapSincePrev) : QString();

    m_out << tsMs << ','
          << seq << ','
          << rawInt16 << ','
          << static_cast<int>(signalQuality) << ','
          << rawUvStr << ','
          << preprocUvStr << ','
          << gapStr
          << '\n';
}

void CsvLogWorker::mergeItem(const LogItem &it)
{
    if (it.seq > m_maxSeenSeq)
        m_maxSeenSeq = it.seq;

    MergedRow &row = m_mergedRows[it.seq];
    row.seq = it.seq;
    if (row.tsMs.isEmpty())
        row.tsMs = it.tsMs;

    const bool looksRaw = (it.kind == QStringLiteral("raw")) || (!qIsNaN(it.rawUv));
    const bool looksPreproc = (it.kind == QStringLiteral("preproc")) || (!qIsNaN(it.preprocUv));

    if (looksRaw)
    {
        row.rawInt16 = it.rawInt16;
        row.signalQuality = it.signalQuality;
        row.rawUv = it.rawUv;
        row.hasRaw = true;
        row.gapSincePrev = it.gapSincePrev;
        // 原始样本时间戳最可信，优先覆盖
        if (!it.tsMs.isEmpty())
            row.tsMs = it.tsMs;
        if (it.wallMs >= 0)
            row.wallMs = it.wallMs;
    }
    if (looksPreproc)
    {
        row.preprocUv = it.preprocUv;
        row.hasPreproc = true;
        if (row.tsMs.isEmpty() && !it.tsMs.isEmpty())
            row.tsMs = it.tsMs;
        if (row.wallMs < 0 && it.wallMs >= 0)
            row.wallMs = it.wallMs;
    }
}

void CsvLogWorker::flushMergedRows(bool forceAll)
{
    while (!m_mergedRows.isEmpty())
    {
        auto it = m_mergedRows.begin(); // QMap 保证 seq 从小到大
        const MergedRow &row = it.value();

        bool ready = row.hasRaw && row.hasPreproc;
        if (!ready && !forceAll)
        {
            // 超过延迟窗口也先写，避免老数据永远积压
            if (m_maxSeenSeq > row.seq && (m_maxSeenSeq - row.seq) >= static_cast<quint64>(m_mergeLag))
                ready = true;
        }

        if (!ready)
            break;

        writeMergedRow(row.tsMs, row.seq, row.rawInt16, row.signalQuality, row.rawUv, row.preprocUv,
                       row.gapSincePrev);
        m_mergedRows.erase(it);
    }
}

void CsvLogWorker::onFlushTick()
{
    if (!m_buf || !m_file.isOpen())
        return;

    writeHeaderIfNeeded();

    // drainBatch：把队列里的元素“取走”（从队列移除）——写完即舍弃缓存的关键
    const QList<LogItem> batch = m_buf->drainBatch(m_batchSize);
    if (batch.isEmpty())
        return;

    for (const LogItem &it : batch)
        mergeItem(it);
    flushMergedRows(false);

    // 每批 flush 一次即可（不要每条 flush）
    m_out.flush();
    m_file.flush();

    const int dc = m_buf->droppedCount();
    if (dc > m_dropMarkWatermark) {
        noteLossEventWallMs(QDateTime::currentMSecsSinceEpoch());
        m_dropMarkWatermark = dc;
    }

    emit drained(batch.size(), m_buf->size(), dc);
}
void CsvLogWorker::writeSpectrumHeaderIfNeeded()
{
    if (!m_spectrumFile.isOpen() || m_spectrumHeaderWritten)
        return;
    if (m_spectrumFile.size() == 0)
    {
        m_spectrumOut
            << "tsMs,seqStart,seqEnd,fsUsed,rms,delta,theta,alpha,beta,gamma,alphaBetaRatio,label,secLoss\n";
        m_spectrumOut.flush();
    }
    m_spectrumHeaderWritten = true;
}

void CsvLogWorker::writeSpectrumRow(const SpectrumResult &sp)
{
    const int secLoss = secLossMarkForRow(sp.wallMs, sp.tsMs);
    m_spectrumOut
        << sp.tsMs << ','
        << sp.seqStart << ','
        << sp.seqEnd << ','
        << QString::number(sp.fsUsed, 'f', 3) << ','
        << QString::number(sp.rms, 'f', 6) << ','
        << QString::number(sp.delta, 'f', 6) << ','
        << QString::number(sp.theta, 'f', 6) << ','
        << QString::number(sp.alpha, 'f', 6) << ','
        << QString::number(sp.beta, 'f', 6) << ','
        << QString::number(sp.gamma, 'f', 6) << ','
        << QString::number(sp.alphaBetaRatio, 'f', 6) << ','
        << sp.label << ','
        << secLoss
        << '\n';
}

void CsvLogWorker::onSpectrumResult(const SpectrumResult &sp)
{
    if (QThread::currentThread() != thread())
        return;

    if (!m_spectrumFile.isOpen())
    {
        if (m_spectrumCsvPath.isEmpty())
            return;
        m_spectrumFile.setFileName(m_spectrumCsvPath);
        if (!m_spectrumFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            return;
        m_spectrumOut.setDevice(&m_spectrumFile);
        m_spectrumOut.setEncoding(QStringConverter::Utf8);
        m_spectrumHeaderWritten = false;
    }

    if (m_spectrumCsvPath.isEmpty()) {
        if (m_spectrumFile.isOpen()) {
            m_spectrumOut.flush();
            m_spectrumFile.flush();
            m_spectrumFile.close();
        }
        m_spectrumHeaderWritten = false;
        return;
    }
    writeSpectrumHeaderIfNeeded();
    writeSpectrumRow(sp);
    m_spectrumOut.flush();
    m_spectrumFile.flush();
}

void CsvLogWorker::writeFftHeaderIfNeeded()
{
    if (!m_fftFile.isOpen() || m_fftHeaderWritten)
        return;
    if (m_fftFile.size() == 0)
    {
        m_fftOut
            << "tsMs,seqStart,seqEnd,fsUsed,nfft,rms,delta,theta,alpha,beta,gamma,label,secLoss\n";
        m_fftOut.flush();
    }
    m_fftHeaderWritten = true;
}

void CsvLogWorker::writeFftRow(const FftResult &fr)
{
    const int secLoss = secLossMarkForRow(fr.wallMs, fr.tsMs);
    m_fftOut
        << fr.tsMs << ','
        << fr.seqStart << ','
        << fr.seqEnd << ','
        << QString::number(fr.fsUsed, 'f', 3) << ','
        << fr.nfft << ','
        << QString::number(fr.rms, 'f', 6) << ','
        << QString::number(fr.delta, 'f', 6) << ','
        << QString::number(fr.theta, 'f', 6) << ','
        << QString::number(fr.alpha, 'f', 6) << ','
        << QString::number(fr.beta, 'f', 6) << ','
        << QString::number(fr.gamma, 'f', 6) << ','
        << fr.label << ','
        << secLoss
        << '\n';
}

void CsvLogWorker::onFftResult(const FftResult &fr)
{
    if (QThread::currentThread() != thread())
        return;

    // 关闭开关时主动关闭
    if (m_fftCsvPath.isEmpty())
    {
        if (m_fftFile.isOpen())
        {
            m_fftOut.flush();
            m_fftFile.flush();
            m_fftFile.close();
        }
        m_fftHeaderWritten = false;
        return;
    }

    if (!m_fftFile.isOpen())
    {
        m_fftFile.setFileName(m_fftCsvPath);
        if (!m_fftFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        {
            emit workerError(QStringLiteral("FFT CSV open failed: %1").arg(m_fftFile.errorString()));
            return;
        }
        m_fftOut.setDevice(&m_fftFile);
        m_fftOut.setEncoding(QStringConverter::Utf8);
        m_fftHeaderWritten = false;
    }

    writeFftHeaderIfNeeded();
    writeFftRow(fr);
    m_fftOut.flush();
    m_fftFile.flush();
}





