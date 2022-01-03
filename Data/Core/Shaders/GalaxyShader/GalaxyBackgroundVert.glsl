#version 400 compatibility

uniform vec3 eyePos4;

void main()
{
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

	vec2 eyeXYDir = normalize(eyePos4.xy);
	float cosPos = -eyeXYDir.x;
	float sinPos = -eyeXYDir.y;	
	mat4 rotateMatrix = mat4(
		cosPos, sinPos,	0,	0,
		-sinPos,  cosPos,	0,	0,
		0,		0,			1,	0,
		0,		0,			0,	1);

	vec4 rotatePos = rotateMatrix*gl_Vertex;
	gl_Position = gl_ModelViewProjectionMatrix * rotatePos;
}