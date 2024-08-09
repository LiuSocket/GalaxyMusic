#version 400 compatibility

#pragma import_defines(ATMOS, EARTH, WANDERING)

const float M_PI = 3.141592654;

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
uniform float groundTop;
uniform mat4 view2ECEFMatrix;
uniform mat4 osg_ViewMatrixInverse;
uniform sampler3D inscatteringTex;

#ifndef EARTH
uniform mat4 atmosColorMatrix;
#endif // EARTH

// get coord of pitch (d0/dH or d0/dh)
// d0 = distance of eye to WGS84 ground
// dv = distance of eye to the ground point vertical below
// dh = distance of horizon
// dH = distance of atmos top behind horizon
float GetCoordPitch(float d0, float dv, float dh, float dH, bool isSky)
{
	float dMinGround = max(0.0, dv / dh);
	float dMinAtmos = max(0.0, atmosHeight - eyeAltitude) / dH;
	return isSky ?
		(1 - 0.5 * (d0 / dH - dMinAtmos) / (1 - dMinAtmos)) :
		(0.5 * (d0 / dh - dMinGround) / (1 - dMinGround));
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

// Rv: radius at the vertex point
vec3 AtmosColor(float vertAlt, vec3 viewVertPos, vec3 viewDir, vec3 viewVertUp, float Rv)
{
	vec3 ECEFEyePos = view2ECEFMatrix[3].xyz;
	float lenCore2Eye = length(ECEFEyePos);
	float lenCore2Eye2 = lenCore2Eye*lenCore2Eye;
	float eyeGeoRadius = lenCore2Eye - eyeAltitude;
	float altRatio = max(0.0, eyeAltitude / atmosHeight);
	float Rt = atmosHeight + Rv;
	float Rt2 = Rt*Rt;
	float Rv2 = Rv*Rv;

	//cosUV(at the eye pos)
	float cosUV = dot(viewUp, viewDir);
	// cosUL = the cos of local Up dir & light source dir
	float cosUL = dot(viewUp, viewLight);
	// vertex is up to eye: -, vertex is down to eye: +
	// cosVertUV(at the vertex pos):
	float cosVertUV = dot(viewVertUp, viewDir);
	// cosVertUL = the cos of vertex Up dir & light source dir
	float cosVertUL = dot(viewVertUp, viewLight);
	
	vec3 viewLightRightDir = normalize(cross(viewLight, viewVertUp));
	vec3 viewLightFrontDir = normalize(cross(viewVertUp, viewLightRightDir));
	mat3 localLight2ViewMatrix = mat3(
		viewLightRightDir.x,	viewLightRightDir.y,	viewLightRightDir.z,
		viewLightFrontDir.x,	viewLightFrontDir.y,	viewLightFrontDir.z,
		viewVertUp.x,			viewVertUp.y,			viewVertUp.z);
	mat3 view2LocalLightMatrix = inverse(localLight2ViewMatrix);
	vec3 localLightSpaceViewDir = view2LocalLightMatrix*viewDir;
	float coordYaw = acos(-localLightSpaceViewDir.y)/M_PI; // ground is the inverse view dir

	float elev = max(0.0, vertAlt);
	// sin & cos of horizon at vertex pos, affected by celestial radius
	float sinVertHoriz = min(1.0, Rv / (Rv + elev)); // it must < 1
	float cosVertHoriz = -sqrt(1.0 - sinVertHoriz * sinVertHoriz); // it must < 0
	
	float lenEye2Far = lenCore2Eye2 + Rt2 + 2.0*lenCore2Eye*Rt*cosUV;
	float lenVert2Core = elev + Rv;
	float lenVertHorizon = sqrt(max(0.0, lenVert2Core*lenVert2Core - Rv2));

	float lenHalfAtmosHorizon = sqrt(Rt2 - Rv2);
	float lenVertAtmosHorizon = lenVertHorizon + lenHalfAtmosHorizon;

	vec4 inscatterVert = Texture4D(vec4(
		GetCoordPitch(lenEye2Far, elev, lenVertHorizon, lenVertAtmosHorizon, (-cosVertUV)>cosVertHoriz),
		GetCoordUL(cosVertUL),
		coordYaw,
		min(1.0, sqrt(elev / atmosHeight))));
	vec4 inscattering = inscatterVert;

	if(eyeAltitude < atmosHeight)
	{
		// sin & cos of eye pos horizon
		float sinEyeHoriz = eyeGeoRadius / lenCore2Eye; // it must < 1
		float cosEyeHoriz = -sqrt(1.0 - sinEyeHoriz * sinEyeHoriz); // it must < 0

		float lenHorizon = sqrt(lenCore2Eye2 - Rv2);
		float lenAtmosHorizon = lenHorizon + lenHalfAtmosHorizon;
		vec4 inscatterEye = Texture4D(vec4(
			GetCoordPitch(lenEye2Far, eyeAltitude, lenHorizon, lenAtmosHorizon, (-cosUV)>cosEyeHoriz),
			GetCoordUL(cosUL),
			coordYaw,
			min(1.0, sqrt(altRatio))));
		inscattering = max(vec4(0.0), inscatterEye-inscatterVert);
	}

	float dotVS = dot(viewDir, viewLight);
	vec3 sunColor = pow(vec3(1.0,0.8,0.2), vec3(5.0-4.0*sqrt(max(0,cosUL*10))*exp2(-altRatio*0.5)));
	vec3 atmosSum = inscattering.rgb*RayleighPhase(dotVS) + inscattering.a*MiePhase(dotVS)*sunColor;

#ifdef EARTH
#else // not EARTH
	atmosSum = (atmosColorMatrix*vec4(atmosSum,1)).rgb;
#endif // EARTH
	return atmosSum;
}

#endif // ATMOS