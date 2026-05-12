#ifndef PREPROCCHUNKUDPSENDER_H
#define PREPROCCHUNKUDPSENDER_H

#include <QObject>
#include <QObject>
#include <QByteArray>
#include <QHostAddress>
#include <QUdpSocket>
#include "core/algorithmengine.h"   // PlotChunk
class PreprocChunkUdpSender :public QObject
{
    Q_OBJECT
public:
    explicit PreprocChunkUdpSender(QObject *parent = nullptr);

    void setTarget(const QHostAddress &addr, quint16 port);

public slots:
    /** 也可去掉 slots 改为普通 public 成员函数，connect 语法不变（Qt5+） */
    void sendPlotChunk(const PlotChunk &chunk);

private:
    QUdpSocket m_socket;
    QHostAddress m_addr;
    quint16 m_port = 50001;
};

#endif // PREPROCCHUNKUDPSENDER_H
