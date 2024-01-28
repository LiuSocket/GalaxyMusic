//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMTerrain.cpp
/// @brief		Galaxy-Music Engine - GMTerrain
/// @version	1.0
/// @author		LiuTao
/// @date		2023.12.31
//////////////////////////////////////////////////////////////////////////

#include "GMTerrain.h"
#include "GMEngine.h"
#include "GMKit.h"
#include <osg/CullFace>
#include <osgDB/ReadFile>

using namespace GM;

/*************************************************************************
Class
*************************************************************************/
namespace GM
{

}	// GM

/*************************************************************************
CGMTerrain Methods
*************************************************************************/

/** @brief 构造 */
CGMTerrain::CGMTerrain() :
	m_strGalaxyShaderPath("Shaders/GalaxyShader/"),
	m_strTerrainShaderPath("Shaders/TerrainShader/")
{
	m_pTerrainRoot_1 = new osg::Group();
}

/** @brief 析构 */
CGMTerrain::~CGMTerrain()
{
}

/** @brief 初始化 */
bool CGMTerrain::Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform)
{
	m_pKernelData = pKernelData;
	m_pConfigData = pConfigData;
	m_pCommonUniform = pCommonUniform;

	for (int i = 0; i < 2; i++)
	{
		osg::ref_ptr<osg::Group> _pRoot = new osg::Group();
		m_pHieTerrainRootVector.push_back(_pRoot);
	}

	return true;
}

/** @brief 更新 */
bool CGMTerrain::Update(double dDeltaTime)
{
	double fTimes = osg::Timer::instance()->time_s();
	int iHie = m_pKernelData->iHierarchy;

	switch (iHie)
	{
	case 0:
	{
	}
	break;
	case 1:
	{

	}
	break;
	case 2:
	{
	}
	break;
	default:
		break;
	}

	return true;
}

/** @brief 更新(在主相机更新姿态之后) */
bool CGMTerrain::UpdateLater(double dDeltaTime)
{
	int iHie = m_pKernelData->iHierarchy;

	osg::Matrixd mViewMatrix = GM_View->getCamera()->getViewMatrix();
	osg::Matrixd mProjMatrix = GM_View->getCamera()->getProjectionMatrix();

	return true;
}

/** @brief 加载 */
bool CGMTerrain::Load()
{
	std::string strGalaxyShader = m_pConfigData->strCorePath + m_strGalaxyShaderPath;
	std::string strTerrainShader = m_pConfigData->strCorePath + m_strTerrainShaderPath;

	return true;
}

void CGMTerrain::ResizeScreen(const int iW, const int iH)
{
}

osg::Node* CGMTerrain::GetTerrainRoot(const int iHie) const
{
	if (0 == iHie)
	{
		return nullptr;
	}
	else if (1 == iHie)
	{
		return m_pTerrainRoot_1.get();
	}
	else
	{
		return nullptr;
	}
}

bool CGMTerrain::CreateTerrain()
{
	// 创建地形，用于1级空间
	_CreateTerrain_1();
	return true;
}

bool CGMTerrain::UpdateHierarchy(int iHieNew)
{
	switch (iHieNew)
	{
	case 0:
	{
		if (!(GM_Root->containsNode(m_pHieTerrainRootVector.at(0))))
		{
			GM_Root->addChild(m_pHieTerrainRootVector.at(0));
		}
		if (GM_Root->containsNode(m_pHieTerrainRootVector.at(1)))
		{
			GM_Root->removeChild(m_pHieTerrainRootVector.at(1));
		}
	}
	break;
	case 1:
	{
		if (!(GM_Root->containsNode(m_pHieTerrainRootVector.at(1))))
		{
			GM_Root->addChild(m_pHieTerrainRootVector.at(1));
		}
		if (GM_Root->containsNode(m_pHieTerrainRootVector.at(0)))
		{
			GM_Root->removeChild(m_pHieTerrainRootVector.at(0));
		}
	}
	break;
	default:
		break;
	}

	return true;
}

bool CGMTerrain::_CreateTerrain_0()
{
	return true;
}

bool CGMTerrain::_CreateTerrain_1()
{
	return true;
}