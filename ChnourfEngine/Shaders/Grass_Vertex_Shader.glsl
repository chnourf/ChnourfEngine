#version 450 core //lower this version if your card does not support GLSL 4.5
layout (location = 0) in vec2 in_quad_position;
layout (location = 1) in vec3 in_position;
layout (location = 2) in int in_normal;
layout (location = 3) in int misc;

layout (std140) uniform ViewConstants
{
	mat4 projection;
	mat4 view;
};

out VS_OUT
{
	vec2 texcoord;
	vec3 normal;
	vec3 fragPos;
} vs_out;

void main()
{
	int normX = ((in_normal >> 16) & 0xff) - 128;
	int normY = ((in_normal >> 8) & 0xff) - 128;
	int normZ = ((in_normal) & 0xff) - 128;
	
	vs_out.normal = normalize(vec3(normX, normY, normZ));

	vec3 position = in_position + vec3(in_quad_position, 0.f);
	
	gl_Position = projection*view*vec4(position, 1.0);
	vs_out.fragPos = position;
}