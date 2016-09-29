#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "multiVideo\Capture.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	connect(ui->btnTeaDemo, SIGNAL(clicked()), this, SLOT(teacherStartDemo()));
	connect(ui->btnStopDemo, SIGNAL(clicked()), this, SLOT(stopDemo()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::teacherStartDemo()
{
	if (capture == nullptr) {
		capture = new Capture();
		capture->setScrCaptureRect(0, 0, 1920, 1080);
		//capture->setOutSize(1152, 648);
		capture->setOutSize(320, 240);
	}

	capture->startRecord();
}

void MainWindow::stopDemo()
{
	if (capture != nullptr)
		capture->stopRecord();
}

void MainWindow::studentStartDemo()
{

}
