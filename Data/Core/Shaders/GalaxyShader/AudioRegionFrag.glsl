#version 400 compatibility

uniform vec3 mouseWorldPos;
uniform vec2 audioUV;

in vec3 vertWorldPos;
in vec4 vertexColor;

void main() 
{
	float distanceMouse = distance(vertWorldPos, mouseWorldPos);
	vec4 color = vertexColor;
	color.a *= clamp(distanceMouse*2, 0, 1);

	vec2 UV = vec2(gl_TexCoord[0].x + gl_TexCoord[0].z, gl_TexCoord[0].y);
	vec2 nearAudio = exp2(-abs(audioUV-UV)*vec2(200,4));
	vec2 endFade = clamp(vec2(gl_TexCoord[0].y, 1 - gl_TexCoord[0].y)*vec2(100,50),0,1);
	color.a = max(color.a, nearAudio.x*nearAudio.y*endFade.x*endFade.y);

	gl_FragColor = color;
}