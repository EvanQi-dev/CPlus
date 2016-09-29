#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Coding: UTF-8(BOM)
#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

#include <QMainWindow>

class Capture;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
	void teacherStartDemo();
	void studentStartDemo();
	void stopDemo();

private:
    Ui::MainWindow *ui;
	Capture *capture = nullptr;
};

#endif // MAINWINDOW_H
