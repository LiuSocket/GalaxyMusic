#version 400 compatibility
#pragma import_defines(ULTRA)

const float alphaMax = 0.95;

uniform mat4 osg_ViewMatrixInverse;
uniform vec3 rangeMin;
uniform vec3 rangeMax;
uniform vec3 screenSize;
uniform sampler3D shapeNoiseTex;
uniform sampler2D blueNoiseTex;

in vec3 worldPos;

vec3 UVW(vec3 WVP)
{
	return clamp((WVP-rangeMin)/(rangeMax-rangeMin),0.0,1.0);
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

vec3 StartPos(vec3 WVD, vec3 WVP, float lenVert, out float lenD)
{
	vec3 deltaRay = mix(rangeMax-WVP, rangeMin-WVP, step(vec3(0),WVD));
	vec3 ratioRay = WVD/abs(deltaRay);
	lenD = 	min(min(length(WVD/ratioRay.x),length(WVD/ratioRay.y)),length(WVD/ratioRay.z));
	lenD = min(lenVert,lenD);
	vec3 startPos = WVP - lenD*WVD;
	return startPos;
}

void main() 
{
	float boxHalfSize = 0.5*(rangeMax.x-rangeMin.x);
	vec3 WCP = osg_ViewMatrixInverse[3].xyz; // world camera pos
	vec3 WVCP = worldPos-WCP;
	vec3 WVD = normalize(WVCP); // world vertex dir
	float lenV = length(WVCP);

	float lenD;
	vec3 startPos = StartPos(WVD, worldPos, lenV, lenD);
	float noiseD = texture2D(blueNoiseTex, gl_FragCoord.xy/128.0).r;

	float lenStep = noiseD*0.05*boxHalfSize;
	vec4 cloud = vec4(1,0,0,0);
	for(int i = 0; all(bvec3(cloud.a<alphaMax, lenStep<lenD, i<64)); i++)
	{
		vec3 stepPos = startPos+WVD*lenStep;
		float radiusRatio = length(stepPos)/boxHalfSize;

#ifdef ULTRA
		float shape = texture3D(shapeNoiseTex, UVW(stepPos)).r;
		float noise = 1-texture3D(shapeNoiseTex, 9.09090909*UVW(stepPos)).r;
		float cloudA = min(1, abs(0.8-shape)*sqrt(max(0,1-radiusRatio))*(0.1+0.1*noise));
		vec3 cloudC = vec3(1.0,0.2,0.0);
		cloudC += vec3(2, 30, 0)*exp2(-cloudA*40)*(1-exp2(-cloudA*20));
#else // not ULTRA
		float shape = texture3D(shapeNoiseTex, 0.11*UVW(stepPos)).r;
		float noise = 1-texture3D(shapeNoiseTex, UVW(stepPos)).r;
		float cloudA = min(1, abs(0.8-shape)*sqrt(max(0,1-radiusRatio))*noise*noise*noise);
		vec3 cloudC = vec3(0.0,0.1,0.5);
		cloudC += vec3(20, 3, 0)*exp2(-cloudA*400)*(1-exp2(-cloudA*200));
#endif // ULTRA

		cloud.rgb = mix(cloudC, cloud.rgb, cloud.a);
		cloud.a = 1 - (1-cloud.a) * (1-cloudA);

		lenStep += 0.05 * boxHalfSize * (0.99+0.01*noiseD);
	}

	float distanceRatio = length(WCP)/boxHalfSize;
	float fall = 7*exp2(-distanceRatio*2)*(1-exp2(-distanceRatio));
	gl_FragColor = vec4(ToneMapping(cloud.rgb), cloud.a*fall/alphaMax);
}