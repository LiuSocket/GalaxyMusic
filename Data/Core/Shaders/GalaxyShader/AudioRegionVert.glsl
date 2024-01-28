#version 400 compatibility

out vec3 vertWorldPos;
out vec4 vertexColor;

void main()
{
	const float alphaMax = 0.25;
	const float angleFadeWidth = 0.04;
	const float maxBPMFadeWidth = 0.05;
	const float minBPMFadeWidth = 0.3;
	vec2 alphaXY = min(gl_MultiTexCoord0.xy,vec2(angleFadeWidth, maxBPMFadeWidth))*
		min(1-gl_MultiTexCoord0.xy,vec2(angleFadeWidth, minBPMFadeWidth));
	vertexColor.a = max(0, alphaXY.x*alphaXY.y*alphaMax/
		(angleFadeWidth*angleFadeWidth*maxBPMFadeWidth*minBPMFadeWidth));

	vertexColor.rgb = gl_Color.rgb;
	vertWorldPos = gl_Vertex.xyz;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}