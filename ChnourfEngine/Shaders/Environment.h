#version 330 core

uniform sampler2D noise;
uniform sampler2D shadowMap;

uniform vec3 viewPos;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform mat4 lightSpaceMatrix;

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

float kernel[9] = float[](
	1.0 / 16, 2.0 / 16, 1.0 / 16,
	2.0 / 16, 4.0 / 16, 2.0 / 16,
	1.0 / 16, 2.0 / 16, 1.0 / 16
	);

float Shadow(vec3 aPosition, vec3 aNormal)
{
	vec4 fragPosLightSpace = lightSpaceMatrix * vec4(aPosition, 1.0);
	// perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	// Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float shadow = 0.f;

	// Get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;
	currentDepth = clamp(currentDepth, 0.0, 1.0);
	float bias = max(0.005 * (1.0 - dot(aNormal, normalize(lightDirection))), 0.0005);

	for (int i = 0; i < 9; ++i)
	{
		//gaussian blur

		ivec2 offset = ivec2(-1 + i / 3, -1 + i % 3);
		float closestDepth = texture(shadowMap, projCoords.xy + 0.001f * vec2(offset)).r;

		// Check whether current frag pos is in shadow
		float isInShadow = (currentDepth - bias) > closestDepth ? 0.1f : 1.0f;
		shadow += kernel[i] * isInShadow;
	}

	return shadow;
}

vec3 Fog(vec3 aColor, vec3 aPos, vec3 viewDir, vec3 lightDir)
{
	float fogDensity = 0.001;
	float fogAmount = (1.0 - exp(-distance(viewPos, aPos)*fogDensity)) *clamp(exp(-(aPos.y - 100) * fogDensity), 0.0, 1.0);
	//float fogAmount = clamp(pow(distance(viewPos, aPos) / 1500.0, 0.5), 0.0, 1.0) * clamp(exp(-(aPos.y - 100) * fogDensity), 0.0, 1.0);
	float sunAmount = max(dot(-viewDir, lightDir), 0.0)* max(dot(-viewDir, lightDir), 0.0);
	// length(lightColor) is costly
	vec3  fogColor = mix(vec3(0.5, 0.6, 0.7) * (0.3 + 0.1*length(lightColor)), lightColor, sunAmount * sunAmount * 0.5);
	return mix(aColor, fogColor, fogAmount);
}