#version 400 compatibility

out vec3 viewPos;

void main()
{
	vec4 viewVertex = gl_ModelViewMatrix*gl_Vertex;
	viewPos = viewVertex.xyz / viewVertex.w;

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}