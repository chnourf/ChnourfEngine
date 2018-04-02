// Environment.h

out vec4 color;

in VS_OUT
{
	vec3 fragPos;
} fs_in;

void main()
{
	vec3 result = vec3(0.0, 0.2, 0.8);

	vec3 lightDir = normalize(-lightDirection);
	vec3 viewDir = normalize(viewPos - fs_in.fragPos);
	result = Fog( result, fs_in.fragPos, viewDir, lightDir );
	color = vec4(result, 0.5);
}