#version 330 compatibility

uniform mat4 osg_ViewMatrixInverse;

out vec4 viewPos;
out vec3 worldPos;

void main()
{
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_Position = ftransform();
	viewPos = gl_ModelViewMatrix*gl_Vertex;
	worldPos = (osg_ViewMatrixInverse*viewPos).xyz;
}