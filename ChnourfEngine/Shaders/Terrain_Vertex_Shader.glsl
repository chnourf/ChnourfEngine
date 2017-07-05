#version 330 core

layout (location = 0) in float in_elevation;
layout (location = 1) in int in_normal;

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

uniform mat4 lightSpaceMatrix;
uniform int cellSize;
uniform float resolution;
uniform ivec2 tileIndex;

void main()
{
	float x = ((gl_VertexID % cellSize) * 1/(float(cellSize) - 1) + tileIndex.x) * cellSize * resolution;
	float z = ((gl_VertexID / cellSize) * 1/(float(cellSize) - 1) + tileIndex.y) * cellSize * resolution;

	vec3 position = vec3(x, in_elevation, z);
    gl_Position = projection * view * vec4(position, 1.0); 
	
    vs_out.texcoord = position.xz;
	
	int normX = ((in_normal >> 16) & 0xff) - 128;
	int normY = ((in_normal >> 8) & 0xff) - 128;
	int normZ = ((in_normal) & 0xff) - 128;
	
	vs_out.normal = normalize(vec3(normX, normY, normZ));
	vs_out.fragPos = position;
	vs_out.fragPosLightSpace = lightSpaceMatrix * vec4(position, 1.0);
}  