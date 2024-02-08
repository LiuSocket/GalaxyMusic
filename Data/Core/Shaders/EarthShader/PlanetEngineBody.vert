#version 400 compatibility

uniform vec3 viewLight;
uniform float engineStartRatio;

out float diffuse;
out vec3 viewPos;
out vec3 viewVertUp;
out float engineIntensity;

void main()
{
	vec4 viewVertex = gl_ModelViewMatrix*gl_Vertex;
	viewPos = viewVertex.xyz / viewVertex.w;

	viewVertUp = gl_NormalMatrix*normalize(gl_Vertex.xyz/gl_Vertex.w);
	diffuse = dot(viewLight, viewVertUp);
	diffuse = smoothstep(-0.4, 1.0, diffuse);

	// for start
	vec3 modelVertUp = normalize(gl_Vertex.xyz);
	engineIntensity = clamp(
		20*(max(abs(modelVertUp.z), 0.4) + 0.05*modelVertUp.x - 1 + engineStartRatio),
		0, 1);

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}