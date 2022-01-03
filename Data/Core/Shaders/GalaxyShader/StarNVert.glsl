#version 400 compatibility

uniform mat4 osg_ViewMatrixInverse;
uniform vec3 starWorldPos;
uniform vec4 playingStarColor;
uniform float level[128];
uniform float galaxyRadius;
uniform sampler2D galaxyTex;

out float lengthV;
out vec4 vertexColor;

void main()
{
	vertexColor = vec4(1);
	float scaleR = gl_MultiTexCoord0.x;
	float scaleRadius = galaxyRadius*2.0/scaleR;
	vec3 WCP = osg_ViewMatrixInverse[3].xyz;
	vec4 modelPos = vec4(gl_Vertex.xyz/vec3(scaleR,scaleR,1.0), gl_Vertex.w);
	vec2 distanceXY = modelPos.xy-WCP.xy;
	vec2 centerPos = scaleRadius*floor(distanceXY/scaleRadius);
	vec2 offset = scaleRadius*floor((distanceXY-centerPos)/(0.5*scaleRadius));
	modelPos.xy -= centerPos + offset;

	float radiusFall = max(0,1-length(modelPos.xy)/(1.5*galaxyRadius));
	vec4 starColor = texture2D(galaxyTex, clamp(modelPos.xy/(2*galaxyRadius)+vec2(0.5),0,1));
	float heightFall = abs(gl_MultiTexCoord0.z-0.5)*mix(4.0,2.0,radiusFall*starColor.a);
	vertexColor.rgb = (vec3(0.1)+3*starColor.rgb*starColor.rgb)
		*max(0.0,(radiusFall-heightFall+gl_MultiTexCoord0.w));

	float distancStar = distance(modelPos.xyz, starWorldPos);
	float disRipple = mod(distancStar*800.0, 128.0);
	float dissipate = exp(-distancStar*10.0);
	lengthV = length((gl_ModelViewMatrix * modelPos).xyz);
	float lenFall = clamp(1-lengthV*scaleR/galaxyRadius, 0, 1);
	float starRipple = level[int(disRipple)]*dissipate;
	vertexColor.rgb = mix(vertexColor.rgb, 2*playingStarColor.rgb, starRipple) * lenFall;
	
	gl_PointSize = (3*gl_MultiTexCoord0.w + starRipple)*6.0 + 4.0;
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * modelPos;
}