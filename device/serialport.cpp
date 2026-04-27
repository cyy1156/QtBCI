#include "serialport.h"

#include <QDebug>

SerialPort::SerialPort(QObject *parent)
    : QObject(parent)
    , m_port(this)
{
    setupConnections();
}

SerialPort::~SerialPort()
{
    close();
}

void SerialPort::setPortName(const QString &name)
{
    m_cfg.portName = name;
    if (m_port.isOpen())
        m_port.setPortName(name);
}

void SerialPort::setBaudRate(qint32 baud)
{
    m_cfg.baudRate = baud;
    if (m_port.isOpen())
        m_port.setBaudRate(baud);
}

void SerialPort::setConfig(const SerialPortConfig &cfg)
{
    m_cfg = cfg;
    if (m_port.isOpen())
        applyConfig();
}

void SerialPort::applyConfig()
{
    m_port.setPortName(m_cfg.portName);
    m_port.setBaudRate(m_cfg.baudRate);
    m_port.setDataBits(m_cfg.dataBits);
    m_port.setParity(m_cfg.parity);
    m_port.setStopBits(m_cfg.stopBits);
    m_port.setFlowControl(m_cfg.flowControl);
    if (m_cfg.readBufferSize > 0)
        m_port.setReadBufferSize(m_cfg.readBufferSize);
}

void SerialPort::setupConnections()
{
    connect(&m_port, &QSerialPort::readyRead, this, [this] {
        const QByteArray chunk = m_port.readAll();
        if (!chunk.isEmpty())
            emit dataReceived(chunk);
    });

    connect(&m_port, &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError error) {
        if (error == QSerialPort::NoError)
            return;
        emit portError(m_port.errorString());
    });
}

bool SerialPort::openReadWrite()
{
    if (m_port.isOpen())
    {
        m_port.clear(QSerialPort::AllDirections);
        m_port.close();
    }
    applyConfig();
    const bool ok = m_port.open(QIODevice::ReadWrite);
    if (!ok)
        emit portError(m_port.errorString());
    return ok;
}

bool SerialPort::openReadOnly()
{
    if (m_port.isOpen())
    {
        m_port.clear(QSerialPort::AllDirections);
        m_port.close();
    }
    applyConfig();
    const bool ok = m_port.open(QIODevice::ReadOnly);
    if (!ok)
        emit portError(m_port.errorString());
    return ok;
}

void SerialPort::close()
{
    if (m_port.isOpen())
    {
        m_port.clear(QSerialPort::AllDirections);
        m_port.close();
    }
}

bool SerialPort::isOpen() const
{
    return m_port.isOpen();
}

qint64 SerialPort::write(const QByteArray &data)
{
    if (!m_port.isOpen())
        return -1;
    return m_port.write(data);
}

QString SerialPort::toHexSpaced(const QByteArray &data)
{
    if (data.isEmpty())
        return {};
    return QString::fromLatin1(data.toHex(' '));
}
