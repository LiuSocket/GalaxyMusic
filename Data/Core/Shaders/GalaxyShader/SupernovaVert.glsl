#version 400 compatibility

uniform float level[128];
uniform float supernovaLight;

out vec2 modelPosXY;

void main()
{
	vec4 modelPos = gl_Vertex;
	vec2 signPos = sign(modelPos.xy);
	vec2 absScale = abs(modelPos.xy*gl_Normal.xy);
	modelPos.xy = signPos*max(vec2(2,4), absScale*supernovaLight*(1+level[0]));
	modelPosXY = gl_Vertex.xy*vec2(0.01, 0.02);

	float lenXY = length(modelPosXY); 
	modelPos.xy += 4*exp2(-lenXY*lenXY*2)*signPos*normalize(absScale.yx);

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * modelPos;
}