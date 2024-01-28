#version 400 compatibility

uniform vec4 playingStarColor;
uniform float level[128];
uniform float times;
uniform sampler3D shapeNoiseTex;
uniform sampler2D starTex;

in vec4 modelPos;
in vec3 viewPos;
in vec3 viewNormal;

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
	vec2 angle = -times*vec2(0.101,0.083);
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
	float dotVN = max(0,dot(-viewVertDir, viewNorm));

	vec2 dirtNoise = texture3D(shapeNoiseTex, noiseCoord_1*25.0).xy;
	vec2 shapeNoise = texture3D(shapeNoiseTex, noiseCoord_2*13.0*(1-0.1*dirtNoise.y-0.05*abs(fract(times*0.1)-0.5)-0.02*level[0])).xy;
	vec3 starBaseColor = texture(starTex, gl_TexCoord[0].xy+vec2(times*0.01,0)).rgb;
	float brightness = starBaseColor.r;
	starBaseColor *= playingStarColor.rgb/vec3(0.710, 0.4, 0.094);
	starBaseColor = ToneMapping(starBaseColor*4);

	float levelCoord = (1-brightness)*127;
	vec3 sunColor = mix(starBaseColor,
		mix(playingStarColor.rgb,
			mix(starBaseColor, vec3(1), 0.8),
			mix(1-dirtNoise.x, shapeNoise.y, 2*abs(shapeNoise.x-0.5))),
		0.2+0.8*brightness*level[int(levelCoord)]);
	sunColor = mix(vec3(1), sunColor, dotVN*exp2(-length(viewPos)*5));
	gl_FragColor = vec4(sunColor,1);
}