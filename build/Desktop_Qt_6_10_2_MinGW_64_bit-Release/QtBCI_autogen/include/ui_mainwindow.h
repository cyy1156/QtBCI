/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QFrame *frameControl;
    QVBoxLayout *verticalLayoutControl;
    QLabel *label;
    QSpacerItem *verticalSpacer_5;
    QPushButton *pushButton_start;
    QSpacerItem *verticalSpacer_4;
    QPushButton *pushButton_stop;
    QSpacerItem *verticalSpacer_3;
    QPushButton *pushButton_clear;
    QSpacerItem *verticalSpacer_2;
    QPushButton *pushButton_save;
    QSpacerItem *verticalSpacer_6;
    QPushButton *pushButton_serialConfig;
    QSpacerItem *verticalSpacer;
    QPushButton *pushButton_picture;
    QVBoxLayout *verticalLayoutRight;
    QFrame *framePlot;
    QVBoxLayout *verticalLayoutPlot;
    QListWidget *listWidget_picture;
    QFrame *frameLog;
    QVBoxLayout *verticalLayoutLog;
    QListWidget *listWidget_text;
    QMenuBar *menubar;
    QMenu *menu;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1200, 780);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setSpacing(8);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(8, 8, 8, 8);
        frameControl = new QFrame(centralwidget);
        frameControl->setObjectName("frameControl");
        frameControl->setMinimumSize(QSize(180, 0));
        frameControl->setMaximumSize(QSize(220, 16777215));
        frameControl->setFrameShape(QFrame::Shape::StyledPanel);
        frameControl->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayoutControl = new QVBoxLayout(frameControl);
        verticalLayoutControl->setSpacing(10);
        verticalLayoutControl->setObjectName("verticalLayoutControl");
        label = new QLabel(frameControl);
        label->setObjectName("label");
        QFont font;
        font.setPointSize(18);
        font.setBold(true);
        label->setFont(font);
        label->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayoutControl->addWidget(label);

        verticalSpacer_5 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayoutControl->addItem(verticalSpacer_5);

        pushButton_start = new QPushButton(frameControl);
        pushButton_start->setObjectName("pushButton_start");

        verticalLayoutControl->addWidget(pushButton_start);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayoutControl->addItem(verticalSpacer_4);

        pushButton_stop = new QPushButton(frameControl);
        pushButton_stop->setObjectName("pushButton_stop");

        verticalLayoutControl->addWidget(pushButton_stop);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayoutControl->addItem(verticalSpacer_3);

        pushButton_clear = new QPushButton(frameControl);
        pushButton_clear->setObjectName("pushButton_clear");

        verticalLayoutControl->addWidget(pushButton_clear);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayoutControl->addItem(verticalSpacer_2);

        pushButton_save = new QPushButton(frameControl);
        pushButton_save->setObjectName("pushButton_save");

        verticalLayoutControl->addWidget(pushButton_save);

        verticalSpacer_6 = new QSpacerItem(20, 30, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayoutControl->addItem(verticalSpacer_6);

        pushButton_serialConfig = new QPushButton(frameControl);
        pushButton_serialConfig->setObjectName("pushButton_serialConfig");

        verticalLayoutControl->addWidget(pushButton_serialConfig);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayoutControl->addItem(verticalSpacer);

        pushButton_picture = new QPushButton(frameControl);
        pushButton_picture->setObjectName("pushButton_picture");

        verticalLayoutControl->addWidget(pushButton_picture);


        horizontalLayout->addWidget(frameControl);

        verticalLayoutRight = new QVBoxLayout();
        verticalLayoutRight->setSpacing(8);
        verticalLayoutRight->setObjectName("verticalLayoutRight");
        framePlot = new QFrame(centralwidget);
        framePlot->setObjectName("framePlot");
        framePlot->setFrameShape(QFrame::Shape::StyledPanel);
        framePlot->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayoutPlot = new QVBoxLayout(framePlot);
        verticalLayoutPlot->setObjectName("verticalLayoutPlot");
        verticalLayoutPlot->setContentsMargins(6, 6, 6, 6);
        listWidget_picture = new QListWidget(framePlot);
        listWidget_picture->setObjectName("listWidget_picture");

        verticalLayoutPlot->addWidget(listWidget_picture);


        verticalLayoutRight->addWidget(framePlot);

        frameLog = new QFrame(centralwidget);
        frameLog->setObjectName("frameLog");
        frameLog->setMaximumSize(QSize(16777215, 220));
        frameLog->setFrameShape(QFrame::Shape::StyledPanel);
        frameLog->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayoutLog = new QVBoxLayout(frameLog);
        verticalLayoutLog->setObjectName("verticalLayoutLog");
        verticalLayoutLog->setContentsMargins(6, 6, 6, 6);
        listWidget_text = new QListWidget(frameLog);
        listWidget_text->setObjectName("listWidget_text");

        verticalLayoutLog->addWidget(listWidget_text);


        verticalLayoutRight->addWidget(frameLog);


        horizontalLayout->addLayout(verticalLayoutRight);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1200, 24));
        menu = new QMenu(menubar);
        menu->setObjectName("menu");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menu->menuAction());

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "QtBCI", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "\346\216\247\345\210\266\351\235\242\346\235\277", nullptr));
        pushButton_start->setText(QCoreApplication::translate("MainWindow", "\345\274\200\345\247\213", nullptr));
        pushButton_stop->setText(QCoreApplication::translate("MainWindow", "\345\201\234\346\255\242", nullptr));
        pushButton_clear->setText(QCoreApplication::translate("MainWindow", "\346\270\205\351\231\244", nullptr));
        pushButton_save->setText(QCoreApplication::translate("MainWindow", "\344\277\235\345\255\230", nullptr));
        pushButton_serialConfig->setText(QCoreApplication::translate("MainWindow", "\344\270\262\345\217\243\350\256\276\347\275\256", nullptr));
        pushButton_picture->setText(QCoreApplication::translate("MainWindow", "\346\233\264\346\215\242\345\233\276\350\241\250", nullptr));
        menu->setTitle(QCoreApplication::translate("MainWindow", "\345\215\225\351\200\232\351\201\223\350\256\276\345\244\207\346\216\247\345\210\266\345\231\250", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
