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
#include "GMTerrain.h"
#include "GMKit.h"
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <ppl.h>
using namespace concurrency;

using namespace GM;
/*************************************************************************
Macro Defines
*************************************************************************/

#define TRANS_ALT_NUM			(128)			// 透过率图的高度采样数 [0,fAtmosThick]m
#define TRANS_PITCH_NUM			(256)			// 透过率图的太阳俯仰角余弦值采样数 [地平线余弦值,1]

#define IRRA_ALT_NUM			(128)			// 辐照度的高度采样数 [0,fAtmosThick]m
#define IRRA_UP_NUM				(128)			// 辐照度的太阳方向与上方向的点乘采样数 [-1,1]

#define SCAT_COS_NUM			(32)			// 散射图的太阳方向与视线方向的点乘采样数 [-1,1]
#define SCAT_LIGHT_NUM			(16)			// 散射图的太阳方向与上方向的点乘采样数 [-1,1]
#define SCAT_PITCH_NUM			(256)			// 散射图的俯仰方向与上方向的点乘采样数 [-1,1]
#define SCAT_ALT_NUM			(16)			// 散射图的大气点高度采样数 [0,fAtmosThick]m

/*************************************************************************
CGMPlanet Methods
*************************************************************************/

/** @brief 构造 */
CGMPlanet::CGMPlanet(): m_pKernelData(nullptr), m_pConfigData(nullptr), m_pCommonUniform(nullptr),
	m_strCoreModelPath("Models/"),m_pTerrain(nullptr), m_pCelestialScaleVisitor(nullptr)
{
	m_pTerrain = new CGMTerrain();
	m_pCelestialScaleVisitor = new CGMCelestialScaleVisitor();
}

/** @brief 析构 */
CGMPlanet::~CGMPlanet()
{
}

/** @brief 初始化 */
bool CGMPlanet::Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform)
{
	m_pKernelData = pKernelData;
	m_pConfigData = pConfigData;
	m_pCommonUniform = pCommonUniform;

	m_pTerrain->Init(m_pKernelData, m_pConfigData, m_pCommonUniform);

	return true;
}

bool CGMPlanet::CreatePlanet()
{
	m_pTerrain->CreateTerrain();

	return false;
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
	std::string strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/Earth_DEM_";
	//_Panorama_2_CubeDEM(strPanoPath, strCubemapPath);

	//strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/Earth_base.tif";
	//strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/Earth_base_";
	//_Panorama_2_CubeMap(strPanoPath, strCubemapPath);
	
	//strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/wanderingEarth_base_real.tif";
	//strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/wanderingEarth_base_real_";
	//_Panorama_2_CubeMap(strPanoPath, strCubemapPath);

	//strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/Earth_illum_real.tif";
	//strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/Earth_illum_real_";
	//_Panorama_2_CubeMap(strPanoPath, strCubemapPath);

	//strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/Earth_cloud_real.tif";
	//strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/Earth_cloud_real_";
	//_Panorama_2_CubeMap(strPanoPath, strCubemapPath);

	strPanoPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/wanderingEarth_cloud.tif";
	strCubemapPath = m_pConfigData->strCorePath + "Textures/Sphere/Earth/wanderingEarth_cloud_real_";
	//_Panorama_2_CubeMap(strPanoPath, strCubemapPath);

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
	const std::string& strCubeDEMPath)
{
	osg::ref_ptr<osg::Image> pBedImg = osgDB::readImageFile(strPanoPath);
	if (!pBedImg.valid()) return false;

	int iSize = 4096;
	float fHalfSize = iSize * 0.5f;

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

		GLushort* pCubeDEMData = new GLushort[iSize * iSize];
		for (int x = 0; x < iSize; x++)
		{
			for (int y = 0; y < iSize; y++)
			{
				// 要注意，每张图的周围一圈像素都是保护边界，与相邻的图片边界相同，这样可以避免边缘接缝问题
				// 每个像素的方向
				osg::Vec3 vDir = vCenter
					+ vAxisX * (x * iSize / float(iSize - 1) - fHalfSize) / fHalfSize
					+ vAxisY * (y * iSize / float(iSize - 1) - fHalfSize) / fHalfSize;
				vDir.normalize();

				float fLon = atan2(vDir.y(), vDir.x());// 弧度
				float fLat = asin(vDir.z());// 弧度
				// tif的左下角坐标：(0.0, 0.0)
				float fX = 0.5f + fLon / (osg::PI * 2);
				float fY = 0.5f + fLat / (osg::PI);
				float fElevBed = CGMKit::GetImageColor(pBedImg, fX, fY, true).r();
				// float to short
				GLushort sElevBed = min(1.0f, sqrt(abs(fElevBed * 1e-4)))*32767;
				if (fElevBed > 0)
					sElevBed = 32768 + sElevBed;
				else
					sElevBed = 32768 - sElevBed;

				// 最终要转成raw，所以y需要颠倒
				int iAddress = x + (iSize - 1 - y) * iSize;
				pCubeDEMData[iAddress] = sElevBed;
			}
		}

		osg::ref_ptr<osg::Image> pCubeImage = new osg::Image();
		pCubeImage->setImage(iSize, iSize, 1, GL_R16F, GL_RED, GL_UNSIGNED_SHORT, (unsigned char*)pCubeDEMData, osg::Image::USE_NEW_DELETE);
		osgDB::writeImageFile(*(pCubeImage.get()), strCubeDEMPath + std::to_string(i) + ".tif");
	}
	); // end parallel_for
	return true;
}

bool CGMPlanet::_Panorama_2_CubeMap(const std::string& strPanoramaPath, const std::string& strCubeMapPath)
{
	osg::ref_ptr<osg::Image> pPanoImg = osgDB::readImageFile(strPanoramaPath);
	if(!pPanoImg.valid()) return false;

	int iSize = pPanoImg->t() / 2;
	int iByteNum = iSize * iSize * 4;
	float fHalfSize = iSize * 0.5f;

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

		unsigned char* pCubeColorData = new unsigned char[iByteNum];
		for (int x = 0; x < iSize; x++)
		{
			for (int y = 0; y < iSize; y++)
			{
				// 要注意，每张图的周围一圈像素都是保护边界，与相邻的图片边界互补，这样可以避免边缘接缝问题
				// 每个像素的方向
				osg::Vec3 vDir = vCenter
					+ vAxisX * (x - fHalfSize + 0.5f) / (fHalfSize-1)
					+ vAxisY * (y - fHalfSize + 0.5f) / (fHalfSize-1);
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
		osgDB::writeImageFile(*(pCubeImage.get()), strCubeMapPath + std::to_string(i) + ".tif");
	}
	); // end parallel_for
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
