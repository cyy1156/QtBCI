#ifndef CSVLOGWORKER_H
#define CSVLOGWORKER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QString>
#include <QMap>
#include <core/psdfeatureextractor.h>

class LogBuffer;
struct LogItem;
struct SpectrumResult;
struct FftResult;
class CsvLogWorker : public QObject
{
    Q_OBJECT
public:
    explicit CsvLogWorker(LogBuffer* buffer,QObject *parent = nullptr);
    void setOutputPath(const QString &csvPath){m_csvPath=csvPath;}
    void setFlushIntervalMs(int ms) { m_flushIntervalMs = ms; }
    void setBatchSize(int n) { m_batchSize = n; }
    void setSpectrumOutputPath(const QString & csvpath)
    {m_spectrumCsvPath=csvpath;}
    void setFftOutputPath(const QString &csvPath)
    { m_fftCsvPath = csvPath; }
public slots:
    void start();   // 打开文件 + 启动定时拉取
    void stop();    // 停止定时器 + 关闭文件
    void onSpectrumResult(const SpectrumResult &sp);
    void onFftResult(const FftResult &fr);
signals:
    void workerError(const QString &msg);
    void workerInfo(const QString &msg);
    void drained(int items, int queueSizeAfter, int droppedCount);

private slots:
    void onFlushTick();

private:
    void writeHeaderIfNeeded();
    void mergeItem(const LogItem &it);
    void flushMergedRows(bool forceAll);
    void writeMergedRow(const QString &tsMs, quint64 seq, qint16 rawInt16, quint8 signalQuality,
                        double rawUv, double preprocUv);

    void writeSpectrumHeaderIfNeeded();
    void writeSpectrumRow(const SpectrumResult &sp);

    void writeFftHeaderIfNeeded();
    void writeFftRow(const FftResult &fr);

    struct MergedRow
    {
        QString tsMs;
        quint64 seq = 0;
        qint16 rawInt16 = 0;
        quint8 signalQuality = 255;
        double rawUv = 0.0;
        double preprocUv = 0.0;
        bool hasRaw = false;
        bool hasPreproc = false;
    };

private:
    LogBuffer *m_buf=nullptr;

    QString m_csvPath;//定义名字
    QFile m_file;
    QTextStream m_out;//对象按“文本流”的方式写进文件
    QTimer m_timer;
    int m_flushIntervalMs =100;//50~100ms
    int m_batchSize =512;   //每次最多写多少条
    bool m_headerWritten=false;
    QMap<quint64, MergedRow> m_mergedRows;
    quint64 m_maxSeenSeq = 0;
    int m_mergeLag = 256; // 允许 preproc 相对 raw 的延迟窗口
    QString m_spectrumCsvPath;
    QFile m_spectrumFile;
    QTextStream m_spectrumOut;
    bool m_spectrumHeaderWritten = false;

    QString m_fftCsvPath;
    QFile m_fftFile;
    QTextStream m_fftOut;
    bool m_fftHeaderWritten = false;
};

#endif // CSVLOGWORKER_H
