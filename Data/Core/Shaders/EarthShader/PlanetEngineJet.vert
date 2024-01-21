#version 400 compatibility

out vec3 viewPos;
out float dotNV;

void main()
{
	vec4 viewVertex = gl_ModelViewMatrix*gl_Vertex;
	viewPos = viewVertex.xyz / viewVertex.w;
	vec3 viewDir = normalize(viewPos);
	vec3 viewNormal = normalize(gl_NormalMatrix*gl_Normal);
	dotNV = dot(viewNormal, -viewDir);

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}