#version 400 compatibility

out vec4 modelPos;
out vec3 viewPos;
out vec3 viewNormal;

void main()
{
	modelPos = gl_Vertex;
	vec4 viewVertex = gl_ModelViewMatrix*gl_Vertex;
	viewPos = viewVertex.xyz / viewVertex.w;
	viewNormal = normalize(gl_NormalMatrix*gl_Normal);

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}