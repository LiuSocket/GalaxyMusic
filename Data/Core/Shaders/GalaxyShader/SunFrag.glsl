#version 400 compatibility

uniform vec4 playingStarColor;
uniform float level[128];
uniform float times;
uniform sampler3D shapeNoiseTex;

in vec4 modelPos;
in vec3 worldPos;
in vec3 viewPos;
in vec3 viewNormal;

void main() 
{
	vec2 angle = -times*vec2(0.11,0.13);
	vec2 cosA = cos(angle);
	vec2 sinA = sin(angle);
	mat4 rotateMatrix_1 = mat4(
		cosA.x, sinA.x,	0,	0,
		-sinA.x,cosA.x,	0,	0,
		0,		0,		1,	0,
		0,		0,		0,	1);
	mat4 rotateMatrix_2 = mat4(
		cosA.y, sinA.y,	0,	0,
		-sinA.y,cosA.y,	0,	0,
		0,		0,		1,	0,
		0,		0,		0,	1);
	vec3 noiseCoord_1 = (rotateMatrix_1 * modelPos).xyz;
	vec3 noiseCoord_2 = (rotateMatrix_2 * modelPos).xyz;

	vec3 viewVertDir = normalize(viewPos);
	vec3 viewNorm = normalize(viewNormal);
	float dotVN = max(dot(-viewVertDir, viewNorm), 0);

	float shapeNoise = texture3D(shapeNoiseTex, noiseCoord_1*43*(1-0.05*sin(times)-0.03*sqrt(level[0]))).r;
	float dirtNoise = texture3D(shapeNoiseTex, noiseCoord_2*51.0).r;

	vec3 brightColor = mix(playingStarColor.xyz, vec3(1), 0.95);
	vec3 sunColor = mix(
		abs(1.5*playingStarColor.xzy-0.2),
		mix(playingStarColor.xyz, brightColor, mix(1-dirtNoise, shapeNoise, 2*abs(shapeNoise-0.5))),
		sqrt(shapeNoise));
	sunColor = mix(brightColor, sunColor, 0.3*dotVN);
	gl_FragColor = vec4(sunColor,1);
}