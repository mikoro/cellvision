#include "stdafx.h"

#include "MainWindow.h"
#include "Log.h"

using namespace CellVision;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
	ui.setupUi(this);

	resize(1280, 800);
	ui.splitterMain->setSizes({ 500, 10 });
}

Log& MainWindow::getLog()
{
	static Log log("cellvision.log");
	return log;
}
