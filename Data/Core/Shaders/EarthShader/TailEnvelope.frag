#version 400 compatibility
#pragma import_defines(SPIRAL)

const float M_PI = 3.141592657;
const float PROGRESS_1 = 0.020;
const float PROGRESS_2 = 0.050;
const float PROGRESS_2_1 = 0.053;
const float PROGRESS_3 = 0.080;
const float PROGRESS_3_1 = 0.083;

uniform float unit;
uniform float times;
uniform float wanderProgress;
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

#ifdef SPIRAL
	const vec2 speed = vec2(0.02,0.023);
#else // not SPIRAL
	const vec2 speed = vec2(0.01,0.012);
#endif // SPIRAL or not

	float noiseD1 = texture(noise2DTex, gl_TexCoord[0].xy*vec2(0.5, 0.7) - times*speed.x).r;
	vec2 coord = gl_TexCoord[0].xy - vec2(0.1*noiseD1, times*speed.y);
	float noiseD = texture(noise2DTex, fract(coord)).r;

	const float gForward = 0.98;
	float forwardScattering = (1-gForward*gForward)/(4*M_PI*pow(1+gForward*gForward-2*gForward*dotVL, 1.5));
	float scattering = 0.5 + forwardScattering;

	// diffuse
	vec3 diffuse = vec3(0.7, 1.2, 3.0)*scattering;
	diffuse *= smoothstep(vec3(-0.55,-0.52,-0.5), vec3(0.2,0.24,0.28), vec3(dotVUL));

	float v = gl_TexCoord[0].y;
#ifdef SPIRAL
	float fade_0 = clamp(v*30, 0, 1)*clamp(1-v, 0, 1);
	float fade_1 = clamp((v-1)*30, 0, 1)*clamp(2-v, 0, 1);
	float alpha = (1-noiseD)*(fade_0 + fade_1);

	float tailCordX = fract(gl_TexCoord[0].x*0.2);
	float edgeFade = min(1, 8*tailCordX*(1-tailCordX)*abs(dotNV));
	alpha*= edgeFade;

	// accelerate progress
	float spiralAcceGrowTime = clamp((wanderProgress-PROGRESS_1)*2/(PROGRESS_2-PROGRESS_1), 0, 1);
	float spiralAcceFallTime = clamp((wanderProgress-PROGRESS_2_1)*0.2/(PROGRESS_2-PROGRESS_2_1), 0, 1);
	float spiralAcceGrowSpace = clamp((1.2*spiralAcceGrowTime-v)*5, 0, 1);
	float spiralAcceFallSpace = clamp((v-0.5+1.5*spiralAcceFallTime)*2, 0, 1);
	// decelerate progress
	float spiralDeceGrowTime = clamp((wanderProgress-PROGRESS_2)*2/(PROGRESS_3-PROGRESS_2), 0, 1);
	float spiralDeceFallTime = clamp((wanderProgress-PROGRESS_3_1)*0.2/(PROGRESS_3-PROGRESS_3_1), 0, 1);
	float spiralDeceGrowSpace = clamp((1.2*spiralDeceGrowTime-(v-1))*5, 0, 1);
	float spiralDeceFallSpace = clamp((v-1.5+1.5*spiralDeceFallTime)*2, 0, 1);
	// acce / dece
	alpha *= mix(spiralAcceGrowSpace*spiralAcceFallSpace, spiralDeceGrowSpace*spiralDeceFallSpace, float(v>1));

	// ambient color
	vec3 ambient = vec3(0.2, 0.35, 0.5)*exp2(-max(0, v-float(v>1))*20);
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