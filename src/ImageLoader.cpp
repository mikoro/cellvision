// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#include "stdafx.h"

#include "ImageLoader.h"
#include "MainWindow.h"
#include "Log.h"

using namespace CellVision;

ImageLoaderResult ImageLoader::loadFromMultipageTiff(const std::string& filePath, int channelCount, int imagesPerChannel, int selectedChannel)
{
	Log& log = MainWindow::getLog();
	log.logInfo("Loading multipage TIFF image from %s", filePath);

	TIFF* tiffFile = TIFFOpen(filePath.c_str(), "r");

	if (tiffFile == nullptr)
	{
		log.logWarning("Could not open image file");
		return ImageLoaderResult();
	}

	ImageLoaderResult result;

	TIFFGetField(tiffFile, TIFFTAG_IMAGEWIDTH, &result.width);
	TIFFGetField(tiffFile, TIFFTAG_IMAGELENGTH, &result.height);

	result.depth = imagesPerChannel;
	result.data.reserve(result.width * result.height * result.depth);

	std::vector<uint32_t> tempData(result.width * result.height);

	for (int i = 0; i < imagesPerChannel; ++i)
	{
		int directoryIndex = i * channelCount + selectedChannel;

		if (!TIFFSetDirectory(tiffFile, directoryIndex))
		{
			log.logWarning("Could not set TIFF directory");
			TIFFClose(tiffFile);
			return ImageLoaderResult();
		}

		if (TIFFReadRGBAImage(tiffFile, result.width, result.height, &tempData[0], 0))
		{
			result.data.insert(result.data.end(), tempData.begin(), tempData.end());

			//std::string tempFileName = tfm::format("temp_%d.png", i);
			//QImage tempImage(reinterpret_cast<uchar*>(&tempData[0]), result.width, result.height, QImage::Format_ARGB32, 0, 0);
			//tempImage.rgbSwapped().save(QString::fromStdString(tempFileName));
		}
		else
		{
			log.logWarning("Could not read TIFF rgba data");
			TIFFClose(tiffFile);
			return ImageLoaderResult();
		}
	}

	TIFFClose(tiffFile);

	return result;
}
