#version 400 compatibility

uniform vec4 playingStarColor;
uniform float starAlpha;

in vec2 falloff;
in vec4 vertexColor;

void main() 
{
	vec2 fall = 2*abs(gl_PointCoord.st-0.5);
	float radius = clamp(1-length(fall),0,1);
	radius = pow(radius, min(16.0, 2.0+25.0*falloff.y));

	vec4 vertColor = mix(0.7+0.3*vertexColor, vertexColor, starAlpha);
	vec4 finalColor = mix(vec4(0.8,1.0,1.2,1.0), 1.2*playingStarColor+vec4(1), starAlpha);
	gl_FragColor = falloff.x*mix(vertColor*radius, finalColor, radius);
}