#version 400 compatibility

uniform float backgroundSunScale;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	vec4 modelVertex = gl_Vertex;
	modelVertex.xyz *= backgroundSunScale;
	gl_Position = gl_ModelViewProjectionMatrix * modelVertex;
}