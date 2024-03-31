#version 400 compatibility

const float M_PI = 3.141592657;

const float PROGRESS_0 = 0.005;
const float PROGRESS_1 = 0.020;
const float PROGRESS_1_1 = 0.023;

uniform float unit;
uniform float times;
uniform float wanderProgress;
uniform vec3 viewLight;
uniform sampler2D noise2DTex;

in vec3 viewModelVertPos;
in vec3 viewPos;
out vec4 fragColor;

vec3 ToneMapping(vec3 color)
{
	const float A = 2.51;
	const float B = 0.03;
	const float C = 2.43;
	const float D = 0.59;
	const float E = 0.14;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}

void main()
{
	vec2 UV = gl_TexCoord[0].xy;
	vec2 coord1 = UV;
	coord1.y *= 2.8*coord1.y;
	coord1 -= times*0.05-0.11-0.1*gl_TexCoord[0].z;
	float noiseD1 = texture(noise2DTex, coord1).r;

	vec2 coord = UV;
	coord.y *= 10*coord.y*(1-0.8*coord.y);
	coord -= vec2(times*(0.05+0.02*abs(gl_TexCoord[0].z-0.9)), 0.1*noiseD1);
	float noiseD = texture(noise2DTex, fract(coord)).r;

	float lenV = length(viewPos);
	vec3 viewDir = normalize(viewPos);
	float dotVL = dot(viewDir, viewLight);
	float dotVUL = dot(normalize(viewModelVertPos), viewLight);

	// shadow
	float shadowEdge = dotVUL + 0.85*sqrt(UV.y) - 0.3;
	vec3 shadow = smoothstep(vec3(-0.55,-0.52,-0.5), vec3(0.2,0.24,0.28), vec3(shadowEdge));

	const float gForward = 0.98;
	float forwardScattering = (1-gForward*gForward)/(4*M_PI*pow(1+gForward*gForward-2*gForward*dotVL,1.5));
	float scattering = 0.5 + forwardScattering;
	// diffuse
	vec3 diffuse = vec3(0.7, 1.2, 3.0)*shadow*scattering;
	// ambient color
	vec3 ambient = vec3(0.2, 0.35, 0.5)*exp2(-UV.y*10);

	float alpha = (1-noiseD)*sqrt(clamp(UV.y*2,0,1))*(1-UV.y);
	alpha *= exp2(-lenV*unit*1e-9) * (1 - exp2(-lenV*unit*5e-7));
	// progress
	float ringGrowTime = clamp((wanderProgress-PROGRESS_0)*2/(PROGRESS_1-PROGRESS_0),0,1);
	float ringFallTime = clamp((wanderProgress-PROGRESS_1_1)/(PROGRESS_1-PROGRESS_1_1),0,1);
	float ringGrowSpace = clamp((1.5*ringGrowTime-UV.y)*2, 0, 1);
	float ringFallSpace = clamp((UV.y-0.5+1.5*ringFallTime)*2, 0, 1);
	alpha *= ringGrowSpace*ringFallSpace;

	vec3 color = ToneMapping(ambient + diffuse);
	fragColor = vec4(color, alpha);
}