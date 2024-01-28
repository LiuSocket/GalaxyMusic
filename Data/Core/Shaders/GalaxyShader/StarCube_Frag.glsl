#version 400 compatibility

in float falloff;
in vec3 vertexColor;

void main() 
{
	vec2 fall = 2*abs(gl_PointCoord.st-0.5);
	float radius = clamp(1-length(fall),0,1);
	float alpha = falloff*radius;
	// alpha*alpha to void alpha jump when 4->3
	float finalAlpha = max(3*(alpha-0.6), 0.5*alpha*alpha);
	gl_FragColor = vec4(mix(0.7+0.3*vertexColor, vec3(0.8,0.9,1.0), alpha), 1.0)*finalAlpha;
}