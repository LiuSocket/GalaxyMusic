#version 400 compatibility
#pragma import_defines(AURORA, SATURN, EARTH, WANDERING)

uniform vec3 viewUp;
uniform vec3 viewLight;
uniform float atmosHeight;
uniform float eyeAltitude;
uniform float minDotUL;
uniform vec2 planetRadius; // x= equator radius, y = polar radius
uniform mat4 view2ECEFMatrix;
uniform sampler3D inscatteringTex;

#ifdef EARTH
#ifdef WANDERING
uniform float wanderProgress;
#endif // WANDERING	
#else // not EARTH
uniform mat4 atmosColorMatrix;
#endif // EARTH

#ifdef SATURN
uniform float cosNorthLight;
uniform sampler2D ringTex;
in vec3 shadowVertPos;
#endif // SATURN

#ifdef AURORA
//uniform sampler2D auroraTex;
#endif // AURORA

in vec4 viewPos;
in vec3 viewNormal;

vec3 ToneMapping(vec3 color)
{
	const float A = 2.51;
	const float B = 0.03;
	const float C = 2.43;
	const float D = 0.59;
	const float E = 0.14;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}

// cosSkyDH = cosDir of sky with the horizon fact
float CosSkyDH(float cosDir, float cosHoriz)
{
	return (cosDir-cosHoriz)/(1-cosHoriz);
}

// get coord of cosUV (the cos of local Up dir & view dir)
float GetCoordPitch(float cosUV)
{
	return 0.5 + sqrt(abs(cosUV))*sign(cosUV) * 0.5;
}

// get coord of cosUL (the cos of local Up dir & light source dir)
float GetCoordUL(float cosUL)
{
	return (cosUL-minDotUL)/(1-minDotUL);
}

// x = pitch, y = cosUL, z = cosVL, w = alt
vec4 Texture4D(vec4 coord)
{
	const float ALT_NUM = 16.0;
	float altI = coord.w*ALT_NUM;
	vec3 UVW_0 = vec3(coord.xy, (min(floor(altI), ALT_NUM - 1) + coord.z)/ALT_NUM);
	vec3 UVW_1 = vec3(coord.xy, (min(ceil(altI), ALT_NUM - 1) + coord.z)/ALT_NUM);
	vec4 color_0 = texture(inscatteringTex, UVW_0);
	vec4 color_1 = texture(inscatteringTex, UVW_1);
	
	return mix(color_0, color_1, fract(altI));
}

// rE = radius of Equator
// rP = radius of Polar
// cosTheta = dot(vPlanetNorth, vSkyUp)
float GeoRadius(float rE, float rP, float cosTheta)
{
	float rE2 = rE*rE;
	float rP2 = rP*rP;
	return rE * rP / sqrt(rP2 + cosTheta * cosTheta * (rE2 - rP2));
}

