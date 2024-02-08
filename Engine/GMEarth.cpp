//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMEarth.cpp
/// @brief		Galaxy-Music Engine - GMEarth
/// @version	1.0
/// @author		LiuTao
/// @date		2023.11.29
//////////////////////////////////////////////////////////////////////////

#include "GMEarth.h"
#include "GMEngine.h"
#include "GMXml.h"
#include "GMKit.h"
#include <osg/PointSprite>
#include <osg/LineWidth>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/PositionAttitudeTransform>
#include <osg/PolygonOffset>
#include <osg/Depth>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/CullFace>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

using namespace GM;

/*************************************************************************
constexpr
*************************************************************************/

constexpr double EARTH_EQUATOR = 6378137.0; 		// 地球赤道半径
constexpr double EARTH_POLAR = 6356752.0; 			// 地球两极半径

/*************************************************************************
Class
*************************************************************************/
namespace GM
{
	class CRTTFinishCallback : public osg::Camera::DrawCallback
	{
	public:
		CRTTFinishCallback(osg::Image* pBaseImg, osg::Image* pIllumImg, int i)
			:_pBaseImage(pBaseImg), _pIllumImage(pIllumImg), iCount(i), bWritten(false) {}

		virtual void operator() (osg::RenderInfo& renderInfo) const
		{
			if (!bWritten)
			{
				std::string strImgNum = std::to_string(iCount);
				osgDB::writeImageFile(*(_pBaseImage), "../../Data/Core/Textures/Sphere/Earth/engineBody" + strImgNum + ".tif");
				osgDB::writeImageFile(*(_pIllumImage.get()), "../../Data/Core/Textures/Sphere/Earth/bloom" + strImgNum + ".tif");
				std::cout << strImgNum << " RTT Finished!" << std::endl;

				bWritten = true;
			}
		}

	private:
		osg::ref_ptr<osg::Image>	_pBaseImage;
		osg::ref_ptr<osg::Image>	_pIllumImage;
		int							iCount;
		mutable bool				bWritten; // 是否已经写入硬盘
	};

	class CGenEngineDataVisitor : public osg::NodeVisitor
	{
	public:
		CGenEngineDataVisitor(): NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN){}

		void apply(osg::Node& node) { traverse(node); }
		void apply(osg::Geode& node)
		{
			for (unsigned int i = 0; i < node.getNumDrawables(); ++i)
			{
				osg::Geometry* geom = dynamic_cast<osg::Geometry*>(node.getDrawable(i));
				if (geom)
				{
					osg::ref_ptr<osg::Vec3Array> pVert = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
					if (!pVert.valid()) continue;
					// 全球一共1万座行星发动机
					const int iEngineNum = 10000;

					int iVertNum = pVert->size();
					if (iVertNum < iEngineNum) continue;

					std::string strEarthPath = "../../Data/Core/Textures/Sphere/Earth/";
					osg::ref_ptr<osg::Image> pDEMImg = osgDB::readImageFile(strEarthPath + "DEM_bed.tif");

					int iSize = 4 * iEngineNum;
					float* pData = new float[iSize];		
					for (int i = 0; i < iEngineNum; i++)
					{
						float fLon = pVert->at(i).y() * osg::PI;
						float fLat = pVert->at(i).z() * osg::PI;
						float fDEM = CGMKit::GetImageColor(pDEMImg, fLon / (osg::PI * 2) + 0.5, fLat / osg::PI + 0.5, true).r();

						pData[i * 4] = fLon;// 经度（弧度）
						pData[i * 4 + 1] = fLat;// 纬度（弧度）
						pData[i * 4 + 2] = max(0.0f, fDEM);// 发动机底部海拔高度（米）
						pData[i * 4 + 3] = pVert->at(i).x() * 1e5f;// 发动机本身高度（米）
					}
					osg::ref_ptr<osg::Image> pEngineDataImage = new osg::Image();
					pEngineDataImage->setImage(iEngineNum, 1, 1, GL_RGBA32F, GL_RGBA, GL_FLOAT, (unsigned char*)pData, osg::Image::USE_NEW_DELETE);
					osgDB::writeImageFile(*(pEngineDataImage), strEarthPath + "EarthEngineData.tif");
					std::cout << "Engine Data generation succeed!" << std::endl;
				}
			}
			traverse(node);
		}
	};

	class CGenEngineBodyVisitor : public osg::NodeVisitor
	{
	public:
		CGenEngineBodyVisitor(osg::Image* pDataImg, const double fUnit)
			: NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _fUnit(fUnit),
			_pEarthEngineDataImg(pDataImg)
		{
			ellipsoid.setRadiusEquator(osg::WGS_84_RADIUS_EQUATOR / _fUnit);
			ellipsoid.setRadiusPolar(osg::WGS_84_RADIUS_POLAR / _fUnit);
		}

		void apply(osg::Node& node) { traverse(node); }
		void apply(osg::Geode& node)
		{
			for (unsigned int k = 0; k< node.getNumDrawables(); ++k)
			{
				osg::Geometry* geom = dynamic_cast<osg::Geometry*>(node.getDrawable(k));
				if (!geom) continue;

				osg::ref_ptr<osg::Vec3Array> pVertOld = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
				osg::ref_ptr<osg::Vec2Array> pCoordOld = dynamic_cast<osg::Vec2Array*>(geom->getTexCoordArray(0));
				if (!pVertOld.valid() || !pCoordOld.valid()) continue;

				int iEngineNum = _pEarthEngineDataImg->s();
				int iVertPerEngine = int(pVertOld->size());
				int iVertNum = iEngineNum * iVertPerEngine;

				geom->setUseVertexBufferObjects(true);
				geom->setUseDisplayList(false);
				geom->setDataVariance(osg::Object::STATIC);

				osg::ref_ptr<osg::Vec3Array> pVerts = new osg::Vec3Array();
				pVerts->reserve(iVertNum);
				osg::ref_ptr<osg::Vec4Array> pCoords = new osg::Vec4Array;
				pCoords->reserve(iVertNum);
				osg::ref_ptr<osg::DrawElementsUInt> pEle = new osg::DrawElementsUInt(GL_TRIANGLES);
				pEle->reserve(iVertNum);

				for (int i = 0; i < iEngineNum; i++)
				{
					osg::Vec4f vData = CGMKit::GetImageColor(_pEarthEngineDataImg, float(i + 0.5) / float(iEngineNum), 0);
					double fLon = vData.x();
					double fLat = vData.y();
					double fDEM = vData.z() / _fUnit;
					double fX, fY, fZ;
					ellipsoid.convertLatLongHeightToXYZ(fLat, fLon, fDEM, fX, fY, fZ);
					osg::Vec3 vSpherePos = osg::Vec3(fX, fY, fZ);

					osg::Vec3 vVertUp = ellipsoid.computeLocalUpVector(vSpherePos.x(), vSpherePos.y(), vSpherePos.z());
					osg::Vec3 vVertEast = osg::Vec3(0, 0, 1) ^ vVertUp;
					vVertEast.normalize();
					osg::Vec3 vVertNorth = vVertUp ^ vVertEast;
					vVertNorth.normalize();

					// model matrix 模型空间转世界空间矩阵
					osg::Matrixd mModelMatrix = osg::Matrixd(
						vVertEast.x(),	vVertEast.y(),	vVertEast.z(),	0,
						vVertNorth.x(),	vVertNorth.y(),	vVertNorth.z(),	0,
						vVertUp.x(),	vVertUp.y(),	vVertUp.z(),	0,
						vSpherePos.x(),	vSpherePos.y(),	vSpherePos.z(),	1);

					// 行星发动机直径分两种，大的30000米，小的21000米
					float fScale = 1.0 / _fUnit;
					fScale *= (vData.w() > 1e4) ? 1.0f : 0.7f;
					// 发动机所在位置的地球半径
					float fRadius = vSpherePos.length();

					// 绘制发动机主体
					for (int j = 0; j < iVertPerEngine; j++)
					{
						// xy = UV, z = vertex altitude, w = earth radius at the vertex point
						osg::Vec4 vCoord = osg::Vec4(pCoordOld->at(j).x(), pCoordOld->at(j).y(),
							pVertOld->at(j).z() * fScale, fRadius);

						pVerts->push_back(mModelMatrix.preMult(pVertOld->at(j) * fScale));
						pCoords->push_back(vCoord);
						pEle->push_back(i * iVertPerEngine + j);
					}
				}

				geom->setVertexArray(pVerts);
				geom->setTexCoordArray(0, pCoords);
				geom->setNormalBinding(osg::Geometry::BIND_OFF);
				geom->setPrimitiveSet(0, pEle);
			}
			traverse(node);
		}
	private:
		osg::EllipsoidModel ellipsoid;
		double				_fUnit;
		// 流浪地球行星发动机数据,xy=经纬度（弧度），z=底高（米），w=发动机高度（米）
		// 图片宽度（s）= 发动机数量，高度（t）= 1
		osg::ref_ptr<osg::Image> _pEarthEngineDataImg;
	};

}	// GM

/*************************************************************************
CGMEarth Methods
*************************************************************************/

/** @brief 构造 */
CGMEarth::CGMEarth() : CGMPlanet(),
	m_strGalaxyShaderPath("Shaders/GalaxyShader/"),
	m_strEarthShaderPath("Shaders/EarthShader/"),
	m_fCloudBottom(5e3f), m_fCloudTop(1e4f),
	m_vEarthCoordScaleUniform(new osg::Uniform("coordScale_Earth", osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f))),//UV缩放，四层够用了
	m_fWanderProgressUniform(new osg::Uniform("wanderProgress", 0.0f)),
	m_fEngineStartRatioUniform(new osg::Uniform("engineStartRatio", 0.0f)),
	m_pEarthTail(nullptr)
{
	m_pEarthRoot_1 = new osg::Group();
	m_pEarthRoot_2 = new osg::Group();

	m_pEllipsoid = new osg::EllipsoidModel();
	m_pEarthTail = new CGMEarthTail();
}

/** @brief 析构 */
CGMEarth::~CGMEarth()
{
}

/** @brief 初始化 */
bool CGMEarth::Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform)
{
	CGMPlanet::Init(pConfigData);

	Panorama2CubeMap();

	m_pKernelData = pKernelData;
	m_pCommonUniform = pCommonUniform;

	// 读取dds时需要垂直翻转
	m_pDDSOptions = new osgDB::Options("dds_flip");
	std::string strSphereTexPath = m_pConfigData->strCorePath + "Textures/Sphere/";
	std::string strVolumeTexPath = m_pConfigData->strCorePath + "Textures/Volume/";

	if (m_pConfigData->bWanderingEarth)
	{
		// 用于存储行星发动机所有参数的图片
		m_pEarthEngineDataImg = osgDB::readImageFile(strSphereTexPath + "Earth/EarthEngineData.tif");
		// 初始化流浪地球尾迹
		m_pEarthTail->Init(pKernelData, pConfigData, pCommonUniform);
	}
	for (int i = 0; i <= 2; i++)
	{
		osg::ref_ptr<osg::Group> _pRoot = new osg::Group();
		m_pHieEarthRootVector.push_back(_pRoot);
	}

	m_aEarthBaseTex = _CreateDDSTex2DArray(strSphereTexPath + "Earth/Earth_base_");
	m_aEarthCloudTex = _CreateDDSTex2DArray(strSphereTexPath + "Earth/Earth_cloud_");
	if (m_pConfigData->bWanderingEarth)
	{
		_AddTex2DArray(m_aEarthBaseTex, strSphereTexPath + "Earth/wanderingEarth_base_");
		_AddTex2DArray(m_aEarthCloudTex, strSphereTexPath + "Earth/wanderingEarth_cloud_");
	}

	m_aIllumTex = _CreateDDSTex2DArray(strSphereTexPath + "Earth/Earth_illum_");

	// 加载DEM
	m_aDEMTex = new osg::Texture2DArray;
	m_aDEMTex->setTextureSize(4096, 4096, 6);
	for (int i = 0; i < 6; i++)
	{
		std::string fileName = strSphereTexPath + "Earth/Earth_DEM_" + std::to_string(i) + ".raw";
		osg::ref_ptr<osg::Image> pImg = osgDB::readImageFile(fileName);
		pImg->setImage(4096, 4096, 1, GL_R16F, GL_RED, GL_UNSIGNED_SHORT, pImg->data(), osg::Image::NO_DELETE);
		m_aDEMTex->setImage(i, pImg);
	}
	m_aDEMTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	m_aDEMTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_aDEMTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_aDEMTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_aDEMTex->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
	m_aDEMTex->setInternalFormat(GL_R16F);
	m_aDEMTex->setSourceFormat(GL_RED);
	m_aDEMTex->setSourceType(GL_UNSIGNED_SHORT);

	//// 极光
	//m_aAuroraTex = _CreateDDSTex2DArray(strSphereTexPath + "aurora.dds");
	
	// 云的细节纹理
	m_pCloudDetailTex = _CreateDDSTexture(strVolumeTexPath + "CloudDetail.dds", osg::Texture::REPEAT, osg::Texture::REPEAT);
	// 散射三维纹理
	osg::ref_ptr <osg::Image> pImg = osgDB::readImageFile(strSphereTexPath + "Inscattering/Inscattering_64_6400.raw");
	if (pImg.valid())
	{
		pImg->setImage(SCAT_COS_NUM, SCAT_LIGHT_NUM, SCAT_PITCH_NUM * SCAT_ALT_NUM,
			GL_RGB32F, GL_RGB, GL_FLOAT, pImg->data(), osg::Image::NO_DELETE);
		m_pInscatteringTex = new osg::Texture3D;
		m_pInscatteringTex->setImage(pImg);
		m_pInscatteringTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		m_pInscatteringTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
		m_pInscatteringTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		m_pInscatteringTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
		m_pInscatteringTex->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
		m_pInscatteringTex->setInternalFormat(GL_RGB32F);
		m_pInscatteringTex->setSourceFormat(GL_RGB);
		m_pInscatteringTex->setSourceType(GL_FLOAT);
	}

	// 获取当前天体贴图的尺寸，用于修改UV缩放系数
	float fBaseTexSize = m_aEarthBaseTex->getTextureWidth();
	float fCloudTexSize = m_aEarthCloudTex->getTextureWidth();
	float fIlluTexSize = m_aIllumTex->getTextureWidth();
	float fDEMTexSize = m_aDEMTex->getTextureWidth();
	// 根据贴图大小，修改m_vCoordScaleUniform
	// 为了避免六边形边缘采样时有缝，特意在每个面的贴图边缘留了一个像素的保护像素，和隔壁像素重合
	// 所以采样时需要根据贴图大小，向每个面各自的中心缩小一个像素
	// x = baseTex; y = cloudTex; z = illumTex;
	osg::Vec4f vCoordScale(
		(fBaseTexSize - 2.0f) / fBaseTexSize,
		(fCloudTexSize - 2.0f) / fCloudTexSize,
		(fIlluTexSize - 2.0f) / fIlluTexSize,
		(fDEMTexSize - 1.0f) / fDEMTexSize);
	m_vEarthCoordScaleUniform->set(vCoordScale);

	return true;
}

