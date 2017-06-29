#version 450 core //lower this version if your card does not support GLSL 4.5
layout (location = 0) in vec2 in_quad_position;
layout (location = 1) in vec3 in_position;
layout (location = 2) in ivec4 in_normal;
layout (location = 3) in ivec4 misc;

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

uniform float elapsedTime;

void main()
{	
	vs_out.normal = normalize(vec3(in_normal.x - 128, in_normal.y - 128, in_normal.z - 128));

	float direction = misc.g * 3.141592 / 128;
	vec3 offset = vec3((in_quad_position.x - 0.5) * cos(direction), in_quad_position.y, (in_quad_position.x - 0.5) * sin(direction));

	float param = in_position.x + in_position.z;
	float Arg = elapsedTime * 0.001 + param * 0.25 + fract(param);
    vec3 Wind = vec3(1.0, 0.0, 1.0);
	float oscillation = Wind.z + 0.2 * sin(Wind.z * 6 * Arg) * (1.5 - Wind.z);
	oscillation *= in_quad_position.y + 0.5;
	vec3 vWaveDistortion = (vec3(Wind.x, -0.17, Wind.y) * Wind.z + (fract(in_position.x) - fract(in_position.y)) * vec3(0.05, 0.0, 0.05)) * oscillation;
		
	offset += vWaveDistortion;
	//offset *= (misc.z)/255;
	
	vec3 position = in_position + offset;
	
	gl_Position = projection*view*vec4(position, 1.0);
	vs_out.fragPos = position;
	
	vs_out.texcoord = -in_quad_position + vec2(0.5, 0.0);
}