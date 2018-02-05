#version 450 core //lower this version if your card does not support GLSL 4.5
layout (location = 0) in vec2 in_quad_position;
layout (location = 1) in vec3 in_position;
layout (location = 2) in ivec4 in_normal;
layout (location = 3) in ivec4 misc;

uniform sampler2D grassColorTexture;

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
	vec3 colorModifier;
} vs_out;

uniform float elapsedTime;
uniform mat4 lightSpaceMatrix;

void main()
{	
	vs_out.normal = normalize(vec3(in_normal.x - 128, in_normal.y - 128, in_normal.z - 128));

	float direction = misc.x / 128.0 * 3.141592;
	vec3 offset = vec3((in_quad_position.x - 0.5) * cos(direction), in_quad_position.y * vs_out.normal.y, (in_quad_position.x - 0.5) * sin(direction));

	float param = in_position.x + in_position.z;
	float Arg = elapsedTime + param * 0.25 + fract(param);
    vec3 Wind = vec3(.3, 0.0, .3);
	float oscillation = Wind.z + 0.2 * sin(Wind.z * 6 * Arg) * (1.5 - Wind.z);
	oscillation *= in_quad_position.y + 0.5;
	vec3 vWaveDistortion = (vec3(Wind.x, -0.17, Wind.y) * Wind.z + (fract(in_position.x) - fract(in_position.y)) * vec3(0.05, 0.0, 0.05)) * oscillation;
		
	offset += vWaveDistortion;
	
	vec3 position = in_position + offset;
	
	gl_Position = projection*view*vec4(position, 1.0);
	vs_out.fragPos = position;
	
	vs_out.texcoord = - in_quad_position + vec2(0.5, 0.0);

	vec2 rainfallAndTemperature = vec2(misc.z/255.0, misc.w/255.0);

	vs_out.colorModifier = textureLod(grassColorTexture, rainfallAndTemperature, 0.0).rgb;
}