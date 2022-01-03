#version 400 compatibility

out vec3 viewPos;

void main()
{
	viewPos = (gl_ModelViewMatrix*gl_Vertex).xyz;
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_Position = ftransform();
}