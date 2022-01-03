#version 400 compatibility

in vec2 falloff;
in vec4 vertexColor;

void main() 
{
	vec2 fall = 2*abs(gl_PointCoord.st-0.5);
	float radius = clamp(1-length(fall),0,1);
	radius = pow(radius, min(16.0, 2.0+25.0*falloff.y));

	gl_FragColor = falloff.x*mix((0.7+0.3*vertexColor)*radius, vec4(0.8,1.0,1.2,1.0), radius);
}