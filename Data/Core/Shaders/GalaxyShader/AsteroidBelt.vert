#version 400 compatibility

uniform vec4 playingStarColor;
uniform float unit;
uniform float planetPointAlpha;

uniform sampler2D asteroidDataTex;

out vec4 vertexColor;

void main()
{
	const float PI = 3.14159265358979;
	const float texWidth = 8192;
	const float texHeight = 2;	
	float vertexID = float(gl_VertexID);
	int iPixelY = gl_VertexID / int(texWidth);
	float pixelY = float(iPixelY);
	float pixelX = vertexID - pixelY * texWidth;

	vec4 asteroidData = texture(asteroidDataTex, vec2(pixelX+0.5, pixelY+0.5)/vec2(texWidth,texHeight));
	vec2 asteroidPos = asteroidData.xy;
	vec2 asteroidVelocity = asteroidData.zw;
	vec2 vertXY = asteroidPos/unit;
	float noise = abs(sin(vertexID));
	vec4 modelPos = vec4(vertXY, 4*sin(10.3*gl_VertexID)*1e9/unit, 1);
	vec3 modelSunDir = normalize(modelPos.xyz);
	vec3 viewSunDir = normalize((gl_ModelViewMatrix * vec4(modelSunDir,0)).xyz);
	vec3 viewPos = (gl_ModelViewMatrix * modelPos).xyz;
	float lengthV = length(viewPos)*unit*1e-15;

	vertexColor.rgb = mix(vec3(0.2), playingStarColor.rgb, 0.3+0.7*exp2(-lengthV*300.0))
		+ vec3(clamp(1-length(modelPos.xyz)*1.9e-12*unit,0,1));
	vertexColor.rgb *= 0.1+0.3*noise;
	vertexColor.a = planetPointAlpha;

	gl_PointSize = (1+noise)*(1+3*exp2(-lengthV*1e4))*clamp(1.5-length(asteroidPos*1e-12),0,1);
	gl_Position = gl_ModelViewProjectionMatrix * modelPos;

	float isFast = clamp((length(asteroidVelocity)-2e4)*5e-5, 0, 1);
	vertexColor.rgb = mix(vertexColor.rgb, vec3(1), isFast);
}