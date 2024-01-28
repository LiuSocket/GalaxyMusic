#version 400 compatibility

uniform float starAlpha;

in vec4 vertexColor;
in float playingStar;
out vec4 fragColor;

void main() 
{
	vec2 fall = 2*abs(gl_PointCoord.st-0.5);
	float lenFall = length(fall);
	float shapeX = clamp(1-sqrt(abs(fall.x-fall.y)),0.3,0.8);
	float shineLine = mix(0.5, 1.0, playingStar)*pow(shapeX, 8*lenFall);
	float shineFall = max(0.1,1-abs(fall.x*fall.y));

	float alpha = vertexColor.a*shineFall*shineLine*max(0, 10*starAlpha-9);
	fragColor = vec4(mix(vertexColor.rgb, vec3(1), min(0.5,pow(shineFall, 15)*shineLine)), 1.0)*alpha;
}