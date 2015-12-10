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

	private slots:

		void on_pushButtonBrowseImageFilePath_clicked();
		void on_pushButtonLoad_clicked();
		void on_horizontalSliderZDepth_valueChanged();

	private:

		Ui::MainWindowClass ui;
	};
}
