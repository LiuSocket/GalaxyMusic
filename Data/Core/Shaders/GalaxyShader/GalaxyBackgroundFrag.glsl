#version 400 compatibility

uniform sampler2D galaxyBackgroundTex;
uniform sampler2D noiseTex;
uniform float starAlpha;

void main()
{
	vec4 color = texture(galaxyBackgroundTex, gl_TexCoord[0].st);
	float brightness = (color.r+color.g+color.b)*0.3333333;

	vec4 starNoise = texture2D(noiseTex, gl_TexCoord[0].st*vec2(83.2,20.8));
	vec4 tmpV = clamp((vec4(0.25,0.5,0.75,1.0)-brightness)*4,0,1);
	tmpV = max(starNoise-tmpV,0);
	float alpha = (0.3+0.15*(tmpV.r+tmpV.g+tmpV.b+tmpV.a))*(1-starAlpha);
	color.a *= alpha;
	gl_FragColor = color;
}