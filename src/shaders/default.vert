#version 330 core

uniform float u_Time;

in vec3 in_position;
out vec3 posColor;
void main()
{
	gl_Position = vec4(in_position,1.0f);
	posColor = in_position;
}