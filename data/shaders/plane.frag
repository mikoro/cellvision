// Copyright (C) 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#version 330

in vec3 worldPositionVarying;

out vec4 color;

uniform sampler3D texture0;
uniform float scaleY;
uniform float scaleZ;

void main()
{
	vec3 wp = worldPositionVarying;
	
	wp.y /= scaleY;
	wp.z /= scaleZ;
	
	color = texture(texture0, wp);
	color.a = 1.0f;
	
	if (wp.x < 0.0f || wp.x > 1.0f
	|| wp.y < 0.0f || wp.y > 1.0f
	|| wp.z < 0.0f || wp.z > 1.0f)
	{
		discard;
	}
}
