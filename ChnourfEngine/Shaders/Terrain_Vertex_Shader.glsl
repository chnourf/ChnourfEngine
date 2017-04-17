#version 330 core
layout (location = 0) in vec3 in_position;
out vec3 TexCoords;
out float height;

layout (std140) uniform ViewConstants
{
	mat4 projection;
	//useless
	mat4 view;
};

uniform mat4 cubemapView;

void main()
{
    vec4 pos =   projection * view * vec4(in_position, 1.0);  
	height = in_position.y/20;
	gl_Position = pos;
    TexCoords = in_position;
}  