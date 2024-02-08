#version 400 compatibility

const float M_PI = 3.1415926;

uniform float engineStartRatio;

out vec3 viewPos;
out float jetLength;// [0.0, 1.0]

void main()
{
	vec4 viewVertex = gl_ModelViewMatrix*gl_Vertex;
	viewPos = viewVertex.xyz / viewVertex.w;
	
	vec3 modelVertUp = normalize(gl_Vertex.xyz);
	jetLength = clamp(
		10*(max(abs(modelVertUp.z), 0.4) + 0.05*cos(modelVertUp.z*1.5*M_PI)*modelVertUp.x - 1 + engineStartRatio),
		0, 1);

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}