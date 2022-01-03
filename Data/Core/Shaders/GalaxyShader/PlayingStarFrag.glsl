#version 400 compatibility

uniform vec4 playingStarColor;

in vec4 viewPos;
in vec3 viewNormal;

void main() 
{
	vec3 viewDir = normalize(viewPos.xyz);
	float density = 1-gl_TexCoord[0].p;
	float alphaFresnel = dot(normalize(viewNormal),viewDir);
	float alpha = 0.3*mix(alphaFresnel*alphaFresnel, 1, density*density)*density;
	gl_FragColor = vec4(playingStarColor.rgb, alpha);
}