#version 400 compatibility

uniform mat4 osg_ViewMatrixInverse;

out float falloff;
out float coordW;
out vec3 spin;
out vec4 vertexColor;

void main()
{
	falloff = gl_Vertex.w;
	vertexColor = vec4(clamp(vec3(2)-abs(vec3(1,2,3)-vec3(gl_Color.r)),0,1),gl_Color.a);

	coordW = gl_Color.z;
	spin = step(-vec3(0.5,1.5,2.5),-vec3(gl_Color.y));
	gl_PointSize = 256.0*(0.2+0.8*gl_Color.a)*(0.15+0.85*pow(falloff,30))*(0.1+0.9*falloff);
	gl_Position = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz,1);
}