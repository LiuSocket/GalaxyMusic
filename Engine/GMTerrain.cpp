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
CGMTerrain::CGMTerrain(): m_vTileOffsetUniform(new osg::Uniform("tileOffset", osg::Vec3f(0.0f, 0.0f, 0.0f)))
{
	m_pCelestialScaleVisitor = new CGMCelestialScaleVisitor();

	// 初始化1级瓦片中心点方向
	vTileDirVec_1.reserve(24);
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
			osg::Vec3d vECEFDir = vCenter * 2;
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
			vTileDirVec_1.push_back(vECEFDir);
		}
	}
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

		osg::Vec3f vTileOffset = osg::Vec3f(0.0f, 0.0f, 0.0f);
		m_vTileOffsetUniform->get(vTileOffset);

		for (int i = 0; i < m_pTileRoot_0->getNumChildren(); i++)
		{
			// 先全部显示
			m_pTileRoot_0->getChild(i)->setNodeMask(~0);
		}

		// 同时看到的地形块可以有如下情况：
		// 1个：1赤道、1极地
		// 2个：2赤道、1赤道+1极地、2极地
		// 3个：2赤道+1极地
		// 4个：4赤道、2赤道+2极地、4极地
		// 遍历所有地形块，根据相机位置和朝向，决定哪些地形块需要显示，哪些地形块需要旋转
		for (auto& itr : m_sTileVec_1)
		{
			int iID = itr.iTileID;
			bool bVisible = false;
			for (int k = 0; k < itr.vTileDirVec.size(); k++)
			{
				double fDot = vCore2EyeDir * itr.vTileDirVec.at(k);
				if (fDot > 0.7)// 显示并旋转
				{
					bVisible = true;
					//旋转
					if (itr.bPolar)
					{
						itr.pTileTrans->asPositionAttitudeTransform()->setAttitude(osg::Quat(osg::PI*k, osg::Vec3d(0, 1, 0)));
						vTileOffset.z() = k*4;
						iID += 16 + vTileOffset.z();
					}
					else
					{
						if (0 == k) // posX
						{
							itr.pTileTrans->asPositionAttitudeTransform()->setAttitude(osg::Quat(0, osg::Vec3d(0, 0, 1)));
						}
						else if (1 == k) // negX
						{
							itr.pTileTrans->asPositionAttitudeTransform()->setAttitude(osg::Quat(osg::PI, osg::Vec3d(0, 0, 1)));
						}
						else if (2 == k) // posY
						{
							itr.pTileTrans->asPositionAttitudeTransform()->setAttitude(osg::Quat(osg::PI_2, osg::Vec3d(0, 0, 1)));
						}
						else // if (3 == k) negY
						{
							itr.pTileTrans->asPositionAttitudeTransform()->setAttitude(osg::Quat(osg::PI * 1.5, osg::Vec3d(0, 0, 1)));
						}

						if (0 == itr.iTileID || 3 == itr.iTileID)
						{
							vTileOffset.x() = k * 4;
							iID += vTileOffset.x();
						}
						else
						{
							vTileOffset.y() = k * 4;
							iID += vTileOffset.y();
						}
					}
					// 瓦片体贴图的偏移
					m_vTileOffsetUniform->set(vTileOffset);
					break;
				}
			}
			// 显示或隐藏
			if (bVisible)
			{
				if (0 == itr.pTileTrans->getNodeMask())
					itr.pTileTrans->setNodeMask(~0);

				osg::Node* pNode = m_pTileRoot_0->getChild(iID);
				if (pNode && 0 != pNode->getNodeMask())
					pNode->setNodeMask(0);
			}
			else
			{
				if (0 != itr.pTileTrans->getNodeMask())
					itr.pTileTrans->setNodeMask(0);
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
	_CreateTile_0();
	_CreateTile_1();

	return true;
}

bool CGMTerrain::_CreateTile_0()
{
	m_pTileRoot_0 = new osg::Group();
	m_pHieTerrainRootVector.at(1)->addChild(m_pTileRoot_0);

	m_sTileVec_0.reserve(24);
	for (int j = 0; j < 6; j++)
	{
		bool bPolar = (4 <= j);
		for (int i = 0; i < 4; i++)
		{
			osg::ref_ptr<osg::Geode> pTerrainQuaterGeode = new osg::Geode();
			osg::ref_ptr<osg::Geometry>	pTerrainQuaterGeom = _MakeHexahedronQuaterGeometry(bPolar, 31);

			double fUnit = m_pKernelData->fUnitArray->at(1);
			// 赤道地形块的ID为0-15，北极地形块的ID为16-19，南极地形块的ID为20-23
			m_pCelestialScaleVisitor->SetQuatorFace(true, j * 4 + i);
			m_pCelestialScaleVisitor->SetRadius(osg::WGS_84_RADIUS_EQUATOR / fUnit, osg::WGS_84_RADIUS_POLAR / fUnit);
			pTerrainQuaterGeom->accept(*m_pCelestialScaleVisitor);	// 改变大小

			m_pTileRoot_0->addChild(pTerrainQuaterGeode.get());
			pTerrainQuaterGeode->addDrawable(pTerrainQuaterGeom.get());

			STileData sTile;
			sTile.iTileID = i;
			sTile.bPolar = bPolar;
			m_sTileVec_0.push_back(sTile);
		}
	}
	return true;
}

bool CGMTerrain::_CreateTile_1()
{
	m_pTileRoot_1 = new osg::Group();
	m_pHieTerrainRootVector.at(1)->addChild(m_pTileRoot_1);

	m_sTileVec_1.reserve(8);
	for (int j = 0; j < 2; j++)
	{
		// 0: 赤道，1: 极地，每个地形块分为4个四分之一地形块，总共需要4个赤道和4个极地
		// 同时看到的地形块最多4个，可以有三种组合：4赤道，2赤道+2极地，4极地
		// 所以只需要创建4个赤道和4个极地地形块，然后根据需要旋转和显隐
		bool bPolar = (1 == j);
		for (int i = 0; i < 4; i++)
		{
			osg::ref_ptr<osg::PositionAttitudeTransform> pTerrainQuaterTrans = new osg::PositionAttitudeTransform();
			osg::ref_ptr<osg::Geode> pTerrainQuaterGeode = new osg::Geode();
			osg::ref_ptr<osg::Geometry>	pTerrainQuaterGeom = _MakeHexahedronQuaterGeometry(bPolar);

			double fUnit = m_pKernelData->fUnitArray->at(1);
			// 赤道地形块的ID为0-15，北极地形块的ID为16-19，南极地形块的ID为20-23
			m_pCelestialScaleVisitor->SetQuatorFace(true, bPolar ? 16 + i : i);
			m_pCelestialScaleVisitor->SetRadius(osg::WGS_84_RADIUS_EQUATOR / fUnit, osg::WGS_84_RADIUS_POLAR / fUnit);
			pTerrainQuaterGeom->accept(*m_pCelestialScaleVisitor);	// 改变大小

			m_pTileRoot_1->addChild(pTerrainQuaterTrans.get());
			pTerrainQuaterTrans->addChild(pTerrainQuaterGeode.get());
			pTerrainQuaterGeode->addDrawable(pTerrainQuaterGeom.get());

			STileData sTile;
			sTile.pTileTrans = pTerrainQuaterTrans;
			sTile.iTileID = i;
			sTile.bPolar = bPolar;
			if (bPolar)// 极地，有2*4种可能
			{
				for (int k = 0; k < 2; k++)
				{
					int iID = 16 + k * 4 + i;
					sTile.vTileDirVec.push_back(vTileDirVec_1.at(iID));
				}
			}
			else // 赤道，有4*4种可能
			{
				for (int k = 0; k < 4; k++)
				{
					int iID = k * 4 + i;
					sTile.vTileDirVec.push_back(vTileDirVec_1.at(iID));
				}
			}
			m_sTileVec_1.push_back(sTile);
		}
	}
	return true;
}

osg::Geometry* CGMTerrain::_MakeHexahedronQuaterGeometry(const bool bPolar, int iSegment) const
{
	// 为了效率，限制iSegment的上限，以防element超过65536，特意设置成2^n-1是为了保证高程图的分辨率是2^n
	iSegment = osg::clampBetween(iSegment, 3, 255);
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

			// 默认为临界值的点，防止三角函数失效
			float fLon = bPolar ? osg::PI_2 : 0.0f;// 弧度
			float fLat = bPolar ? osg::PI_2 : 0.0f;// 弧度
			if (vCenter != vDir)
			{
				fLon = atan2(vDir.y(), vDir.x());// 弧度 (-PI, PI]
				fLat = asin(vDir.z());// 弧度 [-PI/2, PI/2]
			}

			verts->push_back(vDir);
			// 0层纹理单元 xy = WGS84对应的UV，[0.0, 1.0]
			// 1层纹理单元 xy = 瓦片体贴图UV，[0.0, 1.0]; z = 面对应的编号0-23
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