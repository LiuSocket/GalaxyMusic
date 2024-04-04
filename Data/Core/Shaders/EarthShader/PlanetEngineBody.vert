#version 400 compatibility

const float M_PI = 3.1415926;

uniform vec3 viewLight;
uniform vec3 engineStartRatio;

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
	vec3 MVU = normalize(gl_Vertex.xyz);
	float lon = abs(atan(MVU.x, MVU.y))/M_PI;
	lon = (engineStartRatio.z > 0.5) ? lon : 1-lon;
	engineIntensity = max(clamp(20*(MVU.z-1+(engineStartRatio.y-0.04)),0,1),
		clamp(20*(2*(engineStartRatio.x-0.05)-lon),0,1)*clamp((0.2-abs(MVU.z))*10,0,1));

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}