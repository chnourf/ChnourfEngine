#version 330 core
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;

out VS_OUT
{
	vec2 texcoord;
	vec3 normal;
	vec3 fragPos;
	vec4 fragPosLightSpace;
} vs_out;

layout (std140) uniform ViewConstants
{
	mat4 projection;
	mat4 view;
};

uniform mat4 cubemapView;
uniform mat4 lightSpaceMatrix;

void main()
{
    gl_Position = projection * view * vec4(in_position, 1.0);  
    vs_out.texcoord = in_position.xz;
	vs_out.normal = in_normal;
	vs_out.fragPos = in_position;
	vs_out.fragPosLightSpace = lightSpaceMatrix * vec4(in_position, 1.0);
}  