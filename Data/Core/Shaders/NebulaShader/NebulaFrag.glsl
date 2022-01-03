#version 400 compatibility

uniform vec3 screenSize;

#pragma import_defines(RAYMARCHING)

uniform mat4 osg_ViewMatrixInverse;
uniform vec3 rangeMin;
uniform vec3 rangeMax;
uniform sampler2D galaxyTex;

in vec4 viewPos;
in vec3 worldPos;

vec3 UVW(vec3 WVP)
{
	return clamp((WVP-rangeMin)/(rangeMax-rangeMin),0.0,1.0);
}

#ifdef RAYMARCHING

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
const float M_PI = 3.141592657;
const float cMax = 4.0;
const float alphaMax = 0.95;

uniform float countNum;
uniform float noiseNum;
uniform vec2 deltaShakeVec;
uniform mat4 deltaViewProjMatrix;

uniform sampler2D lastRayMarchTex;
uniform sampler2D lastVectorTex;
uniform sampler2D galaxyHeightTex;
uniform sampler2D blueNoiseSampler;
uniform sampler2D noise2DTex;
uniform sampler3D noiseShapeTex;
uniform sampler3D noiseErosionTex;
uniform sampler3D noiseCurlTex;

vec3 ToneMapping(vec3 color)
{
	const float A = 2.51;
	const float B = 0.03;
	const float C = 2.43;
	const float D = 0.59;
	const float E = 0.14;
	return pow((color * (A * color + B)) / (color * (C * color + D) + E), vec3(1.0/2.2));
}

vec3 StartPos(vec3 WVD, vec3 WVP, float lenVert, out float lenD)
{
	vec3 deltaRay = mix(rangeMax-WVP, rangeMin-WVP, step(vec3(0),WVD));
	vec3 ratioRay = WVD/abs(deltaRay);
	lenD = 	min(min(length(WVD/ratioRay.x),length(WVD/ratioRay.y)),length(WVD/ratioRay.z));
	lenD = min(lenVert,lenD);
	vec3 startPos = WVP - lenD*WVD;
	return startPos;
}

vec3 Curl(float texCurl, vec3 galaxyInformation, float coordZ)
{
	return vec3(sin(texCurl*M_PI), cos(texCurl*M_PI), 0.5*sin(coordZ*4*M_PI));
}

vec3 GalaxyInformation(vec3 galaxyPos, vec4 galaxyHeight, vec3 galaxy2DNoise)
{
	/* 
	* galaxyHeight.r = redHeight
	* galaxyHeight.g = dustHeight
	* galaxyHeight.b = nebulaHeight
	* galaxyHeight.a = nebulaNoise	
	*/
	float nebulaNoise = (galaxyHeight.a-0.5)*(0.05+0.2*length(galaxyPos.xy));
	return vec3(1)-step(galaxyHeight.rgb,(vec3(20,15,7)+vec3(30,10,10)*galaxy2DNoise)*abs(galaxyPos.z+nebulaNoise));
}

float GalaxyCore(vec3 pos)
{
	const mat4 rotateMatrix = mat4(
		0.707106781, -0.707106781,	0,	0,
		0.707106781,  0.707106781,	0,	0,
		0,						0,	1,	0,
		0,						0,	0,	1);
	vec3 newPos = (rotateMatrix*vec4(pos,1)).xyz;
	vec3 newDir = normalize(newPos);
	float rangeZ = 0.5*(rangeMax.z-rangeMin.z);
	vec3 shapeCore1 = newPos/(rangeZ*vec3(3.0,1.7,1.0));
	vec3 shapeCore2 = vec3(newPos.xy/(0.3*(rangeMax.xy-rangeMin.xy)),0);
	float mixFact = pow(max(1-abs(newDir.z),1e-10),20);
	vec3 shapeCore	= mix(shapeCore1, shapeCore2, mixFact);
	float galaxyCore = max(0, 1-length(shapeCore));
	return galaxyCore;
}

vec3 GalaxyShape(vec3 galaxyDens, float texShape)
{
	return clamp(vec3(0.3,0.1,0.1)*(vec3(0.4,0.2,0.3)*galaxyDens-abs(vec3(0.7, 0.8, 0.9)-texShape)),0,1);
}

#else // not RAYMARCHING

uniform sampler2D TAATex;

