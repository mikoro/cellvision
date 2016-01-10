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
	vec3 texcoord = worldPositionVarying;
	
	texcoord.y /= scaleY;
	texcoord.z /= scaleZ;
	
	texcoord.z = 1.0f - texcoord.z;
	
	if (texcoord.x < 0.0f || texcoord.x > 1.0f ||
	texcoord.y < 0.0f || texcoord.y > 1.0f ||
	texcoord.z < 0.0f || texcoord.z > 1.0f)
	{
		discard;
	}
	
	color = texture(texture0, texcoord);
	color.a = 1.0f;
}
