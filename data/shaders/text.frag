#version 330

// Copyright (C) 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

in vec2 texcoordVarying;

out vec4 color;

uniform sampler2D tex0;

void main()
{
	vec2 texcoord = vec2(texcoordVarying.x, -texcoordVarying.y);
	color = texture(tex0, texcoord);
}
