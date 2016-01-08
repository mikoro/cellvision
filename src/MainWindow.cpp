// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#include "stdafx.h"

#include "MainWindow.h"
#include "Log.h"
#include "ImageLoader.h"
#include "MetadataLoader.h"

using namespace CellVision;

std::map<int, bool> MainWindow::keyMap;
std::map<int, bool> MainWindow::keyMapOnce;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
	ui.setupUi(this);

	resize(1280, 1000);
	ui.splitterMain->setSizes({ 1000, 1 });

	setFocus();

	updateChannelSelectors();

	connect(ui.checkBoxRedChannelEnabled, SIGNAL(stateChanged(int)), this, SLOT(updateChannelSelectors()));
	connect(ui.checkBoxGreenChannelEnabled, SIGNAL(stateChanged(int)), this, SLOT(updateChannelSelectors()));
	connect(ui.checkBoxBlueChannelEnabled, SIGNAL(stateChanged(int)), this, SLOT(updateChannelSelectors()));
	connect(ui.checkBoxGrayscaleChannelEnabled, SIGNAL(stateChanged(int)), this, SLOT(updateChannelSelectors()));

	pixelValueValidator.setLocale(QLocale::English);

	ui.lineEditPixelWidth->setValidator(&pixelValueValidator);
	ui.lineEditPixelHeight->setValidator(&pixelValueValidator);
	ui.lineEditPixelDepth->setValidator(&pixelValueValidator);
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

	return QMainWindow::event(event);
}

void MainWindow::on_pushButtonBrowseTiffImage_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setWindowTitle(tr("Select TIFF image file"));
	fileDialog.setNameFilter(tr("TIFF files (*.tif);;All files (*.*)"));

	if (fileDialog.exec())
		ui.lineEditTiffImageFileName->setText(fileDialog.selectedFiles().at(0));
}

void MainWindow::on_pushButtonBrowseMetadataFile_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setWindowTitle(tr("Select metadata file"));
	fileDialog.setNameFilter(tr("Text files (*.txt);;All files (*.*)"));

	if (fileDialog.exec())
		ui.lineEditMetadataFileName->setText(fileDialog.selectedFiles().at(0));
}

void MainWindow::on_pushButtonLoadFromMetadata_clicked()
{
	this->setCursor(Qt::WaitCursor);

	std::string fileName = ui.lineEditMetadataFileName->text().toStdString();

	MetadataLoaderResult result = MetadataLoader::loadFromFile(fileName);
	
	QLocale locale(QLocale::English);

	ui.spinBoxChannelCount->setValue(result.channelCount);
	ui.spinBoxImagesPerChannel->setValue(result.imagesPerChannel);
	ui.lineEditPixelWidth->setText(locale.toString(result.pixelWidth));
	ui.lineEditPixelHeight->setText(locale.toString(result.pixelHeight));
	ui.lineEditPixelDepth->setText(locale.toString(result.pixelDepth));

	this->setCursor(Qt::ArrowCursor);
}

void MainWindow::on_pushButtonLoadAndDisplay_clicked()
{
	this->setCursor(Qt::WaitCursor);

	std::string fileName = ui.lineEditTiffImageFileName->text().toStdString();

	ImageLoaderResult result = ImageLoader::loadFromMultipageTiff(fileName, 0, 0, 0);
	//ui.renderWidget->uploadImageData(result);

	this->setCursor(Qt::ArrowCursor);
}

void MainWindow::on_pushButtonShowFullscreen_clicked()
{
}

void MainWindow::updateChannelSelectors()
{
	if (ui.checkBoxGrayscaleChannelEnabled->isChecked())
	{
		ui.checkBoxRedChannelEnabled->setEnabled(false);
		ui.checkBoxGreenChannelEnabled->setEnabled(false);
		ui.checkBoxBlueChannelEnabled->setEnabled(false);

		ui.spinBoxRedChannel->setEnabled(false);
		ui.spinBoxGreenChannel->setEnabled(false);
		ui.spinBoxBlueChannel->setEnabled(false);
		ui.spinBoxGrayscaleChannel->setEnabled(true);
	}
	else
	{
		ui.checkBoxRedChannelEnabled->setEnabled(true);
		ui.checkBoxGreenChannelEnabled->setEnabled(true);
		ui.checkBoxBlueChannelEnabled->setEnabled(true);

		ui.spinBoxRedChannel->setEnabled(ui.checkBoxRedChannelEnabled->isChecked());
		ui.spinBoxGreenChannel->setEnabled(ui.checkBoxGreenChannelEnabled->isChecked());
		ui.spinBoxBlueChannel->setEnabled(ui.checkBoxBlueChannelEnabled->isChecked());
		ui.spinBoxGrayscaleChannel->setEnabled(false);
	}
}
