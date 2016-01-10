// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#include "stdafx.h"

#include "MathHelper.h"

using namespace CellVision;

void MathHelper::orthonormalize(QMatrix4x4& matrix)
{
	QVector3D x = matrix.column(0).toVector3D();
	QVector3D y = matrix.column(1).toVector3D();
	QVector3D z = matrix.column(2).toVector3D();

	QVector3D nx = x.normalized();
	QVector3D ny = y.normalized();

	y = y - QVector3D::dotProduct(y, x) * nx;
	z = z - QVector3D::dotProduct(z, x) * nx - QVector3D::dotProduct(z, y) * ny;

	x.normalize();
	y.normalize();
	z.normalize();

	matrix.setColumn(0, QVector4D(x, 0.0f));
	matrix.setColumn(1, QVector4D(y, 0.0f));
	matrix.setColumn(2, QVector4D(z, 0.0f));
}
