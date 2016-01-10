// Copyright (C) 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#version 330

in vec3 position;

uniform mat4 mvp;

void main()
{
	gl_Position = mvp * vec4(position, 1.0f);
}
