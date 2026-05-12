/**
 * 示例代码：将 AlgorithmEngine::plotChunkReady（预处理后窗口 y）经 UDP 发到本机 Python。
 * 用法：复制到工程 core/（或任意模块），CMake 增加 Qt::Network，在 MainWindow 里：
 *   m_preprocUdp = new PreprocUdpSender(this);
 *   m_preprocUdp->setTarget(QHostAddress::LocalHost, 50001);
 *   connect(m_alg, &AlgorithmEngine::plotChunkReady, m_preprocUdp, &PreprocUdpSender::sendPlotChunk);
 */
#ifndef PREPROC_UDP_SENDER_EXAMPLE_H
#define PREPROC_UDP_SENDER_EXAMPLE_H

#include <QByteArray>
#include <QHostAddress>
#include <QObject>
#include <QUdpSocket>

#include "algorithmengine.h" // PlotChunk —— 若单独编译示例可前向声明并改签名

class PreprocUdpSender : public QObject
{
    Q_OBJECT
public:
    explicit PreprocUdpSender(QObject *parent = nullptr);

    void setEnabled(bool on) { m_enabled = on; }
    void setTarget(const QHostAddress &addr, quint16 port);

public slots:
    void sendPlotChunk(const PlotChunk &chunk);

private:
    QUdpSocket m_socket;
    QHostAddress m_addr;
    quint16 m_port = 50001;
    bool m_enabled = true;
};

#endif
