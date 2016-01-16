#version 330

// Copyright (C) 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

out vec4 color;

uniform vec4 lineColor;

void main()
{
	color = lineColor;
}
