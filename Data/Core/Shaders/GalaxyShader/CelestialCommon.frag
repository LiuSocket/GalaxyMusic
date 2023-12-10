#version 400 compatibility
#extension GL_EXT_texture_array : enable

#pragma import_defines(ATMOS, EARTH)

uniform vec3 viewUp;
uniform vec3 viewLight;
uniform float minDotUL;

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
uniform mat4 osg_ViewMatrixInverse;
uniform sampler3D inscatteringTex;

#ifdef EARTH
#else // not EARTH
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
// cosUV : [-1,1]
// coord : [0,1]
float GetCoordPitch(float cosUV)
{
	return 0.5 + cosUV * 0.5;
}

// get coord of cosUL (the cos of local Up dir & light source dir)
float GetCoordUL(float cosUL)
{
	return (cosUL-minDotUL)/(1-minDotUL);
}

// get coord of altitude
float GetCoordAlt(float alt, float horizonDisMax, float Rv)
{
	float horizonDis = sqrt(alt * alt + 2 * alt * Rv);
	return horizonDis / horizonDisMax;
}

// x = cosVL, y = cosUL, z = pitch, w = alt
vec4 Texture4D(vec4 coord)
{
	const float PITCH_NUM = 128.0;
	const float ALT_NUM = 16.0;

	float altI = coord.w*ALT_NUM;
	float z = coord.z * (PITCH_NUM - 1) / PITCH_NUM + 0.5 / PITCH_NUM; // [0.5/PITCH_NUM, 1 - 0.5/PITCH_NUM]

	vec3 UVW_0 = vec3(coord.xy, (min(floor(altI), ALT_NUM - 1) + z)/ALT_NUM);
	vec3 UVW_1 = vec3(coord.xy, (min(ceil(altI), ALT_NUM - 1) + z)/ALT_NUM);
	vec4 color_0 = texture(inscatteringTex, UVW_0);
	vec4 color_1 = texture(inscatteringTex, UVW_1);

	// don't know why the data at pos(0.999,0.999,0.999) is wrong, (less than zero)
	return mix(color_0, color_1, fract(altI));
}

// Rv: radius at the vertex point
vec3 AtmosColor(float vertAlt, vec3 viewVertPos, vec3 viewDir, vec3 viewVertUp, float Rv)
{
	// distance of horizon at top atmosphere
	float horizonDisMax = sqrt(atmosHeight * atmosHeight + 2 * atmosHeight * Rv);

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

	// sin & cos of horizon at near atmosphere pos, affected by celestial radius
	float sinNearHoriz = min(1, Rv / min(Ra, Rv + eyeAltitude));
	float cosNearHoriz = -sqrt(1-sinNearHoriz*sinNearHoriz);
	vec3 atmosNear = Texture4D(vec4(
		(cosVL-cosMinVL)/deltaCosVL,
		GetCoordUL(cosUL),
		GetCoordPitch(CosDH(cosUV, cosNearHoriz)),
		GetCoordAlt(eyeAltitude, horizonDisMax, Rv))).rgb;

	// sin & cos of horizon at vertex pos, affected by celestial radius
	float sinVertHoriz = min(1, Rv / (Rv + vertAlt));
	float cosVertHoriz = -sqrt(1-sinVertHoriz*sinVertHoriz);	
	vec3 atmosVert = Texture4D(vec4(
		(cosVL-cosVertMinVL)/deltaCosVertVL,
		GetCoordUL(cosVertUL),
		GetCoordPitch(CosDH(cosVertDir, cosVertHoriz)),
		GetCoordAlt(vertAlt, horizonDisMax, Rv))).rgb;
	vec3 atmosSum = max(vec3(0), atmosNear - atmosVert);

	if(eyeAltitude < groundTop)
	{
		float eye2Core = length(osg_ViewMatrixInverse[3].xyz);
		vec3 viewCorePos = -viewUp*eye2Core;
		float localRadius = eye2Core - eyeAltitude;
		float midAlt = eye2Core*sinUV-localRadius;
		vec3 viewMidPos = viewDir*dot(viewDir,viewCorePos)-viewCorePos;
		vec3 viewMidUp = normalize(viewMidPos);
		// cosMidUL = the cos of mid pos viewUp dir & light source dir
		float cosMidUL = dot(viewMidUp, viewLight);
		vec3 atmosMid = Texture4D(vec4(
			(cosVL-cosVertMinVL)/deltaCosVertVL, // to do
			GetCoordUL(cosMidUL),
			0.5, // to do
			GetCoordAlt(midAlt, horizonDisMax, Rv))).rgb;

		atmosSum = mix(atmosSum, abs(2*atmosMid-atmosVert-atmosNear), step(0,-cosUV)*step(0,-cosVertDir));
	}

#ifdef EARTH
	return atmosSum;
#else // not EARTH
	return (atmosColorMatrix*vec4(atmosSum,1)).rgb;
#endif // EARTH
}

#endif // ATMOS