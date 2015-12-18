#version 330

in vec3 position;
in vec3 texcoord;

out vec3 texcoordVarying;

uniform mat4 mvp;

float texcoordScale = 0.99f;

void main()
{
	texcoordVarying = texcoord * texcoordScale;
	gl_Position = mvp * vec4(position, 1.0f);
}
