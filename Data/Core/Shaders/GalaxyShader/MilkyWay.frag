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
const float STEP_LENGTH = 0.005;
const float RANGE_MAX = 5;
const float M_PI = 3.141592657;
const float ALPHA_MAX = 0.99;
const float LENGTH_MAX = 7.0;

uniform float galaxyAlpha;
uniform float pixelLength;
uniform vec2 shakeVec;
uniform vec2 deltaShakeVec;
uniform vec3 screenSize;
uniform vec3 rangeMin;
uniform vec3 rangeMax;
uniform vec3 eyeFrontDir;
uniform vec3 eyeRightDir;
uniform vec3 eyeUpDir;
uniform vec4 noiseVec4;
uniform mat4 invProjMatrix;
uniform mat4 deltaViewProjMatrix;
uniform mat4 osg_ViewMatrixInverse;

uniform sampler2D lastVectorTex;
uniform sampler2D galaxyTex;
uniform sampler2D galaxyHeightTex;
uniform sampler2D noise2DTex;
uniform sampler2D blueNoiseTex;
uniform sampler3D noiseShapeTex;
uniform sampler3D noiseErosionTex;
uniform sampler3D noiseCurlTex;

in vec3 weight;
in vec3 localVertDir;

vec3 UVW(vec3 WVP)
{
	return clamp((WVP-rangeMin)/(rangeMax-rangeMin),0.0,1.0);
}

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

vec3 Curl(float texCurl, vec4 galaxyData, float coordZ)
{
	return 0.2*vec3(sin(texCurl*M_PI), cos(texCurl*M_PI), sin(coordZ*20*M_PI));
}

vec4 GalaxyData(vec3 galaxyPos, vec4 galaxyHeight, vec4 galaxy2DNoise)
{
	/* 
	* galaxyHeight.r = redHeight
	* galaxyHeight.g = dustHeight
	* galaxyHeight.b = nebulaHeight
	* galaxyHeight.a = nebulaNoise	
	*/
	float nebulaNoise = (galaxyHeight.a-0.5)*(0.05+0.2*length(galaxyPos.xy));
	return vec4(1)-step(galaxyHeight,(vec4(20,15,10,10)+vec4(30,10,10,10)*galaxy2DNoise)*abs(galaxyPos.z+nebulaNoise));
}

float GalaxyCoreDens(vec3 pos)
{
	const mat4 rotateMatrix = mat4(
		0.707106781, -0.707106781,	0,	0,
		0.707106781,  0.707106781,	0,	0,
		0,						0,	1,	0,
		0,						0,	0,	1);
	vec3 newPos = (rotateMatrix*vec4(pos,1)).xyz;
	vec3 newDir = normalize(newPos);
	float rangeZ = 0.35*(rangeMax.z-rangeMin.z);
	vec3 shapeCore = newPos/(rangeZ*vec3(3.0,1.7,1.0));
	float lenCore = length(shapeCore);
	return exp2(-lenCore*lenCore*8);
}

vec4 GalaxyShape(vec4 galaxyDens, vec4 texShape)
{
	const mat4 shapeMatrix = mat4(
		0,	0,	1,	0,
		0,  0,	1,	0,
		0,	0,	1,	0,
		0,	0,	1,	0);
	return clamp(galaxyDens-abs(vec4(0.5, 0.5, 0.0, 0.5)-shapeMatrix*texShape),0,1);
}

