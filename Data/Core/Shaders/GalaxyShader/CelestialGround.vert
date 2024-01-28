#version 400 compatibility
#pragma import_defines(EARTH)

#ifdef EARTH
uniform float unit;
uniform vec4 coordScale_Earth;
uniform sampler2DArray DEMTex;

#endif // EARTH

out vec2 texCoord_0;
out vec3 texCoord_1;
out vec4 viewPos;
out vec3 viewNormal;

float DEM(in float normDEM)
{
	float x = 2*normDEM-1;
	return sign(x)*x*x*1e4;
}

void main()
{
	viewNormal = normalize(gl_NormalMatrix*gl_Normal);
	vec4 modelVertex = gl_Vertex;
	viewPos = gl_ModelViewMatrix*modelVertex;

#ifdef EARTH
	vec3 DEMCoord = gl_MultiTexCoord1.xyz;
	DEMCoord.xy = (DEMCoord.xy - 0.5)*coordScale_Earth.w + 0.5;
	float elev = DEM(texture(DEMTex, DEMCoord).r);
	modelVertex.xyz += max(0, elev/unit)*gl_Normal;
	gl_Position = gl_ModelViewProjectionMatrix*modelVertex;
#else // not EARTH
	gl_Position = ftransform();
#endif // EARTH or not

	texCoord_0 = gl_MultiTexCoord0.xy;
	texCoord_1 = gl_MultiTexCoord1.xyz;

}