void main() 
{
	bool isInAtmos = eyeAltitude < atmosHeight;
	float inAtmos = float(isInAtmos);

#ifdef AURORA
	//baseColor = texture(auroraTex, gl_TexCoord[0].xy);
#endif // AURORA

	float shadow = 0;
#ifdef SATURN
	const float ringMinR = 0.0074658;
	const float ringMaxR = 0.0139826;
	float shadowVertDis = length(vec2(shadowVertPos.x,shadowVertPos.y/max(1e-9, abs(cosNorthLight))));
	float coordU = clamp((shadowVertDis-ringMinR)/(ringMaxR-ringMinR),0,1);
	shadow = texture(ringTex, vec2(coordU, 0.5)).a;
	shadow *= step(0, shadowVertPos.y*sign(cosNorthLight));
#endif // SATURN

	vec3 ECEFEyePos = view2ECEFMatrix[3].xyz;
	float lenCore2Eye = length(ECEFEyePos);
	vec3 viewDir = normalize(viewPos.xyz);
	//cosUV up is +, down is -
	float cosUV = dot(viewUp, viewDir); 
	float eyeGeoRadius = lenCore2Eye - eyeAltitude;
	float lenCore2EyeAtmTop = eyeGeoRadius + atmosHeight;

	vec3 viewCorePos = -viewUp*lenCore2Eye;
	vec3 viewCore2FarPos = viewPos.xyz-viewCorePos;
	vec3 viewMid2Far = dot(viewCore2FarPos,viewDir)*viewDir;
	vec3 viewMidPos = viewPos.xyz - viewMid2Far;
	float lenCore2Mid = distance(viewCorePos, viewMidPos);
	vec3 ECEFMidPos = (view2ECEFMatrix*vec4(viewMidPos, 1.0)).xyz;
	// lenCore2Mid => lenCore2Near: wrong but useful
	float midGeoRadius = GeoRadius(planetRadius.x, planetRadius.y, abs(normalize(ECEFMidPos).z));

	float nearGround = exp2(min(0,atmosHeight-eyeAltitude)/atmosHeight);
	// mid geo radius near eye -> eyeGeoRadius
	midGeoRadius = mix(midGeoRadius, eyeGeoRadius, nearGround);
	float lenCore2Near = atmosHeight + midGeoRadius;
	float lenMid2Near = sqrt(lenCore2Near*lenCore2Near - lenCore2Mid*lenCore2Mid);
	// approach near pos
	vec3 viewNearPos = viewMidPos - normalize(viewMid2Far)*lenMid2Near;
	viewNearPos *= (1-inAtmos)*(1-nearGround);

	vec3 viewNearUp = normalize(viewNearPos-viewCorePos);
	float cosNUV = dot(viewNearUp, viewDir);
	float sinNUV = sqrt(1 - cosNUV * cosNUV);
	// cosNUL = viewUp at near atmosphere pos & light
	float cosNUL = dot(viewNearUp, viewLight);
	// cosVL = the cos of view dir & light source dir
	float cosVL = dot(viewDir, viewLight);
	// sin & cos of horizon, affected by celestial radius
	float sinHoriz = min(1, midGeoRadius / lenCore2Near);
	float cosHoriz = -sqrt(1 - sinHoriz * sinHoriz);
	// sin & cos of eye pos horizon
	float sinEyeHoriz = min(1, eyeGeoRadius / lenCore2Eye);
	float cosEyeHoriz = -sqrt(1 - sinEyeHoriz * sinEyeHoriz);
	cosHoriz = mix(cosHoriz, cosEyeHoriz, nearGround);

	vec4 atmosSum = vec4(0,0,0,0);
	if(isInAtmos)
	{
		// cosUL = cos of viewUp & light
		float cosUL = dot(viewUp, viewLight);
		float sinUV = sqrt(1 - cosUV * cosUV);

		vec2 eyeLightDir = vec2(sqrt(1 - cosUL*cosUL), cosUL);
		vec2 eyeViewFrontDir = vec2(sinUV, cosUV);
		vec2 eyeViewBackDir = vec2(-sinUV, cosUV);
		float cosMinVL = dot(eyeViewBackDir, eyeLightDir);
		float cosMaxVL = dot(eyeViewFrontDir, eyeLightDir);
		float deltaCosVL = cosMaxVL-cosMinVL;

		atmosSum = Texture4D(vec4(
			GetCoordPitch(CosSkyDH(cosUV, cosHoriz)),
			GetCoordUL(cosUL),
			(cosMaxVL-cosVL)/deltaCosVL,
			min(1, sqrt(eyeAltitude / atmosHeight))));
	}
	else
	{
		vec2 nearLightDir = vec2(sqrt(1 - cosNUL*cosNUL), cosNUL);
		vec2 nearViewFrontDir = vec2(sinNUV, cosNUV);
		vec2 nearViewBackDir = vec2(-sinNUV, cosNUV);
		float cosMinVL = dot(nearViewBackDir, nearLightDir);
		float cosMaxVL = dot(nearViewFrontDir, nearLightDir);
		float deltaCosVL = cosMaxVL-cosMinVL;
		deltaCosVL = (2*step(0, deltaCosVL)-1) * max(1e-9, abs(deltaCosVL));

		atmosSum = Texture4D(vec4(
			GetCoordPitch(CosSkyDH(cosNUV, cosHoriz)),
			GetCoordUL(cosNUL),
			(cosMaxVL-cosVL)/deltaCosVL,
			1));
	}

#ifdef EARTH
#ifdef WANDERING
	float meanSum = (atmosSum.r+atmosSum.g+atmosSum.b)*0.33;
	atmosSum.rgb = mix(atmosSum.rgb, vec3(1.0,1.2,1.0)*meanSum, 0.5*clamp(10*(wanderProgress-0.1),0,1));
#endif // WANDERING
#else // not EARTH
	atmosSum.rgb = (atmosColorMatrix*vec4(atmosSum.rgb,1)).rgb;
#endif // EARTH
	vec3 color = ToneMapping(atmosSum.rgb * (1 - shadow));
	float alpha = atmosSum.a * (1 - shadow);

	gl_FragColor = vec4(color, alpha);
}