#version 400 compatibility

uniform vec3 screenSize;
uniform sampler2D tailTex;
uniform float times;
uniform float unit;

in vec3 viewPos;
in float dotNV;
in float jetLength;

void main()
{
	float lenV = length(viewPos);
	float minDistance = 2e6/unit;
	if((unit < 1e6 && lenV < minDistance) || jetLength < 1e-7) discard;

	float alpha = gl_TexCoord[0].x*clamp(5*(gl_TexCoord[0].x-1+jetLength),0,1);

	float wave = 0.9 + 0.1*sin(gl_TexCoord[0].x*30 + times*(10+10*gl_TexCoord[0].y));
	float intensity = gl_TexCoord[0].y*wave;
	vec3 color = mix(vec3(0.0, 0.3, 1.0), vec3(0.4, 0.7, 1.0), mix(alpha, 1, pow(abs(dotNV),50)));
	color *= vec3(1,0.7+0.3*intensity,1);
	float jetAlpha = alpha*(0.4+0.6*intensity)*(0.5+0.5*step(1e6, unit));

	if(unit > 1e6)
	{
		float tailAlpha = texture(tailTex, gl_FragCoord.xy/screenSize.xy).a;
		float tailMask = 1 - 0.8*tailAlpha;
		gl_FragColor = vec4(color, 1.0)*jetAlpha*tailMask;
	}
	else
	{
		jetAlpha *= clamp((lenV-minDistance)/minDistance, 0, 1);
		gl_FragColor = vec4(color, 1.0) * jetAlpha;
	}
}