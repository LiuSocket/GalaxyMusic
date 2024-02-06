#pragma import_defines(SATURN)

#ifdef SATURN
uniform float cosNorthLight;
uniform sampler2D ringTex;
in vec3 shadowVertPos;
#endif // SATURN

#ifdef EARTH

#ifdef WANDERING
uniform sampler2D tailTex;
uniform float engineIntensity;
uniform vec3 screenSize;
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
#endif // EARTH

	vec3 cloudCoord = texCoord_1;
	cloudCoord.xy = (cloudCoord.xy - 0.5)*celestialCoordScale.y + 0.5;
	
	vec4 baseColor = texture(cloudTex, cloudCoord);
#ifdef EARTH
#ifdef WANDERING
	vec3 wanderingCloudCoord = cloudCoord;
	wanderingCloudCoord.z += 6;
	vec4 wanderingColor = texture(cloudTex, wanderingCloudCoord);
	baseColor.a = mix(baseColor.a, wanderingColor.a, clamp((wanderProgress-0.7)*100, 0, 1));
#endif // WANDERING	
	float lenV = length(viewPos.xyz);
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
	color = 0.03 + diffuse;
#ifdef WANDERING
	vec3 ambient = vec3(0.07,0.11,0.15)*exp2(min(0, texCoord_0.y-0.48)*25);
	color = 0.03 + ambient + 0.5*diffuse;
	vec3 illumEngine = engineIntensity*(1-exp2(-baseColor.r*vec3(0.1,0.2,0.3)));
	color += illumEngine;
#endif // WANDERING
#endif // EARTH
	float alpha = baseColor.a*sqrt(clamp(15*abs(dot(viewVertUp, viewDir)), 0, 1));

	// radius at the vertex point
	float Rv = mix(planetRadius.x, planetRadius.y, clamp(abs(texCoord_0.y*2-1), 0, 1));
	color += AtmosColor(cloudTop, viewPos.xyz, viewDir, viewVertUp, Rv);
	color = ToneMapping(color*(1 - 0.9*shadow));

#ifdef EARTH
#ifdef WANDERING
	if(unit > 1e6)
	{
		vec4 tailColor = texture(tailTex, gl_FragCoord.xy/screenSize.xy);
		color = mix(color, tailColor.rgb, tailColor.a);
		alpha = 1-(1-alpha)*(1-tailColor.a);		
	}
#endif // WANDERING
#endif // EARTH
	gl_FragColor = vec4(color, alpha);
}