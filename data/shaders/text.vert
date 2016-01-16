#version 330

// Copyright (C) 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

in vec2 position;
in vec2 texcoord;

out vec2 texcoordVarying;

void main()
{
	gl_Position = vec4(position, 0.0f, 1.0f);
	texcoordVarying = texcoord;
}
