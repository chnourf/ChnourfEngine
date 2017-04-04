//fragment shader
#version 450 core
 
out vec4 out_color;
 
in vec2 texcoord;
in vec3 normal;
in vec3 fragPos;

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
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(-lightDirection);//normalize(pointLightPosition - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * (diff * vec3(texture(material.diffuse, texcoord)));
    
    // Specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = lightColor * (spec * vec3(texture(material.specular, texcoord)));  
        
    vec3 result = ambient + diffuse + specular;
    out_color = vec4(result, 1.0f);

    //out_color = mix(texture(ourTexture1, texcoord),texture(ourTexture2, texcoord),0.5);
}