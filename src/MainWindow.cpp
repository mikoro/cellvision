// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#include "stdafx.h"

#include "MainWindow.h"
#include "Log.h"
#include "MetadataLoader.h"
#include "ImageLoader.h"

using namespace CellVision;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
	ui.setupUi(this);

	QLocale locale(QLocale::English);

	doubleValueValidator.setLocale(locale);

	ui.lineEditImageWidth->setValidator(&doubleValueValidator);
	ui.lineEditImageHeight->setValidator(&doubleValueValidator);
	ui.lineEditImageDepth->setValidator(&doubleValueValidator);

	QSettings settings;

	resize(settings.value("windowSize", QSize(1280, 1000)).toSize());
	ui.splitterMain->setSizes({ 1000, 1 });

	ui.lineEditTiffImageFileName->setText(settings.value("tiffImageFileName", "").toString());
	ui.lineEditMetadataFileName->setText(settings.value("metadataFileName", "").toString());
	ui.spinBoxChannelCount->setValue(settings.value("channelCount", 1).toInt());
	ui.spinBoxImagesPerChannel->setValue(settings.value("imagesPerChannel", 1).toInt());
	ui.lineEditImageWidth->setText(locale.toString(settings.value("imageWidth", 1.0).toDouble()));
	ui.lineEditImageHeight->setText(locale.toString(settings.value("imageHeight", 1.0).toDouble()));
	ui.lineEditImageDepth->setText(locale.toString(settings.value("imageDepth", 1.0).toDouble()));
	ui.spinBoxRedChannel->setValue(settings.value("redChannel", 1).toInt());
	ui.spinBoxGreenChannel->setValue(settings.value("greenChannel", 1).toInt());
	ui.spinBoxBlueChannel->setValue(settings.value("blueChannel", 1).toInt());
	ui.checkBoxRedChannelEnabled->setChecked(settings.value("redChannelEnabled", false).toBool());
	ui.checkBoxGreenChannelEnabled->setChecked(settings.value("greenChannelEnabled", false).toBool());
	ui.checkBoxBlueChannelEnabled->setChecked(settings.value("blueChannelEnabled", false).toBool());
	backgroundColor = settings.value("backgroundColor", QColor(100, 100, 100, 255)).value<QColor>();
	lineColor = settings.value("lineColor", QColor(255, 255, 255, 128)).value<QColor>();

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

	if (ke->key() == Qt::Key_F)
	{
		if (ui.splitterMain->sizes()[1] != 0)
			ui.splitterMain->setSizes({ 1000, 0 });
		else
			ui.splitterMain->setSizes({ 1000, 1 });
	}

	ke->accept();
}

void MainWindow::closeEvent(QCloseEvent* ce)
{
	QSettings settings;

	settings.setValue("windowSize", this->size());
	settings.setValue("tiffImageFileName", ui.lineEditTiffImageFileName->text());
	settings.setValue("metadataFileName", ui.lineEditMetadataFileName->text());
	settings.setValue("channelCount", ui.spinBoxChannelCount->value());
	settings.setValue("imagesPerChannel", ui.spinBoxImagesPerChannel->value());
	settings.setValue("imageWidth", ui.lineEditImageWidth->text());
	settings.setValue("imageHeight", ui.lineEditImageHeight->text());
	settings.setValue("imageDepth", ui.lineEditImageDepth->text());
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
	ui.lineEditImageWidth->setText(locale.toString(result.imageWidth));
	ui.lineEditImageHeight->setText(locale.toString(result.imageHeight));
	ui.lineEditImageDepth->setText(locale.toString(result.imageDepth));

	this->setCursor(Qt::ArrowCursor);
}

void MainWindow::on_pushButtonLoadWindowed_clicked()
{
	this->setCursor(Qt::WaitCursor);

	QLocale locale(QLocale::English);

	ImageLoaderInfo info;
	info.fileName = ui.lineEditTiffImageFileName->text().toStdString();
	info.channelCount = ui.spinBoxChannelCount->value();
	info.imagesPerChannel = ui.spinBoxImagesPerChannel->value();
	info.redChannelEnabled = ui.checkBoxRedChannelEnabled->isChecked();
	info.greenChannelEnabled = ui.checkBoxGreenChannelEnabled->isChecked();
	info.blueChannelEnabled = ui.checkBoxBlueChannelEnabled->isChecked();
	info.redChannelIndex = ui.spinBoxRedChannel->value();
	info.greenChannelIndex = ui.spinBoxGreenChannel->value();
	info.blueChannelIndex = ui.spinBoxBlueChannel->value();

	RenderWidgetSettings settings;
	settings.imageLoaderInfo = info;
	settings.backgroundColor = backgroundColor;
	settings.lineColor = lineColor;
	settings.imageWidth = locale.toFloat(ui.lineEditImageWidth->text());
	settings.imageHeight = locale.toFloat(ui.lineEditImageHeight->text());
	settings.imageDepth = locale.toFloat(ui.lineEditImageDepth->text());

	ui.renderWidget->initialize(settings);
	ui.renderWidget->setFocus();

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
	QColor color = colorDialog.getColor(backgroundColor, this, "Pick background color", QColorDialog::ShowAlphaChannel);

	if (color.isValid())
	{
		backgroundColor = color;
		updateFrameColors();
	}
}

void MainWindow::on_pushButtonPickLineColor_clicked()
{
	QColorDialog colorDialog;
	QColor color = colorDialog.getColor(lineColor, this, "Pick line color", QColorDialog::ShowAlphaChannel);

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
