//fragment shader
//#version 450 core
 
out vec4 out_color;
 
in VS_OUT
{
	vec2 texcoord;
	vec3 normal;
	vec3 fragPos;
} fs_in;


struct Material {
    vec3 ambient;
	sampler2D  diffuse;
    sampler2D  specular;
    float shininess;
}; 
  
uniform Material grassMaterial;

void main()
{
    // Ambient
    vec3 ambient = lightColor * grassMaterial.ambient;
 	
    // Diffuse 
    vec3 norm = normalize(fs_in.normal);
    vec3 lightDir = normalize(-lightDirection);
    float diff = max((dot(norm, lightDir)), 0.0);
	vec4 diffuseTextColor = texture(grassMaterial.diffuse, fs_in.texcoord);
	if (diffuseTextColor.a <=  (0.05 + 0.95 * length(fs_in.fragPos - viewPos) / 200))
	{
		discard;
	}

	// test
	vec3 amb = clamp(0.5+0.5*fs_in.normal.y,0.0,1.0)*vec3(0.40,0.60,0.80)*.3;
	vec3 bac = clamp( 0.2 + 0.8*dot( normalize( vec3(-lightDir.x, 0.0, lightDir.z ) ), fs_in.normal ), 0.0, 1.0 )*vec3(0.40,0.50,0.60)*0.3;

    vec3 diffuse = (lightColor * diff + amb + bac) * vec3(texture(grassMaterial.diffuse, fs_in.texcoord));
    
    // Specular
    vec3 viewDir = normalize(viewPos - fs_in.fragPos);
 
	vec3 halfwayDir = normalize(lightDir + viewDir);
 
    float spec = pow(max(dot(fs_in.normal, halfwayDir), 0.0), 4.0);
    vec3 specular = lightColor * (spec * vec3(texture(grassMaterial.specular, fs_in.texcoord)));  
        
    vec3 result = diffuse;

    result = Fog( result, fs_in.fragPos, viewDir, lightDir );

    out_color = vec4(result, 1.0);

}