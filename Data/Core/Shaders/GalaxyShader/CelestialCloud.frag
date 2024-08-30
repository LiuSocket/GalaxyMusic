#pragma import_defines(SATURN)

#ifdef SATURN
uniform float cosNorthLight;
uniform sampler2D ringTex;
in vec3 shadowVertPos;
#endif // SATURN

#ifdef EARTH

#ifdef WANDERING
uniform vec3 screenSize;
uniform sampler2DArray illumTex;
#endif // WANDERING

uniform float unit;
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

	float lenV = length(viewPos.xyz);
	vec3 cloudCoord = texCoord_1;
	cloudCoord.xy = (cloudCoord.xy - 0.5)*celestialCoordScale.y + 0.5;
	
	vec4 baseColor = texture(cloudTex, cloudCoord);
#ifdef EARTH
#ifdef WANDERING
	vec3 illumCoord = texCoord_1;
	illumCoord.xy = (illumCoord.xy - 0.5)*celestialCoordScale.z + 0.5;
	vec4 illum = texture(illumTex, illumCoord);

	vec3 wanderingCloudCoord = cloudCoord;
	wanderingCloudCoord.z += 6;
	vec4 wanderingColor = texture(cloudTex, wanderingCloudCoord);
	float wanderingCloud = max(0, min(wanderingColor.a, 1-illum.a*illum.a*1.5));

	float torqueArea = clamp((0.3-abs(texCoord_0.y*2-1))*4,0,1); torqueArea *= torqueArea;
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
	vec3 diffuse = vec3(max(dotVUL+0.01,minFact));
	vec3 color = baseColor.rgb * (0.03+diffuse);

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
	color = 0.05 + diffuse;
#ifdef WANDERING
	vec3 ambient = vec3(0.07,0.11,0.15)*allEngineStart;
	color = 0.05 + ambient + diffuse;
	vec3 illumEngine = allEngineStart*(1-exp2(-illum.a*vec3(0.1,0.2,0.3)));
	color += illumEngine;
#endif // WANDERING
#endif // EARTH
	float alpha = baseColor.a*sqrt(clamp(15*abs(dot(viewVertUp, viewDir)), 0, 1));

	// radius of sealevel at the vertex point
	float Rs = mix(planetRadius.x, planetRadius.y, clamp(abs(texCoord_0.y*2-1), 0, 1));
	color += AtmosColor(cloudTop, viewDir, viewVertUp, lenV, Rs);
	color = ToneMapping(color*(1 - 0.9*shadow));

#ifdef EARTH
#ifdef WANDERING
	if((wanderProgress > PROGRESS_0) && (unit > 1e6))
	{
		vec4 tailColor = texture(tailTex, gl_FragCoord.xy/screenSize.xy);
		color = mix(color, tailColor.rgb, tailColor.a);
		alpha = 1-(1-alpha)*(1-tailColor.a);		
	}
#endif // WANDERING
#endif // EARTH
	gl_FragColor = vec4(color, alpha);
}