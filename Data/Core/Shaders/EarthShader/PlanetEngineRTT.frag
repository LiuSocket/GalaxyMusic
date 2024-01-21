#version 400 compatibility

uniform sampler2D engineTex;

void main()
{
	vec2 radiusXY = 2 * abs(gl_PointCoord.st - 0.5);
	float fall = clamp(1 - length(radiusXY), 0, 1);
	if(fall < 0.01) discard;
	
	// engine body
	gl_FragData[0] = texture(engineTex, 0.5 + 20*(gl_PointCoord.st-0.5));
	// bloom
	gl_FragData[1] = vec4(1, 0, 0, pow(fall, 16.0));
}