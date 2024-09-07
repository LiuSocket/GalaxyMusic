#version 400 compatibility

#pragma import_defines(ATMOS, EARTH, WANDERING)

const float M_PI = 3.141592654;
const float HORIZON_OFFSET = 0.004; // to solve the bug of discontinuous at the horizon

const float PROGRESS_0 =	0.005;
const float PROGRESS_1 =	0.03; // end of brake time
const float PROGRESS_1_1 =	0.04;
const float PROGRESS_2 =	0.09; // middle of torque time
const float PROGRESS_2_1 =	0.1;
const float PROGRESS_3 =	0.15; // end of torque time
const float PROGRESS_3_1 =	0.152;

uniform vec3 viewUp;
uniform vec3 viewLight;
uniform float minDotUL;
uniform float unit;

#ifdef WANDERING
uniform sampler2D tailTex;
uniform vec3 engineStartRatio;
uniform float wanderProgress;
#endif // WANDERING

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

#ifdef ATMOS

uniform float atmosHeight;
uniform float eyeAltitude;
uniform mat4 view2ECEFMatrix;
uniform mat4 osg_ViewMatrixInverse;
uniform sampler3D inscatteringTex;

#ifndef EARTH
uniform mat4 atmosColorMatrix;
#endif // EARTH

// rE = radius of Equator
// rP = radius of Polar
// cosTheta = dot(vPlanetNorth, vSkyUp)
float GeoRadius(float rE, float rP, float cosTheta)
{
	float rE2 = rE*rE;
	float rP2 = rP*rP;
	return rE * rP / sqrt(rP2 + cosTheta * cosTheta * (rE2 - rP2));
}

