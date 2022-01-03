#version 400 compatibility

uniform mat4 osg_ViewMatrixInverse;
uniform vec3 shapeUVW;
uniform vec2 cubeInfo; // x = cubeMinSize, y = targetDistance
uniform sampler3D shapeNoiseTex;

out vec2 falloff;
out vec4 vertexColor;

float Remap(float val, float vmin, float vmax, float r0, float r1)
{
	float vr = (clamp(val, vmin, vmax)-vmin)/(vmax-vmin);
	return r0 + vr * (r1-r0); 
}

void main()
{
	float cubeSize = gl_Vertex.w;
	vertexColor = gl_Color;
	vec3 WCP = osg_ViewMatrixInverse[3].xyz;
	float isMinCube = step(cubeSize, 1.5*cubeInfo.x);
	vec4 modelPos = vec4(gl_Vertex.xyz+cubeSize*0.11*(1.0-isMinCube), 1.0);

	vec3 distanceXYZ = modelPos.xyz-WCP;
	vec3 offset = cubeSize*fract(distanceXYZ/cubeSize);
	modelPos.xyz += offset;
	modelPos.xyz -= cubeSize*floor((modelPos.xyz+vec3(0.5*cubeSize))/cubeSize);

	float lengthV = length((gl_ModelViewMatrix * modelPos).xyz);
	float lenFall = clamp(2*lengthV/(0.5*cubeSize)-1, 0, 1);
	lenFall = 1-lenFall;

	vec3 worldPos = modelPos.xyz + WCP;
	vec3 coord3D = worldPos*shapeUVW;
	float shapeNoise = 1-texture3D(shapeNoiseTex, coord3D).r;
	float fallShape = mix(Remap(shapeNoise*shapeNoise, 0.0, 0.3, 0.0, 1.0), 1.0, isMinCube);
	float fallNear = mix(Remap(cubeInfo.y,cubeInfo.x*0.01,cubeInfo.x*2.0,0.0,1.0), 1.0, isMinCube);
	falloff = vec2(
		clamp(1.0-(cubeInfo.y/cubeSize-1.0)/7.0, 0.0, 1.0)*fallNear*fallShape*lenFall,
		exp2(-lengthV/3e-4));

	gl_PointSize = 10.0*(0.5+0.5*gl_Color.a*lenFall)*(0.2+0.8*fallShape)*(1.0+29.0*falloff.y);
	gl_Position = gl_ModelViewProjectionMatrix * modelPos;
}