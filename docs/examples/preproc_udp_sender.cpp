/**
 * 与 preproc_udp_sender.h 配套；报文格式须与 udp_preproc_receiver.py 一致。
 */
#include "preproc_udp_sender.h"

#include <QDataStream>

namespace {
constexpr char kMagic[4] = {'Q', 'B', 'P', 'C'}; // QtBCI Preproc Chunk
constexpr quint8 kVersion = 1;
}

PreprocUdpSender::PreprocUdpSender(QObject *parent) : QObject(parent)
{
    m_addr = QHostAddress(QHostAddress::LocalHost);
}

void PreprocUdpSender::setTarget(const QHostAddress &addr, quint16 port)
{
    m_addr = addr;
    m_port = port;
}

void PreprocUdpSender::sendPlotChunk(const PlotChunk &chunk)
{
    if (!m_enabled || m_port == 0)
        return;

    const int n = chunk.y.size();
    if (n <= 0)
        return;

    QByteArray dg;
    dg.reserve(48 + n * int(sizeof(double)));
    QDataStream ds(&dg, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds.setFloatingPointPrecision(QDataStream::DoublePrecision);

    ds.writeRawData(kMagic, 4);
    ds << quint8(kVersion);
    ds.writeRawData("\0\0\0", 3);

    ds << quint64(chunk.seqStart);
    ds << quint64(chunk.seqEnd);
    ds << qint64(chunk.anchorWallMs);
    ds << quint64(chunk.anchorSeq);
    ds << quint32(n);
    ds << quint32(0); // reserved / alignment pad

    for (int i = 0; i < n; ++i)
        ds << double(chunk.y[i]);

    m_socket.writeDatagram(dg, m_addr, m_port);
}
