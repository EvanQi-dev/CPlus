/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QPushButton *btnTeaDemo;
    QPushButton *btnStuDemo;
    QPushButton *btnStopDemo;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(633, 302);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        btnTeaDemo = new QPushButton(centralWidget);
        btnTeaDemo->setObjectName(QStringLiteral("btnTeaDemo"));
        btnTeaDemo->setGeometry(QRect(120, 110, 75, 23));
        btnStuDemo = new QPushButton(centralWidget);
        btnStuDemo->setObjectName(QStringLiteral("btnStuDemo"));
        btnStuDemo->setGeometry(QRect(260, 110, 75, 23));
        btnStopDemo = new QPushButton(centralWidget);
        btnStopDemo->setObjectName(QStringLiteral("btnStopDemo"));
        btnStopDemo->setGeometry(QRect(400, 110, 75, 23));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 633, 23));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        btnTeaDemo->setText(QApplication::translate("MainWindow", "\346\225\231\345\270\210\346\274\224\347\244\272", 0));
        btnStuDemo->setText(QApplication::translate("MainWindow", "\345\255\246\347\224\237\346\274\224\347\244\272", 0));
        btnStopDemo->setText(QApplication::translate("MainWindow", "\345\201\234\346\255\242\346\274\224\347\244\272", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
