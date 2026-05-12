#include "udptelemetryhub.h"

#include <QByteArray>
#include <cstring>

namespace {

void writeLeU32(unsigned char *p, int off, quint32 v)
{
    for (int i = 0; i < 4; ++i)
        p[off + i] = static_cast<unsigned char>((v >> (8 * i)) & 0xFF);
}

void writeLeU64(unsigned char *p, int off, quint64 v)
{
    for (int i = 0; i < 8; ++i)
        p[off + i] = static_cast<unsigned char>((v >> (8 * i)) & 0xFF);
}

void writeLeI64(unsigned char *p, int off, qint64 v)
{
    writeLeU64(p, off, static_cast<quint64>(v));
}

void writeLeI32(unsigned char *p, int off, qint32 v)
{
    writeLeU32(p, off, static_cast<quint32>(v));
}

void writeNativeF64(unsigned char *p, int off, double d)
{
    union {
        double v;
        unsigned char b[8];
    } u{};
    u.v = d;
    std::memcpy(p + off, u.b, 8);
}

quint8 labelByte(const QString &label)
{
    return label.contains(QStringLiteral("artifact"), Qt::CaseInsensitive) ? quint8(1) : quint8(0);
}

} // namespace

UdpTelemetryHub::UdpTelemetryHub(QObject *parent)
    : QObject(parent)
    , m_addr(QHostAddress::LocalHost)
{}

void UdpTelemetryHub::applySettings(const NetworkStreamSettings &s)
{
    m_masterEnabled = s.enabled
                      && s.protocol.compare(QStringLiteral("udp"), Qt::CaseInsensitive) == 0;
    m_sendPreproc = s.sendPreproc;
    m_sendFft = s.sendFft;
    m_sendPsd = s.sendPsd;
    m_portPreproc = s.portPreproc;
    m_portFft = s.portFft;
    m_portPsd = s.portPsd;

    QHostAddress addr(s.host);
    if (addr.isNull() || addr.protocol() != QAbstractSocket::IPv4Protocol) {
        m_addr = QHostAddress::LocalHost;
    } else {
        m_addr = addr;
    }
}

void UdpTelemetryHub::sendPreprocPbc1(const PlotChunk &chunk)
{
    if (!m_masterEnabled || !m_sendPreproc)
        return;

    const quint32 n = static_cast<quint32>(chunk.y.size());
    QByteArray buf;
    buf.resize(44 + int(n) * 8);
    auto *p = reinterpret_cast<unsigned char *>(buf.data());
    p[0] = 'P';
    p[1] = 'B';
    p[2] = 'C';
    p[3] = '1';
    p[4] = 1;
    p[5] = 0;
    p[6] = 0;
    p[7] = 0;

    auto w64 = [&](int off, quint64 v) { writeLeU64(p, off, v); };
    auto w32 = [&](int off, quint32 v) { writeLeU32(p, off, v); };
    auto w64s = [&](int off, qint64 v) { writeLeI64(p, off, v); };

    w64(8, chunk.seqStart);
    w64(16, chunk.seqEnd);
    w64s(24, chunk.anchorWallMs);
    w64(32, chunk.anchorSeq);
    w32(40, n);

    int off = 44;
    for (quint32 i = 0; i < n; ++i) {
        union {
            double d;
            unsigned char b[8];
        } u{};
        u.d = chunk.y[int(i)];
        for (int k = 0; k < 8; ++k)
            p[off + k] = u.b[k];
        off += 8;
    }

    m_socket.writeDatagram(buf, m_addr, m_portPreproc);
}

void UdpTelemetryHub::sendFftPbf1(const FftResult &fr)
{
    if (!m_masterEnabled || !m_sendFft)
        return;

    constexpr int kSize = 93;
    QByteArray buf(kSize, 0);
    auto *p = reinterpret_cast<unsigned char *>(buf.data());
    p[0] = 'P';
    p[1] = 'B';
    p[2] = 'F';
    p[3] = '1';
    p[4] = 1;
    p[5] = 0;
    p[6] = 0;
    p[7] = 0;

    writeLeU64(p, 8, fr.seqStart);
    writeLeU64(p, 16, fr.seqEnd);
    writeLeI64(p, 24, fr.wallMs);
    writeNativeF64(p, 32, fr.fsUsed);
    writeLeI32(p, 40, static_cast<qint32>(fr.nfft));

    writeNativeF64(p, 44, fr.delta);
    writeNativeF64(p, 52, fr.theta);
    writeNativeF64(p, 60, fr.alpha);
    writeNativeF64(p, 68, fr.beta);
    writeNativeF64(p, 76, fr.gamma);
    writeNativeF64(p, 84, fr.rms);
    p[92] = labelByte(fr.label);

    m_socket.writeDatagram(buf, m_addr, m_portFft);
}

void UdpTelemetryHub::sendPsdPbp1(const SpectrumResult &sp)
{
    if (!m_masterEnabled || !m_sendPsd)
        return;

    constexpr int kSize = 101;
    QByteArray buf(kSize, 0);
    auto *p = reinterpret_cast<unsigned char *>(buf.data());
    p[0] = 'P';
    p[1] = 'B';
    p[2] = 'P';
    p[3] = '1';
    p[4] = 1;
    p[5] = 0;
    p[6] = 0;
    p[7] = 0;

    writeLeU64(p, 8, sp.seqStart);
    writeLeU64(p, 16, sp.seqEnd);
    writeLeI64(p, 24, sp.wallMs);
    writeNativeF64(p, 32, sp.fsUsed);
    writeLeI32(p, 40, 0); // 预留：与 PBF1 头对齐；PSD 路径无 nfft，填 0

    writeNativeF64(p, 44, sp.delta);
    writeNativeF64(p, 52, sp.theta);
    writeNativeF64(p, 60, sp.alpha);
    writeNativeF64(p, 68, sp.beta);
    writeNativeF64(p, 76, sp.gamma);
    writeNativeF64(p, 84, sp.rms);
    writeNativeF64(p, 92, sp.alphaBetaRatio);
    p[100] = labelByte(sp.label);

    m_socket.writeDatagram(buf, m_addr, m_portPsd);
}
