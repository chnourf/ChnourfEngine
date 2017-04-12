#version 330 core
layout (location = 0) in vec3 in_position;
out vec3 TexCoords;

layout (std140) uniform ViewConstants
{
	mat4 projection;
	//useless
	mat4 view;
};

uniform mat4 cubemapView;

void main()
{
    vec4 pos =   projection * cubemapView * vec4(in_position, 1.0);  
	gl_Position = pos.xyww;
    TexCoords = in_position;
}  