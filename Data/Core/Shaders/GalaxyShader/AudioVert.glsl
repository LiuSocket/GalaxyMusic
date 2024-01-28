#version 400 compatibility
#pragma import_defines(WELCOME)

uniform mat4 osg_ViewMatrixInverse;

uniform vec3 mouseWorldPos;
uniform vec3 starWorldPos;
uniform float level[128];

out vec4 vertexColor;
out float playingStar;

const float M_PI = 3.141592657;

void main()
{
	vec3 WCP = osg_ViewMatrixInverse[3].xyz;
	vec4 viewPos = gl_ModelViewMatrix*gl_Vertex;
	float lengthV = length(viewPos.xyz);
	float lenFall = 1.0-clamp(lengthV*0.01,0.0,1.0);
	lenFall *= lenFall;

	float distanceMouse = distance(gl_Vertex.xyz, mouseWorldPos);
	float distanceStar = distance(gl_Vertex.xyz, starWorldPos);

	float disRipple = mod(distanceStar*10.0, 128.0);
#ifdef WELCOME
	float dissipate = 3*exp(-distanceStar*0.2);
#else
	float dissipate = exp(-distanceStar*0.5);
#endif // WELCOME or not
	float starRipple = level[int(disRipple)]*dissipate;
	playingStar = step(-0.001,-distanceStar);
	float mouseSelect = 1 - step(1, distanceMouse*mix(2,6,exp2(-abs(WCP.z))));	
	float ripple = min(1.0,max(mouseSelect, starRipple));

	vertexColor.rgb = 3*mix(vec3(1.0), gl_Color.rgb, max(playingStar,ripple));
	vertexColor.a = 2*lenFall;

	gl_PointSize = max(14*playingStar*(exp2(-lengthV*5.0)+1)*lenFall,16*mouseSelect*exp2(-lengthV*0.02)) +
		(11*starRipple+1)*(2*exp2(-lengthV*8.0)+2)*lenFall;
	gl_Position = ftransform();
}