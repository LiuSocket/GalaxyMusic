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
#include <osg/PositionAttitudeTransform>
#include <osgDB/ReadFile>

using namespace GM;

/*************************************************************************
Class
*************************************************************************/

/*************************************************************************
CGMTerrain Methods
*************************************************************************/

/** @brief 构造 */
CGMTerrain::CGMTerrain()
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
	default:
		break;
	}

	return true;
}

/** @brief 更新(在主相机更新姿态之后) */
bool CGMTerrain::UpdateLater(double dDeltaTime)
{
	int iHie = m_pKernelData->iHierarchy;

	switch (iHie)
	{
	case 0:
	{
	}
	break;
	case 1:
	{
		osg::Vec3d vEye, vCenter, vUp;
		GM_View->getCamera()->getViewMatrixAsLookAt(vEye, vCenter, vUp);
		// 计算相机朝向
		osg::Vec3d vFrontDir = vCenter - vEye;
		vFrontDir.normalize();
		// 计算相机位置背向地心方向的单位向量
		osg::Vec3d vCore2EyeDir = vEye;
		vCore2EyeDir.normalize();

		// 同时看到的地形块可以有如下情况：
		// 1个：1赤道、1极地
		// 2个：2赤道、1赤道+1极地、2极地
		// 3个：2赤道+1极地
		// 4个：4赤道、2赤道+2极地、4极地
		// 遍历所有地形块，根据相机位置和朝向，决定哪些地形块需要显示，哪些地形块需要旋转
		for (auto& itr : m_sQuatorVec)
		{
			bool bVisible = false;
			for (int k = 0; k < itr.vCenterDirVec.size(); k++)
			{
				double fDot = vCore2EyeDir * itr.vCenterDirVec.at(k);
				if (fDot > 0.7)// 显示并旋转
				{
					bVisible = true;
					//旋转
					if (itr.bPolar)
					{
						itr.pQuatorTrans->asPositionAttitudeTransform()->setAttitude(osg::Quat(osg::PI*k, osg::Vec3d(0, 1, 0)));
					}
					else
					{
						if (0 == k) // posX
						{
							itr.pQuatorTrans->asPositionAttitudeTransform()->setAttitude(osg::Quat(0, osg::Vec3d(0, 0, 1)));
						}
						else if (1 == k) // negX
						{
							itr.pQuatorTrans->asPositionAttitudeTransform()->setAttitude(osg::Quat(osg::PI, osg::Vec3d(0, 0, 1)));
						}
						else if (2 == k) // posY
						{
							itr.pQuatorTrans->asPositionAttitudeTransform()->setAttitude(osg::Quat(osg::PI_2, osg::Vec3d(0, 0, 1)));
						}
						else // if (3 == k) negY
						{
							itr.pQuatorTrans->asPositionAttitudeTransform()->setAttitude(osg::Quat(osg::PI * 1.5, osg::Vec3d(0, 0, 1)));
						}
					}
					break;
				}
			}
			// 显示或隐藏
			if (bVisible)
			{
				if (0 == itr.pQuatorTrans->getNodeMask())
				{
					itr.pQuatorTrans->setNodeMask(~0);
				}
			}
			else
			{
				if (0 != itr.pQuatorTrans->getNodeMask())
				{
					itr.pQuatorTrans->setNodeMask(0);
				}
			}
		}
	}
	break;
	default:
		break;
	}

	return true;
}

void CGMTerrain::ResizeScreen(const int iW, const int iH)
{
}

