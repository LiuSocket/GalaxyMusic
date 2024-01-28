#version 330 compatibility

uniform vec3 screenSize;
uniform vec2 shakeVec;
uniform sampler2D lastTex;
uniform sampler2D currentTex;
uniform sampler2D velocityTex;

void main()
{
	vec2 coord = gl_TexCoord[0].xy - shakeVec/screenSize.xy;
	vec4 currentColor = texture2D(currentTex, coord);
	currentColor.a = smoothstep(0,1,currentColor.a);
	vec3 posDiff = texture2D(velocityTex, gl_TexCoord[0].xy).xyz;
	vec2 lastUV = gl_TexCoord[0].xy - posDiff.xy;
	vec4 lastColor = texture2D(lastTex, lastUV);

	vec2 isOut = step(-0.5,-abs(lastUV-0.5));
	float similar = clamp(1-length(posDiff)*5e4, 0, 1)*isOut.x*isOut.y;

	float rayPixLen = 1.0/screenSize.z; // 2.0 4.0
	vec2 coordModXY = mod(gl_FragCoord.xy, vec2(rayPixLen)) - rayPixLen*0.5;// -0.5,0.5 / -1.5,-0.5,0.5,1.5
	vec4 coordMod = vec4(coordModXY,coordModXY);
	vec4 maskV = step(0, coordMod + vec4(1, 1, -1, -1)*screenSize.z - vec4(shakeVec,shakeVec));
	float shakeMask = maskV.x*maskV.y*(1-maskV.z)*(1-maskV.w);
	gl_FragColor = mix(currentColor, lastColor, (0.8-shakeMask*0.5)*similar);
}