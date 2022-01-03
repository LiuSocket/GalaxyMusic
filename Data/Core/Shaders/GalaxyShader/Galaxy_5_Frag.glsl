#version 400 compatibility

uniform sampler2D galaxyTex;

void main() 
{
	gl_FragColor = texture2D(galaxyTex, gl_TexCoord[0].xy);
}