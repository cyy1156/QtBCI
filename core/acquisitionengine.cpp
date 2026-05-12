
#include "acquisitionengine.h"
#include <QDateTime>

AcquisitionEngine::AcquisitionEngine(QObject *parent) : QObject(parent)
{
    m_serial = new SerialPort(this);
    m_assembler = new ThinkGearFrameAssembler(this);
    m_parser = new ThinkGearPayloadParser(this);
    m_converter = new RawtOutUvProcessor(this);

    connect(m_serial, &SerialPort::dataReceived, m_assembler, &ThinkGearFrameAssembler::onBytes);
    connect(m_assembler, &ThinkGearFrameAssembler::frameReady, this, &AcquisitionEngine::onAssemblerFrame);
    connect(m_assembler, &ThinkGearFrameAssembler::rxBufferOverflowed, this,
            &AcquisitionEngine::onRxBufferOverflowed);
    connect(m_assembler, &ThinkGearFrameAssembler::checksumFailureOccurred, this,
            &AcquisitionEngine::onChecksumFailure);
    connect(m_assembler, &ThinkGearFrameAssembler::lengthResyncOccurred, this,
            &AcquisitionEngine::onLengthResync);
    connect(m_parser, &ThinkGearPayloadParser::parseWarning, this, &AcquisitionEngine::warningMessage);
    connect(m_parser, &ThinkGearPayloadParser::rawReceived, this, &AcquisitionEngine::onRawInt16);
    connect(m_parser, &ThinkGearPayloadParser::signalQualityReceived, this, &AcquisitionEngine::onSignalQuality);
    connect(m_parser, &ThinkGearPayloadParser::rawReceived, m_converter, &RawtOutUvProcessor::onRaw);
    connect(m_converter, &RawtOutUvProcessor::uvValueReady, this, &AcquisitionEngine::onUvReady);
}

void AcquisitionEngine::onAssemblerFrame(const QByteArray &frame)
{
    m_lastFrameWallMs = QDateTime::currentMSecsSinceEpoch();
    m_parser->parseFrame(frame);
}

void AcquisitionEngine::onRxBufferOverflowed(int previousSize)
{
    const qint64 ms = QDateTime::currentMSecsSinceEpoch();
    emit linkDiagnostic(QStringLiteral("LINK"),
                        QStringLiteral("rx_buffer_overflow cleared buffer (prevSize=%1)")
                            .arg(previousSize),
                        ms);
}

void AcquisitionEngine::onChecksumFailure(quint64 totalSoFar)
{
    const qint64 ms = QDateTime::currentMSecsSinceEpoch();
    emit linkDiagnostic(QStringLiteral("LINK"),
                        QStringLiteral("checksum_fail total=%1").arg(totalSoFar), ms);
}

void AcquisitionEngine::onLengthResync(quint64 totalSoFar)
{
    const qint64 ms = QDateTime::currentMSecsSinceEpoch();
    emit linkDiagnostic(QStringLiteral("LINK"),
                        QStringLiteral("length_resync total=%1").arg(totalSoFar), ms);
}

void AcquisitionEngine::onRawInt16(qint16 raw)
{
    if (!m_running)
        return;
    m_lastRawInt16 = raw;
}

void AcquisitionEngine::onSignalQuality(quint8 v)
{
    if (!m_running)
        return;
    m_lastSignalQuality = v;
}

void AcquisitionEngine::onUvReady(double uv)
{
    if (!m_running)
        return;

    RawPacket p;
    const qint64 ms =
        (m_lastFrameWallMs >= 0) ? m_lastFrameWallMs : QDateTime::currentMSecsSinceEpoch();
    p.wallMs = ms;
    p.tsMs = QDateTime::fromMSecsSinceEpoch(ms, Qt::LocalTime)
                 .toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
    p.seq = static_cast<quint64>(m_converter->sampleCount());
    p.rawInt16 = m_lastRawInt16;
    p.signalQuality = m_lastSignalQuality;
    p.rawUv = uv;
    p.gapSincePrev = 0;

    if (!m_havePrevSeq)
    {
        emit rawPacketReady(p);
        m_prevSeq = p.seq;
        m_havePrevSeq = true;
        return;
    }

    if (p.seq <= m_prevSeq)
    {
        // 序号回退或重复：视为会话复位，不报 gap
        emit rawPacketReady(p);
        m_prevSeq = p.seq;
        return;
    }

    if (p.seq > m_prevSeq + 1)
        p.gapSincePrev = p.seq - m_prevSeq - 1;

    emit rawPacketReady(p);
    if (p.gapSincePrev > 0)
        emit streamGapDetected(p.gapSincePrev, m_prevSeq, p.seq, p.wallMs);
    m_prevSeq = p.seq;
}
void AcquisitionEngine::start(const QString &portName, qint32 baudRate)
{
    SerialPortConfig cfg;
    cfg.portName = portName;
    cfg.baudRate = baudRate;
    cfg.readBufferSize = 32 * 1024;
    startWithConfig(cfg);
}

void AcquisitionEngine::startWithConfig(const SerialPortConfig &cfg)
{
    if (m_running) {
        emit statusMessage(QStringLiteral("采集已在运行，忽略重复开始"));
        return;
    }

    m_converter->resetSequence(0);
    m_lastRawInt16=0;
    m_lastSignalQuality=255;
    m_lastFrameWallMs = -1;
    m_havePrevSeq = false;
    m_prevSeq = 0;
    if (m_assembler)
        m_assembler->clearBuffer();

    SerialPortConfig actual = cfg;
    if (actual.readBufferSize <= 0)
        actual.readBufferSize = 32 * 1024;
    m_serial->setConfig(actual);

    if (!m_serial->openReadOnly()) {
        m_running = false;
        emit runningChanged(false);
        emit warningMessage(QStringLiteral("串口打开失败: %1").arg(actual.portName));
        return;
    }
    m_running = true;
    emit runningChanged(true);
    emit statusMessage(QStringLiteral("采集已启动: %1 @ %2")
                           .arg(actual.portName)
                           .arg(actual.baudRate));
}

void AcquisitionEngine::stop()
{
    if (!m_running && !(m_serial && m_serial->isOpen()))
        return;

    m_running = false;
    emit runningChanged(false);

    if (m_serial && m_serial->isOpen())
        m_serial->close();
    if (m_assembler)
        m_assembler->clearBuffer();

    emit statusMessage(QStringLiteral("采集已停止"));
}
