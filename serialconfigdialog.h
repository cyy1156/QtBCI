#ifndef SERIALCONFIGDIALOG_H
#define SERIALCONFIGDIALOG_H

#include <QDialog>

#include "device/serialport.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class SerialConfigDialog;
}
QT_END_NAMESPACE

class SerialConfigDialog : public QDialog {
    Q_OBJECT
public:
    explicit SerialConfigDialog(QWidget *parent = nullptr);
    ~SerialConfigDialog() override;

    void setFromConfig(const SerialPortConfig &cfg);
    SerialPortConfig toConfig() const;

private:
    void populateFixedCombos();
    void refreshPortList(const QString &preferPort);

    Ui::SerialConfigDialog *ui;
};

#endif // SERIALCONFIGDIALOG_H