/** @brief 更新 */
bool CGMEarth::Update(double dDeltaTime)
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

	if (m_pConfigData->bWanderingEarth)
		m_pEarthTail->Update(dDeltaTime);

	return true;
}

/** @brief 更新(在主相机更新姿态之后) */
bool CGMEarth::UpdateLater(double dDeltaTime)
{
	int iHie = m_pKernelData->iHierarchy;

	osg::Matrixd mViewMatrix = GM_View->getCamera()->getViewMatrix();
	osg::Matrixd mProjMatrix = GM_View->getCamera()->getProjectionMatrix();

	// 修改全球阴影相机
	m_pGlobalShadowCamera->setViewMatrix(mViewMatrix);
	m_pGlobalShadowCamera->setProjectionMatrix(mProjMatrix);

	if (m_pConfigData->bWanderingEarth)
		m_pEarthTail->UpdateLater(dDeltaTime);

	return true;
}

/** @brief 加载 */
bool CGMEarth::Load()
{
	std::string strGalaxyShader = m_pConfigData->strCorePath + m_strGalaxyShaderPath;
	std::string strEarthShader = m_pConfigData->strCorePath + m_strEarthShaderPath;

	if (m_pSSEarthGround_1.valid())
	{
		CGMKit::LoadShaderWithCommonFrag(m_pSSEarthGround_1,
			strGalaxyShader + "CelestialGround.vert",
			strGalaxyShader + "CelestialGround.frag",
			strGalaxyShader + "CelestialCommon.frag",
			"EarthGround_1");
		CGMKit::LoadShaderWithCommonFrag(m_pSSEarthGround_2,
			strGalaxyShader + "CelestialGround.vert",
			strGalaxyShader + "CelestialGround.frag",
			strGalaxyShader + "CelestialCommon.frag",
			"EarthGround_2");
	}
	if (m_pSSEarthCloud_1.valid())
	{
		CGMKit::LoadShaderWithCommonFrag(m_pSSEarthCloud_1,
			strGalaxyShader + "CelestialCloud.vert",
			strGalaxyShader + "CelestialCloud.frag",
			strGalaxyShader + "CelestialCommon.frag",
			"EarthCloud_1");
		CGMKit::LoadShaderWithCommonFrag(m_pSSEarthCloud_2,
			strGalaxyShader + "CelestialCloud.vert",
			strGalaxyShader + "CelestialCloud.frag",
			strGalaxyShader + "CelestialCommon.frag",
			"EarthCloud_2");
	}
	if (m_pSSEarthAtmos_1.valid())
	{
		CGMKit::LoadShader(m_pSSEarthAtmos_1,
			strGalaxyShader + "CelestialAtmosphere.vert",
			strGalaxyShader + "CelestialAtmosphere.frag",
			"EarthAtmosphere_1");
		CGMKit::LoadShader(m_pSSEarthAtmos_2,
			strGalaxyShader + "CelestialAtmosphere.vert",
			strGalaxyShader + "CelestialAtmosphere.frag",
			"EarthAtmosphere_2");
	}

	if (m_pEarthEnginePointNode_1.valid())
	{
		CGMKit::LoadShader(m_pEarthEnginePointNode_1->getStateSet(),
			strEarthShader + "PlanetEnginePoint.vert",
			strEarthShader + "PlanetEnginePoint.frag",
			"PlanetEnginePoint_1");
	}
	if (m_pEarthEnginePointNode_2.valid())
	{
		CGMKit::LoadShader(m_pEarthEnginePointNode_2->getStateSet(),
			strEarthShader + "PlanetEnginePoint.vert",
			strEarthShader + "PlanetEnginePoint.frag",
			"PlanetEnginePoint_2");
	}
	if (m_pEarthEngineJetNode_1.valid())
	{
		CGMKit::LoadShader(m_pEarthEngineJetNode_1->getStateSet(),
			strEarthShader + "PlanetEngineJet.vert",
			strEarthShader + "PlanetEngineJet.frag",
			"PlanetEngineJet_1");
	}
	if (m_pEarthEngineJetNode_2.valid())
	{
		CGMKit::LoadShader(m_pEarthEngineJetNode_2->getStateSet(),
			strEarthShader + "PlanetEngineJet.vert",
			strEarthShader + "PlanetEngineJet.frag",
			"PlanetEngineJet_2");
	}
	if (m_pEarthEngineBody_1.valid())
	{
		CGMKit::LoadShaderWithCommonFrag(m_pEarthEngineBody_1->getStateSet(),
			strEarthShader + "PlanetEngineBody.vert",
			strEarthShader + "PlanetEngineBody.frag",
			strGalaxyShader + "CelestialCommon.frag",
			"PlanetEngineBody_1");
	}
	if (m_pEarthEngineBody_2.valid())
	{
		CGMKit::LoadShaderWithCommonFrag(m_pEarthEngineBody_2->getStateSet(),
			strEarthShader + "PlanetEngineBody.vert",
			strEarthShader + "PlanetEngineBody.frag",
			strGalaxyShader + "CelestialCommon.frag",
			"PlanetEngineBody_2");
	}
	if (m_pEarthEngineStream.valid())
	{
		CGMKit::LoadShader(m_pEarthEngineStream->getStateSet(),
			strEarthShader + "PlanetEngineJetStream.vert",
			strEarthShader + "PlanetEngineJetStream.frag",
			"PlanetEngineJetStream");
	}

	if (m_pSSGlobalShadow.valid())
	{
		CGMKit::LoadShader(m_pSSGlobalShadow,
			strEarthShader + "GlobalShadow.vert",
			strEarthShader + "GlobalShadow.frag",
			"Global Shadow");
	}

	if (m_pConfigData->bWanderingEarth)
		m_pEarthTail->Load();

	return true;
}

void CGMEarth::SetUniform(
	osg::Uniform* pPlanetRadius,
	osg::Uniform* pViewLight,
	osg::Uniform* pCloudTop,
	osg::Uniform* pGroundTop,
	osg::Uniform* pAtmosHeight,
	osg::Uniform* pMinDotUL,
	osg::Uniform* pEyeAltitude,
	osg::Uniform* pView2ECEF)
{
	m_vPlanetRadiusUniform = pPlanetRadius;
	m_vViewLightUniform = pViewLight;
	m_fCloudTopUniform = pCloudTop;
	m_fGroundTopUniform = pGroundTop;
	m_fAtmosHeightUniform = pAtmosHeight;
	m_fMinDotULUniform = pMinDotUL;
	m_fEyeAltitudeUniform = pEyeAltitude;
	m_mView2ECEFUniform = pView2ECEF;

	if (m_pConfigData->bWanderingEarth)
	{
		m_pEarthTail->SetUniform(m_vViewLightUniform, m_fEngineStartRatioUniform);
	}
}

void CGMEarth::ResizeScreen(const int iW, const int iH)
{
	m_pGlobalShadowCamera->resize(iW, iH);

	if (m_pConfigData->bWanderingEarth)
		m_pEarthTail->ResizeScreen(iW, iH);
}

osg::Node* CGMEarth::GetEarthRoot(const int iHie) const
{
	if (0 == iHie)
	{
		return nullptr;
	}
	else if (1 == iHie)
	{
		return m_pEarthRoot_1.get();
	}
	else if (2 == iHie)
	{
		return m_pEarthRoot_2.get();
	}
	else
	{
		return nullptr;
	}
}

void CGMEarth::SetVisible(const bool bVisible)
{
	if (bVisible)
	{
		if (0 == m_pEarthRoot_1->getNodeMask())
		{
			m_pEarthRoot_1->setNodeMask(~0);
			m_pEarthRoot_2->setNodeMask(~0);
		}
	}
	else
	{
		if (0 != m_pEarthRoot_1->getNodeMask())
		{
			m_pEarthRoot_1->setNodeMask(0);
			m_pEarthRoot_2->setNodeMask(0);
		}
	}

	// 流浪地球
	if (m_pConfigData->bWanderingEarth)
	{
		if (bVisible)
		{
			if (0 == m_pEarthEnginePointNode_1->getNodeMask())
			{
				m_pEarthEnginePointNode_1->setNodeMask(~0);
				m_pEarthEngineJetNode_1->setNodeMask(~0);
				m_pEarthEnginePointNode_2->setNodeMask(~0);
				m_pEarthEngineJetNode_2->setNodeMask(~0);
				m_pEarthEngineStream->setNodeMask(~0);
				m_pEarthEngineBody_1->setNodeMask(~0);
				m_pEarthEngineBody_2->setNodeMask(~0);
			}
		}
		else
		{
			if (0 != m_pEarthEnginePointNode_1->getNodeMask())
			{
				m_pEarthEnginePointNode_1->setNodeMask(0);
				m_pEarthEngineJetNode_1->setNodeMask(0);
				m_pEarthEnginePointNode_2->setNodeMask(0);
				m_pEarthEngineJetNode_2->setNodeMask(0);
				m_pEarthEngineStream->setNodeMask(0);
				m_pEarthEngineBody_1->setNodeMask(0);
				m_pEarthEngineBody_2->setNodeMask(0);
			}
		}

		m_pEarthTail->SetVisible(bVisible);
	}
}

void CGMEarth::SetEarthRotate(const osg::Quat& qRotate)
{
	m_pPlanet_2_Transform->asPositionAttitudeTransform()->setAttitude(qRotate);

	if (m_pConfigData->bWanderingEarth)
		m_pEarthTail->SetEarthTailRotate(qRotate);
}

void CGMEarth::SetWanderingEarthProgress(const float fProgress)
{
	if (!m_pConfigData->bWanderingEarth) return;

	m_fWanderProgressUniform->set(osg::clampBetween(fProgress, 0.0f, 1.0f));
	// 发动机在月球危机时开启
	m_fEngineStartRatioUniform->set(fmaxf((fProgress-0.4f)*10.0f, 0.0f));
}

bool CGMEarth::CreateEarth()
{
	_CreateGlobalCloudShadow();

	// 创建地球，用于1级空间
	_CreateEarth_1();
	// 创建地球，用于2级空间
	_CreateEarth_2();

	// 创建流浪地球相关的东西
	if (m_pConfigData->bWanderingEarth)
	{
		_CreateWanderingEarth();
	}

	return true;
}

