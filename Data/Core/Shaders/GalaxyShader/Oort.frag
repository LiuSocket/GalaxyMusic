#version 400 compatibility

#pragma import_defines(RAYS_2, RAYS_3)

struct commonParam {
	vec4 norm;
	vec3 WCP;
	vec3 WVD;
	float dotNorm;
	float lenUnit;
	float lenMin;
	float lenMax;
	float noiseD;
};
const float hash[256] = float[](
	151,160,137, 91, 90, 15,131, 13,201, 95, 96, 53,194,233,  7,225,
	140, 36,103, 30, 69,142,  8, 99, 37,240, 21, 10, 23,190,  6,148,
	247,120,234, 75,  0, 26,197, 62, 94,252,219,203,117, 35, 11, 32,
	 57,177, 33, 88,237,149, 56, 87,174, 20,125,136,171,168, 68,175,
	 74,165, 71,134,139, 48, 27,166, 77,146,158,231, 83,111,229,122,
	 60,211,133,230,220,105, 92, 41, 55, 46,245, 40,244,102,143, 54,
	 65, 25, 63,161,  1,216, 80, 73,209, 76,132,187,208, 89, 18,169,
	200,196,135,130,116,188,159, 86,164,100,109,198,173,186,  3, 64,
	 52,217,226,250,124,123,  5,202, 38,147,118,126,255, 82, 85,212,
	207,206, 59,227, 47, 16, 58, 17,182,189, 28, 42,223,183,170,213,
	119,248,152,  2, 44,154,163, 70,221,153,101,155,167, 43,172,  9,
	129, 22, 39,253, 19, 98,108,110, 79,113,224,232,178,185,112,104,
	218,246, 97,228,251, 34,242,193,238,210,144, 12,191,179,162,241,
	 81, 51,145,235,249, 14,239,107, 49,192,214, 31,181,199,106,157,
	184, 84,204,176,115,121, 50, 45,127,  4,150,254,138,236,205, 93,
	222,114, 67, 29, 24, 72,243,141,128,195, 78, 66,215, 61,156,180
);

const int STEP_NUM = 64;
const float LAST_SCLAE = 32;
const float RANGE_MAX = 6;
const float M_PI = 3.141592657;
const float ALPHA_MAX = 0.99;

uniform float level[128];
uniform float times;
uniform float unit;
uniform float pixelLength;
uniform float oortVisible;
uniform vec2 shakeVec;
uniform vec2 deltaShakeVec;
uniform vec3 screenSize;
uniform vec3 eyeFrontDir;
uniform vec3 eyeRightDir;
uniform vec3 eyeUpDir;
uniform vec3 starWorldPos;
uniform vec4 noiseVec4;
uniform vec4 playingStarColor;
uniform mat4 invProjMatrix;
uniform mat4 deltaViewProjMatrix;
uniform mat4 osg_ViewMatrixInverse;
uniform mat4 attitudeMatrix;

uniform sampler2D lastVectorTex;
uniform sampler2D blueNoiseTex;
uniform sampler3D noiseShapeTex;
uniform sampler3D noiseErosionTex;

float stepLen = 1e-2*unit;
float OortRadius = 10*unit;

in vec3 weight;
in vec3 localVertDir;

// (length min,length max)
vec2 LenMinMax(vec3 worldCorePos, float radius, vec3 worldEyePos, vec3 worldVertDir)
{
	float dotCD = dot(worldCorePos-worldEyePos, worldVertDir);
	vec3 worldCorePointM = worldEyePos + dotCD*worldVertDir;
	float dstCore = distance(worldCorePos, worldCorePointM)/radius;
	float lenCoreH = radius*sqrt(max(0,1-dstCore*dstCore));
	vec2 lenMinMax = max(vec2(0), dotCD + vec2(-lenCoreH, lenCoreH));
	return mix(lenMinMax, vec2(1e9, -1e9), step(1, dstCore));
}

vec3 ToneMapping(vec3 color)
{
	const float A = 2.51;
	const float B = 0.03;
	const float C = 2.43;
	const float D = 0.59;
	const float E = 0.14;
	return pow((color * (A * color + B)) / (color * (C * color + D) + E), vec3(1.0/2.2));
}

