//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMPlanet.cpp
/// @brief		Galaxy-Music Engine - GMPlanet
/// @version	1.0
/// @author		LiuTao
/// @date		2023.09.23
//////////////////////////////////////////////////////////////////////////

#include "GMPlanet.h"
#include "GMEngine.h"

#include "GMKit.h"
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <ppl.h>
using namespace concurrency;

using namespace GM;
/*************************************************************************
Macro Defines
*************************************************************************/

/*************************************************************************
CGMPlanet Methods
*************************************************************************/

/** @brief 构造 */
CGMPlanet::CGMPlanet(): CGMTerrain()
{
}

/** @brief 析构 */
CGMPlanet::~CGMPlanet()
{
}

/** @brief 初始化 */
bool CGMPlanet::Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform)
{
	CGMTerrain::Init(pKernelData, pConfigData, pCommonUniform);

	return true;
}

bool CGMPlanet::Update(double dDeltaTime)
{
	return CGMTerrain::Update(dDeltaTime);
}

bool CGMPlanet::UpdateLater(double dDeltaTime)
{
	return CGMTerrain::UpdateLater(dDeltaTime);
}

void CGMPlanet::SetVisible(const bool bVisible)
{
	CGMTerrain::SetVisible(bVisible);
}

bool CGMPlanet::CreatePlanet()
{
	CreateTerrain();

	return false;
}

bool CGMPlanet::UpdateHierarchy(int iHieNew)
{
	CGMTerrain::UpdateHierarchy(iHieNew);
	return true;
}

