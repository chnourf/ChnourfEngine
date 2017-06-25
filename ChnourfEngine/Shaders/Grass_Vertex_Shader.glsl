#version 450 core //lower this version if your card does not support GLSL 4.5
layout (location = 0) in float in_elevation;
layout (location = 1) in int in_normal;

layout (std140) uniform ViewConstants
{
	mat4 projection;
	mat4 view;
};

out VS_OUT
{
	vec3 position;
	vec3 normal;
} vs_out;

uniform int cellSize;
uniform float resolution;
uniform ivec2 tileIndex;


void main()
{
	// same calc done in terrain shader, not opti !
	float x = ((gl_VertexID % cellSize) * 1/(float(cellSize) - 1) + tileIndex.x) * cellSize * resolution;
	float z = ((gl_VertexID / cellSize) * 1/(float(cellSize) - 1) + tileIndex.y) * cellSize * resolution;

	vec3 position = vec3(x, in_elevation, z);
	
	int normX = ((in_normal >> 16) & 0xff) - 128;
	int normY = ((in_normal >> 8) & 0xff) - 128;
	int normZ = ((in_normal) & 0xff) - 128;
	
	vs_out.normal = normalize(vec3(normX, normY, normZ));

  gl_Position = projection*view*vec4(position, 1.0);
  vs_out.position = position;
}