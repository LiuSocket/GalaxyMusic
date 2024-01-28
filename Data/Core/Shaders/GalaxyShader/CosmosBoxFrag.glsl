#version 400 compatibility

uniform samplerCube cubeMapTex;
uniform sampler3D shapeNoiseTex;
uniform float times;
uniform float starAlpha;
//uniform float myWorldAlpha;

void main()
{
	vec3 texCoord = normalize(gl_TexCoord[0].xyz-vec3(0.5,0.5,0.5));
	vec3 cubeMapColor = textureCube(cubeMapTex, texCoord).rgb;
	vec4 shape4 = texture3D(shapeNoiseTex, texCoord*(0.5+0.1*abs(fract(times*0.1)-0.5)));
	float shapeNoise = (8*shape4.x+4*shape4.y+2*shape4.z+shape4.w)/15.0;
	vec3 texColor = cubeMapColor*(0.1+0.2*shapeNoise)*starAlpha;
	//gl_FragColor = vec4(mix(texColor, vec3(0.8235, 0.7843, 0.6510), myWorldAlpha), 1.0);
	gl_FragColor = vec4(texColor, 1.0);
}