osg::Geometry* CGMPlanet::MakeHexahedronSphereGeometry(int iSegment)
{
	// 为了效率，限制iSegment的上限，以防element超过65536
	iSegment = osg::clampBetween(iSegment, 2, 32);
	float fHalfSize = iSegment * 0.5f;
	int iVertPerEdge = iSegment + 1;
	int iVertPerFace = iVertPerEdge * iVertPerEdge;
	// 国际日期变更线上需要两份UV，所以需要两份顶点
	int iVertCalenderLine = (iSegment/2 + 1) * 2 + (iSegment + 1);
	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);

	osg::Vec3Array* verts = new osg::Vec3Array();
	osg::Vec2Array* coords0 = new osg::Vec2Array();
	osg::Vec3Array* coords1 = new osg::Vec3Array();
	osg::Vec3Array* normals = new osg::Vec3Array();
	osg::DrawElementsUShort* el = new osg::DrawElementsUShort(GL_TRIANGLES);

	verts->reserve(iVertPerFace * 6 + iVertCalenderLine);
	coords0->reserve(iVertPerFace * 6 + iVertCalenderLine);
	coords1->reserve(iVertPerFace * 6 + iVertCalenderLine);
	normals->reserve(iVertPerFace * 6 + iVertCalenderLine);
	el->reserve(iSegment * iSegment * 6 * 6);

	geom->setTexCoordArray(0, coords0);
	geom->setTexCoordArray(1, coords1);
	geom->setNormalArray(normals);
	geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	geom->setVertexArray(verts);
	geom->addPrimitiveSet(el);

	osg::Vec3 vCenter = osg::Vec3(0, 1, 0);
	osg::Vec3 vAxisX = osg::Vec3(1, 0, 0);
	osg::Vec3 vAxisY = osg::Vec3(0, 0, 1);
	for (int i = 0; i < 6; i++)
	{
		switch (i)
		{
		case 0:
		{
			// posX
			vCenter = osg::Vec3(1, 0, 0);
			vAxisX = osg::Vec3(0, 1, 0);
			vAxisY = osg::Vec3(0, 0, 1);
		}
		break;
		case 1:
		{
			// negX
			vCenter = osg::Vec3(-1, 0, 0);
			vAxisX = osg::Vec3(0, -1, 0);
			vAxisY = osg::Vec3(0, 0, 1);
		}
		break;
		case 2:
		{
			// posY
			vCenter = osg::Vec3(0, 1, 0);
			vAxisX = osg::Vec3(-1, 0, 0);
			vAxisY = osg::Vec3(0, 0, 1);
		}
		break;
		case 3:
		{
			// negY
			vCenter = osg::Vec3(0, -1, 0);
			vAxisX = osg::Vec3(1, 0, 0);
			vAxisY = osg::Vec3(0, 0, 1);
		}
		break;
		case 4:
		{
			// posZ
			vCenter = osg::Vec3(0, 0, 1);
			vAxisX = osg::Vec3(0, 1, 0);
			vAxisY = osg::Vec3(-1, 0, 0);
		}
		break;
		case 5:
		{
			// negZ
			vCenter = osg::Vec3(0, 0, -1);
			vAxisX = osg::Vec3(0, 1, 0);
			vAxisY = osg::Vec3(1, 0, 0);
		}
		break;
		default:
			break;
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
				float fLat = asin(vDir.z());// 弧度

				verts->push_back(vDir);
				// 0层纹理单元 xy = WGS84对应的UV，[0.0, 1.0]
				// 1层纹理单元 xy = 六面体贴图UV，[0.0, 1.0]; z = 面对应的编号012345
				coords0->push_back(osg::Vec2(0.5f + fLon / (osg::PI * 2), 0.5f + fLat / (osg::PI)));
				coords1->push_back(osg::Vec3(float(x) / float(iSegment), float(y) / float(iSegment), i));
				normals->push_back(vDir);
				if (x < iSegment && y < iSegment)
				{
					osg::Vec3 vFragOut = vCenter
						+ vAxisX * (x + 0.5 - fHalfSize) / fHalfSize
						+ vAxisY * (y + 0.5 - fHalfSize) / fHalfSize;
					bool bEast = vFragOut.y() > 0;

					el->push_back(_GetVertIndex(i, x, y, iSegment, bEast));
					el->push_back(_GetVertIndex(i, x + 1, y, iSegment, bEast));
					el->push_back(_GetVertIndex(i, x, y + 1, iSegment, bEast));
					el->push_back(_GetVertIndex(i, x + 1, y, iSegment, bEast));
					el->push_back(_GetVertIndex(i, x + 1, y + 1, iSegment, bEast));
					el->push_back(_GetVertIndex(i, x, y + 1, iSegment, bEast));
				}
			}
		}
	}

	// 国际日期变更线上的顶点，由于处在三个面上，需要分成三段分别考虑
	// 经度为-180°，按照纬度升序排列顶点，第一段面朝下(negZ)，第三段面朝上(posZ)
	
	// negZ
	for (int k = 0; k <= iSegment / 2; k++)
	{
		vCenter = osg::Vec3(0, 0, -1);
		vAxisX = osg::Vec3(0, 1, 0);
		vAxisY = osg::Vec3(1, 0, 0);

		osg::Vec3 vDir = vCenter - vAxisY * k / fHalfSize;
		vDir.normalize();
		float fLat = asin(vDir.z());// 弧度

		verts->push_back(vDir);
		// 0层纹理单元 xy = WGS84对应的UV，[0.0, 1.0]
		// 1层纹理单元 xy = 六面体贴图UV，[0.0, 1.0]; z = 面对应的编号5
		coords0->push_back(osg::Vec2(0.0f, 0.5f + fLat / (osg::PI)));
		coords1->push_back(osg::Vec3(0.5f, 0.5f * (1 - k / float(iSegment / 2)), 5));
		normals->push_back(vDir);
	}
	// negX
	for (int k = 0; k <= iSegment; k++)
	{
		vCenter = osg::Vec3(-1, 0, 0);
		vAxisX = osg::Vec3(0, -1, 0);
		vAxisY = osg::Vec3(0, 0, 1);

		osg::Vec3 vDir = vCenter + vAxisY * (k - fHalfSize) / fHalfSize;
		vDir.normalize();
		float fLat = asin(vDir.z());// 弧度

		verts->push_back(vDir);
		// 0层纹理单元 xy = WGS84对应的UV，[0.0, 1.0]
		// 1层纹理单元 xy = 六面体贴图UV，[0.0, 1.0]; z = 面对应的编号1
		coords0->push_back(osg::Vec2(0.0f, 0.5f + fLat / (osg::PI)));
		coords1->push_back(osg::Vec3(0.5f, k / float(iSegment), 1));
		normals->push_back(vDir);
	}
	// posZ
	for (int k = 0; k <= iSegment / 2; k++)
	{
		vCenter = osg::Vec3(0, 0, 1);
		vAxisX = osg::Vec3(0, 1, 0);
		vAxisY = osg::Vec3(-1, 0, 0);

		osg::Vec3 vDir = vCenter + vAxisY * (fHalfSize - k) / fHalfSize;
		vDir.normalize();
		float fLat = asin(vDir.z());// 弧度

		verts->push_back(vDir);
		// 0层纹理单元 xy = WGS84对应的UV，[0.0, 1.0]
		// 1层纹理单元 xy = 六面体贴图UV，[0.0, 1.0]; z = 面对应的编号4
		coords0->push_back(osg::Vec2(0.0f, 0.5f + fLat / (osg::PI)));
		coords1->push_back(osg::Vec3(0.5f, 1.0f - 0.5f * k / float(iSegment / 2), 4));
		normals->push_back(vDir);
	}

	return geom;
}

