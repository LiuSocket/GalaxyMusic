#version 400 compatibility

uniform float level[128];
uniform float times;
uniform sampler2D sunNoiseTex;
uniform vec4 playingStarColor;

in vec3 viewPos;

void main() 
{
	float lenV = length(viewPos);
	const float PI = 3.14159265358979;
	vec2 tanDir = normalize(gl_TexCoord[0].xy);
	float noise = texture2D(sunNoiseTex, vec2(atan(tanDir.y/tanDir.x)/PI, times*0.1)).r;
	float noise_1 = texture2D(sunNoiseTex, vec2(atan(tanDir.x/tanDir.y)/PI, times*0.09)).r;
	vec2 fall = abs(gl_TexCoord[0].xy);
	float radius = length(fall);
	float shininess = pow(clamp((1.125+0.01*noise_1)*(1-radius), 0, 1), 4+4*exp2(-lenV*0.5)+2*noise-2.5*level[0]);
	vec4 brightColor = mix(playingStarColor, vec4(1), 0.95);
	vec4 mixColor = mix(playingStarColor, brightColor, shininess);
	gl_FragColor = mixColor * shininess;
}