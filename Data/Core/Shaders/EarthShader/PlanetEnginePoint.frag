#version 400 compatibility

uniform vec3 screenSize;
uniform sampler2D tailTex;
uniform float unit;

in float noise;
in float intensity;

void main()
{
	if(intensity < 0.001) discard;

	vec2 fall = 2 * abs(gl_PointCoord.st - 0.5);
	float radius = clamp(1 - length(fall), 0, 1);
	float alpha = intensity * radius;

	if(unit > 1e6)
	{
		float tailAlpha = texture(tailTex, gl_FragCoord.xy/screenSize.xy).a;
		alpha *= 1-tailAlpha;
	}

	gl_FragColor = vec4(0.7, 0.7+0.3*noise, 0.95, 1.0)*alpha;
}