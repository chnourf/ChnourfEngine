#version 450 core //lower this version if your card does not support GLSL 4.5
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;
 
out vec2 texcoord;
out vec3 normal;
out vec3 fragPos;
 
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
  texcoord = in_texcoord;
  gl_Position = projection * view * model * vec4(in_position, 1.0f);
  fragPos = vec3(model * vec4(in_position, 1.0f));
  normal = mat3(transpose(inverse(model))) * in_normal;
}