#version 400 compatibility

const float alphaMax = 0.9;

uniform mat4 osg_ViewMatrixInverse;
uniform vec4 playingStarColor;
uniform vec3 starWorldPos;
uniform vec3 screenSize;
uniform float level[128];
uniform float times;
uniform sampler3D shapeNoiseTex;
uniform sampler2D blueNoiseSampler;

in vec3 worldPos;

vec3 ToneMapping(vec3 color)
{
	const float A = 2.51;
	const float B = 0.03;
	const float C = 2.43;
	const float D = 0.59;
	const float E = 0.14;
	return pow((color * (A * color + B)) / (color * (C * color + D) + E), vec3(1.0/2.2));
}

vec3 WorldStartPos(vec3 WVD, vec3 WVP, float lenV, out float lengthEnd2Start)
{
	lengthEnd2Start = min(lenV, 2*length(dot(starWorldPos - WVP, WVD)));
	vec3 startPos = WVP - lengthEnd2Start*WVD;
	return startPos;
}

void main() 
{
	vec3 WCP = osg_ViewMatrixInverse[3].xyz; // world camera pos
	vec3 WVCP = worldPos-WCP;
	vec3 WVD = normalize(WVCP); // world vertex dir
	float lenV = length(WVCP);

	float lenD;
	vec3 worldStartPos = WorldStartPos(WVD, worldPos, lenV, lenD);

	float radius = length(worldPos - starWorldPos);
	float noiseD = texture2D(blueNoiseSampler, gl_FragCoord.xy/128.0).r;

	float lenStep = noiseD*0.05*radius;
	vec4 cloud = vec4(0);
	for(int i = 0; all(bvec3(cloud.a<alphaMax, lenStep<lenD, i<32)); i++)
	{
		vec3 stepPos = worldStartPos+WVD*lenStep;
		vec3 stepPolarPos = stepPos-starWorldPos;
		vec3 stepNormal = normalize(stepPolarPos);
		float raidusRatio = max(1e-10,length(stepPolarPos) / radius);
		float shapeNoise = 1-texture3D(shapeNoiseTex, (0.9-0.3*raidusRatio)*stepPolarPos/radius+0.005*times).r;
		float disRipple = mod(128.0 * raidusRatio * (0.8+0.2*shapeNoise), 128.0);
		float audioLevel = level[int(disRipple)];

		vec3 cloudC = playingStarColor.rgb*playingStarColor.rgb*max(0, 2-raidusRatio);
		cloudC = mix(cloudC, playingStarColor.gbr, sqrt(clamp((raidusRatio-0.6+0.2*shapeNoise-0.1*audioLevel), 0, 1)));
		cloudC = mix(cloudC, playingStarColor.brg, clamp((raidusRatio-0.8+0.1*shapeNoise-0.1*audioLevel)*3, 0, 1));

		float cloudA = (0.1+0.1*audioLevel) * max(0, shapeNoise*shapeNoise - pow(raidusRatio,3.5) + 0.6*sin(5*raidusRatio-4*times)-0.5);

		cloudC *= (1 + 2*exp(-cloudA*100)*(1-exp(-cloudA*200)));
		cloud.rgb = mix(cloudC, cloud.rgb, cloud.a);
		cloud.a = 1 - (1-cloud.a) * (1-cloudA);

		lenStep += 0.05 * radius * (0.999+0.001*noiseD);
	}
	
	float fall = 1-exp2(-length(WCP - starWorldPos)*5.0/radius);
	gl_FragColor = vec4(ToneMapping(cloud.rgb),cloud.a*fall);
}