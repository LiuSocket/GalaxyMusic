#version 400 compatibility

uniform sampler2D supernovaTex;
uniform vec4 playingStarColor;
uniform float supernovaAlpha;

in vec2 modelPosXY;
out vec4 fragColor;

void main() 
{
	vec4 supernovaColor = texture(supernovaTex, gl_TexCoord[0].xy)
		* mix(vec4(0.8), vec4(playingStarColor.rgb, 0.4), clamp(length(modelPosXY),0,1));
	fragColor = supernovaColor*supernovaAlpha;
}