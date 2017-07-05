#version 330 core

layout (location = 0) in float in_elevation;

layout (std140) uniform ViewConstants
{
	mat4 projection;
	mat4 view;
};

uniform int cellSize;
uniform float resolution;
uniform ivec2 tileIndex;

void main()
{
	float x = ((gl_VertexID % cellSize) * 1/(float(cellSize) - 1) + tileIndex.x) * cellSize * resolution;
	float z = ((gl_VertexID / cellSize) * 1/(float(cellSize) - 1) + tileIndex.y) * cellSize * resolution;

	vec3 position = vec3(x, in_elevation, z);
    gl_Position = projection * view * vec4(position, 1.0); 
}  