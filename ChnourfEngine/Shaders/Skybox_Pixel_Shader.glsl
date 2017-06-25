//#version 330 core
in vec3 TexCoords;

out vec4 color;
uniform vec3 viewPos;
uniform vec3 lightDirection;
uniform vec3 lightColor;

#define SC (250.0)

const vec3 Kr = vec3(5.5e-6, 13.0e-6, 22.4e-6);

float phase(float alpha, float g){
    float a = 3.0*(1.0-g*g);
    float b = 2.0*(2.0+g*g);
    float c = 1.0+alpha*alpha;
    float d = pow(1.0+g*g-2.0*g*alpha, 1.5);
    return (a/b)*(c/d);
}

float atmospheric_depth(vec3 position, vec3 dir){
    float a = dot(dir, dir);
    float b = 2.0*dot(dir, position);
    float c = dot(position, position)- 1.0;
    float det = b*b-4.0*a*c;
    float detSqrt = sqrt(det);
    float q = (-b - detSqrt)/2.0;
    float t1 = c/q;
    return t1;
}

float horizon_extinction(vec3 position, vec3 dir, float radius){
    float u = dot(dir, -position);
    if(u<0.0){
        return 1.0;
    }
    vec3 near = position + u*dir;
    if(length(near) < radius){
        return 0.0;
    }
    else{
        vec3 v2 = normalize(near)*radius - position;
        float diff = acos(dot(normalize(v2), dir));
        return smoothstep(0.0, 1.0, pow(diff*2.0, 3.0));
    }
}

void main()
{   
	vec3 normalizedToLightDirection = normalize(-lightDirection);

	vec3 rd = normalize(TexCoords.xyz);
	float sundot = clamp(dot(rd,normalizedToLightDirection),0.0,1.0);
	
	//Rayleigh scattering test
	float step_count = 30;
	float rayleigh_factor = 3/2 * (1 + sundot * sundot) * 300.0;
	float mie_factor = phase(sundot, -0.758) * 200.0;
	float spot = smoothstep(0.0, 200.0, phase(sundot, 0.9995))*600.0;
	
	float eye_height = 1-13.0/6400.0;
	vec3 eye_position = vec3(0.0, eye_height, 0.0);
	float eye_depth = atmospheric_depth(eye_position, rd);
	float step_length = eye_depth/float(step_count);
	float eye_extinction = horizon_extinction(eye_position, rd, eye_height-0.05);

	vec3 rayleigh_collected = vec3(0.0, 0.0, 0.0);
	vec3 mie_collected = vec3(0.0, 0.0, 0.0);
	for(int i=0; i<step_count; i++)
	{
		float sample_distance = step_length*float(i);
		vec3 position = eye_position + rd*sample_distance;
		float extinction = horizon_extinction( position, normalizedToLightDirection, eye_height-0.0001);
		float sample_depth = atmospheric_depth(position, normalizedToLightDirection);
		
		float scale = 1e7;
		rayleigh_collected += vec3(1.0,0.9,1.0) * exp(-(sample_distance+sample_depth)* scale * Kr)* extinction;
		mie_collected += exp(-(sample_distance+sample_depth) * scale * vec3(22.4e-6))* extinction;
	}
	
	rayleigh_collected = (rayleigh_collected * eye_extinction * eye_depth * Kr * 1/22e-6)/float(step_count);
	mie_collected = (mie_collected * eye_extinction * eye_depth)/float(step_count);

	vec3 res = vec3(spot*normalize(rayleigh_collected) + mie_factor*mie_collected + rayleigh_factor*rayleigh_collected);

	// clouds
	vec2 sc = viewPos.xz + rd.xz*(SC*1000.0-viewPos.y)/rd.y;
	res = mix( res, length(res)*vec3(2.0,1.9,2.0), 0.5*smoothstep(0.7,0.8,fbm(0.0005*sc/SC)) );

    res = res + vec3(0.0, 0.03, 0.05); //night color
	color = vec4(res, 1.0f);
}
  