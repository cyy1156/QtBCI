#ifndef ACQUISITIONENGINE_H
#define ACQUISITIONENGINE_H

#include <QObject>
#include "device/serialport.h"
#include "thinkgear/thinkgearframeassembler.h"
#include "thinkgear/thinkgearpayloadparser.h"
#include "thinkgear/rawtouvprocessor.h"

struct RawPacket {
    QString tsMs;       // 毫秒精度文本，建议 yyyy-MM-dd HH:mm:ss.zzz
    qint64 wallMs = -1; // 与 tsMs 对应的本地毫秒（组帧校验通过后时刻；用于 FsHint）
    quint64 seq = 0;
    /** 相对上一 emitted 样本缺的点数（连续时为 0） */
    quint64 gapSincePrev = 0;
    qint16 rawInt16 = 0;
    quint8 signalQuality = 255;
    double rawUv = 0.0;
};
Q_DECLARE_METATYPE(RawPacket)

class AcquisitionEngine : public QObject {
    Q_OBJECT
public:
    explicit AcquisitionEngine(QObject *parent = nullptr);

public slots:
    void start(const QString &portName, qint32 baudRate);
    void startWithConfig(const SerialPortConfig &cfg);
    void stop();

signals:
    void rawPacketReady(const RawPacket &pkt);
    void warningMessage(const QString &msg);
    void statusMessage(const QString &msg);
    void runningChanged(bool running);
    /** 序号缺口：缺失样本数、上一序号、当前序号、事件时刻 wallMs（与本包 RawPacket 一致） */
    void streamGapDetected(quint64 missedSamples, quint64 previousSeq, quint64 currentSeq,
                           qint64 eventWallMs);
    /** 链路/组帧诊断；eventWallMs 为事件发生时的墙钟毫秒（用于 PSD/FFT secLoss 秒对齐） */
    void linkDiagnostic(const QString &category, const QString &message, qint64 eventWallMs);

private slots:
    void onAssemblerFrame(const QByteArray &frame);
    void onRxBufferOverflowed(int previousSize);
    void onChecksumFailure(quint64 totalSoFar);
    void onLengthResync(quint64 totalSoFar);
    void onRawInt16(qint16 raw);
    void onSignalQuality(quint8 v);
    void onUvReady(double uv);

private:
    SerialPort *m_serial = nullptr;
    ThinkGearFrameAssembler *m_assembler = nullptr;
    ThinkGearPayloadParser *m_parser = nullptr;
    RawtOutUvProcessor *m_converter = nullptr;

    qint16 m_lastRawInt16 = 0;
    quint8 m_lastSignalQuality = 255;
    bool m_running = false;
    /** 最近一帧通过组帧/校验并交付解析时的墙钟毫秒（供对应 RAW 样本时间戳） */
    qint64 m_lastFrameWallMs = -1;
    bool m_havePrevSeq = false;
    quint64 m_prevSeq = 0;
};

#endif // ACQUISITIONENGINE_H
