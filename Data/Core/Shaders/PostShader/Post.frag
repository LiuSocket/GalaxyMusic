#version 430 compatibility

#pragma import_defines(VOLUME)

uniform vec3 screenSize;
uniform sampler2D sceneTex;
uniform sampler2D backgroundTex;
uniform sampler2D foregroundTex;

#ifdef VOLUME
uniform sampler2D volumeTex;
#endif //VOLUME

void main()
{
	vec2 sideUV = gl_TexCoord[0].xy + vec2(-0.5,-0.5)/screenSize.xy;

	vec4 backgroundColor = texture2D(backgroundTex, gl_TexCoord[0].xy);
	vec4 backSide = texture2D(backgroundTex, sideUV);
	backSide += textureOffset(backgroundTex, sideUV, ivec2(1,0));
	backSide += textureOffset(backgroundTex, sideUV, ivec2(1,1));
	backSide += textureOffset(backgroundTex, sideUV, ivec2(0,1));
	backgroundColor = mix(backgroundColor, backSide*0.25, 0.5);

	vec4 sceneColor = texture2D(sceneTex, gl_TexCoord[0].xy);
	vec4 sceneSide = texture2D(sceneTex, sideUV);
	sceneSide += textureOffset(sceneTex, sideUV, ivec2(1,0));
	sceneSide += textureOffset(sceneTex, sideUV, ivec2(1,1));
	sceneSide += textureOffset(sceneTex, sideUV, ivec2(0,1));
	sceneSide *= 0.25;
	float blurFact = distance(gl_TexCoord[0].xy, vec2(0.5,0.5))*2;
	blurFact = min(1, blurFact*blurFact);
	sceneColor = mix(max(sceneColor,sceneSide), sceneSide, blurFact);

	vec4 color = backgroundColor;

#ifdef VOLUME
	vec4 volumeColor = texture2D(volumeTex, gl_TexCoord[0].xy);
	color.rgb = mix(color.rgb, volumeColor.rgb, volumeColor.a);
	color.a = 1 - (1-color.a)*(1-volumeColor.a);
#endif //VOLUME

	color.rgb = mix(color.rgb, sceneColor.rgb, sceneColor.a);
	color.a = 1 - (1-color.a)*(1-sceneColor.a);

	vec4 foregroundColor = texture2D(foregroundTex, gl_TexCoord[0].xy);
	color.rgb = mix(color.rgb, foregroundColor.rgb, foregroundColor.a);
	color.a = 1 - (1-color.a)*(1-foregroundColor.a);
	
	gl_FragColor = color;
}