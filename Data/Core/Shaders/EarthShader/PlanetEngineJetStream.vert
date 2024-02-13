#version 400 compatibility

const float M_PI = 3.1415926;

uniform vec2 engineStartRatio;

out vec3 viewPos;
out float jetLength;// [0.0, 1.0]

void main()
{
	vec4 viewVertex = gl_ModelViewMatrix*gl_Vertex;
	viewPos = viewVertex.xyz / viewVertex.w;
	
	float startSpeed = 1-0.1*gl_MultiTexCoord0.z;
	vec3 MVU = normalize(gl_Vertex.xyz);
	float lon = abs(atan(MVU.x, MVU.y))/M_PI;
	jetLength = max(clamp(20*(MVU.z-1+(engineStartRatio.y-0.05)*startSpeed),0,1),
		clamp(20*(2*(engineStartRatio.x-0.05)*startSpeed-lon),0,1)*clamp((0.2-abs(MVU.z))*10,0,1));

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}