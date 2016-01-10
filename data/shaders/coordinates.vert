// Copyright (C) 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#version 330

in vec3 position;
in vec3 color;
in float distance;

out vec3 colorVarying;
out float distanceVarying;

uniform mat4 mvp;

void main()
{
	gl_Position = mvp * vec4(position, 1.0f);
	colorVarying = color;
	distanceVarying = distance;
}
