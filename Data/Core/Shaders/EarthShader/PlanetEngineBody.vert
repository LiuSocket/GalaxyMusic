#version 400 compatibility

uniform vec3 viewLight;

out float diffuse;
out vec3 viewPos;
out vec3 viewVertUp;

void main()
{
	vec4 viewVertex = gl_ModelViewMatrix*gl_Vertex;
	viewPos = viewVertex.xyz / viewVertex.w;

	viewVertUp = gl_NormalMatrix*normalize(gl_Vertex.xyz/gl_Vertex.w);
	diffuse = dot(viewLight, viewVertUp);
	diffuse = smoothstep(-0.4, 1.0, diffuse);

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}