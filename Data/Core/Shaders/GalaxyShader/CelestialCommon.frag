#version 400 compatibility

#pragma import_defines(ATMOS, EARTH, WANDERING)

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

// Rv: radius at the vertex point
vec3 AtmosColor(float vertAlt, vec3 viewVertPos, vec3 viewDir, vec3 viewVertUp, float Rv)
{
	vec3 ECEFEyePos = view2ECEFMatrix[3].xyz;
	float lenCore2Eye = length(ECEFEyePos);
	float eyeGeoRadius = lenCore2Eye - eyeAltitude;

	// vertex is up to eye: +, vertex is down to eye: -
	// cosVertDir(at the vertex pos):
	float cosVertDir = dot(viewVertUp, viewDir);
	float sinVertDir = sqrt(1 - cosVertDir*cosVertDir);
	// radius at the top atmos above the vertex point
	float Ra = Rv+atmosHeight;
	// distance of view line to core
	float Rt = sinVertDir*Rv;
	// distance between the vertex and the top atmosphere point through the view path
	float disVert2TopAtm = sqrt(Ra*Ra - Rt*Rt) - Rv*abs(cosVertDir);
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
	// cosVL = the cos of view dir & light source dir
	float cosVL = dot(viewDir, viewLight);

	vec2 localLightDir = vec2(sqrt(1 - cosUL*cosUL), cosUL);
	vec2 localViewFrontDir = vec2(sinUV, cosUV);
	vec2 localViewBackDir = vec2(-sinUV, cosUV);
	float cosMinVL = dot(localViewBackDir, localLightDir);
	float cosMaxVL = dot(localViewFrontDir, localLightDir);
	float deltaCosVL = cosMaxVL-cosMinVL;
	deltaCosVL = step(0, deltaCosVL) * max(1e-9, abs(deltaCosVL));

	float cosVertUV = dot(viewVertUp, viewDir);
	float sinVertUV = sqrt(1-cosVertUV*cosVertUV);
	// cosVertUL = the cos of vertex Up dir & light source dir
	float cosVertUL = dot(viewVertUp, viewLight);
	float sinVertUL = sqrt(1-cosVertUL*cosVertUL);

	vec2 vertLightDir = vec2(sqrt(1 - sinVertUL*sinVertUL), cosVertUL);
	vec2 vertViewFrontDir = vec2(sinVertUV, cosVertUV);
	vec2 vertViewBackDir = vec2(-sinVertUV, cosVertUV);
	float cosVertMinVL = dot(vertViewBackDir, vertLightDir);
	float cosVertMaxVL = dot(vertViewFrontDir, vertLightDir);
	float deltaCosVertVL = cosVertMaxVL-cosVertMinVL;
	deltaCosVertVL = step(0, deltaCosVertVL) * max(1e-9, abs(deltaCosVertVL));

	float nearEye = exp2(-length(viewVertPos)/atmosHeight);
	// sin & cos of eye pos horizon
	float sinEyeHoriz = min(1, eyeGeoRadius / lenCore2Eye);
	float cosEyeHoriz = -sqrt(1 - sinEyeHoriz * sinEyeHoriz);

	// sin & cos of horizon at near atmosphere pos, affected by celestial radius
	float sinNearHoriz = min(1, Rv / min(Ra, Rv + eyeAltitude));
	float cosNearHoriz = -sqrt(1-sinNearHoriz*sinNearHoriz);
	cosNearHoriz = mix(cosNearHoriz, cosEyeHoriz, nearEye);
	vec3 atmosNear = Texture4D(vec4(
		GetCoordPitch(CosDH(cosUV, cosNearHoriz)),
		GetCoordUL(cosUL),
		(cosMaxVL-cosVL)/deltaCosVL,
		min(1, sqrt(eyeAltitude / atmosHeight)))).rgb;

	// sin & cos of horizon at vertex pos, affected by celestial radius
	float sinVertHoriz = min(1, Rv / (Rv + vertAlt));
	float cosVertHoriz = -sqrt(1-sinVertHoriz*sinVertHoriz);
	cosVertHoriz = mix(cosVertHoriz, cosEyeHoriz, nearEye);	
	vec3 atmosVert = Texture4D(vec4(
		GetCoordPitch(CosDH(cosVertDir, cosVertHoriz)),
		GetCoordUL(cosVertUL),
		(cosVertMaxVL-cosVL)/deltaCosVertVL,
		min(1, sqrt(vertAlt / atmosHeight)))).rgb;
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