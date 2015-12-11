#version 330

in vec3 texcoordVarying;

out vec4 color;

uniform sampler3D texture0;

void main()
{
	color = texture(texture0, texcoordVarying);
}
