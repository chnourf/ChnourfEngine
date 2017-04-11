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

uniform samplerCube skybox;

void main()
{
    vec3 I = normalize(fragPos - viewPos);
    vec3 R = reflect(I, normalize(normal));
    out_color = texture(skybox, R);
}