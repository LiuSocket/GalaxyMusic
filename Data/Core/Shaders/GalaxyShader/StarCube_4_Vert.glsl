#version 400 compatibility

uniform mat4 osg_ViewMatrixInverse;
uniform vec3 shapeUVW;
uniform vec2 cubeInfo; // x = cubeMinSize, y = targetDistance
uniform float galaxyHeight;
uniform sampler3D shapeNoiseTex;

out float falloff;
out vec3 vertexColor;

float Remap(float val, float vmin, float vmax, float r0, float r1)
{
	float vr = (clamp(val, vmin, vmax)-vmin)/(vmax-vmin);
	return r0 + vr * (r1-r0); 
}

void main()
{
	float cubeMinSize = cubeInfo.x;
	float targetDistance = cubeInfo.y;
	float cubeSize = gl_Vertex.w;
	vertexColor = gl_Color.rgb;
	vec3 worldFrontDir = -osg_ViewMatrixInverse[2].xyz;
	vec3 worldEyePos = osg_ViewMatrixInverse[3].xyz;
	float notMinCube = step(1.5*cubeMinSize, cubeSize);
	vec4 modelPos = vec4(gl_Vertex.xyz, 1.0);
	vec3 eye2Cube = worldFrontDir*0.5*cubeSize*notMinCube;
	vec3 worldCubePos = worldEyePos + eye2Cube;
	modelPos.xyz += cubeSize*0.11*notMinCube;// 0.11: avoid repetition
	// move to the front to see more vertecis
	vec3 distance2Cube = modelPos.xyz-worldCubePos;
	vec3 cubeeOffset = cubeSize*fract(distance2Cube/cubeSize);
	modelPos.xyz += cubeeOffset;
	modelPos.xyz += eye2Cube - cubeSize*floor((modelPos.xyz+vec3(0.5*cubeSize))/cubeSize);

	float lengthV = length((gl_ModelViewMatrix * modelPos).xyz);
	float lenFall = clamp(2*lengthV/((0.5+0.5*notMinCube)*cubeSize)-1, 0, 1);
	lenFall = 1-lenFall;

	vec3 worldPos = modelPos.xyz + worldEyePos;
	vec3 coord3D = worldPos*shapeUVW;
	float shapeNoise = 1-texture3D(shapeNoiseTex, coord3D).r;
	float fallShape = mix(1.0, Remap(shapeNoise*shapeNoise, 0.0, 0.3, 0.0, 1.0), notMinCube);
	float fallNear = mix(1.0, Remap(targetDistance,cubeMinSize*0.01,cubeMinSize*2.0,0.0,1.0), notMinCube);
	falloff = clamp(1.0-(targetDistance/cubeSize-1.0)/7.0, 0.0, 1.0)*fallNear*fallShape*lenFall;

	gl_PointSize = 10.0*(0.3+0.7*gl_Color.a*lenFall)*(0.2+0.8*fallShape)*(1+9*exp2(-lengthV/3e-4))
		*clamp((galaxyHeight-abs(worldPos.z))/(0.4*galaxyHeight), 0, 1);
	gl_Position = gl_ModelViewProjectionMatrix * modelPos;
}