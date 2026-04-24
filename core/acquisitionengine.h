#ifndef ACQUISITIONENGINE_H
#define ACQUISITIONENGINE_H

#include <QObject>
#include "device/serialport.h"
#include "thinkgear/thinkgearframeassembler.h"
#include "thinkgear/thinkgearpayloadparser.h"
#include "thinkgear/rawtouvprocessor.h"

struct RawPacket {
    QString tsMs;
    quint64 seq = 0;
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
    void stop();

signals:
    void rawPacketReady(const RawPacket &pkt);
    void warningMessage(const QString &msg);
    void statusMessage(const QString &msg);
    void runningChanged(bool running);

private slots:
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
};

#endif // ACQUISITIONENGINE_H
