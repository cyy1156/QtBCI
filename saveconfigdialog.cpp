#include "saveconfigdialog.h"
#include "ui_saveconfigdialog.h"

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

SaveConfigDialog::SaveConfigDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SaveConfigDialog)
{
    ui->setupUi(this);

    connect(ui->btnNewEeg, &QPushButton::clicked, this, &SaveConfigDialog::newEegClicked);
    connect(ui->btnOpenEeg, &QPushButton::clicked, this, &SaveConfigDialog::openEegClicked);
    connect(ui->btnNewTxt, &QPushButton::clicked, this, &SaveConfigDialog::newTxtClicked);
    connect(ui->btnOpenTxt, &QPushButton::clicked, this, &SaveConfigDialog::openTxtClicked);
    connect(ui->btnNewPsd, &QPushButton::clicked, this, &SaveConfigDialog::newPsdClicked);
    connect(ui->btnOpenPsd, &QPushButton::clicked, this, &SaveConfigDialog::openPsdClicked);
    connect(ui->btnNewFft, &QPushButton::clicked, this, &SaveConfigDialog::newFftClicked);
    connect(ui->btnOpenFft, &QPushButton::clicked, this, &SaveConfigDialog::openFftClicked);
    connect(ui->btnSerialCfg, &QPushButton::clicked, this, &SaveConfigDialog::serialConfigClicked);
}

SaveConfigDialog::~SaveConfigDialog()
{
    delete ui;
}

void SaveConfigDialog::loadState(bool csvEnable,
                                 bool uiTxtEnable,
                                 bool psdEnable,
                                 bool fftEnable,
                                 const QString &eegPath,
                                 const QString &txtPath,
                                 const QString &psdPath,
                                 const QString &fftPath)
{
    ui->checkEnableCsv->setChecked(csvEnable);
    ui->checkEnableUiTxt->setChecked(uiTxtEnable);
    ui->checkEnablePsd->setChecked(psdEnable);
    ui->checkEnableFft->setChecked(fftEnable);
    refreshPathLabels(eegPath, txtPath, psdPath, fftPath);
}

void SaveConfigDialog::refreshPathLabels(const QString &eegPath,
                                         const QString &txtPath,
                                         const QString &psdPath,
                                         const QString &fftPath)
{
    const auto fmt = [](const QString &title, const QString &p) {
        return QStringLiteral("%1\n%2").arg(title, p.isEmpty() ? QStringLiteral("未设置") : p);
    };
    ui->labelEegPath->setText(fmt(QStringLiteral("EEG CSV："), eegPath));
    ui->labelTxtPath->setText(fmt(QStringLiteral("操作 TXT："), txtPath));
    ui->labelPsdPath->setText(fmt(QStringLiteral("PSD CSV："), psdPath));
    ui->labelFftPath->setText(fmt(QStringLiteral("FFT CSV："), fftPath));
}

bool SaveConfigDialog::csvLoggingEnabled() const
{
    return ui->checkEnableCsv->isChecked();
}

bool SaveConfigDialog::uiTxtLoggingEnabled() const
{
    return ui->checkEnableUiTxt->isChecked();
}

bool SaveConfigDialog::psdLoggingEnabled() const
{
    return ui->checkEnablePsd->isChecked();
}

bool SaveConfigDialog::fftLoggingEnabled() const
{
    return ui->checkEnableFft->isChecked();
}
