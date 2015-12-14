#include "stdafx.h"

#include "MainWindow.h"
#include "Log.h"
#include "ImageLoader.h"

using namespace CellVision;

std::map<int, bool> MainWindow::keyMap;
std::map<int, bool> MainWindow::keyMapOnce;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
	ui.setupUi(this);

	resize(800, 800);
	ui.splitterMain->setSizes({ 500, 10 });

	ui.renderWidget->setUI(&ui);
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

bool MainWindow::event(QEvent* event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* ke = static_cast<QKeyEvent*>(event);

		if (!ke->isAutoRepeat())
			keyMap[ke->key()] = true;
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

void MainWindow::on_horizontalSliderZDepth_valueChanged()
{
	double value = ui.horizontalSliderZDepth->value() / 10000.0;
	ui.doubleSpinBoxZDepth->setValue(value);
}
