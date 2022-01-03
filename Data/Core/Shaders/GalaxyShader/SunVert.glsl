#version 400 compatibility

uniform mat4 osg_ViewMatrixInverse;

out vec4 modelPos;
out vec3 worldPos;
out vec3 viewPos;
out vec3 viewNormal;

void main()
{
	vec4 viewVertex = gl_ModelViewMatrix*gl_Vertex;
	modelPos = gl_Vertex;
	worldPos = (osg_ViewMatrixInverse*viewVertex).xyz;
	viewPos = viewVertex.xyz / viewVertex.w;
	viewNormal = normalize(gl_NormalMatrix*gl_Normal);

	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_Position = ftransform();
}