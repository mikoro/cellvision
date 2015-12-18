#version 330

in vec3 position;
in vec3 texcoord;

uniform mat4 mvp;

void main()
{
	gl_Position = mvp * vec4(position, 1.0f);
}
