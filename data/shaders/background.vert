// Copyright (C) 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#version 330

in vec2 position;
in vec3 color;

out vec3 colorVarying;

void main()
{
	gl_Position = vec4(position, 0.0f, 1.0f);
	colorVarying = color;
}
