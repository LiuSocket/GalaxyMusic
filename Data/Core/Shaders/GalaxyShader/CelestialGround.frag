const float PROGRESS_0 = 0.1;
const float PROGRESS_1 = 0.15;
const float M_PI = 3.1415926;

uniform sampler2DArray baseTex;
uniform vec2 planetRadius;

#ifdef EARTH

uniform float unit;
uniform vec3 screenSize;
uniform vec4 coordScale_Earth;
uniform sampler2DArray DEMTex;
uniform sampler2DArray illumTex;
uniform sampler2D globalShadowTex;

#else // not EARTH
uniform vec4 coordScale;
#endif // EARTH

in vec2 texCoord_0;
in vec3 texCoord_1;
in vec4 modelVertex;
in vec4 viewPos;
in vec3 viewNormal;

float DEM(in float normDEM)
{
	float x = 2*normDEM-1;
	return sign(x)*x*x*1e4;
}

#ifdef WANDERING
// latCoord:[-1.0, 1.0]
float SeaLevel(in float latCoord, in float seaLevelAddProgress)
{
	// [0.0,0.1] seaLevel + 66m
	float seaLevel = 66*seaLevelAddProgress;
	seaLevel = mix(seaLevel, mix(-300, 600, abs(latCoord)), smoothstep(PROGRESS_0, PROGRESS_1, wanderProgress));
	return seaLevel;
}
#endif // WANDERING	

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
	illumCoord.xy = (illumCoord.xy - 0.5)*celestialCoordScale.z + 0.5;
	vec4 illum = texture(illumTex, illumCoord);
	vec3 darkness = 1-smoothstep(0.0, 0.1, diffuse);
	vec3 illumCity = illum.rgb*darkness;
	float rockMask = 1 - baseColor.a;

	vec3 viewHalf = normalize(viewLight - viewDir);
	float dotNH = max(dot(viewVertUp, viewHalf), 0);
	vec3 specualr = mix(vec3(1.0,0.5,0.0),vec3(0.6,0.4,0.3),max(0,dotVUL))*pow(dotNH, 100)*sqrt(diffuse);
#ifdef WANDERING
	float seaLevelAddProgress = clamp(wanderProgress/PROGRESS_0, 0, 1);
	vec3 wanderingBaseCoord = baseCoord;
	wanderingBaseCoord.z += 6;
	vec4 wanderingColor = texture(baseTex, wanderingBaseCoord);
	float engineMask = (1-wanderingColor.a)*seaLevelAddProgress;
	baseColor.rgb = mix(baseColor.rgb, wanderingColor.rgb, seaLevelAddProgress);

	// for start
	vec3 MVU = normalize(modelVertex.xyz);
	float lon = abs(atan(MVU.x, MVU.y))/M_PI;
	float engineStart = smoothstep(0.0, 0.1, max(MVU.z-1+engineStartRatio,
		clamp(2*engineStartRatio-0.2-lon,0,1)*clamp((0.35-abs(MVU.z))*4,0,1)));
	vec3 ambient = vec3(0.07,0.11,0.15)*(exp2(-abs(texCoord_0.y-0.5)*25)+exp2(min(0,texCoord_0.y-0.67)*50))*engineStart;

	vec3 DEMCoord = texCoord_1;
	DEMCoord.xy = (DEMCoord.xy - 0.5)*celestialCoordScale.w + 0.5;
	float elev = DEM(texture(DEMTex, DEMCoord).r);
	vertAlt = elev/unit;
	float latCoord = texCoord_0.y*2-1;
	float seaLevel = SeaLevel(latCoord, seaLevelAddProgress);
	float elev2Sea = elev-seaLevel;
	// [0.0,0.1] seaLevel + 66m
	rockMask = mix(rockMask, smoothstep(-10.0, 0.0, elev2Sea), seaLevelAddProgress);
	illumCity = max((0.2+0.05*baseColor.rgb)*darkness*engineMask,
		rockMask*(illumCity - seaLevelAddProgress));

	color = mix(vec3(0.0,0.1,0.0), baseColor.rgb, clamp(elev2Sea*0.1, 1-0.7*seaLevelAddProgress, 1.0));
	color *= 0.02 + ambient + 0.5*diffuse;

	vec3 oceanColor = mix(vec3(0.06,0.13,0.2), vec3(0.2,0.3,0.3), seaLevelAddProgress*exp2(min(0, elev2Sea)*0.01));
	oceanColor = oceanColor*(ambient + 0.5*diffuse) + clamp(-elev2Sea*0.01, 0, 1)*specualr;
#else // not WANDERING
	vec3 oceanColor = vec3(0.08,0.1,0.12)*diffuse + specualr;
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
	color = mix(color, vec3(1), illumCity);
#ifdef WANDERING
	vec3 illumEngine = engineStart*(1-exp2(-illum.a*vec3(0.1,0.2,0.3)));
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