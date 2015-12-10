#pragma once

#include <cstdint>
#include <vector>

namespace CellVision
{
	struct ImageLoaderResult
	{
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t count = 0;
		std::vector<uint32_t> data;
	};

	class ImageLoader
	{
		
	public:

		static ImageLoaderResult loadFromMultipageTiff(const std::string& filePath, int channelNumber, int imageCount);
	};
}
