#version 450 core //lower this version if your card does not support GLSL 4.5
layout (location = 0) in vec3 position;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    //gl_Position = lightSpaceMatrix * model * vec4(position, 1.0f);
    gl_Position = lightSpaceMatrix * vec4(position, 1.0f);
}  