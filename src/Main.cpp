#include "stdafx.h"

#include "MainWindow.h"
#include "Log.h"
#include "Common.h"

using namespace CellVision;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	MainWindow mainWindow;
	Log& log = mainWindow.getLog();
	log.logInfo("CellVision v%s", CELLVISION_VERSION);

	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	format.setSamples(4);
	format.setSwapInterval(1);
	format.setVersion(3, 2);
	format.setProfile(QSurfaceFormat::CoreProfile);
	QSurfaceFormat::setDefaultFormat(format);

	try
	{
		mainWindow.show();
		app.exec();
	}
	catch (...)
	{
		log.logException(std::current_exception());
		return -1;
	}

	return 0;
}
