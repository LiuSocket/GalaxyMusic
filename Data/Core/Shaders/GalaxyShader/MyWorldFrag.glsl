#version 400 compatibility

uniform sampler2D noise2DTex;
uniform float myWorldAlpha;

in vec4 vertexColor;

void main() 
{
	vec2 fall = 2*abs(gl_PointCoord.st-0.5);
	float radius = clamp(length(fall),0,1);
	vec3 noiseRGB = texture2D(noise2DTex, 0.5*(gl_PointCoord.st + vertexColor.rg*vertexColor.b)).rgb;
	float noise = mix(mix(noiseRGB.r, noiseRGB.g, min(1,2*vertexColor.a)),noiseRGB.b, max(0, 2*vertexColor.a-1));
	float noiseAlpha = mix(1-sqrt(radius), max(0, noise-radius), radius);
	gl_FragColor = vec4(vertexColor.rgb, noiseAlpha*myWorldAlpha);
}