#include "chartmodedialog.h"
#include "ui_chartmodedialog.h"

ChartModeDialog::ChartModeDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChartModeDialog)
{
    ui->setupUi(this);
}

ChartModeDialog::~ChartModeDialog()
{
    delete ui;
}

void ChartModeDialog::setChartMode(int mode)
{
    ui->radioRaw->setChecked(false);
    ui->radioFft->setChecked(false);
    ui->radioPsd->setChecked(false);
    switch (mode) {
    case 1:
        ui->radioFft->setChecked(true);
        break;
    case 2:
        ui->radioPsd->setChecked(true);
        break;
    case 0:
    default:
        ui->radioRaw->setChecked(true);
        break;
    }
}

int ChartModeDialog::selectedChartMode() const
{
    if (ui->radioFft->isChecked())
        return 1;
    if (ui->radioPsd->isChecked())
        return 2;
    return 0;
}
