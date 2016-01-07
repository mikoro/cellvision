// Copyright (C) 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#version 330

in vec3 texcoordVarying;

out vec4 color;

uniform sampler3D texture0;

void main()
{
	color = texture(texture0, texcoordVarying);
	color.a = 1.0f;
}