osg::Geometry* CGMPlanet::MakeEllipsoidGeometry(
	const osg::EllipsoidModel* ellipsoid,
	int iLonSegments, int iLatSegments,
	float fHae, bool bGenTexCoords,
	bool bWholeMap, bool bFlipNormal,
	float fLatStart, float fLatEnd) const
{
	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);

	if (fLatEnd < fLatStart)
	{
		float tmp = fLatEnd;
		fLatEnd = fLatStart;
		fLatStart = tmp;
	}

	fLatStart = (fLatStart < -90.0) ? -90.0 : fLatStart;
	fLatEnd = (fLatEnd > 90.0) ? 90.0 : fLatEnd;

	double latSegmentSize = (fLatEnd - fLatStart) / (double)iLatSegments; // degrees
	double lonSegmentSize = 360.0 / (double)iLonSegments; // degrees

	osg::Vec3Array* verts = new osg::Vec3Array();
	verts->reserve(iLatSegments * iLonSegments);

	osg::Vec2Array* texCoords = 0;
	osg::Vec3Array* normals = 0;
	if (bGenTexCoords)
	{
		texCoords = new osg::Vec2Array();
		texCoords->reserve(iLatSegments * iLonSegments);
		geom->setTexCoordArray(0, texCoords);

		normals = new osg::Vec3Array();
		normals->reserve(iLatSegments * iLonSegments);
		geom->setNormalArray(normals);
		geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	}

	osg::DrawElementsUShort* el = new osg::DrawElementsUShort(GL_TRIANGLES);
	el->reserve(iLatSegments * iLonSegments * 6);

	for (int y = 0; y <= iLatSegments; ++y)
	{
		double lat = fLatStart + latSegmentSize * (double)y;
		for (int x = 0; x <= iLonSegments; ++x)
		{
			double lon = lonSegmentSize * (double)x - 180.0;
			double gx, gy, gz;
			ellipsoid->convertLatLongHeightToXYZ(osg::DegreesToRadians(lat), osg::DegreesToRadians(lon), fHae, gx, gy, gz);
			verts->push_back(osg::Vec3(gx, gy, gz));

			if (bGenTexCoords)
			{
				double s, t;
				if (bWholeMap)
				{
					s = 1 - (double)x / (double)iLonSegments;
					t = (double)y / (double)iLatSegments;
				}
				else
				{
					s = (lon + 180) / 360.0;
					t = (lat + 90.0) / 180.0;
				}
				texCoords->push_back(osg::Vec2(s, t));
			}

			if (normals)
			{
				osg::Vec3 normal(gx, gy, gz);
				if (bFlipNormal)
				{
					normal *= -1;
				}
				normal.normalize();
				normals->push_back(normal);
			}

			if ((y < iLatSegments) && (x < iLonSegments))
			{
				//int x_plus_1 = x < iLonSegments - 1 ? x + 1 : 0;
				int x_plus_1 = x + 1;
				int y_plus_1 = y + 1;
				if (bFlipNormal)
				{
					el->push_back(y*(iLonSegments + 1) + x);
					el->push_back(y_plus_1*(iLonSegments + 1) + x);
					el->push_back(y*(iLonSegments + 1) + x_plus_1);
					el->push_back(y*(iLonSegments + 1) + x_plus_1);
					el->push_back(y_plus_1*(iLonSegments + 1) + x);
					el->push_back(y_plus_1*(iLonSegments + 1) + x_plus_1);
				}
				else
				{
					el->push_back(y*(iLonSegments + 1) + x);
					el->push_back(y*(iLonSegments + 1) + x_plus_1);
					el->push_back(y_plus_1*(iLonSegments + 1) + x);
					el->push_back(y*(iLonSegments + 1) + x_plus_1);
					el->push_back(y_plus_1*(iLonSegments + 1) + x_plus_1);
					el->push_back(y_plus_1*(iLonSegments + 1) + x);
				}
			}
		}
	}

	geom->setVertexArray(verts);
	geom->addPrimitiveSet(el);

	return geom;
}

