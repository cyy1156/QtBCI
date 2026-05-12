#ifndef NETWORKCONFIGDIALOG_H
#define NETWORKCONFIGDIALOG_H

#include <QDialog>

#include "networkstreamsettings.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class NetworkConfigDialog;
}
QT_END_NAMESPACE

class NetworkConfigDialog : public QDialog {
    Q_OBJECT
public:
    explicit NetworkConfigDialog(QWidget *parent = nullptr);
    ~NetworkConfigDialog() override;

    void setFromSettings(const NetworkStreamSettings &s);
    NetworkStreamSettings toSettings() const;

private slots:
    void refreshOkEnabled();

private:
    void updateDuplicatePortHint();

    Ui::NetworkConfigDialog *ui;
};

#endif // NETWORKCONFIGDIALOG_H
