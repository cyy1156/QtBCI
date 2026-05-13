/********************************************************************************
** Form generated from reading UI file 'serialconfigdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SERIALCONFIGDIALOG_H
#define UI_SERIALCONFIGDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_SerialConfigDialog
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *labelPort;
    QComboBox *comboPort;
    QLabel *labelBaud;
    QComboBox *comboBaud;
    QLabel *labelDataBits;
    QComboBox *comboDataBits;
    QLabel *labelParity;
    QComboBox *comboParity;
    QLabel *labelStopBits;
    QComboBox *comboStopBits;
    QLabel *labelFlow;
    QComboBox *comboFlow;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *SerialConfigDialog)
    {
        if (SerialConfigDialog->objectName().isEmpty())
            SerialConfigDialog->setObjectName("SerialConfigDialog");
        SerialConfigDialog->resize(440, 320);
        SerialConfigDialog->setMinimumSize(QSize(420, 0));
        verticalLayout = new QVBoxLayout(SerialConfigDialog);
        verticalLayout->setObjectName("verticalLayout");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        labelPort = new QLabel(SerialConfigDialog);
        labelPort->setObjectName("labelPort");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, labelPort);

        comboPort = new QComboBox(SerialConfigDialog);
        comboPort->setObjectName("comboPort");
        comboPort->setEditable(true);

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, comboPort);

        labelBaud = new QLabel(SerialConfigDialog);
        labelBaud->setObjectName("labelBaud");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, labelBaud);

        comboBaud = new QComboBox(SerialConfigDialog);
        comboBaud->setObjectName("comboBaud");
        comboBaud->setEditable(true);

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, comboBaud);

        labelDataBits = new QLabel(SerialConfigDialog);
        labelDataBits->setObjectName("labelDataBits");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, labelDataBits);

        comboDataBits = new QComboBox(SerialConfigDialog);
        comboDataBits->setObjectName("comboDataBits");

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, comboDataBits);

        labelParity = new QLabel(SerialConfigDialog);
        labelParity->setObjectName("labelParity");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, labelParity);

        comboParity = new QComboBox(SerialConfigDialog);
        comboParity->setObjectName("comboParity");

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, comboParity);

        labelStopBits = new QLabel(SerialConfigDialog);
        labelStopBits->setObjectName("labelStopBits");

        formLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, labelStopBits);

        comboStopBits = new QComboBox(SerialConfigDialog);
        comboStopBits->setObjectName("comboStopBits");

        formLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, comboStopBits);

        labelFlow = new QLabel(SerialConfigDialog);
        labelFlow->setObjectName("labelFlow");

        formLayout->setWidget(5, QFormLayout::ItemRole::LabelRole, labelFlow);

        comboFlow = new QComboBox(SerialConfigDialog);
        comboFlow->setObjectName("comboFlow");

        formLayout->setWidget(5, QFormLayout::ItemRole::FieldRole, comboFlow);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(SerialConfigDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Orientation::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Cancel|QDialogButtonBox::StandardButton::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(SerialConfigDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, SerialConfigDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, SerialConfigDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(SerialConfigDialog);
    } // setupUi

    void retranslateUi(QDialog *SerialConfigDialog)
    {
        SerialConfigDialog->setWindowTitle(QCoreApplication::translate("SerialConfigDialog", "\344\270\262\345\217\243\350\256\276\347\275\256", nullptr));
        labelPort->setText(QCoreApplication::translate("SerialConfigDialog", "\347\253\257\345\217\243", nullptr));
        labelBaud->setText(QCoreApplication::translate("SerialConfigDialog", "\346\263\242\347\211\271\347\216\207", nullptr));
        labelDataBits->setText(QCoreApplication::translate("SerialConfigDialog", "\346\225\260\346\215\256\344\275\215", nullptr));
        labelParity->setText(QCoreApplication::translate("SerialConfigDialog", "\346\240\241\351\252\214\344\275\215", nullptr));
        labelStopBits->setText(QCoreApplication::translate("SerialConfigDialog", "\345\201\234\346\255\242\344\275\215", nullptr));
        labelFlow->setText(QCoreApplication::translate("SerialConfigDialog", "\346\265\201\346\216\247", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SerialConfigDialog: public Ui_SerialConfigDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SERIALCONFIGDIALOG_H
