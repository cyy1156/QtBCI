#ifndef EEG_FRAME_ASSEMBLER_H
#define EEG_FRAME_ASSEMBLER_H

#include <QByteArray>
#include <QObject>
#include <QString>

/// 自研：0xAA 0xAA 同步 + 变长 payload + 校验的串口帧组帧（原 ThinkGear 类设备流式格式，不依赖厂商 DLL）。
class SerialEegFrameAssembler : public QObject
{
    Q_OBJECT
public:
    explicit SerialEegFrameAssembler(QObject *parent = nullptr);
    void setVerifyChecksum(bool on) { m_verifyChecksum = on; }
    bool verifyChecksum() const { return m_verifyChecksum; }

    void clearBuffer();
    quint64 framesEmitted() const { return m_framesEmitted; }
    quint64 checksumFailures() const { return m_checksumFailures; }
    quint64 lengthResyncs() const { return m_lengthResyncs; }
    quint64 bufferOverflows() const { return m_bufferOverflows; }

public slots:
    void onBytes(const QByteArray &frame);
signals:
    void frameReady(const QByteArray &frame);
    void checksumFailureOccurred(quint64 totalSoFar);
    void lengthResyncOccurred(quint64 totalSoFar);

    void rxBufferOverflowed(int previousSize);

private:
    void processBuffer();
    static bool checksumOk(const QByteArray &frame);

    QByteArray m_rxBuffer;
    bool m_verifyChecksum = true;
    static constexpr int kMaxRxBytes = 64 * 1024;

    quint64 m_framesEmitted = 0;
    quint64 m_checksumFailures = 0;
    quint64 m_lengthResyncs = 0;
    quint64 m_bufferOverflows = 0;
};

#endif // EEG_FRAME_ASSEMBLER_H
