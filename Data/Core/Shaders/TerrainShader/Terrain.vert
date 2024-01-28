#version 400 compatibility

out vec2 texCoord_0;
out vec3 texCoord_1;
out vec4 viewPos;
out vec3 viewNormal;

void main()
{
	viewNormal = normalize(gl_NormalMatrix*gl_Normal);
	vec4 modelVertex = gl_Vertex;
	viewPos = gl_ModelViewMatrix*modelVertex;

	texCoord_0 = gl_MultiTexCoord0.xy;
	texCoord_1 = gl_MultiTexCoord1.xyz;
	gl_Position = ftransform();
}