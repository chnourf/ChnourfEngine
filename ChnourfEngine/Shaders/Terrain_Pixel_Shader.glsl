#version 330 core
in vec3 TexCoords;
in float height;
out vec4 color;

uniform samplerCube cubemap;

void main()
{    
    color = vec4(height, height, height,1);
}
  