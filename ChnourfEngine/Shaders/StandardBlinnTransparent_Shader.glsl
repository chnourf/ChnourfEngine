//fragment shader
#version 450 core
 
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
  
uniform Material material;
 
uniform vec3 viewPos;
uniform vec3 lightDirection;
uniform vec3 lightColor;
//uniform sampler2D ourTexture1;
//uniform sampler2D ourTexture2;

void main()
{
	// Ambient
	vec3 ambient = lightColor * material.ambient;

	// Diffuse 
	vec3 norm = normalize(fs_in.normal);
	vec3 lightDir = normalize(-lightDirection);//normalize(pointLightPosition - fs_in.fragPos);
	float diff = max(abs(dot(norm, lightDir)), 0.0);
	vec4 diffuseTextColor = texture(material.diffuse, fs_in.texcoord);
	if (diffuseTextColor.a < 0.05)
	{
		discard;
	}

	vec3 diffuse = lightColor * (diff * vec3(texture(material.diffuse, fs_in.texcoord)));
	
	// Specular
	vec3 viewDir = normalize(viewPos - fs_in.fragPos);

	vec3 halfwayDir = normalize(lightDir + viewDir);

	float spec = pow(max(dot(fs_in.normal, halfwayDir), 0.0), material.shininess);
	vec3 specular = lightColor * (spec * vec3(texture(material.specular, fs_in.texcoord)));  

	vec3 result = ambient + diffuse + specular;
	out_color = vec4(fs_in.texcoord.x, fs_in.texcoord.y, 0.0f, 1.0f);
}