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
CGMTerrain::CGMTerrain():
	m_vTileOffsetLonLatUniform(new osg::Uniform("tileOffsetLonLat", osg::Vec3f(0.0f, 0.0f, 0.0f))),
	m_vTileOffsetIDUniform(new osg::Uniform("tileOffsetID", osg::Vec3f(0.0f, 0.0f, 0.0f)))
{
	osg::ref_ptr<osg::Group> pRoot_0h = new osg::Group();	// 0h = 0+half
	osg::ref_ptr<osg::Group> pRoot_1 = new osg::Group();	// 1 = 1
	osg::ref_ptr<osg::Group> pRoot_1h = new osg::Group();	// 1h = 1+half
	pRoot_0h->setName("TileRoot_0.5");
	pRoot_1->setName("TileRoot_1.0");
	pRoot_1h->setName("TileRoot_1.5");
	m_pHie1_TileMap[0.5] = pRoot_0h;
	m_pHie1_TileMap[1.0] = pRoot_1;
	m_pHie1_TileMap[1.5] = pRoot_1h;

	m_pCelestialScaleVisitor = new CGMCelestialScaleVisitor();
	// 初始化6个面XYZ方向
	m_vFaceXYZVec.reserve(6);
	m_vFaceXYZVec.push_back(SFaceXYZ(osg::Vec3d(0, 1, 0), osg::Vec3d(0, 0, 1), osg::Vec3d(1, 0, 0)));		// 0
	m_vFaceXYZVec.push_back(SFaceXYZ(osg::Vec3d(0, -1, 0), osg::Vec3d(0, 0, 1), osg::Vec3d(-1, 0, 0)));	// 1
	m_vFaceXYZVec.push_back(SFaceXYZ(osg::Vec3d(-1, 0, 0), osg::Vec3d(0, 0, 1), osg::Vec3d(0, 1, 0)));	// 2
	m_vFaceXYZVec.push_back(SFaceXYZ(osg::Vec3d(1, 0, 0), osg::Vec3d(0, 0, 1), osg::Vec3d(0, -1, 0)));	// 3
	m_vFaceXYZVec.push_back(SFaceXYZ(osg::Vec3d(0, 1, 0), osg::Vec3d(-1, 0, 0), osg::Vec3d(0, 0, 1)));	// 4
	m_vFaceXYZVec.push_back(SFaceXYZ(osg::Vec3d(0, 1, 0), osg::Vec3d(1, 0, 0), osg::Vec3d(0, 0, -1)));	// 5

	// 初始化所有瓦片的中心点方向
	std::vector<osg::Vec3d> vTileDirVec_0;
	std::vector<osg::Vec3d> vTileDirVec_1;
	std::vector<osg::Vec3d> vTileDirVec_2;
	std::vector<osg::Vec3d> vTileDirVec_3;
	std::vector<osg::Vec3d> vTileDirVec_4;
	std::vector<osg::Vec3d> vTileDirVec_5;
	vTileDirVec_0.reserve(6);
	vTileDirVec_1.reserve(6 * 4);
	vTileDirVec_2.reserve(6 * 4 * 4);
	vTileDirVec_3.reserve(6 * 4 * 4 * 4);
	vTileDirVec_4.reserve(6 * 4 * 4 * 4 * 4);
	vTileDirVec_5.reserve(6 * 4 * 4 * 4 * 4 * 4);

	for (int i0 = 0; i0 < 6; i0++)
	{
		// 初始化0层瓦片的6个面的中心点方向
		std::vector<int> iTileVec0;
		iTileVec0.push_back(i0);
		vTileDirVec_0.push_back(_GetTileCenterDir(iTileVec0));
		for (int i1 = 0; i1 < 4; i1++)
		{
			// 初始化1层瓦片的4象限的中心点方向
			std::vector<int> iTileVec1;
			iTileVec1.push_back(i0);
			iTileVec1.push_back(i1);
			vTileDirVec_1.push_back(_GetTileCenterDir(iTileVec1));
			for (int i2 = 0; i2 < 4; i2++)
			{
				// 初始化2层瓦片的16象限的中心点方向
				std::vector<int> iTileVec2;
				iTileVec2.push_back(i0);
				iTileVec2.push_back(i1);
				iTileVec2.push_back(i2);
				vTileDirVec_2.push_back(_GetTileCenterDir(iTileVec2));
				for (int i3 = 0; i3 < 4; i3++)
				{
					// 初始化3层瓦片的64象限的中心点方向
					std::vector<int> iTileVec3;
					iTileVec3.push_back(i0);
					iTileVec3.push_back(i1);
					iTileVec3.push_back(i2);
					iTileVec3.push_back(i3);
					vTileDirVec_3.push_back(_GetTileCenterDir(iTileVec3));
					for (int i4 = 0; i4 < 4; i4++)
					{
						// 初始化4层瓦片的256象限的中心点方向
						std::vector<int> iTileVec4;
						iTileVec4.push_back(i0);
						iTileVec4.push_back(i1);
						iTileVec4.push_back(i2);
						iTileVec4.push_back(i3);
						iTileVec4.push_back(i4);
						vTileDirVec_4.push_back(_GetTileCenterDir(iTileVec4));
						for (int i5 = 0; i5 < 4; i5++)
						{
							// 初始化5层瓦片的1024象限的中心点方向
							std::vector<int> iTileVec5;
							iTileVec5.push_back(i0);
							iTileVec5.push_back(i1);
							iTileVec5.push_back(i2);
							iTileVec5.push_back(i3);
							iTileVec5.push_back(i4);
							iTileVec5.push_back(i5);
							vTileDirVec_5.push_back(_GetTileCenterDir(iTileVec5));
						}
					}
				}
			}
		}
	}
	m_vTileDirVec.push_back(vTileDirVec_0);
	m_vTileDirVec.push_back(vTileDirVec_1);
	m_vTileDirVec.push_back(vTileDirVec_2);
	m_vTileDirVec.push_back(vTileDirVec_3);
	m_vTileDirVec.push_back(vTileDirVec_4);
	m_vTileDirVec.push_back(vTileDirVec_5);
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
		m_pHieRootVector.push_back(pRoot);
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

		osg::Vec3f vTileOffsetLonLat = osg::Vec3f(0.0f, 0.0f, 0.0f);
		osg::Vec3f vTileOffsetID = osg::Vec3f(0.0f, 0.0f, 0.0f);
		m_vTileOffsetLonLatUniform->get(vTileOffsetLonLat);
		m_vTileOffsetIDUniform->get(vTileOffsetID);

		// 先全部显示0.5级地形块
		for (int i = 0; i < m_pHie1_TileMap.at(0.5)->getNumChildren(); i++)
		{
			m_pHie1_TileMap.at(0.5)->getChild(i)->setNodeMask(~0);
		}

		const double fDotThreshold = 0.6;
		// 同时看到的地形块可以有如下情况：
		// 1个：1赤道、1极地
		// 2个：2赤道、1赤道+1极地、2极地
		// 3个：2赤道+1极地
		// 4个：4赤道、2赤道+2极地、4极地
		// 遍历所有地形块，根据相机位置和朝向，决定地形块的显示、隐藏、旋转
		for (auto& itr : m_sTileVec)
		{
			// 目前只有0号和4号面
			bool bPolar = (4 <= itr.iTileQuadVec.at(0));
			// iID是第1.0级瓦片的象限编号，这里只是初始化，后面会根据方向和位置调整
			int iID = itr.iTileQuadVec.at(1);
			bool bVisible = false;
			int iFace = bPolar ? 4 : 0;// 距离最近的面
			double fMaxDot = -1.0;
			// 计算相机朝向和0级瓦片中心点方向的点积
			if (bPolar)
			{
				for (int i = 4; i < 6; i++)
				{
					osg::Vec3d vTile0Dir = m_vTileDirVec.at(0).at(i);
					double fDot = vCore2EyeDir * vTile0Dir;
					if (fDot > fMaxDot)
					{
						fMaxDot = fDot;
						iFace = i;
					}
				}
				// 计算相机朝向和1级瓦片中心点方向的点积
				osg::Vec3d vTile1Dir = m_vTileDirVec.at(1).at(iFace * 4 + itr.iTileQuadVec.at(1));
				if ((vCore2EyeDir * vTile1Dir) > fDotThreshold) bVisible = true;
			}
			else
			{
				int iFace2 = 2;// 距离第二近的面
				for (int i = 0; i < 4; i++)
				{
					osg::Vec3d vTile0Dir = m_vTileDirVec.at(0).at(i);
					double fDotTmp = vCore2EyeDir * vTile0Dir;
					if (fDotTmp > fMaxDot)
					{
						fMaxDot = fDotTmp;
						iFace2 = iFace;
						iFace = i;
					}
				}
				// 计算相机朝向和1级瓦片中心点方向的点积
				// 赤道地区需要判断两个面
				osg::Vec3d vTile1Dir = m_vTileDirVec.at(1).at(iFace * 4 + itr.iTileQuadVec.at(1));
				osg::Vec3d vTile1Dir2 = m_vTileDirVec.at(1).at(iFace2 * 4 + itr.iTileQuadVec.at(1));
				double fDot = vCore2EyeDir * vTile1Dir;
				double fDot2 = vCore2EyeDir * vTile1Dir2;
				if (fDot < fDot2)
				{
					iFace = iFace2;
					fDot = fDot2;
				}
				if (fDot > fDotThreshold) bVisible = true;
			}

			// 显示或隐藏
			if (bVisible)
			{
				//旋转
				if (bPolar) // 如果是极地区域
				{
					if (4 == iFace) // 北极
					{
						itr.pTileTrans->asPositionAttitudeTransform()->setAttitude(osg::Quat(0, osg::Vec3d(0, 1, 0)));
						vTileOffsetLonLat.z() = -1.0f;// 北极-1
						vTileOffsetID.z() = 0;
					}
					else // 南极
					{
						itr.pTileTrans->asPositionAttitudeTransform()->setAttitude(osg::Quat(osg::PI, osg::Vec3d(0, 1, 0)));
						vTileOffsetLonLat.z() = 1.0f;// 南极1
						vTileOffsetID.z() = 4;
					}
					iID += 16 + vTileOffsetID.z();
				}
				else // 如果是赤道区域
				{
					// 是否是1或4象限, iID == 0或3
					bool b1or4 = (0 == iID || 3 == iID);

					if (0 == iFace) // posX
					{
						if (b1or4) vTileOffsetLonLat.x() = 0.0f;
						else vTileOffsetLonLat.y() = 0.0f;

						itr.pTileTrans->asPositionAttitudeTransform()->setAttitude(osg::Quat(0, osg::Vec3d(0, 0, 1)));
					}
					else if (1 == iFace) // negX
					{
						if (b1or4) vTileOffsetLonLat.x() = -0.5f;
						else vTileOffsetLonLat.y() = 0.5f;

						itr.pTileTrans->asPositionAttitudeTransform()->setAttitude(osg::Quat(osg::PI, osg::Vec3d(0, 0, 1)));
					}
					else if (2 == iFace) // posY
					{
						if (b1or4) vTileOffsetLonLat.x() = 0.25f;
						else vTileOffsetLonLat.y() = 0.25f;

						itr.pTileTrans->asPositionAttitudeTransform()->setAttitude(osg::Quat(osg::PI_2, osg::Vec3d(0, 0, 1)));
					}
					else // if (3 == iFace) negY
					{
						if (b1or4) vTileOffsetLonLat.x() = -0.25f;
						else vTileOffsetLonLat.y() = -0.25f;

						itr.pTileTrans->asPositionAttitudeTransform()->setAttitude(osg::Quat(osg::PI * 1.5, osg::Vec3d(0, 0, 1)));
					}

					if (b1or4)
					{
						vTileOffsetID.x() = iFace * 4;
						iID += vTileOffsetID.x();
					}
					else
					{
						vTileOffsetID.y() = iFace * 4;
						iID += vTileOffsetID.y();
					}
				}
				// 瓦片体贴图的偏移
				m_vTileOffsetLonLatUniform->set(vTileOffsetLonLat);
				m_vTileOffsetIDUniform->set(vTileOffsetID);

				// 显示1.0级地形块
				if (0 == itr.pTileTrans->getNodeMask())
					itr.pTileTrans->setNodeMask(~0);
				// 隐藏0.5级地形块
				osg::Node* pNode = m_pHie1_TileMap.at(0.5)->getChild(iID);
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
		if (0 == m_pHieRootVector.at(0)->getNodeMask())
		{
			m_pHieRootVector.at(0)->setNodeMask(~0);
			m_pHieRootVector.at(1)->setNodeMask(~0);
		}
	}
	else
	{
		if (0 != m_pHieRootVector.at(0)->getNodeMask())
		{
			m_pHieRootVector.at(0)->setNodeMask(0);
			m_pHieRootVector.at(1)->setNodeMask(0);
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
		if (!(GM_Root->containsNode(m_pHieRootVector.at(0))))
		{
			GM_Root->addChild(m_pHieRootVector.at(0));
		}
		if (GM_Root->containsNode(m_pHieRootVector.at(1)))
		{
			GM_Root->removeChild(m_pHieRootVector.at(1));
		}
	}
	break;
	case 1:
	{
		if (GM_Root->containsNode(m_pHieRootVector.at(0)))
		{
			GM_Root->removeChild(m_pHieRootVector.at(0));
		}
		if (!(GM_Root->containsNode(m_pHieRootVector.at(1))))
		{
			GM_Root->addChild(m_pHieRootVector.at(1));
		}
	}
	break;
	case 2:
	{
		if (GM_Root->containsNode(m_pHieRootVector.at(1)))
		{
			GM_Root->removeChild(m_pHieRootVector.at(1));
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
	m_pHieRootVector.at(1)->addChild(m_pHie1_TileMap.at(0.5));//0.5级地形块
	m_pHieRootVector.at(1)->addChild(m_pHie1_TileMap.at(1.0));//1.0级地形块
	m_pHieRootVector.at(1)->addChild(m_pHie1_TileMap.at(1.5));//1.5级地形块
	m_sTileVec.reserve(8);

	for (int iFace = 0; iFace < 6; iFace++)
	{
		bool bPolar = (4 <= iFace);
		for (int iQuad1 = 0; iQuad1 < 4; iQuad1++)
		{
			// 创建0.5级地形块
			std::vector<int> iTileVec;
			iTileVec.push_back(bPolar ? 4 : 0); // iFace to do
			iTileVec.push_back(0); // iQuad1 to do
			osg::ref_ptr<osg::Geometry>	pTerrainQuaterGeom_0h = _MakeTileGeometry(iTileVec, 31);
			osg::ref_ptr<osg::Geode> pTerrainQuaterGeode_0h = new osg::Geode();
			pTerrainQuaterGeode_0h->addDrawable(pTerrainQuaterGeom_0h.get());
			m_pHie1_TileMap.at(0.5)->addChild(pTerrainQuaterGeode_0h.get());

			double fUnit = m_pKernelData->fUnitArray->at(1);
			// 赤道地形块的ID为0-15，北极地形块的ID为16-19，南极地形块的ID为20-23
			m_pCelestialScaleVisitor->SetTileLevel(0.5f);
			m_pCelestialScaleVisitor->SetQuatorFace(iFace * 4 + iQuad1);
			m_pCelestialScaleVisitor->SetRadius(osg::WGS_84_RADIUS_EQUATOR / fUnit, osg::WGS_84_RADIUS_POLAR / fUnit);
			pTerrainQuaterGeom_0h->accept(*m_pCelestialScaleVisitor);	// 改变大小

			// 0: 赤道，4: 极地，每个地形块分为4个四分之一地形块，总共需要4个赤道和4个极地
			// 同时看到的地形块最多4个，可以有三种组合：4赤道，2赤道+2极地，4极地
			// 所以只需要创建4个赤道和4个极地地形块，然后根据需要旋转和显隐
			if (0 != iFace && 4 != iFace) continue;

			// 创建1.0级地形块
			osg::ref_ptr<osg::Geometry>	pTerrainQuaterGeom_1 = _MakeTileGeometry(iTileVec, 63);
			osg::ref_ptr<osg::PositionAttitudeTransform> pTerrainQuaterTrans_1 = new osg::PositionAttitudeTransform();
			osg::ref_ptr<osg::Geode> pTerrainQuaterGeode_1 = new osg::Geode();
			pTerrainQuaterGeode_1->addDrawable(pTerrainQuaterGeom_1.get());
			pTerrainQuaterTrans_1->addChild(pTerrainQuaterGeode_1.get());
			m_pHie1_TileMap.at(1.0)->addChild(pTerrainQuaterTrans_1.get());

			// 赤道地形块的ID为0-15，北极地形块的ID为16-19，南极地形块的ID为20-23
			m_pCelestialScaleVisitor->SetTileLevel(1.0f);
			pTerrainQuaterGeom_1->accept(*m_pCelestialScaleVisitor);	// 改变大小

			STileData sTile;
			sTile.pTileTrans = pTerrainQuaterTrans_1;
			sTile.iTileQuadVec.push_back(iFace);	// 0级瓦片的6个面，0-5
			sTile.iTileQuadVec.push_back(iQuad1);	// 1级瓦片的4个象限，0-3
			m_sTileVec.push_back(sTile);
		}
	}

	return true;
}

osg::Geometry* CGMTerrain::_MakeTileGeometry(const std::vector<int>& iTileVec, int iSegment) const
{
	// 为了效率，限制iSegment的上限，以防element超过65536，特意设置成2^n-1是为了保证高程图的分辨率是2^n
	iSegment = osg::clampBetween(iSegment, 3, 255);
	double fSize = double(iSegment);
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

	osg::Vec3d vAxisX = m_vFaceXYZVec.at(iTileVec.at(0)).vX;
	osg::Vec3d vAxisY = m_vFaceXYZVec.at(iTileVec.at(0)).vY;
	bool bPolar = (4 <= iTileVec.at(0));
	// 瓦片中心点在ECEF坐标系中的方向投影到边长为2的正方体上的坐标
	osg::Vec3 vCenterInBox = _GetTileCenterInBox(iTileVec);
	double fLenInBox = 2.0 * exp2(1.0 - iTileVec.size());
	int iID = 0;
	switch (iTileVec.size())
	{
	case 1:
		iID = iTileVec.at(0);
		break;
	case 2:
		iID = iTileVec.at(0) * 4 + iTileVec.at(1);
		break;
	case 3:
		iID = iTileVec.at(0) * 4 * 4 + iTileVec.at(1) * 4 + iTileVec.at(2);
		break;
	default:
		break;
	}

	for (int y = 0; y <= iSegment; ++y)
	{
		for (int x = 0; x <= iSegment; ++x)
		{
			osg::Vec3d vDir = vCenterInBox + (vAxisX * (x / fSize - 0.5) + vAxisY * (y / fSize - 0.5)) * fLenInBox;
			vDir.normalize();

			// 默认的经纬度为临界值，防止三角函数失效
			double fLon = bPolar ? osg::PI_2 : 0;// 弧度
			double fLat = bPolar ? ((4 == iTileVec.at(0)) ? osg::PI_2 : -osg::PI_2) : 0.0;// 弧度
			if (0 != vDir.x() || 0 != vDir.y())
			{
				fLon = atan2(vDir.y(), vDir.x());// 弧度 (-PI, PI]
				fLat = asin(vDir.z());// 弧度 [-PI/2, PI/2]
			}

			verts->push_back(vDir);
			// 0层纹理单元 xy = WGS84对应的UV，[0.0, 1.0]
			// 1层纹理单元 xy = 瓦片体贴图UV，[0.0, 1.0];
			// z = 面对应的编号, 1层瓦片[0,23]，2层瓦片[0,95]
			coords0->push_back(osg::Vec2(0.5 + fLon / (osg::PI * 2), 0.5 + fLat / (osg::PI)));
			coords1->push_back(osg::Vec3(double(x) / double(iSegment), double(y) / double(iSegment), iID));
			normals->push_back(vDir);
			if (x < iSegment && y < iSegment)
			{
				if ((x + y) % 2)
				{
					el->push_back(_GetVertIndex(x, y, iSegment));
					el->push_back(_GetVertIndex(x + 1, y, iSegment));
					el->push_back(_GetVertIndex(x, y + 1, iSegment));
					el->push_back(_GetVertIndex(x + 1, y, iSegment));
					el->push_back(_GetVertIndex(x + 1, y + 1, iSegment));
					el->push_back(_GetVertIndex(x, y + 1, iSegment));
				}
				else
				{
					el->push_back(_GetVertIndex(x, y, iSegment));
					el->push_back(_GetVertIndex(x + 1, y, iSegment));
					el->push_back(_GetVertIndex(x + 1, y + 1, iSegment));
					el->push_back(_GetVertIndex(x, y, iSegment));
					el->push_back(_GetVertIndex(x + 1, y + 1, iSegment));
					el->push_back(_GetVertIndex(x, y + 1, iSegment));
				}
			}
		}
	}
	return geom;
}

osg::Vec3d CGMTerrain::_GetTileCenterDir(const std::vector<int>& iTileVec) const
{
	osg::Vec3d vECEFDir = _GetTileCenterInBox(iTileVec);
	vECEFDir.normalize();
	return vECEFDir;
}

osg::Vec3d CGMTerrain::_GetTileCenterInBox(const std::vector<int>& iTileVec) const
{
	if (iTileVec.empty()) return osg::Vec3d(0, 0, 0);
	// 0级瓦片的信息
	if (1 == iTileVec.size()) return m_vFaceXYZVec.at(iTileVec.at(0)).vZ;

	osg::Vec3d vAxisX = m_vFaceXYZVec.at(iTileVec.at(0)).vX;
	osg::Vec3d vAxisY = m_vFaceXYZVec.at(iTileVec.at(0)).vY;
	osg::Vec3d vECEFPosInBox = m_vFaceXYZVec.at(iTileVec.at(0)).vZ;
	for (int iTileLevel = 1; iTileLevel < iTileVec.size(); iTileLevel++)
	{
		double fXYScale = exp2(-iTileLevel);
		switch (iTileVec.at(iTileLevel))
		{
		case 0:
			vECEFPosInBox += (vAxisX + vAxisY) * fXYScale;
			break;
		case 1:
			vECEFPosInBox -= (vAxisX - vAxisY) * fXYScale;
			break;
		case 2:
			vECEFPosInBox -= (vAxisX + vAxisY) * fXYScale;
			break;
		case 3:
			vECEFPosInBox += (vAxisX - vAxisY) * fXYScale;
			break;
		default:
			return osg::Vec3d(0, 0, 0);
		}
	}
	return vECEFPosInBox;
}
