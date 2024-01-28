#version 400 compatibility

uniform vec2 planetRadius;

out float fall;

void main()
{
	vec4 viewVertex = gl_ModelViewMatrix*gl_Vertex;
	vec3 viewPos = viewVertex.xyz / viewVertex.w;

	fall = gl_MultiTexCoord0.x*(1-exp2(min(0,planetRadius.x*100-length(viewPos))*0.005/planetRadius.x));

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}