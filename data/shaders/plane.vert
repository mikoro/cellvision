#version 330

in vec3 position;
in vec3 texcoord;

out vec3 worldPositionVarying;

uniform mat4 model;
uniform mat4 mvp;

void main()
{
	worldPositionVarying = (model * vec4(position, 1.0f)).xyz;
	gl_Position = mvp * vec4(position, 1.0f);
}