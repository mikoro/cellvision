#version 330

in vec3 worldPositionVarying;

out vec4 color;

uniform sampler3D texture0;

void main()
{
	color = texture(texture0, worldPositionVarying);
	color.a = 1.0f;
}
