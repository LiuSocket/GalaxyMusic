#version 400 compatibility

out vec3 worldPos;
out vec4 viewPos;

void main()
{
	worldPos = gl_Vertex.xyz;
	viewPos = gl_ModelViewMatrix*gl_Vertex;
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_Position = ftransform();
}