#version 400 compatibility

uniform float cloudTop;
uniform vec3 viewLight;
uniform mat4 view2ECEFMatrix;

out vec3 texCoord_1;
out vec4 viewPos;

void main()
{
	texCoord_1 = gl_MultiTexCoord1.xyz;
	vec3 viewNormal = normalize(gl_NormalMatrix*gl_Normal);
	float dotNL = dot(viewNormal, viewLight);

	vec4 modelVertex = gl_Vertex;
	vec3 modelSunDir = (view2ECEFMatrix*vec4(viewLight,0)).xyz;
	modelVertex.rgb -= modelSunDir*cloudTop*(30-29*pow(abs(dotNL),0.2));
	viewPos = gl_ModelViewMatrix*modelVertex;
	gl_Position = gl_ProjectionMatrix*viewPos;
}