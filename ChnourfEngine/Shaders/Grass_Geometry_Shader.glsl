#version 450 core //lower this version if your card does not support GLSL 4.5
layout (points) in;
layout (triangle_strip, max_vertices = 5) out;

out GS_OUT
{
	vec2 texcoord;
	vec3 normal;
	vec3 fragPos;
} gs_out;

in VS_OUT
{
	vec3 position;
} vs_in[];

layout (std140) uniform ViewConstants
{
	mat4 projection;
	mat4 view;
};

void main()
{
	gs_out.texcoord = vec2(0.0);
	gs_out.normal = vec3(0.0,1.0,0.0);
	gs_out.fragPos = vs_in[0].position;

    gl_Position = gl_in[0].gl_Position + projection*view*vec4(-1., 0.0, 0.0, 0.0); 
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + projection*view*vec4( 1., 0.0, 0.0, 0.0);
    EmitVertex();
	
	gl_Position = gl_in[0].gl_Position + projection*view*vec4( 0.0, 2., 0.0, 0.0);
    EmitVertex();
	
	EndPrimitive();
		
	gl_Position = gl_in[0].gl_Position + projection*view*vec4(0.0, 0.0, -1.0, 0.0); 
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + projection*view*vec4(0.0 , 0.0, 1.0, 0.0);
    EmitVertex();
	
	gl_Position = gl_in[0].gl_Position + projection*view*vec4( 0.0, 2., 0.0, 0.0);
    EmitVertex();
	
	EndPrimitive();
}