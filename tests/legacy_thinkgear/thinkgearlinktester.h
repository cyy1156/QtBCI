#ifndef THINKGEARLINKTESTER_H
#define THINKGEARLINKTESTER_H

#include <QObject>
#include <QTimer>
#include <QtGlobal>
#include "../../core/pictureandalgbuffer.h"
#include "../../core/logwbuffer.h"
#include "QThread"
#include "../../core/csvlogworker.h"
class SerialPort;
class SerialEegFrameAssembler;
class SerialEegPayloadParser;
class RawToUvProcessor;

class ThinkGearLinkTester : public QObject
{
    Q_OBJECT
public:
    explicit ThinkGearLinkTester(QObject *parent = nullptr);
    ~ThinkGearLinkTester() override;

    bool start(const QString &portName,qint32 baudRate=57600);
    void stop();

    void setTargetRawPerSec(int v){ m_targetRawPerSec =v;}
    void setTolerance(int v){m_tolerance=v;}
    /** 是否在采集时启动 CsvLogWorker；与 MainWindow 对话框勾选一致 */
    void setCsvLoggingEnabled(bool on) { m_csvLoggingEnabled = on; }
    bool csvLoggingEnabled() const { return m_csvLoggingEnabled; }

    /** 绝对路径；仅在 m_csvLoggingEnabled==true 时有效；在 start() 创建 worker 前设置 */
    void setCsvOutputPath(const QString &absPath) { m_csvOutputPath = absPath; }
    QString csvOutputPath() const { return m_csvOutputPath; }

signals:
    void secondReport(quint64 secIndex,
                      int rawCountSec,
                      int frameCountPerSec,
                      int warningCountPerSec,
                      bool pass);
    void testMessage(const QString &msg);



private slots:
    void onRawUvReady(double uv);
    void onFrameReady(const QByteArray &frame);
    void onParseWarning(const QString &msg);
    void onPortError(const QString &msg);
    void onTick1s();
    void onRawInt16Ready(qint16 raw);

private:
    PictureAndALgBuffer m_pictureandalgBuffer{128*40};
    LogBuffer m_logBuffer{4096};
    SerialPort* m_serial=nullptr;
    SerialEegFrameAssembler *m_assembler = nullptr;
    SerialEegPayloadParser *m_parser = nullptr;
    RawToUvProcessor *m_converter = nullptr;

    QTimer m_tick;

    // 每秒计数器
    int m_rawCountSec = 0;
    int m_frameCountSec = 0;
    int m_warningCountSec = 0;
    quint64 m_secIndex = 0;

    // 阈值
    int m_targetRawPerSec = 256;
    int m_tolerance = 30 ;

    quint64 m_seq=0;
    int m_algBlocksSec =0;//计数块
    int m_plotBlocksSec=0;

    QString currTimeStr;

    QThread *m_logThread=nullptr;
    CsvLogWorker *m_logWorker=nullptr;
    bool m_csvLoggingEnabled = false;
    QString m_csvOutputPath;

    void shutdownLogWriter();   // 停定时器式 stop + quit 线程（原 stop() 里日志部分）
    void startupLogWriter();    // 按 m_csvLoggingEnabled 与 m_csvOutputPath 创建线程并 start worker

    qint16 m_lastRawInt16=0;
};

#endif // THINKGEARLINKTESTER_H
