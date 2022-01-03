#version 400 compatibility

const float M_PI = 3.141592657;

uniform vec2 starAudioPos;
uniform float times;

out vec4 vertexColor;

void main()
{
	float starAudioRadius = starAudioPos.x;
	float starAudioAngleRatio = starAudioPos.y/(2*M_PI);
	vec2 mixFilter = step(vec2(1e-10), gl_MultiTexCoord0.xy);

	float angleDistance = mixFilter.y*min(abs(starAudioAngleRatio-gl_MultiTexCoord0.y),
		min(abs(starAudioAngleRatio-gl_MultiTexCoord0.y-1),abs(starAudioAngleRatio-gl_MultiTexCoord0.y+1)));
	float angleFall = clamp(1.0+1.5*(1-starAudioRadius)-angleDistance*10,0,1);
	float vertexRadius = mix(starAudioRadius, gl_MultiTexCoord0.x, mixFilter.x)
		+ gl_MultiTexCoord0.z*0.03*angleFall;
	float audioAngleRatio = mix(starAudioAngleRatio, gl_MultiTexCoord0.y, mixFilter.y);
	vertexColor = vec4(
		abs(4.0 * audioAngleRatio - 2.5) - 0.5,
		1.5 - abs(4.0 * audioAngleRatio - 1.5),
		1.0 - abs(4.0*audioAngleRatio - 3.0),
		1);
	vertexColor = clamp(vertexColor, 0.0, 1.0);
	float pace = mix(0.1,2.5,starAudioRadius*starAudioRadius);
 	vertexColor.rgb *= mix(0.6+1.4*abs(fract(times*0.5/pace)-0.5)*2, 1.0, mixFilter.x);

	float radiusPuls = max(0,1-abs(abs(starAudioRadius-gl_MultiTexCoord0.x)-mix(-0.2,0.2,fract((times-0.1)*0.5/pace)))*50);
	vertexColor.rgb += radiusPuls*mixFilter.x*(1-mixFilter.y);
	float radiusDistance = abs(starAudioRadius - gl_MultiTexCoord0.x)*mixFilter.x;
	vertexColor.a = exp2(min(0.0, 0.2*(1-starAudioRadius)-radiusDistance)*20) * (1-gl_MultiTexCoord0.z) *
		clamp((1.0-gl_MultiTexCoord0.x) * 50.0, 0.0, 1.0) * step(0.2, angleFall);

	float vertexAudioAngle = audioAngleRatio*2*M_PI;
	float audioX = vertexRadius * cos(vertexAudioAngle);
	float audioY = vertexRadius * sin(vertexAudioAngle);
	float offset = 0.22*sign(audioY-audioX);

	const float galaxyRadius = 5.0;
	vec4 position = vec4(galaxyRadius*vec2(audioX, audioY*(1.0 - abs(offset)) + offset), 0, 1);
	float vertexAngle = - M_PI * 0.25 + vertexRadius * M_PI * 2.0;
	float cosA = cos(vertexAngle);
	float sinA = sin(vertexAngle);
	mat4 rotateMatrix = mat4(
		cosA,	sinA,	0,	0,
		-sinA,	cosA,	0,	0,
		0,		0,		1,	0,
		0,		0,		0,	1);
	vec4 modelPos = rotateMatrix * position;

	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * modelPos;
}