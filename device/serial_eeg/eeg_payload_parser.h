#ifndef EEG_PAYLOAD_PARSER_H
#define EEG_PAYLOAD_PARSER_H

#include <QByteArray>
#include <QObject>
#include <QString>

struct SerialEegBandPower
{
    quint32 delta = 0;
    quint32 theta = 0;
    quint32 lowAlpha = 0;
    quint32 highAlpha = 0;
    quint32 lowBeta = 0;
    quint32 highBeta = 0;
    quint32 lowGamma = 0;
    quint32 midGamma = 0;
};

/// 自研：解析 0xAA 帧内 TLV payload（不依赖厂商 ThinkGear DLL）。
class SerialEegPayloadParser : public QObject
{
    Q_OBJECT
public:
    explicit SerialEegPayloadParser(QObject *parent = nullptr);
public slots:
    void parseFrame(const QByteArray &frame);

signals:
    void rawReceived(qint16 value);
    void signalQualityReceived(quint8 value);
    void attentionReceived(quint8 value);
    void meditationReceived(quint8 value);
    void blinkReceived(quint8 value);
    void eegPowerReceived(const SerialEegBandPower &power);
    void parseWarning(const QString &message);

private:
    static bool extractPayload(const QByteArray &frame, QByteArray &payloadOut, QString *errorMsg);
    void parsePayload(const QByteArray &payload);
};

#endif // EEG_PAYLOAD_PARSER_H
