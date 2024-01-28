#version 400 compatibility

const float M_PI = 3.141592657;

uniform vec3 viewLight;
uniform vec2 planetRadius;// x = fEquatorR, y = fPolarR
uniform mat4 ringShadowMatrix;

uniform sampler2D ringColorTex;

in vec3 viewPos;
in vec3 shadowVertPos;

vec3 ToneMapping(vec3 color)
{
	const float A = 2.51;
	const float B = 0.03;
	const float C = 2.43;
	const float D = 0.59;
	const float E = 0.14;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}

float ToneMapping(float x)
{
	const float A = 2.51;
	const float B = 0.03;
	const float C = 2.43;
	const float D = 0.59;
	const float E = 0.14;
	return (x * (A * x + B)) / (x * (C * x + D) + E);
}

void main()
{
	vec4 baseColor = texture(ringColorTex, gl_TexCoord[0].xy);
	vec3 viewDir = normalize(viewPos);
	float dotVL = dot(viewDir, viewLight);
	const float gBack = -0.3;
	float backScattering = (1-gBack*gBack)/(4*M_PI*pow(1+gBack*gBack-2*gBack*dotVL,1.5));
	const float gForward = 0.99;
	float forwardScattering = (1-gForward*gForward)/(4*M_PI*pow(1+gForward*gForward-2*gForward*dotVL,1.5));
	float scattering = backScattering + forwardScattering;
	// planet shadow
	vec2 shadowVertXYDir = normalize(shadowVertPos.xy);
	float planetR = mix(planetRadius.x, planetRadius.y, abs(ringShadowMatrix[2].y*shadowVertXYDir.y));
	float planetShadow = step(0,shadowVertPos.z) * clamp((planetR-length(shadowVertPos.xy))*1e4,0,1);
	float ringScattering = scattering*(1-planetShadow);
	vec3 color = pow(ToneMapping(baseColor.rgb*baseColor.rgb*ringScattering), vec3(1/2.2));
	float ringAlpha = mix(baseColor.a, sqrt(baseColor.a), ToneMapping(ringScattering));
	gl_FragColor = vec4(color, ringAlpha);
}