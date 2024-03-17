#version 400 compatibility
#pragma import_defines(SPIRAL)

const float M_PI = 3.141592657;

uniform float unit;
uniform float times;
uniform vec2 engineStartRatio;
uniform vec3 viewLight;
uniform sampler2D noise2DTex;

in vec3 viewModelTailDir;
in vec3 viewModelVertPos;
in vec3 viewPos;
in vec3 viewNormal;
out vec4 fragColor;

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

	float noiseD1 = texture(noise2DTex, gl_TexCoord[0].xy*vec2(0.5, 0.7) - times*0.005).r;
	vec2 coord = gl_TexCoord[0].xy - vec2(0.1*noiseD1, times*0.006);
	float noiseD = texture(noise2DTex, fract(coord)).r;

	const float gForward = 0.98;
	float forwardScattering = (1-gForward*gForward)/(4*M_PI*pow(1+gForward*gForward-2*gForward*dotVL, 1.5));
	float scattering = 0.5 + forwardScattering;

	// diffuse
	vec3 diffuse = vec3(0.7, 1.2, 3.0)*scattering;
	diffuse *= smoothstep(vec3(-0.55,-0.52,-0.5), vec3(0.2,0.24,0.28), vec3(dotVUL));

	float v = gl_TexCoord[0].y;
#ifdef SPIRAL
	float fade = clamp(v*30, 0, 1)*clamp(1-v, 0, 1);
	float alpha = (1-noiseD)*fade;

	float tailCordX = fract(gl_TexCoord[0].x*0.2);
	float edgeFade = min(1, 8*tailCordX*(1-tailCordX)*abs(dotNV));
	alpha*= edgeFade;

	// ambient color
	vec3 ambient = vec3(0.2, 0.35, 0.5)*exp2(-v*20);
#else // not SPIRAL
	const float v0 = 1;
	const float v1 = 4;
	const float v2 = 6;

	float torqueGrow = (1+0.5*noiseD1)*(engineStartRatio.x - 0.5);
	float tailGrow = clamp((0.1+0.05*noiseD1)*(engineStartRatio.y - 1), 0, 1);
	float fade_0 = clamp(v*20, 0, 1)*clamp(v0-v, 0, 1)*(1-exp2(min(0, v - v0*torqueGrow)*10));
	float fade_1 = clamp((v-v0)*20, 0, 1)*clamp(v1-v, 0, 1)*(1-exp2(min(0, v - mix(v0, v1, tailGrow))*20));
	float fade_2 = clamp((v-v1)*30, 0, 1)*clamp((v2-v)*0.2, 0.0, 0.5)*(1-exp2(min(0, v - mix(v1, v2, tailGrow))*30));

	float tailCordX = fract(gl_TexCoord[0].x*3);
	float edgeFade = 4*tailCordX*(1-tailCordX)*(1-dotNC*dotNC);

	float alpha = (1-noiseD)*(fade_0 + fade_1 + fade_2);
	alpha *= (4>gl_TexCoord[0].y) ? dotNV*dotNV : edgeFade;

	// ambient color
	vec3 ambient = vec3(0.2, 0.35, 0.5)*exp2(-v*5);
#endif // SPIRAL or not

	alpha *= exp2(-lenV*unit*1e-9) * (1 - exp2(min(0, 1e-3-lenV)*unit*2e-7));
	vec3 color = ToneMapping(ambient + diffuse);
	fragColor = vec4(color, alpha);
}