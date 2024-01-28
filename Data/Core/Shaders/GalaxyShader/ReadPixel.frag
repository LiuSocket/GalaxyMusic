#version 400 compatibility

uniform sampler2D inputTex;

void main()
{
	gl_FragColor = texture2D(inputTex, gl_TexCoord[0].st);
}