void Nebula(commonParam cP, inout vec4 nebulaColor, inout float lenNebula)
{
	float lenUnitMin = cP.lenUnit * cP.norm.w;
	float lenD = min(LENGTH_MAX, cP.lenMax - cP.lenMin);
	vec4 nebulaC = vec4(0);
	float hasNebula = 0;
	float lenFirstRange = STEP_NUM * cP.lenUnit;
	float isInRange = step(0.0,lenFirstRange*(exp2(RANGE_MAX)*(1+LAST_SCLAE)-1)-cP.lenMin);
	float jStart = min(RANGE_MAX-isInRange, floor(log2(cP.lenMin/lenFirstRange+1)));
	float jEnd = min(RANGE_MAX, ceil(log2(cP.lenMax/lenFirstRange+1)));
	for(int j = int(jStart); all(bvec2(nebulaC.a < ALPHA_MAX, j < jEnd)); j++)
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
		for(int i = int(startStepNum);all(bvec3(nebulaC.a<ALPHA_MAX, lenStep<(lenRange*0.999), lenS<(cP.lenMin+lenD))); i++)
		{
			lenS = lenStartStep + lenStep*(1-0.01*cP.noiseD);
			vec3 stepPos = cP.WCP + cP.WVD*lenS;
			vec3 coord3D = UVW(stepPos);

			vec4 galaxy2DNoise = texture2D(noise2DTex, coord3D.xy*15.1);
			vec3 galaxyColor = texture2D(galaxyTex, coord3D.xy).rgb;
			vec4 galaxyHeight = texture2D(galaxyHeightTex, coord3D.xy);
			vec3 galaxyPos = 2*coord3D-1;
			vec4 galaxyData = GalaxyData(galaxyPos,galaxyHeight,galaxy2DNoise);

			float redCover = galaxyData.r;
			float dustCover = galaxyData.g;
			float nebulaCover = galaxyData.b;

			float shapeDens = nebulaCover+dustCover+redCover;
			float coreDens = GalaxyCoreDens(stepPos);

			if((shapeDens+coreDens) > 0.0)
			{
				float nebulaDens = 0.0;
				float dustDens = 0.0;
				float redDens = 0.0;
				if(shapeDens > 0.0)
				{
					float theta = length(galaxyPos)*3;
					float cosTheta = cos(theta);
					float sinTheta = sin(theta);
					mat4 rotateMatrix = mat4(
						cosTheta,	-sinTheta,	0,	0,
						sinTheta,	cosTheta,	0,	0,
						0,					0,	1,	0,
						0,					0,	0,	1);
					vec3 newPos = (rotateMatrix*vec4(stepPos,1)).xyz;
					vec3 newCoord3D = UVW(newPos);
					float texCurl = texture3D(noiseCurlTex,newCoord3D*vec3(50, 50, 100)).r;
					vec3 curlV3 = Curl(texCurl,galaxyData,coord3D.z);
					vec4 texShape = texture3D(noiseShapeTex,newCoord3D*vec3(20, 20, 30)+curlV3);
					vec4 galaxyShape = GalaxyShape(galaxyData,texShape);
					dustDens = galaxyShape.r;
					nebulaDens = galaxyShape.g;
					redDens = galaxyShape.b;
				}
				float texDens = coreDens;//nebulaDens+dustDens+redDens+coreDens;

				// fade for different range
				texDens *= mix(1, clamp((1-lenStep/lenRange)*2,0,1), mod(i+parity,2));
				// fade around the camera
				texDens *= 1-exp2(-lenS*1e4);

				vec3 chromeCore = 500*exp2((coreDens-1)*vec3(1, 10, 13));
				vec3 chromeNebula = galaxyColor*vec3(0.05, 0.1, 0.2);
				vec3 chromeRed = vec3(1, 0.1, 0);
				vec3 chrome = (chromeCore + chromeNebula*nebulaDens + chromeRed*redDens);

				float stepAlpha = min(texDens,1);
				nebulaC += vec4(chrome*stepAlpha, stepAlpha)*(1-nebulaC.a);
			}
			lenStep += lenUnitJ;
			hasNebula = min(hasNebula + step(0.6,nebulaC.a), 1);
			lenNebula = mix(lenS, lenNebula, hasNebula);
		}
	}
	nebulaC.rgb /= 0.001+nebulaC.a;
	float nebulaAlpha = nebulaC.a/ALPHA_MAX;
	nebulaColor = mix(nebulaColor, vec4(ToneMapping(nebulaC.rgb), nebulaAlpha), nebulaAlpha);
}

