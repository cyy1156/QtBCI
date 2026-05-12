#ifndef SAVECONFIGDIALOG_H
#define SAVECONFIGDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class SaveConfigDialog;
}
QT_END_NAMESPACE

/** 「保存」按钮弹出的日志/CSV 路径与开关配置（由 .ui 生成界面）。 */
class SaveConfigDialog : public QDialog {
    Q_OBJECT
public:
    explicit SaveConfigDialog(QWidget *parent = nullptr);
    ~SaveConfigDialog() override;

    void loadState(bool csvEnable,
                   bool uiTxtEnable,
                   bool psdEnable,
                   bool fftEnable,
                   const QString &eegPath,
                   const QString &txtPath,
                   const QString &psdPath,
                   const QString &fftPath);

    void refreshPathLabels(const QString &eegPath,
                           const QString &txtPath,
                           const QString &psdPath,
                           const QString &fftPath);

    bool csvLoggingEnabled() const;
    bool uiTxtLoggingEnabled() const;
    bool psdLoggingEnabled() const;
    bool fftLoggingEnabled() const;

signals:
    void newEegClicked();
    void openEegClicked();
    void newTxtClicked();
    void openTxtClicked();
    void newPsdClicked();
    void openPsdClicked();
    void newFftClicked();
    void openFftClicked();
    void serialConfigClicked();

private:
    Ui::SaveConfigDialog *ui;
};

#endif // SAVECONFIGDIALOG_H
