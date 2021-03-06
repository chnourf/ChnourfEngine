//include Environment.h

in VS_OUT
{
	vec2 rainfallAndTemperature;
	vec2 texcoord;
	vec4 fragPosLightSpace;
	vec3 fragPos;
	float erosion;
	vec3 normal;
} fs_in;

struct Material {
	vec3 ambient;
	sampler2D  diffuse;
	sampler2D  specular;
	float shininess;
}; 

out vec4 out_color;

uniform Material groundMaterial;
uniform Material rockMaterial;
uniform Material snowMaterial;
uniform sampler2D normalMap;
uniform sampler2D grassColor;

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
	vec3 normTest = fs_in.normal + vec3(0.2*fbm(fs_in.texcoord/2.0));
	float r = texture( noise, (7.0/250)*fs_in.fragPos.xz/256.0 ).x;
	vec3 iColor = (r*0.25+0.75)*mix( vec3(0.08,0.05,0.03), vec3(0.10,0.09,0.08), texture(noise,0.00007*vec2(fs_in.fragPos.x,fs_in.fragPos.y*48.0)/250).x );
	//iColor = mix( iColor, 0.80*vec3(0.45,.30,0.15)*(0.50+0.50*r),smoothstep(0.70,0.9,normTest.y) );
	iColor = mix( iColor, 0.65*texture(grassColor, vec2(fs_in.rainfallAndTemperature)).rgb*(0.25+0.75*r),smoothstep(0.95,1.0,normTest.y) );
	textureColor = vec4(1.2*iColor,1.0);
	//textureColor = texture(grassColor, vec2(fs_in.rainfallAndTemperature));
	
	//snow
	float h = smoothstep(450.0,600.0,fs_in.fragPos.y + 100.0*fbm(fs_in.fragPos.xz/250) );
	float e = smoothstep(1.0-0.5*h,1.0-0.1*h,normTest.y);
	float o = 0.3 + 0.7*smoothstep(0.2,0.3,normTest.x+h*h);
	textureColor = mix(textureColor, texture(snowMaterial.diffuse, fs_in.texcoord), smoothstep(0.4f, 0.6f, e*h*o));
	if (fs_in.fragPos.y < 0.f)
	{
		textureColor = vec4(0, 0.7, 1, 1);
	}
	textureColor.rgb = mix(textureColor.rgb, vec3(0.45,.30,0.15), smoothstep(0.f, 1.f, fs_in.erosion));
	vec3 diffuse = (diff * lightColor + amb + bac) * vec3(textureColor);

	// Specular
	vec3 viewDir = normalize(viewPos - fs_in.fragPos);
	//vec3 halfwayDir = normalize(lightDir + viewDir);
  	//
	//float spec = pow(max(dot(norm, halfwayDir), 0.0), 4.0);
	//vec3 specular = lightColor * spec * vec3(1.0,1.0,1.0);  
	
	vec3 result =  Shadow(fs_in.fragPos, norm) * (diffuse) + ambient;
	
	//fog
	result = Fog( result, fs_in.fragPos, viewDir, lightDir );

	out_color = vec4(result, 1.0f);
}
  