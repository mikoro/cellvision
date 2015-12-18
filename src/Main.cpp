#include "stdafx.h"

#include "MainWindow.h"
#include "Log.h"
#include "Common.h"

using namespace CellVision;

namespace
{
	Log* staticLog = nullptr;

	void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
	{
		std::string messageString = message.toStdString();

		switch (type)
		{
			case QtDebugMsg:
				staticLog->logDebug(messageString);
				break;
			case QtInfoMsg:
				staticLog->logInfo(messageString);
				break;
			case QtWarningMsg:
				staticLog->logWarning(messageString);
				break;
			case QtCriticalMsg:
			case QtFatalMsg:
				staticLog->logError(messageString);
				break;
			default: break;
		}
	}
}

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	MainWindow mainWindow;
	Log& log = mainWindow.getLog();
	log.logInfo("CellVision v%s", CELLVISION_VERSION);

	staticLog = &log;
	qInstallMessageHandler(messageHandler);

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
