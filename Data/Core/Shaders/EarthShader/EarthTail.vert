#version 400 compatibility

out vec3 weight;
out vec3 localVertDir;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
	gl_TexCoord[2] = gl_MultiTexCoord2;
	gl_Position = ftransform();

	weight = gl_Color.rgb;
	localVertDir = normalize(gl_Vertex.xyz/gl_Vertex.w);
}