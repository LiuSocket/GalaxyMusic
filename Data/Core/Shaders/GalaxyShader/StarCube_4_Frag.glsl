#version 400 compatibility

uniform vec4 playingStarColor;
uniform float starAlpha;

in float falloff;
in vec3 vertexColor;
out vec4 fragColor;

void main()
{
	vec2 fall = 2*abs(gl_PointCoord.st-0.5);
	float radius = clamp(1-length(fall),0,1);
	float alpha = falloff*radius;

	vec3 vertColor = mix(0.7+0.3*vertexColor, vertexColor, starAlpha);
	vec3 finalColor = mix(vec3(0.8, 0.9, 1.0), playingStarColor.rgb+0.5, starAlpha);
	fragColor = vec4(mix(vertColor, finalColor, alpha), 1.0)*alpha;
}