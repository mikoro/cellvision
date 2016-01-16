#version 330

// Copyright (C) 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

in vec3 colorVarying;

out vec4 color;

void main()
{
	color = vec4(colorVarying, 1.0f);
}
