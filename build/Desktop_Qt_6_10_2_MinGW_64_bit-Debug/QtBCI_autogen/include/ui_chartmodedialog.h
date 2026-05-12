/********************************************************************************
** Form generated from reading UI file 'chartmodedialog.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHARTMODEDIALOG_H
#define UI_CHARTMODEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ChartModeDialog
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayoutGroup;
    QRadioButton *radioRaw;
    QRadioButton *radioFft;
    QRadioButton *radioPsd;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ChartModeDialog)
    {
        if (ChartModeDialog->objectName().isEmpty())
            ChartModeDialog->setObjectName("ChartModeDialog");
        ChartModeDialog->resize(400, 240);
        verticalLayout = new QVBoxLayout(ChartModeDialog);
        verticalLayout->setObjectName("verticalLayout");
        groupBox = new QGroupBox(ChartModeDialog);
        groupBox->setObjectName("groupBox");
        verticalLayoutGroup = new QVBoxLayout(groupBox);
        verticalLayoutGroup->setObjectName("verticalLayoutGroup");
        radioRaw = new QRadioButton(groupBox);
        radioRaw->setObjectName("radioRaw");
        radioRaw->setChecked(true);

        verticalLayoutGroup->addWidget(radioRaw);

        radioFft = new QRadioButton(groupBox);
        radioFft->setObjectName("radioFft");

        verticalLayoutGroup->addWidget(radioFft);

        radioPsd = new QRadioButton(groupBox);
        radioPsd->setObjectName("radioPsd");

        verticalLayoutGroup->addWidget(radioPsd);


        verticalLayout->addWidget(groupBox);

        buttonBox = new QDialogButtonBox(ChartModeDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Orientation::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Cancel|QDialogButtonBox::StandardButton::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(ChartModeDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, ChartModeDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, ChartModeDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(ChartModeDialog);
    } // setupUi

    void retranslateUi(QDialog *ChartModeDialog)
    {
        ChartModeDialog->setWindowTitle(QCoreApplication::translate("ChartModeDialog", "\346\233\264\346\215\242\345\233\276\350\241\250", nullptr));
        groupBox->setTitle(QCoreApplication::translate("ChartModeDialog", "\345\233\276\350\241\250\347\261\273\345\236\213", nullptr));
        radioRaw->setText(QCoreApplication::translate("ChartModeDialog", "\347\224\237\346\210\220\351\242\204\345\244\204\347\220\206\344\271\213\345\220\216\347\232\204\345\233\276\345\203\217", nullptr));
        radioFft->setText(QCoreApplication::translate("ChartModeDialog", "\347\224\237\346\210\220FFT\351\242\221\350\260\261\347\232\204\345\233\276\345\203\217", nullptr));
        radioPsd->setText(QCoreApplication::translate("ChartModeDialog", "\347\224\237\346\210\220\345\212\237\347\216\207\351\242\221\346\256\265\347\232\204\345\233\276\345\203\217", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ChartModeDialog: public Ui_ChartModeDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHARTMODEDIALOG_H
