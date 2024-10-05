#version 450 compatibility

#pragma import_defines(EARTH, TILE)

#ifdef EARTH
uniform float unit;
uniform vec4 coordScale_Earth;
uniform sampler2DArray DEMTex;
#endif // EARTH
#ifdef TILE
uniform vec3 tileOffsetLonLat;
uniform vec3 tileOffsetID;
#endif // TILE

out vec2 texCoord_0;
out vec3 texCoord_1;
out vec4 viewPos;
out vec3 viewNormal;

float DEM(in float normDEM)
{
	float x = 2*normDEM-1;
	return sign(x)*x*x*1e4;
}

void main()
{
	viewNormal = normalize(gl_NormalMatrix*gl_Normal);
	vec4 modelVertex = gl_Vertex;

	vec2 coord0 = gl_MultiTexCoord0.xy;
	vec3 coord1 = gl_MultiTexCoord1.xyz;
#ifdef TILE
	if(TILE>0)
	{
		bool isEquator = coord1.z < 15.5;
		bool isEquator03 = abs(coord1.z - 1.5) > 1.0;
		bool isPolar03 = abs(coord1.z - 17.5) > 1.0;
		bool isSouth = tileOffsetLonLat.z > 0.0;
		// coord 0
		coord0.x = isEquator ?
			isEquator03 ? coord0.x + tileOffsetLonLat.x : coord0.x + tileOffsetLonLat.y :
			isSouth ? (isPolar03 ? (1.5 - coord0.x) : (0.5 - coord0.x)) : coord0.x;
		coord0.y = isEquator ? coord0.y : (isSouth ? 1.0 - coord0.y : coord0.y);
		// coord 1 	
		coord1.z += isEquator ? (isEquator03 ? tileOffsetID.x : tileOffsetID.y) : tileOffsetID.z;
	}
#endif // TILE

#ifdef EARTH
	vec3 DEMCoord = coord1;
	DEMCoord.xy = (DEMCoord.xy - 0.5)*coordScale_Earth.w + 0.5;
	float elev = DEM(texture(DEMTex, DEMCoord).r); // meter
	modelVertex.xyz += max(0, elev/unit)*gl_Normal;
	gl_Position = gl_ModelViewProjectionMatrix*modelVertex;
#else // not EARTH
	gl_Position = ftransform();
#endif // EARTH or not

	viewPos = gl_ModelViewMatrix*modelVertex;
	texCoord_0 = coord0;
	texCoord_1 = coord1;
}