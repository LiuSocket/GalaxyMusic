#version 400 compatibility

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

	// for near discard
	float minDistance = 2e6/unit;
	intensity *= (unit < 1e6) ? clamp((lenV-minDistance)/minDistance, 0, 1) : 1.0;

	gl_PointSize = 2 + 6*exp2(-lenV*unit*1e-7);
	gl_Position = ftransform();
}