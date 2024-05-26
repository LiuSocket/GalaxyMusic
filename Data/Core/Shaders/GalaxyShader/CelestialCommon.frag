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

// cosDH = cosDir with the horizon fact
float CosDH(float cosDir, float cosHoriz)
{
	float deltaCos = cosDir-cosHoriz;
	float isSky = step(0, deltaCos);
	return deltaCos/mix(1+cosHoriz, 1-cosHoriz, isSky);
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
	float altI = coord.w*ALT_NUM;
	vec3 UVW_0 = vec3(coord.xy, (min(floor(altI), ALT_NUM - 1) + coord.z)/ALT_NUM);
	vec3 UVW_1 = vec3(coord.xy, (min(ceil(altI), ALT_NUM - 1) + coord.z)/ALT_NUM);
	vec4 color_0 = texture(inscatteringTex, UVW_0);
	vec4 color_1 = texture(inscatteringTex, UVW_1);

	return mix(color_0, color_1, fract(altI));
}

// Rv: radius at the vertex point
vec3 AtmosColor(float vertAlt, vec3 viewVertPos, vec3 viewDir, vec3 viewVertUp, float Rv)
{
	vec3 ECEFEyePos = view2ECEFMatrix[3].xyz;
	float lenCore2Eye = length(ECEFEyePos);
	float eyeGeoRadius = lenCore2Eye - eyeAltitude;

	// vertex is up to eye: +, vertex is down to eye: -
	// cosVertUV(at the vertex pos):
	float cosVertUV = dot(viewVertUp, viewDir);
	float sinVertUV = sqrt(1-cosVertUV*cosVertUV);
	// radius at the top atmos above the vertex point
	float Ra = Rv+atmosHeight;
	// distance of view line to core
	float Rt = sinVertUV*Rv;
	// distance between the vertex and the top atmosphere point through the view path
	float disVert2TopAtm = sqrt(Ra*Ra - Rt*Rt) - Rv*abs(cosVertUV);
	vec3 viewTopAtmPos = viewVertPos - viewDir*disVert2TopAtm;
	vec3 viewCorePos = viewVertPos - viewVertUp*Rv;
	vec3 viewTopAtmUp = normalize(viewTopAtmPos-viewCorePos);

	vec3 viewNearUp = viewTopAtmUp;
	if(eyeAltitude < atmosHeight)
	{
		viewNearUp = viewUp;
	}
	//cosUV(at the atmos top pos or eye pos)
	float cosUV = dot(viewNearUp, viewDir);
	float sinUV = sqrt(1-cosUV*cosUV);
	// cosUL = the cos of local Up dir & light source dir
	float cosUL = dot(viewNearUp, viewLight);
	// cosVertUL = the cos of vertex Up dir & light source dir
	float cosVertUL = dot(viewVertUp, viewLight);

	vec3 viewLightRightDir = normalize(cross(viewLight, viewNearUp));
	vec3 viewLightFrontDir = normalize(cross(viewNearUp, viewLightRightDir));
	mat3 localLight2ViewMatrix = mat3(
		viewLightRightDir.x,	viewLightRightDir.y,	viewLightRightDir.z,
		viewLightFrontDir.x,	viewLightFrontDir.y,	viewLightFrontDir.z,
		viewNearUp.x,			viewNearUp.y,			viewNearUp.z);
	mat3 view2LocalLightMatrix = inverse(localLight2ViewMatrix);
	vec3 localLightSpaceViewDir = view2LocalLightMatrix*viewDir;
	float coordYaw = acos(localLightSpaceViewDir.y)/M_PI;

	float nearEye = exp2(-length(viewVertPos)/atmosHeight);
	// sin & cos of eye pos horizon
	float sinEyeHoriz = min(1, eyeGeoRadius / lenCore2Eye);
	float cosEyeHoriz = -sqrt(1 - sinEyeHoriz * sinEyeHoriz);

	// sin & cos of horizon at near atmosphere pos, affected by celestial radius
	float sinNearHoriz = min(1, Rv / min(Ra, Rv + eyeAltitude));
	float cosNearHoriz = -sqrt(1-sinNearHoriz*sinNearHoriz);
	cosNearHoriz = mix(cosNearHoriz, cosEyeHoriz, nearEye);
	float altRatio = eyeAltitude / atmosHeight;
	vec3 atmosNear = Texture4D(vec4(
		GetCoordPitch(CosDH(cosUV, cosNearHoriz), altRatio),
		GetCoordUL(cosUL),
		coordYaw,
		min(1, sqrt(altRatio)))).rgb;

	// sin & cos of horizon at vertex pos, affected by celestial radius
	float sinVertHoriz = min(1, Rv / (Rv + vertAlt));
	float cosVertHoriz = -sqrt(1-sinVertHoriz*sinVertHoriz);
	cosVertHoriz = mix(cosVertHoriz, cosEyeHoriz, nearEye);
	float vertAltRatio = vertAlt / atmosHeight;
	vec3 atmosVert = Texture4D(vec4(
		GetCoordPitch(CosDH(cosVertUV, cosVertHoriz), vertAltRatio),
		GetCoordUL(cosVertUL),
		coordYaw,
		min(1, sqrt(vertAltRatio)))).rgb;
	vec3 atmosSum = abs(atmosNear - atmosVert);

#ifdef EARTH
#ifdef WANDERING
	float meanSum = (atmosSum.r+atmosSum.g+atmosSum.b)*0.33;
	atmosSum = mix(atmosSum, vec3(1.0,1.2,1.0)*meanSum, 0.5*wanderProgress);
#else // not WANDERING
#endif // WANDERING
#else // not EARTH
	atmosSum = (atmosColorMatrix*vec4(atmosSum,1)).rgb;
#endif // EARTH
	return atmosSum;
}

#endif // ATMOS