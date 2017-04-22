#version 330 core

in VS_OUT
{
	vec2 texcoord;
	vec3 normal;
	vec3 fragPos;
} fs_in;


out vec4 out_color;

uniform vec3 viewPos;
uniform vec3 lightDirection;
uniform vec3 lightColor;

uniform sampler2D groundTexture;
uniform sampler2D rockTexture;

void main()
{    

	// ambient
	vec3 ambient = vec3(0.1, 0.1, 0.1);

	// difffuse
	vec3 norm = normalize(fs_in.normal);
    vec3 lightDir = normalize(-lightDirection);
    float diff = max(dot(norm, lightDir), 0.0);

	vec4 textureColor = texture(groundTexture, fs_in.texcoord);
	if (norm.y < 0.01)
	{
	   textureColor = texture(rockTexture, fs_in.texcoord);
	}

    vec3 diffuse = diff*vec3(textureColor)* lightColor;

	// Specular
    vec3 viewDir = normalize(viewPos - fs_in.fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
  
    float spec = pow(max(dot(fs_in.normal, halfwayDir), 0.0), 16.0);
    vec3 specular = lightColor * spec * vec3(1.0,1.0,1.0);  

    vec3 result = ambient + diffuse + specular;
    out_color = vec4(result, 1.0f);
}
  