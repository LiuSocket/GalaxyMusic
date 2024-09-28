#version 400 compatibility

void main()
{
	vec2 radiusXY = 2 * abs(gl_PointCoord.st - 0.5);
	float fall = clamp(1 - length(radiusXY), 0, 1);
	if(fall < 0.01) discard;
	
	// bloom
	gl_FragColor = vec4(1, 0, 0, pow(fall, 16.0));
}