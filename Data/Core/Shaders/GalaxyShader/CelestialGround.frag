const float PROGRESS_0 = 0.1;

uniform sampler2DArray baseTex;
uniform vec2 planetRadius;

#ifdef EARTH

uniform float unit;
uniform vec3 screenSize;
uniform vec4 coordScale_Earth;
uniform sampler2DArray DEMTex;
uniform sampler2DArray illumTex;
uniform sampler2D globalShadowTex;

#ifdef WANDERING
uniform sampler2D tailTex;
uniform float engineIntensity;
#endif // WANDERING

#else // not EARTH
uniform vec4 coordScale;
#endif // EARTH

in vec2 texCoord_0;
in vec3 texCoord_1;
in vec4 viewPos;
in vec3 viewNormal;

float DEM(in float normDEM)
{
	float x = 2*normDEM-1;
	return sign(x)*x*x*1e4;
}

// lat:[-1.0, 1.0]
float SeaLevel(in float lat, in float seaLevel_66_Progress)
{
	// [0.0,0.1] seaLevel + 66m
	float seaLevel = 66*seaLevel_66_Progress;
	seaLevel = mix(seaLevel, clamp(abs(lat), 0, 1)*15000-9000, smoothstep(0.3, 0.8, wanderProgress));
	return seaLevel;
}

void main()
{
	vec4 celestialCoordScale = vec4(1,1,1,1);
#ifdef EARTH
	celestialCoordScale = coordScale_Earth;
#else // not EARTH
	celestialCoordScale = coordScale;
#endif // EARTH

	vec3 baseCoord = texCoord_1;
	baseCoord.xy = (baseCoord.xy - 0.5)*celestialCoordScale.x + 0.5;

	vec4 baseColor = texture(baseTex, baseCoord);
	vec3 viewVertUp = normalize(viewNormal);
	vec3 viewDir = normalize(viewPos.xyz);

	const float minFact = 1e-8;
	float dotVUL = dot(viewVertUp, viewLight);
	vec3 diffuse = vec3(max(dotVUL,minFact));
	vec3 color = baseColor.rgb * (0.02+diffuse);

	float vertAlt = 0;

#ifdef EARTH
	vec3 illumCoord = texCoord_1;
	illumCoord.xy = (illumCoord.xy - 0.5)*celestialCoordScale.y + 0.5;
	vec4 illum = texture(illumTex, illumCoord);
	vec3 illumCity = illum.rgb*(vec3(1)-smoothstep(0.0, 0.1, diffuse));
	float rockMask = 1 - baseColor.a;

	vec3 viewHalf = normalize(viewLight - viewDir);
	float dotNH = max(dot(viewVertUp, viewHalf), 0);
#ifdef WANDERING
	float seaLevel_66_Progress = min(1, wanderProgress/PROGRESS_0);
	vec3 wanderingBaseCoord = baseCoord;
	wanderingBaseCoord.z += 6;
	vec4 wanderingColor = texture(baseTex, wanderingBaseCoord);
	baseColor.rgb = mix(baseColor.rgb, wanderingColor.rgb, seaLevel_66_Progress);
	vec3 ambient = vec3(0.07,0.11,0.15)*exp2(min(0, texCoord_0.y-0.48)*25);

	vec3 DEMCoord = texCoord_1;
	DEMCoord.xy = (DEMCoord.xy - 0.5)*celestialCoordScale.w + 0.5;
	float elev = DEM(texture(DEMTex, DEMCoord).r);
	vertAlt = elev/unit;
	float lat = texCoord_0.y*2-1;
	float seaLevel = SeaLevel(lat, seaLevel_66_Progress);
	float elev2Sea = elev-seaLevel;
	// [0.0,0.1] seaLevel + 66m
	rockMask = mix(rockMask, smoothstep(-10.0, 0.0, elev2Sea), seaLevel_66_Progress);

	color = mix(vec3(0.1,0.25,0.0), baseColor.rgb, clamp(elev2Sea*0.005, 1-0.7*seaLevel_66_Progress, 1.0));
	color *= 0.02 + ambient + 0.5*diffuse;

	vec3 oceanColor = mix(vec3(0.06,0.13,0.2), vec3(0.2,0.3,0.3), exp2(min(0, elev2Sea)*0.01));
	oceanColor = oceanColor*(ambient + 0.5*diffuse) + clamp(-elev2Sea*0.01, 0, 1)*vec3(0.5,0.4,0.3)*pow(dotNH, 200)*sqrt(diffuse);
#else // not WANDERING
	vec3 oceanColor = vec3(0.08,0.1,0.12)*diffuse + vec3(1.0,0.9,0.7)*pow(dotNH, 200)*sqrt(diffuse);
#endif // WANDERING	or not
	color = mix(oceanColor, color, rockMask);
#endif // EARTH

#ifdef ATMOS
	vertAlt = groundTop*0.01;
	// radius at the vertex point
	float Rv = mix(planetRadius.x, planetRadius.y, clamp(abs(texCoord_0.y*2-1), 0, 1));
	color += AtmosColor(max(0, vertAlt), viewPos.xyz, viewDir, viewVertUp, Rv);
#endif // ATMOS

#ifdef EARTH
	// global shadow
	vec2 screenCoord = gl_FragCoord.xy/screenSize.xy;
	float globalShadow = texture(globalShadowTex, screenCoord).r;
	color *= mix(1, globalShadow, clamp((eyeAltitude-0.1*atmosHeight)/atmosHeight,0,1));
#endif // EARTH
	color = ToneMapping(color);

#ifdef EARTH
#ifdef WANDERING
	illumCity *= clamp(20*(texCoord_0.y-0.45), 0, 1);
#endif // WANDERING	
	color = mix(color, vec3(1), illumCity);
#ifdef WANDERING
	vec3 illumEngine = engineIntensity*(1-exp2(-illum.a*vec3(0.1,0.2,0.3)));
	color = mix(color, vec3(1), illumEngine);
	if(unit > 1e6)
	{
		vec4 tailColor = texture(tailTex, screenCoord);
		color = mix(color, tailColor.rgb, tailColor.a);
	}
#endif // WANDERING
#endif // EARTH

	gl_FragColor = vec4(color, 1);
}