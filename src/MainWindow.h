#pragma once

#include "ui_MainWindow.h"

namespace CellVision
{
	class Log;

	class MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:

		explicit MainWindow(QWidget* parent = nullptr);

		static Log& getLog();

	private:

		Ui::MainWindowClass ui;
	};
}
