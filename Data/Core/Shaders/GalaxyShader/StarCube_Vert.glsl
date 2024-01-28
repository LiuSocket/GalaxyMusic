#version 400 compatibility

uniform mat4 osg_ViewMatrixInverse;
uniform vec3 centerOffset;
uniform float starDistance;
uniform float unit;

out float falloff;
out vec3 vertexColor;

void main()
{
	float cubeSize = gl_Vertex.w;
	vertexColor = gl_Color.rgb;
	vec3 WCP = osg_ViewMatrixInverse[3].xyz;
	vec4 modelPos = vec4(gl_Vertex.xyz, 1.0);

	vec3 distanceXYZ = modelPos.xyz-WCP*unit/1e15;
	vec3 offset = cubeSize*fract(distanceXYZ/cubeSize);
	modelPos.xyz += offset-centerOffset;
	modelPos.xyz -= cubeSize*floor((modelPos.xyz+vec3(0.5*cubeSize))/cubeSize);

	float lengthV = length((gl_ModelViewMatrix * modelPos).xyz);
	float modelVertDistance = length(modelPos.xyz);
	vec4 fakeModelPos = vec4(normalize(modelPos.xyz)*min(modelVertDistance, starDistance), 1.0);

	falloff = 1-clamp(2*lengthV/(0.5*cubeSize)-1, 0, 1);

	gl_PointSize = 20.0*(0.15+0.85*gl_Color.a*falloff)*(1+9*exp2(-lengthV/30));
	gl_Position = gl_ModelViewProjectionMatrix * fakeModelPos;
}