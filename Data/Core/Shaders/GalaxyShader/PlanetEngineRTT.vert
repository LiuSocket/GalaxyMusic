#version 400 compatibility

void main()
{
	gl_PointSize = 20*gl_MultiTexCoord0.y;
	gl_Position = ftransform();
}