bool CGMEarth::UpdateHierarchy(int iHieNew)
{
	if (iHieNew <= 2)
	{
		// 移除下一空间层级
		if (iHieNew > 0 && GM_Root->containsNode(m_pHieEarthRootVector.at(iHieNew - 1)))
		{
			GM_Root->removeChild(m_pHieEarthRootVector.at(iHieNew - 1));
		}
		// 移除上一空间层级
		if (iHieNew < 2 && GM_Root->containsNode(m_pHieEarthRootVector.at(iHieNew + 1)))
		{
			GM_Root->removeChild(m_pHieEarthRootVector.at(iHieNew + 1));
		}
		// 添加当前空间层级
		if (!(GM_Root->containsNode(m_pHieEarthRootVector.at(iHieNew))))
		{
			GM_Root->addChild(m_pHieEarthRootVector.at(iHieNew));
		}
	}

	switch (iHieNew)
	{
	case 0:
	{
		// 隐藏全球云对地阴影相关的相机和节点
		m_pGlobalShadowCamera->setNodeMask(0);
	}
	break;
	case 1:
	{
		// 显示全球云对地阴影相关的相机和节点
		m_pGlobalShadowCamera->setNodeMask(~0);
		m_mShadowEarthGeode.at(1)->setNodeMask(~0);
		m_mShadowEarthGeode.at(2)->setNodeMask(0);
	}
	break;
	case 2:
	{
		// 显示全球云对地阴影相关的相机和节点
		m_pGlobalShadowCamera->setNodeMask(~0);
		m_mShadowEarthGeode.at(1)->setNodeMask(0);
		m_mShadowEarthGeode.at(2)->setNodeMask(~0);
	}
	break;
	case 3:
	{
		// 隐藏全球云对地阴影相关的相机和节点
		m_pGlobalShadowCamera->setNodeMask(0);
	}
	break;
	default:
		break;
	}

	if (m_pConfigData->bWanderingEarth)
		m_pEarthTail->UpdateHierarchy(iHieNew);

	return true;
}

bool CGMEarth::_CreateGlobalCloudShadow()
{
	int iWidth = 1920;
	int iHeight = 1080;

	m_pGlobalShadowTex = new osg::Texture2D;
	m_pGlobalShadowTex->setName("Global Shadow Tex");
	m_pGlobalShadowTex->setTextureSize(iWidth, iHeight);
	m_pGlobalShadowTex->setInternalFormat(GL_R8);
	m_pGlobalShadowTex->setSourceFormat(GL_RED);
	m_pGlobalShadowTex->setSourceType(GL_UNSIGNED_BYTE);
	m_pGlobalShadowTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_pGlobalShadowTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_pGlobalShadowTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_pGlobalShadowTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_pGlobalShadowTex->setDataVariance(osg::Object::DYNAMIC);
	m_pGlobalShadowTex->setResizeNonPowerOfTwoHint(false);

	m_pGlobalShadowCamera = new osg::Camera;
	m_pGlobalShadowCamera->setName("Global Shadow Camera");
	m_pGlobalShadowCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT);
	m_pGlobalShadowCamera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	m_pGlobalShadowCamera->setClearColor(osg::Vec4(1.0f, 0.0f, 0.0f, 0.0f));
	m_pGlobalShadowCamera->setViewport(0, 0, iWidth, iHeight);
	m_pGlobalShadowCamera->setRenderOrder(osg::Camera::PRE_RENDER);
	m_pGlobalShadowCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	m_pGlobalShadowCamera->attach(osg::Camera::COLOR_BUFFER, m_pGlobalShadowTex);
	m_pGlobalShadowCamera->setAllowEventFocus(false);
	m_pGlobalShadowCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

	for (int i = 1; i <= 2; i++)
	{
		osg::ref_ptr<osg::Geometry>	pShadowEarthGeom = MakeHexahedronSphereGeometry();
		if (!pShadowEarthGeom.valid()) return false;

		double fUnit = m_pKernelData->fUnitArray->at(i);
		m_pCelestialScaleVisitor->SetRadius((EARTH_EQUATOR + m_fCloudTop) / fUnit, (EARTH_POLAR + m_fCloudTop) / fUnit);
		pShadowEarthGeom->accept(*m_pCelestialScaleVisitor);	// 改变大小
		osg::ref_ptr<osg::Geode> pShadowEarth = new osg::Geode();
		m_mShadowEarthGeode[i] = pShadowEarth;
		pShadowEarth->addDrawable(pShadowEarthGeom);
		if (2 == i)
		{
			m_pPlanet_2_Transform = new osg::PositionAttitudeTransform();
			m_pPlanet_2_Transform->addChild(pShadowEarth);
			m_pGlobalShadowCamera->addChild(m_pPlanet_2_Transform);
		}
		else
		{
			m_pGlobalShadowCamera->addChild(pShadowEarth);
		}
	}

	m_pSSGlobalShadow = new osg::StateSet();
	m_pSSGlobalShadow->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	m_pSSGlobalShadow->setMode(GL_BLEND, osg::StateAttribute::OFF);
	m_pSSGlobalShadow->setAttributeAndModes(new osg::CullFace());

	m_pSSGlobalShadow->addUniform(m_vPlanetRadiusUniform);
	m_pSSGlobalShadow->addUniform(m_vEarthCoordScaleUniform);
	m_pSSGlobalShadow->addUniform(m_fCloudTopUniform);
	m_pSSGlobalShadow->addUniform(m_vViewLightUniform);
	m_pSSGlobalShadow->addUniform(m_mView2ECEFUniform);

	int iShadowUnit = 0;
	// 基础纹理
	m_pSSGlobalShadow->setTextureAttributeAndModes(iShadowUnit, m_aEarthCloudTex, osg::StateAttribute::ON);
	osg::ref_ptr<osg::Uniform> pGlobalCloudUniform = new osg::Uniform("cloudTex", iShadowUnit++);
	m_pSSGlobalShadow->addUniform(pGlobalCloudUniform);
	// 云的细节纹理
	m_pSSGlobalShadow->setTextureAttributeAndModes(iShadowUnit, m_pCloudDetailTex, osg::StateAttribute::ON);
	osg::ref_ptr<osg::Uniform> pCloudDetailUniform = new osg::Uniform("cloudDetailTex", iShadowUnit++);
	m_pSSGlobalShadow->addUniform(pCloudDetailUniform);

	if (m_pConfigData->bWanderingEarth)
	{
		m_pSSGlobalShadow->addUniform(m_fWanderProgressUniform);
		m_pSSGlobalShadow->setDefine("WANDERING", osg::StateAttribute::ON);
	}

	// 添加shader
	std::string strShaderPath = m_pConfigData->strCorePath + m_strEarthShaderPath;
	CGMKit::LoadShader(m_pSSGlobalShadow,
		strShaderPath + "GlobalShadow.vert",
		strShaderPath + "GlobalShadow.frag",
		"Global Shadow");

	m_pGlobalShadowCamera->setStateSet(m_pSSGlobalShadow);
	// 将全球云对地阴影相机加到根节点下，只在1、2层级下才显示
	GM_Root->addChild(m_pGlobalShadowCamera);

	return true;
}

