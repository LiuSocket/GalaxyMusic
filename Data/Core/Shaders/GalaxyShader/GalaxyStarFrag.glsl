#version 400 compatibility
#pragma import_defines(HIGH_QUALITY)

#ifdef HIGH_QUALITY
uniform sampler2D distanceTex;
uniform vec3 screenSize;
#endif // HIGH_QUALITY

uniform mat4 osg_ViewMatrixInverse;
uniform float starAlpha;

in float lengthV;
in vec4 vertexColor;

void main() 
{
	float pointScale = 1.0;

#ifdef HIGH_QUALITY

	vec3 WCP = osg_ViewMatrixInverse[3].xyz;
	float lenWCP = length(WCP);

	vec2 projCoord = gl_FragCoord.xy/screenSize.xy;
	// xyz = start length, w = alpha
	vec4 occ = texture2D(distanceTex, projCoord);
	float lenStart = lenWCP + (occ.x-0.5)*10.0 + (occ.y+occ.z/255.0)/25.5;
	float lenInto = 1/max(1e-6, occ.w*occ.w);
	pointScale = clamp((lenStart + lenInto - lengthV) / max(0.01, lenInto),0,1);

#endif // HIGH_QUALITY

	vec2 fall = 2*abs(gl_PointCoord.st-0.5);
	float radius = clamp(1-length(fall),0,1);

	gl_FragColor = vec4(vertexColor.rgb, vertexColor.a*radius*pointScale*starAlpha);
}