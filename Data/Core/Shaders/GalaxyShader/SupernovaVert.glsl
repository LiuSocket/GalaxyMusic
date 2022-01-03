#version 400 compatibility

uniform float level[128];
uniform float supernovaLight;

void main()
{
	vec4 modelPos = gl_Vertex;
	modelPos.xy = sign(modelPos.xy)*max(vec2(2), abs(modelPos.xy)*supernovaLight*(0.6+1.4*level[0]));
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * modelPos;
}