void CGMPlanet::Panorama2CubeMap()
{
	// 全景图转Cubemap
	std::string strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/DEM_bed.tif";
	std::string strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/DEM/Tile0/Earth_DEM_";
	//_Panorama_2_CubeDEM(strPanoPath, strCubemapPath, 1024);
	//strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/DEM/Tile1/Earth_DEM_";
	//_Panorama_2_CubeDEM(strPanoPath, strCubemapPath, 1024, 1);
	//strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/DEM/Tile2/Earth_DEM_";
	//_Panorama_2_CubeDEM(strPanoPath, strCubemapPath, 1024, 2);
	
	//strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/Earth_base.tif";
	//strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/SourceTif/DOM/Tile0/Earth_base_";
	//_Panorama_2_CubeMap(strPanoPath, strCubemapPath, 1024);
	//strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/SourceTif/DOM/Tile1/Earth_base_";
	//_Panorama_2_CubeMap(strPanoPath, strCubemapPath, 1024, 1);

	//strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/wanderingEarth_base_real.tif";
	//strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/SourceTif/DOM/Tile0/wanderingEarth_base_real_";
	//_Panorama_2_CubeMap(strPanoPath, strCubemapPath, 1024);
	//strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/SourceTif/DOM/Tile1/wanderingEarth_base_real_";
	//_Panorama_2_CubeMap(strPanoPath, strCubemapPath, 1024, 1);

	//strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/Earth_illum_real.tif";
	//strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/SourceTif/DOM/Tile0/Earth_illum_real_";
	//_Panorama_2_CubeMap(strPanoPath, strCubemapPath, 512);
	//strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/SourceTif/DOM/Tile1/Earth_illum_real_";
	//_Panorama_2_CubeMap(strPanoPath, strCubemapPath, 512, 1);

	//strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/Earth_cloud_real.tif";
	//strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/SourceTif/DOM/Tile0/Earth_cloud_real_";
	//_Panorama_2_CubeMap(strPanoPath, strCubemapPath, 512);

	//strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/WanderingEarth/wanderingEarth_cloud.tif";
	//strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/SourceTif/DOM/Tile0/wanderingEarth_cloud_real_";
	//_Panorama_2_CubeMap(strPanoPath, strCubemapPath, 512);

	/*strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Jupiter/Jupiter_cloud.tif";
	strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Jupiter/Jupiter_cloud_";
	_Panorama_2_CubeMap(strPanoPath, strCubemapPath);

	strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Mars/Mars_base.tif";
	strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Mars/Mars_base_";
	_Panorama_2_CubeMap(strPanoPath, strCubemapPath);

	strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Mercury/Mercury_base.tif";
	strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Mercury/Mercury_base_";
	_Panorama_2_CubeMap(strPanoPath, strCubemapPath);

	strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Moon/Moon_base.tif";
	strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Moon/Moon_base_";
	_Panorama_2_CubeMap(strPanoPath, strCubemapPath);

	strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Neptune/Neptune_cloud.tif";
	strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Neptune/Neptune_cloud_";
	_Panorama_2_CubeMap(strPanoPath, strCubemapPath);

	strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Pluto/Charon_base.tif";
	strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Pluto/Charon_base_";
	_Panorama_2_CubeMap(strPanoPath, strCubemapPath);

	strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Pluto/Pluto_base.tif";
	strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Pluto/Pluto_base_";
	_Panorama_2_CubeMap(strPanoPath, strCubemapPath);

	strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Saturn/Saturn_cloud.tif";
	strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Saturn/Saturn_cloud_";
	_Panorama_2_CubeMap(strPanoPath, strCubemapPath);

	strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Saturn/Titan_cloud.tif";
	strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Saturn/Titan_cloud_";
	_Panorama_2_CubeMap(strPanoPath, strCubemapPath);

	strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Uranus/Uranus_cloud.tif";
	strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Uranus/Uranus_cloud_";
	_Panorama_2_CubeMap(strPanoPath, strCubemapPath);

	strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Uranus/Uranus_cloud.tif";
	strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Uranus/Uranus_cloud_";
	_Panorama_2_CubeMap(strPanoPath, strCubemapPath);

	strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Venus/Venus_base.tif";
	strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Venus/Venus_base_";
	_Panorama_2_CubeMap(strPanoPath, strCubemapPath);

	strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Venus/Venus_cloud.tif";
	strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Venus/Venus_cloud_";
	_Panorama_2_CubeMap(strPanoPath, strCubemapPath);*/
}

