// Copyright (C) 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#version 330

in vec3 colorVarying;

out vec4 color;

void main()
{
	color = vec4(colorVarying, 0.5f);
}
