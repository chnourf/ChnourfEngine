#version 330 core

layout (location = 0) in float in_elevation;

layout (std140) uniform ViewConstants
{
	mat4 projection;
	mat4 view;
};

uniform int tileSize;
uniform float resolution;
uniform ivec2 tileIndex;

void main()
{
	float x = ((gl_VertexID % tileSize) * 1/(float(tileSize) - 1) + tileIndex.x) * tileSize * resolution;
	float z = ((gl_VertexID / tileSize) * 1/(float(tileSize) - 1) + tileIndex.y) * tileSize * resolution;

	vec3 position = vec3(x, in_elevation, z);
    gl_Position = projection * view * vec4(position, 1.0); 
}  