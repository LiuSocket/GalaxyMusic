#ifdef EARTH

uniform sampler2D baseColorTex;

in float diffuse;
in vec3 viewPos;
in vec3 viewVertUp;
in float engineIntensity;
in float Rg;

void main()
{
	float lenV = length(viewPos)*unit; // meter
	float maxDistance = 8e7; // meter
	if(lenV > maxDistance) discard;

	vec3 viewDir = normalize(viewPos);
	vec3 viewEngineUp = normalize(viewVertUp);
	float alt2Bottom = max(0, gl_TexCoord[0].z); // meter
	float bottomAlt = gl_TexCoord[0].w; // meter
	float vertAlt = alt2Bottom + bottomAlt; // meter

	float illumAlt = clamp(alt2Bottom*1e-4,0,1);
	vec3 streamLight = vec3(0.0,0.08,0.16)*(illumAlt*engineIntensity);
	vec4 baseColor = texture(baseColorTex, gl_TexCoord[0].xy);
	baseColor.rgb *= baseColor.rgb;
	vec4 color = vec4(max(streamLight,vec3(diffuse))*baseColor.rgb, baseColor.a);
	color.a -= clamp(1-(maxDistance-lenV)/(0.9*maxDistance), 0, 1);
	// building progress
	color.a *= step(0, wanderProgress*10-vertAlt*1e-4);
	// atmosphere
	color.rgb += AtmosColor(vertAlt, viewDir, viewEngineUp, Rg);

	vec2 screenCoord = gl_FragCoord.xy/screenSize.xy;
	if((wanderProgress > PROGRESS_3_1) && (unit > 1e6))
	{
		vec3 tailColor = texture(tailColorTex, screenCoord).rgb;
		vec4 tailAlpha4 = texture(tailAlphaTex, screenCoord);
		float tailAlpha = tailAlpha4.a;
		color.rgb = mix(color.rgb, tailColor, tailAlpha);
	}

	color.rgb = ToneMapping(color.rgb);
	color.rgb = pow(color.rgb,vec3(1.0/2.2));
	color.rgb += mix(-NOISE_GRANULARITY, NOISE_GRANULARITY, Random(screenCoord));

	gl_FragColor = color;
}
#endif // EARTH