vec3 PlanePos(vec3 WVD, vec3 WVP, vec3 WCP)
{
	float planeZ = (rangeMax.z+rangeMin.z)*0.5;
	vec3 rangeHalfMax = vec3(rangeMax.xy, planeZ);
	vec3 rangeHalfMin= vec3(rangeMin.xy, planeZ);
	vec3 deltaRay = mix(rangeHalfMax-WVP, rangeHalfMin-WVP, step(vec3(0),WVD));
	vec3 ratioRay = WVD/abs(deltaRay);
	float lenD = min(min(length(WVD/ratioRay.x),length(WVD/ratioRay.y)),length(WVD/ratioRay.z));
	lenD *= step(0.0, WVD.z*(WVP.z-planeZ))*step(0.0,WVD.z*(planeZ-WCP.z));
	return WVP - lenD*WVD;
}

#endif // RAYMARCHING or not

void main() 
{
	vec3 WCP = osg_ViewMatrixInverse[3].xyz;
	vec3 WVD = normalize(worldPos-WCP);
	vec2 projCoord = gl_FragCoord.xy/screenSize.xy;

#ifdef RAYMARCHING

	projCoord /= screenSize.z;
	float lastAlpha = 1.0;
	if (gl_FragCoord.z > 0.0)
	{
		vec2 coordOffset = projCoord - deltaShakeVec/screenSize.xy;
		vec3 lastVector = texture2D(lastVectorTex, coordOffset).xyz;
		vec2 lastUV = coordOffset - lastVector.xy;
		vec2 isIn = step(-0.5,-abs(lastUV-0.5));
		lastAlpha = mix(1.0, texture2D(lastRayMarchTex, lastUV).a, isIn.x*isIn.y);
	}

	float lenVert = length(viewPos.xyz);
	float switchNum = mod(countNum,2);
	if(all(bvec2(switchNum>0.5, 0.001>lastAlpha)))
	{
		gl_FragData[0] = vec4(0);
		gl_FragData[1] = vec4(0,0,0,1);
		gl_FragData[2] = vec4(lenVert,0,0,1);
		return;
	}

	float noiseD = texture2D(blueNoiseSampler, gl_FragCoord.xy*screenSize.z*(1+noiseNum)/128.0).r;
	noiseD = mix(1-noiseD,noiseD,switchNum);
	vec3 WLD = normalize(vec3(0,0,0)-WCP);// world light direction
	float cosVL = dot(WVD,WLD);
	const float g = 0.9;
	float pHG = (1-g*g)/(4*M_PI*pow(1+g*g-2*g*cosVL,1.5));

	float absCosEye = abs(WVD.z);
	float lenD;
	vec3 startPos = StartPos(WVD, worldPos, lenVert, lenD);
	float lenNear = distance(startPos,WCP);

	vec4 nebulaC = vec4(0,0,0,0);
	float stepScale = 1+step(-0.001,-lastAlpha)*(1+15*noiseD);
	float minStep = 1e-3*(1.0+noiseD*(0.05+0.05*noiseNum));
	float lastStep = minStep;
	float lenStep = noiseNum*5e-4;
	float vacuumCount = cMax;
	float stopNum = 0.0;
	float isFirst = 0.0;
	float isVirgin = 1.0;
	float densSum = 0.0;
	float nebulaCount = 0.0;
	int h = 0;

	float lenVoxel = lenVert;
	float lenDeep = lenVert;
	float lenSurface = lenVert;

	for(int i = 0; all(bvec3(nebulaC.a<alphaMax, lenStep<lenD, i<1024)); i++)
	{
		float lenS = lenNear+lenStep;
		vec3 stepPos = startPos+WVD*lenStep;
		vec3 coord3D = UVW(stepPos);
		vec3 galaxy2DNoise = texture2D(noise2DTex, coord3D.xy*15.1).rgb;
		vec3 galaxyColor = texture2D(galaxyTex, coord3D.xy).rgb;
		vec4 galaxyHeight = texture2D(galaxyHeightTex, coord3D.xy);
		vec3 galaxyPos = 2*coord3D-1;
		vec3 galaxyInformation = GalaxyInformation(galaxyPos,galaxyHeight,galaxy2DNoise);

		float redAlpha = galaxyInformation.r;
		float dustAlpha = galaxyInformation.g;
		float nebulaAlpha = galaxyInformation.b;

		float shapeAlpha = nebulaAlpha+dustAlpha+redAlpha;
		float coreAlpha = 0.02*GalaxyCore(stepPos);

		float texDens = 0.0;
		float nebulaDens = 0.0;
		float dustDens = 0.0;
		float redDens = 0.0;
		float coreDens = 0.0;

		if((shapeAlpha+coreAlpha)>0.0001)
		{
			if(shapeAlpha>0.0001)
			{
				float texCurl = texture3D(noiseCurlTex,coord3D*vec3(7, 7, 17)).r;
				vec3 curlV3 = Curl(texCurl,galaxyInformation,coord3D.z);
				float texShape = texture3D(noiseShapeTex,coord3D*vec3(27, 27, 17)+curlV3).r;
				vec3 galaxyShape = GalaxyShape(galaxyInformation,texShape);
				redDens = galaxyShape.r;
				dustDens = galaxyShape.g;
				nebulaDens = galaxyShape.b;
			}
			coreDens = coreAlpha;
			texDens = nebulaDens+dustDens+redDens+coreDens;

			nebulaCount += 1.0;
			isFirst = step(0.5,nebulaCount)*step(-1.5,-nebulaCount);
			stopNum = mix(float(i)*isFirst,stopNum,step(1.5,nebulaCount));
			vacuumCount = max(vacuumCount-1.0,0.0);
			lenStep -= lastStep*isFirst;

			densSum += texDens;
			lenSurface = mix(lenSurface,lenS,isVirgin);
			isVirgin = step(-1,-densSum);
		}
		else
		{
			nebulaCount = 0.0;
			vacuumCount = min(vacuumCount+1.0,cMax);
		}

		float lenMix = exp2(-nebulaC.a*10);
		float lenBack = mix(lenS, max(0.5*(lenNear+lenVert),1e-5), exp2(-texDens*600));
		lenVoxel = mix(lenVoxel, lenBack, lenMix);
		lenDeep = mix(lenDeep, lenS, lenMix);

		vec3 chromeCore = vec3(1.0, 0.9, 0.0);
		vec3 chromeNebula = galaxyColor*vec3(0.5, 0.7, 1.5);
		vec3 chromeRed = vec3(1.0, 0.1, 0.0);
		vec3 chrome = (chromeCore*coreDens + chromeNebula*nebulaDens + chromeRed*redDens)
			/ max(1e-10, coreDens+nebulaDens+redDens+dustDens);

		float densNow = texDens;
		float alpha1 = nebulaC.a;
		nebulaC.a = 1-(1-alpha1)*(1-densNow);
		nebulaC.rgb = mix(chrome, nebulaC.rgb, alpha1);

		float f = max(float(i)-stopNum,0.0);
		vec2 coreStep = step(vec2(-0.001,0.01),vec2(-nebulaAlpha,coreAlpha));
		lastStep = minStep*(1+5*step(-0.5,-nebulaCount))*(1+7*coreStep.x*coreStep.y)
			*(1+f*0.005)*stepScale*(1+(1+f*0.006)*step(cMax-0.5,vacuumCount));

		lenStep += lastStep;
	}

	vec3 allColor = nebulaC.rgb;
	float allAlpha = nebulaC.a/alphaMax;
	// color and alpha
	gl_FragData[0] = vec4(allColor,allAlpha);	

	vec3 posDiff = (deltaViewProjMatrix*vec4(WCP+lenVoxel*WVD,1.0)).xyz/lenVoxel;
	// position different
	gl_FragData[1] = vec4(posDiff,1);

	// surface distance, deep distance, deep alpha
	gl_FragData[2] = vec4(lenSurface, lenDeep, allAlpha, 1);

#else // not RAYMARCHING

	vec4 RTTColor = vec4(0);
	if (gl_FragCoord.z > 0.0)
	{
		RTTColor = texture2D(TAATex, projCoord);
	}
	vec3 planePos = PlanePos(WVD, worldPos, WCP);
	vec3 coord = UVW(planePos);
	vec2 tmp = vec2(1,0) + vec2(-1,1)*step(vec2(1e-5,-1e-5), vec2(coord.z-0.5));
	vec4 galaxy = texture2D(galaxyTex, coord.xy);
	galaxy *= tmp.x*tmp.y;

	float finalAlpha = 1-(1-RTTColor.a)*(1-galaxy.a);
	vec3 finalColor = mix(galaxy.rgb, RTTColor.rgb, RTTColor.a);
	//gl_FragColor = vec4(finalColor, finalAlpha);
	gl_FragColor = RTTColor;
#endif // RAYMARCHING or not
}