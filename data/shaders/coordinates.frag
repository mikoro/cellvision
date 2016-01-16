#version 330

// Copyright (C) 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

in vec3 colorVarying;
in float distanceVarying;

out vec4 color;

void main()
{
	float distance = abs(distanceVarying);
	distance = distance * distance;
	float alpha = (1.0f - distance) * 0.5f;
	
	color = vec4(colorVarying, alpha);
}
