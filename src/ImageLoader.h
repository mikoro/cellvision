// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#pragma once

#include <cstdint>
#include <vector>

namespace CellVision
{
	struct ImageLoaderInfo
	{
		std::string fileName;
		uint16_t channelCount;
		uint16_t imagesPerChannel;
		bool redChannelEnabled;
		bool greenChannelEnabled;
		bool blueChannelEnabled;
		uint16_t redChannelIndex;
		uint16_t greenChannelIndex;
		uint16_t blueChannelIndex;
	};

	struct ImageLoaderResult
	{
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t depth = 0;
		std::vector<uint32_t> data;
	};

	class ImageLoader
	{
		
	public:

		static ImageLoaderResult loadFromMultipageTiff(const ImageLoaderInfo& info);

	private:

		static bool readImageData(TIFF* tiffFile, uint16_t directoryIndex, uint32_t width, uint32_t height, uint32_t* data);
	};
}
