#version 330 core

uniform sampler2D noise;
uniform sampler2D shadowMap;

const mat2 m2 = mat2(0.8, -0.6, 0.6, 0.8);

float fbm(vec2 p)
{
	float f = 0.0;
	f += 0.5000*texture(noise, p / 256.0).x; p = m2*p*2.02;
	f += 0.2500*texture(noise, p / 256.0).x; p = m2*p*2.03;
	f += 0.1250*texture(noise, p / 256.0).x; p = m2*p*2.01;
	f += 0.0625*texture(noise, p / 256.0).x;
	return f / 0.9375;
}


float Shadow(vec4 fragPosLightSpace)
{
	// perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	// Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(shadowMap, projCoords.xy).r;
	// Get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;

	currentDepth = clamp(currentDepth, 0.0, 1.0);

	// Check whether current frag pos is in shadow
	float bias = 0.001;//max(0.05 * (1.0 - dot(fs_in.normal, normalize(lightDirection))), 0.005);
	float shadow = (currentDepth - bias) > closestDepth ? 0.1 : 1.0;

	return shadow;
}