bool CGMEarth::_CreateEarth_1()
{
	m_pEarthGeom_1 = MakeHexahedronSphereGeometry();
	if (!m_pEarthGeom_1.valid()) return false;

	std::string strShaderPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath;
	unsigned int iOnOverride = osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE;
	double fUnit1 = m_pKernelData->fUnitArray->at(1);
	m_pCelestialScaleVisitor->SetRadius(EARTH_EQUATOR / fUnit1, EARTH_POLAR / fUnit1);
	// 改变大小
	m_pEarthGeom_1->accept(*m_pCelestialScaleVisitor);

	// 地球岩石地面
	osg::ref_ptr<osg::Geode> pEarthGround_1 = new osg::Geode();
	pEarthGround_1->addDrawable(m_pEarthGeom_1);
	m_pEarthRoot_1->addChild(pEarthGround_1);

	m_pSSEarthGround_1 = pEarthGround_1->getOrCreateStateSet();
	m_pSSEarthGround_1->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	m_pSSEarthGround_1->setMode(GL_BLEND, osg::StateAttribute::OFF);
	m_pSSEarthGround_1->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	m_pSSEarthGround_1->setAttributeAndModes(new osg::CullFace());
	m_pSSEarthGround_1->setRenderBinDetails(BIN_ROCKSPHERE, "DepthSortedBin");
	// 地球宏定义
	m_pSSEarthGround_1->setDefine("EARTH", osg::StateAttribute::ON);
	m_pSSEarthGround_1->setDefine("ATMOS", osg::StateAttribute::ON);

	int iGroundUnit = 0;
	// 基础贴图
	m_pSSEarthGround_1->setTextureAttributeAndModes(iGroundUnit, m_aEarthBaseTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGrundBaseUniform = new osg::Uniform("baseTex", iGroundUnit++);
	m_pSSEarthGround_1->addUniform(pGrundBaseUniform);
	// 自发光贴图
	m_pSSEarthGround_1->setTextureAttributeAndModes(iGroundUnit, m_aIllumTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGrundIllumUniform = new osg::Uniform("illumTex", iGroundUnit++);
	m_pSSEarthGround_1->addUniform(pGrundIllumUniform);
	// DEM贴图
	m_pSSEarthGround_1->setTextureAttributeAndModes(iGroundUnit, m_aDEMTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pDEMUniform = new osg::Uniform("DEMTex", iGroundUnit++);
	m_pSSEarthGround_1->addUniform(pDEMUniform);
	// 全球阴影
	m_pSSEarthGround_1->setTextureAttributeAndModes(iGroundUnit, m_pGlobalShadowTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGlobalShadowUniform = new osg::Uniform("globalShadowTex", iGroundUnit++);
	m_pSSEarthGround_1->addUniform(pGlobalShadowUniform);
	// 地面上的大气“内散射”纹理
	m_pSSEarthGround_1->setTextureAttributeAndModes(iGroundUnit, m_pInscatteringTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGroundInscatteringUniform = new osg::Uniform("inscatteringTex", iGroundUnit++);
	m_pSSEarthGround_1->addUniform(pGroundInscatteringUniform);

	if (m_pConfigData->bWanderingEarth)
	{
		// 流浪地球尾迹（吹散的大气）
		m_pSSEarthGround_1->setTextureAttributeAndModes(iGroundUnit, m_pEarthTail->GetTAATex(), iOnOverride);
		osg::ref_ptr<osg::Uniform> pGroundTailUniform = new osg::Uniform("tailTex", iGroundUnit++);
		m_pSSEarthGround_1->addUniform(pGroundTailUniform);

		m_pSSEarthGround_1->addUniform(m_fWanderProgressUniform);
		m_pSSEarthGround_1->setDefine("WANDERING", osg::StateAttribute::ON);
	}

	m_pSSEarthGround_1->addUniform(m_pCommonUniform->GetViewUp());
	m_pSSEarthGround_1->addUniform(m_vViewLightUniform);
	m_pSSEarthGround_1->addUniform(m_fAtmosHeightUniform);
	m_pSSEarthGround_1->addUniform(m_fEyeAltitudeUniform);
	m_pSSEarthGround_1->addUniform(m_fGroundTopUniform);
	m_pSSEarthGround_1->addUniform(m_vPlanetRadiusUniform);
	m_pSSEarthGround_1->addUniform(m_fMinDotULUniform);
	m_pSSEarthGround_1->addUniform(m_pCommonUniform->GetScreenSize());
	m_pSSEarthGround_1->addUniform(m_vEarthCoordScaleUniform);
	m_pSSEarthGround_1->addUniform(m_pCommonUniform->GetUnit());
	m_pSSEarthGround_1->addUniform(m_fEngineStartRatioUniform);

	// 添加shader
	CGMKit::LoadShaderWithCommonFrag(m_pSSEarthGround_1,
		strShaderPath + "CelestialGround.vert",
		strShaderPath + "CelestialGround.frag",
		strShaderPath + "CelestialCommon.frag",
		"CelestialGround");

	////////////////////////////////////
	// 地球云层
	m_pEarthCloud_1 = new osg::Geode();
	m_pEarthCloud_1->addDrawable(m_pEarthGeom_1);
	m_pEarthRoot_1->addChild(m_pEarthCloud_1);

	m_pSSEarthCloud_1 = m_pEarthCloud_1->getOrCreateStateSet();
	m_pSSEarthCloud_1->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	m_pSSEarthCloud_1->setMode(GL_BLEND, osg::StateAttribute::ON);
	m_pSSEarthCloud_1->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	m_pSSEarthCloud_1->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false)); // no zbuffer
	m_pSSEarthCloud_1->setAttributeAndModes(new osg::CullFace());
	m_pSSEarthCloud_1->setRenderBinDetails(BIN_CLOUD, "DepthSortedBin");
	// 云层偏移，避免和地面打架
	osg::ref_ptr<osg::PolygonOffset> pCloudPO = new osg::PolygonOffset(-1, -1);
	m_pSSEarthCloud_1->setAttributeAndModes(pCloudPO, iOnOverride);
	m_pSSEarthCloud_1->setDefine("ATMOS", osg::StateAttribute::ON);
	// 默认地球
	m_pSSEarthCloud_1->setDefine("EARTH", osg::StateAttribute::ON);
	m_pSSEarthCloud_1->setDefine("SATURN", osg::StateAttribute::OFF);

	int iCloudUnit = 0;
	// 基础贴图
	m_pSSEarthCloud_1->setTextureAttributeAndModes(iCloudUnit, m_aEarthCloudTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pCloudBaseUniform = new osg::Uniform("cloudTex", iCloudUnit++);
	m_pSSEarthCloud_1->addUniform(pCloudBaseUniform);
	// 云的细节纹理
	m_pSSEarthCloud_1->setTextureAttributeAndModes(iCloudUnit, m_pCloudDetailTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pCloudDetailUniform = new osg::Uniform("cloudDetailTex", iCloudUnit++);
	m_pSSEarthCloud_1->addUniform(pCloudDetailUniform);
	// 云层上的“内散射”纹理
	m_pSSEarthCloud_1->setTextureAttributeAndModes(iCloudUnit, m_pInscatteringTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pCloudInscatteringUniform = new osg::Uniform("inscatteringTex", iCloudUnit++);
	m_pSSEarthCloud_1->addUniform(pCloudInscatteringUniform);

	if (m_pConfigData->bWanderingEarth)
	{
		// 流浪地球尾迹（吹散的大气）
		m_pSSEarthCloud_1->setTextureAttributeAndModes(iCloudUnit, m_pEarthTail->GetTAATex(), iOnOverride);
		osg::ref_ptr<osg::Uniform> pCloudTailUniform = new osg::Uniform("tailTex", iCloudUnit++);
		m_pSSEarthCloud_1->addUniform(pCloudTailUniform);

		m_pSSEarthCloud_1->setTextureAttributeAndModes(iCloudUnit, m_aIllumTex, iOnOverride);
		osg::ref_ptr<osg::Uniform> pCloudIllumUniform = new osg::Uniform("illumTex", iCloudUnit++);
		m_pSSEarthCloud_1->addUniform(pCloudIllumUniform);

		m_pSSEarthCloud_1->addUniform(m_fWanderProgressUniform);
		m_pSSEarthCloud_1->setDefine("WANDERING", osg::StateAttribute::ON);
	}

	m_pSSEarthCloud_1->addUniform(m_pCommonUniform->GetViewUp());
	m_pSSEarthCloud_1->addUniform(m_vViewLightUniform);
	m_pSSEarthCloud_1->addUniform(m_fCloudTopUniform);
	m_pSSEarthCloud_1->addUniform(m_fGroundTopUniform);
	m_pSSEarthCloud_1->addUniform(m_fEyeAltitudeUniform);
	m_pSSEarthCloud_1->addUniform(m_fAtmosHeightUniform);
	m_pSSEarthCloud_1->addUniform(m_vPlanetRadiusUniform);
	m_pSSEarthCloud_1->addUniform(m_fMinDotULUniform);
	m_pSSEarthCloud_1->addUniform(m_pCommonUniform->GetScreenSize());
	m_pSSEarthCloud_1->addUniform(m_vEarthCoordScaleUniform);
	m_pSSEarthCloud_1->addUniform(m_pCommonUniform->GetUnit());
	m_pSSEarthCloud_1->addUniform(m_fEngineStartRatioUniform);

	// 添加shader
	CGMKit::LoadShaderWithCommonFrag(m_pSSEarthCloud_1,
		strShaderPath + "CelestialCloud.vert",
		strShaderPath + "CelestialCloud.frag",
		strShaderPath + "CelestialCommon.frag",
		"CelestialCloud");

	///////////////////////////////////////////////
	// 地球大气
	m_pEarthAtmos_1 = new osg::Geode();
	m_pEarthAtmos_1->addDrawable(m_pEarthGeom_1);
	m_pEarthRoot_1->addChild(m_pEarthAtmos_1);

	m_pSSEarthAtmos_1 = m_pEarthAtmos_1->getOrCreateStateSet();
	m_pSSEarthAtmos_1->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	m_pSSEarthAtmos_1->setMode(GL_BLEND, osg::StateAttribute::ON);
	m_pSSEarthAtmos_1->setAttributeAndModes(new osg::BlendFunc(
		GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_ALPHA, GL_ONE), osg::StateAttribute::ON);
	m_pSSEarthAtmos_1->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false)); // no zbuffer
	m_pSSEarthAtmos_1->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT));
	m_pSSEarthAtmos_1->setRenderBinDetails(BIN_ATMOSPHERE, "DepthSortedBin");
	m_pSSEarthAtmos_1->setDefine("EARTH", osg::StateAttribute::ON);
	m_pSSEarthAtmos_1->setDefine("SATURN", osg::StateAttribute::OFF);

	int iAtmosUnit = 0;
	// “内散射”贴图
	m_pSSEarthAtmos_1->setTextureAttributeAndModes(iAtmosUnit, m_pInscatteringTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pInscatteringUniform = new osg::Uniform("inscatteringTex", iAtmosUnit++);
	m_pSSEarthAtmos_1->addUniform(pInscatteringUniform);

	m_pSSEarthAtmos_1->addUniform(m_pCommonUniform->GetViewUp());
	m_pSSEarthAtmos_1->addUniform(m_vViewLightUniform);
	m_pSSEarthAtmos_1->addUniform(m_fAtmosHeightUniform);
	m_pSSEarthAtmos_1->addUniform(m_fEyeAltitudeUniform);
	m_pSSEarthAtmos_1->addUniform(m_vPlanetRadiusUniform);
	m_pSSEarthAtmos_1->addUniform(m_fMinDotULUniform);
	m_pSSEarthAtmos_1->addUniform(m_mView2ECEFUniform);

	if (m_pConfigData->bWanderingEarth)
	{
		m_pSSEarthAtmos_1->addUniform(m_fWanderProgressUniform);
		m_pSSEarthAtmos_1->setDefine("WANDERING", osg::StateAttribute::ON);
	}

	// 添加shader
	CGMKit::LoadShader(m_pSSEarthAtmos_1,
		strShaderPath + "CelestialAtmosphere.vert",
		strShaderPath + "CelestialAtmosphere.frag",
		"CelestialAtmosphere");

	return true;
}

bool CGMEarth::_CreateEarth_2()
{
	m_pEarthGeom_2 = MakeHexahedronSphereGeometry();
	if (!m_pEarthGeom_2.valid()) return false;

	std::string strShaderPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath;
	unsigned int iOnOverride = osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE;
	double fUnit2 = m_pKernelData->fUnitArray->at(2);
	m_pCelestialScaleVisitor->SetRadius(6378137.0 / fUnit2, 6356752.0 / fUnit2);
	// 改变大小
	m_pEarthGeom_2->accept(*m_pCelestialScaleVisitor);

	// 地球岩石地面
	m_pEarthGround_2 = new osg::Geode();
	m_pEarthGround_2->addDrawable(m_pEarthGeom_2);
	m_pEarthRoot_2->addChild(m_pEarthGround_2);

	m_pSSEarthGround_2 = m_pEarthGround_2->getOrCreateStateSet();
	m_pSSEarthGround_2->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	m_pSSEarthGround_2->setMode(GL_BLEND, osg::StateAttribute::OFF);
	m_pSSEarthGround_2->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	m_pSSEarthGround_2->setAttributeAndModes(new osg::CullFace());
	m_pSSEarthGround_2->setRenderBinDetails(BIN_ROCKSPHERE, "DepthSortedBin");
	// 地球宏定义
	m_pSSEarthGround_2->setDefine("EARTH", osg::StateAttribute::ON);
	m_pSSEarthGround_2->setDefine("ATMOS", osg::StateAttribute::ON);

	int iGroundUnit = 0;
	// 基础贴图
	m_pSSEarthGround_2->setTextureAttributeAndModes(iGroundUnit, m_aEarthBaseTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGrundBaseUniform = new osg::Uniform("baseTex", iGroundUnit++);
	m_pSSEarthGround_2->addUniform(pGrundBaseUniform);
	// 自发光贴图
	m_pSSEarthGround_2->setTextureAttributeAndModes(iGroundUnit, m_aIllumTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGrundIllumUniform = new osg::Uniform("illumTex", iGroundUnit++);
	m_pSSEarthGround_2->addUniform(pGrundIllumUniform);
	// DEM贴图
	m_pSSEarthGround_2->setTextureAttributeAndModes(iGroundUnit, m_aDEMTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pDEMUniform = new osg::Uniform("DEMTex", iGroundUnit++);
	m_pSSEarthGround_2->addUniform(pDEMUniform);
	// 全球阴影
	m_pSSEarthGround_2->setTextureAttributeAndModes(iGroundUnit, m_pGlobalShadowTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGlobalShadowUniform = new osg::Uniform("globalShadowTex", iGroundUnit++);
	m_pSSEarthGround_2->addUniform(pGlobalShadowUniform);
	// 地面上的大气“内散射”纹理
	m_pSSEarthGround_2->setTextureAttributeAndModes(iGroundUnit, m_pInscatteringTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGroundInscatteringUniform = new osg::Uniform("inscatteringTex", iGroundUnit++);
	m_pSSEarthGround_2->addUniform(pGroundInscatteringUniform);

	if (m_pConfigData->bWanderingEarth)
	{
		// 流浪地球尾迹（吹散的大气）
		m_pSSEarthGround_2->setTextureAttributeAndModes(iGroundUnit, m_pEarthTail->GetTAATex(), iOnOverride);
		osg::ref_ptr<osg::Uniform> pGroundTailUniform = new osg::Uniform("tailTex", iGroundUnit++);
		m_pSSEarthGround_2->addUniform(pGroundTailUniform);

		m_pSSEarthGround_2->addUniform(m_fWanderProgressUniform);
		m_pSSEarthGround_2->setDefine("WANDERING", osg::StateAttribute::ON);
	}

	m_pSSEarthGround_2->addUniform(m_pCommonUniform->GetViewUp());
	m_pSSEarthGround_2->addUniform(m_vViewLightUniform);
	m_pSSEarthGround_2->addUniform(m_fAtmosHeightUniform);
	m_pSSEarthGround_2->addUniform(m_fEyeAltitudeUniform);
	m_pSSEarthGround_2->addUniform(m_fGroundTopUniform);
	m_pSSEarthGround_2->addUniform(m_vPlanetRadiusUniform);
	m_pSSEarthGround_2->addUniform(m_fMinDotULUniform);
	m_pSSEarthGround_2->addUniform(m_pCommonUniform->GetScreenSize());
	m_pSSEarthGround_2->addUniform(m_vEarthCoordScaleUniform);
	m_pSSEarthGround_2->addUniform(m_pCommonUniform->GetUnit());
	m_pSSEarthGround_2->addUniform(m_fEngineStartRatioUniform);

	// 添加shader
	CGMKit::LoadShaderWithCommonFrag(m_pSSEarthGround_2,
		strShaderPath + "CelestialGround.vert",
		strShaderPath + "CelestialGround.frag",
		strShaderPath + "CelestialCommon.frag",
		"CelestialGround");

	////////////////////////////////////
	// 地球云层
	m_pEarthCloud_2 = new osg::Geode();
	m_pEarthCloud_2->addDrawable(m_pEarthGeom_2);
	m_pEarthRoot_2->addChild(m_pEarthCloud_2);

	m_pSSEarthCloud_2 = m_pEarthCloud_2->getOrCreateStateSet();
	m_pSSEarthCloud_2->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	m_pSSEarthCloud_2->setMode(GL_BLEND, osg::StateAttribute::ON);
	m_pSSEarthCloud_2->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	m_pSSEarthCloud_2->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false)); // no zbuffer
	m_pSSEarthCloud_2->setAttributeAndModes(new osg::CullFace());
	m_pSSEarthCloud_2->setRenderBinDetails(BIN_CLOUD, "DepthSortedBin");
	// 云层偏移，避免和地面打架
	osg::ref_ptr<osg::PolygonOffset> pCloudPO = new osg::PolygonOffset(-1, -1);
	m_pSSEarthCloud_2->setAttributeAndModes(pCloudPO, iOnOverride);
	m_pSSEarthCloud_2->setDefine("ATMOS", osg::StateAttribute::ON);
	// 默认地球
	m_pSSEarthCloud_2->setDefine("EARTH", osg::StateAttribute::ON);
	m_pSSEarthCloud_2->setDefine("SATURN", osg::StateAttribute::OFF);

	int iCloudUnit = 0;
	// 基础贴图
	m_pSSEarthCloud_2->setTextureAttributeAndModes(iCloudUnit, m_aEarthCloudTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pCloudBaseUniform = new osg::Uniform("cloudTex", iCloudUnit++);
	m_pSSEarthCloud_2->addUniform(pCloudBaseUniform);
	// 云的细节纹理
	m_pSSEarthCloud_2->setTextureAttributeAndModes(iCloudUnit, m_pCloudDetailTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pCloudDetailUniform = new osg::Uniform("cloudDetailTex", iCloudUnit++);
	m_pSSEarthCloud_2->addUniform(pCloudDetailUniform);
	// 云层上的“内散射”纹理
	m_pSSEarthCloud_2->setTextureAttributeAndModes(iCloudUnit, m_pInscatteringTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pCloudInscatteringUniform = new osg::Uniform("inscatteringTex", iCloudUnit++);
	m_pSSEarthCloud_2->addUniform(pCloudInscatteringUniform);

	if (m_pConfigData->bWanderingEarth)
	{
		// 流浪地球尾迹（吹散的大气）
		m_pSSEarthCloud_2->setTextureAttributeAndModes(iCloudUnit, m_pEarthTail->GetTAATex(), iOnOverride);
		osg::ref_ptr<osg::Uniform> pCloudTailUniform = new osg::Uniform("tailTex", iCloudUnit++);
		m_pSSEarthCloud_2->addUniform(pCloudTailUniform);

		m_pSSEarthCloud_2->setTextureAttributeAndModes(iCloudUnit, m_aIllumTex, iOnOverride);
		osg::ref_ptr<osg::Uniform> pCloudIllumUniform = new osg::Uniform("illumTex", iCloudUnit++);
		m_pSSEarthCloud_2->addUniform(pCloudIllumUniform);

		m_pSSEarthCloud_2->addUniform(m_fWanderProgressUniform);
		m_pSSEarthCloud_2->setDefine("WANDERING", osg::StateAttribute::ON);
	}

	m_pSSEarthCloud_2->addUniform(m_pCommonUniform->GetViewUp());
	m_pSSEarthCloud_2->addUniform(m_vViewLightUniform);
	m_pSSEarthCloud_2->addUniform(m_fCloudTopUniform);
	m_pSSEarthCloud_2->addUniform(m_fGroundTopUniform);
	m_pSSEarthCloud_2->addUniform(m_fEyeAltitudeUniform);
	m_pSSEarthCloud_2->addUniform(m_fAtmosHeightUniform);
	m_pSSEarthCloud_2->addUniform(m_vPlanetRadiusUniform);
	m_pSSEarthCloud_2->addUniform(m_fMinDotULUniform);
	m_pSSEarthCloud_2->addUniform(m_pCommonUniform->GetScreenSize());
	m_pSSEarthCloud_2->addUniform(m_vEarthCoordScaleUniform);
	m_pSSEarthCloud_2->addUniform(m_pCommonUniform->GetUnit());
	m_pSSEarthCloud_2->addUniform(m_fEngineStartRatioUniform);

	// 添加shader
	CGMKit::LoadShaderWithCommonFrag(m_pSSEarthCloud_2,
		strShaderPath + "CelestialCloud.vert",
		strShaderPath + "CelestialCloud.frag",
		strShaderPath + "CelestialCommon.frag",
		"CelestialCloud");

	////////////////////////////////////
	// 地球大气
	m_pEarthAtmos_2 = new osg::Geode();
	m_pEarthAtmos_2->addDrawable(m_pEarthGeom_2);
	m_pEarthRoot_2->addChild(m_pEarthAtmos_2);

	m_pSSEarthAtmos_2 = m_pEarthAtmos_2->getOrCreateStateSet();
	m_pSSEarthAtmos_2->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	m_pSSEarthAtmos_2->setMode(GL_BLEND, osg::StateAttribute::ON);
	m_pSSEarthAtmos_2->setAttributeAndModes(new osg::BlendFunc(
		GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_ALPHA, GL_ONE), osg::StateAttribute::ON);
	m_pSSEarthAtmos_2->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false)); // no zbuffer
	m_pSSEarthAtmos_2->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT));
	m_pSSEarthAtmos_2->setRenderBinDetails(BIN_ATMOSPHERE, "DepthSortedBin");
	m_pSSEarthAtmos_2->setDefine("EARTH", osg::StateAttribute::ON);
	m_pSSEarthAtmos_2->setDefine("SATURN", osg::StateAttribute::OFF);

	int iAtmosUnit = 0;
	// “内散射”贴图
	m_pSSEarthAtmos_2->setTextureAttributeAndModes(iAtmosUnit, m_pInscatteringTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pInscatteringUniform = new osg::Uniform("inscatteringTex", iAtmosUnit++);
	m_pSSEarthAtmos_2->addUniform(pInscatteringUniform);

	m_pSSEarthAtmos_2->addUniform(m_pCommonUniform->GetViewUp());
	m_pSSEarthAtmos_2->addUniform(m_vViewLightUniform);
	m_pSSEarthAtmos_2->addUniform(m_fAtmosHeightUniform);
	m_pSSEarthAtmos_2->addUniform(m_fEyeAltitudeUniform);
	m_pSSEarthAtmos_2->addUniform(m_vPlanetRadiusUniform);
	m_pSSEarthAtmos_2->addUniform(m_fMinDotULUniform);
	m_pSSEarthAtmos_2->addUniform(m_mView2ECEFUniform);

	if (m_pConfigData->bWanderingEarth)
	{
		m_pSSEarthAtmos_2->addUniform(m_fWanderProgressUniform);
		m_pSSEarthAtmos_2->setDefine("WANDERING", osg::StateAttribute::ON);
	}

	// 添加shader
	CGMKit::LoadShader(m_pSSEarthAtmos_2,
		strShaderPath + "CelestialAtmosphere.vert",
		strShaderPath + "CelestialAtmosphere.frag",
		"CelestialAtmosphere");

	return true;
}

bool CGMEarth::_CreateWanderingEarth()
{
	// 临时添加的生成“行星发动机数据”的工具函数
	//_GenEarthEngineData();
	// 临时添加的生成“行星发动机分布图”和“周围bloom图”的工具函数
	//_GenEarthEngineTexture();
	
	// 临时添加的生成流浪地球版本的各个贴图的工具函数
	//std::string strPath_0 = m_pConfigData->strCorePath + "Textures/Sphere/Earth/wanderingEarth_base_real_";
	//std::string strPath_1 = m_pConfigData->strCorePath + "Textures/Sphere/Earth/engineBody";
	//std::string strOut = m_pConfigData->strCorePath + "Textures/Sphere/Earth/wanderingEarth_base_";
	//_MixWEETexture(strPath_0, strPath_1, strOut, 0);
	//strPath_0 = m_pConfigData->strCorePath + "Textures/Sphere/Earth/wanderingEarth_cloud_real_";
	//strPath_1 = m_pConfigData->strCorePath + "Textures/Sphere/Earth/bloom";
	//strOut = m_pConfigData->strCorePath + "Textures/Sphere/Earth/wanderingEarth_cloud_";
	//_MixWEETexture(strPath_0, strPath_1, strOut, 1);
	//strPath_0 = m_pConfigData->strCorePath + "Textures/Sphere/Earth/Earth_illum_real_";
	//strPath_1 = m_pConfigData->strCorePath + "Textures/Sphere/Earth/bloom";
	//strOut = m_pConfigData->strCorePath + "Textures/Sphere/Earth/Earth_illum_";
	//_MixWEETexture(strPath_0, strPath_1, strOut, 2);

	_GenEarthEnginePoint_1();
	_GenEarthEngineJetLine_1();
	_GenEarthEnginePoint_2();
	_GenEarthEngineJetLine_2();
	_GenEarthEngineBody_1();
	_GenEarthEngineBody_2();

	_GenEarthEngineStream();

	if ((m_pConfigData->bWanderingEarth) && (EGMRENDER_LOW != m_pConfigData->eRenderQuality))
	{
		m_pEarthTail->MakeEarthTail();
	}

	return true;
}

bool CGMEarth::_GenEarthEnginePoint_1()
{
	// 流浪地球上的行星发动机的喷射口亮点
	double fUnit = m_pKernelData->fUnitArray->at(1);
	m_pEllipsoid->setRadiusEquator(osg::WGS_84_RADIUS_EQUATOR / fUnit);
	m_pEllipsoid->setRadiusPolar(osg::WGS_84_RADIUS_POLAR / fUnit);
	m_pEarthEnginePointNode_1 = new osg::Geode();
	m_pEarthEnginePointNode_1->addDrawable(_MakeEnginePointGeometry(m_pEllipsoid, fUnit));
	m_pEarthRoot_1->addChild(m_pEarthEnginePointNode_1);

	osg::ref_ptr<osg::StateSet> pSSPlanetEnginePoint = m_pEarthEnginePointNode_1->getOrCreateStateSet();
	pSSPlanetEnginePoint->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
	pSSPlanetEnginePoint->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	pSSPlanetEnginePoint->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSPlanetEnginePoint->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSPlanetEnginePoint->setAttributeAndModes(new osg::BlendFunc(
		GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_ALPHA, GL_ONE), osg::StateAttribute::ON);
	pSSPlanetEnginePoint->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false)); // no zbuffer
	// 喷射亮点偏移，避免和云层打架
	osg::ref_ptr<osg::PolygonOffset> pPO = new osg::PolygonOffset(-1.5, -1.5);
	pSSPlanetEnginePoint->setAttributeAndModes(pPO, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	pSSPlanetEnginePoint->setRenderBinDetails(BIN_PLANET_POINT, "DepthSortedBin");
	pSSPlanetEnginePoint->addUniform(m_pCommonUniform->GetScreenSize());
	pSSPlanetEnginePoint->addUniform(m_pCommonUniform->GetUnit());
	pSSPlanetEnginePoint->addUniform(m_fEngineStartRatioUniform);

	// 流浪地球尾迹（吹散的大气）
	pSSPlanetEnginePoint->setTextureAttributeAndModes(0, m_pEarthTail->GetTAATex(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	osg::ref_ptr<osg::Uniform> pTailUniform = new osg::Uniform("tailTex", 0);
	pSSPlanetEnginePoint->addUniform(pTailUniform);

	std::string strVertPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEnginePoint.vert";
	std::string strFragPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEnginePoint.frag";
	CGMKit::LoadShader(pSSPlanetEnginePoint, strVertPath, strFragPath, "PlanetEnginePoint_1");

	return true;
}

bool CGMEarth::_GenEarthEnginePoint_2()
{
	// 流浪地球上的行星发动机的喷射口亮点
	double fUnit = m_pKernelData->fUnitArray->at(2);
	m_pEllipsoid->setRadiusEquator(osg::WGS_84_RADIUS_EQUATOR / fUnit);
	m_pEllipsoid->setRadiusPolar(osg::WGS_84_RADIUS_POLAR / fUnit);
	m_pEarthEnginePointNode_2 = new osg::Geode();
	m_pEarthEnginePointNode_2->addDrawable(_MakeEnginePointGeometry(m_pEllipsoid, fUnit));
	m_pEarthRoot_2->addChild(m_pEarthEnginePointNode_2);

	osg::ref_ptr<osg::StateSet> pSSPlanetEnginePoint = m_pEarthEnginePointNode_2->getOrCreateStateSet();
	pSSPlanetEnginePoint->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
	pSSPlanetEnginePoint->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	pSSPlanetEnginePoint->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSPlanetEnginePoint->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSPlanetEnginePoint->setAttributeAndModes(new osg::BlendFunc(
		GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pSSPlanetEnginePoint->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false)); // no zbuffer
	// 喷射亮点偏移，避免和云层打架
	osg::ref_ptr<osg::PolygonOffset> pPO = new osg::PolygonOffset(-1.5, -1.5);
	pSSPlanetEnginePoint->setAttributeAndModes(pPO, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	pSSPlanetEnginePoint->setRenderBinDetails(BIN_PLANET_POINT, "DepthSortedBin");
	pSSPlanetEnginePoint->addUniform(m_pCommonUniform->GetScreenSize());
	pSSPlanetEnginePoint->addUniform(m_pCommonUniform->GetUnit());
	pSSPlanetEnginePoint->addUniform(m_fEngineStartRatioUniform);

	// 流浪地球尾迹（吹散的大气）
	pSSPlanetEnginePoint->setTextureAttributeAndModes(0, m_pEarthTail->GetTAATex(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	osg::ref_ptr<osg::Uniform> pTailUniform = new osg::Uniform("tailTex", 0);
	pSSPlanetEnginePoint->addUniform(pTailUniform);

	std::string strVertPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEnginePoint.vert";
	std::string strFragPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEnginePoint.frag";
	CGMKit::LoadShader(pSSPlanetEnginePoint, strVertPath, strFragPath, "PlanetEnginePoint_2");

	return true;
}

bool CGMEarth::_GenEarthEngineJetLine_1()
{
	// 流浪地球上的行星发动机的喷射流
	double fUnit = m_pKernelData->fUnitArray->at(1);
	m_pEllipsoid->setRadiusEquator(osg::WGS_84_RADIUS_EQUATOR / fUnit);
	m_pEllipsoid->setRadiusPolar(osg::WGS_84_RADIUS_POLAR / fUnit);
	m_pEarthEngineJetNode_1 = new osg::Geode();
	m_pEarthEngineJetNode_1->addDrawable(_MakeEngineJetLineGeometry(m_pEllipsoid, fUnit));
	m_pEarthRoot_1->addChild(m_pEarthEngineJetNode_1);

	osg::ref_ptr<osg::LineWidth> pLineWidth = new osg::LineWidth;
	pLineWidth->setWidth(2);
	osg::ref_ptr<osg::StateSet> pSSPlanetEngineJet = m_pEarthEngineJetNode_1->getOrCreateStateSet();
	pSSPlanetEngineJet->setAttributeAndModes(pLineWidth, osg::StateAttribute::ON);
	pSSPlanetEngineJet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSPlanetEngineJet->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSPlanetEngineJet->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pSSPlanetEngineJet->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false)); // no zbuffer
	// 喷射流偏移，避免和云层打架
	osg::ref_ptr<osg::PolygonOffset> pPO = new osg::PolygonOffset(-1.5, -1.5);
	pSSPlanetEngineJet->setAttributeAndModes(pPO, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	pSSPlanetEngineJet->setRenderBinDetails(BIN_PLANET_JET, "DepthSortedBin");
	pSSPlanetEngineJet->addUniform(m_pCommonUniform->GetTime());
	pSSPlanetEngineJet->addUniform(m_pCommonUniform->GetScreenSize());
	pSSPlanetEngineJet->addUniform(m_pCommonUniform->GetUnit());
	pSSPlanetEngineJet->addUniform(m_fEngineStartRatioUniform);

	// 流浪地球尾迹（吹散的大气）
	pSSPlanetEngineJet->setTextureAttributeAndModes(0, m_pEarthTail->GetTAATex(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	osg::ref_ptr<osg::Uniform> pTailUniform = new osg::Uniform("tailTex", 0);
	pSSPlanetEngineJet->addUniform(pTailUniform);

	std::string strJetVertPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEngineJet.vert";
	std::string strJetFragPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEngineJet.frag";
	CGMKit::LoadShader(pSSPlanetEngineJet, strJetVertPath, strJetFragPath, "PlanetEngineJet_1");

	return true;
}

bool CGMEarth::_GenEarthEngineJetLine_2()
{
	// 流浪地球上的行星发动机的喷射流
	double fUnit = m_pKernelData->fUnitArray->at(2);
	m_pEllipsoid->setRadiusEquator(osg::WGS_84_RADIUS_EQUATOR / fUnit);
	m_pEllipsoid->setRadiusPolar(osg::WGS_84_RADIUS_POLAR / fUnit);
	m_pEarthEngineJetNode_2 = new osg::Geode();
	m_pEarthEngineJetNode_2->addDrawable(_MakeEngineJetLineGeometry(m_pEllipsoid, fUnit));
	m_pEarthRoot_2->addChild(m_pEarthEngineJetNode_2);

	osg::ref_ptr<osg::LineWidth> pLineWidth = new osg::LineWidth;
	pLineWidth->setWidth(1);
	osg::ref_ptr<osg::StateSet> pSSPlanetEngineJet = m_pEarthEngineJetNode_2->getOrCreateStateSet();
	pSSPlanetEngineJet->setAttributeAndModes(pLineWidth, osg::StateAttribute::ON);
	pSSPlanetEngineJet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSPlanetEngineJet->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSPlanetEngineJet->setAttributeAndModes(new osg::BlendFunc(
		GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pSSPlanetEngineJet->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false)); // no zbuffer
	// 喷射流偏移，避免和云层打架
	osg::ref_ptr<osg::PolygonOffset> pPO = new osg::PolygonOffset(-1.5, -1.5);
	pSSPlanetEngineJet->setAttributeAndModes(pPO, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	pSSPlanetEngineJet->setRenderBinDetails(BIN_PLANET_JET, "DepthSortedBin");
	pSSPlanetEngineJet->addUniform(m_pCommonUniform->GetTime());
	pSSPlanetEngineJet->addUniform(m_pCommonUniform->GetScreenSize());
	pSSPlanetEngineJet->addUniform(m_pCommonUniform->GetUnit());
	pSSPlanetEngineJet->addUniform(m_fEngineStartRatioUniform);

	// 流浪地球尾迹（吹散的大气）
	pSSPlanetEngineJet->setTextureAttributeAndModes(0, m_pEarthTail->GetTAATex(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	osg::ref_ptr<osg::Uniform> pTailUniform = new osg::Uniform("tailTex", 0);
	pSSPlanetEngineJet->addUniform(pTailUniform);

	std::string strJetVertPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEngineJet.vert";
	std::string strJetFragPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEngineJet.frag";
	CGMKit::LoadShader(pSSPlanetEngineJet, strJetVertPath, strJetFragPath, "PlanetEngineJet_2");

	return true;
}

bool CGMEarth::_GenEarthEngineBody_1()
{
	m_pEarthEngineBody_1 = osgDB::readNodeFile(m_pConfigData->strCorePath + m_strCoreModelPath + "theWanderingEarth_engine_LOD5.ive");
	if (!m_pEarthEngineBody_1.valid()) return false;
	m_pEarthRoot_1->addChild(m_pEarthEngineBody_1);

	CGenEngineBodyVisitor cBodyVisitor(m_pEarthEngineDataImg, m_pKernelData->fUnitArray->at(1));
	m_pEarthEngineBody_1->accept(cBodyVisitor);

	osg::ref_ptr<osg::StateSet> pSSEngineBody = m_pEarthEngineBody_1->getOrCreateStateSet();
	pSSEngineBody->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSEngineBody->setMode(GL_BLEND, osg::StateAttribute::OFF);
	pSSEngineBody->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	pSSEngineBody->setMode(GL_ALPHA_TEST, osg::StateAttribute::ON);
	osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
	alphaFunc->setFunction(osg::AlphaFunc::GEQUAL, 0.5f);
	pSSEngineBody->setAttributeAndModes(alphaFunc, osg::StateAttribute::ON);
	pSSEngineBody->setRenderBinDetails(BIN_ROCKSPHERE, "RenderBin");
	pSSEngineBody->setDefine("ATMOS", osg::StateAttribute::ON);
	pSSEngineBody->setDefine("EARTH", osg::StateAttribute::ON);

	pSSEngineBody->addUniform(m_pCommonUniform->GetUnit());
	pSSEngineBody->addUniform(m_pCommonUniform->GetScreenSize());
	pSSEngineBody->addUniform(m_pCommonUniform->GetViewUp());
	pSSEngineBody->addUniform(m_vViewLightUniform);
	pSSEngineBody->addUniform(m_fMinDotULUniform);
	pSSEngineBody->addUniform(m_fGroundTopUniform);
	pSSEngineBody->addUniform(m_fEyeAltitudeUniform);
	pSSEngineBody->addUniform(m_fAtmosHeightUniform);
	pSSEngineBody->addUniform(m_fEngineStartRatioUniform);

	int iTexUnit = 0;
	// base color贴图
	osg::ref_ptr<osg::Uniform> pBaseColorTexUniform = new osg::Uniform("baseColorTex", iTexUnit++);
	pSSEngineBody->addUniform(pBaseColorTexUniform);
	// 大气“内散射”纹理
	pSSEngineBody->setTextureAttributeAndModes(iTexUnit, m_pInscatteringTex, osg::StateAttribute::ON);
	osg::ref_ptr<osg::Uniform> pInscatteringUniform = new osg::Uniform("inscatteringTex", iTexUnit++);
	pSSEngineBody->addUniform(pInscatteringUniform);
	// 流浪地球尾迹（吹散的大气）
	pSSEngineBody->setTextureAttributeAndModes(iTexUnit, m_pEarthTail->GetTAATex(), osg::StateAttribute::ON);
	osg::ref_ptr<osg::Uniform> pTailUniform = new osg::Uniform("tailTex", iTexUnit++);
	pSSEngineBody->addUniform(pTailUniform);

	std::string strEarthShaderPath = m_pConfigData->strCorePath + m_strEarthShaderPath;
	std::string strGalaxyShaderPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath;
	CGMKit::LoadShaderWithCommonFrag(pSSEngineBody,
		strEarthShaderPath + "PlanetEngineBody.vert",
		strEarthShaderPath + "PlanetEngineBody.frag",
		strGalaxyShaderPath + "CelestialCommon.frag",
		"PlanetEngineBody_1");

	return true;
}

bool CGMEarth::_GenEarthEngineBody_2()
{
	m_pEarthEngineBody_2 = osgDB::readNodeFile(m_pConfigData->strCorePath + m_strCoreModelPath + "theWanderingEarth_engine_LOD5.ive");
	if (!m_pEarthEngineBody_2.valid()) return false;
	m_pEarthRoot_2->addChild(m_pEarthEngineBody_2);

	CGenEngineBodyVisitor cBodyVisitor(m_pEarthEngineDataImg, m_pKernelData->fUnitArray->at(2));
	m_pEarthEngineBody_2->accept(cBodyVisitor);

	osg::ref_ptr<osg::StateSet> pSSEngineBody = m_pEarthEngineBody_2->getOrCreateStateSet();
	pSSEngineBody->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSEngineBody->setMode(GL_BLEND, osg::StateAttribute::OFF);
	pSSEngineBody->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	pSSEngineBody->setMode(GL_ALPHA_TEST, osg::StateAttribute::ON);
	osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
	alphaFunc->setFunction(osg::AlphaFunc::GEQUAL, 0.5f);
	pSSEngineBody->setAttributeAndModes(alphaFunc, osg::StateAttribute::ON);
	pSSEngineBody->setRenderBinDetails(BIN_ROCKSPHERE, "RenderBin");
	pSSEngineBody->setDefine("ATMOS", osg::StateAttribute::ON);
	pSSEngineBody->setDefine("EARTH", osg::StateAttribute::ON);

	pSSEngineBody->addUniform(m_pCommonUniform->GetUnit());
	pSSEngineBody->addUniform(m_pCommonUniform->GetScreenSize());
	pSSEngineBody->addUniform(m_pCommonUniform->GetViewUp());
	pSSEngineBody->addUniform(m_vViewLightUniform);
	pSSEngineBody->addUniform(m_fMinDotULUniform);
	pSSEngineBody->addUniform(m_fGroundTopUniform);
	pSSEngineBody->addUniform(m_fEyeAltitudeUniform);
	pSSEngineBody->addUniform(m_fAtmosHeightUniform);
	pSSEngineBody->addUniform(m_fEngineStartRatioUniform);

	int iTexUnit = 0;
	// base color 贴图
	osg::ref_ptr<osg::Uniform> pBaseColorTexUniform = new osg::Uniform("baseColorTex", iTexUnit++);
	pSSEngineBody->addUniform(pBaseColorTexUniform);
	// 大气“内散射”纹理
	pSSEngineBody->setTextureAttributeAndModes(iTexUnit, m_pInscatteringTex, osg::StateAttribute::ON);
	osg::ref_ptr<osg::Uniform> pInscatteringUniform = new osg::Uniform("inscatteringTex", iTexUnit++);
	pSSEngineBody->addUniform(pInscatteringUniform);
	// 流浪地球尾迹（吹散的大气）
	pSSEngineBody->setTextureAttributeAndModes(iTexUnit, m_pEarthTail->GetTAATex(), osg::StateAttribute::ON);
	osg::ref_ptr<osg::Uniform> pTailUniform = new osg::Uniform("tailTex", iTexUnit++);
	pSSEngineBody->addUniform(pTailUniform);

	std::string strEarthShaderPath = m_pConfigData->strCorePath + m_strEarthShaderPath;
	std::string strGalaxyShaderPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath;
	CGMKit::LoadShaderWithCommonFrag(pSSEngineBody,
		strEarthShaderPath + "PlanetEngineBody.vert",
		strEarthShaderPath + "PlanetEngineBody.frag",
		strGalaxyShaderPath + "CelestialCommon.frag",
		"PlanetEngineBody_2");

	return true;
}

bool CGMEarth::_GenEarthEngineStream()
{
	// 流浪地球上的行星发动机的喷射流，用于近地视角
	double fUnit = m_pKernelData->fUnitArray->at(1);
	m_pEllipsoid->setRadiusEquator(osg::WGS_84_RADIUS_EQUATOR / fUnit);
	m_pEllipsoid->setRadiusPolar(osg::WGS_84_RADIUS_POLAR / fUnit);
	m_pEarthEngineStream = new osg::Geode();
	m_pEarthEngineStream->addDrawable(_MakeEngineJetStreamGeometry(m_pEllipsoid, fUnit));
	m_pEarthRoot_1->addChild(m_pEarthEngineStream);

	osg::ref_ptr<osg::StateSet> pSSEngineStream = m_pEarthEngineStream->getOrCreateStateSet();
	pSSEngineStream->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSEngineStream->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSEngineStream->setAttributeAndModes(new osg::BlendFunc(
		GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_ALPHA, GL_ONE), osg::StateAttribute::ON);
	pSSEngineStream->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false)); // no zbuffer
	pSSEngineStream->setRenderBinDetails(BIN_PLANET_JET, "DepthSortedBin"); // to do
	pSSEngineStream->addUniform(m_pCommonUniform->GetTime());
	pSSEngineStream->addUniform(m_pCommonUniform->GetUnit());
	pSSEngineStream->addUniform(m_fEngineStartRatioUniform);

	// 喷射流噪声贴图
	pSSEngineStream->setTextureAttributeAndModes(0,
		_CreateTexture2D(m_pConfigData->strCorePath + "Textures/Volume/BlueNoise.jpg", 1),
		osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	osg::ref_ptr<osg::Uniform> pBlueNoiseUniform = new osg::Uniform("blueNoiseTex", 0);
	pSSEngineStream->addUniform(pBlueNoiseUniform);

	std::string strShaderPath = m_pConfigData->strCorePath + m_strEarthShaderPath;
	CGMKit::LoadShader(pSSEngineStream,
		strShaderPath + "PlanetEngineJetStream.vert",
		strShaderPath + "PlanetEngineJetStream.frag",
		"PlanetEngineJetStream");

	return true;
}

osg::Texture* CGMEarth::_CreateTexture2D(const std::string & fileName, const int iChannelNum) const
{
	return _CreateTexture2D(osgDB::readImageFile(fileName), iChannelNum);
}

osg::Texture* CGMEarth::_CreateTexture2D(osg::Image * pImg, const int iChannelNum) const
{
	if (!pImg) return nullptr;

	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	texture->setImage(pImg);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	switch (iChannelNum)
	{
	case 1:
	{
		texture->setInternalFormat(GL_R8);
		texture->setSourceFormat(GL_RED);
	}
	break;
	case 2:
	{
		texture->setInternalFormat(GL_RG8);
		texture->setSourceFormat(GL_RG);
	}
	break;
	case 3:
	{
		texture->setInternalFormat(GL_RGB8);
		texture->setSourceFormat(GL_RGB);
	}
	break;
	case 4:
	{
		texture->setInternalFormat(GL_RGBA8);
		texture->setSourceFormat(GL_RGBA);
	}
	break;
	default:
	{
		texture->setInternalFormat(GL_RGBA8);
		texture->setSourceFormat(GL_RGBA);
	}
	}

	texture->setSourceType(GL_UNSIGNED_BYTE);
	return texture.release();
}

osg::Texture* CGMEarth::_CreateDDSTexture(const std::string& fileName,
	osg::Texture::WrapMode eWrap_S, osg::Texture::WrapMode eWrap_T, bool bFlip) const
{
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	if (bFlip)
	{
		texture->setImage(osgDB::readImageFile(fileName, m_pDDSOptions));
	}
	else
	{
		texture->setImage(osgDB::readImageFile(fileName));
	}
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, eWrap_S);
	texture->setWrap(osg::Texture::WRAP_T, eWrap_T);
	texture->setBorderColor(osg::Vec4(0,0,0,0));
	texture->setSourceType(GL_UNSIGNED_BYTE);
	return texture.release();
}

osg::Texture2DArray* CGMEarth::_CreateDDSTex2DArray(const std::string& filePreName, bool bFlip) const
{
	osg::Image* pImg = osgDB::readImageFile(filePreName + "0.dds");
	if (!pImg) return nullptr;

	osg::ref_ptr<osg::Texture2DArray> texture = new osg::Texture2DArray;
	texture->setTextureSize(pImg->s(), pImg->t(), 6);
	for (int i = 0; i < 6; i++)
	{
		std::string fileName = filePreName + std::to_string(i) + ".dds";
		if (bFlip)
		{
			texture->setImage(i, osgDB::readImageFile(fileName, m_pDDSOptions));
		}
		else
		{
			texture->setImage(i, osgDB::readImageFile(fileName));
		}
	}

	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
	texture->setBorderColor(osg::Vec4(0, 0, 0, 0));
	texture->setSourceType(GL_UNSIGNED_BYTE);
	return texture.release();
}

bool CGMEarth::_AddTex2DArray(osg::Texture2DArray* pTex, const std::string& filePreName, bool bFlip)
{
	if(!pTex) return false;
	osg::Image* pImg = osgDB::readImageFile(filePreName + "0.dds");
	if (!pImg) return false;

	if (pTex->getTextureWidth() != pImg->s()) return false;
	if (pTex->getTextureHeight() != pImg->t()) return false;
	pTex->setTextureDepth(pTex->getTextureDepth()+6);

	for (int i = 0; i < 6; i++)
	{
		std::string fileName = filePreName + std::to_string(i) + ".dds";
		if (bFlip)
		{
			pTex->setImage(6+i, osgDB::readImageFile(fileName, m_pDDSOptions));
		}
		else
		{
			pTex->setImage(6+i, osgDB::readImageFile(fileName));
		}
	}

	return true;
}

void CGMEarth::_GenEarthEngineData()
{
	osg::ref_ptr<osg::Node> pNode = osgDB::readNodeFile("D:/GMHelp/max/sphere/theWanderingEarth/theWanderingEarthEngineLocation.ive");
	if (!pNode.valid()) return;

	CGenEngineDataVisitor cGenDataVisitor;
	pNode->accept(cGenDataVisitor);
}

void CGMEarth::_GenEarthEngineTexture()
{
	osg::ref_ptr<osg::Geode> pGeode = new osg::Geode();
	double fUnit = m_pKernelData->fUnitArray->at(2);
	// 要保证这里是个球体，不能是椭球
	m_pEllipsoid->setRadiusEquator(6.37e6 / fUnit);
	m_pEllipsoid->setRadiusPolar(6.37e6 / fUnit);
	pGeode->addDrawable(_MakeEnginePointGeometry(m_pEllipsoid, fUnit));

	int iH = 2048;
	int iCharSize = iH * iH * 4;
	for (int i = 0; i < 5; i++)
	{
		osg::ref_ptr<osg::Image> pEngineBodyImage = new osg::Image();
		osg::ref_ptr<osg::Image> pBloomImage = new osg::Image();
		unsigned char* pEngineData = new unsigned char[iCharSize];
		unsigned char* pBloomData = new unsigned char[iCharSize];
		for (int j = 0; j < iCharSize; j++)
		{
			pEngineData[j] = 0;
			pBloomData[j] = 0;
		}
		pEngineBodyImage->setImage(iH, iH, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, pEngineData, osg::Image::USE_NEW_DELETE);
		pBloomImage->setImage(iH, iH, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, pBloomData, osg::Image::USE_NEW_DELETE);

		osg::Vec3 vCenter = osg::Vec3(0, 1, 0);
		osg::Vec3 vUp = osg::Vec3(0, 0, 1);
		switch (i)
		{
		case 0:
		{
			// posX
			vCenter = osg::Vec3(1, 0, 0);
			vUp = osg::Vec3(0, 0, 1);
		}
		break;
		case 1:
		{
			// negX
			vCenter = osg::Vec3(-1, 0, 0);
			vUp = osg::Vec3(0, 0, 1);
		}
		break;
		case 2:
		{
			// posY
			vCenter = osg::Vec3(0, 1, 0);
			vUp = osg::Vec3(0, 0, 1);
		}
		break;
		case 3:
		{
			// negY
			vCenter = osg::Vec3(0, -1, 0);
			vUp = osg::Vec3(0, 0, 1);
		}
		break;
		case 4:
		{
			// posZ
			vCenter = osg::Vec3(0, 0, 1);
			vUp = osg::Vec3(-1, 0, 0);
		}
		break;
		case 5:
		{
			// negZ
			vCenter = osg::Vec3(0, 0, -1);
			vUp = osg::Vec3(1, 0, 0);
		}
		break;
		default:
			break;
		}

		osg::ref_ptr<osg::Camera> pCamera = new osg::Camera;
		pCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		pCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		pCamera->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
		pCamera->setViewport(0, 0, iH, iH);
		pCamera->setRenderOrder(osg::Camera::PRE_RENDER);
		pCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
		pCamera->attach(osg::Camera::COLOR_BUFFER0, pEngineBodyImage);
		pCamera->attach(osg::Camera::COLOR_BUFFER1, pBloomImage);
		pCamera->setAllowEventFocus(false);
		pCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
		pCamera->setViewMatrixAsLookAt(osg::Vec3(0,0,0), vCenter, vUp);
		// 需要考虑一个像素的过渡边缘
		pCamera->setProjectionMatrixAsPerspective(2 * osg::RadiansToDegrees(atan(1024.0 / 1023.0)), 1, 1e-4, 1e-3);
		pCamera->setProjectionResizePolicy(osg::Camera::FIXED);
		pCamera->addChild(pGeode);

		CRTTFinishCallback* pRTTFinishCallback = new CRTTFinishCallback(pEngineBodyImage, pBloomImage, i);
		pCamera->setFinalDrawCallback(pRTTFinishCallback);
		GM_Root->addChild(pCamera);

		osg::ref_ptr<osg::StateSet> pSS = pCamera->getOrCreateStateSet();
		pSS->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
		pSS->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
		pSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pSS->setMode(GL_BLEND, osg::StateAttribute::ON);
		pSS->setAttributeAndModes(new osg::BlendFunc(
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
		), osg::StateAttribute::ON);
		pSS->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer

		osg::ref_ptr<osg::Texture2D> pEngineTex = new osg::Texture2D;
		pEngineTex->setImage(osgDB::readImageFile(m_pConfigData->strCorePath + "Textures/Sphere/Earth/EarthEngine.tga"));
		pEngineTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		pEngineTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
		pEngineTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
		pEngineTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
		pEngineTex->setBorderColor(osg::Vec4(0, 0, 0, 0));
		pEngineTex->setInternalFormat(GL_RGBA8);
		pEngineTex->setSourceFormat(GL_RGBA);
		pEngineTex->setSourceType(GL_UNSIGNED_BYTE);

		int iUnit = 0;
		CGMKit::AddTexture(pSS, pEngineTex, "engineTex", iUnit++);

		std::string strVertPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEngineRTT.vert";
		std::string strFragPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEngineRTT.frag";
		CGMKit::LoadShader(pSS, strVertPath, strFragPath, "PlanetEngineRTT");
	}
}

osg::Geometry* CGMEarth::_MakeEnginePointGeometry(const osg::EllipsoidModel* pEllipsoid, const double fUnit) const
{
	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);
	geom->setUseDisplayList(false);
	geom->setDataVariance(osg::Object::STATIC);

	int iEngineNum = m_pEarthEngineDataImg->s();
	osg::ref_ptr<osg::Vec3Array> pVerts = new osg::Vec3Array();
	osg::ref_ptr<osg::Vec2Array> pCoords = new osg::Vec2Array;
	osg::ref_ptr<osg::Vec3Array> pNorms = new osg::Vec3Array;
	osg::ref_ptr<osg::DrawElementsUShort> pEle = new osg::DrawElementsUShort(GL_POINTS);
	pVerts->reserve(iEngineNum);
	pCoords->reserve(iEngineNum);
	pNorms->reserve(iEngineNum);
	pEle->reserve(iEngineNum);

	std::default_random_engine iRandom;
	iRandom.seed(0);
	std::uniform_int_distribution<> iPseudoNoise(0, 10000);

	for (int i = 0; i < iEngineNum; i++)
	{
		osg::Vec4f vData = CGMKit::GetImageColor(m_pEarthEngineDataImg, float(i+0.5)/float(iEngineNum), 0);
		double fLon = vData.x();
		double fLat = vData.y();
		double fTopAlt = (vData.z() + vData.w()) / fUnit;
		double fX, fY, fZ;
		pEllipsoid->convertLatLongHeightToXYZ(fLat, fLon, fTopAlt, fX, fY, fZ);
		osg::Vec3 vSpherePos = osg::Vec3(fX, fY, fZ);
		double fRandom = iPseudoNoise(iRandom) * 1e-4; // 0.0-1.0
		osg::Vec3 vSphereUp = vSpherePos;
		vSphereUp.normalize();
		if (vSphereUp.z() < 0.1)
		{
			fRandom = 0.5 + 0.5 * fRandom;
		}

		// 计算发动机底座直径, 单位：像素
		float fDiameter = (vData.w() / 11000) * 2048 * (3e4 / 6.36e6) / osg::PI_2;
		// 计算发动机喷射方向
		osg::Vec3 vDir = _Pos2Norm(vSpherePos);
		// 计算发动机喷射口位置
		osg::Vec3 vPos = vSpherePos + _NozzlePos(vDir, vSphereUp) / fUnit;
		pVerts->push_back(vPos);
		pCoords->push_back(osg::Vec2(fRandom, fDiameter));
		pNorms->push_back(vDir);
		pEle->push_back(i);
	}

	geom->setVertexArray(pVerts);
	geom->setTexCoordArray(0, pCoords);
	geom->setNormalArray(pNorms);
	geom->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);
	geom->addPrimitiveSet(pEle);
	return geom;
}

osg::Geometry* CGMEarth::_MakeEngineJetLineGeometry(const osg::EllipsoidModel* pEllipsoid, const double fUnit) const
{
	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);
	geom->setUseDisplayList(false);
	geom->setDataVariance(osg::Object::STATIC);

	int iEngineNum = m_pEarthEngineDataImg->s();
	osg::ref_ptr<osg::Vec3Array> pVerts = new osg::Vec3Array();
	osg::ref_ptr<osg::Vec2Array> pCoords = new osg::Vec2Array;
	osg::ref_ptr<osg::Vec3Array> pNorms = new osg::Vec3Array;
	osg::ref_ptr<osg::DrawElementsUShort> pEle = new osg::DrawElementsUShort(GL_LINES);
	pVerts->reserve(iEngineNum * 2);
	pCoords->reserve(iEngineNum * 2);
	pNorms->reserve(iEngineNum * 2);
	pEle->reserve(iEngineNum * 2);

	std::default_random_engine iRandom;
	iRandom.seed(0);
	std::uniform_int_distribution<> iPseudoNoise(0, 10000);

	for (int i = 0; i < iEngineNum; i++)
	{
		osg::Vec4f vData = CGMKit::GetImageColor(m_pEarthEngineDataImg, float(i + 0.5) / float(iEngineNum), 0);
		double fLon = vData.x();
		double fLat = vData.y();
		double fTopAlt = (vData.z() + vData.w()) / fUnit;
		double fX, fY, fZ;
		pEllipsoid->convertLatLongHeightToXYZ(fLat, fLon, fTopAlt, fX, fY, fZ);
		osg::Vec3 vSpherePos = osg::Vec3(fX, fY, fZ);
		osg::Vec3 vSphereUp = vSpherePos;
		vSphereUp.normalize();
		// 计算发动机喷射方向
		osg::Vec3 vVertNorm = _Pos2Norm(vSpherePos);
		// 计算发动机喷射口位置
		osg::Vec3 vPos = vSpherePos + _NozzlePos(vVertNorm, vSphereUp) / fUnit;

		// 行星发动机喷射流需要随机一些才自然 
		double fRandom = iPseudoNoise(iRandom) * 1e-4; // 0.0-1.0
		double fR = pEllipsoid->getRadiusEquator();
		double fNormalLength = fR * 0.05;
		// 如果是推进式发动机，离北极越近，缩放随机越大
		if (vSphereUp.z() > 0.1)
		{
			float fTailScale = 0.3f + 0.7f * pow(osg::clampBetween(vSphereUp.z(), 0.5f, 1.0f), 11);
			// 发动机高度有两种
			float fLineScale = (vData.w() > 1e4) ? 1.0f : 0.5f;
			fNormalLength = fR * fLineScale * (0.01 + 0.15 * fRandom * fTailScale);
		}
		else
		{
			fRandom = 0.5 + 0.5 * fRandom;
		}

		pVerts->push_back(vPos);
		pVerts->push_back(vPos + vVertNorm * fNormalLength);

		pCoords->push_back(osg::Vec2(1, fRandom));
		pCoords->push_back(osg::Vec2(0, fRandom));

		pNorms->push_back(vVertNorm);
		pNorms->push_back(vVertNorm);

		pEle->push_back(i * 2);
		pEle->push_back(i * 2 + 1);
	}

	geom->setVertexArray(pVerts);
	geom->setTexCoordArray(0, pCoords);
	geom->setNormalArray(pNorms);
	geom->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);
	geom->addPrimitiveSet(pEle);
	return geom;
}

osg::Geometry* CGMEarth::_MakeEngineJetStreamGeometry(const osg::EllipsoidModel* pEllipsoid, const double fUnit) const
{
	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);
	geom->setUseDisplayList(false);
	geom->setDataVariance(osg::Object::STATIC);

	int iEngineNum = m_pEarthEngineDataImg->s();
	osg::ref_ptr<osg::Vec3Array> pVerts = new osg::Vec3Array();
	osg::ref_ptr<osg::Vec4Array> pCoords = new osg::Vec4Array;
	osg::ref_ptr<osg::DrawElementsUInt> pEle = new osg::DrawElementsUInt(GL_TRIANGLES);
	pVerts->reserve(iEngineNum * 8);
	pCoords->reserve(iEngineNum * 8);
	pEle->reserve(iEngineNum * 12);

	std::default_random_engine iRandom;
	iRandom.seed(0);
	std::uniform_int_distribution<> iPseudoNoise(0, 10000);

	for (int i = 0; i < iEngineNum; i++)
	{
		osg::Vec4f vData = CGMKit::GetImageColor(m_pEarthEngineDataImg, float(i + 0.5) / float(iEngineNum), 0);
		double fLon = vData.x();
		double fLat = vData.y();
		double fTopAlt = (vData.z() + vData.w()) / fUnit;
		double fX, fY, fZ;
		pEllipsoid->convertLatLongHeightToXYZ(fLat, fLon, fTopAlt, fX, fY, fZ);
		osg::Vec3 vSpherePos = osg::Vec3(fX, fY, fZ);
		osg::Vec3 vSphereUp = vSpherePos;
		vSphereUp.normalize();
		// 计算发动机喷射方向
		osg::Vec3 vVertNorm = _Pos2Norm(vSpherePos);
		// 计算发动机喷射口位置
		osg::Vec3 vPos = vSpherePos + _NozzlePos(vVertNorm, vSphereUp) / fUnit;

		osg::Vec3 vVertBiNorm = osg::Vec3(1, 0, 0);
		osg::Vec3 vVertTangent = osg::Vec3(0, 1, 0);
		if (vVertNorm != osg::Vec3(0, 0, 1))
		{
			vVertBiNorm = osg::Vec3(0, 0, 1) ^ vVertNorm;
			vVertBiNorm.normalize();
			vVertTangent = vVertNorm ^ vVertBiNorm;
			vVertTangent.normalize();
		}

		// 行星发动机喷射流需要随机一些才自然 
		double fRandom = iPseudoNoise(iRandom) * 1e-4; // 0.0-1.0
		double fR = pEllipsoid->getRadiusEquator();
		double fNormalLength = fR * 0.05;

		// 行星发动机喷射流半径分两种，大的2500米，小的1700米
		double fStreamRadius = 2500.0 / fUnit;
		if (vData.w() < 1e4f)
			fStreamRadius = 1700.0 / fUnit;

		// 如果是推进式发动机，离北极越近，缩放随机越大
		if (vSphereUp.z() > 0.1)
		{
			float fTailScale = 0.3f + 0.7f * pow(osg::clampBetween(vSphereUp.z(), 0.5f, 1.0f), 11);
			// 发动机高度有两种
			float fLineScale = (vData.w() > 1e4) ? 1.0f : 0.5f;
			fNormalLength = fR * fLineScale * (0.01 + 0.15 * fRandom * fTailScale);
		}
		else
		{
			fRandom = 0.5 + 0.5 * fRandom;
		}

		// 绘制十字交叉面片
		// 喷射流顶端位置
		osg::Vec3 vStreamTop = vPos + vVertNorm * fNormalLength;
		// 喷射流长宽比
		float fRatio = fNormalLength / fStreamRadius;

		pVerts->push_back(vPos - vVertBiNorm * fStreamRadius);
		pVerts->push_back(vPos + vVertBiNorm * fStreamRadius);
		pVerts->push_back(vStreamTop - vVertBiNorm * fStreamRadius);
		pVerts->push_back(vStreamTop + vVertBiNorm * fStreamRadius);

		pVerts->push_back(vPos - vVertTangent * fStreamRadius);
		pVerts->push_back(vPos + vVertTangent * fStreamRadius);
		pVerts->push_back(vStreamTop - vVertTangent * fStreamRadius);
		pVerts->push_back(vStreamTop + vVertTangent * fStreamRadius);

		pCoords->push_back(osg::Vec4(0, 1, fRandom, fRatio));
		pCoords->push_back(osg::Vec4(1, 1, fRandom, fRatio));
		pCoords->push_back(osg::Vec4(0, 0, fRandom, fRatio));
		pCoords->push_back(osg::Vec4(1, 0, fRandom, fRatio));

		pCoords->push_back(osg::Vec4(0, 1, fRandom, fRatio));
		pCoords->push_back(osg::Vec4(1, 1, fRandom, fRatio));
		pCoords->push_back(osg::Vec4(0, 0, fRandom, fRatio));
		pCoords->push_back(osg::Vec4(1, 0, fRandom, fRatio));

		pEle->push_back(i * 8);
		pEle->push_back(i * 8 + 1);
		pEle->push_back(i * 8 + 2);
		pEle->push_back(i * 8 + 1);
		pEle->push_back(i * 8 + 3);
		pEle->push_back(i * 8 + 2);

		pEle->push_back(i * 8 + 4);
		pEle->push_back(i * 8 + 5);
		pEle->push_back(i * 8 + 6);
		pEle->push_back(i * 8 + 5);
		pEle->push_back(i * 8 + 7);
		pEle->push_back(i * 8 + 6);
	}

	geom->setVertexArray(pVerts);
	geom->setTexCoordArray(0, pCoords);
	geom->setNormalBinding(osg::Geometry::BIND_OFF);
	geom->addPrimitiveSet(pEle);
	return geom;
}

void CGMEarth::_MixWEETexture(
	const std::string& strPath0, const std::string& strPath1, const std::string& strOut,
	const int iType)
{
	for (int iFace = 0; iFace < 5; iFace++)
	{
		osg::ref_ptr<osg::Image> pImage0 = osgDB::readImageFile(
			strPath0 + std::to_string(iFace) + ".tif");
		osg::ref_ptr<osg::Image> pImage1 = osgDB::readImageFile(
			strPath1 + std::to_string(iFace) + ".tif");
		if (!pImage0.valid() || !pImage1.valid()) return;

		int iDataSize = pImage0->s() * pImage0->t() * 4;
		osg::ref_ptr<osg::Image> pOutImage = new osg::Image;
		unsigned char* pData = new unsigned char[iDataSize];
		for (int i = 0; i < pImage0->s(); i++)
		{
			for (int j = 0; j < pImage0->t(); j++)
			{
				float fX = float(i) / float(pImage0->s()-1);
				float fY = float(j) / float(pImage0->t()-1);
				osg::Vec4 c0 = CGMKit::GetImageColor(pImage0, fX, fY);
				osg::Vec4 c1 = CGMKit::GetImageColor(pImage1, 1 - fX, fY, true);
				// 目标图片当前像素R通道的地址
				int iAddress = 4 * (pImage0->s() * j + i);
				// 根据不同图片，采取不同的叠加算法
				switch (iType)
				{
				case 0:
				{
					// base color
					osg::Vec4 c2 = c0;
					c2.r() = CGMKit::Mix(c0.r(), c1.r(), c1.a());
					c2.g() = CGMKit::Mix(c0.g(), c1.g(), c1.a());
					c2.b() = CGMKit::Mix(c0.b(), c1.b(), c1.a());
					c2.a() = CGMKit::Mix(c0.a(), 0, c1.a()); // 0=陆地，1=海洋

					pData[iAddress] = (unsigned char)(c2.r() * 255);
					pData[iAddress + 1] = (unsigned char)(c2.g() * 255);
					pData[iAddress + 2] = (unsigned char)(c2.b() * 255);
					pData[iAddress + 3] = (unsigned char)(c2.a() * 255);
				}
				break;
				case 1:
				{
					// cloud color
					pData[iAddress] = (unsigned char)(c1.a() * 255);
					pData[iAddress + 1] = (unsigned char)(c0.g() * 255);
					pData[iAddress + 2] = (unsigned char)(c0.b() * 255);
					pData[iAddress + 3] = (unsigned char)(c0.a() * 255);
				}
				break;
				case 2:
				{
					// illumination color
					pData[iAddress] = (unsigned char)(c0.r() * 255);
					pData[iAddress + 1] = (unsigned char)(c0.g() * 255);
					pData[iAddress + 2] = (unsigned char)(c0.b() * 255);
					pData[iAddress + 3] = (unsigned char)(c1.a() * 255);
				}
				break;
				default:
					return;
				}
			}
		}
		pOutImage->setImage(pImage0->s(), pImage0->t(), 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, pData, osg::Image::USE_NEW_DELETE);
		osgDB::writeImageFile(*(pOutImage.get()), strOut + std::to_string(iFace) + ".tif");
	}
}

osg::Vec3 CGMEarth::_Pos2Norm(const osg::Vec3& vECEFPos) const
{
	osg::Vec3 vSphereUp = vECEFPos;
	vSphereUp.normalize();
	osg::Vec3 vEast = osg::Vec3(0, 0, 1) ^ vSphereUp;
	vEast.normalize();

	const double fPitch = -osg::PI_4;
	if (vSphereUp.z() < std::cos(fPitch))
	{
		osg::Vec3 vSphereNorm = osg::Quat(fPitch, vEast) * vSphereUp;
		vSphereNorm.normalize();
		return vSphereNorm;
	}

	return osg::Vec3(0, 0, 1);
}