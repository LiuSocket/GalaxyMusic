#version 330 compatibility

uniform vec3 screenSize;
uniform vec2 shakeVec;
uniform sampler2D lastTex;
uniform sampler2D lastDistanceTex;
uniform sampler2D blueNoiseSampler;
uniform sampler2D velocityTex;
uniform sampler2D currentTex;
uniform sampler2D currentDistanceTex;

float NoiseD(vec2 UV)
{
	float noiseD = texture2D(blueNoiseSampler, UV*screenSize.xy/128).r;
	return mix(noiseD,1-noiseD, step(0.0,shakeVec.x+shakeVec.y));
}

void main()
{
	vec2 coordOffset = gl_TexCoord[0].xy - shakeVec/screenSize.xy;
	vec4 currentColor = texture2D(currentTex, coordOffset);
	vec3 posDiff = texture2D(velocityTex, coordOffset).xyz;
	float offsetFact = clamp(1-length(posDiff)*40,0,1);
	vec2 lastUV = gl_TexCoord[0].xy - posDiff.xy;
	vec4 lastColor = texture2D(lastTex, lastUV);
	float noiseD = NoiseD(gl_TexCoord[0].xy);

	if((lastColor.a+currentColor.a)<0.005) discard;

	vec2 isOut = step(-0.5,-abs(lastUV-0.5));
	float noiseMix = mix(0.05+noiseD*0.05, 0.89+noiseD*0.1, offsetFact)*isOut.x*isOut.y;
	gl_FragData[0] = mix(currentColor, lastColor, noiseMix);

	vec3 currentDistance = texture2D(currentDistanceTex, coordOffset).rgb;
	vec3 lastDistance = texture2D(lastDistanceTex, lastUV).rgb;
	float distanceNoiseMix = mix(0.1*noiseD, 0.8+0.1*noiseD, offsetFact)*isOut.x*isOut.y;
	gl_FragData[1] = vec4(mix(currentDistance, lastDistance, distanceNoiseMix),1);
}