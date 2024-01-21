#version 400 compatibility

out vec3 viewModelTailDir;
out vec3 viewModelVertPos;
out vec3 viewPos;
out vec3 viewNormal;

void main()
{
	viewModelTailDir = gl_ModelViewMatrix[2].xyz;
	viewModelVertPos = (gl_ModelViewMatrix*vec4(gl_Vertex.xyz,0)).xyz;
	vec4 viewVertex = gl_ModelViewMatrix*gl_Vertex;
	viewPos = viewVertex.xyz / viewVertex.w;
	viewNormal = normalize(gl_NormalMatrix*gl_Normal);
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}