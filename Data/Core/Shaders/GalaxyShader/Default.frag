#version 330 compatibility

uniform vec3 screenSize;
uniform sampler2D mainTex;

void main()
{
	gl_FragColor = texture2D(mainTex, gl_FragCoord.xy/screenSize.xy);
}