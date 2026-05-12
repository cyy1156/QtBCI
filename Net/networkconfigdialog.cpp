#include "networkconfigdialog.h"
#include "ui_networkconfigdialog.h"

#include <QAbstractSocket>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QHostAddress>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

NetworkConfigDialog::NetworkConfigDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NetworkConfigDialog)
{
    ui->setupUi(this);
    resize(520, 480);

    ui->comboProtocol->clear();
    ui->comboProtocol->addItem(QStringLiteral("UDP"), QStringLiteral("udp"));

    connect(ui->lineHost, &QLineEdit::textChanged, this, &NetworkConfigDialog::refreshOkEnabled);
    connect(ui->comboProtocol, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &NetworkConfigDialog::refreshOkEnabled);
    connect(ui->checkEnableNetwork, &QCheckBox::toggled, this, &NetworkConfigDialog::refreshOkEnabled);
    connect(ui->spinPortPreproc, qOverload<int>(&QSpinBox::valueChanged), this,
            &NetworkConfigDialog::refreshOkEnabled);
    connect(ui->spinPortFft, qOverload<int>(&QSpinBox::valueChanged), this,
            &NetworkConfigDialog::refreshOkEnabled);
    connect(ui->spinPortPsd, qOverload<int>(&QSpinBox::valueChanged), this,
            &NetworkConfigDialog::refreshOkEnabled);

    refreshOkEnabled();
}

NetworkConfigDialog::~NetworkConfigDialog()
{
    delete ui;
}

void NetworkConfigDialog::setFromSettings(const NetworkStreamSettings &s)
{
    ui->comboProtocol->setCurrentIndex(0);

    ui->lineHost->setText(s.host);
    ui->spinPortPreproc->setValue(static_cast<int>(s.portPreproc));
    ui->spinPortFft->setValue(static_cast<int>(s.portFft));
    ui->spinPortPsd->setValue(static_cast<int>(s.portPsd));
    ui->checkEnableNetwork->setChecked(s.enabled);
    ui->checkSendPreproc->setChecked(s.sendPreproc);
    ui->checkSendFft->setChecked(s.sendFft);
    ui->checkSendPsd->setChecked(s.sendPsd);
    refreshOkEnabled();
}

NetworkStreamSettings NetworkConfigDialog::toSettings() const
{
    NetworkStreamSettings o;
    o.protocol = ui->comboProtocol->currentData().toString();
    if (o.protocol.isEmpty())
        o.protocol = QStringLiteral("udp");
    o.host = ui->lineHost->text().trimmed();
    o.portPreproc = static_cast<quint16>(ui->spinPortPreproc->value());
    o.portFft = static_cast<quint16>(ui->spinPortFft->value());
    o.portPsd = static_cast<quint16>(ui->spinPortPsd->value());
    o.enabled = ui->checkEnableNetwork->isChecked();
    o.sendPreproc = ui->checkSendPreproc->isChecked();
    o.sendFft = ui->checkSendFft->isChecked();
    o.sendPsd = ui->checkSendPsd->isChecked();
    return o;
}

void NetworkConfigDialog::updateDuplicatePortHint()
{
    const int a = ui->spinPortPreproc->value();
    const int b = ui->spinPortFft->value();
    const int c = ui->spinPortPsd->value();
    if (a == b && b == c) {
        ui->labelPortDuplicateHint->setText(
            QStringLiteral("当前三端口相同，接收端需在同一 socket 上按魔数区分；建议改为不同端口。"));
    } else if (a == b || a == c || b == c) {
        ui->labelPortDuplicateHint->setText(QStringLiteral("有任意两路端口相同，请确认接收端绑定无冲突。"));
    } else {
        ui->labelPortDuplicateHint->clear();
    }
}

void NetworkConfigDialog::refreshOkEnabled()
{
    const QString hostText = ui->lineHost->text().trimmed();
    QHostAddress addr(hostText);
    const bool hostOk = !hostText.isEmpty() && !addr.isNull()
                        && addr.protocol() == QAbstractSocket::IPv4Protocol
                        && !addr.isMulticast();

    QPushButton *okBtn = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (okBtn) {
        const bool needHost = ui->checkEnableNetwork->isChecked();
        okBtn->setEnabled(!needHost || hostOk);
    }
    updateDuplicatePortHint();
}
