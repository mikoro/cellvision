#version 330

in vec3 texcoordVarying;

out vec4 color;

uniform sampler2D texture0;

void main()
{
	//color = texture(texture0, texcoordVarying.xy);
	color = vec4(1, 0.5, 0, 1);
}
