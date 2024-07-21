#version 400 compatibility
#pragma import_defines(AURORA, SATURN, EARTH, WANDERING)

const float M_PI = 3.141592654;

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

float MiePhase(float dotVS)
{
	const float g = 0.8;
	return (1-g*g)/(4*M_PI*pow(1+g*g-2*g*dotVS, 1.5));	
}

float RayleighPhase(float dotVS)
{
	return 3.0 / (16 * M_PI) * (1 + dotVS * dotVS);
}

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
// pow is different by altRatio
float GetCoordPitch(float cosUV, float altRatio)
{
	return 0.5 + sign(cosUV) * pow(abs(cosUV), 1.0/(1.0+0.8*altRatio)) * 0.5;
}

// get coord of cosUL (the cos of local Up dir & light source dir)
float GetCoordUL(float cosUL)
{
	return (cosUL-minDotUL)/(1-minDotUL);
}

// x = pitch, y = cosUL, z = coordYaw, w = alt
vec4 Texture4D(vec4 coord)
{
	const float ALT_NUM = 32.0;
	const float MIN_Z = 0.5/8.0;
	float coordYaw = clamp(coord.z, MIN_Z, 1-MIN_Z);
	float altI = coord.w*ALT_NUM;
	vec3 UVW_0 = vec3(coord.xy, (min(floor(altI), ALT_NUM - 1) + coordYaw)/ALT_NUM);
	vec3 UVW_1 = vec3(coord.xy, (min(ceil(altI), ALT_NUM - 1) + coordYaw)/ALT_NUM);
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
	
	float altRatio = eyeAltitude / atmosHeight;
	float eyeGeoRadius = lenCore2Eye - eyeAltitude;
	float lenCore2EyeAtmTop = eyeGeoRadius + atmosHeight;

	//cosUV up is +, down is -
	float cosUV = dot(viewUp, viewDir);
	// cosUL = cos of viewUp & light
	float cosUL = dot(viewUp, viewLight);

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
	// cosNUL = viewUp at near atmosphere pos & light
	float cosNUL = dot(viewNearUp, viewLight);

	// sin & cos of horizon, affected by celestial radius
	float sinHoriz = min(1, midGeoRadius / lenCore2Near);
	float cosHoriz = -sqrt(1 - sinHoriz * sinHoriz);
	// sin & cos of eye pos horizon
	float sinEyeHoriz = min(1, eyeGeoRadius / lenCore2Eye);
	float cosEyeHoriz = -sqrt(1 - sinEyeHoriz * sinEyeHoriz);
	cosHoriz = mix(cosHoriz, cosEyeHoriz, nearGround);

	vec3 viewLightRightDir = normalize(cross(viewLight, viewUp));
	vec3 viewLightFrontDir = normalize(cross(viewUp, viewLightRightDir));
	mat3 localLight2ViewMatrix = mat3(
		viewLightRightDir.x,	viewLightRightDir.y,	viewLightRightDir.z,
		viewLightFrontDir.x,	viewLightFrontDir.y,	viewLightFrontDir.z,
		viewUp.x,				viewUp.y,				viewUp.z);
	mat3 view2LocalLightMatrix = inverse(localLight2ViewMatrix);
	vec3 localLightSpaceViewDir = view2LocalLightMatrix*viewDir;
	float coordYaw = acos(localLightSpaceViewDir.y)/M_PI;

	vec4 inscattering = vec4(0,0,0,0);
	if(isInAtmos)
	{
		inscattering = Texture4D(vec4(
			GetCoordPitch(CosSkyDH(cosUV, cosHoriz), altRatio),
			GetCoordUL(cosUL),
			coordYaw,
			min(1, sqrt(altRatio))));
	}
	else
	{
		inscattering = Texture4D(vec4(
			GetCoordPitch(CosSkyDH(cosNUV, cosHoriz), 1),
			GetCoordUL(cosNUL),
			coordYaw,
			1));
	}

	float dotVS = dot(viewDir, viewLight);
	vec3 sunColor = pow(vec3(1.0,0.8,0.2), vec3(5.0-4.0*sqrt(max(0,cosUL*10))*exp2(-altRatio*0.5)));
	vec3 atmosSum = inscattering.rgb*RayleighPhase(dotVS) + inscattering.a*MiePhase(dotVS)*sunColor;

#ifdef EARTH
#else // not EARTH
	atmosSum = (atmosColorMatrix*vec4(atmosSum,1)).rgb;
#endif // EARTH
	vec3 color = ToneMapping(atmosSum * (1 - shadow));
	float alpha = 1-exp2(-(atmosSum.r+atmosSum.g+atmosSum.b)*20);
	alpha *= 1 - shadow;
	gl_FragColor = vec4(color, alpha);
}