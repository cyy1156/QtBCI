#ifndef CSVLOGWORKER_H
#define CSVLOGWORKER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QString>
#include <QMap>
#include <QSet>
#include <core/psdfeatureextractor.h>

class LogBuffer;
struct LogItem;
struct SpectrumResult;
struct FftResult;
class CsvLogWorker : public QObject
{
    Q_OBJECT
public:
    explicit CsvLogWorker(LogBuffer *buffer, QObject *parent = nullptr);
    void setOutputPath(const QString &csvPath) { m_csvPath = csvPath; }
    void setFlushIntervalMs(int ms) { m_flushIntervalMs = ms; }
    void setBatchSize(int n) { m_batchSize = n; }
    void setSpectrumOutputPath(const QString &csvpath) { m_spectrumCsvPath = csvpath; }
    void setFftOutputPath(const QString &csvPath) { m_fftCsvPath = csvPath; }
public slots:
    void start();
    void stop();
    void onSpectrumResult(const SpectrumResult &sp);
    void onFftResult(const FftResult &fr);
    /** 将 wallMs 所在 UTC 秒标记为「该秒曾出现丢包/链路异常」（供 PSD/FFT 的 secLoss 列） */
    void noteLossEventWallMs(qint64 wallMs);
    /** 由 MainWindow 将采集线程的 streamGap 接到写盘线程（须为 public 槽以便 connect 取址） */
    void onAcquisitionStreamGap(quint64 missed, quint64 prevSeq, quint64 curSeq, qint64 eventWallMs);
    void onAcquisitionLinkDiag(const QString &category, const QString &message, qint64 eventWallMs);
signals:
    void workerError(const QString &msg);
    void workerInfo(const QString &msg);
    void drained(int items, int queueSizeAfter, int droppedCount);

private slots:
    void onFlushTick();

private:
    struct MergedRow
    {
        QString tsMs;
        qint64 wallMs = -1;
        quint64 seq = 0;
        qint16 rawInt16 = 0;
        quint8 signalQuality = 255;
        double rawUv = 0.0;
        double preprocUv = 0.0;
        bool hasRaw = false;
        bool hasPreproc = false;
        quint64 gapSincePrev = 0;
    };

    void writeHeaderIfNeeded();
    void mergeItem(const LogItem &it);
    void flushMergedRows(bool forceAll);
    void writeMergedRow(const QString &tsMs, quint64 seq, qint16 rawInt16, quint8 signalQuality,
                        double rawUv, double preprocUv, quint64 gapSincePrev);

    /** 本行 ts/wall 所在 UTC 秒是否在 m_lossUtcSeconds 中：1 是，0 否 */
    int secLossMarkForRow(qint64 wallMs, const QString &tsMs) const;

    void writeSpectrumHeaderIfNeeded();
    void writeSpectrumRow(const SpectrumResult &sp);

    void writeFftHeaderIfNeeded();
    void writeFftRow(const FftResult &fr);

    LogBuffer *m_buf = nullptr;

    QString m_csvPath;
    QFile m_file;
    QTextStream m_out;
    QTimer m_timer;
    int m_flushIntervalMs = 100;
    int m_batchSize = 512;
    bool m_headerWritten = false;
    QMap<quint64, MergedRow> m_mergedRows;
    quint64 m_maxSeenSeq = 0;
    int m_mergeLag = 256;
    QString m_spectrumCsvPath;
    QFile m_spectrumFile;
    QTextStream m_spectrumOut;
    bool m_spectrumHeaderWritten = false;

    QString m_fftCsvPath;
    QFile m_fftFile;
    QTextStream m_fftOut;
    bool m_fftHeaderWritten = false;

    /** 曾发生 streamGap / linkDiag / 等记录事件的 UTC 秒键（epoch_ms/1000） */
    QSet<qint64> m_lossUtcSeconds;
    /** 上次 flush 时见过的 LogBuffer droppedCount，用于检测新增丢弃 */
    int m_dropMarkWatermark = 0;
};

#endif // CSVLOGWORKER_H