bool CGMPlanet::_Panorama_2_CubeDEM(
	const std::string& strPanoPath,
	const std::string& strCubeDEMPath,
	const int iSize,
	const int iTileLevel)
{
	osg::ref_ptr<osg::Image> pBedImg = osgDB::readImageFile(strPanoPath);
	if (!pBedImg.valid()) return false;

	for (int t2 = 0; t2 < (iTileLevel >= 2 ? 4 : 1); t2++)
	{
		for (int t1 = 0; t1 < (iTileLevel >= 1 ? 4 : 1); t1++)
		{
			parallel_for(int(0), int(6), [&](int i) // 多线程
			//for (int i = 0; i < 6; i++)
			{
				osg::Vec3d vCenter = osg::Vec3d(0, 1, 0);
				osg::Vec3d vAxisX = osg::Vec3d(1, 0, 0);
				osg::Vec3d vAxisY = osg::Vec3d(0, 0, 1);
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

				GLushort* pCubeDEMData = new GLushort[iSize * iSize];
				for (int x = 0; x < iSize; x++)
				{
					for (int y = 0; y < iSize; y++)
					{
						// 要注意，每张图的周围一圈像素都是保护边界，与相邻的图片边界相同，这样可以避免边缘接缝问题
						// 每个像素的方向
						osg::Vec3d vDir = vCenter;
						double fSize = double(iSize - 1);
						switch (iTileLevel)
						{
						case 0: // 0级瓦片，六面体
						{
							double fHalfSize = fSize * 0.5;
							vDir += (vAxisX * (x - fHalfSize) + vAxisY * (y - fHalfSize)) / fHalfSize;
						}
						break;
						case 1: // 1级瓦片，六面体的每个面分成4个瓦片
						{		
							if (0 == t1)
								vDir += (vAxisX * x + vAxisY * y) / fSize;
							else if (1 == t1)
								vDir -= (vAxisX * (fSize - x) - vAxisY * y) / fSize;
							else if (2 == t1)
								vDir -= (vAxisX * (fSize - x) + vAxisY * (fSize - y)) / fSize;
							else if (3 == t1)
								vDir += (vAxisX * x - vAxisY * (fSize - y)) / fSize;
							else {}
						}
						break;
						case 2: // 2级瓦片，六面体的每个面分成16个瓦片
						{
							if (0 == t1)
								vDir += (vAxisX + vAxisY) * 0.5;
							else if (1 == t1)
								vDir -= (vAxisX - vAxisY) * 0.5;
							else if (2 == t1)
								vDir -= (vAxisX + vAxisY) * 0.5;
							else if (3 == t1)
								vDir += (vAxisX - vAxisY) * 0.5;
							else {}

							if (0 == t2)
								vDir += (vAxisX * x + vAxisY * y) * 0.5 / fSize;
							else if (1 == t2)
								vDir -= (vAxisX * (fSize - x) - vAxisY * y) * 0.5 / fSize;
							else if (2 == t2)
								vDir -= (vAxisX * (fSize - x) + vAxisY * (fSize - y)) * 0.5 / fSize;
							else if (3 == t2)
								vDir += (vAxisX * x - vAxisY * (fSize - y)) * 0.5 / fSize;
							else {}
						}
						break;
						default:
							break;
						}
						vDir.normalize();

						double fLon = atan2(vDir.y(), vDir.x());// 弧度
						double fLat = asin(vDir.z());// 弧度
						// tif的左下角坐标：(0.0, 0.0)
						double fX = 0.5f + fLon / (osg::PI * 2);
						double fY = 0.5f + fLat / (osg::PI);
						double fElevBed = CGMKit::GetImageColor(pBedImg, fX, fY, true).r();
						// float to short
						GLushort sElevBed = min(1.0f, sqrt(abs(fElevBed * 1e-4))) * 32767;
						if (fElevBed > 0)
							sElevBed = 32768 + sElevBed;
						else
							sElevBed = 32768 - sElevBed;

						int iAddress = x + y * iSize;
						pCubeDEMData[iAddress] = sElevBed;
					}
				}

				osg::ref_ptr<osg::Image> pCubeImage = new osg::Image();
				pCubeImage->setImage(iSize, iSize, 1, GL_R16F, GL_RED, GL_UNSIGNED_SHORT, (unsigned char*)pCubeDEMData, osg::Image::USE_NEW_DELETE);
				switch (iTileLevel)
				{
				case 0: // 0级瓦片，六面体
					osgDB::writeImageFile(*(pCubeImage.get()),
						strCubeDEMPath + std::to_string(i) + ".tif");
					break;
				case 1: // 1级瓦片，六面体的每个面分成4个瓦片
					osgDB::writeImageFile(*(pCubeImage.get()),
						strCubeDEMPath + std::to_string(i) + "_" + std::to_string(t1) + ".tif");
					break;
				case 2: // 2级瓦片，六面体的每个面分成16个瓦片
					osgDB::writeImageFile(*(pCubeImage.get()),
						strCubeDEMPath + std::to_string(i) + "_" + std::to_string(t1) + "_" + std::to_string(t2) + ".tif");
					break;
				default:
					break;
				}
			}
			); // end parallel_for
		}
	}
	return true;
}

