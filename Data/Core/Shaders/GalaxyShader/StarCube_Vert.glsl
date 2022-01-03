#version 400 compatibility

uniform mat4 osg_ViewMatrixInverse;
uniform vec3 centerOffset;
uniform float starDistance;
uniform float unitRatio;

out vec2 falloff;
out vec4 vertexColor;

void main()
{
	float cubeSize = gl_Vertex.w;
	vertexColor = gl_Color;
	vec3 WCP = osg_ViewMatrixInverse[3].xyz;
	vec4 modelPos = vec4(gl_Vertex.xyz, 1.0);

	vec3 distanceXYZ = modelPos.xyz-WCP*unitRatio;
	vec3 offset = cubeSize*fract(distanceXYZ/cubeSize);
	modelPos.xyz += offset-centerOffset;
	modelPos.xyz -= cubeSize*floor((modelPos.xyz+vec3(0.5*cubeSize))/cubeSize);

	float lengthV = length((gl_ModelViewMatrix * modelPos).xyz);
	float modelVertDistance = length(modelPos.xyz);
	vec4 fakeModelPos = vec4(normalize(modelPos.xyz)*min(modelVertDistance, starDistance), 1.0);

	falloff = vec2(
		1-clamp(2*lengthV/(0.5*cubeSize)-1, 0, 1),
		exp2(-lengthV/30));

	gl_PointSize = 10.0 * (0.5+0.5*gl_Color.a*falloff.x)*(1.0+29.0*falloff.y);
	gl_Position = gl_ModelViewProjectionMatrix * fakeModelPos;
}