#version 400 compatibility

uniform samplerCube cubeMapTex;
uniform sampler3D shapeNoiseTex;
uniform float times;
uniform float starAlpha;
uniform float myWorldAlpha;

void main()
{
	vec3 texCoord = normalize(gl_TexCoord[0].xyz-vec3(0.5,0.5,0.5));
	vec3 cubeMapColor = textureCube(cubeMapTex, texCoord).rgb;
	float shapeNoise = texture3D(shapeNoiseTex, texCoord*1.4+times*0.01).r;
	vec3 texColor = cubeMapColor*(0.5-0.3*shapeNoise)*starAlpha;
	gl_FragColor = vec4(mix(texColor, vec3(0.8235, 0.7843, 0.6510), myWorldAlpha), 1.0);
}