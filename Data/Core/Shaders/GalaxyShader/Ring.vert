#version 400 compatibility

uniform mat4 ringShadowMatrix;

out vec3 viewPos;
out vec3 shadowVertPos;

void main()
{
	vec4 viewVertex = gl_ModelViewMatrix*gl_Vertex;
	viewPos = viewVertex.xyz / viewVertex.w;
	shadowVertPos = (ringShadowMatrix*gl_Vertex).xyz;

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}