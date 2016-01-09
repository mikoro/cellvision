// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#pragma once

#include <map>

namespace CellVision
{
	class KeyboardHelper
	{
	public:

		void event(QEvent* e);

		bool keyIsDown(int key);
		bool keyIsDownOnce(int key);

	private:

		std::map<int, bool> keyMap;
		std::map<int, bool> keyMapOnce;
	};
}
