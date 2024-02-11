//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMEarthEngine.cpp
/// @brief		Galaxy-Music Engine - GMEarthEngine
/// @version	1.0
/// @author		LiuTao
/// @date		2024.02.11
//////////////////////////////////////////////////////////////////////////

#include "GMEarthEngine.h"
#include "GMEngine.h"
#include "GMEarthTail.h"
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

	/*
	** 行星发动机的方向控制器的访问器
	*/
	class CEngineDirControlVisitor : public osg::NodeVisitor
	{
	public:
		CEngineDirControlVisitor()
			: NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _fUnit(1e10)
		{
			ellipsoid.setRadiusEquator(osg::WGS_84_RADIUS_EQUATOR / _fUnit);
			ellipsoid.setRadiusPolar(osg::WGS_84_RADIUS_POLAR / _fUnit);
		}

		void SetUnit(const double fUnit)
		{
			ellipsoid.setRadiusEquator(osg::WGS_84_RADIUS_EQUATOR / _fUnit);
			ellipsoid.setRadiusPolar(osg::WGS_84_RADIUS_POLAR / _fUnit);
		}

		void apply(osg::Node& node) { traverse(node); }
		void apply(osg::Geode& node)
		{
			osg::Geometry* geom = dynamic_cast<osg::Geometry*>(node.getDrawable(0));
			if (!geom) return;

			osg::ref_ptr<osg::Vec3Array> pVert = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
			if (!pVert.valid()) return;

			for (int i = 0; i < pVert->size(); i++)
			{
			}

			traverse(node);
		}
	private:
		osg::EllipsoidModel ellipsoid;
		double				_fUnit;
	};

}	// GM

/*************************************************************************
CGMEarthEngine Methods
*************************************************************************/

/** @brief 构造 */
CGMEarthEngine::CGMEarthEngine() : m_pKernelData(nullptr), m_pCommonUniform(nullptr), m_pConfigData(nullptr),
	m_strCoreModelPath("Models/"),
	m_strGalaxyShaderPath("Shaders/GalaxyShader/"),
	m_strEarthShaderPath("Shaders/EarthShader/"),
	m_fEngineStartRatioUniform(new osg::Uniform("engineStartRatio", 0.0f))
{
	m_pEarthEngineRoot_1 = new osg::Group();
	m_pEarthEngineRoot_2 = new osg::Group();

	m_pEllipsoid = new osg::EllipsoidModel();
}

/** @brief 析构 */
CGMEarthEngine::~CGMEarthEngine()
{
}

/** @brief 初始化 */
bool CGMEarthEngine::Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform)
{
	m_pKernelData = pKernelData;
	m_pConfigData = pConfigData;
	m_pCommonUniform = pCommonUniform;

	// 读取dds时需要垂直翻转
	m_pDDSOptions = new osgDB::Options("dds_flip");
	m_pEngineDirControl = new CEngineDirControlVisitor();

	// 用于存储行星发动机所有参数的图片
	m_pEarthEngineDataImg = osgDB::readImageFile(m_pConfigData->strCorePath + "Textures/Sphere/Earth/EarthEngineData.tif");

	return true;
}

/** @brief 更新 */
bool CGMEarthEngine::Update(double dDeltaTime)
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

	float fWanderProgress = 0.0f;
	m_fWanderProgressUniform->get(fWanderProgress);
	unsigned int iJetMask = fWanderProgress > 0.1f ? ~0 : 0;

	m_pEarthEngineStream->setNodeMask(iJetMask);
	m_pEarthEnginePointNode_1->setNodeMask(iJetMask);
	m_pEarthEnginePointNode_2->setNodeMask(iJetMask);
	m_pEarthEngineJetNode_1->setNodeMask(iJetMask);
	m_pEarthEngineJetNode_2->setNodeMask(iJetMask);

	return true;
}

/** @brief 更新(在主相机更新姿态之后) */
bool CGMEarthEngine::UpdateLater(double dDeltaTime)
{
	return true;
}

/** @brief 加载 */
bool CGMEarthEngine::Load()
{
	std::string strGalaxyShader = m_pConfigData->strCorePath + m_strGalaxyShaderPath;
	std::string strEarthShader = m_pConfigData->strCorePath + m_strEarthShaderPath;

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

	return true;
}

void CGMEarthEngine::SetTex(osg::Texture* pEarthTailTex, osg::Texture* pInscatteringTex)
{
	m_pEarthTailTex = pEarthTailTex;
	m_pInscatteringTex = pInscatteringTex;
}

