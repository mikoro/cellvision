// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#include "stdafx.h"

#include "MainWindow.h"
#include "Log.h"
#include "ImageLoader.h"
#include "MetadataLoader.h"

using namespace CellVision;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
	ui.setupUi(this);

	resize(1280, 1000);
	ui.splitterMain->setSizes({ 1000, 1 });

	QLocale locale(QLocale::English);

	pixelValueValidator.setLocale(locale);

	ui.lineEditPixelWidth->setValidator(&pixelValueValidator);
	ui.lineEditPixelHeight->setValidator(&pixelValueValidator);
	ui.lineEditPixelDepth->setValidator(&pixelValueValidator);

	QSettings settings;

	ui.lineEditTiffImageFileName->setText(settings.value("tiffImageFileName", "").toString());
	ui.lineEditMetadataFileName->setText(settings.value("metadataFileName", "").toString());
	ui.spinBoxChannelCount->setValue(settings.value("channelCount", 1).toInt());
	ui.spinBoxImagesPerChannel->setValue(settings.value("imagesPerChannel", 1).toInt());
	ui.lineEditPixelWidth->setText(locale.toString(settings.value("pixelWidth", 1.0).toDouble()));
	ui.lineEditPixelHeight->setText(locale.toString(settings.value("pixelHeight", 1.0).toDouble()));
	ui.lineEditPixelDepth->setText(locale.toString(settings.value("pixelDepth", 1.0).toDouble()));
	ui.spinBoxRedChannel->setValue(settings.value("redChannel", 1).toInt());
	ui.spinBoxGreenChannel->setValue(settings.value("greenChannel", 1).toInt());
	ui.spinBoxBlueChannel->setValue(settings.value("blueChannel", 1).toInt());
	ui.checkBoxRedChannelEnabled->setChecked(settings.value("redChannelEnabled", false).toBool());
	ui.checkBoxGreenChannelEnabled->setChecked(settings.value("greenChannelEnabled", false).toBool());
	ui.checkBoxBlueChannelEnabled->setChecked(settings.value("blueChannelEnabled", false).toBool());
	backgroundColor = settings.value("backgroundColor", QColor(0, 0, 0, 255)).value<QColor>();
	lineColor = settings.value("lineColor", QColor(255, 255, 255, 255)).value<QColor>();

	updateChannelSelectors();
	updateFrameColors();

	connect(ui.spinBoxChannelCount, SIGNAL(valueChanged(int)), this, SLOT(updateChannelSelectors()));
	connect(ui.checkBoxRedChannelEnabled, SIGNAL(stateChanged(int)), this, SLOT(updateChannelSelectors()));
	connect(ui.checkBoxGreenChannelEnabled, SIGNAL(stateChanged(int)), this, SLOT(updateChannelSelectors()));
	connect(ui.checkBoxBlueChannelEnabled, SIGNAL(stateChanged(int)), this, SLOT(updateChannelSelectors()));
}

Log& MainWindow::getLog()
{
	static Log log("cellvision.log");
	return log;
}

void MainWindow::keyPressEvent(QKeyEvent* ke)
{
	if (ke->key() == Qt::Key_Escape)
		close();

	ke->accept();
}

void MainWindow::closeEvent(QCloseEvent* ce)
{
	QSettings settings;

	settings.setValue("tiffImageFileName", ui.lineEditTiffImageFileName->text());
	settings.setValue("metadataFileName", ui.lineEditMetadataFileName->text());
	settings.setValue("channelCount", ui.spinBoxChannelCount->value());
	settings.setValue("imagesPerChannel", ui.spinBoxImagesPerChannel->value());
	settings.setValue("pixelWidth", ui.lineEditPixelWidth->text());
	settings.setValue("pixelHeight", ui.lineEditPixelHeight->text());
	settings.setValue("pixelDepth", ui.lineEditPixelDepth->text());
	settings.setValue("redChannel", ui.spinBoxRedChannel->value());
	settings.setValue("greenChannel", ui.spinBoxGreenChannel->value());
	settings.setValue("blueChannel", ui.spinBoxBlueChannel->value());
	settings.setValue("redChannelEnabled", ui.checkBoxRedChannelEnabled->isChecked());
	settings.setValue("greenChannelEnabled", ui.checkBoxGreenChannelEnabled->isChecked());
	settings.setValue("blueChannelEnabled", ui.checkBoxBlueChannelEnabled->isChecked());
	settings.setValue("backgroundColor", backgroundColor);
	settings.setValue("lineColor", lineColor);

	ce->accept();
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

void MainWindow::on_pushButtonLoadWindowed_clicked()
{
	this->setCursor(Qt::WaitCursor);

	std::string fileName = ui.lineEditTiffImageFileName->text().toStdString();

	ImageLoaderResult result = ImageLoader::loadFromMultipageTiff(fileName, 0, 0, 0);
	//ui.renderWidget->uploadImageData(result);

	this->setCursor(Qt::ArrowCursor);
}

void MainWindow::on_pushButtonLoadFullscreen_clicked()
{
	QDialog* dialog = new QDialog(this);
	QHBoxLayout* layout = new QHBoxLayout(dialog);
	RenderWidget* renderWidget = new RenderWidget(dialog);

	ui.renderWidget->hide();

	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(renderWidget);
	dialog->setLayout(layout);
	dialog->showFullScreen();

	connect(dialog, SIGNAL(rejected()), this, SLOT(fullscreenDialogClosed()));
	connect(dialog, SIGNAL(accepted()), this, SLOT(fullscreenDialogClosed()));
}

void MainWindow::on_pushButtonPickBackgroundColor_clicked()
{
	QColorDialog colorDialog;
	QColor color = colorDialog.getColor(Qt::white, this, "Pick background color", QColorDialog::ShowAlphaChannel);

	if (color.isValid())
	{
		backgroundColor = color;
		updateFrameColors();
	}
}

void MainWindow::on_pushButtonPickLineColor_clicked()
{
	QColorDialog colorDialog;
	QColor color = colorDialog.getColor(Qt::white, this, "Pick line color", QColorDialog::ShowAlphaChannel);

	if (color.isValid())
	{
		lineColor = color;
		updateFrameColors();
	}
}

void MainWindow::updateChannelSelectors()
{
	ui.spinBoxRedChannel->setEnabled(ui.checkBoxRedChannelEnabled->isChecked());
	ui.spinBoxGreenChannel->setEnabled(ui.checkBoxGreenChannelEnabled->isChecked());
	ui.spinBoxBlueChannel->setEnabled(ui.checkBoxBlueChannelEnabled->isChecked());

	ui.spinBoxRedChannel->setMaximum(ui.spinBoxChannelCount->value());
	ui.spinBoxGreenChannel->setMaximum(ui.spinBoxChannelCount->value());
	ui.spinBoxBlueChannel->setMaximum(ui.spinBoxChannelCount->value());
}

void MainWindow::updateFrameColors()
{
	ui.frameBackgroundColor->setStyleSheet(QString("background-color: rgb(%1, %2, %3, %4);").arg(QString::number(backgroundColor.red()), QString::number(backgroundColor.green()), QString::number(backgroundColor.blue()), QString::number(backgroundColor.alpha())));
	ui.frameLineColor->setStyleSheet(QString("background-color: rgb(%1, %2, %3, %4);").arg(QString::number(lineColor.red()), QString::number(lineColor.green()), QString::number(lineColor.blue()), QString::number(lineColor.alpha())));
}

void MainWindow::fullscreenDialogClosed()
{
	ui.renderWidget->show();
}
