#version 330

in vec2 position;
in vec2 texcoord;

out vec3 worldPositionVarying;

uniform mat4 objectToWorld;

void main()
{
	vec2 tempPosition1 = position / 2.0f + vec2(0.5f, 0.5f);
	vec4 tempPosition2 = vec4(tempPosition1, 0.0f, 1.0f);
	tempPosition2 = objectToWorld * tempPosition2;
	worldPositionVarying = tempPosition2.xyz;
	
	gl_Position = vec4(position, 0.0f, 1.0f);
}
