#version 400 compatibility

uniform sampler2D blueNoiseTex;
uniform float times;
uniform float unit;

in vec3 viewPos;

void main()
{
	float lenV = length(viewPos);
	float maxDistance = 4e6/unit;
	if(lenV > maxDistance) discard;

	float fade = gl_TexCoord[0].y;	
	float wave = 0.9 + 0.1*sin(gl_TexCoord[0].y*30 + times*(10+10*gl_TexCoord[0].z));
	float intensity = gl_TexCoord[0].z*wave;
	vec3 color = mix(vec3(0.0, 0.3, 1.0), vec3(0.4, 0.7, 1.0), fade);

	vec2 coord = gl_TexCoord[0].xy*vec2(1.0, 0.02*gl_TexCoord[0].w);
	coord.y += times*(0.4+0.2*gl_TexCoord[0].z);
	float noise = texture(blueNoiseTex, coord).r;

	float alpha = fade*(0.4+0.6*intensity)*(0.4+0.6*noise)*clamp((maxDistance-lenV)*4/maxDistance, 0, 1)
		*(4*gl_TexCoord[0].x*(1-gl_TexCoord[0].x));
	gl_FragColor = vec4(color*alpha, alpha);
}