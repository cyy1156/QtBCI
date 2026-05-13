#include "eeg_frame_assembler.h"

SerialEegFrameAssembler::SerialEegFrameAssembler(QObject *parent)
    : QObject{parent}
{
}

void SerialEegFrameAssembler::clearBuffer()
{
    m_rxBuffer.clear();
}

void SerialEegFrameAssembler::onBytes(const QByteArray &chunk)
{
    if (chunk.isEmpty())
        return;
    m_rxBuffer.append(chunk);

    if (m_rxBuffer.size() > kMaxRxBytes) {
        ++m_bufferOverflows;
        const int prev = m_rxBuffer.size();
        m_rxBuffer.clear();
        emit rxBufferOverflowed(prev);
        return;
    }
    processBuffer();
}

bool SerialEegFrameAssembler::checksumOk(const QByteArray &frame)
{
    if (frame.size() < 4)
        return false;
    const int n = static_cast<unsigned char>(frame[2]);
    if (frame.size() != 4 + n)
        return false;
    unsigned sum = 0;
    for (int i = 0; i < n; ++i)
        sum += static_cast<unsigned char>(frame[3 + i]);
    const auto expected = static_cast<unsigned char>((~sum) & 0xFF);
    return expected == static_cast<unsigned char>(frame[3 + n]);
}

void SerialEegFrameAssembler::processBuffer()
{
    while (true) {
        const int synIndex = m_rxBuffer.indexOf(QByteArrayView("\xAA\xAA"));
        if (synIndex < 0) {
            if (m_rxBuffer.size() > 1) {
                m_rxBuffer.remove(0, m_rxBuffer.size() - 1);
                return;
            }
        }

        if (synIndex > 0)
            m_rxBuffer.remove(0, synIndex);
        if (m_rxBuffer.size() < 3)
            return;
        const auto n = static_cast<unsigned char>(m_rxBuffer[2]);
        if (n == 0 || n > 170) {
            ++m_lengthResyncs;
            emit lengthResyncOccurred(m_lengthResyncs);
            m_rxBuffer.remove(0, 1);
            continue;
        }
        const int frameLen = 4 + static_cast<int>(n);
        if (m_rxBuffer.size() < frameLen)
            return;
        QByteArray frame = m_rxBuffer.sliced(0, frameLen);
        if (m_verifyChecksum && !checksumOk(frame)) {
            ++m_checksumFailures;
            emit checksumFailureOccurred(m_checksumFailures);
            m_rxBuffer.remove(0, 1);
            continue;
        }

        m_rxBuffer.remove(0, frameLen);
        ++m_framesEmitted;
        emit frameReady(frame);
    }
}
