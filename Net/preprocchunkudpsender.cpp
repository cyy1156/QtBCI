#include "preprocchunkudpsender.h"

#include <QByteArray>
PreprocChunkUdpSender::PreprocChunkUdpSender(QObject *parent)
    :QObject(parent)
    ,m_addr(QHostAddress::LocalHost)   {}


void PreprocChunkUdpSender::setTarget(const QHostAddress & addr, quint16 port)
{
    m_addr=addr;
    m_port=port;
}
void PreprocChunkUdpSender::sendPlotChunk(const PlotChunk & chunk)
{
    const quint32 n=static_cast<quint32>(chunk.y.size());
    QByteArray buf;
    buf.resize(44+int(n)*8);
    auto *p =reinterpret_cast<unsigned char*>(buf.data());//buf.data()拿到 QByteArray 内部的原始内存地址
    p[0] = 'P';
    p[1] = 'B';
    p[2] = 'C';
    p[3] = '1';
    p[4] = 1;
    p[5] = 0;
    p[6] = 0;
    p[7] = 0;
    //按照协议对应的字节数把对应的二进制位数写入QBytArray
    auto w64 = [&](int off, quint64 v) {
        for (int i = 0; i < 8; ++i)
            p[off + i] = static_cast<unsigned char>((v >> (8 * i)) & 0xFF);
    };
    auto w32 = [&](int off, quint32 v) {
        for (int i = 0; i < 4; ++i)
            p[off + i] = static_cast<unsigned char>((v >> (8 * i)) & 0xFF);
    };
    auto w64s = [&](int off, qint64 v) { w64(off, static_cast<quint64>(v)); };
    w64(8,chunk.seqStart);
    w64(16,chunk.seqEnd);
    w64s(24,chunk.anchorWallMs);
    w64(32,chunk.anchorSeq);
    w32(40,n);

    int off=44;
    for(quint32 i=0;i<n;++i)
    {
        //把浮点数转换成8个字节对应的数
        union{
            double d;
            unsigned char b[8];
        }u{};
        u.d=chunk.y[int(i)];
        for(int k=0;k<8;k++)
        {
            p[off+k]=u.b[k];
        }
        off+=8;
    }

    m_socket.writeDatagram(buf,m_addr,m_port);
}
