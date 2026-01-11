#version 400 compatibility

uniform sampler2D supernovaTex;
uniform vec4 playingStarColor;
uniform float supernovaAlpha;
uniform float supernovaWidth;

in vec2 modelPosXY;
out vec4 fragColor;

void main() 
{
	vec4 supernovaColor = texture(supernovaTex, gl_TexCoord[0].xy);
	float edgeFade = gl_TexCoord[0].y*(1-gl_TexCoord[0].y);
	supernovaColor *= mix(vec4(3*edgeFade), vec4(playingStarColor.rgb, 0.4), clamp(length(modelPosXY),0,1));
	fragColor = supernovaColor;
	fragColor.a *= supernovaAlpha / max(1.0, 0.2*supernovaWidth);
}