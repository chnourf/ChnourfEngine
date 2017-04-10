#version 330 core
layout (location = 0) in vec3 in_position;
out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;


void main()
{
    vec4 pos =   projection * view * vec4(in_position, 1.0);  
	gl_Position = pos.xyww;
    TexCoords = in_position;
}  