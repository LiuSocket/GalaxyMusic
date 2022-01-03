#version 400 compatibility

uniform sampler2D supernovaTex;
uniform vec4 playingStarColor;

void main() 
{
	vec4 supernovaColor = texture2D(supernovaTex, gl_TexCoord[0].xy);

	vec2 fall = 2*abs(gl_TexCoord[0].xy-0.5);
	float radius = length(fall);
	float shininess = pow(clamp(1-radius,0,1), 5);

	vec4 mixColor = mix(playingStarColor,vec4(1),shininess);
	gl_FragColor = supernovaColor * (vec4(0.8)+0.4*mixColor);
}