#version 400 compatibility

uniform vec4 playingStarColor;
uniform float planetTailAlpha;
uniform float planetLineAlpha;

in float fall;

void main()
{
	float alpha = fall*(1-planetLineAlpha)*planetTailAlpha;
	if(alpha<0.003) discard;
	gl_FragColor = vec4(playingStarColor.rgb*(1+fall)+0.5, alpha);
}