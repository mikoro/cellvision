// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#include "stdafx.h"

#include "MainWindow.h"
#include "Log.h"
#include "ImageLoader.h"

using namespace CellVision;

std::map<int, bool> MainWindow::keyMap;
std::map<int, bool> MainWindow::keyMapOnce;
QVector4D MainWindow::mouseDelta;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
	ui.setupUi(this);

	resize(1280, 1000);
	ui.splitterMain->setSizes({ 1000, 1 });

	ui.renderWidget->setUI(&ui);

	resetMouseDeltaTimer.setInterval(100);
	resetMouseDeltaTimer.setSingleShot(true);

	connect(&resetMouseDeltaTimer, SIGNAL(timeout()), this, SLOT(resetMouseDelta()));

	setFocus();
}

Log& MainWindow::getLog()
{
	static Log log("cellvision.log");
	return log;
}

bool MainWindow::keyIsDown(int key)
{
	if (keyMap.count(key) == 0)
		return false;

	return keyMap[key];
}

bool MainWindow::keyIsDownOnce(int key)
{
	if (keyMap.count(key) == 0 || keyMapOnce[key])
		return false;

	if (keyMap[key])
	{
		keyMapOnce[key] = true;
		return true;
	}

	return false;
}

QVector4D MainWindow::getMouseDelta()
{
	return mouseDelta;
}

bool MainWindow::event(QEvent* event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* ke = static_cast<QKeyEvent*>(event);

		if (!ke->isAutoRepeat())
			keyMap[ke->key()] = true;

		if (ke->key() == Qt::Key_Escape)
			close();
	}

	if (event->type() == QEvent::KeyRelease)
	{
		QKeyEvent* ke = static_cast<QKeyEvent*>(event);

		if (!ke->isAutoRepeat())
		{
			keyMap[ke->key()] = false;
			keyMapOnce[ke->key()] = false;
		}
	}

	if (event->type() == QEvent::MouseMove)
	{
		QMouseEvent* me = static_cast<QMouseEvent*>(event);

		if (me->buttons() == Qt::LeftButton)
		{
			QPoint tempDelta = me->globalPos() - lastMousePosition;
			mouseDelta.setX(tempDelta.x());
			mouseDelta.setY(tempDelta.y());
		}

		lastMousePosition = me->globalPos();
		resetMouseDeltaTimer.start();
	}

	if (event->type() == QEvent::Wheel)
	{
		QWheelEvent* we = static_cast<QWheelEvent*>(event);

		mouseDelta.setZ(we->angleDelta().x() / 120.0);
		mouseDelta.setW(we->angleDelta().y() / 120.0);

		resetMouseDeltaTimer.start();
	}

	if (event->type() == QEvent::MouseButtonRelease)
	{
		mouseDelta.setX(0.0f);
		mouseDelta.setY(0.0f);
		mouseDelta.setZ(0.0f);
		mouseDelta.setW(0.0f);
	}

	return QMainWindow::event(event);
}

void MainWindow::on_pushButtonBrowseImageFilePath_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setWindowTitle(tr("Select image file"));
	fileDialog.setNameFilter(tr("TIFF files (*.tif);;All files (*.*)"));

	if (fileDialog.exec())
		ui.lineEditImageFilePath->setText(fileDialog.selectedFiles().at(0));
}

void MainWindow::on_pushButtonLoad_clicked()
{
	this->setCursor(Qt::WaitCursor);

	std::string filePath = ui.lineEditImageFilePath->text().toStdString();
	int channelCount = ui.spinBoxChannelCount->value();
	int imagesPerChannel = ui.spinBoxImagesPerChannel->value();
	int selectedChannel = ui.spinBoxSelectedChannel->value();

	ImageLoaderResult result = ImageLoader::loadFromMultipageTiff(filePath, channelCount, imagesPerChannel, selectedChannel);

	ui.renderWidget->uploadImageData(result);

	this->setCursor(Qt::ArrowCursor);
}

void MainWindow::resetMouseDelta()
{
	mouseDelta.setX(0.0f);
	mouseDelta.setY(0.0f);
	mouseDelta.setZ(0.0f);
	mouseDelta.setW(0.0f);
}
