#version 400 compatibility

#pragma import_defines(EDIT)
#pragma import_defines(CAPTURE)

uniform sampler2D galaxyTex;
uniform vec3 mouseWorldPos;
uniform float level[128];

in vec3 worldPos;
in vec4 viewPos;

#ifdef EDIT

#endif // EDIT

void main() 
{
	vec2 texCoord = gl_TexCoord[0].xy;

#ifdef EDIT
	vec3 starVec = worldPos-mouseWorldPos;
	vec2 starDir = normalize(starVec.xy);
	float distancStar = length(starVec);
	float disRipple = mod(distancStar*10.0, 128.0);
	float starRipple = 0.05*exp(-distancStar*5.0);
#ifdef CAPTURE	
	starRipple = 0.02*sqrt(level[int(disRipple)])*exp(-distancStar);
#endif // CAPTURE
	texCoord -= starRipple*starDir;
#endif // EDIT

	vec4 starColor = texture2D(galaxyTex, texCoord);
	starColor.a *= 1-exp2(-length(viewPos.xyz)*0.1);

	gl_FragColor = starColor;
}