void Oort(commonParam cP, inout vec4 oortColor, inout float lenOort)
{
	float lenUnitMin = cP.lenUnit * cP.norm.w;
	float lenD = min(40*unit, cP.lenMax - cP.lenMin);
	vec4 oortC = vec4(0);
	float hasOort = 0;
	float lenFirstRange = STEP_NUM * cP.lenUnit;
	float isInRange = step(0.0,lenFirstRange*(exp2(RANGE_MAX)*(1+LAST_SCLAE)-1)-cP.lenMin);
	float jStart = min(RANGE_MAX-isInRange, floor(log2(cP.lenMin/lenFirstRange+1)));
	float jEnd = min(RANGE_MAX, ceil(log2(cP.lenMax/lenFirstRange+1)));
	for(int j = int(jStart); all(bvec2(oortC.a < ALPHA_MAX, j < jEnd)); j++)
	{
		float lastScale = mix(1.0, LAST_SCLAE, step(RANGE_MAX, j+1.5));
		float numJ = exp2(j);
		float lenUnitMinJ = lenUnitMin*numJ;
		float lenUnitJ = cP.lenUnit * numJ;
		float parity = step(lenUnitMinJ, mod(cP.dotNorm, 2*lenUnitMinJ));
		float lenStartJ = mod(cP.dotNorm, lenUnitMinJ)/cP.norm.w;
		float lenMinJ = lenStartJ + lenFirstRange * (numJ-1);
		float stepNumJ = STEP_NUM*lastScale;
		float lenRange = lenFirstRange*numJ*lastScale;
		float startStepNum = floor(stepNumJ*max(0, cP.lenMin-lenMinJ)/lenRange);
		float lenStartStep = lenMinJ + cP.noiseD*lenUnitJ*0.1;
		float lenStep = startStepNum*lenUnitJ; // from (i*lenUnitJ) to (lenRange)
		float lenS = lenStartStep + lenStep; // distance from eye to this step
		for(int i = int(startStepNum);all(bvec3(oortC.a<ALPHA_MAX, lenStep<(lenRange*0.999), lenS<(cP.lenMin+lenD))); i++)
		{
			lenS = lenStartStep + lenStep*(1-0.01*cP.noiseD);
			vec3 stepWorldPos = cP.WCP + cP.WVD*lenS;
			vec3 stepPolarPos = (attitudeMatrix*vec4(stepWorldPos-starWorldPos,1)).xyz;
			vec3 stepPolarOutDir = normalize(stepPolarPos);
			float radiusRatio = max(1e-10, length(stepPolarPos) / OortRadius);
			float disRipple = mod(128.0 * radiusRatio, 128.0);
			float audioLevel = level[int(disRipple)];

			float theta = 0.02*times + (exp2(-radiusRatio*(8-3*abs(stepPolarOutDir.z)))-1)*3;
			float cosTheta = cos(theta);
			float sinTheta = sin(theta);
			mat4 rotateMatrix = mat4(
				cosTheta,	-sinTheta,	0,	0,
				sinTheta,	cosTheta,	0,	0,
				0,					0,	1,	0,
				0,					0,	0,	1);
			vec3 newPos = (rotateMatrix*vec4(stepPolarPos,1)).xyz;
			vec3 coord3D = (2-1.6*radiusRatio)*newPos/OortRadius;
			vec4 shape4 = texture3D(noiseShapeTex, coord3D);
			shape4 *= vec4(0.4,
				0.2-0.05*audioLevel,
				0.1-0.05*audioLevel,
				0.05-0.1*audioLevel);
			float texDens = max(0, 1-(shape4.x+shape4.y+shape4.z+shape4.w))*numJ/exp2(RANGE_MAX-1);
			// fade for different range
			texDens *= mix(1, clamp((1-lenStep/lenRange)*2,0,1), mod(i+parity,2));
			// fade around the camera
			texDens *= 1-exp2(-lenS/stepLen);

			float cloudA = 1 - exp2(-texDens);
			cloudA *= min(1, radiusRatio*radiusRatio*radiusRatio + 0.5*pow(1-abs(stepPolarOutDir.z),7)) * (1 - radiusRatio);
			vec3 chrome = mix(playingStarColor.rgb, vec3(1), 0.5*exp2(-radiusRatio*10)) * (1 - 0.7*audioLevel*audioLevel);
			oortC += vec4(chrome*cloudA, cloudA)*(1-oortC.a);

			lenStep += lenUnitJ;
			hasOort = min(hasOort + step(0.6,oortC.a), 1);
			lenOort = mix(lenS, lenOort, hasOort);
		}
	}
	oortC.rgb /= 0.001+oortC.a;
	float oortAlpha = oortVisible*oortC.a/ALPHA_MAX;
	oortColor = mix(oortColor, vec4(ToneMapping(oortC.rgb), oortAlpha), oortAlpha);
}

