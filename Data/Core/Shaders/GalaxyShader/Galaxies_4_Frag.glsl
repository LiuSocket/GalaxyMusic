#version 400 compatibility
#extension GL_EXT_texture_array : enable

uniform sampler2DArray galaxiesTex;
uniform float starAlpha;

in float falloff;
in float coordW;
in vec3 spin;
in vec4 vertexColor;

void main() 
{
	vec2 coord = gl_PointCoord.st;
	vec2 texUV = mix(coord, vec2(coord.t,-coord.s), spin.x);
	texUV = mix(texUV, -coord, spin.y);
	texUV = mix(texUV, vec2(-coord.t,coord.s), spin.z);
	vec4 galaxiesColor = texture2DArray(galaxiesTex, vec3(texUV, coordW));

	vec2 fall = 2*abs(coord-0.5);
	float radius = length(fall);
	radius = clamp(1-radius,0,1);
	radius *= radius; radius *= radius;

	gl_FragColor = starAlpha*mix(
		(vec4(2)+vertexColor)*2*falloff*radius,
		galaxiesColor*(vec4(0.9) + 0.2*vertexColor),
		min(falloff + 0.2, 1.0));
}