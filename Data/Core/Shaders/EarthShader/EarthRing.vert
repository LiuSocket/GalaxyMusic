#version 400 compatibility

out vec3 viewModelVertPos;
out vec3 viewPos;

void main()
{
	viewModelVertPos = (gl_ModelViewMatrix*vec4(gl_Vertex.xyz,0)).xyz;
	vec4 viewVertex = gl_ModelViewMatrix*gl_Vertex;
	viewPos = viewVertex.xyz / viewVertex.w;
	
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}