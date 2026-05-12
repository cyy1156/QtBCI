#include "serialconfigdialog.h"
#include "ui_serialconfigdialog.h"

#include <QtGlobal>
#include <QComboBox>
#include <QSerialPortInfo>

SerialConfigDialog::SerialConfigDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SerialConfigDialog)
{
    ui->setupUi(this);
    populateFixedCombos();
}

SerialConfigDialog::~SerialConfigDialog()
{
    delete ui;
}

void SerialConfigDialog::populateFixedCombos()
{
    ui->comboBaud->clear();
    const QList<int> bauds{9600, 19200, 38400, 57600, 115200, 230400, 460800};
    for (int v : bauds)
        ui->comboBaud->addItem(QString::number(v), v);

    auto *db = ui->comboDataBits;
    db->clear();
    db->addItem(QStringLiteral("5"), static_cast<int>(QSerialPort::Data5));
    db->addItem(QStringLiteral("6"), static_cast<int>(QSerialPort::Data6));
    db->addItem(QStringLiteral("7"), static_cast<int>(QSerialPort::Data7));
    db->addItem(QStringLiteral("8"), static_cast<int>(QSerialPort::Data8));

    auto *pb = ui->comboParity;
    pb->clear();
    pb->addItem(QStringLiteral("None"), static_cast<int>(QSerialPort::NoParity));
    pb->addItem(QStringLiteral("Even"), static_cast<int>(QSerialPort::EvenParity));
    pb->addItem(QStringLiteral("Odd"), static_cast<int>(QSerialPort::OddParity));
    pb->addItem(QStringLiteral("Mark"), static_cast<int>(QSerialPort::MarkParity));
    pb->addItem(QStringLiteral("Space"), static_cast<int>(QSerialPort::SpaceParity));

    auto *sb = ui->comboStopBits;
    sb->clear();
    sb->addItem(QStringLiteral("1"), static_cast<int>(QSerialPort::OneStop));
    sb->addItem(QStringLiteral("1.5"), static_cast<int>(QSerialPort::OneAndHalfStop));
    sb->addItem(QStringLiteral("2"), static_cast<int>(QSerialPort::TwoStop));

    auto *fb = ui->comboFlow;
    fb->clear();
    fb->addItem(QStringLiteral("None"), static_cast<int>(QSerialPort::NoFlowControl));
    fb->addItem(QStringLiteral("RTS/CTS"), static_cast<int>(QSerialPort::HardwareControl));
    fb->addItem(QStringLiteral("XON/XOFF"), static_cast<int>(QSerialPort::SoftwareControl));
}

void SerialConfigDialog::refreshPortList(const QString &preferPort)
{
    ui->comboPort->clear();
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
        ui->comboPort->addItem(info.portName(), info.portName());
    ui->comboPort->setCurrentText(preferPort);
}

void SerialConfigDialog::setFromConfig(const SerialPortConfig &cfg)
{
    refreshPortList(cfg.portName);
    ui->comboBaud->setCurrentText(QString::number(cfg.baudRate));

    const int di = qMax(0, ui->comboDataBits->findData(static_cast<int>(cfg.dataBits)));
    ui->comboDataBits->setCurrentIndex(di);
    const int pi = qMax(0, ui->comboParity->findData(static_cast<int>(cfg.parity)));
    ui->comboParity->setCurrentIndex(pi);
    const int si = qMax(0, ui->comboStopBits->findData(static_cast<int>(cfg.stopBits)));
    ui->comboStopBits->setCurrentIndex(si);
    const int fi = qMax(0, ui->comboFlow->findData(static_cast<int>(cfg.flowControl)));
    ui->comboFlow->setCurrentIndex(fi);
}

SerialPortConfig SerialConfigDialog::toConfig() const
{
    SerialPortConfig c;
    c.portName = ui->comboPort->currentText().trimmed();
    c.baudRate = ui->comboBaud->currentText().toInt();
    c.dataBits = static_cast<QSerialPort::DataBits>(ui->comboDataBits->currentData().toInt());
    c.parity = static_cast<QSerialPort::Parity>(ui->comboParity->currentData().toInt());
    c.stopBits = static_cast<QSerialPort::StopBits>(ui->comboStopBits->currentData().toInt());
    c.flowControl = static_cast<QSerialPort::FlowControl>(ui->comboFlow->currentData().toInt());
    return c;
}