void main() 
{
	vec2 cf = step(1.0, mod(gl_FragCoord.xy,2.0));
	float checkBox = step(0.5, mod(cf.x + cf.y, 2.0));
	vec2 shakeV2 = mix(shakeVec, vec2(shakeVec.y, -shakeVec.x), checkBox);
	vec3 localDir = normalize(localVertDir);
	vec3 localNear = localDir/dot(localDir,eyeFrontDir);
	float lenNearClipX = dot(localNear,eyeRightDir);
	float lenNearClipY = dot(localNear,eyeUpDir);
	vec3 localShake = (eyeRightDir*shakeV2.x + eyeUpDir*shakeV2.y)*pixelLength;
	// after shake
	localDir = normalize(localNear + localShake);
	vec3 WVD = localDir;
	vec3 WCP = osg_ViewMatrixInverse[3].xyz;

	vec2 rayMarchScreenSize = screenSize.xy*screenSize.z;
	vec2 projCoord = gl_FragCoord.xy/rayMarchScreenSize;
	vec3 viewDir = (invProjMatrix * vec4(projCoord*2-1, 1, 1)).xyz; 
	viewDir = normalize(viewDir);

	float noiseD = texture2D(blueNoiseTex, gl_FragCoord.xy*screenSize.z/(abs(fract(times*20)-0.5)*10+64)).r;

	// bounding shape
	vec2 lenMinMax = LenMinMax(starWorldPos, OortRadius, WCP, WVD);
	float lenMin = lenMinMax.x;
	float lenMax = lenMinMax.y;

	// back position velocity difference
	vec3 backPosDiff = (deltaViewProjMatrix*vec4(WCP + WVD*lenMax,1)).xyz/max(1, lenMax);

	vec3 normal_0 = normalize(gl_TexCoord[0].xyz);
	vec4 norm_0 = vec4(normal_0, dot(localDir,normal_0));
	vec4 norm_1 = norm_0;
	vec4 norm_2 = norm_0;

#ifdef RAYS_2
	vec3 normal_1 = normalize(gl_TexCoord[1].xyz);
	norm_1 = vec4(normal_1, dot(localDir,normal_1));
#endif // RAYS_2
#ifdef RAYS_3
	vec3 normal_2 = normalize(gl_TexCoord[2].xyz);
	norm_2 = vec4(normal_2, dot(localDir,normal_2));
#endif // RAYS_3

	vec3 dotNorm = vec3(
		dot(-WCP, norm_0.xyz),
		dot(-WCP, norm_1.xyz),
		dot(-WCP, norm_2.xyz));
	vec3 nd = vec3(norm_0.w, norm_1.w, norm_2.w);
	vec3 lenStart = mod(dotNorm, stepLen)/nd;
	vec3 lenUnit = stepLen/nd;

	commonParam cP;
	cP.norm = norm_0;
	cP.WCP = WCP;
	cP.WVD = WVD;
	cP.dotNorm = dotNorm.x;
	cP.lenUnit = lenUnit.x;
	cP.lenMin = max(lenMin,lenStart.x);
	cP.lenMax = lenMax;
	cP.noiseD = noiseD;

	vec4 oortColor = vec4(0);
	float oortAlpha = 1.0;
	float lenOort = cP.lenMin;// oort surface length

	float lenOort_0 = cP.lenMin;
	vec4 oortC_0 = vec4(0);
	Oort(cP, oortC_0, lenOort_0);

	lenOort = lenOort_0;
	oortColor = oortC_0;

#ifdef RAYS_2
	cP.norm = norm_1;
	cP.dotNorm = dotNorm.y;
	cP.lenUnit = lenUnit.y;
	cP.lenMin = max(lenMin,lenStart.y);

	float lenOort_1 = cP.lenMin;
	vec4 oortC_1 = vec4(0);
	Oort(cP, oortC_1, lenOort_1);
	
	lenOort = mix(lenOort, lenOort_1, weight.y);
	oortColor = mix(oortColor, oortC_1, weight.y);
#endif // RAYS_2

#ifdef RAYS_3
	cP.norm = norm_2;
	cP.dotNorm = dotNorm.z;
	cP.lenUnit = lenUnit.z;
	cP.lenMin = max(lenMin,lenStart.z);

	float lenOort_2 = cP.lenMin;
	vec4 oortC_2 = vec4(0);
	Oort(cP, oortC_2, lenOort_2);
		
	lenOort = mix(lenOort, lenOort_2, weight.z);
	oortColor = mix(oortColor, oortC_2, weight.z);
#endif // RAYS_3

	oortAlpha = oortColor.a;

	vec3 posDiff = ((deltaViewProjMatrix*vec4(WCP+lenOort*WVD,1.0)).xyz)/max(1, lenOort);
	// position different
	gl_FragData[0] = vec4(mix(backPosDiff, posDiff, oortAlpha), 1);

	// color and alpha
	gl_FragData[1] = vec4(oortColor.rgb, oortAlpha);
}