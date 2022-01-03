#version 400 compatibility
#pragma import_defines(HIGH_QUALITY)

#ifdef HIGH_QUALITY
uniform sampler2D distanceTex;
uniform vec3 screenSize;
#endif // HIGH_QUALITY

uniform float starAlpha;

in float lengthV;
in vec4 vertexColor;

void main() 
{
	float pointScale = 1.0;
#ifdef HIGH_QUALITY
	vec2 projCoord = gl_FragCoord.xy/screenSize.xy;
	// x = surface distance, y = deep distance, z = deep alpha
	vec3 occ = texture2D(distanceTex, projCoord).xyz;
	float lenStart = occ.x;
	float lenStop = lenStart + (occ.y-occ.x)/max(1e-6,occ.z);
	pointScale = clamp((lenStop-lengthV)/max(1e-6,lenStop-lenStart),0,1);
	pointScale *= pointScale;
#endif // HIGH_QUALITY

	vec2 fall = 2*abs(gl_PointCoord.st-0.5);
	float radius = length(fall);
	radius = clamp(1-radius,0,1);
	radius *= radius;

	gl_FragColor = vertexColor*radius*pointScale*starAlpha;
}