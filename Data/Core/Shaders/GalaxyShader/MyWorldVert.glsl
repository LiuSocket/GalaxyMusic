#version 400 compatibility

const float M_PI = 3.141592657;

uniform mat4 osg_ViewMatrixInverse;
uniform float myWorldAlpha;

out vec4 vertexColor;

void main()
{
	vertexColor = gl_Color;
	gl_PointSize = (0.3 + 0.7*gl_Color.a) * mix(128, 32, myWorldAlpha);
	gl_Position = ftransform();
}