#version 330 core

in VS_OUT
{
	vec2 texcoord;
	vec3 normal;
	vec3 fragPos;
	vec4 fragPosLightSpace;
} fs_in;


out vec4 out_color;

uniform vec3 viewPos;
uniform vec3 lightDirection;
uniform vec3 lightColor;

uniform sampler2D groundTexture;
uniform sampler2D rockTexture;
uniform sampler2D shadowMap;

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
    // Check whether current frag pos is in shadow
	float bias = max(0.05 * (1.0 - dot(fs_in.normal, normalize(lightDirection))), 0.005);
    float shadow = (currentDepth - bias) > closestDepth  ? 1.0 : 0.0;

    return 1;//shadow;
}

void main()
{    

	// ambient
	vec3 ambient = vec3(0.1, 0.1, 0.1);

	// difffuse
	vec3 norm = normalize(fs_in.normal);
    vec3 lightDir = normalize(-lightDirection);
    float diff = max(dot(norm, lightDir), 0.0);

	vec4 textureColor = texture(groundTexture, fs_in.texcoord);
	if (norm.y < 0.7)
	{
	   textureColor = texture(rockTexture, fs_in.texcoord);
	}

    vec3 diffuse = diff*vec3(textureColor)* lightColor;

	// Specular
    vec3 viewDir = normalize(viewPos - fs_in.fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
  
    float spec = pow(max(dot(fs_in.normal, halfwayDir), 0.0), 64.0);
    vec3 specular = lightColor * spec * vec3(1.0,1.0,1.0);  

    vec3 result = Shadow(fs_in.fragPosLightSpace) * (diffuse + 0.3 * specular) + ambient;
    out_color = vec4(result, 1.0f);
}
  