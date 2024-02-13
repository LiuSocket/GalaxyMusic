#version 400 compatibility

const float M_PI = 3.141592657;

uniform float unit;
uniform float times;
uniform vec2 engineStartRatio;
uniform vec3 viewLight;
uniform sampler2D noise2DTex;

float EARTH_RADIUS = 6.36e6/unit;

in vec3 viewModelTailDir;
in vec3 viewModelVertPos;
in vec3 viewPos;
in vec3 viewNormal;
out vec4 fragColor;

// length of point to line
float Len_Point2Line(vec3 point, vec3 lineDir)
{	
	return length(point-dot(point, lineDir)*lineDir);
}

vec3 ToneMapping(vec3 color)
{
	const float A = 2.51;
	const float B = 0.03;
	const float C = 2.43;
	const float D = 0.59;
	const float E = 0.14;
	return pow((color * (A * color + B)) / (color * (C * color + D) + E), vec3(1.0/2.2));
}

void main()
{
	vec3 viewDir = normalize(viewPos);
	vec3 viewNorm = normalize(viewNormal);
	float lenV = length(viewPos);
	float dotVL = dot(viewDir, viewLight);
	float dotNV = dot(viewNorm, -viewDir);
	float dotVUL = dot(normalize(viewModelVertPos), viewLight);
	float dotNC = dot(viewNorm, normalize(cross(viewModelTailDir, viewDir)));

	const float v0 = 1;
	const float v1 = 4;
	const float v2 = 6;
	float v = gl_TexCoord[0].y;

	float noiseD1 = texture(noise2DTex, gl_TexCoord[0].xy*vec2(0.5, 0.7) - times*0.005).r;
	vec2 coord = gl_TexCoord[0].xy;
	coord.x -= mix(0.2, 0.05, float(v>v0))*noiseD1;
	coord.y -= times*0.006;
	float noiseD = texture(noise2DTex, fract(coord)).r;

	const float gForward = 0.98;
	float forwardScattering = (1-gForward*gForward)/(4*M_PI*pow(1+gForward*gForward-2*gForward*dotVL, 1.5));
	float scattering = 0.5 + forwardScattering;

	// ambient color
	vec3 ambient = vec3(0.2, 0.35, 0.5)*exp2(-gl_TexCoord[0].y*5);
	// diffuse
	vec3 diffuse = vec3(0.7, 1.2, 3.0)*scattering;
	diffuse *= smoothstep(vec3(-0.55,-0.52,-0.5), vec3(0.2,0.24,0.28), vec3(dotVUL));

	float fade_0 = clamp(v*30, 0, 1)*clamp((v0-v)*6, 0, 1)
		*(1-exp2(min(0, v - v0*(engineStartRatio.x - 0.5))*10));
	float fade_1 = clamp((v-v0)*20, 0, 1)*clamp(v1-v, 0, 1)
		*(1-exp2(min(0, v - mix(v0, v1, clamp(0.15*(engineStartRatio.y - 1), 0, 1)))*20));
	float fade_2 = clamp((v-v1)*5, 0, 1)*clamp(v2-v, 0, 1)
		*(1-exp2(min(0, v - mix(v1, v2, clamp(0.15*(engineStartRatio.y - 1), 0, 1)))*30));

	float tailCordX = fract(gl_TexCoord[0].x*3);
	float edgeFade = 4*tailCordX*(1-tailCordX)*(1-dotNC*dotNC);

	float alpha = (1-noiseD)*(fade_0 + fade_1 + fade_2);
	alpha *= (4>gl_TexCoord[0].y) ? abs(dotNV) : edgeFade;
	alpha *= exp2(-lenV*unit*1e-9) * (1 - exp2(min(0, 1e-3-lenV)*unit*2e-7));

	vec3 color = ToneMapping(ambient + diffuse);
	fragColor = vec4(color, alpha);
}