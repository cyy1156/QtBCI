#ifndef CHARTMODEDIALOG_H
#define CHARTMODEDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class ChartModeDialog;
}
QT_END_NAMESPACE

/** 与 MainWindow::ChartMode 数值一致：0=原始时域，1=FFT，2=功率频段 */
class ChartModeDialog : public QDialog {
    Q_OBJECT
public:
    explicit ChartModeDialog(QWidget *parent = nullptr);
    ~ChartModeDialog() override;

    void setChartMode(int mode);
    /** @return 0 RawTime, 1 FftSpectrum, 2 BandPower */
    int selectedChartMode() const;

private:
    Ui::ChartModeDialog *ui;
};

#endif // CHARTMODEDIALOG_H
