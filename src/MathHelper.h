// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#pragma once

#include <QMatrix4x4>

namespace CellVision
{
	class MathHelper
	{
	public:

		static void orthonormalize(QMatrix4x4& matrix);
		static QMatrix4x4 rotationMatrix(float angle, const QVector3D& axis);
	};
}