void CGMEarthEngine::SetUniform(
	osg::Uniform* pViewLight,
	osg::Uniform* pGroundTop,
	osg::Uniform* pAtmosHeight,
	osg::Uniform* pMinDotUL,
	osg::Uniform* pEyeAltitude,
	osg::Uniform* pWanderProgress)
{
	m_vViewLightUniform = pViewLight;
	m_fGroundTopUniform = pGroundTop;
	m_fAtmosHeightUniform = pAtmosHeight;
	m_fMinDotULUniform = pMinDotUL;
	m_fEyeAltitudeUniform = pEyeAltitude;
	m_fWanderProgressUniform = pWanderProgress;
}

void CGMEarthEngine::ResizeScreen(const int iW, const int iH)
{
}

osg::Node* CGMEarthEngine::GetEarthEngineRoot(const int iHie) const
{
	if (0 == iHie)
	{
		return nullptr;
	}
	else if (1 == iHie)
	{
		return m_pEarthEngineRoot_1.get();
	}
	else if (2 == iHie)
	{
		return m_pEarthEngineRoot_2.get();
	}
	else
	{
		return nullptr;
	}
}

void CGMEarthEngine::SetVisible(const bool bVisible)
{
	if (bVisible)
	{
		if (0 == m_pEarthEngineRoot_1->getNodeMask())
		{
			m_pEarthEngineRoot_1->setNodeMask(~0);
			m_pEarthEngineRoot_2->setNodeMask(~0);
		}
	}
	else
	{
		if (0 != m_pEarthEngineRoot_1->getNodeMask())
		{
			m_pEarthEngineRoot_1->setNodeMask(0);
			m_pEarthEngineRoot_2->setNodeMask(0);
		}
	}
}

void CGMEarthEngine::SetWanderingEarthProgress(const float fProgress)
{
	// 发动机在月球危机时开启
	m_fEngineStartRatioUniform->set(fmaxf((fProgress-0.3f)*10.0f, 0.0f));

	//m_pEarthEngineJetNode_2->accept(*m_pEngineDirManager);
}

bool CGMEarthEngine::CreateEngine()
{
	// 临时添加的生成“行星发动机数据”的工具函数
	//_GenEarthEngineData();
	// 临时添加的生成“行星发动机分布图”和“周围bloom图”的工具函数
	//_GenEarthEngineTexture();

	_GenEarthEnginePoint_1();
	_GenEarthEngineJetLine_1();
	_GenEarthEnginePoint_2();
	_GenEarthEngineJetLine_2();
	_GenEarthEngineBody_1();
	_GenEarthEngineBody_2();

	_GenEarthEngineStream();

	return true;
}

bool CGMEarthEngine::UpdateHierarchy(int iHieNew)
{
	return true;
}

