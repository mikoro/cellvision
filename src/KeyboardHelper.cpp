// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#include "stdafx.h"

#include "KeyboardHelper.h"

using namespace CellVision;

void KeyboardHelper::event(QEvent* e)
{
	if (e->type() == QEvent::KeyPress)
	{
		QKeyEvent* ke = static_cast<QKeyEvent*>(e);

		if (!ke->isAutoRepeat())
			keyMap[ke->key()] = true;
	}

	if (e->type() == QEvent::KeyRelease)
	{
		QKeyEvent* ke = static_cast<QKeyEvent*>(e);

		if (!ke->isAutoRepeat())
		{
			keyMap[ke->key()] = false;
			keyMapOnce[ke->key()] = false;
		}
	}
}

bool KeyboardHelper::keyIsDown(int key)
{
	if (keyMap.count(key) == 0)
		return false;

	return keyMap[key];
}

bool KeyboardHelper::keyIsDownOnce(int key)
{
	if (keyMap.count(key) == 0 || keyMapOnce[key])
		return false;

	if (keyMap[key])
	{
		keyMapOnce[key] = true;
		return true;
	}

	return false;
}
