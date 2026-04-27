#include "uibridge.h"

#include "mainwindow.h"

UiBridge::UiBridge(MainWindow *backend, QObject *parent)
    : QObject(parent), m_backend(backend)
{
    if (!m_backend)
        return;

    connect(m_backend, &MainWindow::logLineReady, this, [this](const QString &line) {
        onLogLineReady(line);
    });
    connect(m_backend, &MainWindow::plotDataReady, this, [this](const QVector<double> &values) {
        onPlotDataReady(values);
    });
    connect(m_backend, &MainWindow::runningChanged, this, [this](bool running) {
        if (m_running == running)
            return;
        m_running = running;
        emit runningChanged();
    });
}

QString UiBridge::portName() const
{
    return m_portName;
}

void UiBridge::setPortName(const QString &value)
{
    const QString trimmed = value.trimmed();
    if (trimmed.isEmpty() || trimmed == m_portName)
        return;
    m_portName = trimmed;
    emit portNameChanged();
}

QStringList UiBridge::logs() const
{
    return m_logs;
}

QVariantList UiBridge::plotPoints() const
{
    return m_plotPoints;
}

bool UiBridge::running() const
{
    return m_running;
}

void UiBridge::start()
{
    if (!m_backend)
        return;
    m_backend->startWithPort(m_portName);
}

void UiBridge::stop()
{
    if (!m_backend)
        return;
    m_backend->stopAcquisition();
}

void UiBridge::clear()
{
    if (!m_backend)
        return;
    m_backend->clearSession();
}

void UiBridge::openSettings()
{
    if (!m_backend)
        return;
    m_backend->openSaveDialog();
}

void UiBridge::onLogLineReady(const QString &line)
{
    m_logs.push_back(line);
    constexpr int maxLines = 500;
    if (m_logs.size() > maxLines)
        m_logs.remove(0, m_logs.size() - maxLines);
    emit logsChanged();
}

void UiBridge::onPlotDataReady(const QVector<double> &values)
{
    if (values.isEmpty())
        return;

    QVariantList points;
    points.reserve(values.size());
    for (double v : values)
        points.push_back(v);
    m_plotPoints = points;
    emit plotPointsChanged();
}
