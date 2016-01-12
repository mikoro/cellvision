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

QMatrix4x4 MathHelper::rotationMatrix(float angle, const QVector3D& axis)
{
	angle *= (float(M_PI) / 180.0f);

	float cosine = cos(angle);
	float invCosine = 1.0f - cosine;
	float sine = sin(angle);
	float x = axis.x();
	float y = axis.y();
	float z = axis.z();

	QMatrix4x4 result;

	result(0, 0) = cosine + x * x * invCosine;
	result(0, 1) = x * y * invCosine - z * sine;
	result(0, 2) = x * z * invCosine + y * sine;
	result(0, 3) = 0.0f;

	result(1, 0) = y * x * invCosine + z * sine;
	result(1, 1) = cosine + y * y * invCosine;
	result(1, 2) = y * z * invCosine - x * sine;
	result(1, 3) = 0.0f;

	result(2, 0) = z * x * invCosine - y * sine;
	result(2, 1) = z * y * invCosine + x * sine;
	result(2, 2) = cosine + z * z * invCosine;
	result(2, 3) = 0.0f;

	result(3, 0) = 0.0f;
	result(3, 1) = 0.0f;
	result(3, 2) = 0.0f;
	result(3, 3) = 1.0f;

	return result;
}
