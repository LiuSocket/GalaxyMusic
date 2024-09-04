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
	m_pKernelData(nullptr), m_pConfigData(nullptr), m_pCommonUniform(nullptr),
	m_strGalaxyShaderPath("Shaders/GalaxyShader/"),
	m_strTerrainShaderPath("Shaders/TerrainShader/"),
	m_pCelestialScaleVisitor(nullptr)
{
	m_pCelestialScaleVisitor = new CGMCelestialScaleVisitor();
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
		osg::ref_ptr<osg::Group> pRoot = new osg::Group();
		m_pHieTerrainRootVector.push_back(pRoot);
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
	if (2 > iHie)
	{
		return m_pHieTerrainRootVector.at(iHie);
	}
	else
	{
		return nullptr;
	}
}

bool CGMTerrain::CreateTerrain()
{
	// 创建地形，用于0级空间
	_CreateTerrain_0();

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
	for (int j = 0; j < 2; j++)
	{
		// 0: 赤道，1: 极地
		bool bPolar = (1 == j);
		for (int i = 0; i < 4; i++)
		{
			osg::ref_ptr<osg::Transform> pTerrainQuaterTrans = new osg::Transform();
			osg::ref_ptr<osg::Geode> pTerrainQuaterGeode = new osg::Geode();
			osg::ref_ptr<osg::Geometry>	pTerrainQuaterGeom = _MakeHexahedronQuaterGeometry(bPolar);

			double fUnit = m_pKernelData->fUnitArray->at(1);
			m_pCelestialScaleVisitor->SetRadius(osg::WGS_84_RADIUS_EQUATOR / fUnit, osg::WGS_84_RADIUS_POLAR / fUnit);
			pTerrainQuaterGeom->accept(*m_pCelestialScaleVisitor);	// 改变大小

			m_pHieTerrainRootVector.at(1)->addChild(pTerrainQuaterTrans.get());
			pTerrainQuaterTrans->addChild(pTerrainQuaterGeode.get());
			pTerrainQuaterGeode->addDrawable(pTerrainQuaterGeom.get());

			if (bPolar)
				m_pTerrainPolarTransVec.push_back(pTerrainQuaterTrans);
			else
				m_pTerrainEquatorTransVec.push_back(pTerrainQuaterTrans);
		}
	}
	return true;
}

osg::Geometry* CGMTerrain::_MakeHexahedronQuaterGeometry(const bool bPolar, int iSegment) const
{
	// 为了效率，限制iSegment的上限，以防element超过65536
	iSegment = osg::clampBetween(iSegment, 2, 256);
	float fHalfSize = float(iSegment);
	int iVertPerEdge = iSegment + 1;
	int iVertPerFace = iVertPerEdge * iVertPerEdge;
	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);

	osg::Vec3Array* verts = new osg::Vec3Array();
	osg::Vec3Array* coords0 = new osg::Vec3Array();
	osg::Vec3Array* normals = new osg::Vec3Array();
	osg::DrawElementsUShort* el = new osg::DrawElementsUShort(GL_TRIANGLES);

	verts->reserve(iVertPerFace);
	coords0->reserve(iVertPerFace);
	normals->reserve(iVertPerFace);
	el->reserve(iSegment * iSegment * 6);

	geom->setTexCoordArray(0, coords0);
	geom->setNormalArray(normals);
	geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	geom->setVertexArray(verts);
	geom->addPrimitiveSet(el);

	osg::Vec3 vCenter = osg::Vec3(0, 1, 0);
	osg::Vec3 vAxisX = osg::Vec3(0, 0, 1);
	osg::Vec3 vAxisY = osg::Vec3(1, 0, 0);
	if (bPolar)
	{
		vCenter = osg::Vec3(0, 0, 1);
		vAxisX = osg::Vec3(1, 0, 0);
		vAxisY = osg::Vec3(0, 1, 0);
	}

	for (int y = 0; y <= iSegment; ++y)
	{
		for (int x = 0; x <= iSegment; ++x)
		{
			osg::Vec3 vDir = vCenter
				+ vAxisX * (x - fHalfSize) / fHalfSize
				+ vAxisY * (y - fHalfSize) / fHalfSize;
			vDir.normalize();

			float fLon = atan2(vDir.y(), vDir.x());// 弧度 (-PI, PI]
			float fLat = asin(vDir.z());// 弧度 [-PI/2, PI/2]

			verts->push_back(vDir);
			// 0层纹理单元 xy = 六面体贴图UV，[0.0, 1.0];
			coords0->push_back(osg::Vec3(float(x) / float(iSegment), float(y) / float(iSegment), fLat));
			normals->push_back(vDir);
			if (x < iSegment && y < iSegment)
			{
				el->push_back(_GetVertIndex(x, y, iSegment));
				el->push_back(_GetVertIndex(x + 1, y, iSegment));
				el->push_back(_GetVertIndex(x, y + 1, iSegment));
				el->push_back(_GetVertIndex(x + 1, y, iSegment));
				el->push_back(_GetVertIndex(x + 1, y + 1, iSegment));
				el->push_back(_GetVertIndex(x, y + 1, iSegment));
			}
		}
	}
	return geom;
}