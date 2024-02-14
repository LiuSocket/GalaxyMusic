#version 400 compatibility

#pragma import_defines(SATURN)

uniform float cloudTop;

#ifdef SATURN
uniform mat4 planetShadowMatrix;
out vec3 shadowVertPos;
#endif // SATURN

out vec2 texCoord_0;
out vec3 texCoord_1;
out vec4 viewPos;
out vec3 viewNormal;

void main()
{
	vec4 modelVertex = gl_Vertex;
	vec3 up = normalize(modelVertex.xyz);
	modelVertex.xyz += up*cloudTop;
	viewPos = gl_ModelViewMatrix*modelVertex;
	viewNormal = normalize(gl_NormalMatrix*gl_Normal);
#ifdef SATURN
	shadowVertPos = (planetShadowMatrix*modelVertex).xyz;
#endif // SATURN

	texCoord_0 = gl_MultiTexCoord0.xy;
	texCoord_1 = gl_MultiTexCoord1.xyz;
	gl_Position = gl_ProjectionMatrix*viewPos;
}