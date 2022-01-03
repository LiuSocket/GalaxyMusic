#version 400 compatibility

uniform float times;
uniform float unitRatio;

out vec4 vertexColor;

void main()
{
	vec4 modelPos = vec4(gl_Vertex.xy, 0, 1);

	const float PI = 3.14159265358979;
	const float PI_2 = PI*2.0;
	float angle = times*PI_2*0.05/gl_Vertex.w;
	float cosA = cos(angle);
	float sinA = sin(angle);
	mat4 rotateMatrix = mat4(
		cosA, 	sinA,	0,	0,
		-sinA,  cosA,	0,	0,
		0,		0,		1,	0,
		0,		0,		0,	1);
	modelPos = rotateMatrix * modelPos;

	float lengthV = length((gl_ModelViewMatrix * modelPos).xyz)*unitRatio;
	float falloff = exp2(-lengthV*100.0);
	vertexColor = vec4(vec3(0.0, 1.0, 0.5)*(1+falloff), 1.0);

	gl_PointSize = 8.0*(1+2*falloff);
	gl_Position = gl_ModelViewProjectionMatrix * modelPos;
}