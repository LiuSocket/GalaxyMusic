#version 400 compatibility

uniform float supernovaBloomScale;
uniform float supernovaBloomAlpha;
uniform vec4 playingStarColor;

in vec2 modelPosXY;
out vec4 fragColor;

void main() 
{
	float radius = length(modelPosXY);
	float bloom = supernovaBloomScale*supernovaBloomAlpha*0.5*
		(exp2(-radius*radius*(100-75*supernovaBloomAlpha)) + exp(-radius*(2-supernovaBloomAlpha)));
	fragColor = vec4(exp2(-radius*0.02/max(vec3(0.01),playingStarColor.rgb)), bloom);
}