#version 400 compatibility

const float M_PI = 3.141592657;

uniform float unit;
uniform float times;
uniform vec2 engineStartRatio;
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
	vec2 coord1 = gl_TexCoord[0].xy;
	coord1.x *= coord1.x;
	coord1 -= 0.11+times*0.05;
	float noiseD1 = texture(noise2DTex, coord1).r;

	vec2 coord = gl_TexCoord[0].xy;
	coord.x *= 1.91*coord.x;
	coord -= vec2(0.1*noiseD1, times*0.06);
	float noiseD = texture(noise2DTex, fract(coord)).r;

	float lenV = length(viewPos);
	vec3 viewDir = normalize(viewPos);
	float dotVL = dot(viewDir, viewLight);
	float dotVUL = dot(normalize(viewModelVertPos), viewLight);

	const float gForward = 0.98;
	float forwardScattering = (1-gForward*gForward)/(4*M_PI*pow(1+gForward*gForward-2*gForward*dotVL,1.5));
	float scattering = 0.5 + forwardScattering;
	// diffuse
	vec3 diffuse = vec3(0.7, 1.2, 3.0)*scattering;
	diffuse *= smoothstep(vec3(-0.55,-0.52,-0.5), vec3(0.2,0.24,0.28), vec3(dotVUL));
	// ambient color
	vec3 ambient = vec3(0.2, 0.35, 0.5)*exp2(-gl_TexCoord[0].x*20);

	float alpha = 1-noiseD;
	alpha *= exp2(-lenV*unit*1e-9) * (1 - exp2(min(0, 1e-3-lenV)*unit*2e-7));
	vec3 color = ToneMapping(ambient + diffuse);
	fragColor = vec4(color, alpha);
}