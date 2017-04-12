#version 450 core //lower this version if your card does not support GLSL 4.5
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;
 
out VS_OUT
{
	vec2 texcoord;
	vec3 normal;
	vec3 fragPos;
} vs_out;


layout (std140) uniform ViewConstants
{
	mat4 projection;
	mat4 view;
};

uniform	mat4 model;

void main()
{
  vs_out.texcoord = in_texcoord;
  gl_Position = projection * view * model * vec4(in_position, 1.0f);
  vs_out.fragPos = vec3(model * vec4(in_position, 1.0f));
  vs_out.normal = mat3(transpose(inverse(model))) * in_normal;
}