// get sky coord of pitch (d0/dH)
// d0 = distance of point to the sky
// dV = distance of point to the ground point vertical above
// dH = distance of atmos top behind horizon
float GetSkyCoordPitch(float d0, float dV, float dH)
{
	return 1.0 - (0.5-HORIZON_OFFSET) * clamp((d0 - dV) / (dH - dV), 0, 1);
}
// get ground coord of pitch (d0/dh)
// d0 = distance of point to the ground
// dv = distance of point to the ground point vertical below
// dh = distance of horizon
float GetGroundCoordPitch(float d0, float dv, float dh)
{
	return (0.5-HORIZON_OFFSET) * clamp((d0 - dv) / (dh - dv), 0, 1);
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

// vertAlt, and Rg are all in meter
// Rg : radius of sealevel ground at the vertex point
vec3 AtmosColor(float vertAlt, vec3 viewDir, vec3 viewVertUp, float Rg)
{
	float atmosH_meter = atmosHeight*unit; // meter
	float elev = max(0.0, vertAlt); // meter
	float Rc = length(view2ECEFMatrix[3].xyz)*unit; // meter
	float Rt = Rg + atmosH_meter; // meter
	float Rv = Rg + elev; // meter

	float Rg2 = Rg*Rg; // meter
	float Rc2 = Rc*Rc; // meter
	float Rt2 = Rt*Rt; // meter
	float Rv2 = Rv*Rv; // meter

	// vertex is up to eye: -, vertex is down to eye: +
	// cosVertUV(at the vertex pos):
	float cosVertUV = dot(viewVertUp, viewDir);
	// sinVertUV(at the vertex pos):
	float sinVertUV2 = max(0, 1-cosVertUV*cosVertUV);
	// cosVertUL = the cos of vertex Up dir & light source dir
	float cosVertUL = dot(viewVertUp, viewLight);
	// length of vert to mid point
	float lenVert2Mid = Rv*abs(cosVertUV);
	// length2 of core to mid point
	float lenCore2Mid2 = Rv2*sinVertUV2;
	// length of atmos top to mid point
	float lenAtmos2Mid = sqrt(max(0.0, Rt2 - lenCore2Mid2));
	// length of atmos top to mid point
	float lenGround2Mid = sqrt(max(0.0, Rg2 - lenCore2Mid2));

	vec3 viewLightRightDir = normalize(cross(viewLight, viewVertUp));
	vec3 viewLightFrontDir = normalize(cross(viewVertUp, viewLightRightDir));
	mat3 localLight2ViewMatrix = mat3(
		viewLightRightDir.x,	viewLightRightDir.y,	viewLightRightDir.z,
		viewLightFrontDir.x,	viewLightFrontDir.y,	viewLightFrontDir.z,
		viewVertUp.x,			viewVertUp.y,			viewVertUp.z);
	mat3 view2LocalLightMatrix = inverse(localLight2ViewMatrix);
	vec3 localLightSpaceViewDir = view2LocalLightMatrix*viewDir;
	float coordYaw = acos(-localLightSpaceViewDir.y)/M_PI;

	float lenVertHorizon = sqrt(max(0.0, Rv2 - Rg2)); // meter
	float lenHorizonMax = sqrt(Rt2 - Rg2); // meter
	float lenVertAtmosHorizon = lenVertHorizon + lenHorizonMax; // meter
	float lenVert2Top = max(0.0, atmosH_meter - elev); // meter

	float elevCoord = min(1.0, sqrt(elev / atmosH_meter));
	vec4 inscattering = vec4(0);
	if(eyeAltitude < atmosHeight)
	{
		//cosUV(at the eye pos)
		float cosUV = dot(viewUp, viewDir);
		// sinVertUV(at the vertex pos):
		float sinUV2 = max(0, 1-cosUV*cosUV);
		// cosUL = the cos of local Up dir & light source dir
		float cosUL = dot(viewUp, viewLight);
		// length of eye to mid point
		float lenEye2Mid = Rc*abs(cosUV);

		// sin & cos of eye pos horizon
		float sinEyeHoriz = min(1.0, Rg / Rc); // it must < 1
		float cosEyeHoriz = -sqrt(1.0 - sinEyeHoriz * sinEyeHoriz); // it must < 0
		bool isEyeToSky = (-cosUV) > cosEyeHoriz;
		// sin & cos of vert pos horizon
		// float sinVertHoriz = min(1.0, Rg / Rv); // it must < 1
		// float cosVertHoriz = -sqrt(1.0 - sinVertHoriz * sinVertHoriz); // it must < 0
		// bool isVertToSky = (-cosVertUV) > cosVertHoriz;

		float lenEye2Atmos = lenAtmos2Mid - lenEye2Mid; // meter
		float lenEye2Ground = lenEye2Mid - lenGround2Mid; // meter
		float lenEye2Top = max(0.0, atmosHeight - eyeAltitude)*unit; // meter
		float lenEye2Horizon = sqrt(max(0.0, Rc2 - Rg2)); // meter
		float lenAtmosHorizon = lenEye2Horizon + lenHorizonMax; // meter
		float eyeSkyCoordPitch = GetSkyCoordPitch(lenEye2Atmos, lenEye2Top, lenAtmosHorizon);
		float eyeGroundCoordPitch = GetGroundCoordPitch(lenEye2Ground, eyeAltitude*unit, lenEye2Horizon);

		vec4 inscatterEye = Texture4D(vec4(
			isEyeToSky ? eyeSkyCoordPitch : eyeGroundCoordPitch,
			GetCoordUL(cosUL),
			coordYaw,
			sqrt(max(0.0, eyeAltitude / atmosHeight))));

		float lenVert2Atmos = lenAtmos2Mid - lenVert2Mid; // meter
		float lenVert2Ground = lenVert2Mid - lenGround2Mid; // meter
		float vertSkyCoordPitch = GetSkyCoordPitch(lenVert2Atmos, lenVert2Top, lenVertAtmosHorizon);
		float vertGroundCoordPitch = GetGroundCoordPitch(lenVert2Ground, elev, lenVertHorizon);

		vec4 inscatterVert = Texture4D(vec4(
			isEyeToSky ? vertSkyCoordPitch : vertGroundCoordPitch,
			GetCoordUL(cosVertUL),
			coordYaw,
			elevCoord));

		inscattering = abs(inscatterVert-inscatterEye);

		//inscattering = vec4(0, mix(float(isVertToSky),float(isEyeToSky), float(gl_FragCoord.x<960)),0,0);
	}
	else
	{
		float lenVert2Atmos = lenAtmos2Mid - lenVert2Mid; // meter
		float skyCoordPitch = GetSkyCoordPitch(lenVert2Atmos, lenVert2Top, lenVertAtmosHorizon);

		inscattering = Texture4D(vec4(
			skyCoordPitch,
			GetCoordUL(cosVertUL),
			coordYaw,
			elevCoord));
	}

	float dotVS = dot(viewDir, viewLight);
	const vec3 sunColor = vec3(1.0,0.8,0.6);
	vec3 atmosSum = (inscattering.rgb*RayleighPhase(dotVS) + inscattering.a*MiePhase(dotVS))*sunColor;

#ifdef EARTH
#else // not EARTH
	atmosSum = (atmosColorMatrix*vec4(atmosSum,1)).rgb;
#endif // EARTH
	return atmosSum;
}

#endif // ATMOS