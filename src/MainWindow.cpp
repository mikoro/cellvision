#include "stdafx.h"

#include "MainWindow.h"
#include "Log.h"
#include "ImageLoader.h"

using namespace CellVision;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
	ui.setupUi(this);

	resize(1280, 800);
	ui.splitterMain->setSizes({ 500, 10 });

	ui.renderWidget->setUI(&ui);
}

Log& MainWindow::getLog()
{
	static Log log("cellvision.log");
	return log;
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
	std::string filePath = ui.lineEditImageFilePath->text().toStdString();
	int imagesPerChannel = ui.spinBoxImagesPerChannel->value();
	int selectedChannel = ui.spinBoxSelectedChannel->value();

	ImageLoaderResult result = ImageLoader::loadFromMultipageTiff(filePath, selectedChannel, imagesPerChannel);
}

void MainWindow::on_horizontalSliderZDepth_valueChanged()
{
	double value = ui.horizontalSliderZDepth->value() / 10000.0;
	ui.doubleSpinBoxZDepth->setValue(value);
}
