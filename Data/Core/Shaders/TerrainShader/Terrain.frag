#pragma import_defines(WANDERING)

uniform sampler2DArray baseTex;
uniform vec2 planetRadius;

#ifdef EARTH

uniform float unit;
uniform vec3 screenSize;
uniform vec4 coordScale_Earth;
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

	vec4 baseColor = texture2DArray(baseTex, baseCoord);
	vec3 viewVertUp = normalize(viewNormal);
	vec3 viewDir = normalize(viewPos.xyz);

	const float minFact = 1e-8;
	float dotVUL = dot(viewVertUp, viewLight);
	vec3 diffuse = vec3(max(dotVUL,minFact));
	vec3 color = baseColor.rgb * (0.02+0.98*diffuse);

#ifdef EARTH
	vec3 illumCoord = texCoord_1;
	illumCoord.xy = (illumCoord.xy - 0.5)*celestialCoordScale.y + 0.5;

	vec4 illum = texture2DArray(illumTex, illumCoord);
	vec3 illumCity = illum.rgb*(vec3(1)-smoothstep(0.0, 0.1, diffuse));
	float oceanMask = baseColor.a;

	vec3 viewHalf = normalize(viewLight-viewDir);
	float dotNH = max(dot(viewVertUp, viewHalf), 0);
#ifdef WANDERING
	vec3 oceanColor = vec3(0.1,0.2,0.15)*diffuse + 0.5*pow(dotNH, 50)*sqrt(diffuse);
#else // not WANDERING
	vec3 oceanColor = vec3(0.12,0.17,0.22)*diffuse + vec3(1.0,0.9,0.7)*pow(dotNH, 200)*sqrt(diffuse);
#endif // WANDERING	
	color = mix(color, oceanColor, oceanMask);
#endif // EARTH

#ifdef ATMOS
	float vertAlt = groundTop*0.01;
	// radius at the vertex point
	float Rv = mix(planetRadius.x, planetRadius.y, clamp(abs(texCoord_0.y*2-1), 0, 1));
	color += AtmosColor(vertAlt, viewPos.xyz, viewDir, viewVertUp, Rv);
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