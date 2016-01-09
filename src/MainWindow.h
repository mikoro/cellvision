// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#pragma once

#include <map>

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
		static bool keyIsDown(int key);
		static bool keyIsDownOnce(int key);

	protected:

		bool event(QEvent* event) override;
		void closeEvent(QCloseEvent* ce) override;

	private slots:

		void on_pushButtonBrowseTiffImage_clicked();
		void on_pushButtonBrowseMetadataFile_clicked();
		void on_pushButtonLoadFromMetadata_clicked();
		void on_pushButtonLoadWindowed_clicked();
		void on_pushButtonLoadFullscreen_clicked();
		void on_pushButtonPickBackgroundColor_clicked();
		void on_pushButtonPickLineColor_clicked();

		void updateChannelSelectors();
		void updateFrameColors();
		void fullscreenDialogClosed();

	private:

		Ui::MainWindowClass ui;

		QDoubleValidator pixelValueValidator;

		static std::map<int, bool> keyMap;
		static std::map<int, bool> keyMapOnce;

		QColor backgroundColor;
		QColor lineColor;
	};
}
