//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMVolumeBasic.h
/// @brief		Galaxy-Music Engine - GMEngine.cpp
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.10
//////////////////////////////////////////////////////////////////////////

#include "GMEngine.h"

using namespace GM;

int main(int argc, char **argv)
{
	osg::ref_ptr<osgViewer::Viewer> pViewer = GM_ENGINE_PTR->CreateViewer();
	while (!pViewer->done())
	{
		GM_ENGINE_PTR->Update();
	}
	return 0;
}