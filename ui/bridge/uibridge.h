#ifndef UIBRIDGE_H
#define UIBRIDGE_H

#include <QObject>
#include <QStringList>
#include <QVariantList>

class MainWindow;

class UiBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString portName READ portName WRITE setPortName NOTIFY portNameChanged)
    Q_PROPERTY(QStringList logs READ logs NOTIFY logsChanged)
    Q_PROPERTY(QVariantList plotPoints READ plotPoints NOTIFY plotPointsChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)

public:
    explicit UiBridge(MainWindow *backend, QObject *parent = nullptr);

    QString portName() const;
    void setPortName(const QString &value);
    QStringList logs() const;
    QVariantList plotPoints() const;
    bool running() const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void clear();
    Q_INVOKABLE void openSettings();

signals:
    void portNameChanged();
    void logsChanged();
    void plotPointsChanged();
    void runningChanged();

private:
    void onLogLineReady(const QString &line);
    void onPlotDataReady(const QVector<double> &values);

    MainWindow *m_backend = nullptr;
    QString m_portName = QStringLiteral("COM7");
    QStringList m_logs;
    QVariantList m_plotPoints;
    bool m_running = false;
};

#endif // UIBRIDGE_H
