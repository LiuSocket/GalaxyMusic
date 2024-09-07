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

	vec3 viewDir = normalize(viewPos.xyz);
	vec3 ECEFEyePos = view2ECEFMatrix[3].xyz;
	float Rc = length(ECEFEyePos);
	//cosUV(at the eye pos)
	float cosUV = dot(viewUp, viewDir);
	float signDisEye2Mid = -Rc*cosUV;
	float lenEye2MidPos = max(0, signDisEye2Mid); // >0
	vec3 viewMidPos = viewDir*lenEye2MidPos;
	vec3 ECEFMidPos = (view2ECEFMatrix*vec4(viewMidPos, 1.0)).xyz;
	float Rg = GeoRadius(planetRadius.x, planetRadius.y, abs(normalize(ECEFMidPos).z));
	float Rt = Rg + atmosHeight;
	float Rg2 = Rg*Rg;
	float Rt2 = Rt*Rt;
	float lenHorizonMax = sqrt(Rt2 - Rg2);

	vec4 inscattering = vec4(0);
	if(eyeAltitude < atmosHeight)
	{
		float Rc2 = Rc*Rc;
		// sinVertUV(at the vertex pos):
		float sinUV2 = max(0, 1-cosUV*cosUV);
		// cosUL = cos of viewUp & light
		float cosUL = dot(viewUp, viewLight);

		vec3 viewLightRightDir = normalize(cross(viewLight, viewUp));
		vec3 viewLightFrontDir = normalize(cross(viewUp, viewLightRightDir));
		mat3 localLight2ViewMatrix = mat3(
			viewLightRightDir.x,	viewLightRightDir.y,	viewLightRightDir.z,
			viewLightFrontDir.x,	viewLightFrontDir.y,	viewLightFrontDir.z,
			viewUp.x,				viewUp.y,				viewUp.z);
		mat3 view2LocalLightMatrix = inverse(localLight2ViewMatrix);
		vec3 localLightSpaceViewDir = view2LocalLightMatrix*viewDir;
		float coordYaw = acos(localLightSpaceViewDir.y)/M_PI;

		float lenEye2Atmos = signDisEye2Mid + sqrt(max(0.0, Rt2 - Rc2*sinUV2));
		float lenEye2Top = max(0.0, atmosHeight - eyeAltitude);
		float lenEye2Horizon = sqrt(max(0.0, Rc2 - Rg2));
		float lenAtmosHorizon = lenEye2Horizon + lenHorizonMax;
			
		inscattering = Texture4D(vec4(
			GetSkyCoordPitch(lenEye2Atmos, lenEye2Top, lenAtmosHorizon),
			GetCoordUL(cosUL),
			coordYaw,
			min(1, sqrt(eyeAltitude / atmosHeight))));
	}
	else // out of atmosphere
	{
		vec3 viewCorePos = -viewUp*Rc;
		vec3 viewCore2Mid = viewMidPos - viewCorePos;
		float lenCore2Mid = length(viewCore2Mid);
		float lenMid2Near = sqrt(max(0, Rt2 - lenCore2Mid*lenCore2Mid));
		vec3 viewNearPos = (viewMidPos - lenMid2Near*viewDir);
		vec3 viewCore2NearPos = viewNearPos - viewCorePos;
		vec3 viewNearUp = normalize(viewCore2NearPos);
		// cosNUL = viewUp at near atmosphere pos & light
		float cosNUL = dot(viewNearUp, viewLight);

		vec3 viewLightRightDir = normalize(cross(viewLight, viewNearUp));
		vec3 viewLightFrontDir = normalize(cross(viewNearUp, viewLightRightDir));
		mat3 localLight2ViewMatrix = mat3(
			viewLightRightDir.x,	viewLightRightDir.y,	viewLightRightDir.z,
			viewLightFrontDir.x,	viewLightFrontDir.y,	viewLightFrontDir.z,
			viewNearUp.x,			viewNearUp.y,			viewNearUp.z);
		mat3 view2LocalLightMatrix = inverse(localLight2ViewMatrix);
		vec3 localLightSpaceViewDir = view2LocalLightMatrix*viewDir;
		float coordYaw = acos(localLightSpaceViewDir.y)/M_PI;

		inscattering = Texture4D(vec4(
			GetSkyCoordPitch(2*lenMid2Near, 0.0, 2*lenHorizonMax),
			GetCoordUL(cosNUL),
			coordYaw,
			1.0));
	}

	float dotVS = dot(viewDir, viewLight);
	const vec3 sunColor = vec3(1.0,0.8,0.6);
	vec3 atmosSum = (inscattering.rgb*RayleighPhase(dotVS) + inscattering.a*MiePhase(dotVS))*sunColor;

#ifdef EARTH
#else // not EARTH
	atmosSum = (atmosColorMatrix*vec4(atmosSum,1)).rgb;
#endif // EARTH
	vec3 color = ToneMapping(atmosSum * (1 - shadow));
	float alpha = 1-exp2(-(atmosSum.r+atmosSum.g+atmosSum.b)*9);
	alpha *= 1 - shadow;
	gl_FragColor = vec4(color, alpha);
}