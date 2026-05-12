/********************************************************************************
** Form generated from reading UI file 'networkconfigdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NETWORKCONFIGDIALOG_H
#define UI_NETWORKCONFIGDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_NetworkConfigDialog
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupProtocol;
    QFormLayout *formLayoutProtocol;
    QLabel *labelProtocol;
    QComboBox *comboProtocol;
    QGroupBox *groupTarget;
    QFormLayout *formLayoutTarget;
    QLabel *labelHost;
    QLineEdit *lineHost;
    QLabel *labelPort;
    QSpinBox *spinPort;
    QGroupBox *groupStreams;
    QVBoxLayout *verticalLayoutStreams;
    QCheckBox *checkEnableNetwork;
    QCheckBox *checkSendPreproc;
    QCheckBox *checkSendFft;
    QCheckBox *checkSendPsd;
    QLabel *labelHint;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *NetworkConfigDialog)
    {
        if (NetworkConfigDialog->objectName().isEmpty())
            NetworkConfigDialog->setObjectName("NetworkConfigDialog");
        NetworkConfigDialog->resize(420, 360);
        verticalLayout = new QVBoxLayout(NetworkConfigDialog);
        verticalLayout->setObjectName("verticalLayout");
        groupProtocol = new QGroupBox(NetworkConfigDialog);
        groupProtocol->setObjectName("groupProtocol");
        formLayoutProtocol = new QFormLayout(groupProtocol);
        formLayoutProtocol->setObjectName("formLayoutProtocol");
        labelProtocol = new QLabel(groupProtocol);
        labelProtocol->setObjectName("labelProtocol");

        formLayoutProtocol->setWidget(0, QFormLayout::ItemRole::LabelRole, labelProtocol);

        comboProtocol = new QComboBox(groupProtocol);
        comboProtocol->setObjectName("comboProtocol");

        formLayoutProtocol->setWidget(0, QFormLayout::ItemRole::FieldRole, comboProtocol);


        verticalLayout->addWidget(groupProtocol);

        groupTarget = new QGroupBox(NetworkConfigDialog);
        groupTarget->setObjectName("groupTarget");
        formLayoutTarget = new QFormLayout(groupTarget);
        formLayoutTarget->setObjectName("formLayoutTarget");
        labelHost = new QLabel(groupTarget);
        labelHost->setObjectName("labelHost");

        formLayoutTarget->setWidget(0, QFormLayout::ItemRole::LabelRole, labelHost);

        lineHost = new QLineEdit(groupTarget);
        lineHost->setObjectName("lineHost");

        formLayoutTarget->setWidget(0, QFormLayout::ItemRole::FieldRole, lineHost);

        labelPort = new QLabel(groupTarget);
        labelPort->setObjectName("labelPort");

        formLayoutTarget->setWidget(1, QFormLayout::ItemRole::LabelRole, labelPort);

        spinPort = new QSpinBox(groupTarget);
        spinPort->setObjectName("spinPort");
        spinPort->setMinimum(1);
        spinPort->setMaximum(65535);
        spinPort->setValue(50001);

        formLayoutTarget->setWidget(1, QFormLayout::ItemRole::FieldRole, spinPort);


        verticalLayout->addWidget(groupTarget);

        groupStreams = new QGroupBox(NetworkConfigDialog);
        groupStreams->setObjectName("groupStreams");
        verticalLayoutStreams = new QVBoxLayout(groupStreams);
        verticalLayoutStreams->setObjectName("verticalLayoutStreams");
        checkEnableNetwork = new QCheckBox(groupStreams);
        checkEnableNetwork->setObjectName("checkEnableNetwork");

        verticalLayoutStreams->addWidget(checkEnableNetwork);

        checkSendPreproc = new QCheckBox(groupStreams);
        checkSendPreproc->setObjectName("checkSendPreproc");

        verticalLayoutStreams->addWidget(checkSendPreproc);

        checkSendFft = new QCheckBox(groupStreams);
        checkSendFft->setObjectName("checkSendFft");

        verticalLayoutStreams->addWidget(checkSendFft);

        checkSendPsd = new QCheckBox(groupStreams);
        checkSendPsd->setObjectName("checkSendPsd");

        verticalLayoutStreams->addWidget(checkSendPsd);

        labelHint = new QLabel(groupStreams);
        labelHint->setObjectName("labelHint");
        labelHint->setWordWrap(true);

        verticalLayoutStreams->addWidget(labelHint);


        verticalLayout->addWidget(groupStreams);

        buttonBox = new QDialogButtonBox(NetworkConfigDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Orientation::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Cancel|QDialogButtonBox::StandardButton::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(NetworkConfigDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, NetworkConfigDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, NetworkConfigDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(NetworkConfigDialog);
    } // setupUi

    void retranslateUi(QDialog *NetworkConfigDialog)
    {
        NetworkConfigDialog->setWindowTitle(QCoreApplication::translate("NetworkConfigDialog", "\347\275\221\347\273\234\350\256\276\347\275\256", nullptr));
        groupProtocol->setTitle(QCoreApplication::translate("NetworkConfigDialog", "\344\274\240\350\276\223\345\215\217\350\256\256", nullptr));
        labelProtocol->setText(QCoreApplication::translate("NetworkConfigDialog", "\345\215\217\350\256\256", nullptr));
        groupTarget->setTitle(QCoreApplication::translate("NetworkConfigDialog", "\347\233\256\346\240\207\345\234\260\345\235\200\357\274\210UDP\357\274\211", nullptr));
        labelHost->setText(QCoreApplication::translate("NetworkConfigDialog", "IPv4 \344\270\273\346\234\272", nullptr));
        labelPort->setText(QCoreApplication::translate("NetworkConfigDialog", "\347\253\257\345\217\243", nullptr));
        groupStreams->setTitle(QCoreApplication::translate("NetworkConfigDialog", "\345\217\221\351\200\201\347\232\204\346\225\260\346\215\256", nullptr));
        checkEnableNetwork->setText(QCoreApplication::translate("NetworkConfigDialog", "\345\220\257\347\224\250\347\275\221\347\273\234\345\217\221\351\200\201", nullptr));
        checkSendPreproc->setText(QCoreApplication::translate("NetworkConfigDialog", "\351\242\204\345\244\204\347\220\206\346\227\266\345\237\237\347\252\227\357\274\210PBC1\357\274\214\344\270\216 Python udp_infer_live \345\205\274\345\256\271\357\274\211", nullptr));
        checkSendFft->setText(QCoreApplication::translate("NetworkConfigDialog", "\351\242\221\346\256\265\345\212\237\347\216\207\357\274\210FFT \350\267\257\345\276\204\357\274\214PBF1\357\274\211", nullptr));
        checkSendPsd->setText(QCoreApplication::translate("NetworkConfigDialog", "\351\242\221\346\256\265\345\212\237\347\216\207\357\274\210PSD \350\267\257\345\276\204\357\274\214PBP1\357\274\211", nullptr));
        labelHint->setText(QCoreApplication::translate("NetworkConfigDialog", "\346\217\220\347\244\272\357\274\232FFT/PSD \344\270\272\345\220\204\351\242\221\346\256\265\346\240\207\351\207\217\347\211\271\345\276\201\357\274\214\351\235\236\345\205\250\351\242\221\350\260\261\346\233\262\347\272\277\343\200\202", nullptr));
    } // retranslateUi

};

namespace Ui {
    class NetworkConfigDialog: public Ui_NetworkConfigDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NETWORKCONFIGDIALOG_H
