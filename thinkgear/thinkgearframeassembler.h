#ifndef THINKGEARFRAMEASSEMBLER_H
#define THINKGEARFRAMEASSEMBLER_H

#include <QObject>
#include<QByteArray>
#include<QString>
class ThinkGearFrameAssembler : public QObject
{
    Q_OBJECT
public:
    explicit ThinkGearFrameAssembler(QObject *parent = nullptr);
    void setVerifyChecksum(bool on){m_verifyChecksum= on;}
    bool verifyChecksum() const{return m_verifyChecksum;}

    void clearBuffer();
    quint64 framesEmitted() const {return m_framesEmitted;}
    quint64 checksumFailures() const { return m_checksumFailures; }
    quint64 lengthResyncs() const { return m_lengthResyncs; }
    quint64 bufferOverflows() const { return m_bufferOverflows; }

public slots:
    void onBytes(const QByteArray &frame);
signals:
    void frameReady(const QByteArray &frame);
    /** 每发生一次校验失败时发射（参数为累计次数） */
    void checksumFailureOccurred(quint64 totalSoFar);
    /** 每因非法长度丢弃同步字节时发射（参数为累计次数） */
    void lengthResyncOccurred(quint64 totalSoFar);

    void rxBufferOverflowed(int previousSize);
private:
    void pocessBuffer();
    static bool checksumOk(const QByteArray &frame);

    QByteArray m_rxBuffer;
    bool m_verifyChecksum =true;
    static constexpr int kMaxRxBytes=64*1024;

    quint64 m_framesEmitted = 0;    // 已发送/发射的帧数统计
    quint64 m_checksumFailures = 0; // 校验失败次数统计
    quint64 m_lengthResyncs = 0;   // 长度重同步次数统计
    quint64 m_bufferOverflows = 0;  // 缓冲区溢出次数统计

};

#endif // THINKGEARFRAMEASSEMBLER_H
