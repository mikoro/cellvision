// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#pragma once

#include <cstdint>

namespace CellVision
{
	struct MetadataLoaderResult
	{
		uint64_t channelCount = 0;
		uint64_t imagesPerChannel = 0;
		float imageWidth = 0.0f;
		float imageHeight = 0.0f;
		float imageDepth = 0.0f;
	};

	class MetadataLoader
	{
		
	public:

		static MetadataLoaderResult loadFromFile(const std::string& fileName);
	};
}
