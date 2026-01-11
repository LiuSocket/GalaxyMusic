#version 400 compatibility

uniform float level[128];
uniform float supernovaLight;
uniform float supernovaWidth;

out vec2 modelPosXY;

void main()
{
	vec4 modelPos = gl_Vertex;
	vec2 absScale = abs(gl_Normal.xy);
	// change the length
	modelPos.xy *= mix(vec2(1), vec2(supernovaLight*(1+2*abs(level[1]-level[0]))), absScale);
	// change the width
	modelPos.xy *= mix(vec2(1), vec2(supernovaWidth), absScale.yx);

	modelPosXY = gl_Vertex.xy*vec2(0.01, 0.02);
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * modelPos;
}