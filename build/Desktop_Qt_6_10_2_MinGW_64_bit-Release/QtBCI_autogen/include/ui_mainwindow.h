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
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
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
    QWidget *widget;
    QFrame *frame;
    QWidget *widget_2;
    QFrame *frame_2;
    QVBoxLayout *verticalLayout_3;
    QLabel *label;
    QSpacerItem *verticalSpacer_5;
    QPushButton *pushButton_start;
    QSpacerItem *verticalSpacer_4;
    QPushButton *pushButton_stop;
    QSpacerItem *verticalSpacer_3;
    QPushButton *pushButton_clear;
    QSpacerItem *verticalSpacer_2;
    QPushButton *pushButton_save;
    QSpacerItem *verticalSpacer;
    QWidget *widget_3;
    QWidget *widget1;
    QVBoxLayout *verticalLayout;
    QWidget *widget_4;
    QVBoxLayout *verticalLayout_2;
    QFrame *frame_3;
    QHBoxLayout *horizontalLayout_3;
    QListWidget *listWidget_picture;
    QWidget *widget_5;
    QHBoxLayout *horizontalLayout;
    QFrame *frame_4;
    QHBoxLayout *horizontalLayout_2;
    QListWidget *listWidget_text;
    QStatusBar *statusbar;
    QMenuBar *menubar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 500);
        MainWindow->setMaximumSize(QSize(800, 500));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        widget = new QWidget(centralwidget);
        widget->setObjectName("widget");
        widget->setGeometry(QRect(0, 0, 801, 471));
        frame = new QFrame(widget);
        frame->setObjectName("frame");
        frame->setGeometry(QRect(0, 0, 801, 451));
        frame->setFrameShape(QFrame::Shape::StyledPanel);
        frame->setFrameShadow(QFrame::Shadow::Raised);
        widget_2 = new QWidget(frame);
        widget_2->setObjectName("widget_2");
        widget_2->setGeometry(QRect(0, 0, 120, 451));
        frame_2 = new QFrame(widget_2);
        frame_2->setObjectName("frame_2");
        frame_2->setGeometry(QRect(0, 0, 104, 451));
        frame_2->setFrameShape(QFrame::Shape::StyledPanel);
        frame_2->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayout_3 = new QVBoxLayout(frame_2);
        verticalLayout_3->setObjectName("verticalLayout_3");
        label = new QLabel(frame_2);
        label->setObjectName("label");

        verticalLayout_3->addWidget(label);

        verticalSpacer_5 = new QSpacerItem(20, 38, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_5);

        pushButton_start = new QPushButton(frame_2);
        pushButton_start->setObjectName("pushButton_start");

        verticalLayout_3->addWidget(pushButton_start);

        verticalSpacer_4 = new QSpacerItem(20, 48, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_4);

        pushButton_stop = new QPushButton(frame_2);
        pushButton_stop->setObjectName("pushButton_stop");

        verticalLayout_3->addWidget(pushButton_stop);

        verticalSpacer_3 = new QSpacerItem(20, 48, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_3);

        pushButton_clear = new QPushButton(frame_2);
        pushButton_clear->setObjectName("pushButton_clear");

        verticalLayout_3->addWidget(pushButton_clear);

        verticalSpacer_2 = new QSpacerItem(20, 38, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_2);

        pushButton_save = new QPushButton(frame_2);
        pushButton_save->setObjectName("pushButton_save");

        verticalLayout_3->addWidget(pushButton_save);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_3->addItem(verticalSpacer);

        widget_3 = new QWidget(frame);
        widget_3->setObjectName("widget_3");
        widget_3->setGeometry(QRect(120, 0, 681, 451));
        widget1 = new QWidget(widget_3);
        widget1->setObjectName("widget1");
        widget1->setGeometry(QRect(0, 0, 681, 451));
        verticalLayout = new QVBoxLayout(widget1);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        widget_4 = new QWidget(widget1);
        widget_4->setObjectName("widget_4");
        verticalLayout_2 = new QVBoxLayout(widget_4);
        verticalLayout_2->setObjectName("verticalLayout_2");
        frame_3 = new QFrame(widget_4);
        frame_3->setObjectName("frame_3");
        frame_3->setFrameShape(QFrame::Shape::StyledPanel);
        frame_3->setFrameShadow(QFrame::Shadow::Raised);
        horizontalLayout_3 = new QHBoxLayout(frame_3);
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        listWidget_picture = new QListWidget(frame_3);
        listWidget_picture->setObjectName("listWidget_picture");

        horizontalLayout_3->addWidget(listWidget_picture);


        verticalLayout_2->addWidget(frame_3);


        verticalLayout->addWidget(widget_4);

        widget_5 = new QWidget(widget1);
        widget_5->setObjectName("widget_5");
        horizontalLayout = new QHBoxLayout(widget_5);
        horizontalLayout->setObjectName("horizontalLayout");
        frame_4 = new QFrame(widget_5);
        frame_4->setObjectName("frame_4");
        frame_4->setFrameShape(QFrame::Shape::StyledPanel);
        frame_4->setFrameShadow(QFrame::Shadow::Raised);
        horizontalLayout_2 = new QHBoxLayout(frame_4);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        listWidget_text = new QListWidget(frame_4);
        listWidget_text->setObjectName("listWidget_text");

        horizontalLayout_2->addWidget(listWidget_text);


        horizontalLayout->addWidget(frame_4);


        verticalLayout->addWidget(widget_5);

        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 24));
        MainWindow->setMenuBar(menubar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "<html><head/><body><p align=\"center\"><span style=\" font-size:12pt; font-weight:700;\">\346\216\247\345\210\266\351\235\242\346\235\277</span></p></body></html>", nullptr));
        pushButton_start->setText(QCoreApplication::translate("MainWindow", "\345\274\200\345\247\213", nullptr));
        pushButton_stop->setText(QCoreApplication::translate("MainWindow", "\345\201\234\346\255\242", nullptr));
        pushButton_clear->setText(QCoreApplication::translate("MainWindow", "\346\270\205\351\231\244", nullptr));
        pushButton_save->setText(QCoreApplication::translate("MainWindow", "\344\277\235\345\255\230", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
