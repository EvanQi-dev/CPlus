#include "mainwindow.h"
#include <QApplication>
#include "include\HLog.h"

int main(int argc, char *argv[])
{
	HLog::init();
	LOG_INFO("MultiVideoDemo start");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
	int r = a.exec();

	HLog::close();

	return r;
}
