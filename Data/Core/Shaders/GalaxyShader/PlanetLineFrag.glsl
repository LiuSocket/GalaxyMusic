#version 400 compatibility

uniform float times;

void main() 
{
	float fall = fract(gl_TexCoord[0].x - times*0.05/gl_TexCoord[0].y);
	gl_FragColor = vec4(vec3(0.0, 2.0, 1.0)*fall, 1.0)*fall;
}