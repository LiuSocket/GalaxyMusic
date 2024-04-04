#version 400 compatibility

const float M_PI = 3.1415926;

uniform vec3 engineStartRatio;
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
	float startSpeed = 1-0.1*noise;
	vec3 MVU = normalize(gl_Vertex.xyz);
	float lon = abs(atan(MVU.x, MVU.y))/M_PI;
	lon = (engineStartRatio.z > 0.5) ? lon : 1-lon;
	intensity *= max(clamp(30*(MVU.z-1+engineStartRatio.y*startSpeed),0,1),
		clamp(30*(2*engineStartRatio.x*startSpeed-lon),0,1)*clamp((0.2-abs(MVU.z))*10,0,1));
	// for near discard
	float minDistance = 2e6/unit;
	intensity *= (unit < 1e6) ? clamp((lenV-minDistance)/minDistance, 0, 1) : 1.0;

	gl_PointSize = 2 + 6*exp2(-lenV*unit*1e-7);
	gl_Position = ftransform();
}