void main() 
{
	vec2 cf = step(1.0, mod(gl_FragCoord.xy,2.0));
	float checkBox = step(0.5, mod(cf.x + cf.y, 2.0));

	vec3 localDir = normalize(localVertDir);
	vec3 localNear = localDir/dot(localDir,eyeFrontDir);
	vec2 shakeV2 = shakeVec + 0.25*(mix(noiseVec4.xy, noiseVec4.zw, checkBox) - vec2(0.5))*step(-0.9,-screenSize.z);
	vec3 localShake = (eyeRightDir*shakeV2.x + eyeUpDir*shakeV2.y)*pixelLength;
	// after shake
	localDir = normalize(localNear + localShake);

	vec3 WVD = localDir;
	vec3 WCP = osg_ViewMatrixInverse[3].xyz;

	vec2 rayMarchScreenSize = screenSize.xy*screenSize.z;
	vec2 projCoord = gl_FragCoord.xy/rayMarchScreenSize;
	vec3 viewDir = (invProjMatrix * vec4(projCoord*2-1, 1, 1)).xyz; 
	viewDir = normalize(viewDir);

	float noiseD = texture2D(blueNoiseTex, gl_FragCoord.xy*screenSize.z/128.0).r;

	// bounding shape
	vec2 lenMinMax_1 = LenMinMax(vec3(0, 0, 2.53), 3.0, WCP, WVD);
	vec2 lenMinMax_2 = LenMinMax(vec3(0, 0, -2.53), 3.0, WCP, WVD);
	vec2 lenMinMax_3 = LenMinMax(vec3(0, 0, 44.76), 45.1, WCP, WVD);
	vec2 lenMinMax_4 = LenMinMax(vec3(0, 0, -44.76), 45.1, WCP, WVD);
	float lenMin = min(max(lenMinMax_1.x, lenMinMax_2.x),max(lenMinMax_3.x, lenMinMax_4.x));
	float lenMax = max(min(lenMinMax_1.y, lenMinMax_2.y),min(lenMinMax_3.y, lenMinMax_4.y));

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
	cP.WCP = WCP;
	cP.WVD = WVD;
	cP.dotNorm = dotNorm.x;
	cP.lenUnit = lenUnit.x;
	cP.lenMin = max(lenMin,lenStart.x);
	cP.lenMax = lenMax;
	cP.noiseD = noiseD;

	vec4 nebulaColor = vec4(0);
	float nebulaAlpha = 1.0;
	float lenNebula = cP.lenMin;// nebula surface length

	float lenNebula_0 = cP.lenMin;
	vec4 nebulaC_0 = vec4(0);
	Nebula(cP, nebulaC_0, lenNebula_0);

	lenNebula = lenNebula_0;
	nebulaColor = nebulaC_0;

#ifdef RAYS_2
	cP.norm = norm_1;
	cP.dotNorm = dotNorm.y;
	cP.lenUnit = lenUnit.y;
	cP.lenMin = max(lenMin,lenStart.y);

	float lenNebula_1 = cP.lenMin;
	vec4 nebulaC_1 = vec4(0);
	Nebula(cP, nebulaC_1, lenNebula_1);
	
	lenNebula = mix(lenNebula, lenNebula_1, weight.y);
	nebulaColor = mix(nebulaColor, nebulaC_1, weight.y);
#endif // RAYS_2

#ifdef RAYS_3
	cP.norm = norm_2;
	cP.dotNorm = dotNorm.z;
	cP.lenUnit = lenUnit.z;
	cP.lenMin = max(lenMin,lenStart.z);

	float lenNebula_2 = cP.lenMin;
	vec4 nebulaC_2 = vec4(0);
	Nebula(cP, nebulaC_2, lenNebula_2);
		
	lenNebula = mix(lenNebula, lenNebula_2, weight.z);
	nebulaColor = mix(nebulaColor, nebulaC_2, weight.z);
#endif // RAYS_3

	nebulaAlpha = nebulaColor.a*galaxyAlpha;

	vec3 posDiff = ((deltaViewProjMatrix*vec4(WCP+lenNebula*WVD,1.0)).xyz)/max(1, lenNebula);
	// position different
	gl_FragData[0] = vec4(mix(backPosDiff, posDiff, nebulaAlpha), 1);

	// color and alpha
	gl_FragData[1] = vec4(nebulaColor.rgb, nebulaAlpha);	

	// RG = min length, B = into length, A = nebula Alpha
	float lenWCP = length(WCP);
	float lenSurfaceR = (lenNebula - lenWCP)*0.1+0.5;// 0.0~1.0
	float lenSurfaceG = fract(lenSurfaceR*255);
	float lenSurfaceB = fract(lenSurfaceG*255);
	gl_FragData[2] = vec4(lenSurfaceR, lenSurfaceG, lenSurfaceB, nebulaAlpha);
}