#version 400 compatibility

uniform vec4 playingStarColor;
uniform float times;
uniform float planetLineAlpha;

void main() 
{
	float fall = fract(gl_TexCoord[0].x - times/gl_TexCoord[0].y);
	//fall = pow(fall,9);
	gl_FragColor = vec4(playingStarColor.rgb*(1+fall)+0.5, fall*planetLineAlpha);
}