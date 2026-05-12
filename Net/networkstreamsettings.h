#ifndef NETWORKSTREAMSETTINGS_H
#define NETWORKSTREAMSETTINGS_H

#include <QtGlobal>
#include <QString>

/** 网络外发配置（与 QSettings 键 network/* 对应）。v0.2：三流各独立 UDP 端口。 */
struct NetworkStreamSettings {
    bool enabled = true;
    QString protocol = QStringLiteral("udp");
    QString host = QStringLiteral("127.0.0.1");
    quint16 portPreproc = 50001; // PBC1
    quint16 portFft = 50002;     // PBF1
    quint16 portPsd = 50003;     // PBP1
    bool sendPreproc = true;
    bool sendFft = false;
    bool sendPsd = false;
};

namespace NetworkSettingsStore {
void load(NetworkStreamSettings *out);
void save(const NetworkStreamSettings &in);
} // namespace NetworkSettingsStore

#endif // NETWORKSTREAMSETTINGS_H
