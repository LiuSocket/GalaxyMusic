#version 400 compatibility

uniform mat4 osg_ViewMatrixInverse;

out vec3 worldPos;

void main()
{
	worldPos = (osg_ViewMatrixInverse*(gl_ModelViewMatrix*gl_Vertex)).xyz;
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_Position = ftransform();
}