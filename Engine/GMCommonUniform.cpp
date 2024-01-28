//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMCommonUniform.cpp
/// @brief		Galaxy-Music Engine - Common Uniform
/// @version	1.0
/// @author		LiuTao
/// @date		2022.08.21
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMEngine.h"
#include "GMCommonUniform.h"
#include <osg/Timer>

using namespace GM;

/*************************************************************************
 Macro Defines
*************************************************************************/
#define PULSE_NUM				128 		// max number of audio pulse

/*************************************************************************
CGMCommonUniform Methods
*************************************************************************/

CGMCommonUniform::CGMCommonUniform():
	m_vScreenSizeUniform(new osg::Uniform("screenSize", osg::Vec3f(1920.0f, 1080.0f, 0.5f))),
	m_fTimeUniform(new osg::Uniform("times", 0.0f)),
	m_vStarColorUniform(new osg::Uniform("playingStarColor", osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f))),
	m_fLevelArrayUniform(new osg::Uniform(osg::Uniform::Type::FLOAT, "level", PULSE_NUM)),
	m_fUnitUniform(new osg::Uniform("unit", 1.0f)),
	m_vStarHiePosUniform(new osg::Uniform("starWorldPos", osg::Vec3f(0.0f, 0.0f, 0.0f))),
	m_fGalaxyAlphaUniform(new osg::Uniform("galaxyAlpha", 1.0f)),
	m_mDeltaVPMatrixUniform(new osg::Uniform("deltaViewProjMatrix", osg::Matrixf())),
	m_mMainInvProjUniform(new osg::Uniform("invProjMatrix", osg::Matrixf())),
	m_vEyeFrontDirUniform(new osg::Uniform("eyeFrontDir", osg::Vec3f(0.0f, 0.0f, -1.0f))),
	m_vEyeRightDirUniform(new osg::Uniform("eyeRightDir", osg::Vec3f(1.0f, 0.0f, 0.0f))),
	m_vEyeUpDirUniform(new osg::Uniform("eyeUpDir", osg::Vec3f(0.0f, 1.0f, 0.0f))),
	m_vViewUpUniform(new osg::Uniform("viewUp", osg::Vec3f(0.0f, 1.0f, 0.0f)))
{
	for (int i = 0; i < PULSE_NUM; i++)
	{
		m_fLevelArrayUniform->setElement(i, 0.0f);
	}
}

CGMCommonUniform::~CGMCommonUniform()
{
}

void CGMCommonUniform::Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData)
{
	m_pKernelData = pKernelData;

	int iScreenWidth = pConfigData->iScreenWidth;
	int iScreenHeight = pConfigData->iScreenHeight;

	m_vScreenSizeUniform->set(osg::Vec3f(iScreenWidth, iScreenHeight, 0.5f));
}

void CGMCommonUniform::Update(double dDeltaTime)
{
	double fTimes = osg::Timer::instance()->time_s();
	m_fTimeUniform->set(float(fTimes));
}

void CGMCommonUniform::UpdateLater(double dDeltaTime)
{
	osg::Matrixd mViewMatrix = GM_View->getCamera()->getViewMatrix();
	osg::Vec3d vEye, vCenter, vUp;
	GM_View->getCamera()->getViewMatrixAsLookAt(vEye, vCenter, vUp);

	int iHie = GM_ENGINE.GetHierarchy();

	if (2 >= iHie)
	{
		// 将摄像机view空间下的Up向量传入shader中
		osg::Vec3d vWorldSky = vEye;
		double fEye2Center = vWorldSky.normalize();
		osg::Vec4d v4ViewUp = mViewMatrix.preMult(osg::Vec4d(vWorldSky, 0.0));
		osg::Vec3d vViewUp = osg::Vec3d(v4ViewUp.x(), v4ViewUp.y(), v4ViewUp.z());
		vViewUp.normalize();
		m_vViewUpUniform->set(osg::Vec3f(vViewUp));
	}
}

bool CGMCommonUniform::UpdateHierarchy(int iHieNew)
{
	m_fUnitUniform->set(float(GM_ENGINE.GetUnit(iHieNew)));
	return true;
}

void CGMCommonUniform::ResizeScreen(const int width, const int height)
{
	m_vScreenSizeUniform->set(osg::Vec3f(width, height, 0.5f));
}

void CGMCommonUniform::SetAudioLevel(const float fLevel)
{
	for (int i = PULSE_NUM - 2; i >= 0; i--)
	{
		float fL;
		m_fLevelArrayUniform->getElement(i, fL);
		m_fLevelArrayUniform->setElement(i + 1, fL);
	}
	m_fLevelArrayUniform->setElement(0, fLevel);
}