bool CGMPlanet::_Panorama_2_CubeMap(
	const std::string& strPanoramaPath,
	const std::string& strCubeMapPath,
	const int iSize,
	const int iTileLevel)
{
	osg::ref_ptr<osg::Image> pPanoImg = osgDB::readImageFile(strPanoramaPath);
	if(!pPanoImg.valid()) return false;

	for (int t1 = 0; t1 < (iTileLevel >= 1 ? 4 : 1); t1++)
	{
		parallel_for(int(0), int(6), [&](int i) // 多线程
		//for (int i = 0; i < 6; i++)
		{
			osg::Vec3 vCenter = osg::Vec3(0, 1, 0);
			osg::Vec3 vAxisX = osg::Vec3(1, 0, 0);
			osg::Vec3 vAxisY = osg::Vec3(0, 0, 1);
			switch (i)
			{
			case 0:
			{
				// posX
				vCenter = osg::Vec3(1, 0, 0);
				vAxisX = osg::Vec3(0, 1, 0);
				vAxisY = osg::Vec3(0, 0, 1);
			}
			break;
			case 1:
			{
				// negX
				vCenter = osg::Vec3(-1, 0, 0);
				vAxisX = osg::Vec3(0, -1, 0);
				vAxisY = osg::Vec3(0, 0, 1);
			}
			break;
			case 2:
			{
				// posY
				vCenter = osg::Vec3(0, 1, 0);
				vAxisX = osg::Vec3(-1, 0, 0);
				vAxisY = osg::Vec3(0, 0, 1);
			}
			break;
			case 3:
			{
				// negY
				vCenter = osg::Vec3(0, -1, 0);
				vAxisX = osg::Vec3(1, 0, 0);
				vAxisY = osg::Vec3(0, 0, 1);
			}
			break;
			case 4:
			{
				// posZ
				vCenter = osg::Vec3(0, 0, 1);
				vAxisX = osg::Vec3(0, 1, 0);
				vAxisY = osg::Vec3(-1, 0, 0);
			}
			break;
			case 5:
			{
				// negZ
				vCenter = osg::Vec3(0, 0, -1);
				vAxisX = osg::Vec3(0, 1, 0);
				vAxisY = osg::Vec3(1, 0, 0);
			}
			break;
			default:
				break;
			}

			unsigned char* pCubeColorData = new unsigned char[iSize * iSize * 4];
			for (int x = 0; x < iSize; x++)
			{
				for (int y = 0; y < iSize; y++)
				{
					// 要注意，每张图的周围一圈像素都是保护边界，与相邻的图片边界互补，这样可以避免边缘接缝问题
					// 每个像素的方向	
					osg::Vec3 vDir = vCenter;
					switch (iTileLevel)
					{
					case 0: // 六面体
					{
						float fHalfSize = iSize * 0.5f;
						vDir += (vAxisX * (x - fHalfSize + 0.5f) + vAxisY * (y - fHalfSize + 0.5f)) / (fHalfSize - 1);
					}
					break;
					case 1: // 第1级瓦片，六面体的每个面分成4个瓦片
					{
						float fSize1 = float(iSize - 2);
						if (0 == t1)
							vDir += (vAxisX * (x - 0.5f) + vAxisY * (y - 0.5f)) / fSize1;
						else if (1 == t1)
							vDir -= (vAxisX * (iSize - 0.5f - x) - vAxisY * (y - 0.5f)) / fSize1;
						else if (2 == t1)
							vDir -= (vAxisX * (iSize - 0.5f - x) + vAxisY * (iSize - 0.5f - y)) / fSize1;
						else if (3 == t1)
							vDir += (vAxisX * (x - 0.5f) - vAxisY * (iSize - 0.5f - y)) / fSize1;
						else {}
					}
					break;
					default:
						break;
					}
					vDir.normalize();

					float fLon = atan2(vDir.y(), vDir.x());// 弧度
					float fLat = asin(vDir.z());// 弧度
					// tif的左下角坐标：(0.0, 0.0)
					osg::Vec4f vColor = CGMKit::GetImageColor(pPanoImg.get(),
						0.5f + fLon / (osg::PI * 2),
						0.5f + fLat / (osg::PI),
						true);

					int iAddress = 4 * (x + y * iSize);
					pCubeColorData[iAddress] = (unsigned char)(vColor.r() * 255);
					pCubeColorData[iAddress + 1] = (unsigned char)(vColor.g() * 255);
					pCubeColorData[iAddress + 2] = (unsigned char)(vColor.b() * 255);
					pCubeColorData[iAddress + 3] = (unsigned char)(vColor.a() * 255);
				}
			}

			osg::ref_ptr<osg::Image> pCubeImage = new osg::Image();
			pCubeImage->setImage(iSize, iSize, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, pCubeColorData, osg::Image::USE_NEW_DELETE);
			switch (iTileLevel)
			{
			case 0: // 六面体
				osgDB::writeImageFile(*(pCubeImage.get()), strCubeMapPath + std::to_string(i) + ".tif");
				break;
			case 1: // 第1级瓦片，六面体的每个面分成4个瓦片
				osgDB::writeImageFile(*(pCubeImage.get()), strCubeMapPath + std::to_string(i) + "_" + std::to_string(t1) + ".tif");
				break;
			default:
				break;
			}
		}
		); // end parallel_for
	}
	return true;
}

int CGMPlanet::_GetVertIndex(const int iFace, const int iX, const int iY, const int iSegment, const bool bEast)
{
	int iHalfSeg = iSegment / 2;
	int iVertPerEdge = iSegment + 1;
	int iVertPerFace = iVertPerEdge * iVertPerEdge;
	int iIndex = iVertPerFace * iFace + iY * iVertPerEdge + iX;

	// 如果在东半球，就不需要考虑“纬度突变”的情况
	if (bEast) return iIndex;

	// 判断是否在国际日期变更线上
	if (iHalfSeg == iX)
	{
		if (5 == iFace && iHalfSeg >= iY)
		{
			// negZ
			iIndex = iVertPerFace * 6 + (iHalfSeg - iY);
		}
		else if (1 == iFace)
		{
			// negX
			iIndex = iVertPerFace * 6 + iHalfSeg + 1 + iY;
		}
		else if (4 == iFace && iHalfSeg <= iY)
		{
			// posZ
			iIndex = iVertPerFace * 6 + 2*(iSegment + 1) + iHalfSeg - iY;
		}
		else {}
	}

	return iIndex;
}
