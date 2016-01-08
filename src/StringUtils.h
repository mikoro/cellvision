// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#pragma once

#include <string>

namespace CellVision
{
	class StringUtils
	{
	public:

		static std::istream& safeGetline(std::istream& is, std::string& t);
	};
}
