#pragma import_defines(SATURN)
#pragma import_defines(RESOLUTION_QUARTER)

#ifdef SATURN
uniform float cosNorthLight;
uniform sampler2D ringTex;
in vec3 shadowVertPos;
#endif // SATURN

#ifdef EARTH

#ifdef WANDERING
uniform sampler2DArray illumTex;
#endif // WANDERING

uniform vec4 coordScale_Earth;
uniform sampler2D cloudDetailTex;

#else // not EARTH

uniform vec4 coordScale;

#endif // EARTH

uniform float cloudTop;
uniform vec2 planetRadius;
uniform sampler2DArray cloudTex;

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
#endif // EARTH or not

	float lenV = length(viewPos.xyz); // hierarchy unit
	vec3 cloudCoord = texCoord_1;
	cloudCoord.xy = (cloudCoord.xy - 0.5)*celestialCoordScale.y + 0.5;
	
	vec4 baseColor = texture(cloudTex, cloudCoord);
	baseColor.rgb *= baseColor.rgb;
#ifdef EARTH
#ifdef WANDERING
	vec3 illumCoord = texCoord_1;
	illumCoord.xy = (illumCoord.xy - 0.5)*celestialCoordScale.z + 0.5;
	vec4 illum = texture(illumTex, illumCoord);

	vec3 wanderingCloudCoord = cloudCoord;
	wanderingCloudCoord.z += 6;
	vec4 wanderingColor = texture(cloudTex, wanderingCloudCoord);
	float wanderingCloud = max(0, min(wanderingColor.a, 1-illum.a*illum.a*1.5));

	float torqueArea = clamp((0.3-abs(texCoord_0.y*2-1))*2,0,1); torqueArea *= torqueArea;
	// for start
	float lon = abs(fract(texCoord_0.x-0.25)*2-1);
	lon = (engineStartRatio.z > 0.5) ? lon : 1-lon;
	// x = torque, y = propulsion
	vec2 engineStart = vec2(
		smoothstep(0.0, 0.2, clamp(2*engineStartRatio.x-lon,0,1)*torqueArea)*exp2(-abs(texCoord_0.y-0.5)*25),
		smoothstep(0.0, 0.2, (texCoord_0.y-1)*2+1.3*engineStartRatio.y)*exp2(min(0,texCoord_0.y-0.67)*40));
	float allEngineStart = max(engineStart.x, engineStart.y);

	baseColor.a = mix(baseColor.a, wanderingCloud, allEngineStart*min(1, engineStartRatio.x));
#endif // WANDERING	

	// cloud detail
	vec4 detail4 = texture(cloudDetailTex, texCoord_1.xy*27);
	vec4 detailMix = clamp((baseColor.a-vec4(0.2,0.35,0.5,0.65))/0.15, vec4(0), vec4(1));
	float detail = mix(mix(mix(mix(detail4.x,
		detail4.y, detailMix.x),
		detail4.z, detailMix.y),
		detail4.w, detailMix.z),
		1, detailMix.w);
	vec2 edgeXY = clamp(20*(1+4*detail)*(0.5-abs(texCoord_1.xy-0.5)), baseColor.a, 1.0);
	float edge = edgeXY.x*edgeXY.y;

	float detailFinal = detail*clamp(baseColor.a/0.2, 0, 1);
	baseColor.a = mix(baseColor.a, detailFinal, edge*exp2(-lenV*0.5/planetRadius.x));
#endif // EARTH

	vec3 viewDir = normalize(viewPos.xyz);
	vec3 viewVertUp = normalize(viewNormal);
	const float minFact = 1e-8;
	float dotVUL = dot(viewVertUp, viewLight);
	vec3 diffuse = vec3(max(dotVUL,minFact));
	vec3 color = baseColor.rgb * (0.001+diffuse);

	float shadow = 0;
#ifdef SATURN
	const float ringMinR = 0.0074658;
	const float ringMaxR = 0.0139826;
	float shadowVertDis = length(vec2(shadowVertPos.x,shadowVertPos.y/max(1e-9, abs(cosNorthLight))));
	float coordU = clamp((shadowVertDis-ringMinR)/(ringMaxR-ringMinR),0,1);
	shadow = texture(ringTex, vec2(coordU, 0.5)).a;
	shadow *= step(0, shadowVertPos.y*sign(cosNorthLight));
	shadow *= step(0, dotVUL);
