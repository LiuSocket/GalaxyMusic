#version 400 compatibility
#extension GL_EXT_texture_array : enable

uniform vec2 planetRadius;
uniform vec4 coordScale_Earth;
uniform sampler2DArray cloudTex;
uniform sampler2D cloudDetailTex;

in vec3 texCoord_1;
in vec4 viewPos;

void main() 
{
	float lenV = length(viewPos.xyz);

	vec3 cloudCoord = texCoord_1;
	cloudCoord.xy = (cloudCoord.xy - 0.5)*coordScale_Earth.y + 0.5;

	float cloudAlpha = texture(cloudTex, cloudCoord).a;
	// cloud detail
	vec4 detail4 = texture(cloudDetailTex, texCoord_1.xy*27);
	vec4 detailMix = clamp((cloudAlpha-vec4(0.2,0.35,0.5,0.65))/0.15, vec4(0), vec4(1));
	float detail = mix(mix(mix(mix(detail4.x,
		detail4.y, detailMix.x),
		detail4.z, detailMix.y),
		detail4.w, detailMix.z),
		1, detailMix.w);
	detail *= clamp(cloudAlpha/0.2, 0, 1);
	cloudAlpha = mix(cloudAlpha, detail, exp2(-lenV/planetRadius.x));
	float cloudShadow = 1-cloudAlpha;
	gl_FragColor = vec4(cloudShadow, 0, 0, 1);
}