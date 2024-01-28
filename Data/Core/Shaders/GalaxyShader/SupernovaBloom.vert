#version 400 compatibility

out vec2 modelPosXY;

void main()
{
	vec4 modelPos = gl_Vertex;
	modelPosXY = gl_Vertex.xy*vec2(0.005,0.0065);
	gl_Position = gl_ModelViewProjectionMatrix * modelPos;
}