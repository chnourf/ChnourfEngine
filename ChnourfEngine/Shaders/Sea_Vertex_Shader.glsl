#version 330 core
layout(location = 0) in vec3 in_position;

out VS_OUT
{
	vec3 fragPos;
} vs_out;

layout (std140) uniform ViewConstants
{
	mat4 projection;
	mat4 view;
};

void main()
{
	gl_Position = projection * view * vec4(in_position, 1.0);
	vs_out.fragPos = in_position;
}
