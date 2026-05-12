#ifndef UDPTELEMETRYHUB_H
#define UDPTELEMETRYHUB_H

#include <QObject>
#include <QHostAddress>
#include <QUdpSocket>

#include "networkstreamsettings.h"
#include "core/algorithmengine.h"

/** 单一 QUdpSocket：PBC1 / PBF1 / PBP1 发往各自配置端口（同主机）。 */
class UdpTelemetryHub : public QObject {
    Q_OBJECT
public:
    explicit UdpTelemetryHub(QObject *parent = nullptr);

    void applySettings(const NetworkStreamSettings &s);

public slots:
    void sendPreprocPbc1(const PlotChunk &chunk);
    void sendFftPbf1(const FftResult &fr);
    void sendPsdPbp1(const SpectrumResult &sp);

private:
    QUdpSocket m_socket;
    QHostAddress m_addr;
    quint16 m_portPreproc = 50001;
    quint16 m_portFft = 50002;
    quint16 m_portPsd = 50003;
    bool m_masterEnabled = true;
    bool m_sendPreproc = true;
    bool m_sendFft = false;
    bool m_sendPsd = false;
};

#endif // UDPTELEMETRYHUB_H
