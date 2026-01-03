#version 400 compatibility

#pragma import_defines(RESOLUTION_QUARTER)

uniform vec3 screenSize;

uniform sampler2D colorTex;
uniform sampler2D alphaTex;

void main()
{
	vec2 uv = gl_FragCoord.xy/screenSize.xy;
	vec4 finalColor = texture(colorTex, uv);

#ifdef RESOLUTION_QUARTER
	vec2 subPixDir = sign(mod(gl_FragCoord.xy, vec2(4))-2);
	vec2 alphaPixelUnit = subPixDir*4;
#else // !RESOLUTION_QUARTER
	vec2 subPixDir = sign(mod(gl_FragCoord.xy, vec2(2))-1);
	vec2 alphaPixelUnit = subPixDir*2;
#endif // RESOLUTION_QUARTER or not

	vec4 alpha4_00 = texture(alphaTex, uv);
	vec4 alpha4_10 = texture(alphaTex, (gl_FragCoord.xy + vec2(alphaPixelUnit.x, 0))/screenSize.xy);
	vec4 alpha4_01 = texture(alphaTex, (gl_FragCoord.xy + vec2(0, alphaPixelUnit.y))/screenSize.xy);
	vec4 alpha4_11 = texture(alphaTex, (gl_FragCoord.xy + alphaPixelUnit)/screenSize.xy);

	bool isLU = (subPixDir.y-subPixDir.x)>1;
	bool isRU = (subPixDir.y+subPixDir.x)>1;
	bool isRD = (subPixDir.x-subPixDir.y)>1;
	bool isLD = (subPixDir.y+subPixDir.x)<(-1);

	vec4 corner4 = vec4(isRD, isLD, isLU, isRU);
	vec4 filter_00 = 1 - corner4*0.25;
	vec4 filter_10 = vec4(
		isRD ? 0.75 : float(isRU),
		isLD ? 0.75 : float(isLU),
		isLU ? 0.75 : float(isLD),
		isRU ? 0.75 : float(isRD));
	vec4 filter_01 = vec4(
		isRD ? 0.75 : float(isLD),
		isLD ? 0.75 : float(isRD),
		isLU ? 0.75 : float(isRU),
		isRU ? 0.75 : float(isLU));	
	vec4 filter_11 = corner4*0.75;	

#ifdef RESOLUTION_QUARTER
	float finalAlpha = 0.125*(dot(filter_00, alpha4_00)
					+ dot(filter_10, alpha4_10)
					+ dot(filter_01, alpha4_01)
					+ dot(filter_11, alpha4_11));
#else // !RESOLUTION_QUARTER
	float finalAlpha = 0.125*(dot(filter_00, alpha4_00)
					+ dot(filter_10, alpha4_10)
					+ dot(filter_01, alpha4_01)
					+ dot(filter_11, alpha4_11));
#endif // RESOLUTION_QUARTER or not

	gl_FragColor = vec4(finalColor.rgb, finalAlpha);
}