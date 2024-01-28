#version 400 compatibility

uniform sampler2D supernovaTex;
uniform vec4 playingStarColor;
uniform vec2 supernovaAlpha;

in float bStick;
in vec2 modelPosXY;
out vec4 fragColor;

void main() 
{
	vec4 supernovaColor = texture2D(supernovaTex, gl_TexCoord[0].xy);
	supernovaColor.rgb *= mix(vec3(1), playingStarColor.rgb, clamp(length(modelPosXY)*(3-2*bStick),0,1));
	supernovaColor *= mix(supernovaAlpha.x, supernovaAlpha.y, bStick);
	fragColor = supernovaColor;
}