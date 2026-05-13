#include "eeg_payload_parser.h"

SerialEegPayloadParser::SerialEegPayloadParser(QObject *parent)
    : QObject{parent}
{
}

bool SerialEegPayloadParser::extractPayload(const QByteArray &frame, QByteArray &payloadOut,
                                            QString *errorMsg)
{
    payloadOut.clear();
    if (frame.size() < 4) {
        if (errorMsg)
            *errorMsg = QStringLiteral("frame too short");
        return false;
    }
    if (static_cast<quint8>(frame[0]) != 0xAA || static_cast<quint8>(frame[1]) != 0xAA) {
        if (errorMsg)
            *errorMsg = QStringLiteral("bad sync");
        return false;
    }
    const int n = static_cast<unsigned char>(frame[2]);
    if (n == 0 || n > 170) {
        if (errorMsg)
            *errorMsg = QStringLiteral("invalid payload length");
        return false;
    }
    if (frame.size() != 4 + n) {
        if (errorMsg)
            *errorMsg = QStringLiteral("frame size mismatch");
        return false;
    }
    payloadOut = frame.sliced(3, n);
    return true;
}

void SerialEegPayloadParser::parseFrame(const QByteArray &frame)
{
    QByteArray payload;
    QString err;
    if (!extractPayload(frame, payload, &err)) {
        emit parseWarning(err);
        return;
    }
    parsePayload(payload);
}

static quint32 readU24Be(const char *p)
{
    const auto b0 = static_cast<quint32>(static_cast<quint8>(p[0]));
    const auto b1 = static_cast<quint32>(static_cast<quint8>(p[1]));
    const auto b2 = static_cast<quint32>(static_cast<quint8>(p[2]));
    return (b0 << 16) | (b1 << 8) | b2;
}

void SerialEegPayloadParser::parsePayload(const QByteArray &payload)
{
    int i = 0;
    const int n = payload.size();
    while (i < n) {
        const auto code = static_cast<unsigned char>(payload[i]);
        switch (code) {
        case 0x02:
            if (i + 1 >= n) {
                emit parseWarning(QStringLiteral("truncated 0x02"));
                return;
            }
            emit signalQualityReceived(static_cast<quint8>(payload[i + 1]));
            i += 2;
            break;
        case 0x04:
            if (i + 1 >= n) {
                emit parseWarning(QStringLiteral("truncated 0x04"));
                return;
            }
            emit attentionReceived(static_cast<quint8>(payload[i + 1]));
            i += 2;
            break;
        case 0x05:
            if (i + 1 >= n) {
                emit parseWarning(QStringLiteral("truncated 0x05"));
                return;
            }
            emit meditationReceived(static_cast<quint8>(payload[i + 1]));
            i += 2;
            break;
        case 0x16:
            if (i + 1 >= n) {
                emit parseWarning(QStringLiteral("truncated 0x16"));
                return;
            }
            emit blinkReceived(static_cast<quint8>(payload[i + 1]));
            i += 2;
            break;
        case 0x80: {
            if (i + 3 >= n) {
                emit parseWarning(QStringLiteral("truncated 0x80"));
                return;
            }
            const quint8 vlen = static_cast<quint8>(payload[i + 1]);
            if (vlen != 0x02) {
                emit parseWarning(QStringLiteral("invalid 0x80 length"));
                return;
            }
            const quint8 hi = static_cast<quint8>(payload[i + 2]);
            const quint8 lo = static_cast<quint8>(payload[i + 3]);
            qint32 rawdata = (static_cast<qint32>(hi) << 8) | lo;
            if (rawdata > 32767)
                rawdata -= 65536;
            emit rawReceived(static_cast<qint16>(rawdata));
            i += 4;
            break;
        }
        case 0x83: {
            if (i + 25 >= n) {
                emit parseWarning(QStringLiteral("truncated 0x83"));
                return;
            }
            SerialEegBandPower p;
            const char *base = payload.constData() + i;
            p.delta = readU24Be(base + 2);
            p.theta = readU24Be(base + 5);
            p.lowAlpha = readU24Be(base + 8);
            p.highAlpha = readU24Be(base + 11);
            p.lowBeta = readU24Be(base + 14);
            p.highBeta = readU24Be(base + 17);
            p.lowGamma = readU24Be(base + 20);
            p.midGamma = readU24Be(base + 23);
            emit eegPowerReceived(p);
            i += 26;
            break;
        }
        default:
            ++i;
            break;
        }
    }
}
