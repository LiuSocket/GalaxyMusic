#version 400 compatibility

#pragma import_defines(BRAKE_TIME, TORQUE_TIME_0, TORQUE_TIME_1)
#pragma import_defines(RAYS_2, RAYS_3)

struct commonParam {
	vec4 norm;
	vec3 modelEyePos;
	vec3 modelPixDir;
	vec3 modelSunDir;
	float scattering;
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

const float PROGRESS_0 =	0.005;
const float PROGRESS_1 =	0.03; // end of brake time
const float PROGRESS_1_1 =	0.035;
const float PROGRESS_2 =	0.09; // middle of torque time
const float PROGRESS_2_1 =	0.1;
const float PROGRESS_3 =	0.15; // end of torque time
const float PROGRESS_4 =	0.152;

const int STEP_NUM = 64;
const int LIGHT_SAMPLE = 4;
const float LAST_SCLAE = 128;
const float RANGE_MAX = 3;
const float M_PI = 3.141592654;
const float ALPHA_MAX = 0.99;

uniform float unit;
uniform float times;
uniform float pixelLength;
uniform float tailVisible;
uniform float wanderProgress;
uniform vec2 shakeVec;
uniform vec2 deltaShakeVec;
uniform vec3 engineStartRatio;
uniform vec3 screenSize;
uniform vec3 eyeFrontDir;
uniform vec3 eyeRightDir;
uniform vec3 eyeUpDir;
uniform vec3 viewLight;
uniform vec4 noiseVec4;
uniform mat4 invProjMatrix;
uniform mat4 deltaViewProjMatrix;
uniform mat4 osg_ViewMatrixInverse;
// world up is "y", earth tail direction is near "z", 
uniform mat4 world2ECEFMatrix;
uniform mat4 view2ECEFMatrix;
uniform mat4 world2SpiralMatrix;
uniform mat4 view2SpiralMatrix;

uniform sampler2D lastVectorTex;
uniform sampler2D blueNoiseTex;
uniform sampler3D noiseShapeTex;
uniform sampler3D noiseErosionTex;

float STEP_LENGTH = 1e4/unit;
float EARTH_RADIUS = 6.36e6/unit;
float ATMOS_RADIUS = 7.2e6/unit;
float TAIL_RADIUS = 1.7e6/unit;
float TAIL_LENGTH = 4e7/unit;
float LENGTH_MAX = 4e7/unit;
float TAIL_ATMOS_RATIO = TAIL_RADIUS / ATMOS_RADIUS;
float EARTH_ATMOS_RATIO = EARTH_RADIUS / ATMOS_RADIUS;
float LIGHT_LEN[LIGHT_SAMPLE] = float[](5e4/unit, 1e4/unit, 1e6/unit, 1e7/unit);

in vec3 weight;
in vec3 localVertDir;

vec2 SphereLenMinMax(float sphereR, vec3 modelSpherePos, vec3 modelEyePos, vec3 modelPixDir, out float dstTail)
{
	float dotSD = dot(modelSpherePos-modelEyePos, modelPixDir);
	vec3 tailPointM = modelEyePos + dotSD*modelPixDir;
	dstTail = distance(modelSpherePos, tailPointM)/sphereR;
	float lenTailH = sphereR*sqrt(max(0,1-dstTail*dstTail));
	vec2 lenTailMinMax = dotSD + vec2(-lenTailH, lenTailH);
	return mix(lenTailMinMax, vec2(1e9,0.0), float(1 < dstTail));
}

vec2 SphereLenMinMax(float sphereR, vec3 modelSpherePos, vec3 modelEyePos, vec3 modelPixDir)
{
	float dstTail = 0;
	return SphereLenMinMax(sphereR, modelSpherePos, modelEyePos, modelPixDir, dstTail);
}

#ifdef BRAKE_TIME
// (length min,length max)
// dstEarth means the normalized distance to Earth core, [0,1]
vec2 LenMinMax(vec3 modelEyePos, vec3 modelPixDir, out float dstEarth)
{
	// cull by earth
	float dotED = dot(-modelEyePos, modelPixDir);
	vec3 modelCorePointM = modelEyePos + dotED*modelPixDir;
	dstEarth = length(modelCorePointM)/EARTH_RADIUS;
	float lenEarthH = EARTH_RADIUS*sqrt(max(0,1-dstEarth*dstEarth));
	vec2 lenMinMax = vec2(0, 1e9);

	// tail sphere
	float radiusI = EARTH_RADIUS * 15;
	for(int i = 0 ; i < 2 ; i ++)
	{
		vec3 spherePosI = vec3(0.0, 0.0, (float(i)*2-1)*EARTH_RADIUS*14.9);
		vec2 lenTailMinMax = SphereLenMinMax(radiusI, spherePosI, modelEyePos, modelPixDir);

		lenMinMax.x = max(lenMinMax.x, lenTailMinMax.x);
		lenMinMax.y = min(lenMinMax.y, lenTailMinMax.y);
	}

	lenMinMax.y = min(lenMinMax.y, dotED - lenEarthH + 1e9*float(dstEarth>1));
	return lenMinMax;
}

float AtmosDens(vec3 modelStepPos, vec3 modelStepDir)
{
	float ratioZ = modelStepPos.z / ATMOS_RADIUS;
	float ratioXY = length(modelStepPos.xy) / ATMOS_RADIUS;
	float ratioR = length(modelStepPos) / ATMOS_RADIUS;
	float texDens = 0.1;
	return texDens;
}

#elif (defined(TORQUE_TIME_0) || defined(TORQUE_TIME_1))
// (length min,length max)
// dstEarth means the normalized distance to Earth core, [0,1]
vec2 LenMinMax(vec3 modelEyePos, vec3 modelPixDir, out float dstEarth)
{
	// cull by earth
	float dotED = dot(-modelEyePos, modelPixDir);
	vec3 modelCorePointM = modelEyePos + dotED*modelPixDir;
	dstEarth = length(modelCorePointM)/EARTH_RADIUS;
	float lenEarthH = EARTH_RADIUS*sqrt(max(0,1-dstEarth*dstEarth));
	vec2 lenMinMax = vec2(1e9, 0.0);

	// tail sphere
	float radiusI = EARTH_RADIUS*1.03;
	for(int i = 0 ; i < 2 ; i ++)
	{
		vec3 sphereOutPosI = (float(i)*2-1)*EARTH_RADIUS*vec3(0.0, -0.1, 0.3);
#ifdef TORQUE_TIME_1
		sphereOutPosI.z *= -1;
#endif // TORQUE_TIME_0
		vec2 lenOutMinMax = SphereLenMinMax(radiusI, sphereOutPosI, modelEyePos, modelPixDir);

		lenMinMax.x = min(lenMinMax.x, lenOutMinMax.x);
		lenMinMax.y = max(lenMinMax.y, lenOutMinMax.y);
	}

	lenMinMax.y = min(lenMinMax.y, dotED - lenEarthH + 1e9*float(1<dstEarth));
	return lenMinMax;
}

float AtmosDens(vec3 modelStepPos, vec3 modelStepDir)
{
	float startProgress = (wanderProgress - PROGRESS_2)*50;
	float endProgress = 1-(PROGRESS_3 - wanderProgress)*100;
	float torqueSign = 1;
#ifdef TORQUE_TIME_0
	startProgress = (wanderProgress - PROGRESS_1)*50;
	endProgress = 1-(PROGRESS_2 - wanderProgress)*100;
	torqueSign = -1;
#endif // TORQUE_TIME_0

	float ratioAlt = length(modelStepPos) / EARTH_RADIUS;
	float ratioZ = modelStepPos.z / EARTH_RADIUS;
	float thetaYZ = atan(modelStepPos.y / modelStepPos.z)/M_PI;

	vec3 erosionUVW = vec3(modelStepPos.x, thetaYZ, length(modelStepPos.yz)) / vec3(0.81*EARTH_RADIUS, 0.5, 1.403*EARTH_RADIUS);
	erosionUVW.y += times*0.021*torqueSign;
	vec3 offsetUVW = texture3D(noiseErosionTex, fract(erosionUVW)).bgr;

	vec3 shapeUVW = vec3(modelStepPos.x, thetaYZ, length(modelStepPos.yz)) / vec3(EARTH_RADIUS, 1.0, 0.5*EARTH_RADIUS);
	shapeUVW.y += times*0.017*torqueSign;
	shapeUVW += offsetUVW*0.5;
	shapeUVW.z += thetaYZ*torqueSign*2*exp2(-abs(ratioZ));

	vec4 shape4 = texture3D(noiseShapeTex, fract(shapeUVW));
	float texDens = max(0, shape4.x - 0.35*shape4.y - 0.11*shape4.z - 0.09*shape4.w);
	texDens *= max((thetaYZ*torqueSign + 0.16) * 1.6, exp2(-ratioZ*ratioZ*20));
	// fade by altitude
	texDens *= exp2(min(0, 1-ratioAlt)*5);
	// start progress
	texDens *= clamp((startProgress-abs(ratioZ))*3, 0, 1);
	// end progress
	texDens *= clamp((abs(ratioZ)-endProgress)*3, 0, 1);

	return texDens;
}

#else // !BRAKE_TIME && !TORQUE_TIME_X

// (length min,length max)
// dstEarth means the normalized distance to Earth core, [0,1]
vec2 LenMinMax(vec3 modelEyePos, vec3 modelPixDir, out float dstEarth)
{
	// cull by earth
	float dotED = dot(-modelEyePos, modelPixDir);
	vec3 modelCorePointM = modelEyePos + dotED*modelPixDir;
	dstEarth = length(modelCorePointM)/EARTH_RADIUS;
	float lenEarthH = EARTH_RADIUS*sqrt(max(0,1-dstEarth*dstEarth));
	vec2 lenMinMax = vec2(1e9, 0);
	float dstOut = dstEarth;

	// tail sphere
	for(int i = 0 ; i < 6 ; i ++)
	{
		float f = float(i);
		float radiusScale = 0.45 + 0.55*exp2(-f*0.8);
		float radiusI = ATMOS_RADIUS * radiusScale;
		vec3 spherePosI = vec3(0.0, 0.0, EARTH_RADIUS*(0.17 - 0.1*float(0.5<f) + 0.9*f*radiusScale));

		float dstTail = 0;
		vec2 lenTailMinMax = SphereLenMinMax(radiusI, spherePosI, modelEyePos, modelPixDir, dstTail);

		lenMinMax.x = min(lenMinMax.x, lenTailMinMax.x);
		lenMinMax.y = max(lenMinMax.y, lenTailMinMax.y);
		dstOut = min(dstOut, dstTail);
	}

	// tail cylinder
	float lenEyePos = length(modelEyePos);
	vec3 verticalVec = normalize(vec3(-modelPixDir.y, modelPixDir.x, 0.0));
	float lenCyd2View = abs(dot(verticalVec, modelEyePos));
	vec3 eyePosDirXY = normalize(vec3(modelEyePos.xy, 0.0));
	vec3 pixDirXY = normalize(vec3(modelPixDir.xy, 0.0));	
	float dstCyd = max(lenCyd2View/TAIL_RADIUS, step(0, dot(eyePosDirXY, modelPixDir)));
	float sinTailView = sqrt(1 - modelPixDir.z*modelPixDir.z);
	float tailLongAxisRatio = 1.0/max(1e-9, sinTailView);
	float lenTailCydH = TAIL_RADIUS*tailLongAxisRatio*sqrt(1-dstCyd*dstCyd);
	float lenTouchPoint = dot(-modelEyePos.xy, pixDirXY.xy)*tailLongAxisRatio;
	vec2 lenCydMinMax = lenTouchPoint + vec2(-lenTailCydH, lenTailCydH);
	// some bool
	float isInTail = (1 - step(TAIL_RADIUS, length(modelEyePos.xy)));
	float isEyeTailSameSide = step(0, modelEyePos.z);
	// tail cylinder cut, cut the sphere when it's z<0
	vec3 cutPointPos = vec3(modelEyePos.xy + modelPixDir.xy*abs(modelEyePos.z)/max(1e-9, abs(modelPixDir.z)),0);
	float cutMask = step(0, dot(normalize(cutPointPos), eyePosDirXY*sign(modelEyePos.z)));
	float cutMaskFrontSide = max(max(cutMask, step(0, -modelPixDir.z)), step(-0.9, -abs(dot(eyePosDirXY, pixDirXY))));
	float cutMaskTailSide = cutMask*step(0, -modelPixDir.z);
	cutMask = mix(cutMaskFrontSide, cutMaskTailSide, isEyeTailSameSide);
	float cydMask = max(dstCyd, cutMask*step(TAIL_RADIUS, length(cutPointPos)));
	lenCydMinMax.x = max(lenCydMinMax.x, lenEyePos*(1-isEyeTailSameSide));
	lenCydMinMax = mix(lenCydMinMax, vec2(1e9, 0), step(1, cydMask));

	// tail cylinder | sphere
	lenMinMax.x = min(lenMinMax.x, lenCydMinMax.x);
	lenMinMax.y = max(lenMinMax.y, lenCydMinMax.y);
	dstOut = min(dstOut, cydMask);

	lenMinMax = max(vec2(0), lenMinMax);
	lenMinMax = mix(lenMinMax, vec2(1e9, 0), float(1 < dstOut));
	lenMinMax = mix(lenMinMax, vec2(max(0, modelEyePos.z-TAIL_LENGTH), lenEyePos), isInTail*isEyeTailSameSide);
	lenMinMax.y = min(lenMinMax.y, dotED - lenEarthH + 1e9*step(1, dstEarth));

	return lenMinMax;
}

float AtmosDens(vec3 modelStepPos, vec3 modelStepDir)
{
	float ratioZ = modelStepPos.z / ATMOS_RADIUS;
	float ratioXY = length(modelStepPos.xy) / ATMOS_RADIUS;
	float ratioR = length(modelStepPos) / ATMOS_RADIUS;

	vec3 erosionUVW = modelStepPos/ATMOS_RADIUS;
	erosionUVW *= vec3(1, 1, 0.413);
	erosionUVW.z -= times*0.004;

	vec3 offsetUVW = texture3D(noiseErosionTex, fract(erosionUVW)).bgr;
	vec3 shapeUVW = modelStepPos/ATMOS_RADIUS;
	float shapeScale = 0.7 - 0.1*smoothstep(-0.1, 0.1, modelStepDir.z) + 0.3*smoothstep(0.2, 1.0, modelStepDir.z);
	float wrap = clamp((ratioXY-0.2)*5,0,1);
	shapeUVW.xy *= (1.5+2*wrap)*shapeScale;
	shapeUVW.z *= 0.2+0.1*wrap;		
	shapeUVW.z -= times*0.005;
	shapeUVW += offsetUVW*(0.2 + 0.5*wrap);

	vec4 shape4 = texture3D(noiseShapeTex, fract(shapeUVW));
	float texDens = max(0, shape4.x - 0.35*shape4.y - 0.11*shape4.z - 0.09*shape4.w);

	// fade by ratioR
	float sphereAltFade = clamp(engineStartRatio.x-0.3,0,1)*clamp(1-(ratioR-EARTH_ATMOS_RATIO)*6, 0, 1);	
	float torqueTailFade = sqrt(clamp((clamp((engineStartRatio.x-0.2)*0.5,0,1)*(2.3-EARTH_ATMOS_RATIO)+EARTH_ATMOS_RATIO-ratioR)*2,0,1));
	float propulsionTailFade = sqrt(clamp((0.95*TAIL_LENGTH/ATMOS_RADIUS - ratioR)*0.5, 0, 1))*clamp(engineStartRatio.y+1-ratioR,0,1);
	float altFade = mix(torqueTailFade, sphereAltFade, clamp(2*(ratioXY-0.5), 0, 1));
	altFade = mix(propulsionTailFade, altFade, clamp((ratioXY-TAIL_ATMOS_RATIO)*10, 0, 1));

	texDens *= altFade;
	// fade near ground
	texDens *= clamp((ratioR - EARTH_ATMOS_RATIO)*100, 0, 1);
	// cut the south half sphere
	texDens *= clamp((modelStepDir.z + 1.1 - ratioXY)*5,0,1);

	// fade by radiusXY
	// the propulsion engine fade
	float fadeR = clamp((TAIL_ATMOS_RATIO - ratioXY)*20, 0, 1);
	// the torque engine fade
	float torqueRScale = 2.5*exp2(-ratioZ*0.5)*TAIL_ATMOS_RATIO;
	fadeR += clamp(clamp(4-ratioZ, 0, 1)*300*(ratioXY - torqueRScale)*(1.3*torqueRScale - ratioXY), 0, 1);
	texDens *= mix(1, fadeR, clamp((ratioR-EARTH_ATMOS_RATIO)*3, 0, 1));
	return texDens;
}

#endif // BRAKE_TIME || TORQUE_TIME_X || not

float Dens2Bright(float dens)
{
	const float A = 0.01;
	return (1-exp(-dens*A*2))*exp(-dens*A);
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

void Tail(commonParam cP, inout vec4 tailColor, inout float lenTail)
{
	float lenUnitMin = cP.lenUnit * cP.norm.w;
	float lenD = min(LENGTH_MAX, cP.lenMax - cP.lenMin);
	vec4 tailC = vec4(0);
	float denSum = 0.0;
	float hasTail = 0.0;
	float lenFirstRange = STEP_NUM * cP.lenUnit;
	float isInRange = step(0.0,lenFirstRange*(exp2(RANGE_MAX)*(1+LAST_SCLAE)-1)-cP.lenMin);
	float jStart = min(RANGE_MAX-isInRange, floor(log2(cP.lenMin/lenFirstRange+1)));
	float jEnd = min(RANGE_MAX, ceil(log2(cP.lenMax/lenFirstRange+1)));
	for(int j = int(jStart); all(bvec2(tailC.a < ALPHA_MAX, j < jEnd)); j++)
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
		float lenStartStep = lenMinJ + cP.noiseD*lenUnitJ*0.01;
		float lenStep = startStepNum*lenUnitJ; // from (i*lenUnitJ) to (lenRange)
		float lenS = lenStartStep + lenStep; // distance from eye to this step
		for(int i = int(startStepNum);all(bvec3(tailC.a<ALPHA_MAX, lenStep<(lenRange*0.999), lenS<(cP.lenMin+lenD))); i++)
		{
			lenS = lenStartStep + lenStep*(1-0.01*cP.noiseD);
			vec3 modelStepPos = cP.modelEyePos + cP.modelPixDir*lenS;
			vec3 modelStepDir = normalize(modelStepPos);

			float atmosDens = AtmosDens(modelStepPos, modelStepDir);
			atmosDens *= numJ/exp2(RANGE_MAX-1);
			// fade for different range
			atmosDens *= mix(1, clamp((1-lenStep/lenRange)*2,0,1), mod(i+parity,2));
			// fade around the camera
			atmosDens *= 1-exp2(-lenS/STEP_LENGTH);

			if(atmosDens > 0.01)
			{
				// dens sum for cloud color
				denSum += atmosDens*cP.lenUnit;
				// ambient color
				float ratioR = length(modelStepPos) / EARTH_RADIUS;
				float ambientFade = (1-exp2(-atmosDens*80))*exp2(-atmosDens*40)*exp2(min(0,1-ratioR)*5);
				vec3 ambient = vec3(0.2, 0.3, 0.5)*ambientFade;
				// diffuse color
				float densLight = atmosDens;
				for(int k = 0; k < LIGHT_SAMPLE; k++)
				{
					float lenK = (1+0.1*cP.noiseD)*LIGHT_LEN[k];
					vec3 modelStepPosK = modelStepPos - cP.modelSunDir*lenK;
					vec3 modelStepDirK = normalize(modelStepPosK);
					float atmosDensK = AtmosDens(modelStepPosK, modelStepDirK);
					densLight += atmosDensK;
				}
				float beersLawOut = Dens2Bright(denSum*5e7)*Dens2Bright(densLight);
				vec3 diffuse = vec3(70,90,120)*cP.scattering*beersLawOut;
				// earth shadow
				float dotVUL = dot(modelStepDir, cP.modelSunDir);
				diffuse *= smoothstep(vec3(-0.55,-0.52,-0.5), vec3(0.2,0.24,0.28), vec3(dotVUL));
				// all color
				vec3 chrome = ambient + diffuse; 

				float tailA = 1 - exp2(-atmosDens * (0.25+0.2*exp2(min(0,1-ratioR))));
				tailC += vec4(chrome*tailA, tailA)*(1-tailC.a);			
			}

			lenStep += lenUnitJ;
			hasTail = min(hasTail + step(0.6,tailC.a), 1);
			lenTail = mix(lenS, lenTail, hasTail);
		}
	}
	tailC.rgb /= 0.001+tailC.a;
	float tailAlpha = tailVisible*tailC.a/ALPHA_MAX;
	tailColor = mix(tailColor, vec4(ToneMapping(tailC.rgb), tailAlpha), tailAlpha);
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

#if (defined(TORQUE_TIME_0) || defined(TORQUE_TIME_1))
	// eye position in model space
	vec3 modelEyePos = (world2SpiralMatrix*vec4(WCP,1)).xyz;
	// pixel direction in model space
	vec3 modelPixDir = (world2SpiralMatrix*vec4(WVD,0)).xyz;
	// sun direction in model space
	vec3 modelSunDir = (view2SpiralMatrix*vec4(viewLight,0)).xyz;
#else // !TORQUE_TIME_X
	// eye position in model space
	vec3 modelEyePos = (world2ECEFMatrix*vec4(WCP,1)).xyz;
	// pixel direction in model space
	vec3 modelPixDir = (world2ECEFMatrix*vec4(WVD,0)).xyz;
	// sun direction in model space
	vec3 modelSunDir = (view2ECEFMatrix*vec4(viewLight,0)).xyz;
#endif // TORQUE_TIME_X or not

	vec2 rayMarchScreenSize = screenSize.xy*screenSize.z;
	vec2 projCoord = gl_FragCoord.xy/rayMarchScreenSize;
	vec3 viewDir = (invProjMatrix * vec4(projCoord*2-1, 1, 1)).xyz; 
	viewDir = normalize(viewDir);
	// pixel direction in model space
	float dotVL = dot(modelPixDir, modelSunDir);
	const float gForward = 0.98;
	float forwardScattering = (1-gForward*gForward)/(4*M_PI*pow(1+gForward*gForward-2*gForward*dotVL,1.5));
	float scattering = 0.5 + forwardScattering;

	float noiseD = texture(blueNoiseTex, gl_FragCoord.xy*screenSize.z/(abs(fract(times*10)-0.5)*10+64)).r;
	// bounding shape
	float dstEarth = 0;
	vec2 lenMinMax = LenMinMax(modelEyePos, modelPixDir, dstEarth);
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
	vec3 lenStart = mod(dotNorm, STEP_LENGTH)/nd;
	vec3 lenUnit = STEP_LENGTH/nd;

	commonParam cP;
	cP.norm = norm_0;
	cP.modelEyePos = modelEyePos;
	cP.modelPixDir = modelPixDir;
	cP.modelSunDir = modelSunDir;
	cP.scattering = scattering;
	cP.dotNorm = dotNorm.x;
	cP.lenUnit = lenUnit.x;
	cP.lenMin = max(lenMin,lenStart.x);
	cP.lenMax = lenMax;
	cP.noiseD = noiseD;

	float earthBright = 0*exp(min(0, 1-dstEarth)*200);
	vec4 backColor = vec4(vec3(0.8,0.9,1.0)*earthBright,0);
	float tailAlpha = 1.0;
	float lenTail = cP.lenMin;// tail surface length

	float lenTail_0 = cP.lenMin;
	vec4 tailC_0 = backColor;
	Tail(cP, tailC_0, lenTail_0);

	lenTail = lenTail_0;
	vec4 tailColor = tailC_0;

#ifdef RAYS_2
	cP.norm = norm_1;
	cP.dotNorm = dotNorm.y;
	cP.lenUnit = lenUnit.y;
	cP.lenMin = max(lenMin,lenStart.y);

	float lenTail_1 = cP.lenMin;
	vec4 tailC_1 = backColor;
	Tail(cP, tailC_1, lenTail_1);
	
	lenTail = mix(lenTail, lenTail_1, weight.y);
	tailColor = mix(tailColor, tailC_1, weight.y);
#endif // RAYS_2

#ifdef RAYS_3
	cP.norm = norm_2;
	cP.dotNorm = dotNorm.z;
	cP.lenUnit = lenUnit.z;
	cP.lenMin = max(lenMin,lenStart.z);

	float lenTail_2 = cP.lenMin;
	vec4 tailC_2 = backColor;
	Tail(cP, tailC_2, lenTail_2);
		
	lenTail = mix(lenTail, lenTail_2, weight.z);
	tailColor = mix(tailColor, tailC_2, weight.z);
#endif // RAYS_3

	tailAlpha = (wanderProgress > PROGRESS_0) ? tailColor.a : 0.0;

	vec3 posDiff = ((deltaViewProjMatrix*vec4(WCP+lenTail*WVD,1.0)).xyz)/max(1, lenTail);
	// position different
	gl_FragData[0] = vec4(mix(backPosDiff, posDiff, tailAlpha), 1);

	// color and alpha
	gl_FragData[1] = vec4(tailColor.rgb, tailAlpha);

	//gl_FragData[1] = vec4(fract(max(0, lenMinMax.y-lenMinMax.x)*1e4),tailAlpha,0.1,1.0);
	//gl_FragData[1] = vec4(fract(lenMinMax.x*1e4),tailAlpha,0.1,1.0);
}