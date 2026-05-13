#include "networkstreamsettings.h"

#include <algorithm>

#include <core/app_settings.h>

namespace {

quint16 clampPort(int v)
{
    return static_cast<quint16>(std::clamp(v, 1, 65535));
}

} // namespace

void NetworkSettingsStore::load(NetworkStreamSettings *out)
{
    if (!out)
        return;
    QSettings s = userSettings();
    out->enabled = s.value(QStringLiteral("network/enabled"), true).toBool();
    out->protocol = s.value(QStringLiteral("network/protocol"), QStringLiteral("udp")).toString();
    if (out->protocol.isEmpty())
        out->protocol = QStringLiteral("udp");
    out->host = s.value(QStringLiteral("network/host"), QStringLiteral("127.0.0.1")).toString();
    if (out->host.isEmpty())
        out->host = QStringLiteral("127.0.0.1");

    const int legacyPort = s.value(QStringLiteral("network/port"), 0).toInt();
    const bool hadLegacy = s.contains(QStringLiteral("network/port"));

    int pp = s.value(QStringLiteral("network/port_preproc"), 0).toInt();
    if (pp <= 0 && hadLegacy)
        pp = legacyPort > 0 ? legacyPort : 50001;
    if (pp <= 0)
        pp = 50001;
    out->portPreproc = clampPort(pp);

    int pf = s.value(QStringLiteral("network/port_fft"), 0).toInt();
    if (pf <= 0)
        pf = 50002;
    out->portFft = clampPort(pf);

    int ppsd = s.value(QStringLiteral("network/port_psd"), 0).toInt();
    if (ppsd <= 0)
        ppsd = 50003;
    out->portPsd = clampPort(ppsd);

    out->sendPreproc = s.value(QStringLiteral("network/send_preproc"), true).toBool();
    out->sendFft = s.value(QStringLiteral("network/send_fft"), false).toBool();
    out->sendPsd = s.value(QStringLiteral("network/send_psd"), false).toBool();
}

void NetworkSettingsStore::save(const NetworkStreamSettings &in)
{
    QSettings s = userSettings();
    s.setValue(QStringLiteral("network/enabled"), in.enabled);
    s.setValue(QStringLiteral("network/protocol"), in.protocol);
    s.setValue(QStringLiteral("network/host"), in.host);
    s.setValue(QStringLiteral("network/port_preproc"), static_cast<int>(in.portPreproc));
    s.setValue(QStringLiteral("network/port_fft"), static_cast<int>(in.portFft));
    s.setValue(QStringLiteral("network/port_psd"), static_cast<int>(in.portPsd));
    s.remove(QStringLiteral("network/port"));
    s.setValue(QStringLiteral("network/send_preproc"), in.sendPreproc);
    s.setValue(QStringLiteral("network/send_fft"), in.sendFft);
    s.setValue(QStringLiteral("network/send_psd"), in.sendPsd);
}
