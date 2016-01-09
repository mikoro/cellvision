// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

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
	QCoreApplication::setOrganizationDomain("cellvision.com");

#ifdef Q_OS_WIN32
	QCoreApplication::setOrganizationName("CellVision");
	QCoreApplication::setApplicationName("CellVision");
#else
	QCoreApplication::setOrganizationName("cellvision");
	QCoreApplication::setApplicationName("cellvision");
#endif

	QSurfaceFormat format;
	format.setSwapInterval(1);
	format.setSamples(4);
	format.setVersion(3, 3);
#ifndef __APPLE__
	format.setProfile(QSurfaceFormat::CompatibilityProfile);
#else
	format.setProfile(QSurfaceFormat::CoreProfile);
#endif
	QSurfaceFormat::setDefaultFormat(format);

	QApplication app(argc, argv);
	
	QDir::setCurrent(QCoreApplication::applicationDirPath());

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
