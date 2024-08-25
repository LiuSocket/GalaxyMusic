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

// get sky coord of pitch (d0/dH)
// d0 = distance of point to the sky
// dV = distance of point to the ground point vertical above
// dH = distance of atmos top behind horizon
float GetSkyCoordPitch(float d0, float dV, float dH)
{
	return 1.0 - 0.5 * clamp((d0 - dV) / (dH - dV), 0.0, 1.0);
}
// get ground coord of pitch (d0/dh)
// d0 = distance of point to the ground
// dv = distance of point to the ground point vertical below
// dh = distance of horizon
float GetGroundCoordPitch(float d0, float dv, float dh)
{
	return 0.5 * clamp((d0 - dv) / (dh - dv), 0.0, 1.0);
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
	float Rc = length(ECEFEyePos);
	float Rc2 = Rc*Rc;
	vec3 viewDir = normalize(viewPos.xyz);
	
	float altRatio = eyeAltitude / atmosHeight;
	float eyeGeoRadius = Rc - eyeAltitude;

	//cosUV up is +, down is -
	float cosUV = dot(viewUp, viewDir);
	float sinUV2 = max(0.0, 1.0 - cosUV*cosUV);
	// cosUL = cos of viewUp & light
	float cosUL = dot(viewUp, viewLight);
	// sin & cos of eye pos horizon
	float sinEyeHoriz = min(1.0, eyeGeoRadius / Rc); // it must < 1
	float cosEyeHoriz = -sqrt(1.0 - sinEyeHoriz * sinEyeHoriz); // it must < 0

	vec3 viewCorePos = -viewUp*Rc;
	vec3 viewCore2FarPos = viewPos.xyz-viewCorePos;
	vec3 viewMid2Far = dot(viewCore2FarPos,viewDir)*viewDir;
	vec3 viewMidPos = viewPos.xyz - viewMid2Far;
	float lenCore2Mid = distance(viewCorePos, viewMidPos);
	vec3 ECEFFarPos = (view2ECEFMatrix*viewPos).xyz;
	vec3 ECEFMidPos = (view2ECEFMatrix*vec4(viewMidPos, 1.0)).xyz;
	float farGeoRadius = GeoRadius(planetRadius.x, planetRadius.y, abs(normalize(ECEFFarPos).z));
	// lenCore2Mid => lenCore2Far: wrong but useful
	float midGeoRadius = GeoRadius(planetRadius.x, planetRadius.y, abs(normalize(ECEFMidPos).z));
	float nearGround = exp2(min(0,atmosHeight-eyeAltitude)/atmosHeight);
	// mid geo radius near eye -> eyeGeoRadius
	midGeoRadius = mix(midGeoRadius, eyeGeoRadius, nearGround);
	
	float lenCore2Far = atmosHeight + farGeoRadius;
	float lenCore2Far2 = lenCore2Far*lenCore2Far;
	float lenMid2Far = sqrt(lenCore2Far2 - lenCore2Mid*lenCore2Mid);
	// approach top atmos pos
	vec3 viewFarPos = viewMidPos + normalize(viewMid2Far)*lenMid2Far;
	vec3 viewFarUp = normalize(viewFarPos-viewCorePos);
	//cosFUV up is +, down is -
	float cosFUV = dot(viewFarUp, viewDir);
	float sinFUV = sqrt(max(0.0, 1.0-cosFUV*cosFUV));
	// cosFUL = viewUp at far atmosphere pos & light
	float cosFUL = dot(viewFarUp, viewLight);

	vec3 viewLightRightDir = normalize(cross(viewLight, viewUp));
	vec3 viewLightFrontDir = normalize(cross(viewUp, viewLightRightDir));
	mat3 localLight2ViewMatrix = mat3(
		viewLightRightDir.x,	viewLightRightDir.y,	viewLightRightDir.z,
		viewLightFrontDir.x,	viewLightFrontDir.y,	viewLightFrontDir.z,
		viewUp.x,				viewUp.y,				viewUp.z);
	mat3 view2LocalLightMatrix = inverse(localLight2ViewMatrix);
	vec3 localLightSpaceViewDir = view2LocalLightMatrix*viewDir;
	float coordYaw = acos(localLightSpaceViewDir.y)/M_PI;

	// radius of sealevel ground
	float Rs = midGeoRadius;
	float Rs2 = Rs*Rs;
	float Rt = Rs + atmosHeight;
	float Rt2 = Rt*Rt;
	float lenHorizonMax = sqrt(Rt2 - Rs2);

	vec4 inscattering = vec4(0);
	if(eyeAltitude < atmosHeight)
	{
		float lenEye2Horizon = sqrt(max(0.0, Rc2 - Rs2));
		float lenAtmosHorizon = lenEye2Horizon + lenHorizonMax;

		float lenEye2Atmos = length(viewFarPos);
		//float lenEye2Ground = Rc*cosUV - sqrt(max(0.0, Rs2 - Rc2*sinUV2));
		float lenEye2Top = max(0.0, atmosHeight - eyeAltitude);

		//bool isToSky = cosUV > cosEyeHoriz;
		float eyeSkyCoordPitch = GetSkyCoordPitch(lenEye2Atmos, lenEye2Top, lenAtmosHorizon);
		//float eyeGroundCoordPitch = GetGroundCoordPitch(lenEye2Ground, eyeAltitude, lenEye2Horizon);

		inscattering = Texture4D(vec4(
			eyeSkyCoordPitch,
			GetCoordUL(cosUL),
			coordYaw,
			min(1, sqrt(altRatio))));
	}
	else
	{
		float lenFar2Atmos = 2*lenMid2Far;
		//float lenFar2Ground = lenCore2Far*cosFUV - sqrt(max(0.0, Rs2 - lenCore2Far2*sinFUV*sinFUV));

		//bool isToSky = (-cosUV) > cosEyeHoriz;
		float skyCoordPitch = GetSkyCoordPitch(lenFar2Atmos, 0.0, 2*lenHorizonMax);
		//float groundCoordPitch = GetGroundCoordPitch(lenFar2Ground, atmosHeight, lenHorizonMax);

		inscattering = Texture4D(vec4(
			skyCoordPitch,
			GetCoordUL(cosFUL),
			coordYaw,
			1));
	}

	float dotVS = dot(viewDir, viewLight);
	const vec3 sunColor = vec3(1.0,0.5,0.0);
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