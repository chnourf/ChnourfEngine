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
uniform Material snowMaterial;
uniform sampler2D shadowMap;
uniform sampler2D normalMap;
uniform sampler2D noise;


const mat2 m2 = mat2(0.8,-0.6,0.6,0.8);

float fbm( vec2 p )
{
    float f = 0.0;
    f += 0.5000*texture( noise, p/256.0 ).x; p = m2*p*2.02;
    f += 0.2500*texture( noise, p/256.0 ).x; p = m2*p*2.03;
    f += 0.1250*texture( noise, p/256.0 ).x; p = m2*p*2.01;
    f += 0.0625*texture( noise, p/256.0 ).x;
    return f/0.9375;
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

	currentDepth = clamp(currentDepth,0.0,1.0);

    // Check whether current frag pos is in shadow
	float bias = 0.001;//max(0.05 * (1.0 - dot(fs_in.normal, normalize(lightDirection))), 0.005);
    float shadow = (currentDepth - bias) > closestDepth  ? 0.1 : 1.0;

    return shadow;
}

void main()
{    
	vec3 ambient = vec3(0.0,0.0,0.0);

	// normal
	vec3 norm = texture(normalMap, fs_in.texcoord).rgb;
	norm = normalize(norm * 2.0 - 1.0); 
	vec3 T = vec3(1,0,0);
	vec3 B = normalize(cross(fs_in.normal, T));
	norm = normalize((mat3(T,B,fs_in.normal)) * norm); 
	
	// difffuse
    vec3 lightDir = normalize(-lightDirection);
    float diff = max(dot(norm, lightDir), 0.0);
	
	// test
	vec3 amb = clamp(0.5+0.5*norm.y,0.0,1.0)*vec3(0.40,0.60,0.80)*.3;
	vec3 bac = clamp( 0.2 + 0.8*dot( normalize( vec3(-lightDir.x, 0.0, lightDir.z ) ), norm ), 0.0, 1.0 )*vec3(0.40,0.50,0.60)*0.3;

	vec4 textureColor = texture(groundMaterial.diffuse, fs_in.texcoord);
	
	//rocks
	//textureColor = mix(texture(rockMaterial.diffuse, fs_in.texcoord), textureColor, smoothstep(0.6,0.8,norm.y));
	
	
	// test
	float r = texture( noise, (7.0/250)*fs_in.fragPos.xz/256.0 ).x;
	vec3 iColor = (r*0.25+0.75)*mix( vec3(0.08,0.05,0.03), vec3(0.10,0.09,0.08), texture(noise,0.00007*vec2(fs_in.fragPos.x,fs_in.fragPos.y*48.0)/250).x );
	iColor = mix( iColor, 0.80*vec3(0.45,.30,0.15)*(0.50+0.50*r),smoothstep(0.70,0.9,norm.y) );
    iColor = mix( iColor, 0.65*vec3(0.30,.30,0.10)*(0.25+0.75*r),smoothstep(0.95,1.0,norm.y) );
	textureColor = vec4(1.2*iColor,1.0);
	
	//snow
	float h = smoothstep(55.0,80.0,fs_in.fragPos.y + 25.0*fbm(0.01*fs_in.fragPos.xz/250) );
	float e = smoothstep(1.0-0.5*h,1.0-0.1*h,norm.y);
	float o = 0.3 + 0.7*smoothstep(0.0,0.1,norm.x+h*h);
	textureColor = mix(textureColor, texture(snowMaterial.diffuse, fs_in.texcoord), e*h*o);
    vec3 diffuse = (diff * lightColor + amb + bac) * vec3(textureColor);

	// Specular
    vec3 viewDir = normalize(viewPos - fs_in.fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
  
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 4.0);
    vec3 specular = lightColor * spec * vec3(1.0,1.0,1.0);  
	
    vec3 result =  Shadow(fs_in.fragPosLightSpace) * (diffuse + 0.1 * specular) + ambient;
	
	//fog
	float fogDensity = 0.001;
	float fogAmount = (1.0 - exp( -distance(viewPos, fs_in.fragPos)*fogDensity))*clamp(exp(-fs_in.fragPos.y * fogDensity * 1), 0.0, 1.0);;
	float sunAmount = max( dot( -viewDir,lightDir ), 0.0 )* max( dot( -viewDir,lightDir ), 0.0 );
    vec3  fogColor  = mix(vec3(0.5 ,0.6, 0.7), lightColor, pow(sunAmount,2.0) * 0.5);
    result = mix( result, fogColor, fogAmount );


    out_color = vec4(result, 1.0f);
}
  