#endif // SATURN

#ifdef EARTH
	diffuse = vec3(max(dotVUL+0.01,minFact));
	color = 0.002 + diffuse;
#ifdef WANDERING
	vec3 ambient = vec3(0.04,0.05,0.07)*allEngineStart;
	color = 0.002 + ambient + diffuse;
	vec3 illumEngine = allEngineStart*(1-exp2(-illum.a*vec3(0.1,0.2,0.3)));
	color += illumEngine;
#endif // WANDERING
#endif // EARTH
	float alpha = baseColor.a*sqrt(clamp(15*abs(dot(viewVertUp, viewDir)), 0, 1));
	vec2 screenCoord = gl_FragCoord.xy/screenSize.xy;
	vec3 ECEFPos = (view2ECEFMatrix*vec4(viewPos.xyz, 1.0)).xyz;
	// radius of sealevel ground at the vertex point, meter
	float Rg = GeoRadius(planetRadius.x, planetRadius.y, abs(normalize(ECEFPos).z))*unit;
	color += AtmosColor(cloudTop*unit, viewDir, viewVertUp, Rg);
	color = ToneMapping(color*(1 - 0.9*shadow));
	color = pow(color,vec3(1.0/2.2));
	color += mix(-NOISE_GRANULARITY, NOISE_GRANULARITY, Random(screenCoord));

#ifdef EARTH
#ifdef WANDERING
	if((wanderProgress > PROGRESS_0) && (unit > 1e6))
	{
		vec4 tailColor = texture(tailColorTex, screenCoord);
#ifdef RESOLUTION_QUARTER
		vec2 subPixDir = sign(mod(gl_FragCoord.xy, vec2(4))-2);
		vec2 alphaPixelUnit = subPixDir*4;
#else // !RESOLUTION_QUARTER
		vec2 subPixDir = sign(mod(gl_FragCoord.xy, vec2(2))-1);
		vec2 alphaPixelUnit = subPixDir*2;
#endif // RESOLUTION_QUARTER or not
		vec4 alpha4_00 = texture(tailAlphaTex, screenCoord);
		vec4 alpha4_10 = texture(tailAlphaTex, (gl_FragCoord.xy + vec2(alphaPixelUnit.x, 0))/screenSize.xy);
		vec4 alpha4_01 = texture(tailAlphaTex, (gl_FragCoord.xy + vec2(0, alphaPixelUnit.y))/screenSize.xy);
		vec4 alpha4_11 = texture(tailAlphaTex, (gl_FragCoord.xy + alphaPixelUnit)/screenSize.xy);

		bool isLU = (subPixDir.y-subPixDir.x)>1;
		bool isRU = (subPixDir.y+subPixDir.x)>1;
		bool isRD = (subPixDir.x-subPixDir.y)>1;
		bool isLD = (subPixDir.y+subPixDir.x)<(-1);

		vec4 corner4 = vec4(isRD, isLD, isLU, isRU);
		vec4 filter_00 = 1 - corner4*0.25;
		vec4 filter_10 = vec4(
			isRD ? 0.75 : float(isRU),
			isLD ? 0.75 : float(isLU),
			isLU ? 0.75 : float(isLD),
			isRU ? 0.75 : float(isRD));
		vec4 filter_01 = vec4(
			isRD ? 0.75 : float(isLD),
			isLD ? 0.75 : float(isRD),
			isLU ? 0.75 : float(isRU),
			isRU ? 0.75 : float(isLU));	
		vec4 filter_11 = corner4*0.75;

#ifdef RESOLUTION_QUARTER
		float tailAlpha = 0.125*(dot(filter_00, alpha4_00)
					+ dot(filter_10, alpha4_10)
					+ dot(filter_01, alpha4_01)
					+ dot(filter_11, alpha4_11));
#else // !RESOLUTION_QUARTER
		float tailAlpha = 0.125*(dot(filter_00, alpha4_00)
					+ dot(filter_10, alpha4_10)
					+ dot(filter_01, alpha4_01)
					+ dot(filter_11, alpha4_11));
#endif // RESOLUTION_QUARTER or not
		color = mix(color, tailColor.rgb, tailAlpha); // to do
		alpha = 1-(1-alpha)*(1-tailAlpha);		
	}
#endif // WANDERING
#endif // EARTH
	gl_FragColor = vec4(color, alpha);
}