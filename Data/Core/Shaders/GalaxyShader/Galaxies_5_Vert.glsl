#version 400 compatibility

uniform mat4 osg_ViewMatrixInverse;
uniform vec2 cubeInfo; // x = cubeMinSize, y = targetDistance

out float coordW;
out vec3 spin;
out vec4 vertexColor;
out float falloff;

float Remap(float val, float vmin, float vmax, float r0, float r1)
{
	float vr = (clamp(val, vmin, vmax)-vmin)/(vmax-vmin);
	return r0 + vr * (r1-r0); 
}

void main()
{
	float cubeSize = gl_Vertex.w;
	vertexColor = vec4(clamp(vec3(2)-abs(vec3(1,2,3)-vec3(gl_Color.r)),0,1),gl_Color.a);
	vec3 WCP = osg_ViewMatrixInverse[3].xyz;
	float isMinCube = step(cubeSize, 1.5*cubeInfo.x);
	// 3.141,5.926,0.5358 is just PI, no meaning
	vec4 modelPos = vec4(gl_Vertex.xyz + vec3(3.141,5.926,0.5358), 1.0);
	vec3 distanceXYZ = modelPos.xyz-WCP;
	vec3 offset = cubeSize*fract(distanceXYZ/cubeSize);
	modelPos.xyz += offset;
	modelPos.xyz -= cubeSize*floor((modelPos.xyz+vec3(0.5*cubeSize))/cubeSize);

	modelPos = mix(modelPos, vec4(gl_Vertex.xyz, 1), isMinCube);

	float lengthV = length((gl_ModelViewMatrix * modelPos).xyz);
	float lenFall = mix(1.0,0.75,isMinCube)*clamp(1-lengthV/(0.5*cubeSize), 0.0, 1.0);

	float fallNear = mix(Remap(cubeInfo.y,cubeInfo.x*0.01,cubeInfo.x*2.0,0.0,1.0), 1.0, isMinCube);
	falloff = clamp(1.0-(cubeInfo.y/cubeSize-1.0)/3.0, 0.0, 1.0)*fallNear*lenFall;

	coordW = gl_Color.z;
	spin = step(-vec3(0.5,1.5,2.5),-vec3(gl_Color.y));
	gl_PointSize = 256.0*(0.2+0.8*gl_Color.a)*(0.15+0.85*pow(lenFall,30))*(0.1+0.9*lenFall);
	gl_Position = gl_ModelViewProjectionMatrix * modelPos;
}