//#version 450 core //lower this version if your card does not support GLSL 4.5
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
	vec3 normal;
} vs_in[];

layout (std140) uniform ViewConstants
{
	mat4 projection;
	mat4 view;
};

uniform float elapsedTime;

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
	gs_out.normal = vs_in[0].normal;

	float grassHeight = fbm(vec2(vs_in[0].position.x*10, vs_in[0].position.z*10));
	float modifTime = elapsedTime/3000.f + 0.5* grassHeight;
	float modif = cos(modifTime)*cos(3*modifTime)*cos(5*modifTime)*cos(7*modifTime)*sin(25*modifTime)*0.3*grassHeight;
	//mat4 rot = rotationMatrix(vec3(0.0, 1.0, 0.0), 3.0*fbm(vec2(vs_in[0].position.x*100, vs_in[0].position.z*100)));
	
	// there has to be a better way to do that, acos is evil
	mat4 terrainRot = rotationMatrix(cross(vec3(0.0, 1.0, 0.0), vs_in[0].normal), acos(dot(vec3(0.0, 1.0, 0.0), vs_in[0].normal)));
	mat4 rot = terrainRot * rotationMatrix(vs_in[0].normal, 3.0*fbm(vec2(vs_in[0].position.x*100, vs_in[0].position.z*100)));

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4(-0.25, 0.0, 0.0, 0.0);
	gs_out.texcoord = vec2(.99, .99);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4( 0.25, 0.0, 0.0, 0.0);
	gs_out.texcoord = vec2(0.01, .99);
	EmitVertex();
	
	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4( 0.25 + modif, grassHeight, 0.0, 0.0);
	gs_out.texcoord = vec2(0.01, 0.01);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4(-0.25 + modif, grassHeight, 0.0, 0.0);
	gs_out.texcoord = vec2(.99, 0.01);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4(-0.25 , 0.0, 0.0, 0.0);
	gs_out.texcoord = vec2(.99, .99);
	EmitVertex();
	
	EndPrimitive();

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4(0.0, 0.0, -0.25, 0.0); 
	gs_out.texcoord = vec2(.99, .99);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4( 0.0, 0.0, 0.25, 0.0);
	gs_out.texcoord = vec2(0.01, .99);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4( 0.0 + modif, grassHeight, 0.25, 0.0);
	gs_out.texcoord = vec2(0.01, 0.01);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4(0.0 + modif, grassHeight, -0.25, 0.0);
	gs_out.texcoord = vec2(.99, 0.01);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + projection*view*rot*vec4(0.0 , 0.0, -0.25, 0.0);
	gs_out.texcoord = vec2(.99, .99);
	EmitVertex();

	EndPrimitive();
}