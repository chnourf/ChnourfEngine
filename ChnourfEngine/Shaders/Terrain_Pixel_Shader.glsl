#version 330 core

in VS_OUT
{
	vec2 texcoord;
	vec3 normal;
	vec3 fragPos;
	vec4 fragPosLightSpace;
} fs_in;

struct Material {
    vec3 ambient;
	sampler2D  diffuse;
    sampler2D  specular;
    float shininess;
}; 

out vec4 out_color;

uniform vec3 viewPos;
uniform vec3 lightDirection;
uniform vec3 lightColor;

uniform Material groundMaterial;
uniform Material rockMaterial;
uniform sampler2D shadowMap;
uniform sampler2D normalMap;

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

	currentDepth = clamp(currentDepth,0.0,1.0);

    // Check whether current frag pos is in shadow
	float bias = 0.01;//max(0.05 * (1.0 - dot(fs_in.normal, normalize(lightDirection))), 0.005);
    float shadow = (currentDepth - bias) > closestDepth  ? 0.1 : 1.0;

    return shadow;
}

void main()
{    
	// ambient
	vec3 ambient = vec3(0.1, 0.1, 0.1);

	// normal
	vec3 norm = texture(normalMap, fs_in.texcoord).rgb;
	norm = normalize(norm * 2.0 - 1.0); 
	
	vec3 T = vec3(1,0,0);
	vec3 B = normalize(cross(fs_in.normal, T));
	  
	norm = normalize((mat3(T,B,fs_in.normal)) * norm); 

	// difffuse
    vec3 lightDir = normalize(-lightDirection);
    float diff = max(dot(norm, lightDir), 0.0);

	vec4 textureColor = texture(groundMaterial.diffuse, fs_in.texcoord);
	if (norm.y < 0.7)
	{
	   textureColor = texture(rockMaterial.diffuse, fs_in.texcoord);
	}
    vec3 diffuse = diff*vec3(textureColor)* lightColor;

	// Specular
    vec3 viewDir = normalize(viewPos - fs_in.fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
  
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
    vec3 specular = lightColor * spec * vec3(1.0,1.0,1.0);  

    vec3 result =  Shadow(fs_in.fragPosLightSpace) * (diffuse + 0.3 * specular) + ambient;

	//fog
	float fogDensity = 0.002;
	float fogAmount = (1.0 - exp( -distance(viewPos, fs_in.fragPos)*fogDensity))*clamp(exp(-fs_in.fragPos.y * fogDensity * 0.1), 0.0, 1.0);;
	float sunAmount = max( dot( -viewDir,lightDir ), 0.0 );
    vec3  fogColor  = mix(vec3(0.5 ,0.6, 0.7), lightColor, pow(sunAmount,2.0));
    result = mix( result, fogColor, fogAmount );

    out_color = vec4(result, 1.0f);
}
  