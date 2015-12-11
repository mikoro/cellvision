#version 330

in vec2 position;
in vec2 texcoord;

out vec3 texcoordVarying;

uniform float texcoordZ;

void main()
{
	texcoordVarying = vec3(texcoord, texcoordZ);
	gl_Position = vec4(position, 0.0f, 1.0f);
}
