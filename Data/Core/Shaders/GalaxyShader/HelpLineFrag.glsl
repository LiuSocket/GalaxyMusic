#version 400 compatibility

in vec4 vertexColor;

void main() 
{
	gl_FragColor = vertexColor;
}