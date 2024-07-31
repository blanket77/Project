/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *pushButtonPuyopuyo;
    QPushButton *pushButtonTetris;
    QPushButton *pushButtonPuyopuyoTetris;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(384, 216);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        pushButtonPuyopuyo = new QPushButton(centralwidget);
        pushButtonPuyopuyo->setObjectName("pushButtonPuyopuyo");
        pushButtonPuyopuyo->setGeometry(QRect(142, 58, 100, 20));
        pushButtonTetris = new QPushButton(centralwidget);
        pushButtonTetris->setObjectName("pushButtonTetris");
        pushButtonTetris->setGeometry(QRect(142, 98, 100, 20));
        pushButtonPuyopuyoTetris = new QPushButton(centralwidget);
        pushButtonPuyopuyoTetris->setObjectName("pushButtonPuyopuyoTetris");
        pushButtonPuyopuyoTetris->setGeometry(QRect(142, 138, 100, 20));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 187, 17));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Puyopuyo, Tetris, PuyopuyoTetris", nullptr));
        pushButtonPuyopuyo->setText(QCoreApplication::translate("MainWindow", "Puyopuyo", nullptr));
        pushButtonTetris->setText(QCoreApplication::translate("MainWindow", "Tetris", nullptr));
        pushButtonPuyopuyoTetris->setText(QCoreApplication::translate("MainWindow", "PuyopuyoTetris", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
