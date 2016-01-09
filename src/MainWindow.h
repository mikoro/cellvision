// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

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

	protected:

		void keyPressEvent(QKeyEvent* ke) override;
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

		QColor backgroundColor;
		QColor lineColor;
	};
}
