#version 450 core //lower this version if your card does not support GLSL 4.5
layout (points) in;
layout (triangle_strip, max_vertices = 10) out;

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

uniform float elapsedTime;

uniform sampler2D noise;

const mat2 m2 = mat2(0.8,-0.6,0.6,0.8);

float fbm( vec2 p )
{
    float f = 0.0;
    f += 0.5000*texture( noise, p/256.0 ).x; p = m2*p*2.02;
    f += 0.2500*texture( noise, p/256.0 ).x; p = m2*p*2.03;
    f += 0.1250*texture( noise, p/256.0 ).x; p = m2*p*2.01;
    f += 0.0625*texture( noise, p/256.0 ).x;
    return f/0.9375;
}

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void main()
{
	gs_out.fragPos = vs_in[0].position;

	
	
	float grassHeight = fbm(vec2(vs_in[0].position.x*10, vs_in[0].position.z*10));
	float modif = cos(elapsedTime/300.0 + grassHeight*10)*0.1*grassHeight;
	mat4 rot = rotationMatrix(vec3(0.0, 1.0, 0.0), 3.0*fbm(vec2(vs_in[0].position.x*100, vs_in[0].position.z*100)));

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4(-0.25, 0.0, 0.0, 0.0);
	gs_out.normal = vec3(0.0,0.0,1.0);
	gs_out.texcoord = vec2(.99, .99);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4( 0.25, 0.0, 0.0, 0.0);
	gs_out.normal = vec3(0.0,0.0,1.0);
	gs_out.texcoord = vec2(0.01, .99);
	EmitVertex();
	
	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4( 0.25 + modif, grassHeight, 0.0, 0.0);
	gs_out.normal = vec3(0.0,0.0,1.0);
	gs_out.texcoord = vec2(0.01, 0.01);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4(-0.25 + modif, grassHeight, 0.0, 0.0);
	gs_out.normal = vec3(0.0,0.0,1.0);
	gs_out.texcoord = vec2(.99, 0.01);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4(-0.25 , 0.0, 0.0, 0.0);
	gs_out.normal = vec3(0.0,0.0,1.0);
	gs_out.texcoord = vec2(.99, .99);
	EmitVertex();
	
	EndPrimitive();

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4(0.0, 0.0, -0.25, 0.0); 
	gs_out.normal = vec3(1.0,0.0,0.0);
	gs_out.texcoord = vec2(.99, .99);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4( 0.0, 0.0, 0.25, 0.0);
	gs_out.normal = vec3(1.0,0.0,0.0);
	gs_out.texcoord = vec2(0.01, .99);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4( 0.0 + modif, grassHeight, 0.25, 0.0);
	gs_out.normal = vec3(1.0,0.0,0.0);
	gs_out.texcoord = vec2(0.01, 0.01);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4(0.0 + modif, grassHeight, -0.25, 0.0);
	gs_out.normal = vec3(1.0,0.0,0.0);	
	gs_out.texcoord = vec2(.99, 0.01);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4(0.0 , 0.0, -0.25, 0.0);
	gs_out.normal = vec3(1.0,0.0,0.0);
	gs_out.texcoord = vec2(.99, .99);
	EmitVertex();

	EndPrimitive();
}