// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#include "stdafx.h"

#include "MetadataLoader.h"
#include "MainWindow.h"
#include "Log.h"
#include "StringUtils.h"

using namespace CellVision;

MetadataLoaderResult MetadataLoader::loadFromFile(const std::string& fileName)
{
	Log& log = MainWindow::getLog();
	log.logInfo("Loading metadata from %s", fileName);

	std::ifstream file(fileName);

	if (!file.good())
	{
		log.logWarning("Could not open metadata file");
		return MetadataLoaderResult();
	}

	MetadataLoaderResult result;
	std::string line;

	while (StringUtils::safeGetline(file, line))
	{
		std::replace(line.begin(), line.end(), '\t', ' ');
		std::stringstream ss(line);
		std::string part;
		ss >> part;
		
		if (part == "SizeC")
			ss >> result.channelCount;
		else if (part == "SizeZ")
			ss >> result.imagesPerChannel;
		else if (part == "HardwareSetting|ScannerSettingRecord|dblSizeX")
		{
			ss >> part;
			ss >> result.imageWidth;
		}
		else if (part == "HardwareSetting|ScannerSettingRecord|dblSizeY")
		{
			ss >> part;
			ss >> result.imageHeight;
		}
		else if (part == "HardwareSetting|ScannerSettingRecord|dblSizeZ")
		{
			ss >> part;
			ss >> result.imageDepth;
		}
	}

	return result;
}
