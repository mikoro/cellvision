// Copyright (C) 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#version 330

in vec3 position;

out vec3 worldPositionVarying;

uniform mat4 modelMatrix;
uniform mat4 mvp;

void main()
{
	worldPositionVarying = (modelMatrix * vec4(position, 1.0f)).xyz;
	gl_Position = mvp * vec4(position, 1.0f);
}
