#version 330

in vec3 worldPositionVarying;

out vec4 color;

uniform sampler3D texture0;

void main()
{
	color = texture(texture0, worldPositionVarying);
	color.a = 1.0f;
	
	if (worldPositionVarying.x < 0.0f || worldPositionVarying.x > 1.0f
	|| worldPositionVarying.y < 0.0f || worldPositionVarying.y > 1.0f
	|| worldPositionVarying.z < 0.0f || worldPositionVarying.z > 1.0f)
	{
		color.a = 0.5f;
	}
}
