#ifdef EARTH

uniform float unit;
uniform float wanderProgress;
uniform vec3 screenSize;
uniform sampler2D baseColorTex;
uniform sampler2D tailTex;

in float diffuse;
in vec3 viewPos;
in vec3 viewVertUp;
in float engineIntensity;

void main()
{
	float lenV = length(viewPos);
	float maxDistance = 8e7/unit;
	if(lenV > maxDistance) discard;

	vec3 viewDir = normalize(viewPos);
	vec3 viewEngineUp = normalize(viewVertUp);
	float vertAlt = max(0, gl_TexCoord[0].z);

	float illumAlt = clamp(vertAlt*unit*1e-4,0,1);
	vec3 streamLight = vec3(0.0,0.4,0.6)*(illumAlt*engineIntensity);
	vec4 baseColor = texture(baseColorTex, gl_TexCoord[0].xy);
	vec4 color = vec4(max(streamLight,vec3(diffuse))*baseColor.rgb, baseColor.a);
	color.a -= clamp(1-(maxDistance-lenV)/(0.9*maxDistance), 0, 1);
	// building progress
	color.a *= step(0, wanderProgress*10-vertAlt*unit*1e-4);

	// radius of sealevel at the vertex point
	float Rs = gl_TexCoord[0].w;
	color.rgb += AtmosColor(vertAlt, viewDir, viewEngineUp, lenV, Rs);

	if((wanderProgress > PROGRESS_3_1) && (unit > 1e6))
	{
		vec4 tailColor = texture(tailTex, gl_FragCoord.xy/screenSize.xy);
		color.rgb = mix(color.rgb, tailColor.rgb, tailColor.a);
	}

	gl_FragColor = vec4(ToneMapping(color.rgb), color.a);
}
#endif // EARTH