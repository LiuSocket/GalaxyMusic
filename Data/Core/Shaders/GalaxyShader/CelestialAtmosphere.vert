#version 400 compatibility
#pragma import_defines(SATURN)

uniform float atmosHeight;

#ifdef SATURN
uniform mat4 planetShadowMatrix;
out vec3 shadowVertPos;
#endif // SATURN

out vec4 viewPos;
out vec3 viewNormal;

void main()
{
	vec4 modelVertex = gl_Vertex;
	vec3 modelVertDir = normalize(modelVertex.xyz);
	modelVertex.xyz += modelVertDir * atmosHeight;
	viewPos = gl_ModelViewMatrix*modelVertex;
	viewNormal = normalize(gl_NormalMatrix*gl_Normal);

#ifdef SATURN
	shadowVertPos = (planetShadowMatrix*modelVertex).xyz;
#endif // SATURN

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ProjectionMatrix*viewPos;
}