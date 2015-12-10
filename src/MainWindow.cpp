#include "stdafx.h"

#include "MainWindow.h"
#include "Log.h"

using namespace CellVision;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
	ui.setupUi(this);
}

MainWindow::~MainWindow()
{

}

Log& MainWindow::getLog()
{
	static Log log("cellvision.log");
	return log;
}
