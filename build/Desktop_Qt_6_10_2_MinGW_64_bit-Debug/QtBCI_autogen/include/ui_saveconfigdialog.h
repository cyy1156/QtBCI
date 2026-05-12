/********************************************************************************
** Form generated from reading UI file 'saveconfigdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SAVECONFIGDIALOG_H
#define UI_SAVECONFIGDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_SaveConfigDialog
{
public:
    QVBoxLayout *verticalLayout;
    QCheckBox *checkEnableCsv;
    QCheckBox *checkEnableUiTxt;
    QCheckBox *checkEnablePsd;
    QCheckBox *checkEnableFft;
    QLabel *labelEegPath;
    QHBoxLayout *horizontalLayoutEeg;
    QPushButton *btnNewEeg;
    QPushButton *btnOpenEeg;
    QLabel *labelTxtPath;
    QHBoxLayout *horizontalLayoutTxt;
    QPushButton *btnNewTxt;
    QPushButton *btnOpenTxt;
    QLabel *labelPsdPath;
    QHBoxLayout *horizontalLayoutPsd;
    QPushButton *btnNewPsd;
    QPushButton *btnOpenPsd;
    QLabel *labelFftPath;
    QHBoxLayout *horizontalLayoutFft;
    QPushButton *btnNewFft;
    QPushButton *btnOpenFft;
    QPushButton *btnSerialCfg;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *SaveConfigDialog)
    {
        if (SaveConfigDialog->objectName().isEmpty())
            SaveConfigDialog->setObjectName("SaveConfigDialog");
        SaveConfigDialog->resize(560, 720);
        SaveConfigDialog->setMinimumSize(QSize(520, 0));
        verticalLayout = new QVBoxLayout(SaveConfigDialog);
        verticalLayout->setObjectName("verticalLayout");
        checkEnableCsv = new QCheckBox(SaveConfigDialog);
        checkEnableCsv->setObjectName("checkEnableCsv");

        verticalLayout->addWidget(checkEnableCsv);

        checkEnableUiTxt = new QCheckBox(SaveConfigDialog);
        checkEnableUiTxt->setObjectName("checkEnableUiTxt");

        verticalLayout->addWidget(checkEnableUiTxt);

        checkEnablePsd = new QCheckBox(SaveConfigDialog);
        checkEnablePsd->setObjectName("checkEnablePsd");

        verticalLayout->addWidget(checkEnablePsd);

        checkEnableFft = new QCheckBox(SaveConfigDialog);
        checkEnableFft->setObjectName("checkEnableFft");

        verticalLayout->addWidget(checkEnableFft);

        labelEegPath = new QLabel(SaveConfigDialog);
        labelEegPath->setObjectName("labelEegPath");
        labelEegPath->setWordWrap(true);

        verticalLayout->addWidget(labelEegPath);

        horizontalLayoutEeg = new QHBoxLayout();
        horizontalLayoutEeg->setObjectName("horizontalLayoutEeg");
        btnNewEeg = new QPushButton(SaveConfigDialog);
        btnNewEeg->setObjectName("btnNewEeg");

        horizontalLayoutEeg->addWidget(btnNewEeg);

        btnOpenEeg = new QPushButton(SaveConfigDialog);
        btnOpenEeg->setObjectName("btnOpenEeg");

        horizontalLayoutEeg->addWidget(btnOpenEeg);


        verticalLayout->addLayout(horizontalLayoutEeg);

        labelTxtPath = new QLabel(SaveConfigDialog);
        labelTxtPath->setObjectName("labelTxtPath");
        labelTxtPath->setWordWrap(true);

        verticalLayout->addWidget(labelTxtPath);

        horizontalLayoutTxt = new QHBoxLayout();
        horizontalLayoutTxt->setObjectName("horizontalLayoutTxt");
        btnNewTxt = new QPushButton(SaveConfigDialog);
        btnNewTxt->setObjectName("btnNewTxt");

        horizontalLayoutTxt->addWidget(btnNewTxt);

        btnOpenTxt = new QPushButton(SaveConfigDialog);
        btnOpenTxt->setObjectName("btnOpenTxt");

        horizontalLayoutTxt->addWidget(btnOpenTxt);


        verticalLayout->addLayout(horizontalLayoutTxt);

        labelPsdPath = new QLabel(SaveConfigDialog);
        labelPsdPath->setObjectName("labelPsdPath");
        labelPsdPath->setWordWrap(true);

        verticalLayout->addWidget(labelPsdPath);

        horizontalLayoutPsd = new QHBoxLayout();
        horizontalLayoutPsd->setObjectName("horizontalLayoutPsd");
        btnNewPsd = new QPushButton(SaveConfigDialog);
        btnNewPsd->setObjectName("btnNewPsd");

        horizontalLayoutPsd->addWidget(btnNewPsd);

        btnOpenPsd = new QPushButton(SaveConfigDialog);
        btnOpenPsd->setObjectName("btnOpenPsd");

        horizontalLayoutPsd->addWidget(btnOpenPsd);


        verticalLayout->addLayout(horizontalLayoutPsd);

        labelFftPath = new QLabel(SaveConfigDialog);
        labelFftPath->setObjectName("labelFftPath");
        labelFftPath->setWordWrap(true);

        verticalLayout->addWidget(labelFftPath);

        horizontalLayoutFft = new QHBoxLayout();
        horizontalLayoutFft->setObjectName("horizontalLayoutFft");
        btnNewFft = new QPushButton(SaveConfigDialog);
        btnNewFft->setObjectName("btnNewFft");

        horizontalLayoutFft->addWidget(btnNewFft);

        btnOpenFft = new QPushButton(SaveConfigDialog);
        btnOpenFft->setObjectName("btnOpenFft");

        horizontalLayoutFft->addWidget(btnOpenFft);


        verticalLayout->addLayout(horizontalLayoutFft);

        btnSerialCfg = new QPushButton(SaveConfigDialog);
        btnSerialCfg->setObjectName("btnSerialCfg");

        verticalLayout->addWidget(btnSerialCfg);

        buttonBox = new QDialogButtonBox(SaveConfigDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Orientation::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Cancel|QDialogButtonBox::StandardButton::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(SaveConfigDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, SaveConfigDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, SaveConfigDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(SaveConfigDialog);
    } // setupUi

    void retranslateUi(QDialog *SaveConfigDialog)
    {
        SaveConfigDialog->setWindowTitle(QCoreApplication::translate("SaveConfigDialog", "\344\277\235\345\255\230\351\205\215\347\275\256", nullptr));
        checkEnableCsv->setText(QCoreApplication::translate("SaveConfigDialog", "\345\274\200\345\247\213\351\207\207\351\233\206\346\227\266\345\220\257\347\224\250 EEG CSV \344\277\235\345\255\230", nullptr));
        checkEnableUiTxt->setText(QCoreApplication::translate("SaveConfigDialog", "\345\220\257\347\224\250\346\223\215\344\275\234\346\227\245\345\277\227 TXT \344\277\235\345\255\230", nullptr));
        checkEnablePsd->setText(QCoreApplication::translate("SaveConfigDialog", "\345\274\200\345\247\213\351\207\207\351\233\206\346\227\266\345\220\257\347\224\250 \347\273\217\350\277\207Welch \351\235\236\345\217\202\346\225\260\345\212\237\347\216\207\350\260\261\344\274\260\350\256\241(PSD)\347\256\227\346\263\225\344\277\235\345\255\230", nullptr));
        checkEnableFft->setText(QCoreApplication::translate("SaveConfigDialog", "\345\274\200\345\247\213\351\207\207\351\233\206\346\227\266\345\220\257\347\224\250 FFT \351\242\221\350\260\261\357\274\210\344\272\224\351\242\221\346\256\265\345\271\205\345\200\274\357\274\211CSV \344\277\235\345\255\230", nullptr));
        labelEegPath->setText(QCoreApplication::translate("SaveConfigDialog", "EEG CSV\357\274\232", nullptr));
        btnNewEeg->setText(QCoreApplication::translate("SaveConfigDialog", "\346\226\260\345\273\272 EEG CSV", nullptr));
        btnOpenEeg->setText(QCoreApplication::translate("SaveConfigDialog", "\346\211\223\345\274\200 EEG CSV", nullptr));
        labelTxtPath->setText(QCoreApplication::translate("SaveConfigDialog", "\346\223\215\344\275\234 TXT\357\274\232", nullptr));
        btnNewTxt->setText(QCoreApplication::translate("SaveConfigDialog", "\346\226\260\345\273\272 \346\223\215\344\275\234TXT", nullptr));
        btnOpenTxt->setText(QCoreApplication::translate("SaveConfigDialog", "\346\211\223\345\274\200 \346\223\215\344\275\234TXT", nullptr));
        labelPsdPath->setText(QCoreApplication::translate("SaveConfigDialog", "PSD CSV\357\274\232", nullptr));
        btnNewPsd->setText(QCoreApplication::translate("SaveConfigDialog", "\346\226\260\345\273\272 PSD CSV", nullptr));
        btnOpenPsd->setText(QCoreApplication::translate("SaveConfigDialog", "\346\211\223\345\274\200 PSD CSV", nullptr));
        labelFftPath->setText(QCoreApplication::translate("SaveConfigDialog", "FFT CSV\357\274\232", nullptr));
        btnNewFft->setText(QCoreApplication::translate("SaveConfigDialog", "\346\226\260\345\273\272 FFT CSV", nullptr));
        btnOpenFft->setText(QCoreApplication::translate("SaveConfigDialog", "\346\211\223\345\274\200 FFT CSV", nullptr));
        btnSerialCfg->setText(QCoreApplication::translate("SaveConfigDialog", "\344\270\262\345\217\243\351\205\215\347\275\256...", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SaveConfigDialog: public Ui_SaveConfigDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SAVECONFIGDIALOG_H
