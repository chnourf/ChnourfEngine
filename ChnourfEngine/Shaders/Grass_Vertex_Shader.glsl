#version 450 core //lower this version if your card does not support GLSL 4.5
layout(location = 0) in vec3 in_position;

layout (std140) uniform ViewConstants
{
	mat4 projection;
	mat4 view;
};

void main()
{
  gl_Position = projection*view*vec4(in_position, 1.0);
}