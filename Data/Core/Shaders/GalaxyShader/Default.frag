#version 400 compatibility

uniform vec3 screenSize;
uniform sampler2D colorTex;
uniform sampler2D alphaTex;

vec2 CheckBoxMask(vec2 fragCoord)
{
	vec4 cf = step(vec4(1.0, 1.0, 2.0, 2.0), mod(fragCoord.xyxy,vec4(2.0, 2.0, 4.0, 4.0)));
	return step(vec2(0.5), mod(vec2(cf.x + cf.y, cf.z + cf.w), vec2(2.0)));
}

/* noise float between 0.0~1.0 */
float NoiseV1(vec2 co)
{
	return fract(sin(dot(co, vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
	vec2 pixUnit = 1.0/screenSize.xy;
	vec2 uv = gl_FragCoord.xy*pixUnit;
	vec2 checkBox = CheckBoxMask(gl_FragCoord.xy);
	float noise = NoiseV1(uv);
	vec2 noiseV2 = pixUnit*noise*(checkBox-0.5);
	gl_FragColor = texture(colorTex, uv + noiseV2);
}