void CGMTerrain::SetVisible(const bool bVisible)
{
	if (bVisible)
	{
		if (0 == m_pHieTerrainRootVector.at(0)->getNodeMask())
		{
			m_pHieTerrainRootVector.at(0)->setNodeMask(~0);
			m_pHieTerrainRootVector.at(1)->setNodeMask(~0);
		}
	}
	else
	{
		if (0 != m_pHieTerrainRootVector.at(0)->getNodeMask())
		{
			m_pHieTerrainRootVector.at(0)->setNodeMask(0);
			m_pHieTerrainRootVector.at(1)->setNodeMask(0);
		}
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
		if (GM_Root->containsNode(m_pHieTerrainRootVector.at(0)))
		{
			GM_Root->removeChild(m_pHieTerrainRootVector.at(0));
		}
		if (!(GM_Root->containsNode(m_pHieTerrainRootVector.at(1))))
		{
			GM_Root->addChild(m_pHieTerrainRootVector.at(1));
		}
	}
	break;
	case 2:
	{
		if (GM_Root->containsNode(m_pHieTerrainRootVector.at(1)))
		{
			GM_Root->removeChild(m_pHieTerrainRootVector.at(1));
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
	std::vector<osg::Vec3d> vAllCenterDirVec;
	vAllCenterDirVec.reserve(24);
	osg::Vec3d vCenter = osg::Vec3d(0, 1, 0);
	osg::Vec3d vAxisX = osg::Vec3d(1, 0, 0);
	osg::Vec3d vAxisY = osg::Vec3d(0, 0, 1);
	for (int i = 0; i < 6; i++)
	{
		switch (i)
		{
		case 0:
		{
			// posX
			vCenter = osg::Vec3d(1, 0, 0);
			vAxisX = osg::Vec3d(0, 1, 0);
			vAxisY = osg::Vec3d(0, 0, 1);
		}
		break;
		case 1:
		{
			// negX
			vCenter = osg::Vec3d(-1, 0, 0);
			vAxisX = osg::Vec3d(0, -1, 0);
			vAxisY = osg::Vec3d(0, 0, 1);
		}
		break;
		case 2:
		{
			// posY
			vCenter = osg::Vec3d(0, 1, 0);
			vAxisX = osg::Vec3d(-1, 0, 0);
			vAxisY = osg::Vec3d(0, 0, 1);
		}
		break;
		case 3:
		{
			// negY
			vCenter = osg::Vec3d(0, -1, 0);
			vAxisX = osg::Vec3d(1, 0, 0);
			vAxisY = osg::Vec3d(0, 0, 1);
		}
		break;
		case 4:
		{
			// posZ
			vCenter = osg::Vec3d(0, 0, 1);
			vAxisX = osg::Vec3d(0, 1, 0);
			vAxisY = osg::Vec3d(-1, 0, 0);
		}
		break;
		case 5:
		{
			// negZ
			vCenter = osg::Vec3d(0, 0, -1);
			vAxisX = osg::Vec3d(0, 1, 0);
			vAxisY = osg::Vec3d(1, 0, 0);
		}
		break;
		default:
			break;
		}

		for (int j = 0; j < 4; j++)
		{
			osg::Vec3d vECEFDir = vCenter * 2.5;// 2.1是为了让地形块中点与四个边界点的距离尽量相等
			switch (j)
			{
			case 0:
			{
				vECEFDir += vAxisX + vAxisY;
			}
			break;
			case 1:
			{
				vECEFDir -= vAxisX - vAxisY;
			}
			break;
			case 2:
			{
				vECEFDir -= vAxisX + vAxisY;
			}
			break;
			case 3:
			{
				vECEFDir += vAxisX - vAxisY;
			}
			break;
			default:
				break;
			}
			vECEFDir.normalize();
			vAllCenterDirVec.push_back(vECEFDir);
		}
	}

	m_sQuatorVec.reserve(8);
	for (int j = 0; j < 2; j++)
	{
		// 0: 赤道，1: 极地，每个地形块分为4个四分之一地形块，总共需要4个赤道和4个极地
		// 同时看到的地形块最多4个，可以有三种组合：4赤道，2赤道+2极地，4极地
		// 所以只需要创建4个赤道和4个极地地形块，然后根据需要旋转和显隐
		bool bPolar = (1 == j);
		for (int i = 0; i < 4; i++)
		{
			osg::ref_ptr<osg::Transform> pTerrainQuaterTrans = new osg::PositionAttitudeTransform();
			osg::ref_ptr<osg::Geode> pTerrainQuaterGeode = new osg::Geode();
			osg::ref_ptr<osg::Geometry>	pTerrainQuaterGeom = _MakeHexahedronQuaterGeometry(bPolar);

			double fUnit = m_pKernelData->fUnitArray->at(1);
			// 赤道地形块的ID为0-15，北极地形块的ID为16-19，南极地形块的ID为20-23
			m_pCelestialScaleVisitor->SetQuatorFace(true, bPolar ? 16 + i : i);
			m_pCelestialScaleVisitor->SetRadius(osg::WGS_84_RADIUS_EQUATOR / fUnit, osg::WGS_84_RADIUS_POLAR / fUnit);
			pTerrainQuaterGeom->accept(*m_pCelestialScaleVisitor);	// 改变大小

			m_pHieTerrainRootVector.at(1)->addChild(pTerrainQuaterTrans.get());
			pTerrainQuaterTrans->addChild(pTerrainQuaterGeode.get());
			pTerrainQuaterGeode->addDrawable(pTerrainQuaterGeom.get());

			SQuatorData sQuator;
			sQuator.pQuatorTrans = pTerrainQuaterTrans;
			sQuator.iQuatorID = j;
			sQuator.bPolar = bPolar;
			if (bPolar)// 极地，有8种可能
			{
				for (int k = 0; k < 2; k++)
				{
					int iID = 16 + k * 4 + i;
					sQuator.vCenterDirVec.push_back(vAllCenterDirVec.at(iID));
				}
			}
			else // 赤道，有16种可能
			{
				for (int k = 0 ; k < 4 ; k++)
				{
					int iID = k * 4 + i;
					sQuator.vCenterDirVec.push_back(vAllCenterDirVec.at(iID));
				}
			}
			m_sQuatorVec.push_back(sQuator);
		}
	}
	return true;
}

osg::Geometry* CGMTerrain::_MakeHexahedronQuaterGeometry(const bool bPolar, int iSegment) const
{
	// 为了效率，限制iSegment的上限，以防element超过65536
	iSegment = osg::clampBetween(iSegment, 2, 256);
	float fSize = float(iSegment);
	int iVertPerEdge = iSegment + 1;
	int iVertPerFace = iVertPerEdge * iVertPerEdge;
	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);

	osg::Vec3Array* verts = new osg::Vec3Array();
	osg::Vec2Array* coords0 = new osg::Vec2Array();
	osg::Vec3Array* coords1 = new osg::Vec3Array();
	osg::Vec3Array* normals = new osg::Vec3Array();
	osg::DrawElementsUShort* el = new osg::DrawElementsUShort(GL_TRIANGLES);

	verts->reserve(iVertPerFace);
	coords0->reserve(iVertPerFace);
	coords1->reserve(iVertPerFace);
	normals->reserve(iVertPerFace);
	el->reserve(iSegment * iSegment * 6);

	geom->setTexCoordArray(0, coords0);
	geom->setTexCoordArray(1, coords1);
	geom->setNormalArray(normals);
	geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	geom->setVertexArray(verts);
	geom->addPrimitiveSet(el);

	osg::Vec3 vCenter = osg::Vec3(1, 0, 0);
	osg::Vec3 vAxisX = osg::Vec3(0, 1, 0);
	osg::Vec3 vAxisY = osg::Vec3(0, 0, 1);
	if (bPolar)
	{
		vCenter = osg::Vec3(0, 0, 1);
		vAxisX = osg::Vec3(0, 1, 0);
		vAxisY = osg::Vec3(-1, 0, 0);
	}

	for (int y = 0; y <= iSegment; ++y)
	{
		for (int x = 0; x <= iSegment; ++x)
		{
			osg::Vec3 vDir = vCenter + vAxisX * x / fSize + vAxisY * y / fSize;
			vDir.normalize();

			float fLon = atan2(vDir.y(), vDir.x());// 弧度 (-PI, PI]
			float fLat = asin(vDir.z());// 弧度 [-PI/2, PI/2]

			verts->push_back(vDir);
			// 0层纹理单元 xy = WGS84对应的UV，[0.0, 1.0]
			// 1层纹理单元 xy = 四分之一面体贴图UV，[0.0, 1.0]; z = 面对应的编号0-23
			coords0->push_back(osg::Vec2(0.5f + fLon / (osg::PI * 2), 0.5f + fLat / (osg::PI)));
			coords1->push_back(osg::Vec3(float(x) / float(iSegment), float(y) / float(iSegment), 0));
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