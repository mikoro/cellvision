// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#include "stdafx.h"

#include "ImageLoader.h"
#include "MainWindow.h"
#include "Log.h"

using namespace CellVision;

ImageLoaderResult ImageLoader::loadFromMultipageTiff(const ImageLoaderInfo& info)
{
	Log& log = MainWindow::getLog();
	log.logInfo("Loading multipage TIFF image from %s", info.fileName);

	TIFF* tiffFile = TIFFOpen(info.fileName.c_str(), "r");

	if (tiffFile == nullptr)
	{
		log.logWarning("Could not open image file");
		return ImageLoaderResult();
	}

	ImageLoaderResult result;

	TIFFGetField(tiffFile, TIFFTAG_IMAGEWIDTH, &result.width);
	TIFFGetField(tiffFile, TIFFTAG_IMAGELENGTH, &result.height);

	result.depth = info.imagesPerChannel;
	result.data.reserve(result.width * result.height * result.depth);

	std::vector<uint32_t> tempRedData(result.width * result.height);
	std::vector<uint32_t> tempGreenData(result.width * result.height);
	std::vector<uint32_t> tempBlueData(result.width * result.height);
	std::vector<uint32_t> tempData(result.width * result.height);

	std::fill(tempRedData.begin(), tempRedData.end(), 0);
	std::fill(tempGreenData.begin(), tempGreenData.end(), 0);
	std::fill(tempBlueData.begin(), tempBlueData.end(), 0);

	for (uint16_t i = 0; i < info.imagesPerChannel; ++i)
	{
		if (info.redChannelEnabled)
		{
			uint16_t directoryIndex = i * info.channelCount + info.redChannelIndex - 1;

			if (!readImageData(tiffFile, directoryIndex, result.width, result.height, &tempRedData[0]))
				return ImageLoaderResult();
		}

		if (info.greenChannelEnabled)
		{
			uint16_t directoryIndex = i * info.channelCount + info.greenChannelIndex - 1;

			if (!readImageData(tiffFile, directoryIndex, result.width, result.height, &tempGreenData[0]))
				return ImageLoaderResult();
		}
		
		if (info.blueChannelEnabled)
		{
			uint16_t directoryIndex = i * info.channelCount + info.blueChannelIndex - 1;

			if (!readImageData(tiffFile, directoryIndex, result.width, result.height, &tempBlueData[0]))
				return ImageLoaderResult();
		}

		for (uint64_t j = 0; j < result.width * result.height; ++j)
		{
			uint32_t red = tempRedData[j];
			uint32_t green = tempGreenData[j];
			uint32_t blue = tempBlueData[j];
			uint32_t combined = 0xff000000;

			combined |= red & 0x000000ff;
			combined |= (green & 0x000000ff) << 8;
			combined |= (blue & 0x000000ff) << 16;

			tempData[j] = combined;
		}

		result.data.insert(result.data.end(), tempData.begin(), tempData.end());
	}

	TIFFClose(tiffFile);

	return result;
}

bool ImageLoader::readImageData(TIFF* tiffFile, uint16_t directoryIndex, uint32_t width, uint32_t height, uint32_t* data)
{
	Log& log = MainWindow::getLog();

	if (!TIFFSetDirectory(tiffFile, directoryIndex))
	{
		log.logWarning("Could not set TIFF directory");
		TIFFClose(tiffFile);
		return false;
	}

	if (!TIFFReadRGBAImage(tiffFile, width, height, data, 0))
	{
		log.logWarning("Could not read TIFF rgba data");
		TIFFClose(tiffFile);
		return false;
	}
	
	return true;
}
