#version 400 compatibility

in vec4 vertexColor;

void main() 
{
	vec2 fall = 2*abs(gl_PointCoord.st-0.5);
	float radius = clamp(1-length(fall),0,1);
	radius = 1.3*radius*radius*radius;
	gl_FragColor = vec4(vertexColor.rgb,vertexColor.a*radius);
}