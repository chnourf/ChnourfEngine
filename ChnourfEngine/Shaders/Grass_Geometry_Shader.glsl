#version 450 core //lower this version if your card does not support GLSL 4.5
layout (points) in;
layout (triangle_strip, max_vertices = 5) out;

out GS_OUT
{
	vec2 texcoord;
	vec3 normal;
	vec3 fragPos;
} gs_out;

layout (std140) uniform ViewConstants
{
	mat4 projection;
	mat4 view;
};

void main()
{
    gl_Position = gl_in[0].gl_Position + vec4(-0.5, 0.0, 0.0, 0.0); 
    EmitVertex();

    gl_Position = projection*view*gl_in[0].gl_Position + vec4( 0.5, 0.0, 0.0, 0.0);
    EmitVertex();
	
	gl_Position = projection*view*gl_in[0].gl_Position + vec4( 0.0, 0.5, 0.0, 0.0);
    EmitVertex();
	
	EndPrimitive();
	
	gs_out.texcoord = vec2(0.0);
	gs_out.normal = vec3(0.0,1.0,0.0);
}