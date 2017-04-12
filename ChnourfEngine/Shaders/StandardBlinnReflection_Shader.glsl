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

uniform samplerCube skybox;

void main()
{
    vec3 I = normalize(fs_in.fragPos - viewPos);
    vec3 R = reflect(I, normalize(fs_in.normal));
    out_color = texture(skybox, R);
}