#version 400 compatibility

uniform float level[128];
uniform float times;
uniform float backgroundSunAlpha;
uniform sampler2D sunNoiseTex;
uniform vec4 playingStarColor;

void main() 
{
	vec2 tanDir = normalize(gl_TexCoord[0].xy);
	float noise = texture2D(sunNoiseTex, vec2(atan(tanDir.y/tanDir.x)*0.1, times*0.02)).r;
	vec2 fall = abs(gl_TexCoord[0].xy);
	float radius = length(fall);
	float shininess = exp2(-sqrt(max(0, radius - 0.015))*(8 - pow(noise, 1 + 9*radius) - level[0]));
	gl_FragColor = vec4(playingStarColor.rgb*smoothstep(0.0, 0.02, shininess), backgroundSunAlpha*shininess);
}