#version 400 compatibility

out vec4 viewPos;
out vec3 viewNormal;

void main()
{
	viewPos = gl_ModelViewMatrix*gl_Vertex;
	viewNormal = gl_NormalMatrix*gl_Normal;
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_Position = ftransform();
}