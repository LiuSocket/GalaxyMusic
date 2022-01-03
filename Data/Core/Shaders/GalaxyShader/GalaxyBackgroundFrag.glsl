#version 400 compatibility

uniform sampler2D galaxyBackgroundTex;
uniform sampler2D noiseTex;
uniform float starAlpha;

void main()
{
	vec3 color = texture2D(galaxyBackgroundTex, gl_TexCoord[0].st).rgb;
	float brightness = (color.r+color.g+color.b)*0.3333333;

	vec4 starNoise = texture2D(noiseTex, gl_TexCoord[0].st*vec2(40,10));
	vec4 tmpV = clamp((vec4(0.25,0.5,0.75,1.0)-brightness)*4,0,1);
	tmpV = max(starNoise-tmpV,0);
	color *= (0.75+0.25*(tmpV.r+tmpV.g+tmpV.b+tmpV.a))*(1-starAlpha);
	gl_FragColor = vec4(color, 1.0);
}