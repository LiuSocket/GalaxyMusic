#version 400 compatibility
#pragma import_defines(WANDERING)

#ifdef WANDERING
const float PROGRESS_0 = 0.1;
const float PROGRESS_1 = 0.15;
const float PROGRESS_2 = 0.2;
const float PROGRESS_3 = 0.25;
const float PROGRESS_4 = 0.31;// must be a little larger than 0.3, or you will see tail flash
uniform float wanderProgress;
#endif // WANDERING	

uniform vec2 planetRadius;
uniform vec4 coordScale_Earth;
uniform sampler2DArray cloudTex;
uniform sampler2D cloudDetailTex;

in vec2 texCoord_0;
in vec3 texCoord_1;
in vec4 viewPos;

void main() 
{
	float lenV = length(viewPos.xyz);
	vec3 cloudCoord = texCoord_1;
	cloudCoord.xy = (cloudCoord.xy - 0.5)*coordScale_Earth.y + 0.5;

	float cloudAlpha = texture(cloudTex, cloudCoord).a;
#ifdef WANDERING
	vec3 wanderingCloudCoord = cloudCoord;
	wanderingCloudCoord.z += 6;
	float wanderingCloudAlpha = texture(cloudTex, wanderingCloudCoord).a;

	float torqueArea = clamp((0.3-abs(texCoord_0.y*2-1))*4,0,1); torqueArea *= torqueArea;
	float torqueStart = float(wanderProgress>PROGRESS_0 && wanderProgress<PROGRESS_3)*torqueArea;
	float allStart = max(torqueStart, clamp((wanderProgress-(PROGRESS_4+0.05))*25, 0, 1));

	cloudAlpha = mix(cloudAlpha, wanderingCloudAlpha, allStart);
#endif // WANDERING	
	// cloud detail
	vec4 detail4 = texture(cloudDetailTex, texCoord_1.xy*27);
	vec4 detailMix = clamp((cloudAlpha-vec4(0.2,0.35,0.5,0.65))/0.15, vec4(0), vec4(1));
	float detail = mix(mix(mix(mix(detail4.x,
		detail4.y, detailMix.x),
		detail4.z, detailMix.y),
		detail4.w, detailMix.z),
		1, detailMix.w);
	vec2 edgeXY = clamp(20*(1+4*detail)*(0.5-abs(texCoord_1.xy-0.5)), cloudAlpha, 1.0);
	float edge = edgeXY.x*edgeXY.y;

	float detailFinal = detail*clamp(cloudAlpha/0.2, 0, 1);
	cloudAlpha = mix(cloudAlpha, detailFinal, edge*exp2(-lenV*0.5/planetRadius.x));
	float cloudShadow = 1-cloudAlpha;
	gl_FragColor = vec4(cloudShadow, 0, 0, 1);
}