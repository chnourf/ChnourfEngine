#version 330 core

layout (location = 0) in float in_elevation;
layout (location = 1) in int in_normal;
layout (location = 2) in int in_rainfallTemperatureErosion;

out VS_OUT
{
	vec2 rainfallAndTemperature;
	vec2 texcoord;
	vec4 fragPosLightSpace;
	vec3 fragPos;
	float erosion;
	vec3 normal;
} vs_out;

layout (std140) uniform ViewConstants
{
	mat4 projection;
	mat4 view;
};

uniform mat4 lightSpaceMatrix;
uniform int tileSize;
uniform float resolution;
uniform ivec2 tileIndex;

void main()
{
	float x = ((gl_VertexID % tileSize) * 1/(float(tileSize) - 1) + tileIndex.x) * tileSize * resolution;
	float z = ((gl_VertexID / tileSize) * 1/(float(tileSize) - 1) + tileIndex.y) * tileSize * resolution;

	vec3 position = vec3(x, in_elevation, z);
    gl_Position = projection * view * vec4(position, 1.0); 
	
    vs_out.texcoord = position.xz;
	
	int normX = ((in_normal) & 0xff) - 128;
	int normY = ((in_normal >> 8) & 0xff) - 128;
	int normZ = ((in_normal >> 16) & 0xff) - 128;
	
	vs_out.normal = normalize(vec3(normX, normY, normZ));
	vs_out.fragPos = position;
	vs_out.fragPosLightSpace = lightSpaceMatrix * vec4(position, 1.0);

	vs_out.rainfallAndTemperature = vec2(((in_rainfallTemperatureErosion >> 8) & 0xff)/255.0, (in_rainfallTemperatureErosion & 0xff)/255.0) ;
	vs_out.erosion = ((in_rainfallTemperatureErosion >> 16) & 0xff)/255.0;
}  