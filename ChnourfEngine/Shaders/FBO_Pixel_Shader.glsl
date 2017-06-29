#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D screenTexture;
uniform float exposure;

const float A = 0.22;
const float B = 0.30;
const float C = 0.10;
const float D = 0.20;
const float E = 0.01;
const float F = 0.30;

void main()
{
    vec3 hdrColor = texture(screenTexture, TexCoords, 0).rgb;
	
	//Exposure
	hdrColor = hdrColor * exposure;
	
	// Reinhardt tonemapping
	//hdrColor = hdrColor/(1+hdrColor);
	//hdrColor = pow(hdrColor, vec3(1.0/2.2));
	
	// Filmic tonemapping, gamma included
	//vec3 x = max(vec3(0), hdrColor - vec3(0.004));
    //hdrColor = (x*(6.2*x + 0.5)) / (x*(6.2*x + 1.7) + 0.06);

	// Custom Filmic Tonemapping
    hdrColor = ((hdrColor * (A*hdrColor + C*B) + D*E)/(hdrColor * (A*hdrColor + B) + D*F)) - E/F;
	//hdrColor = pow(hdrColor, vec3(2.2));

    color = vec4(hdrColor, 1);
}