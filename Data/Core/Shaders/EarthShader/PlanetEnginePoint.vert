#version 400 compatibility

const float M_PI = 3.1415926;

uniform float engineStartRatio;
uniform float unit;

out float noise;
out float intensity;

void main()
{
	vec4 viewVertex = gl_ModelViewMatrix*gl_Vertex;
	vec3 viewPos = viewVertex.xyz / viewVertex.w;
	vec3 viewDir = normalize(viewPos);
	float lenV = length(viewPos);
	vec3 viewNormal = normalize(gl_NormalMatrix*gl_Normal);
	noise = gl_MultiTexCoord0.x;
	intensity = min(1, 1.5*noise*max(0, dot(viewNormal,-viewDir))*exp2(-lenV*unit*1e-8));
	// for start
	vec3 modelVertUp = normalize(gl_Vertex.xyz);
	intensity *= clamp(
		10*(max(abs(modelVertUp.z), 0.4) + 0.05*cos(modelVertUp.z*1.5*M_PI)*modelVertUp.x - 1 + engineStartRatio),
		0, 1);
	// for near discard
	float minDistance = 2e6/unit;
	intensity *= (unit < 1e6) ? clamp((lenV-minDistance)/minDistance, 0, 1) : 1.0;

	gl_PointSize = 2 + 6*exp2(-lenV*unit*1e-7);
	gl_Position = ftransform();
}