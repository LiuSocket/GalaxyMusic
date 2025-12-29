#version 400 compatibility

uniform vec3 screenSize;

uniform sampler2D colorTex;
uniform sampler2D alphaTex;

void main()
{
	vec2 uv = gl_FragCoord.xy/screenSize.xy;
	vec3 finalColor = texture(colorTex, uv).rgb;
	vec4 alpha4 =  texture(alphaTex, uv);
	gl_FragColor = vec4(finalColor, alpha4.x);
}