bool CGMEarthEngine::_GenEarthEnginePoint_1()
{
	// 流浪地球上的行星发动机的喷射口亮点
	double fUnit = m_pKernelData->fUnitArray->at(1);
	m_pEllipsoid->setRadiusEquator(osg::WGS_84_RADIUS_EQUATOR / fUnit);
	m_pEllipsoid->setRadiusPolar(osg::WGS_84_RADIUS_POLAR / fUnit);
	m_pEarthEnginePointNode_1 = new osg::Geode();
	m_pEarthEnginePointNode_1->addDrawable(_MakeEnginePointGeometry(m_pEllipsoid, fUnit));
	m_pEarthEngineRoot_1->addChild(m_pEarthEnginePointNode_1);

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
	pSSPlanetEnginePoint->setTextureAttributeAndModes(0, m_pEarthTailTex, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	osg::ref_ptr<osg::Uniform> pTailUniform = new osg::Uniform("tailTex", 0);
	pSSPlanetEnginePoint->addUniform(pTailUniform);

	std::string strVertPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEnginePoint.vert";
	std::string strFragPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEnginePoint.frag";
	CGMKit::LoadShader(pSSPlanetEnginePoint, strVertPath, strFragPath, "PlanetEnginePoint_1");

	return true;
}

bool CGMEarthEngine::_GenEarthEnginePoint_2()
{
	// 流浪地球上的行星发动机的喷射口亮点
	double fUnit = m_pKernelData->fUnitArray->at(2);
	m_pEllipsoid->setRadiusEquator(osg::WGS_84_RADIUS_EQUATOR / fUnit);
	m_pEllipsoid->setRadiusPolar(osg::WGS_84_RADIUS_POLAR / fUnit);
	m_pEarthEnginePointNode_2 = new osg::Geode();
	m_pEarthEnginePointNode_2->addDrawable(_MakeEnginePointGeometry(m_pEllipsoid, fUnit));
	m_pEarthEngineRoot_2->addChild(m_pEarthEnginePointNode_2);

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
	pSSPlanetEnginePoint->setTextureAttributeAndModes(0, m_pEarthTailTex, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	osg::ref_ptr<osg::Uniform> pTailUniform = new osg::Uniform("tailTex", 0);
	pSSPlanetEnginePoint->addUniform(pTailUniform);

	std::string strVertPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEnginePoint.vert";
	std::string strFragPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEnginePoint.frag";
	CGMKit::LoadShader(pSSPlanetEnginePoint, strVertPath, strFragPath, "PlanetEnginePoint_2");

	return true;
}

bool CGMEarthEngine::_GenEarthEngineJetLine_1()
{
	// 流浪地球上的行星发动机的喷射流
	double fUnit = m_pKernelData->fUnitArray->at(1);
	m_pEllipsoid->setRadiusEquator(osg::WGS_84_RADIUS_EQUATOR / fUnit);
	m_pEllipsoid->setRadiusPolar(osg::WGS_84_RADIUS_POLAR / fUnit);
	m_pEarthEngineJetNode_1 = new osg::Geode();
	m_pEarthEngineJetNode_1->addDrawable(_MakeEngineJetLineGeometry(m_pEllipsoid, fUnit));
	m_pEarthEngineRoot_1->addChild(m_pEarthEngineJetNode_1);

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
	pSSPlanetEngineJet->setTextureAttributeAndModes(0, m_pEarthTailTex, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	osg::ref_ptr<osg::Uniform> pTailUniform = new osg::Uniform("tailTex", 0);
	pSSPlanetEngineJet->addUniform(pTailUniform);

	std::string strJetVertPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEngineJet.vert";
	std::string strJetFragPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEngineJet.frag";
	CGMKit::LoadShader(pSSPlanetEngineJet, strJetVertPath, strJetFragPath, "PlanetEngineJet_1");

	return true;
}

bool CGMEarthEngine::_GenEarthEngineJetLine_2()
{
	// 流浪地球上的行星发动机的喷射流
	double fUnit = m_pKernelData->fUnitArray->at(2);
	m_pEllipsoid->setRadiusEquator(osg::WGS_84_RADIUS_EQUATOR / fUnit);
	m_pEllipsoid->setRadiusPolar(osg::WGS_84_RADIUS_POLAR / fUnit);
	m_pEarthEngineJetNode_2 = new osg::Geode();
	m_pEarthEngineJetNode_2->addDrawable(_MakeEngineJetLineGeometry(m_pEllipsoid, fUnit));
	m_pEarthEngineRoot_2->addChild(m_pEarthEngineJetNode_2);

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
	pSSPlanetEngineJet->setTextureAttributeAndModes(0, m_pEarthTailTex, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	osg::ref_ptr<osg::Uniform> pTailUniform = new osg::Uniform("tailTex", 0);
	pSSPlanetEngineJet->addUniform(pTailUniform);

	std::string strJetVertPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEngineJet.vert";
	std::string strJetFragPath = m_pConfigData->strCorePath + m_strEarthShaderPath + "PlanetEngineJet.frag";
	CGMKit::LoadShader(pSSPlanetEngineJet, strJetVertPath, strJetFragPath, "PlanetEngineJet_2");

	return true;
}

bool CGMEarthEngine::_GenEarthEngineBody_1()
{
	m_pEarthEngineBody_1 = osgDB::readNodeFile(m_pConfigData->strCorePath + m_strCoreModelPath + "theWanderingEarth_engine_LOD5.ive");
	if (!m_pEarthEngineBody_1.valid()) return false;
	m_pEarthEngineRoot_1->addChild(m_pEarthEngineBody_1);

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
	pSSEngineBody->addUniform(m_fWanderProgressUniform);

	int iTexUnit = 0;
	// base color贴图
	osg::ref_ptr<osg::Uniform> pBaseColorTexUniform = new osg::Uniform("baseColorTex", iTexUnit++);
	pSSEngineBody->addUniform(pBaseColorTexUniform);
	// 大气“内散射”纹理
	pSSEngineBody->setTextureAttributeAndModes(iTexUnit, m_pInscatteringTex, osg::StateAttribute::ON);
	osg::ref_ptr<osg::Uniform> pInscatteringUniform = new osg::Uniform("inscatteringTex", iTexUnit++);
	pSSEngineBody->addUniform(pInscatteringUniform);
	// 流浪地球尾迹（吹散的大气）
	pSSEngineBody->setTextureAttributeAndModes(iTexUnit, m_pEarthTailTex, osg::StateAttribute::ON);
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

bool CGMEarthEngine::_GenEarthEngineBody_2()
{
	m_pEarthEngineBody_2 = osgDB::readNodeFile(m_pConfigData->strCorePath + m_strCoreModelPath + "theWanderingEarth_engine_LOD5.ive");
	if (!m_pEarthEngineBody_2.valid()) return false;
	m_pEarthEngineRoot_2->addChild(m_pEarthEngineBody_2);

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
	pSSEngineBody->addUniform(m_fWanderProgressUniform);

	int iTexUnit = 0;
	// base color 贴图
	osg::ref_ptr<osg::Uniform> pBaseColorTexUniform = new osg::Uniform("baseColorTex", iTexUnit++);
	pSSEngineBody->addUniform(pBaseColorTexUniform);
	// 大气“内散射”纹理
	pSSEngineBody->setTextureAttributeAndModes(iTexUnit, m_pInscatteringTex, osg::StateAttribute::ON);
	osg::ref_ptr<osg::Uniform> pInscatteringUniform = new osg::Uniform("inscatteringTex", iTexUnit++);
	pSSEngineBody->addUniform(pInscatteringUniform);
	// 流浪地球尾迹（吹散的大气）
	pSSEngineBody->setTextureAttributeAndModes(iTexUnit, m_pEarthTailTex, osg::StateAttribute::ON);
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

bool CGMEarthEngine::_GenEarthEngineStream()
{
	// 流浪地球上的行星发动机的喷射流，用于近地视角
	double fUnit = m_pKernelData->fUnitArray->at(1);
	m_pEllipsoid->setRadiusEquator(osg::WGS_84_RADIUS_EQUATOR / fUnit);
	m_pEllipsoid->setRadiusPolar(osg::WGS_84_RADIUS_POLAR / fUnit);
	m_pEarthEngineStream = new osg::Geode();
	m_pEarthEngineStream->addDrawable(_MakeEngineJetStreamGeometry(m_pEllipsoid, fUnit));
	m_pEarthEngineRoot_1->addChild(m_pEarthEngineStream);

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

osg::Texture* CGMEarthEngine::_CreateTexture2D(const std::string & fileName, const int iChannelNum) const
{
	return _CreateTexture2D(osgDB::readImageFile(fileName), iChannelNum);
}

osg::Texture* CGMEarthEngine::_CreateTexture2D(osg::Image * pImg, const int iChannelNum) const
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

osg::Texture* CGMEarthEngine::_CreateDDSTexture(const std::string& fileName,
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

void CGMEarthEngine::_GenEarthEngineData()
{
	osg::ref_ptr<osg::Node> pNode = osgDB::readNodeFile("D:/GMHelp/max/sphere/theWanderingEarth/theWanderingEarthEngineLocation.ive");
	if (!pNode.valid()) return;

	CGenEngineDataVisitor cGenDataVisitor;
	pNode->accept(cGenDataVisitor);
}

void CGMEarthEngine::_GenEarthEngineTexture()
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

osg::Geometry* CGMEarthEngine::_MakeEnginePointGeometry(const osg::EllipsoidModel* pEllipsoid, const double fUnit) const
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
			fRandom = 0.4 + 0.6 * fRandom;
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

osg::Geometry* CGMEarthEngine::_MakeEngineJetLineGeometry(const osg::EllipsoidModel* pEllipsoid, const double fUnit) const
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
			float fLineScale = (vData.w() > 1e4) ? 1.0f : 0.2f;
			fNormalLength = fR * (0.01 + 0.15 * fRandom * fTailScale * fLineScale);
		}
		else
		{
			fRandom = 0.4 + 0.6 * fRandom;
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

osg::Geometry* CGMEarthEngine::_MakeEngineJetStreamGeometry(const osg::EllipsoidModel* pEllipsoid, const double fUnit) const
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
			float fLineScale = (vData.w() > 1e4) ? 1.0f : 0.2f;
			fNormalLength = fR * (0.01 + 0.15 * fRandom * fTailScale * fLineScale);
		}
		else
		{
			fRandom = 0.4 + 0.6 * fRandom;
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

void CGMEarthEngine::_MixWEETexture(
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

osg::Vec3 CGMEarthEngine::_Pos2Norm(const osg::Vec3& vECEFPos) const
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