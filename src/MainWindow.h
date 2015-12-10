#pragma once

#include <QOpenGLFunctions_3_2_Core>

#include "ui_MainWindow.h"

namespace CellVision
{
	class Log;

	class MainWindow : public QMainWindow, protected QOpenGLFunctions_3_2_Core
	{
		Q_OBJECT

	public:

		explicit MainWindow(QWidget* parent = nullptr);
		~MainWindow();

		static Log& getLog();

	private:

		Ui::MainWindowClass ui;
	};
}
