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
		static QVector4D getMouseDelta();

	protected:

		bool event(QEvent* event) override;

	private slots:

		void on_pushButtonBrowseImageFilePath_clicked();
		void on_pushButtonLoad_clicked();
		void resetMouseDelta();

	private:

		Ui::MainWindowClass ui;

		static std::map<int, bool> keyMap;
		static std::map<int, bool> keyMapOnce;
		static QVector4D mouseDelta;

		QPoint lastMousePosition;
		QTimer resetMouseDeltaTimer;
	};
}
