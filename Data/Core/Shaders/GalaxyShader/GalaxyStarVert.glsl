#version 400 compatibility
#pragma import_defines(WELCOME)

uniform vec3 starWorldPos;
uniform vec4 playingStarColor;
uniform float level[128];

out float lengthV;
out vec4 vertexColor;

void main()
{
	lengthV = length((gl_ModelViewMatrix*gl_Vertex).xyz);
	float lenFall = clamp(1.5-lengthV*0.04, 0, 1);

	vertexColor.rgb = gl_Color.rgb;
	vertexColor.a = gl_Color.a*lenFall;
	
	float distancStar = distance(gl_Vertex.xyz, starWorldPos);
	float disRipple = mod(distancStar*10.0, 128.0);

	float starRipple = level[int(disRipple)];
#ifdef WELCOME
	starRipple *= exp(-distancStar*0.1);
	vertexColor.rgb *= (1+starRipple);
#else
	starRipple *= exp(-distancStar*4.0);
	vertexColor.rgb = mix(vertexColor.rgb, 2.0*playingStarColor.rgb, starRipple);
#endif // WELCOME or not

	gl_PointSize = (gl_Color.a + 1.5 + 4.5*exp2(-lengthV*0.5) + starRipple)*lenFall;
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_Position = ftransform();
}