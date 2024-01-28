#version 400 compatibility

uniform vec4 playingStarColor;
uniform float times;
uniform float unit;
uniform float planetNum;
uniform float planetPointAlpha;

out vec4 vertexColor;
out float isAsteroid;

void main()
{
	vec2 vertXY = gl_Vertex.xy;
	vec4 modelPos = vec4(vertXY, 0, 1);

	const float PI = 3.14159265358979;
	float angle = -times*PI*2.0/gl_MultiTexCoord0.x;
	float cosA = cos(angle);
	float sinA = sin(angle);
	mat4 rotateMatrix = mat4(
		cosA, 	-sinA,	0,	0,
		sinA,  cosA,	0,	0,
		0,		0,		1,	0,
		0,		0,		0,	1);
	modelPos = rotateMatrix * modelPos;

	isAsteroid = step(planetNum, gl_VertexID + 0.5);
	float randomAsteroid = abs(sin(float(gl_VertexID)));
	modelPos += vec4(sin(vec3(10.1,10.2,10.3)*gl_VertexID)*vec3(0.1,0.1,0.003)*length(vertXY)*isAsteroid,0.0);

	vec3 modelSunDir = normalize(modelPos.xyz);
	vec3 viewSunDir = normalize((gl_ModelViewMatrix * vec4(modelSunDir,0)).xyz);
	vec3 viewPos = (gl_ModelViewMatrix * modelPos).xyz;
	float lengthV = length(viewPos)*unit*1e-15;
	float planetFalloff = exp2(-lengthV*100.0);
	float asteroidFalloff = exp2(-lengthV*300.0);

	vec3 planetColor = mix(vec3(1), playingStarColor.rgb*(1+planetFalloff)+vec3(0.5), planetPointAlpha);
	vec3 asteroidColor = mix(vec3(0.2), playingStarColor.rgb, 0.3+0.7*asteroidFalloff)
		+ vec3(clamp(1-length(modelPos.xyz)*unit*1.9e-12,0,1));
	asteroidColor *= 0.1+0.9*randomAsteroid;
	vertexColor.rgb = mix(planetColor, asteroidColor, isAsteroid);
	vertexColor.a = mix(1, planetPointAlpha, isAsteroid);

	gl_PointSize = mix((2.5+2.5*planetPointAlpha)*(1+2*planetFalloff), (5+5*randomAsteroid)*(0.2+0.8*asteroidFalloff), isAsteroid);
	gl_Position = gl_ModelViewProjectionMatrix * modelPos;
}