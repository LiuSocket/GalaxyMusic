#version 400 compatibility

uniform float level[128];
uniform float times;
uniform float backgroundSunAlpha;
uniform float eyeAltitude;
uniform float sunEdgePos; // (0.0,1.0)
uniform sampler2D sunNoiseTex;
uniform vec4 playingStarColor;

void main() 
{
	const float PI = 3.141592654;
	vec2 tanDir = normalize(gl_TexCoord[0].xy);
	float noise = texture2D(sunNoiseTex, vec2(atan(tanDir.y/tanDir.x)/PI, times*0.02)).r;
	vec2 fall = abs(gl_TexCoord[0].xy);
	float radius = length(fall);
	float nearSun = exp2(-eyeAltitude);
	float shininess = exp2(-pow(max(0, radius-sunEdgePos), 0.5+(0.8+0.2*noise)*(1-0.8*nearSun)) * (4.5-1.5*level[0]*noise)); 
	vec3 mixColor = mix(playingStarColor.rgb, vec3(1), min(1-0.2*nearSun, backgroundSunAlpha+shininess*shininess));
	gl_FragColor = vec4(mixColor, shininess);
}