#version 400 compatibility
#pragma import_defines(TILE_LEVEL)

void main()
{
#ifdef TILE_LEVEL
	gl_PointSize = 10*gl_MultiTexCoord0.y*exp2(TILE_LEVEL);
#else
	gl_PointSize = 10*gl_MultiTexCoord0.y;
#endif
	gl_Position = ftransform();
}