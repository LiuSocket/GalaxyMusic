#version 400 compatibility
#pragma import_defines(WANDERING)

#ifdef WANDERING
const float PROGRESS_0 =	0.005;
const float PROGRESS_1 =	0.03; // end of brake time
const float PROGRESS_1_1 =	0.04;
const float PROGRESS_2 =	0.09; // middle of torque time
const float PROGRESS_2_1 =	0.1;
const float PROGRESS_3 =	0.15; // end of torque time
const float PROGRESS_3_1 =	0.152;
uniform vec3 engineStartRatio;
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
	// for start
	float lon = abs(fract(texCoord_0.x-0.25)*2-1);
	lon = (engineStartRatio.z > 0.5) ? lon : 1-lon;
	// x = torque, y = propulsion
	vec2 engineStart = vec2(smoothstep(0.0, 0.2, clamp(2*engineStartRatio.x-lon,0,1)*torqueArea)*exp2(-abs(texCoord_0.y-0.5)*25),
		smoothstep(0.0, 0.2, (texCoord_0.y-1)*2+engineStartRatio.y)*exp2(min(0,texCoord_0.y-0.67)*40));
	float allEngineStart = max(engineStart.x, engineStart.y);

	cloudAlpha = mix(cloudAlpha, wanderingCloudAlpha, allEngineStart*min(1, engineStartRatio.x));
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