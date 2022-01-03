#include "GMGalaxy.h"
//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMGalaxy.cpp
/// @brief		Galaxy-Music Engine - GMGalaxy
/// @version	1.0
/// @author		LiuTao
/// @date		2020.11.27
//////////////////////////////////////////////////////////////////////////

#include "GMGalaxy.h"
#include "GMEngine.h"
#include "GMDataManager.h"
#include "GMKit.h"
#include <osg/PointSprite>
#include <osg/LineWidth>
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/TextureCubeMap>
#include <osg/PositionAttitudeTransform>
#include <osgDB/ReadFile>

using namespace GM;
/*************************************************************************
Macro Defines
*************************************************************************/

#define GM_MIN_STARS_CUBE		8e18		// cube恒星的最小尺寸
#define GM_MIN_GALAXIES_CUBE	2e25		// cube星系的最小尺寸
#define PULSE_NUM				128 		// max number of audio pulse

/*************************************************************************
Class
*************************************************************************/

/*************************************************************************
CGMGalaxy Methods
*************************************************************************/

/** @brief 构造 */
CGMGalaxy::CGMGalaxy() :CGMNebula(), m_fGalaxyRadius(5e20),
	m_strGalaxyShaderPath("Shaders/GalaxyShader/"), m_strGalaxyTexPath("Textures/Galaxy/"),
	m_strPlayingStarName(L""), m_vPlayingAudioCoord(SGMAudioCoord(0.5, 0.0)),
	m_vPlayingStarWorldPos(osg::Vec3d(0.0, 0.0, 0.0)),
	m_pMousePosUniform(new osg::Uniform("mouseWorldPos", osg::Vec3f(0.0f, 0.0f, 0.0f))),
	m_pStarHiePosUniform(new osg::Uniform("starWorldPos", osg::Vec3f(0.0f, 0.0f, 0.0f))),
	m_pStarAudioPosUniform(new osg::Uniform("starAudioPos", osg::Vec2f(0.5f, 0.0f))),
	m_pTimesUniform(new osg::Uniform("times", 0.0f)),
	m_pStarColorUniform(new osg::Uniform("playingStarColor", osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f))),
	m_pLevelArrayUniform(new osg::Uniform(osg::Uniform::Type::FLOAT, "level", PULSE_NUM)),
	m_pGalaxyRadiusUniform(new osg::Uniform("galaxyRadius", 5.0f)),
	m_pEyePos4Uniform(new osg::Uniform("eyePos4", osg::Vec3f(0.0f, -2.5f, 0.0f))),
	m_pCubeCenterUniform(new osg::Uniform("centerOffset", osg::Vec3f(0.0f, 0.0f, 0.0f))),
	m_pShapeUVWUniform(new osg::Uniform("shapeUVW", osg::Vec3f(3.2f, 3.2f, 4.0f))),
	m_pStarAlphaUniform(new osg::Uniform("starAlpha", 1.0f)),
	m_pGalaxiesAlphaUniform(new osg::Uniform("galaxiesAlpha", 1.0f)),
	m_pSupernovaLightUniform(new osg::Uniform("supernovaLight", 1.0f)),
	m_pStarDistanceUniform(new osg::Uniform("starDistance", 1.0f)),
	m_pUnitRatioUniform(new osg::Uniform("unitRatio", 1.0f)),
	m_pMyWorldAlphaUniform(new osg::Uniform("myWorldAlpha", 0.0f)),
	m_iPlanetCount(0),
	m_bEdit(false), m_bCapture(false), m_bWelcome(false)
{
	for (int i = 0; i < PULSE_NUM; i++)
	{
		m_pLevelArrayUniform->setElement(i, 0.0f);
	}
}

/** @brief 析构 */
CGMGalaxy::~CGMGalaxy()
{
}

/** @brief 初始化 */
bool CGMGalaxy::Init(SGMKernelData * pKernelData, SGMConfigData * pConfigData)
{
	CGMNebula::Init(pKernelData, pConfigData);

	std::string strGalaxyTexPath = m_pConfigData->strCorePath + m_strGalaxyTexPath;
	m_pGalaxyColorTex = _CreateTexture2D(strGalaxyTexPath + "milkyWay.tga", 4);

	// 初始化第一层级的中心恒星变换节点

	// 初始化第二层级的中心恒星变换节点，也是第二层级行星的公转轨道中心变换节点
	if (!m_pStar_2_Transform.valid())
	{
		m_pStar_2_Transform = new osg::PositionAttitudeTransform();
		m_pStar_2_Transform->asPositionAttitudeTransform()->setPosition(osg::Vec3d(0, 0, 0));
		m_pHierarchyRootVector.at(2)->addChild(m_pStar_2_Transform.get());
	}

	// 初始化第三层级的中心恒星变换节点，也是奥尔特云、第三层级行星的公转轨道中心变换节点
	if (!m_pStar_3_Transform.valid())
	{
		m_pStar_3_Transform = new osg::PositionAttitudeTransform();
		m_pHierarchyRootVector.at(3)->addChild(m_pStar_3_Transform.get());
	}

	// 加载星系纹理数组
	const int iTexNum = 32;
	std::string strGalaxiesPath = strGalaxyTexPath + "GalaxyArray/";
	m_pGalaxiesTex = new osg::Texture2DArray();
	m_pGalaxiesTex->setTextureSize(256,256, iTexNum);
	for (int i = 0; i < iTexNum; i++)
	{
		std::string strName = std::to_string(i);
		if (i < 10)
		{
			strName = "00" + strName;
		}
		else if (i < 100)
		{
			strName = "0" + strName;
		}
		m_pGalaxiesTex->setImage(i, osgDB::readImageFile(strGalaxiesPath + strName + ".jpg"));
	}
	m_pGalaxiesTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_pGalaxiesTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_pGalaxiesTex->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	m_pGalaxiesTex->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	m_pGalaxiesTex->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
	m_pGalaxiesTex->setInternalFormat(GL_RGB);
	m_pGalaxiesTex->setSourceFormat(GL_RGB);
	m_pGalaxiesTex->setSourceType(GL_UNSIGNED_BYTE);

	size_t iMaxNum = 65536;
	// 初始化立方体恒星数组
	m_pCubeVertArray = new osg::Vec4Array;
	m_pCubeColorArray = new osg::Vec4Array();
	m_pCubeElement = new osg::DrawElementsUShort(GL_POINTS);
	m_pCubeVertArray->reserve(iMaxNum);
	m_pCubeColorArray->reserve(iMaxNum);
	m_pCubeElement->reserve(iMaxNum);

	float fMinAlpha = 0.25f;
	int x = 0;
	while (x < iMaxNum)
	{
		float fU = (m_iRandom() % 10000)*1e-4f;
		float fV = (m_iRandom() % 10000)*1e-4f;
		float fW = (m_iRandom() % 10000)*1e-4f;
		float fAlpha = 1.0f - _Get3DValue(fU*4.0f, fV*4.0f, fW*4.0f);
		fAlpha *= fAlpha;
		if (fAlpha > fMinAlpha)
		{
			float fRandomX = fU - 0.5f;
			float fRandomY = fV - 0.5f;
			float fRandomZ = fW - 0.5f;
			float fX = fRandomX;
			float fY = fRandomY;
			float fZ = fRandomZ;
			osg::Vec3f vColor = _getRandomStarColor();

			m_pCubeVertArray->push_back(osg::Vec4(fX, fY, fZ, 1.0f));
			m_pCubeColorArray->push_back(osg::Vec4(vColor, (fAlpha - fMinAlpha) / (1.0f - fMinAlpha)));
			m_pCubeElement->push_back(x);
			x++;
		}
	}

	// 初始化球面恒星数组
	m_pSphereVertArray = new osg::Vec4Array;
	m_pSphereColorArray = new osg::Vec4Array;
	m_pSphereElement = new osg::DrawElementsUShort(GL_POINTS);
	m_pSphereVertArray->reserve(iMaxNum);
	m_pSphereColorArray->reserve(iMaxNum);
	m_pSphereElement->reserve(iMaxNum);

	int iEleNum = 0;
	for (size_t i = 0; i < iMaxNum; i++)
	{
		osg::Vec4 vPos = m_pCubeVertArray->at(i);
		osg::Vec3 vVertPos = osg::Vec3(vPos.x(), vPos.y(), vPos.z());
		float fLength = vVertPos.normalize();
		if (fLength <= 0.5f)
		{
			float fFalloff = 1.0f - osg::clampBetween(2.0f*fLength / 0.5f - 1.0f, 0.0f, 1.0f);
			osg::Vec4 vert = osg::Vec4(vVertPos, fFalloff);
			m_pSphereVertArray->push_back(vert);
			osg::Vec4 color = m_pCubeColorArray->at(i);
			m_pSphereColorArray->push_back(color);
			m_pSphereElement->push_back(iEleNum++);
		}
	}

	// 星系旋转枚举(0123)和纹理编号
	// xyz = 旋转枚举，分别代表图片旋转0°、90°、180°、270°
	// 000 = 0°
	// 100 = 90°
	// 010 = 180°
	// 001 = 270°
	// w = 星系纹理编号
	m_pGalaxiesInfo = new osg::Vec4Array;
	m_pGalaxiesInfo->reserve(iMaxNum);

	float fLayerMax = m_pGalaxiesTex->getNumImages();
	float fLayer = 0.0f;
	for (int i = 0; i < iMaxNum; i++)
	{
		unsigned int iRandom = m_iRandom();
		float fHue = (iRandom % 1024) / 256.0f;
		float fAlpha = m_pCubeColorArray->at(i).a();
		int iSpin = iRandom % 4;
		m_pGalaxiesInfo->push_back(osg::Vec4(fHue, iSpin, fLayer, fAlpha));
		if (3 == (i % 4))
		{
			fLayer = std::fmod(fLayer + 1.0f, fLayerMax);
		}
	}

	if (!m_pEye_4_Transform.valid())
	{
		m_pEye_4_Transform = new osg::PositionAttitudeTransform();
		m_pHierarchyRootVector.at(4)->addChild(m_pEye_4_Transform.get());
	}

	m_pCosmosBoxNode = new CCosmosBox();
	GM_Root->addChild(m_pCosmosBoxNode.get());

	return true;
}

/** @brief 更新 */
bool CGMGalaxy::Update(double dDeltaTime)
{
	double dTime = osg::Timer::instance()->time_s();
	float fTimes = std::fmod((float)dTime, 1000000.0f);
	m_pTimesUniform->set(fTimes);

	if (m_bWelcome)
	{
		if (GM_ENGINE.IsWelcomeFinished())
		{
			if (m_pStateSetGalaxy.valid())
			{
				m_pStateSetGalaxy->setDefine("WELCOME", osg::StateAttribute::OFF);
			}
			if (m_pGeodeAudio.valid())
			{
				m_pGeodeAudio->getOrCreateStateSet()->setDefine("WELCOME", osg::StateAttribute::OFF);
			}
			m_bWelcome = false;
		}
	}
	else
	{
		if (m_bCapture)
		{
			m_pStarAudioPosUniform->set(osg::Vec2f(m_vPlayingAudioCoord.radius, m_vPlayingAudioCoord.angle));
		}
	}

	CGMNebula::Update(dDeltaTime);
	return true;
}

/** @brief 更新(在主相机更新姿态之后) */
bool CGMGalaxy::UpdateLater(double dDeltaTime)
{
	if (m_pConfigData->bHighQuality && m_pStateSetGalaxy.valid() && m_pGeodePointsN_4.valid())
	{
		unsigned int iShakeCount = GetShakeCount();
		if (iShakeCount % 2)
		{
			m_pStateSetGalaxy->setTextureAttributeAndModes(1, m_TAADistanceMap_1.get());
			m_pGeodePointsN_4->getStateSet()->setTextureAttributeAndModes(1, m_TAADistanceMap_1.get());
		}
		else
		{
			m_pStateSetGalaxy->setTextureAttributeAndModes(1, m_TAADistanceMap_0.get());
			m_pGeodePointsN_4->getStateSet()->setTextureAttributeAndModes(1, m_TAADistanceMap_0.get());
		}
	}

	osg::Vec3d vEye, vCenter, vUp;
	GM_View->getCamera()->getViewMatrixAsLookAt(vEye, vCenter, vUp);
	double fDistance = GM_ENGINE.GetHierarchyTargetDistance();

	float fLight = std::exp2(- fDistance*GM_ENGINE.GetUnit() / 5e22);
	m_pSupernovaLightUniform->set(fLight);

	int iHie = GM_ENGINE.GetHierarchy();
	if (4 == iHie)
	{
		m_pEyePos4Uniform->set(osg::Vec3(vEye));
	}
	else
	{
		osg::Vec3 vEyePos4 = m_pKernelData->vEyePosArray->at(4);
		m_pEyePos4Uniform->set(vEyePos4);
	}

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
	case 3:
	{
		m_pStarDistanceUniform->set(osg::clampBetween(float(fDistance)*100.0f, 1.0f, 4000.0f));
	}
	break;
	case 4:
	{
		if (m_pEye_4_Transform.valid())
		{
			m_pEye_4_Transform->asPositionAttitudeTransform()->setPosition(osg::Vec3f(vEye));

			// 控制cube恒星的显隐
			double fCubeMinSize = GM_MIN_STARS_CUBE / m_pKernelData->fUnitArray->at(4);
			for (int i = 0; i < int(m_pStarsGeomVector.size()); i++)
			{
				if (fDistance < fCubeMinSize * std::pow(2, i + 3))
				{
					m_pStarsGeomVector.at(i)->setNodeMask(~0);
				}
				else
				{
					m_pStarsGeomVector.at(i)->setNodeMask(0);
				}
			}
			osg::Vec2f vCubeInfo;
			m_pStarsCubeInfoUniform->get(vCubeInfo);
			vCubeInfo.y() = fDistance;
			m_pStarsCubeInfoUniform->set(vCubeInfo);
		}

		float fStarAlpha = 1.0 - exp2(std::fmin(0.0, 0.001 - fDistance)*100.0);
		m_pStarAlphaUniform->set(fStarAlpha);

		m_pGalaxyBackgroundGeode->setNodeMask((fDistance > 0.05) ? 0 : ~0);
		m_pCosmosBoxGeode->setNodeMask((fDistance > 0.001) ? ~0 : 0);
	}
	break;
	case 5:
	{
		if (m_pGalaxyGroup_Transform.valid())
		{
			osg::Vec3f vMinGalaxyGroupPos = vEye;
			float fLength = vMinGalaxyGroupPos.normalize();
			vMinGalaxyGroupPos *= std::fmin(fLength, 2 * GM_MIN_RADIUS);
			m_pGalaxyGroup_Transform->asPositionAttitudeTransform()->setPosition(vMinGalaxyGroupPos);
		}

		if (m_pEye_5_Transform.valid())
		{
			m_pEye_5_Transform->asPositionAttitudeTransform()->setPosition(osg::Vec3f(vEye));

			double fGalaxiesCubeMinSize = GM_MIN_GALAXIES_CUBE / m_pKernelData->fUnitArray->at(5);
			for (int i = 0; i < int(m_pGalaxiesGeomVector.size()); i++)
			{
				if (fDistance < fGalaxiesCubeMinSize * std::pow(2, i + 2))
				{
					m_pGalaxiesGeomVector.at(i)->setNodeMask(~0);
				}
				else
				{
					m_pGalaxiesGeomVector.at(i)->setNodeMask(0);
				}
			}
			osg::Vec2f vGalaxiesInfo;
			m_pGalaxiesInfoUniform->get(vGalaxiesInfo);
			vGalaxiesInfo.y() = fDistance;
			m_pGalaxiesInfoUniform->set(vGalaxiesInfo);
		}

		if (m_pGeodeSupercluster.valid())
		{
			m_pGeodeSupercluster->setNodeMask((fDistance < 1) ? ~0 : 0);
		}
		if (m_pGeodeUltracluster.valid())
		{
			m_pGeodeUltracluster->setNodeMask((fDistance < 10) ? ~0 : 0);
		}

		if (m_pConfigData->bPhoto)
		{
			m_pMyWorldAlphaUniform->set(osg::clampBetween(float((fDistance - 1) / 99), 0.0f, 1.0f));
		}
	}
	break;
	case 6:
	{
		if (m_pConfigData->bPhoto)
		{
			m_pMyWorldAlphaUniform->set(osg::clampBetween(float((fDistance - 1e-5) / 9.9e-4), 0.0f, 1.0f));
		}
	}
	break;
	default:
		break;
	}

	CGMNebula::UpdateLater(dDeltaTime);
	return true;
}


/** @brief 加载 */
bool CGMGalaxy::Load()
{
	if ( m_pGeodeSun_2.valid())
	{
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SunVert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SunFrag.glsl";
		CGMKit::LoadShader(m_pGeodeSun_2->getStateSet(), strVertPath, strFragPath);
	}
	if (m_pSunBloomTransform.valid())
	{
		std::string strBloomVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SunBloomVert.glsl";
		std::string strBloomFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SunBloomFrag.glsl";
		CGMKit::LoadShader(m_pSunBloomTransform->getStateSet(), strBloomVertPath, strBloomFragPath);
	}

	if (m_pGeodePlanets_2.valid() && m_pGeodePlanets_3.valid())
	{
		std::string strPlanetVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Planets_Vert.glsl";
		std::string strPlanetFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Planets_Frag.glsl";
		CGMKit::LoadShader(m_pGeodePlanets_2->getStateSet(), strPlanetVertPath, strPlanetFragPath);
		CGMKit::LoadShader(m_pGeodePlanets_3->getStateSet(), strPlanetVertPath, strPlanetFragPath);
	}

	if (m_pGeodePlanetsLine_2.valid() && m_pGeodePlanetsLine_3.valid())
	{
		std::string strLineVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "PlanetLineVert.glsl";
		std::string strLineFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "PlanetLineFrag.glsl";
		CGMKit::LoadShader(m_pGeodePlanetsLine_2->getStateSet(), strLineVertPath, strLineFragPath);
		CGMKit::LoadShader(m_pGeodePlanetsLine_3->getStateSet(), strLineVertPath, strLineFragPath);
	}

	if (m_pOortCloudGeode_3.valid() && m_pOort_4_Transform.valid())
	{
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "OortCloudVert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "OortCloudFrag.glsl";
		CGMKit::LoadShader(m_pOortCloudGeode_3->getOrCreateStateSet(), strVertPath, strFragPath);
		CGMKit::LoadShader(m_pOort_4_Transform->getStateSet(), strVertPath, strFragPath);
	}

	if (m_pGeodeStarCube_4.valid())
	{
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarCube_4_Vert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarCube_4_Frag.glsl";
		CGMKit::LoadShader(m_pGeodeStarCube_4->getStateSet(), strVertPath, strFragPath);
	}
	if (m_pGeodeStarCube.valid())
	{
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarCube_Vert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarCube_Frag.glsl";
		CGMKit::LoadShader(m_pGeodeStarCube->getStateSet(), strVertPath, strFragPath);
	}
	if (m_pStateSetGalaxy.valid())
	{
		std::string strStarVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyStarVert.glsl";
		std::string strStarFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyStarFrag.glsl";
		CGMKit::LoadShader(m_pStateSetGalaxy.get(), strStarVertPath, strStarFragPath);
	}
	if (m_pGeodePointsN_4.valid())
	{
		std::string strStarVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarNVert.glsl";
		std::string strStarFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarNFrag.glsl";
		osg::ref_ptr<osg::StateSet> pPointsNSS = m_pGeodePointsN_4->getOrCreateStateSet();
		CGMKit::LoadShader(pPointsNSS.get(), strStarVertPath, strStarFragPath);
	}
	if (m_pGeodeAudio.valid())
	{
		std::string strAudioVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "AudioVert.glsl";
		std::string strAudioFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "AudioFrag.glsl";
		CGMKit::LoadShader(m_pGeodeAudio->getStateSet(), strAudioVertPath, strAudioFragPath);
	}
	if (m_pStateSetPlane.valid())
	{
		std::string strGalaxyPlaneVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyPlaneVert.glsl";
		std::string strGalaxyPlaneFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyPlaneFrag.glsl";
		CGMKit::LoadShader(m_pStateSetPlane.get(), strGalaxyPlaneVertPath, strGalaxyPlaneFragPath);
	}
	if(m_pPlayingStarTransform.valid())
	{
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "PlayingStarVert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "PlayingStarFrag.glsl";
		CGMKit::LoadShader(m_pPlayingStarTransform->getStateSet(), strVertPath, strFragPath);
	}
	if (m_pGeodeHelpLine.valid())
	{
		std::string strLineVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "HelpLineVert.glsl";
		std::string strLineFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "HelpLineFrag.glsl";
		CGMKit::LoadShader(m_pGeodeHelpLine->getStateSet(), strLineVertPath, strLineFragPath);
	}

	if (m_pGeodeGalaxyGroup_4.valid())
	{
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxies_4_Vert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxies_4_Frag.glsl";
		CGMKit::LoadShader(m_pGeodeGalaxyGroup_4->getStateSet(), strVertPath, strFragPath);
	}
	if (m_pGeodeGalaxyGroup_5.valid() && m_pGeodeGalaxies_5.valid())
	{
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxies_5_Vert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxies_5_Frag.glsl";
		CGMKit::LoadShader(m_pGeodeGalaxyGroup_5->getStateSet(), strVertPath, strFragPath);
		CGMKit::LoadShader(m_pGeodeGalaxies_5->getStateSet(), strVertPath, strFragPath);
	}

	if (m_pGeodeSupercluster.valid() && m_pGeodeUltracluster.valid())
	{
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SuperclusterVert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SuperclusterFrag.glsl";
		CGMKit::LoadShader(m_pGeodeSupercluster->getStateSet(), strVertPath, strFragPath);
		CGMKit::LoadShader(m_pGeodeUltracluster->getStateSet(), strVertPath, strFragPath);
	}

	if (m_pGeodeMyWorld_5.valid() && m_pGeodeMyWorld_6.valid())
	{
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "MyWorldVert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "MyWorldFrag.glsl";
		CGMKit::LoadShader(m_pGeodeMyWorld_5->getStateSet(), strVertPath, strFragPath);
		CGMKit::LoadShader(m_pGeodeMyWorld_6->getStateSet(), strVertPath, strFragPath);
	}

	if (m_pGalaxyBackgroundGeode.valid())
	{
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyBackgroundVert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyBackgroundFrag.glsl";
		CGMKit::LoadShader(m_pGalaxyBackgroundGeode->getStateSet(), strVertPath, strFragPath);
	}

	if (m_pCosmosBoxGeode.valid())
	{
		std::string strCosmosBoxVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "CosmosBoxVert.glsl";
		std::string strCosmosBoxFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "CosmosBoxFrag.glsl";
		CGMKit::LoadShader(m_pCosmosBoxGeode->getStateSet(), strCosmosBoxVertPath, strCosmosBoxFragPath);
	}

	if (m_pSupernovaTransform.valid())
	{
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SupernovaVert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SupernovaFrag.glsl";
		CGMKit::LoadShader(m_pSupernovaTransform->getStateSet(), strVertPath, strFragPath);
	}

	return CGMNebula::Load();
}

void CGMGalaxy::SetEditMode(const bool bEnable)
{
	m_bEdit = bEnable;
	m_pStateSetPlane->setDefine("EDIT", bEnable ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
	if(bEnable)
	{
		_DetachAudioPoints();
	}
	else
	{
		_AttachAudioPoints();
	}
}

bool CGMGalaxy::GetEditMode()
{
	return m_bEdit;
}

void CGMGalaxy::SetCapture(const bool bEnable)
{
	if (bEnable != m_bCapture)
	{
		m_bCapture = bEnable;
		m_pStateSetPlane->setDefine("CAPTURE", bEnable ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
	}
}

void CGMGalaxy::Welcome()
{
	m_bWelcome = true;
	if (m_pStateSetGalaxy.valid())
	{
		m_pStateSetGalaxy->setDefine("WELCOME", osg::StateAttribute::ON);
	}
	if (m_pGeodeAudio.valid())
	{
		m_pGeodeAudio->getOrCreateStateSet()->setDefine("WELCOME", osg::StateAttribute::ON);
	}
}

bool CGMGalaxy::CreateGalaxy(double fDiameter)
{
	m_fGalaxyRadius = fDiameter * 0.5;
	m_pGalaxyRadiusUniform->set(float(m_fGalaxyRadius / m_pKernelData->fUnitArray->at(4)));

	// 0123层级空间下都可见的星空cube
	_CreateStarCube();

	// 创建太阳，只需要在2级空间创建
	_CreateSun();

	// 创建行星，用于2、3级空间
	_CreatePlanets();

	// 3层级空间下的物体
	// 创建奥尔特云，用于3级空间
	_CreateOortCloud();

	// 4层级空间下的物体
	_CreateAudioPoints();
	_CreateGalaxyPoints();
	_CreateGalaxyPointsN_4(2);	// 2倍密度的星星
	_CreateGalaxyPointsN_4(4);	// 4倍密度的星星
	_CreateGalaxyPointsN_4(8);	// 8倍密度的星星

	_CreateStarCube_4(); // 创建PointSprite恒星，用于4级空间
	_CreateGalaxyPlane_4();
	_CreateGalaxies_4(); // 创建PointSprite星系群，用于4级空间

	// 5层级空间下的物体
	_CreateGalaxyPlane_5();
	_CreateGalaxies_5();
	//_CreateSupercluster();
	//_CreateUltracluster();

	if (m_pConfigData->bPhoto)
	{
		_CreateMyWorld();
	}

	// 创建超新星
	_CreateSupernova();

	// 背景
	_CreateGalaxyBackground();
	_CreateCosmosBox();

	if (m_pConfigData->bHighQuality)
	{
		MakeNebula(fDiameter, fDiameter, 0.13f*fDiameter);
	}

	return true;
}

void CGMGalaxy::SetMousePosition(const osg::Vec3d& vHierarchyPos)
{
	if (m_bWelcome) return;

	osg::Vec3f vPosHierarchy = vHierarchyPos;
	m_pMousePosUniform->set(vPosHierarchy);
}

void CGMGalaxy::SetCurrentStar(const osg::Vec3d& vWorldPos, const std::wstring& wstrName)
{
	if (m_bWelcome) return;

	m_vPlayingStarWorldPos = vWorldPos;
	osg::Vec3f vPosHierarchy = vWorldPos / m_pKernelData->fUnitArray->at(4);
	if (4 > m_pKernelData->iHierarchy)
	{
		osg::Vec3d vTargetHie = GM_ENGINE_PTR->GetHierarchyLastTargetPos(4);
		vPosHierarchy = (vPosHierarchy - vTargetHie)*GM_UNIT_SCALE;
	}
	else
	{
		vPosHierarchy = vWorldPos / m_pKernelData->fUnitArray->at(m_pKernelData->iHierarchy);
	}
	m_pStarHiePosUniform->set(vPosHierarchy);
	m_strPlayingStarName = wstrName;

	// 设置激活恒星信息节点的位置
	if(m_pPlayingStarTransform.valid())
	{
		m_pPlayingStarTransform->asPositionAttitudeTransform()->setPosition(vPosHierarchy);
	}
	// 设置奥尔特云、中心恒星、行星轨道等的位置
	if (m_pStar_3_Transform.valid())
	{
		m_pStar_3_Transform->asPositionAttitudeTransform()->setPosition(vPosHierarchy);
	}
	if (m_pOort_4_Transform.valid())
	{
		m_pOort_4_Transform->asPositionAttitudeTransform()->setPosition(vPosHierarchy);
	}

	if (m_pSunBloomTransform.valid())
	{
		m_pSunBloomTransform->setPosition(osg::Vec3d(0,0,0));
	}

	// 设置超新星的位置
	if (m_pSupernovaTransform.valid())
	{
		if (4 > m_pKernelData->iHierarchy)
		{
			osg::Vec3d vTargetHie = GM_ENGINE_PTR->GetHierarchyLastTargetPos(4);
			osg::Vec3f vPosHie = (vPosHierarchy - vTargetHie)*GM_UNIT_SCALE;
			m_pSupernovaTransform->setPosition(vPosHie);
		}
		else
		{
			m_pSupernovaTransform->setPosition(vPosHierarchy);
		}
	}

	// 设置当前颜色
	SGMStarCoord vStarcoord = GM_ENGINE_PTR->GetDataManager()->GetStarCoord(wstrName);
	m_pStarColorUniform->set(GM_ENGINE_PTR->GetDataManager()->GetAudioColor(vStarcoord));
}

void CGMGalaxy::SetAudioLevel(float fLevel)
{
	for (int i = PULSE_NUM-2; i >= 0; i--)
	{
		float fL;
		m_pLevelArrayUniform->getElement(i, fL);
		m_pLevelArrayUniform->setElement(i+1, fL);
	}
	m_pLevelArrayUniform->setElement(0, fLevel);
}

osg::Vec3d CGMGalaxy::GetStarWorldPos()
{
	return m_vPlayingStarWorldPos;
}

bool CGMGalaxy::UpdateHierarchy(int iHierarchy)
{
	if (iHierarchy > 0 && GM_Root->containsNode(m_pHierarchyRootVector.at(iHierarchy - 1).get()))
	{
		GM_Root->removeChild(m_pHierarchyRootVector.at(iHierarchy - 1).get());
	}
	if (!(GM_Root->containsNode(m_pHierarchyRootVector.at(iHierarchy).get())))
	{
		GM_Root->addChild(m_pHierarchyRootVector.at(iHierarchy).get());
	}
	if (iHierarchy < 6 && GM_Root->containsNode(m_pHierarchyRootVector.at(iHierarchy + 1).get()))
	{
		GM_Root->removeChild(m_pHierarchyRootVector.at(iHierarchy + 1).get());
	}

	switch (iHierarchy)
	{
	case 0:
	{
		if (m_pSupernovaTransform.valid() && GM_Root->containsNode(m_pSupernovaTransform.get()))
		{
			GM_Root->removeChild(m_pSupernovaTransform.get());
		}
		if (m_pGeodeStarCube.valid())
		{
			m_pGeodeStarCube->setNodeMask(~0);
			m_pUnitRatioUniform->set(float(m_pKernelData->fUnitArray->at(0) / m_pKernelData->fUnitArray->at(3)));
			m_pStarDistanceUniform->set(1.0f);
		}
	}
	break;
	case 1:
	{
		if (m_pSupernovaTransform.valid() && !GM_Root->containsNode(m_pSupernovaTransform.get()))
		{
			GM_Root->addChild(m_pSupernovaTransform.get());
		}
		if (m_pGeodeStarCube.valid())
		{
			m_pGeodeStarCube->setNodeMask(~0);
			m_pUnitRatioUniform->set(float(m_pKernelData->fUnitArray->at(1) / m_pKernelData->fUnitArray->at(3)));
			m_pStarDistanceUniform->set(1.0f);
		}
	}
	break;
	case 2:
	{
		if (m_pGeodeStarCube.valid())
		{
			m_pGeodeStarCube->setNodeMask(~0);
			m_pUnitRatioUniform->set(float(m_pKernelData->fUnitArray->at(2) / m_pKernelData->fUnitArray->at(3)));
			m_pStarDistanceUniform->set(1.0f);
		}

		if (m_pSunBloomTransform.valid())
		{
			m_pSunBloomTransform->setPosition(osg::Vec3d(0,0,0));
		}
	}
	break;
	case 3:
	{
		osg::Vec3d vTargetHie = GM_ENGINE_PTR->GetHierarchyLastTargetPos(4);

		double fCubeSize = GM_MIN_STARS_CUBE / m_pKernelData->fUnitArray->at(4);
		osg::Vec3f vOffset = osg::Vec3f(
			std::fmod(vTargetHie.x(), fCubeSize),
			std::fmod(vTargetHie.y(), fCubeSize),
			std::fmod(vTargetHie.z(), fCubeSize))
			* GM_UNIT_SCALE;
		m_pCubeCenterUniform->set(vOffset);

		osg::Vec3d vPosHie = m_vPlayingStarWorldPos / m_pKernelData->fUnitArray->at(4);
		vPosHie = (vPosHie - vTargetHie)*GM_UNIT_SCALE;
		if (m_pSupernovaTransform.valid())
		{
			m_pSupernovaTransform->setPosition(vPosHie);
		}
		if (m_pStar_3_Transform.valid())
		{
			m_pStar_3_Transform->asPositionAttitudeTransform()->setPosition(vPosHie);
		}
		
		if (m_pGeodeStarCube.valid())
		{
			m_pGeodeStarCube->setNodeMask(~0);
			m_pUnitRatioUniform->set(1.0f);
		}
		m_pStarAlphaUniform->set(0.0f);
		m_pStarHiePosUniform->set(osg::Vec3f(vPosHie));
	}
	break;
	case 4:
	{
		osg::Vec3f vPosHierarchy = m_vPlayingStarWorldPos / m_pKernelData->fUnitArray->at(4);
		if (m_pSupernovaTransform.valid())
		{
			m_pSupernovaTransform->setPosition(vPosHierarchy);
		}
		if (m_pGeodeStarCube.valid())
		{
			m_pGeodeStarCube->setNodeMask(0);
		}
		if (m_pOort_4_Transform.valid())
		{
			m_pOort_4_Transform->asPositionAttitudeTransform()->setPosition(vPosHierarchy);
		}
		m_pStarDistanceUniform->set(4000.0f);
		m_pStarHiePosUniform->set(vPosHierarchy);
	}
	break;
	case 5:
	{
		if (m_pSupernovaTransform.valid())
		{
			if (!GM_Root->containsNode(m_pSupernovaTransform.get()))
			{
				GM_Root->addChild(m_pSupernovaTransform.get());
			}
			osg::Vec3f vPosHierarchy = m_vPlayingStarWorldPos / m_pKernelData->fUnitArray->at(5);
			m_pSupernovaTransform->setPosition(vPosHierarchy);
		}
		if (m_pGeodeStarCube.valid())
		{
			m_pGeodeStarCube->setNodeMask(0);
		}
	}
	break;
	case 6:
	{
		if (m_pSupernovaTransform.valid() && GM_Root->containsNode(m_pSupernovaTransform.get()))
		{
			GM_Root->removeChild(m_pSupernovaTransform.get());
		}
		if (m_pGeodeStarCube.valid())
		{
			m_pGeodeStarCube->setNodeMask(0);
		}
	}
	break;
	default:
		return false;
	}

	return true;
}

bool CGMGalaxy::_CreateSupernova()
{
	m_pSupernovaTransform = new osg::AutoTransform();
	m_pSupernovaTransform->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
	m_pSupernovaTransform->setAutoScaleToScreen(true);
	GM_Root->addChild(m_pSupernovaTransform.get());

	osg::ref_ptr<osg::Geode> pGeodeSupernova = new osg::Geode();
	osg::Geometry* pGeometry = new osg::Geometry();

	// 顶点
	osg::ref_ptr<osg::Vec3Array> verArray = new osg::Vec3Array;
	// 光圈
	float fHalfWidth = 32;
	float fHalfHeight = 32;
	verArray->push_back(osg::Vec3(-fHalfWidth, -fHalfHeight, 0.0f));
	verArray->push_back(osg::Vec3(fHalfWidth, -fHalfHeight, 0.0f));
	verArray->push_back(osg::Vec3(fHalfWidth, fHalfHeight, 0.0f));
	verArray->push_back(osg::Vec3(-fHalfWidth, -fHalfHeight, 0.0f));
	verArray->push_back(osg::Vec3(fHalfWidth, fHalfHeight, 0.0f));
	verArray->push_back(osg::Vec3(-fHalfWidth, fHalfHeight, 0.0f));
	// 横亮线
	fHalfWidth = 100;
	fHalfHeight = 2;
	verArray->push_back(osg::Vec3(-fHalfWidth, -fHalfHeight, 0.0f));
	verArray->push_back(osg::Vec3(fHalfWidth, -fHalfHeight, 0.0f));
	verArray->push_back(osg::Vec3(fHalfWidth, fHalfHeight, 0.0f));
	verArray->push_back(osg::Vec3(-fHalfWidth, -fHalfHeight, 0.0f));
	verArray->push_back(osg::Vec3(fHalfWidth, fHalfHeight, 0.0f));
	verArray->push_back(osg::Vec3(-fHalfWidth, fHalfHeight, 0.0f));
	// 竖亮线
	fHalfWidth = 2;
	fHalfHeight = 60;
	verArray->push_back(osg::Vec3(fHalfWidth, -fHalfHeight, 0.0f));
	verArray->push_back(osg::Vec3(fHalfWidth, fHalfHeight, 0.0f));
	verArray->push_back(osg::Vec3(-fHalfWidth, fHalfHeight, 0.0f));
	verArray->push_back(osg::Vec3(fHalfWidth, -fHalfHeight, 0.0f));
	verArray->push_back(osg::Vec3(-fHalfWidth, fHalfHeight, 0.0f));
	verArray->push_back(osg::Vec3(-fHalfWidth, -fHalfHeight, 0.0f));
	pGeometry->setVertexArray(verArray);

	// UV：亮线图片128*8像素，在上端；光圈图片100*100像素，在下面，四周留白10像素
	osg::ref_ptr<osg::Vec2Array> textArray = new osg::Vec2Array;
	// 光圈
	osg::Vec2 vLD = osg::Vec2(13.5f / 128.0f, 9.5f / 128.0f);
	float fDeltaU = 101.0f / 128.0f;
	float fDeltaV = 101.0f / 128.0f;
	textArray->push_back(vLD);
	textArray->push_back(vLD + osg::Vec2(fDeltaU, 0));
	textArray->push_back(vLD + osg::Vec2(fDeltaU, fDeltaV));
	textArray->push_back(vLD);
	textArray->push_back(vLD + osg::Vec2(fDeltaU, fDeltaV));
	textArray->push_back(vLD + osg::Vec2(0, fDeltaV));
	// 亮线，横竖亮线的UV顺序完全一样
	vLD = osg::Vec2(0.0f, 120.5f / 128.0f);
	fDeltaU = 1.0f;
	fDeltaV = 7.0f / 128.0f;
	// 横亮线
	textArray->push_back(vLD);
	textArray->push_back(vLD + osg::Vec2(fDeltaU, 0));
	textArray->push_back(vLD + osg::Vec2(fDeltaU, fDeltaV));
	textArray->push_back(vLD);
	textArray->push_back(vLD + osg::Vec2(fDeltaU, fDeltaV));
	textArray->push_back(vLD + osg::Vec2(0, fDeltaV));
	// 竖亮线
	textArray->push_back(vLD);
	textArray->push_back(vLD + osg::Vec2(fDeltaU, 0));
	textArray->push_back(vLD + osg::Vec2(fDeltaU, fDeltaV));
	textArray->push_back(vLD);
	textArray->push_back(vLD + osg::Vec2(fDeltaU, fDeltaV));
	textArray->push_back(vLD + osg::Vec2(0, fDeltaV));
	pGeometry->setTexCoordArray(0, textArray);

	// 法线
	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 0, 1));
	pGeometry->setNormalArray(normal);
	pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
	// Primitive
	pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, 18));

	pGeometry->setUseVertexBufferObjects(true);
	pGeometry->setUseDisplayList(false);
	pGeometry->setDataVariance(osg::Object::DYNAMIC);

	pGeodeSupernova->addDrawable(pGeometry);
	m_pSupernovaTransform->addChild(pGeodeSupernova.get());
	osg::ref_ptr<osg::StateSet> pSSS = m_pSupernovaTransform->getOrCreateStateSet();

	pSSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSS->setMode(GL_BLEND, osg::StateAttribute::ON);
	osg::ref_ptr<osg::BlendEquation> blendEqua = new osg::BlendEquation(osg::BlendEquation::RGBA_MAX);
	pSSS->setAttributeAndModes(blendEqua.get(), osg::StateAttribute::ON);
	pSSS->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pSSS->setRenderBinDetails(BIN_SUPERNOVA, "DepthSortedBin");

	std::string strGalaxyTexPath = m_pConfigData->strCorePath + m_strGalaxyTexPath;
	osg::ref_ptr<osg::Texture> _supernovaTex = _CreateTexture2D(strGalaxyTexPath + "supernova.jpg", 3);
	int iUnit = 0;
	pSSS->setTextureAttributeAndModes(iUnit, _supernovaTex.get());
	pSSS->addUniform(new osg::Uniform("supernovaTex", iUnit));
	iUnit++;

	pSSS->addUniform(m_pStarColorUniform.get());
	pSSS->addUniform(m_pLevelArrayUniform.get());
	pSSS->addUniform(m_pSupernovaLightUniform.get());

	// 添加shader
	std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SupernovaVert.glsl";
	std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SupernovaFrag.glsl";
	return CGMKit::LoadShader(pSSS.get(), strVertPath, strFragPath);
}

bool CGMGalaxy::_CreateStarCube()
{
	float fCubeSize = GM_MIN_STARS_CUBE / m_pKernelData->fUnitArray->at(3);

	if (!m_pGeodeStarCube.valid())
	{
		m_pGeodeStarCube = new osg::Geode();
		m_pGeodeStarCube->setNodeMask(0);
		m_pCosmosBoxNode->addChild(m_pGeodeStarCube.get());
		osg::ref_ptr<osg::StateSet> pSS_3 = m_pGeodeStarCube->getOrCreateStateSet();

		pSS_3->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
		pSS_3->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
		pSS_3->setRenderBinDetails(BIN_STARS, "DepthSortedBin");

		pSS_3->addUniform(m_pCubeCenterUniform.get());
		pSS_3->addUniform(m_pUnitRatioUniform.get());
		pSS_3->addUniform(m_pStarDistanceUniform.get());
		pSS_3->addUniform(m_pStarColorUniform.get());

		// 添加shader
		std::string strStarVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarCube_Vert.glsl";
		std::string strStarFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarCube_Frag.glsl";
		CGMKit::LoadShader(pSS_3.get(), strStarVertPath, strStarFragPath);
	}

	size_t iNum = m_pCubeVertArray->size();
	osg::ref_ptr<osg::Vec4Array> vertArray = new osg::Vec4Array;
	vertArray->reserve(iNum);
	for (size_t i = 0; i < iNum; i++)
	{
		osg::Vec4 vert = m_pCubeVertArray->at(i);
		vertArray->push_back(vert*fCubeSize);
	}

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
	pGeometry->setVertexArray(vertArray.get());
	pGeometry->setColorArray(m_pCubeColorArray.get());
	pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 1, 0));
	pGeometry->setNormalArray(normal.get());
	pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
	pGeometry->addPrimitiveSet(m_pCubeElement.get());

	pGeometry->setUseVertexBufferObjects(true);
	pGeometry->setUseDisplayList(false);
	pGeometry->setDataVariance(osg::Object::DYNAMIC);

	m_pGeodeStarCube->addDrawable(pGeometry.get());
	return true;
}

bool CGMGalaxy::_CreateSun()
{
	// 创建太阳光球
	// 太阳半径：6.963e8 米
	double fSunRadius2 = 6.963e8 / m_pKernelData->fUnitArray->at(2);
	m_pGeodeSun_2 = new osg::Geode;
	osg::ref_ptr<const osg::EllipsoidModel>	_sunModel_2 = new osg::EllipsoidModel(fSunRadius2, fSunRadius2);
	osg::ref_ptr<osg::Geometry> pDrawable_2 = _MakeEllipsoidGeometry(_sunModel_2, 32, 16, 0, true, true);
	m_pGeodeSun_2->addDrawable(pDrawable_2.get());
	m_pStar_2_Transform->addChild(m_pGeodeSun_2.get());

	osg::ref_ptr<osg::StateSet>	pSS_2 = m_pGeodeSun_2->getOrCreateStateSet();
	pSS_2->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSS_2->setMode(GL_BLEND, osg::StateAttribute::OFF);
	pSS_2->setAttributeAndModes(new osg::CullFace());
	pSS_2->setRenderBinDetails(BIN_STAR_PLAYING, "DepthSortedBin");
	pSS_2->addUniform(m_pStarColorUniform.get());
	pSS_2->addUniform(m_pLevelArrayUniform.get());
	pSS_2->addUniform(m_pTimesUniform.get());
	int iUnit2 = 0;
	pSS_2->setTextureAttributeAndModes(iUnit2, m_3DShapeTex_128.get());
	pSS_2->addUniform(new osg::Uniform("shapeNoiseTex", iUnit2));
	iUnit2++;

	// 添加shader
	std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SunVert.glsl";
	std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SunFrag.glsl";
	CGMKit::LoadShader(pSS_2.get(), strVertPath, strFragPath);

	// 创建太阳辉光
	m_pSunBloomTransform = new osg::AutoTransform();
	m_pSunBloomTransform->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
	m_pHierarchyRootVector.at(2)->addChild(m_pSunBloomTransform.get());

	osg::ref_ptr<osg::Geode> pGeodeSunBloom = new osg::Geode();
	osg::Geometry* pBloomGeometry = new osg::Geometry();
	double fHalfWidth = 9 * fSunRadius2;
	// 顶点
	osg::ref_ptr<osg::Vec3Array> verArray = new osg::Vec3Array;
	verArray->reserve(6);
	verArray->push_back(osg::Vec3(-fHalfWidth, -fHalfWidth, 0.0f));
	verArray->push_back(osg::Vec3(fHalfWidth, -fHalfWidth, 0.0f));
	verArray->push_back(osg::Vec3(fHalfWidth, fHalfWidth, 0.0f));
	verArray->push_back(osg::Vec3(-fHalfWidth, -fHalfWidth, 0.0f));
	verArray->push_back(osg::Vec3(fHalfWidth, fHalfWidth, 0.0f));
	verArray->push_back(osg::Vec3(-fHalfWidth, fHalfWidth, 0.0f));
	pBloomGeometry->setVertexArray(verArray);
	// UV
	osg::ref_ptr<osg::Vec2Array> textArray = new osg::Vec2Array;
	verArray->reserve(6);
	textArray->push_back(osg::Vec2(-1, -1));
	textArray->push_back(osg::Vec2(1, -1));
	textArray->push_back(osg::Vec2(1, 1));
	textArray->push_back(osg::Vec2(-1, -1));
	textArray->push_back(osg::Vec2(1, 1));
	textArray->push_back(osg::Vec2(-1, 1));
	pBloomGeometry->setTexCoordArray(0, textArray);
	// 法线
	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 0, 1));
	pBloomGeometry->setNormalArray(normal);
	pBloomGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
	// Primitive
	pBloomGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, 6));
	pGeodeSunBloom->addDrawable(pBloomGeometry);
	m_pSunBloomTransform->addChild(pGeodeSunBloom.get());
	osg::ref_ptr<osg::StateSet> pSSS = m_pSunBloomTransform->getOrCreateStateSet();

	pSSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSS->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSS->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pSSS->setRenderBinDetails(BIN_SUN_BLOOM, "DepthSortedBin");

	int iUnitBloom = 0;
	std::string strGalaxyTexPath = m_pConfigData->strCorePath + m_strGalaxyTexPath;
	pSSS->setTextureAttributeAndModes(iUnitBloom, _CreateTexture2D(strGalaxyTexPath + "sunNoise.jpg", 1));
	pSSS->addUniform(new osg::Uniform("sunNoiseTex", iUnitBloom));
	iUnitBloom++;

	pSSS->addUniform(m_pStarColorUniform.get());
	pSSS->addUniform(m_pLevelArrayUniform.get());
	pSSS->addUniform(m_pTimesUniform.get());

	// 添加shader
	std::string strBloomVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SunBloomVert.glsl";
	std::string strBloomFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SunBloomFrag.glsl";
	return CGMKit::LoadShader(pSSS.get(), strBloomVertPath, strBloomFragPath);
}

bool CGMGalaxy::_CreatePlanets()
{
	m_iPlanetCount = 0;
	const double fAU = 1.496e11;

	// step_1.0 - 行星轨迹线初始化
	// 这里有个潜在的bug，每条行星轨迹线有256个顶点，当行星数量大于256颗时，顶点数量会超出这个最大值
	const size_t iMaxNum = 65536;

	std::string strLineVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "PlanetLineVert.glsl";
	std::string strLineFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "PlanetLineFrag.glsl";

	osg::ref_ptr<osg::LineWidth> pLineWidth = new osg::LineWidth;
	pLineWidth->setWidth(1);

	// 第2层级行星轨迹线
	m_pGeodePlanetsLine_2 = new osg::Geode();
	m_pStar_2_Transform->addChild(m_pGeodePlanetsLine_2.get());
	osg::ref_ptr<osg::Geometry> pPlanetLineGeom_2 = new osg::Geometry();
	pPlanetLineGeom_2->setUseVertexBufferObjects(true);
	pPlanetLineGeom_2->setUseDisplayList(false);
	pPlanetLineGeom_2->setDataVariance(osg::Object::STATIC);

	m_pPlanetLineVerts_2 = new osg::Vec3Array();
	m_pPlanetLineVerts_2->reserve(iMaxNum);
	m_pPlanetLineCoords_2 = new osg::Vec3Array;
	m_pPlanetLineCoords_2->reserve(iMaxNum);
	m_pPlanetLineElement_2 = new osg::DrawElementsUShort(GL_LINES);
	m_pPlanetLineElement_2->reserve(iMaxNum);

	osg::ref_ptr<osg::StateSet> pLineSS2 = m_pGeodePlanetsLine_2->getOrCreateStateSet();
	pLineSS2->setAttributeAndModes(pLineWidth.get(), osg::StateAttribute::ON);
	pLineSS2->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pLineSS2->setMode(GL_BLEND, osg::StateAttribute::ON);
	pLineSS2->setAttributeAndModes(new osg::BlendFunc(), osg::StateAttribute::ON);
	pLineSS2->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pLineSS2->setRenderBinDetails(BIN_PLANET_LINE, "DepthSortedBin");
	pLineSS2->addUniform(m_pTimesUniform.get());
	CGMKit::LoadShader(pLineSS2.get(), strLineVertPath, strLineFragPath);

	// 第3层级行星轨迹线
	m_pGeodePlanetsLine_3 = new osg::Geode();
	m_pStar_3_Transform->addChild(m_pGeodePlanetsLine_3.get());
	osg::ref_ptr<osg::Geometry> pPlanetLineGeom_3 = new osg::Geometry();
	pPlanetLineGeom_3->setUseVertexBufferObjects(true);
	pPlanetLineGeom_3->setUseDisplayList(false);
	pPlanetLineGeom_3->setDataVariance(osg::Object::STATIC);

	m_pPlanetLineVerts_3 = new osg::Vec3Array();
	m_pPlanetLineVerts_3->reserve(iMaxNum);
	m_pPlanetLineCoords_3 = new osg::Vec3Array;
	m_pPlanetLineCoords_3->reserve(iMaxNum);
	m_pPlanetLineElement_3 = new osg::DrawElementsUShort(GL_LINES);
	m_pPlanetLineElement_3->reserve(iMaxNum);

	osg::ref_ptr<osg::StateSet> pLineSS3 = m_pGeodePlanetsLine_3->getOrCreateStateSet();
	pLineSS3->setAttributeAndModes(pLineWidth.get(), osg::StateAttribute::ON);
	pLineSS3->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pLineSS3->setMode(GL_BLEND, osg::StateAttribute::ON);
	pLineSS3->setAttributeAndModes(new osg::BlendFunc(), osg::StateAttribute::ON);
	pLineSS3->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pLineSS3->setRenderBinDetails(BIN_PLANET_LINE, "DepthSortedBin");
	pLineSS3->addUniform(m_pTimesUniform.get());
	CGMKit::LoadShader(pLineSS3.get(), strLineVertPath, strLineFragPath);

	// step_1.5 - 行星初始化
	// 行星点精灵数量上限
	const size_t iMaxPointNum = 256;
	std::string strPlanetVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Planets_Vert.glsl";
	std::string strPlanetFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Planets_Frag.glsl";

	// 第2层级行星
	m_pGeodePlanets_2 = new osg::Geode();
	m_pStar_2_Transform->addChild(m_pGeodePlanets_2.get());
	osg::ref_ptr<osg::Geometry> pPlanetGeom_2 = new osg::Geometry();
	pPlanetGeom_2->setUseVertexBufferObjects(true);
	pPlanetGeom_2->setUseDisplayList(false);
	pPlanetGeom_2->setDataVariance(osg::Object::STATIC);

	m_pPlanetVertArray_2 = new osg::Vec4Array;
	m_pPlanetElement_2 = new osg::DrawElementsUShort(GL_POINTS);
	m_pPlanetVertArray_2->reserve(iMaxPointNum);
	m_pPlanetElement_2->reserve(iMaxPointNum);
	m_pPlanetVertArray_2->push_back(osg::Vec4(0, 0, 0, 1));
	m_pPlanetElement_2->push_back(0);

	osg::ref_ptr<osg::StateSet> pSSPlanets_2 = m_pGeodePlanets_2->getOrCreateStateSet();
	pSSPlanets_2->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
	pSSPlanets_2->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	pSSPlanets_2->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSPlanets_2->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSPlanets_2->setRenderBinDetails(BIN_PLANETS, "DepthSortedBin");
	pSSPlanets_2->addUniform(m_pTimesUniform.get());
	pSSPlanets_2->addUniform(m_pUnitRatioUniform.get());
	CGMKit::LoadShader(pSSPlanets_2.get(), strPlanetVertPath, strPlanetFragPath);

	// 第3层级行星
	m_pGeodePlanets_3 = new osg::Geode();
	m_pStar_3_Transform->addChild(m_pGeodePlanets_3.get());
	osg::ref_ptr<osg::Geometry> pPlanetGeom_3 = new osg::Geometry();
	pPlanetGeom_3->setUseVertexBufferObjects(true);
	pPlanetGeom_3->setUseDisplayList(false);
	pPlanetGeom_3->setDataVariance(osg::Object::STATIC);

	m_pPlanetVertArray_3 = new osg::Vec4Array;
	m_pPlanetElement_3 = new osg::DrawElementsUShort(GL_POINTS);
	m_pPlanetVertArray_3->reserve(iMaxPointNum);
	m_pPlanetElement_3->reserve(iMaxPointNum);
	m_pPlanetVertArray_3->push_back(osg::Vec4(0, 0, 0, 1));
	m_pPlanetElement_3->push_back(0);

	osg::ref_ptr<osg::StateSet> pSSPlanets_3 = m_pGeodePlanets_3->getOrCreateStateSet();
	pSSPlanets_3->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
	pSSPlanets_3->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	pSSPlanets_3->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSPlanets_3->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSPlanets_3->setRenderBinDetails(BIN_PLANETS, "DepthSortedBin");
	pSSPlanets_3->addUniform(m_pTimesUniform.get());
	pSSPlanets_3->addUniform(m_pUnitRatioUniform.get());
	CGMKit::LoadShader(pSSPlanets_3.get(), strPlanetVertPath, strPlanetFragPath);

	// step_2.0 - 添加行星
	// 水星
	_AddPlanet(fAU*0.3871, 0.241, 0);
	// 金星
	_AddPlanet(fAU*0.7233, 0.616, 1);
	// 地球
	_AddPlanet(fAU, 1.0, 3);
	// 火星
	_AddPlanet(fAU*1.52, 1.882, 5);
	// 木星
	_AddPlanet(fAU*5.20, 11.86, 7);
	// 土星
	_AddPlanet(fAU*9.55, 29.46, 11);
	// 天王星
	_AddPlanet(fAU*19.22, 84.01, 13);
	// 海王星
	_AddPlanet(fAU*30.11, 164.82, 17);

	// step_3.0 - 添加结束后，将行星轨迹线挂到对应节点下
	pPlanetLineGeom_2->setVertexArray(m_pPlanetLineVerts_2.get());
	pPlanetLineGeom_2->setTexCoordArray(0, m_pPlanetLineCoords_2.get());
	pPlanetLineGeom_2->addPrimitiveSet(m_pPlanetLineElement_2.get());
	m_pGeodePlanetsLine_2->addDrawable(pPlanetLineGeom_2.get());

	pPlanetLineGeom_3->setVertexArray(m_pPlanetLineVerts_3.get());
	pPlanetLineGeom_3->setTexCoordArray(0, m_pPlanetLineCoords_3.get());
	pPlanetLineGeom_3->addPrimitiveSet(m_pPlanetLineElement_3.get());
	m_pGeodePlanetsLine_3->addDrawable(pPlanetLineGeom_3.get());

	// step_3.5 - 添加结束后，将行星挂到对应节点下
	pPlanetGeom_2->setVertexArray(m_pPlanetVertArray_2.get());
	pPlanetGeom_2->addPrimitiveSet(m_pPlanetElement_2.get());
	m_pGeodePlanets_2->addDrawable(pPlanetGeom_2.get());

	pPlanetGeom_3->setVertexArray(m_pPlanetVertArray_3.get());
	pPlanetGeom_3->addPrimitiveSet(m_pPlanetElement_3.get());
	m_pGeodePlanets_3->addDrawable(pPlanetGeom_3.get());

	return true;
}

bool CGMGalaxy::_AddPlanet(const double fRadius, const double fOrbitalPeriod, const double fStartPos)
{
	double fR2 = fRadius / m_pKernelData->fUnitArray->at(2);
	double fR3 = fRadius / m_pKernelData->fUnitArray->at(3);

	// 添加行星轨迹线
	const int iAngleSegments = 128;	// 角度分段数
	const int iVertex = iAngleSegments * 2;	// 顶点数量
	float fAngleEach = osg::PI * 2.0 / float(iAngleSegments);
	for (int x = 0; x < iAngleSegments; x++)
	{
		for (int i = 0; i < 2; i++)
		{
			float f = x + i;
			float fX = cos(fAngleEach * f + fStartPos);
			float fY = sin(fAngleEach * f + fStartPos);

			m_pPlanetLineVerts_2->push_back(osg::Vec3(fR2*fX, fR2*fY, 0));
			m_pPlanetLineCoords_2->push_back(osg::Vec3f(f / float(iAngleSegments), fOrbitalPeriod, float(m_iPlanetCount)));
			m_pPlanetLineElement_2->push_back(m_iPlanetCount * iVertex + 2 * x + i);

			m_pPlanetLineVerts_3->push_back(osg::Vec3(fR3*fX, fR3*fY, 0));
			m_pPlanetLineCoords_3->push_back(osg::Vec3f(f / float(iAngleSegments), fOrbitalPeriod, float(m_iPlanetCount)));
			m_pPlanetLineElement_3->push_back(m_iPlanetCount * iVertex + 2 * x + i);
		}
	}

	// 添加行星点精灵
	float fStartX = cos(fStartPos);
	float fStartY = sin(fStartPos);
	m_pPlanetVertArray_2->push_back(osg::Vec4(fR2 * fStartX, fR2 * fStartY, float(1 + m_iPlanetCount), fOrbitalPeriod));
	m_pPlanetElement_2->push_back(1 + m_iPlanetCount);
	m_pPlanetVertArray_3->push_back(osg::Vec4(fR3 * fStartX, fR3 * fStartY, float(1 + m_iPlanetCount), fOrbitalPeriod));
	m_pPlanetElement_3->push_back(1 + m_iPlanetCount);

	m_iPlanetCount++;
	return true;
}

bool CGMGalaxy::_CreateOortCloud()
{
	m_pOort_4_Transform = new osg::PositionAttitudeTransform();
	m_pHierarchyRootVector.at(4)->addChild(m_pOort_4_Transform.get());

	m_pOortCloudGeode_3 = new osg::Geode;
	osg::ref_ptr<const osg::EllipsoidModel>	_ellipsoidModel_3 = new osg::EllipsoidModel(20.0, 20.0);
	osg::ref_ptr<osg::Geometry> pDrawable_3 = _MakeEllipsoidGeometry(_ellipsoidModel_3, 32, 16, 0, true, true, true);
	m_pOortCloudGeode_3->addDrawable(pDrawable_3.get());
	m_pStar_3_Transform->addChild(m_pOortCloudGeode_3.get());

	osg::ref_ptr<osg::Geode> pOortCloudGeode_4 = new osg::Geode;
	osg::ref_ptr<const osg::EllipsoidModel>	_ellipsoidModel_4 = new osg::EllipsoidModel(2e-4, 2e-4);
	osg::ref_ptr<osg::Geometry> pDrawable_4 = _MakeEllipsoidGeometry(_ellipsoidModel_4, 32, 16, 0, true, true, true);
	pOortCloudGeode_4->addDrawable(pDrawable_4.get());
	m_pOort_4_Transform->addChild(pOortCloudGeode_4.get());

	osg::ref_ptr<osg::StateSet>	pSS_3 = m_pOortCloudGeode_3->getOrCreateStateSet();
	pSS_3->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSS_3->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSS_3->setAttributeAndModes(new osg::CullFace());
	pSS_3->setAttributeAndModes(new osg::BlendFunc(), osg::StateAttribute::ON);
	pSS_3->setRenderBinDetails(BIN_STAR_PLAYING, "DepthSortedBin");
	pSS_3->addUniform(m_pStarColorUniform.get());
	pSS_3->addUniform(m_pStarHiePosUniform.get());
	pSS_3->addUniform(m_vScreenSizeUniform.get());
	pSS_3->addUniform(m_pLevelArrayUniform.get());
	pSS_3->addUniform(m_pTimesUniform.get());
	int iUnit3 = 0;
	pSS_3->setTextureAttributeAndModes(iUnit3, m_3DShapeTex_128.get());
	pSS_3->addUniform(new osg::Uniform("shapeNoiseTex", iUnit3));
	iUnit3++;
	pSS_3->setTextureAttributeAndModes(iUnit3, m_blueNoiseTex.get());
	pSS_3->addUniform(new osg::Uniform("blueNoiseSampler", iUnit3));
	iUnit3++;

	osg::ref_ptr<osg::StateSet>	pSS_4 = m_pOort_4_Transform->getOrCreateStateSet();
	pSS_4->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSS_4->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSS_4->setAttributeAndModes(new osg::CullFace());
	pSS_4->setAttributeAndModes(new osg::BlendFunc(), osg::StateAttribute::ON);
	pSS_4->setRenderBinDetails(BIN_STAR_PLAYING, "DepthSortedBin");
	pSS_4->addUniform(m_pStarColorUniform.get());
	pSS_4->addUniform(m_pStarHiePosUniform.get());
	pSS_4->addUniform(m_vScreenSizeUniform.get());
	pSS_4->addUniform(m_pLevelArrayUniform.get());
	pSS_4->addUniform(m_pTimesUniform.get());
	int iUnit4 = 0;
	pSS_4->setTextureAttributeAndModes(iUnit4, m_3DShapeTex_128.get());
	pSS_4->addUniform(new osg::Uniform("shapeNoiseTex", iUnit4));
	iUnit4++;
	pSS_4->setTextureAttributeAndModes(iUnit4, m_blueNoiseTex.get());
	pSS_4->addUniform(new osg::Uniform("blueNoiseSampler", iUnit4));
	iUnit4++;

	// 添加shader
	std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "OortCloudVert.glsl";
	std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "OortCloudFrag.glsl";
	CGMKit::LoadShader(pSS_3.get(), strVertPath, strFragPath);
	CGMKit::LoadShader(pSS_4.get(), strVertPath, strFragPath);

	return true;
}

bool CGMGalaxy::_CreateAudioPoints()
{
	m_pGeodeAudio = new osg::Geode();
	m_pHierarchyRootVector.at(4)->addChild(m_pGeodeAudio.get());

	// 音频文件的星辰坐标Vector
	std::vector<SGMStarCoord> coordVector;
	GM_ENGINE_PTR->GetDataManager()->GetStarCoordVector(coordVector);

	// 从数据管理模块读取数据，创建未激活状态的音频星几何体
	osg::ref_ptr<osg::Geometry> pGeomAudio = _CreateAudioGeometry(coordVector);
	if (!pGeomAudio.valid()) return false;
	m_pGeodeAudio->addDrawable(pGeomAudio.get());

	osg::ref_ptr<osg::StateSet> pStateSetAudio = m_pGeodeAudio->getOrCreateStateSet();
	pStateSetAudio->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
	pStateSetAudio->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	pStateSetAudio->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pStateSetAudio->setMode(GL_BLEND, osg::StateAttribute::ON);
	pStateSetAudio->setAttributeAndModes(new osg::BlendFunc(), osg::StateAttribute::ON);
	pStateSetAudio->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pStateSetAudio->setRenderBinDetails(BIN_AUDIO_POINT, "DepthSortedBin");

	pStateSetAudio->addUniform(m_pMousePosUniform.get());
	pStateSetAudio->addUniform(m_pStarHiePosUniform.get());
	pStateSetAudio->addUniform(m_pLevelArrayUniform.get());
	pStateSetAudio->addUniform(m_pStarAlphaUniform.get());

	// 添加shader
	std::string strAudioVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "AudioVert.glsl";
	std::string strAudioFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "AudioFrag.glsl";
	return CGMKit::LoadShader(pStateSetAudio.get(), strAudioVertPath, strAudioFragPath);
}

bool CGMGalaxy::_CreateGalaxyPoints()
{
	// 4级空间下的星系半径
	double fGalaxyRadius4 = m_fGalaxyRadius / m_pKernelData->fUnitArray->at(4);

	osg::ref_ptr<osg::Geode> pGalaxyPointsGeode = new osg::Geode();
	m_pHierarchyRootVector.at(4)->addChild(pGalaxyPointsGeode.get());

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();

	size_t iNum = 65536;
	osg::ref_ptr<osg::Vec3Array> vertArray = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec2Array> texcoordArray = new osg::Vec2Array;
	osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array();
	osg::ref_ptr<osg::DrawElementsUShort> el = new osg::DrawElementsUShort(GL_POINTS);
	vertArray->reserve(iNum);
	texcoordArray->reserve(iNum);
	colorArray->reserve(iNum);
	el->reserve(iNum);

	int x = 0;
	while (x < iNum)
	{
		float fRandomX = (m_iRandom() % 10000)*0.0001f - 0.5f;
		float fRandomY = (m_iRandom() % 10000)*0.0001f - 0.5f;
		float fRandomAlpha = (m_iRandom() % 1000)*0.001f;
		float fX = fGalaxyRadius4 * 2.0f * fRandomX;
		float fY = fGalaxyRadius4 * 2.0f * fRandomY;
		float fU = fRandomX + 0.5f;
		float fV = fRandomY + 0.5f;

		float fA = _GetGalaxyValue(fU, fV, 3);
		if (fRandomAlpha < fA)
		{
			float fRandomR = (m_iRandom() % 100)*0.01f - 0.5f;
			float fR = max(0.0f, _GetGalaxyValue(fU, fV, 0) + fRandomR * fRandomR * fRandomR);
			float fG = _GetGalaxyValue(fU, fV, 1);
			float fB = _GetGalaxyValue(fU, fV, 2);

			float fRGBMax = max(max(max(fR, fG), fB), 1e-5);
			fR /= fRGBMax;
			fG /= fRGBMax;
			fB /= fRGBMax;

			float fRandomZ = (m_iRandom() % 1000)*0.002f - 1.0f;
			float fSignZ = (fRandomZ > 0) ? 1.0f : -1.0f;
			float fSmooth = fSignZ * (3 * fRandomZ*fRandomZ - 2 * abs(fRandomZ*fRandomZ*fRandomZ));
			float fZ = (fRandomAlpha + 0.2f)*0.05f*fGalaxyRadius4*fSmooth;
			float fRandomRadius = (m_iRandom() % 1000)*0.001f;
			fRandomRadius = (fA + _GetGalaxyHeight(fU, fV)) * fRandomRadius * fRandomRadius;
			float fRadiusNow = osg::Vec2(fRandomX, fRandomY).length();
			float fTmp = pow(min(1.0f, 1.03f*(1.0f - fRadiusNow)), 11);
			fZ = fZ * (0.5 + 3 * fTmp*fTmp - 2 * fTmp*fTmp*fTmp);
			vertArray->push_back(osg::Vec3(fX, fY, fZ));
			texcoordArray->push_back(osg::Vec2(fU, fV));
			colorArray->push_back(osg::Vec4(fR, fG, fB, fRandomRadius));
			el->push_back(x);
			x++;
		}
	}

	pGeometry->setVertexArray(vertArray.get());
	pGeometry->setTexCoordArray(0, texcoordArray.get());
	pGeometry->setColorArray(colorArray.get());
	pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 1, 0));
	pGeometry->setNormalArray(normal.get());
	pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
	pGeometry->addPrimitiveSet(el.get());

	pGeometry->setUseVertexBufferObjects(true);
	pGeometry->setUseDisplayList(false);
	pGeometry->setDataVariance(osg::Object::DYNAMIC);

	pGalaxyPointsGeode->addDrawable(pGeometry.get());
	m_pStateSetGalaxy = pGalaxyPointsGeode->getOrCreateStateSet();

	m_pStateSetGalaxy->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);

	m_pStateSetGalaxy->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	m_pStateSetGalaxy->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	m_pStateSetGalaxy->setMode(GL_BLEND, osg::StateAttribute::ON);
	osg::ref_ptr<osg::BlendEquation> blendEqua = new osg::BlendEquation(osg::BlendEquation::RGBA_MAX);
	m_pStateSetGalaxy->setAttributeAndModes(blendEqua.get(), osg::StateAttribute::ON);
	m_pStateSetGalaxy->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	m_pStateSetGalaxy->setRenderBinDetails(BIN_STARS, "DepthSortedBin");

	if (m_pConfigData->bHighQuality)
	{
		m_pStateSetGalaxy->setDefine("HIGH_QUALITY", osg::StateAttribute::ON);

		m_pStateSetGalaxy->setTextureAttributeAndModes(1, m_TAADistanceMap_0.get());
		m_pStateSetGalaxy->addUniform(new osg::Uniform("distanceTex", 1));

		m_pStateSetGalaxy->addUniform(m_vScreenSizeUniform.get());
	}

	m_pStateSetGalaxy->addUniform(m_pStarHiePosUniform.get());
	m_pStateSetGalaxy->addUniform(m_pLevelArrayUniform.get());
	m_pStateSetGalaxy->addUniform(m_pStarColorUniform.get());
	m_pStateSetGalaxy->addUniform(m_pStarAlphaUniform.get());

	// 添加shader
	std::string strStarVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyStarVert.glsl";
	std::string strStarFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyStarFrag.glsl";
	return CGMKit::LoadShader(m_pStateSetGalaxy.get(), strStarVertPath, strStarFragPath);
}

bool CGMGalaxy::_CreateStarCube_4()
{
	float fCubeMinSize = GM_MIN_STARS_CUBE / m_pKernelData->fUnitArray->at(4);

	if (!m_pGeodeStarCube_4.valid())
	{
		m_pGeodeStarCube_4 = new osg::Geode();
		m_pEye_4_Transform->addChild(m_pGeodeStarCube_4.get());
		osg::ref_ptr<osg::StateSet> pSS_4 = m_pGeodeStarCube_4->getOrCreateStateSet();

		pSS_4->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
		pSS_4->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
		pSS_4->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pSS_4->setMode(GL_BLEND, osg::StateAttribute::ON);
		osg::ref_ptr<osg::BlendEquation> blendEqua = new osg::BlendEquation(osg::BlendEquation::RGBA_MAX);
		pSS_4->setAttributeAndModes(blendEqua.get(), osg::StateAttribute::ON);
		pSS_4->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
		pSS_4->setRenderBinDetails(BIN_STARS, "DepthSortedBin");

		// x = cubeMinSize, y = targetDistance
		m_pStarsCubeInfoUniform = new osg::Uniform("cubeInfo", osg::Vec2f(fCubeMinSize, 1.0f));
		pSS_4->addUniform(m_pStarsCubeInfoUniform.get());
		pSS_4->addUniform(m_pStarColorUniform.get());
		pSS_4->addUniform(m_pShapeUVWUniform.get());
		pSS_4->addUniform(m_pStarAlphaUniform.get());

		int iUnit = 1;
		pSS_4->setTextureAttributeAndModes(iUnit, m_3DShapeTex_128.get());
		pSS_4->addUniform(new osg::Uniform("shapeNoiseTex", iUnit));
		iUnit++;

		// 添加shader
		std::string strStarVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarCube_4_Vert.glsl";
		std::string strStarFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarCube_4_Frag.glsl";
		CGMKit::LoadShader(pSS_4.get(), strStarVertPath, strStarFragPath);	
	}

	size_t iNum = m_pCubeVertArray->size();
	for (int i = 0; i < 4; i++)
	{
		float fCubeSize = fCubeMinSize * std::pow(2,float(i));

		osg::ref_ptr<osg::Vec4Array> vertArray = new osg::Vec4Array;
		vertArray->reserve(iNum);
		for (size_t i = 0; i < iNum; i++)
		{
			osg::Vec4 vert = m_pCubeVertArray->at(i);
			vertArray->push_back(vert*fCubeSize);
		}

		osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
		pGeometry->setNodeMask(0);
		pGeometry->setVertexArray(vertArray.get());
		pGeometry->setColorArray(m_pCubeColorArray.get());
		pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

		osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
		normal->push_back(osg::Vec3(0, 1, 0));
		pGeometry->setNormalArray(normal.get());
		pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
		pGeometry->addPrimitiveSet(m_pCubeElement.get());

		pGeometry->setUseVertexBufferObjects(true);
		pGeometry->setUseDisplayList(false);
		pGeometry->setDataVariance(osg::Object::DYNAMIC);

		m_pStarsGeomVector.push_back(pGeometry);
		m_pGeodeStarCube_4->addDrawable(pGeometry.get());
	}

	return true;
}

bool CGMGalaxy::_CreateGalaxyPointsN_4(int iDens)
{
	/**
	* 创建N倍密度的PointSprite星系点阵
	* 虽然最终渲染的范围是1/N个星系的尺寸
	* 然后在vertex shader中进行平移和缩放
	* 点阵的位置根据三维纹理设置，保证看不出重复感
	*/
	if (!m_pGeodePointsN_4.valid())
	{
		m_pGeodePointsN_4 = new osg::Geode();
		m_pHierarchyRootVector.at(4)->addChild(m_pGeodePointsN_4.get());
		osg::ref_ptr<osg::StateSet> pSSN = m_pGeodePointsN_4->getOrCreateStateSet();

		pSSN->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
		pSSN->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
		pSSN->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pSSN->setMode(GL_BLEND, osg::StateAttribute::ON);
		osg::ref_ptr<osg::BlendEquation> blendEqua = new osg::BlendEquation(osg::BlendEquation::RGBA_MAX);
		pSSN->setAttributeAndModes(blendEqua.get(), osg::StateAttribute::ON);
		pSSN->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
		pSSN->setRenderBinDetails(BIN_STARS, "DepthSortedBin");

		int iUnit = 1;
		if (m_pConfigData->bHighQuality)
		{
			pSSN->setDefine("HIGH_QUALITY", osg::StateAttribute::ON);

			pSSN->setTextureAttributeAndModes(iUnit, m_TAADistanceMap_0.get());
			pSSN->addUniform(new osg::Uniform("distanceTex", iUnit));
			iUnit++;
			pSSN->addUniform(m_vScreenSizeUniform.get());
		}
		pSSN->setTextureAttributeAndModes(iUnit, m_pGalaxyColorTex.get());
		pSSN->addUniform(new osg::Uniform("galaxyTex", iUnit));
		iUnit++;

		pSSN->addUniform(m_pStarHiePosUniform.get());
		pSSN->addUniform(m_pLevelArrayUniform.get());
		pSSN->addUniform(m_pStarColorUniform.get());
		pSSN->addUniform(m_pGalaxyRadiusUniform.get());
		pSSN->addUniform(m_pStarAlphaUniform.get());

		// 添加shader
		std::string strStarVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarNVert.glsl";
		std::string strStarFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarNFrag.glsl";
		CGMKit::LoadShader(pSSN.get(), strStarVertPath, strStarFragPath);
	}

	osg::Vec3f vUVW;
	m_pShapeUVWUniform->get(vUVW);

	float fDens = float(osg::clampBetween(iDens, 2, 16));
	// 4级空间下的星系半径
	double fGalaxyRadius4 = m_fGalaxyRadius / m_pKernelData->fUnitArray->at(4);
	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
	size_t iNum = 65536;
	osg::ref_ptr<osg::Vec3Array> vertArray = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> texcoordArray = new osg::Vec4Array;
	osg::ref_ptr<osg::DrawElementsUShort> el = new osg::DrawElementsUShort(GL_POINTS);
	vertArray->reserve(iNum);
	texcoordArray->reserve(iNum);
	el->reserve(iNum);

	float fMinAlpha = 0.25f;
	int x = 0;
	while (x < iNum)
	{
		float fU = (m_iRandom() % 10000)*1e-4f;
		float fV = (m_iRandom() % 10000)*1e-4f;
		float fW = (m_iRandom() % 10000)*1e-4f;
		float fDiameter = fGalaxyRadius4 * 2.0f;
		float fUScale = fDiameter * vUVW.x() / fDens;
		float fVScale = fDiameter * vUVW.y() / fDens;
		float fAlpha = 1.0f - _Get3DValue(fU * fUScale, fV * fVScale, fW);
		fAlpha *= fAlpha;
		if (fAlpha > fMinAlpha)
		{
			float fRandomX = fU - 0.5f;
			float fRandomY = fV - 0.5f;
			float fRandomZ = fW - 0.5f;
			float fX = fDiameter * fRandomX;
			float fY = fDiameter * fRandomY;
			float fZ = fRandomZ / vUVW.z();

			vertArray->push_back(osg::Vec3(fX, fY, fZ));
			// fU、fV不需要传入，所以在UV的位置上存放缩放系数
			texcoordArray->push_back(osg::Vec4(fDens, fDens, fW, (fAlpha - fMinAlpha)/(1.0f - fMinAlpha)));
			el->push_back(x);
			x++;
		}
	}

	pGeometry->setVertexArray(vertArray.get());
	pGeometry->setTexCoordArray(0, texcoordArray.get());

	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 1, 0));
	pGeometry->setNormalArray(normal.get());
	pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
	pGeometry->addPrimitiveSet(el.get());

	pGeometry->setUseVertexBufferObjects(true);
	pGeometry->setUseDisplayList(false);
	pGeometry->setDataVariance(osg::Object::DYNAMIC);

	m_pGeodePointsN_4->addDrawable(pGeometry.get());
	
	return true;
}

bool CGMGalaxy::_CreateGalaxyPlane_4()
{
	// 4级空间下的星系半径
	double fGalaxyRadius4 = m_fGalaxyRadius / m_pKernelData->fUnitArray->at(4);

	osg::ref_ptr<osg::Geode> pGalaxyPlaneGeode = new osg::Geode();
	m_pHierarchyRootVector.at(4)->addChild(pGalaxyPlaneGeode.get());

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
	osg::ref_ptr<osg::Vec3Array> verArray = new osg::Vec3Array;
	verArray->push_back(osg::Vec3(-fGalaxyRadius4, -fGalaxyRadius4, 0));
	verArray->push_back(osg::Vec3(fGalaxyRadius4, -fGalaxyRadius4, 0));
	verArray->push_back(osg::Vec3(fGalaxyRadius4, fGalaxyRadius4, 0));
	verArray->push_back(osg::Vec3(-fGalaxyRadius4, fGalaxyRadius4, 0));
	pGeometry->setVertexArray(verArray);

	osg::ref_ptr<osg::Vec2Array> textArray = new osg::Vec2Array;
	textArray->push_back(osg::Vec2(0, 0));
	textArray->push_back(osg::Vec2(1, 0));
	textArray->push_back(osg::Vec2(1, 1));
	textArray->push_back(osg::Vec2(0, 1));
	pGeometry->setTexCoordArray(0, textArray);

	osg::ref_ptr < osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 0, 1));
	pGeometry->setNormalArray(normal);
	pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

	pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

	pGeometry->setUseVertexBufferObjects(true);
	pGeometry->setUseDisplayList(false);
	pGeometry->setDataVariance(osg::Object::DYNAMIC);

	pGalaxyPlaneGeode->addDrawable(pGeometry.get());

	m_pStateSetPlane = pGalaxyPlaneGeode->getOrCreateStateSet();
	m_pStateSetPlane->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	m_pStateSetPlane->setMode(GL_BLEND, osg::StateAttribute::ON);
	m_pStateSetPlane->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	m_pStateSetPlane->setAttributeAndModes(new osg::BlendFunc(), osg::StateAttribute::ON);
	m_pStateSetPlane->setRenderBinDetails(BIN_GALAXY_PLANE, "DepthSortedBin");

	int iUnit = 0;
	m_pStateSetPlane->setTextureAttributeAndModes(iUnit, m_pGalaxyColorTex.get());
	m_pStateSetPlane->addUniform(new osg::Uniform("galaxyTex", iUnit));
	iUnit++;

	m_pStateSetPlane->addUniform(m_pLevelArrayUniform.get());
	m_pStateSetPlane->addUniform(m_pMousePosUniform.get());

	// 添加shader
	std::string strGalaxyPlaneVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyPlaneVert.glsl";
	std::string strGalaxyPlaneFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyPlaneFrag.glsl";
	return CGMKit::LoadShader(m_pStateSetPlane.get(), strGalaxyPlaneVertPath, strGalaxyPlaneFragPath);
}

bool CGMGalaxy::_CreateGalaxies_4()
{
	size_t iMaxNum = m_pCubeVertArray->size();

	if (!m_pGeodeGalaxyGroup_4.valid())
	{
		m_pGeodeGalaxyGroup_4 = new osg::Geode();
		m_pEye_4_Transform->addChild(m_pGeodeGalaxyGroup_4.get());
		osg::ref_ptr<osg::StateSet> pSSG_4 = m_pGeodeGalaxyGroup_4->getOrCreateStateSet();

		pSSG_4->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
		pSSG_4->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
		pSSG_4->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pSSG_4->setMode(GL_BLEND, osg::StateAttribute::ON);
		osg::ref_ptr<osg::BlendEquation> blendEqua = new osg::BlendEquation(osg::BlendEquation::RGBA_MAX);
		pSSG_4->setAttributeAndModes(blendEqua.get(), osg::StateAttribute::ON);
		pSSG_4->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
		pSSG_4->setRenderBinDetails(BIN_GALAXIES, "DepthSortedBin");

		pSSG_4->addUniform(m_pStarAlphaUniform.get());

		int iUnit = 1;
		pSSG_4->setTextureAttributeAndModes(iUnit, m_pGalaxiesTex.get());
		pSSG_4->addUniform(new osg::Uniform("galaxiesTex", iUnit));
		iUnit++;

		// 添加shader
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxies_4_Vert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxies_4_Frag.glsl";
		CGMKit::LoadShader(pSSG_4.get(), strVertPath, strFragPath);
	}

	osg::ref_ptr<osg::Vec4Array> vertArray = new osg::Vec4Array;
	osg::ref_ptr<osg::Vec4Array> galaxiesInfoArray = new osg::Vec4Array;
	vertArray->reserve(iMaxNum);
	galaxiesInfoArray->reserve(iMaxNum);

	int iEleNum = 0;
	for (size_t i = 0; i < iMaxNum; i++)
	{
		osg::Vec4 vPos = m_pCubeVertArray->at(i);
		osg::Vec3 vVertPos = osg::Vec3(vPos.x(), vPos.y(), vPos.z());
		float fLength = vVertPos.normalize();
		if (fLength <= 0.5f)
		{
			float fFalloff = 0.75f*osg::clampBetween(1.0f - fLength / 0.5f, 0.0f, 1.0f);
			osg::Vec4 vert = osg::Vec4(vVertPos, fFalloff);
			vertArray->push_back(vert);
			osg::Vec4 info = m_pGalaxiesInfo->at(i);
			galaxiesInfoArray->push_back(info);
		}
	}

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
	pGeometry->setVertexArray(vertArray.get());
	pGeometry->setColorArray(galaxiesInfoArray.get());
	pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 1, 0));
	pGeometry->setNormalArray(normal.get());
	pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
	pGeometry->addPrimitiveSet(m_pSphereElement.get());

	pGeometry->setUseVertexBufferObjects(true);
	pGeometry->setUseDisplayList(false);
	pGeometry->setDataVariance(osg::Object::DYNAMIC);

	m_pGeodeGalaxyGroup_4->addDrawable(pGeometry.get());

	return true;
}

bool CGMGalaxy::_CreateGalaxyPlane_5()
{
	// 5级空间下的星系半径
	double fGalaxyRadius5 = m_fGalaxyRadius / m_pKernelData->fUnitArray->at(5);

	osg::ref_ptr<osg::Geode> pGalaxy_5_Geode = new osg::Geode();
	m_pHierarchyRootVector.at(5)->addChild(pGalaxy_5_Geode.get());

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
	osg::ref_ptr<osg::Vec3Array> verArray = new osg::Vec3Array;
	verArray->push_back(osg::Vec3(-fGalaxyRadius5, -fGalaxyRadius5, 0));
	verArray->push_back(osg::Vec3(fGalaxyRadius5, -fGalaxyRadius5, 0));
	verArray->push_back(osg::Vec3(fGalaxyRadius5, fGalaxyRadius5, 0));
	verArray->push_back(osg::Vec3(-fGalaxyRadius5, fGalaxyRadius5, 0));
	pGeometry->setVertexArray(verArray);

	osg::ref_ptr<osg::Vec2Array> textArray = new osg::Vec2Array;
	textArray->push_back(osg::Vec2(0, 0));
	textArray->push_back(osg::Vec2(1, 0));
	textArray->push_back(osg::Vec2(1, 1));
	textArray->push_back(osg::Vec2(0, 1));
	pGeometry->setTexCoordArray(0, textArray);

	osg::ref_ptr< osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 0, 1));
	pGeometry->setNormalArray(normal);
	pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

	pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

	pGeometry->setUseVertexBufferObjects(true);
	pGeometry->setUseDisplayList(false);
	pGeometry->setDataVariance(osg::Object::DYNAMIC);

	pGalaxy_5_Geode->addDrawable(pGeometry.get());

	//!< 5级空间下银河盘面的状态集
	osg::ref_ptr<osg::StateSet>	pStateSet_5 = pGalaxy_5_Geode->getOrCreateStateSet();
	pStateSet_5->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pStateSet_5->setMode(GL_BLEND, osg::StateAttribute::ON);
	pStateSet_5->setAttributeAndModes(new osg::BlendFunc(), osg::StateAttribute::ON);
	pStateSet_5->setRenderBinDetails(BIN_GALAXY_PLANE, "DepthSortedBin");

	std::string strGalaxyTexPath = m_pConfigData->strCorePath + m_strGalaxyTexPath;
	osg::ref_ptr<osg::Texture> _galaxyTex = _CreateTexture2D(strGalaxyTexPath + "milkyWay.tga", 4);
	int iUnit = 0;
	pStateSet_5->setTextureAttributeAndModes(iUnit, _galaxyTex.get());
	pStateSet_5->addUniform(new osg::Uniform("galaxyTex", iUnit));
	iUnit++;

	// 添加shader
	std::string strGalaxyPlaneVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxy_5_Vert.glsl";
	std::string strGalaxyPlaneFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxy_5_Frag.glsl";
	return CGMKit::LoadShader(pStateSet_5.get(), strGalaxyPlaneVertPath, strGalaxyPlaneFragPath);
}

bool CGMGalaxy::_CreateGalaxies_5()
{
	if (!m_pEye_5_Transform.valid())
	{
		m_pEye_5_Transform = new osg::PositionAttitudeTransform();
		m_pHierarchyRootVector.at(5)->addChild(m_pEye_5_Transform.get());
	}

	size_t iNum = m_pCubeVertArray->size();
	double fCubeMinSize = GM_MIN_GALAXIES_CUBE / m_pKernelData->fUnitArray->at(5);
	m_pGalaxiesInfoUniform = new osg::Uniform("cubeInfo", osg::Vec2f(fCubeMinSize, 1.0f));

	// 第0层星系群，单独处理
	if (!m_pGalaxyGroup_Transform.valid())
	{
		m_pGalaxyGroup_Transform = new osg::PositionAttitudeTransform();
		m_pHierarchyRootVector.at(5)->addChild(m_pGalaxyGroup_Transform.get());

		m_pGeodeGalaxyGroup_5 = new osg::Geode();
		m_pGalaxyGroup_Transform->addChild(m_pGeodeGalaxyGroup_5.get());
		osg::ref_ptr<osg::StateSet> pSSGroup = m_pGeodeGalaxyGroup_5->getOrCreateStateSet();

		pSSGroup->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
		pSSGroup->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
		pSSGroup->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pSSGroup->setMode(GL_BLEND, osg::StateAttribute::ON);
		osg::ref_ptr<osg::BlendEquation> blendEqua = new osg::BlendEquation(osg::BlendEquation::RGBA_MAX);
		pSSGroup->setAttributeAndModes(blendEqua.get(), osg::StateAttribute::ON);
		pSSGroup->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
		pSSGroup->setRenderBinDetails(BIN_GALAXIES, "DepthSortedBin");

		pSSGroup->addUniform(m_pGalaxiesInfoUniform.get());

		int iUnit = 1;
		pSSGroup->setTextureAttributeAndModes(iUnit, m_pGalaxiesTex.get());
		pSSGroup->addUniform(new osg::Uniform("galaxiesTex", iUnit));
		iUnit++;

		// 添加shader
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxies_5_Vert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxies_5_Frag.glsl";
		CGMKit::LoadShader(pSSGroup.get(), strVertPath, strFragPath);

		osg::ref_ptr<osg::Vec4Array> vertArray = new osg::Vec4Array;
		vertArray->reserve(iNum);
		for (size_t i = 0; i < iNum; i++)
		{
			osg::Vec4 vert = m_pCubeVertArray->at(i);
			vertArray->push_back(vert*fCubeMinSize);
		}

		osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
		pGeometry->setNodeMask(0);
		pGeometry->setVertexArray(vertArray.get());
		pGeometry->setColorArray(m_pGalaxiesInfo.get());
		pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

		osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
		normal->push_back(osg::Vec3(0, 1, 0));
		pGeometry->setNormalArray(normal.get());
		pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
		pGeometry->addPrimitiveSet(m_pCubeElement.get());

		pGeometry->setUseVertexBufferObjects(true);
		pGeometry->setUseDisplayList(false);
		pGeometry->setDataVariance(osg::Object::DYNAMIC);

		// 注意：这里必须是将最小星系群放在第一个位置，否则显影会出问题
		m_pGalaxiesGeomVector.push_back(pGeometry);
		m_pGeodeGalaxyGroup_5->addDrawable(pGeometry.get());
	}

	if (!m_pGeodeGalaxies_5.valid())
	{
		m_pGeodeGalaxies_5 = new osg::Geode();
		m_pEye_5_Transform->addChild(m_pGeodeGalaxies_5.get());
		osg::ref_ptr<osg::StateSet> pSSG_5 = m_pGeodeGalaxies_5->getOrCreateStateSet();

		pSSG_5->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
		pSSG_5->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
		pSSG_5->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pSSG_5->setMode(GL_BLEND, osg::StateAttribute::ON);
		osg::ref_ptr<osg::BlendEquation> blendEqua = new osg::BlendEquation(osg::BlendEquation::RGBA_MAX);
		pSSG_5->setAttributeAndModes(blendEqua.get(), osg::StateAttribute::ON);
		pSSG_5->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
		pSSG_5->setRenderBinDetails(BIN_GALAXIES, "DepthSortedBin");

		pSSG_5->addUniform(m_pGalaxiesInfoUniform.get());

		int iUnit = 1;
		pSSG_5->setTextureAttributeAndModes(iUnit, m_pGalaxiesTex.get());
		pSSG_5->addUniform(new osg::Uniform("galaxiesTex", iUnit));
		iUnit++;

		// 添加shader
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxies_5_Vert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxies_5_Frag.glsl";
		CGMKit::LoadShader(pSSG_5.get(), strVertPath, strFragPath);
	}

	// 在眼点变换节点下，只创建第1、2、3层星系群，第0层星系群，单独处理
	for (int i = 1; i < 4; i++)
	{
		float fCubeSize = fCubeMinSize*std::pow(2, float(i));

		osg::ref_ptr<osg::Vec4Array> vertArray = new osg::Vec4Array;
		vertArray->reserve(iNum);
		for (size_t i = 0; i < iNum; i++)
		{
			osg::Vec4 vert = m_pCubeVertArray->at(i);
			vertArray->push_back(vert*fCubeSize);
		}

		osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
		pGeometry->setNodeMask(0);
		pGeometry->setVertexArray(vertArray.get());
		pGeometry->setColorArray(m_pGalaxiesInfo.get());
		pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

		osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
		normal->push_back(osg::Vec3(0, 1, 0));
		pGeometry->setNormalArray(normal.get());
		pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
		pGeometry->addPrimitiveSet(m_pCubeElement.get());

		pGeometry->setUseVertexBufferObjects(true);
		pGeometry->setUseDisplayList(false);
		pGeometry->setDataVariance(osg::Object::DYNAMIC);

		m_pGalaxiesGeomVector.push_back(pGeometry);
		m_pGeodeGalaxies_5->addDrawable(pGeometry.get());
	}

	return true;
}

bool CGMGalaxy::_CreateSupercluster()
{
	const float fSuperclusterSize = 0.5f;

	m_pGeodeSupercluster = new osg::Geode;
	m_pGeodeSupercluster->setNodeMask(0);// 默认先隐藏
	m_pGeodeSupercluster->addDrawable(MakeBoxGeometry(fSuperclusterSize, fSuperclusterSize, fSuperclusterSize));
	m_pHierarchyRootVector.at(5)->addChild(m_pGeodeSupercluster.get());

	osg::ref_ptr<osg::StateSet> pSS = m_pGeodeSupercluster->getOrCreateStateSet();
	pSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSS->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSS->setAttributeAndModes(new osg::BlendFunc(), osg::StateAttribute::ON);
	pSS->setAttributeAndModes(new osg::CullFace());
	pSS->setRenderBinDetails(BIN_SUPERCLUSTER, "DepthSortedBin");

	SGMVolumeRange sVR;
	sVR.fXMin = - fSuperclusterSize * 0.5f;
	sVR.fXMax = fSuperclusterSize * 0.5f;
	sVR.fYMin = -fSuperclusterSize * 0.5f;
	sVR.fYMax = fSuperclusterSize * 0.5f;
	sVR.fZMin = -fSuperclusterSize * 0.5f;
	sVR.fZMax = fSuperclusterSize * 0.5f;

	osg::Vec3f vRangeMin = osg::Vec3f(sVR.fXMin, sVR.fYMin, sVR.fZMin);
	osg::ref_ptr<osg::Uniform> pRangeMin = new osg::Uniform("rangeMin", vRangeMin);
	pSS->addUniform(pRangeMin.get());
	osg::Vec3f vRangeMax = osg::Vec3f(sVR.fXMax, sVR.fYMax, sVR.fZMax);
	osg::ref_ptr<osg::Uniform> pRangeMax = new osg::Uniform("rangeMax", vRangeMax);
	pSS->addUniform(pRangeMax.get());

	pSS->addUniform(m_vScreenSizeUniform.get());

	int iUnit = 0;
	pSS->setTextureAttributeAndModes(iUnit, m_3DShapeTex_128.get());
	pSS->addUniform(new osg::Uniform("shapeNoiseTex", iUnit));
	iUnit++;
	pSS->setTextureAttributeAndModes(iUnit, m_blueNoiseTex.get());
	pSS->addUniform(new osg::Uniform("blueNoiseSampler", iUnit));
	iUnit++;

	// 添加shader
	std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SuperclusterVert.glsl";
	std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SuperclusterFrag.glsl";
	return CGMKit::LoadShader(pSS.get(), strVertPath, strFragPath);
}

bool CGMGalaxy::_CreateUltracluster()
{
	const float fUltraclusterSize = 0.5f / 0.11f;

	m_pGeodeUltracluster = new osg::Geode;
	m_pGeodeUltracluster->setNodeMask(0);// 默认先隐藏
	m_pGeodeUltracluster->addDrawable(MakeBoxGeometry(fUltraclusterSize, fUltraclusterSize, fUltraclusterSize));
	m_pHierarchyRootVector.at(5)->addChild(m_pGeodeUltracluster.get());

	osg::ref_ptr<osg::StateSet> pSS = m_pGeodeUltracluster->getOrCreateStateSet();
	pSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSS->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSS->setAttributeAndModes(new osg::BlendFunc(), osg::StateAttribute::ON);
	pSS->setAttributeAndModes(new osg::CullFace());
	pSS->setRenderBinDetails(BIN_ULTRACLUSTER, "DepthSortedBin");
	pSS->setDefine("ULTRA", osg::StateAttribute::ON);

	SGMVolumeRange sVR;
	sVR.fXMin = -fUltraclusterSize * 0.5f;
	sVR.fXMax = fUltraclusterSize * 0.5f;
	sVR.fYMin = -fUltraclusterSize * 0.5f;
	sVR.fYMax = fUltraclusterSize * 0.5f;
	sVR.fZMin = -fUltraclusterSize * 0.5f;
	sVR.fZMax = fUltraclusterSize * 0.5f;

	osg::Vec3f vRangeMin = osg::Vec3f(sVR.fXMin, sVR.fYMin, sVR.fZMin);
	osg::ref_ptr<osg::Uniform> pRangeMin = new osg::Uniform("rangeMin", vRangeMin);
	pSS->addUniform(pRangeMin.get());
	osg::Vec3f vRangeMax = osg::Vec3f(sVR.fXMax, sVR.fYMax, sVR.fZMax);
	osg::ref_ptr<osg::Uniform> pRangeMax = new osg::Uniform("rangeMax", vRangeMax);
	pSS->addUniform(pRangeMax.get());

	pSS->addUniform(m_vScreenSizeUniform.get());

	int iUnit = 0;
	pSS->setTextureAttributeAndModes(iUnit, m_3DShapeTex_128.get());
	pSS->addUniform(new osg::Uniform("shapeNoiseTex", iUnit));
	iUnit++;
	pSS->setTextureAttributeAndModes(iUnit, m_blueNoiseTex.get());
	pSS->addUniform(new osg::Uniform("blueNoiseSampler", iUnit));
	iUnit++;

	// 添加shader
	std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SuperclusterVert.glsl";
	std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SuperclusterFrag.glsl";
	return CGMKit::LoadShader(pSS.get(), strVertPath, strFragPath);
}

bool CGMGalaxy::_CreateMyWorld()
{
	m_pMyWorld_5_AutoTrans = new osg::AutoTransform();
	m_pMyWorld_5_AutoTrans->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_CAMERA);
	m_pHierarchyRootVector.at(5)->addChild(m_pMyWorld_5_AutoTrans.get());
	m_pGeodeMyWorld_5 = new osg::Geode();
	m_pMyWorld_5_AutoTrans->addChild(m_pGeodeMyWorld_5.get());

	m_pMyWorld_6_AutoTrans = new osg::AutoTransform();
	m_pMyWorld_6_AutoTrans->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_CAMERA);
	m_pHierarchyRootVector.at(6)->addChild(m_pMyWorld_6_AutoTrans.get());
	m_pGeodeMyWorld_6 = new osg::Geode();
	m_pMyWorld_6_AutoTrans->addChild(m_pGeodeMyWorld_6.get());

	// 创建我的世界的点状几何体
	osg::ref_ptr<osg::Geometry> pGeometry_5 = new osg::Geometry();
	osg::ref_ptr<osg::Geometry> pGeometry_6 = new osg::Geometry();

	// 计算世界尺寸和相机参数
	float fWorldSize = 200.0f;
	float fFovy = osg::inDegrees(40.0f);
	float fTan = std::tan(fFovy*0.5f);
	// 相机在终极时刻的目标半径（5级空间下）
	float fCameraFinalRadius_5 = fWorldSize * 0.5f / fTan - fWorldSize * 0.5f;

	size_t iNum = 65536;
	osg::ref_ptr<osg::Vec3Array> vertArray_5 = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> vertArray_6 = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> texcoordArray = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array();
	osg::ref_ptr<osg::DrawElementsUInt> el = new osg::DrawElementsUInt(GL_POINTS);
	vertArray_5->reserve(iNum);
	vertArray_6->reserve(iNum);
	texcoordArray->reserve(iNum);
	colorArray->reserve(iNum);
	el->reserve(iNum);

	int x = 0;
	while (x < iNum)
	{
		float fRandomX = (m_iRandom() % 10000)*0.0001f - 0.5f;
		float fRandomY = (m_iRandom() % 10000)*0.0001f - 0.5f;
		float fRandomZ = (m_iRandom() % 10000)*0.0001f - 0.5f;
		float fRandomAlpha = (m_iRandom() % 1000)*0.001f;
		float fX = fWorldSize * fRandomX;
		float fY = fWorldSize * fRandomY;
		float fZ = fWorldSize * fRandomZ;

		osg::Vec3 vViewDir = osg::Vec3(fX, fY, fZ - fCameraFinalRadius_5);
		// 相机中轴线方向距离相机1个单位的平面上的向量
		osg::Vec3 vNear = vViewDir / abs(vViewDir.z());
		float fProjU = 0.5f * vNear.x() / fTan + 0.5f;
		float fProjV = 0.5f * vNear.y() / fTan + 0.5f;
		// 如果顶点在画面内
		if (fProjU <= 1.0f && fProjU >= 0.0f && fProjV <= 1.0f && fProjV >= 0.0f)
		{
			float fU = fRandomX + 0.5f;
			float fV = fRandomY + 0.5f;
			float fW = fRandomZ + 0.5f;
			float fHole = _Get3DValue(
				std::fmod(4 * fU, 1.0f),
				std::fmod(4 * fV, 1.0f),
				std::fmod(4 * fW, 1.0f));
			if (fHole < 0.45)
			{
				osg::Vec4f vColor = _GetPhotoColor(fProjU, fProjV);
				if (vColor.a() > 0.0f)
				{
					vertArray_5->push_back(osg::Vec3(fX, fY, fZ));
					texcoordArray->push_back(osg::Vec3(fU, fV, fW));
					colorArray->push_back(osg::Vec4(vColor.r(), vColor.g(), vColor.b(), fRandomAlpha*(1 - fHole)));
					el->push_back(x);
					x++;
				}
			}
		}	
	}

	int y = 0;
	while (y < iNum)
	{
		vertArray_6->push_back(vertArray_5->at(y) / GM_UNIT_SCALE);
		y++;
	}

	// 5
	pGeometry_5->setVertexArray(vertArray_5.get());
	pGeometry_5->setTexCoordArray(0, texcoordArray.get());
	pGeometry_5->setColorArray(colorArray.get());
	pGeometry_5->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 1, 0));
	pGeometry_5->setNormalArray(normal.get());
	pGeometry_5->setNormalBinding(osg::Geometry::BIND_OVERALL);
	pGeometry_5->addPrimitiveSet(el.get());

	pGeometry_5->setUseVertexBufferObjects(true);
	pGeometry_5->setUseDisplayList(false);
	pGeometry_5->setDataVariance(osg::Object::DYNAMIC);

	m_pGeodeMyWorld_5->addDrawable(pGeometry_5.get());

	// 6
	pGeometry_6->setVertexArray(vertArray_6.get());
	pGeometry_6->setTexCoordArray(0, texcoordArray.get());
	pGeometry_6->setColorArray(colorArray.get());
	pGeometry_6->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	pGeometry_6->setNormalArray(normal.get());
	pGeometry_6->setNormalBinding(osg::Geometry::BIND_OVERALL);
	pGeometry_6->addPrimitiveSet(el.get());

	pGeometry_6->setUseVertexBufferObjects(true);
	pGeometry_6->setUseDisplayList(false);
	pGeometry_6->setDataVariance(osg::Object::DYNAMIC);

	m_pGeodeMyWorld_6->addDrawable(pGeometry_6.get());

	// 状态集
	osg::ref_ptr<osg::StateSet> pSS_5 = m_pGeodeMyWorld_5->getOrCreateStateSet();
	osg::ref_ptr<osg::StateSet> pSS_6 = m_pGeodeMyWorld_6->getOrCreateStateSet();

	pSS_5->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
	pSS_5->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	pSS_5->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSS_5->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSS_5->setAttributeAndModes(new osg::BlendFunc(), osg::StateAttribute::ON);
	pSS_5->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pSS_5->setRenderBinDetails(BIN_GALAXIES, "DepthSortedBin");
	pSS_5->addUniform(m_pMyWorldAlphaUniform.get());
	int iUnit_5 = 0;
	pSS_5->setTextureAttributeAndModes(iUnit_5, m_2DNoiseTex.get());
	pSS_5->addUniform(new osg::Uniform("noise2DTex", iUnit_5));
	iUnit_5++;

	pSS_6->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
	pSS_6->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	pSS_6->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSS_6->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSS_6->setAttributeAndModes(new osg::BlendFunc(), osg::StateAttribute::ON);
	pSS_6->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pSS_6->setRenderBinDetails(BIN_GALAXIES, "DepthSortedBin");
	pSS_6->addUniform(m_pMyWorldAlphaUniform.get());
	int iUnit_6 = 0;
	pSS_6->setTextureAttributeAndModes(iUnit_6, m_2DNoiseTex.get());
	pSS_6->addUniform(new osg::Uniform("noise2DTex", iUnit_6));
	iUnit_6++;

	// 添加shader
	std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "MyWorldVert.glsl";
	std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "MyWorldFrag.glsl";
	CGMKit::LoadShader(pSS_5.get(), strVertPath, strFragPath);
	CGMKit::LoadShader(pSS_6.get(), strVertPath, strFragPath);

	return true;
}

bool CGMGalaxy::_CreateGalaxyBackground()
{
	m_pGalaxyBackgroundGeode = new osg::Geode;
	m_pGalaxyBackgroundGeode->setNodeMask(0);
	osg::ref_ptr<const osg::EllipsoidModel>	_ellipsoidModel = new osg::EllipsoidModel(1.0,1.0);
	osg::ref_ptr<osg::Geometry> pDrawable = _MakeEllipsoidGeometry(_ellipsoidModel, 32, 8, 0, true, true, true, -180.0f, 180.0f, -45.0f, 45.0f);
	m_pGalaxyBackgroundGeode->addDrawable(pDrawable.get());
	m_pCosmosBoxNode->addChild(m_pGalaxyBackgroundGeode.get());

	osg::ref_ptr<osg::StateSet>	pSSGalaxyBackground = m_pGalaxyBackgroundGeode->getOrCreateStateSet();
	pSSGalaxyBackground->setRenderBinDetails(BIN_GALAXY_BACKGROUND, "RenderBin");

	int iUnit = 0;
	std::string strTexPath = m_pConfigData->strCorePath + m_strGalaxyTexPath;
	pSSGalaxyBackground->setTextureAttributeAndModes(iUnit, _CreateTexture2D(strTexPath + "galaxyBackground.jpg", 3));
	pSSGalaxyBackground->addUniform(new osg::Uniform("galaxyBackgroundTex", iUnit));
	iUnit++;
	pSSGalaxyBackground->setTextureAttributeAndModes(iUnit, _CreateTexture2D(strTexPath + "starNoise.tga", 4));
	pSSGalaxyBackground->addUniform(new osg::Uniform("noiseTex", iUnit));
	iUnit++;

	pSSGalaxyBackground->addUniform(m_pStarAlphaUniform.get());
	pSSGalaxyBackground->addUniform(m_pEyePos4Uniform.get());

	// 添加shader
	std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyBackgroundVert.glsl";
	std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyBackgroundFrag.glsl";
	return CGMKit::LoadShader(pSSGalaxyBackground.get(), strVertPath, strFragPath);
}

bool CGMGalaxy::_CreateCosmosBox()
{
	m_pCosmosBoxGeode = new osg::Geode();
	m_pCosmosBoxGeode->addDrawable(MakeBoxGeometry(2,2,2));
	m_pCosmosBoxNode->addChild(m_pCosmosBoxGeode.get());

	std::string strCubeMapPath = m_pConfigData->strCorePath + m_strGalaxyTexPath + "Skybox/";
	std::string strCubeMapPrefix = "Skybox";
	osg::ref_ptr<osg::Texture> pCubeMapTex = _ReadCubeMap(strCubeMapPath, strCubeMapPrefix);

	osg::ref_ptr<osg::StateSet> pStateSetCosmosBox = m_pCosmosBoxGeode->getOrCreateStateSet();
	pStateSetCosmosBox->setRenderBinDetails(BIN_COSMOS, "RenderBin");

	int iUnit = 0;
	pStateSetCosmosBox->setTextureAttributeAndModes(iUnit, pCubeMapTex.get());
	pStateSetCosmosBox->addUniform(new osg::Uniform("cubeMapTex", iUnit));
	iUnit++;

	pStateSetCosmosBox->setTextureAttributeAndModes(iUnit, m_3DShapeTex_128.get());
	pStateSetCosmosBox->addUniform(new osg::Uniform("shapeNoiseTex", iUnit));
	iUnit++;

	pStateSetCosmosBox->addUniform(m_pStarAlphaUniform.get());
	pStateSetCosmosBox->addUniform(m_pTimesUniform.get());
	pStateSetCosmosBox->addUniform(m_pMyWorldAlphaUniform.get());

	// 添加shader
	std::string strCosmosBoxVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "CosmosBoxVert.glsl";
	std::string strCosmosBoxFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "CosmosBoxFrag.glsl";
	return CGMKit::LoadShader(pStateSetCosmosBox.get(), strCosmosBoxVertPath, strCosmosBoxFragPath);
}

bool CGMGalaxy::_DetachAudioPoints()
{
	SGMStarCoord vStarCoord = GM_ENGINE_PTR->GetDataManager()->GetStarCoord(m_strPlayingStarName);
	m_vPlayingAudioCoord = GM_ENGINE_PTR->GetDataManager()->GetAudioCoord(m_strPlayingStarName);
	m_pStarAudioPosUniform->set(osg::Vec2f(m_vPlayingAudioCoord.radius, m_vPlayingAudioCoord.angle));

	// 音频文件的星辰坐标Vector
	std::vector<SGMStarCoord> coordVector;
	GM_ENGINE_PTR->GetDataManager()->GetStarCoordVector(coordVector);

	std::vector<SGMStarCoord> newCoordVector;
	for (auto itr = coordVector.begin(); itr != coordVector.end(); itr++)
	{
		if (vStarCoord != *itr)
		{
			newCoordVector.push_back(*itr);
		}
	}
	if (newCoordVector.size() != (coordVector.size()-1))
	{
		return false;
	}

	osg::Geometry* pGeometry = dynamic_cast<osg::Geometry*>(m_pGeodeAudio->getDrawable(0));
	if (pGeometry)
	{
		m_pGeodeAudio->removeChild(pGeometry);
	}
	// 从修改过的数据管理模块读取数据，重新创建未激活状态的音频星几何体
	osg::ref_ptr<osg::Geometry> pGeomAudio = _CreateAudioGeometry(newCoordVector);
	if (!pGeomAudio.valid()) return false;
	m_pGeodeAudio->addDrawable(pGeomAudio.get());

	// 创建激活的音频星
	if (!m_pPlayingStarTransform.valid())
	{
		m_pPlayingStarTransform = new osg::PositionAttitudeTransform();
		osg::Geode* pGeode = new osg::Geode();
		osg::Geometry* pGeom = _CreateConeGeometry();
		pGeode->addDrawable(pGeom);
		m_pPlayingStarTransform->addChild(pGeode);
		m_pHierarchyRootVector.at(4)->addChild(m_pPlayingStarTransform.get());

		osg::ref_ptr<osg::StateSet> pSS = m_pPlayingStarTransform->getOrCreateStateSet();

		pSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pSS->setMode(GL_BLEND, osg::StateAttribute::ON);
		pSS->setAttributeAndModes(new osg::BlendFunc(), osg::StateAttribute::ON);
		pSS->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
		pSS->setRenderBinDetails(BIN_STAR_PLAYING, "DepthSortedBin");

		pSS->addUniform(m_pStarColorUniform.get());

		// 添加shader
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "PlayingStarVert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "PlayingStarFrag.glsl";
		CGMKit::LoadShader(pSS.get(), strVertPath, strFragPath);
	}
	osg::Vec3f vPos;
	m_pStarHiePosUniform->get(vPos);
	m_pPlayingStarTransform->asPositionAttitudeTransform()->setPosition(vPos);
	//m_pPlayingStarTransform->setNodeMask(~0);
	m_pPlayingStarTransform->setNodeMask(0);

	// 创建音频空间坐标指示线
	if (!m_pGeodeHelpLine.valid())
	{
		m_pGeodeHelpLine = new osg::Geode();
		osg::Geometry* pLineGeometry = _CreateHelpLineGeometry();
		m_pGeodeHelpLine->addDrawable(pLineGeometry);
		m_pHierarchyRootVector.at(4)->addChild(m_pGeodeHelpLine.get());

		osg::ref_ptr<osg::StateSet> pLineSS = m_pGeodeHelpLine->getOrCreateStateSet();
		osg::ref_ptr<osg::LineWidth> pLineWidth = new osg::LineWidth;
		pLineWidth->setWidth(1);
		pLineSS->setAttributeAndModes(pLineWidth.get(), osg::StateAttribute::ON);
		pLineSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pLineSS->setMode(GL_BLEND, osg::StateAttribute::ON);
		pLineSS->setAttributeAndModes(new osg::BlendFunc(), osg::StateAttribute::ON);
		pLineSS->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
		pLineSS->setRenderBinDetails(BIN_HELP_LINE, "DepthSortedBin");

		pLineSS->addUniform(m_pStarAudioPosUniform.get());
		pLineSS->addUniform(m_pTimesUniform.get());

		// 添加shader
		std::string strLineVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "HelpLineVert.glsl";
		std::string strLineFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "HelpLineFrag.glsl";
		CGMKit::LoadShader(pLineSS.get(), strLineVertPath, strLineFragPath);
	}
	m_pGeodeHelpLine->setNodeMask(~0);

	return true;
}

bool CGMGalaxy::_AttachAudioPoints()
{
	SGMAudioData sData;
	sData.name = m_strPlayingStarName;
	sData.audioCoord = m_vPlayingAudioCoord;
	GM_ENGINE_PTR->GetDataManager()->SetAudioData(sData);

	// 音频文件的星辰坐标Vector
	std::vector<SGMStarCoord> coordVector;
	GM_ENGINE_PTR->GetDataManager()->GetStarCoordVector(coordVector);
	osg::Geometry* pGeometry = dynamic_cast<osg::Geometry*>(m_pGeodeAudio->getDrawable(0));
	if (pGeometry)
	{
		m_pGeodeAudio->removeChild(pGeometry);
	}
	// 从修改过的数据管理模块读取数据，重新创建未激活状态的音频星几何体
	osg::ref_ptr<osg::Geometry> pGeomAudio = _CreateAudioGeometry(coordVector);
	if (!pGeomAudio.valid()) return false;
	m_pGeodeAudio->addDrawable(pGeomAudio.get());

	// 隐藏激活的音频星
	if (m_pPlayingStarTransform.valid())
	{
		m_pPlayingStarTransform->setNodeMask(0);
	}
	if (m_pGeodeHelpLine.valid())
	{
		m_pGeodeHelpLine->setNodeMask(0);
	}
	return true;
}

osg::Geometry* CGMGalaxy::_CreateAudioGeometry(std::vector<SGMStarCoord>& coordVector)
{
	// 4级空间下的星系半径
	double fGalaxyRadius4 = m_fGalaxyRadius / m_pKernelData->fUnitArray->at(4);

	osg::Geometry* pGeomAudio = new osg::Geometry();

	size_t iNum = coordVector.size();
	if (0 == iNum) return nullptr;

	osg::ref_ptr<osg::Vec3Array> vertArray = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array();
	osg::ref_ptr<osg::DrawElementsUShort> el = new osg::DrawElementsUShort(GL_POINTS);
	vertArray->reserve(iNum);
	colorArray->reserve(iNum);
	el->reserve(iNum);

	int x = 0;
	for (auto itr : coordVector)
	{
		float fX = float(itr.x) / GM_COORD_MAX;
		float fY = float(itr.y) / GM_COORD_MAX;
		float fZ = float(itr.z) / GM_COORD_MAX;

		osg::Vec4f vColor = GM_ENGINE_PTR->GetDataManager()->GetAudioColor(itr);

		fX *= fGalaxyRadius4;
		fY *= fGalaxyRadius4;
		fZ *= 0.001f * fGalaxyRadius4;

		vertArray->push_back(osg::Vec3f(fX, fY, fZ));
		colorArray->push_back(vColor);
		el->push_back(x);
		x++;
	}

	pGeomAudio->setVertexArray(vertArray.get());
	pGeomAudio->setColorArray(colorArray.get());
	pGeomAudio->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3f(0, 1, 0));
	pGeomAudio->setNormalArray(normal.get());
	pGeomAudio->setNormalBinding(osg::Geometry::BIND_OVERALL);
	pGeomAudio->addPrimitiveSet(el.get());

	pGeomAudio->setUseVertexBufferObjects(true);
	pGeomAudio->setUseDisplayList(false);
	pGeomAudio->setDataVariance(osg::Object::DYNAMIC);

	return pGeomAudio;
}

osg::Geometry* CGMGalaxy::_CreateConeGeometry()
{
	float fRadius = 1.5f;
	float fHeight = 2.0f;
	int segments = 32;
	float deltaAngle = osg::PI * 2 / (float)segments;

	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);

	osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
	verts->reserve(2 + segments);
	geom->setVertexArray(verts.get());

	osg::ref_ptr<osg::Vec3Array> textArray = new osg::Vec3Array;
	textArray->reserve(2 + segments);
	geom->setTexCoordArray(0, textArray.get());

	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->reserve(2 + segments);
	geom->setNormalArray(normal.get());
	geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

	osg::ref_ptr <osg::DrawElementsUShort> el = new osg::DrawElementsUShort(GL_TRIANGLE_FAN);
	el->reserve(2 + segments);
	geom->addPrimitiveSet(el.get());

	verts->push_back(osg::Vec3(0, 0, 0)); // center point
	verts->push_back(osg::Vec3(fRadius, 0, fHeight)); // first point
	textArray->push_back(osg::Vec3(0, 0, 0));
	textArray->push_back(osg::Vec3(1, 0, 1));
	normal->push_back(osg::Vec3(0, 0, -1));
	normal->push_back(osg::Vec3(1, 0, 0));
	el->push_back(0);
	el->push_back(1);

	for (int i = 0; i < segments; ++i)
	{
		double angle = deltaAngle * float(i+1);
		double cosA = cos(angle);
		double sinA = sin(angle);
		double x = fRadius * cosA;
		double y = fRadius * sinA;
		verts->push_back(osg::Vec3f(x, y, fHeight));
		textArray->push_back(osg::Vec3(cosA, sinA, 1));
		normal->push_back(osg::Vec3(cosA, sinA, 0));
		el->push_back(2 + i);
	}

	return geom;
}

osg::Geometry* CGMGalaxy::_CreateHelpLineGeometry()
{
	// 4级空间下的星系半径
	double fGalaxyRadius4 = m_fGalaxyRadius / m_pKernelData->fUnitArray->at(4);

	// 角度分段数
	int iAngleSegments = 128;
	// 半径分段数
	int iRadiusSegments = 128;
	int iVertexNum = 2 * (iAngleSegments + iRadiusSegments);

	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);
	geom->setUseDisplayList(false);
	geom->setDataVariance(osg::Object::DYNAMIC);

	osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
	verts->reserve(iVertexNum);
	geom->setVertexArray(verts.get());

	osg::ref_ptr<osg::Vec3Array> coordArray = new osg::Vec3Array;
	coordArray->reserve(iVertexNum);
	geom->setTexCoordArray(0, coordArray.get());

	osg::ref_ptr<osg::DrawElementsUShort> el = new osg::DrawElementsUShort(GL_LINES);
	el->reserve(iVertexNum);
	geom->addPrimitiveSet(el.get());

	// 角度辅助线
	float fAngleEach = osg::PI * 2.0 / float(iAngleSegments);
	for (int x = 0; x < 2 * iAngleSegments; x++)
	{
		float fMod2 = float(x % 2);
		float fX = fGalaxyRadius4 * cos(fAngleEach * x * 0.5);
		float fY = fGalaxyRadius4 * sin(fAngleEach * x * 0.5);
		verts->push_back(osg::Vec3(fX, fY, 0));
		coordArray->push_back(osg::Vec3(0.0f, (float(x) - fMod2 * 0.4 + 0.5) / float(2 * iAngleSegments), 1.0f - fMod2));
		el->push_back(x);
	}

	// 半径辅助线
	float fRadiusEach = fGalaxyRadius4 / float(iRadiusSegments);
	for (int y = 0; y < 2 * iRadiusSegments; y++)
	{
		verts->push_back(osg::Vec3(0, fGalaxyRadius4*0.5*float(y)/float(iRadiusSegments), 0));
		coordArray->push_back(osg::Vec3((float(y + y % 2) + 0.5) / float(2 * iRadiusSegments), 0, 0));
		el->push_back(2 * iAngleSegments + y);
	}

	return geom;
}

osg::Geometry* CGMGalaxy::_MakeEllipsoidGeometry(
	const osg::EllipsoidModel * ellipsoid,
	int iLonSegments, int iLatSegments,
	float fHae, bool bGenTexCoords,
	bool bWholeMap, bool bFlipNormal,
	float fLonStart, float fLonEnd,
	float fLatStart, float fLatEnd)
{
	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);

	if (fLatEnd < fLatStart)
	{
		float tmp = fLatEnd;
		fLatEnd = fLatStart;
		fLatStart = tmp;
	}
	if (fLonEnd < fLonStart)
	{
		float tmp = fLonEnd;
		fLonEnd = fLonStart;
		fLonStart = tmp;
	}
	fLonStart = (fLonStart < -180.0) ? -180.0 : fLonStart;
	fLonEnd = (fLonEnd > 180.0) ? 180.0 : fLonEnd;
	fLatStart = (fLatStart < -90.0) ? -90.0 : fLatStart;
	fLatEnd = (fLatEnd > 90.0) ? 90.0 : fLatEnd;

	double latSegmentSize = (fLatEnd - fLatStart) / (double)iLatSegments; // degrees
	double lonSegmentSize = (fLonEnd - fLonStart) / (double)iLonSegments; // degrees

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

float CGMGalaxy::_GetGalaxyValue(float fX, float fY, int iChannel, bool bLinear)
{
	if (!m_pGalaxyImage.valid())
	{
		std::string strFile = m_pConfigData->strCorePath + m_strGalaxyTexPath + "milkyWay.tga";
		m_pGalaxyImage = osgDB::readImageFile(strFile);
	}
	if (m_pGalaxyImage.valid())
	{
		unsigned int iWidth = m_pGalaxyImage->s();
		unsigned int iHeight = m_pGalaxyImage->t();

		float fS = fX*iWidth;
		float fT = fY*iHeight;

		float fDeltaS = fS - (int)fS;
		float fDeltaT = fT - (int)fT;
		unsigned int s = (unsigned int)fS;
		unsigned int t = (unsigned int)fT;
		unsigned int s_next = (s == iWidth) ? iWidth : (s + 1);
		unsigned int t_next = (t == iHeight) ? iHeight : (t + 1);

		float fValue_00 = 0;
		float fValue_10 = 0;
		float fValue_01 = 0;
		float fValue_11 = 0;

		switch (iChannel)
		{
		case 0:
		{
			fValue_00 = m_pGalaxyImage->getColor(s, t).r();
		}
		break;
		case 1:
		{
			fValue_00 = m_pGalaxyImage->getColor(s, t).g();
		}
		break;
		case 2:
		{
			fValue_00 = m_pGalaxyImage->getColor(s, t).b();
		}
		break;
		case 3:
		{
			fValue_00 = m_pGalaxyImage->getColor(s, t).a();
		}
		break;
		default: {}
		}

		if (bLinear)
		{
			switch (iChannel)
			{
			case 0:
			{
				fValue_10 = m_pGalaxyImage->getColor(s_next, t).r();
				fValue_01 = m_pGalaxyImage->getColor(s, t_next).r();
				fValue_11 = m_pGalaxyImage->getColor(s_next, t_next).r();
			}
			break;
			case 1:
			{
				fValue_10 = m_pGalaxyImage->getColor(s_next, t).g();
				fValue_01 = m_pGalaxyImage->getColor(s, t_next).g();
				fValue_11 = m_pGalaxyImage->getColor(s_next, t_next).g();
			}
			break;
			case 2:
			{
				fValue_10 = m_pGalaxyImage->getColor(s_next, t).b();
				fValue_01 = m_pGalaxyImage->getColor(s, t_next).b();
				fValue_11 = m_pGalaxyImage->getColor(s_next, t_next).b();
			}
			break;
			case 3:
			{
				fValue_10 = m_pGalaxyImage->getColor(s_next, t).a();
				fValue_01 = m_pGalaxyImage->getColor(s, t_next).a();
				fValue_11 = m_pGalaxyImage->getColor(s_next, t_next).a();
			}
			break;
			default: {}
			}
		}

		float fValue = fValue_00;
		if (bLinear)
		{
			fValue =
				fValue_00 * (1 - fDeltaS) * (1 - fDeltaT)
				+ fValue_10 * fDeltaS * (1 - fDeltaT)
				+ fValue_01 * (1 - fDeltaS) * fDeltaT
				+ fValue_11 * fDeltaS * fDeltaT;
		}
		return fValue;
	}
	return 0.0f;
}

float CGMGalaxy::_GetGalaxyHeight(float fX, float fY)
{
	if (!m_pGalaxyHeightImage.valid())
	{
		std::string strFile = m_pConfigData->strCorePath + m_strCoreNebulaTexPath + "milkyWay_height.tga";
		m_pGalaxyHeightImage = osgDB::readImageFile(strFile);
	}
	if (m_pGalaxyHeightImage.valid())
	{
		unsigned int iWidth = m_pGalaxyHeightImage->s();
		unsigned int iHeight = m_pGalaxyHeightImage->t();

		float fS = fX*iWidth;
		float fT = fY*iHeight;

		float fDeltaS = fS - (int)fS;
		float fDeltaT = fT - (int)fT;
		unsigned int s = (unsigned int)fS;
		unsigned int t = (unsigned int)fT;
		unsigned int s_next = (s == iWidth) ? iWidth : (s + 1);
		unsigned int t_next = (t == iHeight) ? iHeight : (t + 1);

		float fValue_00 = 0;
		float fValue_10 = 0;
		float fValue_01 = 0;
		float fValue_11 = 0;

		fValue_00 = m_pGalaxyHeightImage->getColor(s, t).b();
		fValue_10 = m_pGalaxyHeightImage->getColor(s_next, t).b();
		fValue_01 = m_pGalaxyHeightImage->getColor(s, t_next).b();
		fValue_11 = m_pGalaxyHeightImage->getColor(s_next, t_next).b();

		return fValue_00 * (1 - fDeltaS) * (1 - fDeltaT)
			+ fValue_10 * fDeltaS * (1 - fDeltaT)
			+ fValue_01 * (1 - fDeltaS) * fDeltaT
			+ fValue_11 * fDeltaS * fDeltaT;
	}
	return 0.0f;
}

float CGMGalaxy::_Get3DValue(float fX, float fY, float fZ)
{
	if (!m_shapeImg.valid())
	{
		std::string strTexturePath = m_pConfigData->strCorePath + m_strGalaxyTexPath + "noiseShape128.tga";
		m_shapeImg = osgDB::readImageFile(strTexturePath);
	}
	if (m_shapeImg.valid())
	{
		unsigned int iWidth = 128;
		unsigned int iHeight = 128;
		unsigned int iDepth = 128;

		float fS = std::fmodf(fX, 1.0f) * iWidth;
		float fT = std::fmodf(fY, 1.0f) * iHeight;
		float fR = std::fmodf(fZ, 1.0f) * iDepth;

		float fDeltaS = fS - (int)fS;
		float fDeltaT = fT - (int)fT;
		float fDeltaR = fR - (int)fR;
		unsigned int s = (unsigned int)fS;
		unsigned int t = (unsigned int)fT;
		unsigned int r = (unsigned int)fR;
		unsigned int s_next = (s == iWidth-1) ? 0 : (s + 1);
		unsigned int t_next = (t == iHeight-1) ? 0 : (t + 1);
		unsigned int r_next = (r == iDepth-1) ? 0 : (r + 1);

		float fValue_000 = _Get3DValue(s,		t,		r		);
		float fValue_100 = _Get3DValue(s_next,	t,		r		);
		float fValue_010 = _Get3DValue(s,		t_next,	r		);
		float fValue_110 = _Get3DValue(s_next,	t_next,	r		);
		float fValue_001 = _Get3DValue(s,		t,		r_next	);
		float fValue_101 = _Get3DValue(s_next,	t,		r_next	);
		float fValue_011 = _Get3DValue(s,		t_next,	r_next	);
		float fValue_111 = _Get3DValue(s_next,	t_next,	r_next	);

		float fValue = 
			(fValue_000 * (1 - fDeltaS) * (1 - fDeltaT)
			+ fValue_100 * fDeltaS * (1 - fDeltaT)
			+ fValue_010 * (1 - fDeltaS) * fDeltaT
			+ fValue_110 * fDeltaS * fDeltaT) * (1 - fDeltaR)
			+ (fValue_001 * (1 - fDeltaS) * (1 - fDeltaT)
			+ fValue_101 * fDeltaS * (1 - fDeltaT)
			+ fValue_011 * (1 - fDeltaS) * fDeltaT
			+ fValue_111 * fDeltaS * fDeltaT) * fDeltaR;
		return fValue;
	}
	return 0.0f;
}

float CGMGalaxy::_Get3DValue(unsigned int iX, unsigned int iY, unsigned int iZ)
{
	if (m_shapeImg.valid())
	{
		unsigned int iChannel = iZ / 32;
		unsigned int iSubLayer = iZ % 32;
		switch (iChannel)
		{
		case 0:
		{
			return m_shapeImg->getColor(iX, iY + iSubLayer * 128).r();
		}
		break;
		case 1:
		{
			return m_shapeImg->getColor(iX, iY + iSubLayer * 128).g();
		}
		break;
		case 2:
		{
			return m_shapeImg->getColor(iX, iY + iSubLayer * 128).b();
		}
		break;
		case 3:
		{
			return m_shapeImg->getColor(iX, iY + iSubLayer * 128).a();
		}
		break;
		default:
			break;
		}
	}
	return 0.0f;
}

osg::Vec4f CGMGalaxy::_GetPhotoColor(float fX, float fY)
{
	if (!m_pPhotoImage.valid())
	{
		std::string strPath = m_pConfigData->strCorePath + m_strGalaxyTexPath + "photo.tga";
		m_pPhotoImage = osgDB::readImageFile(strPath);
	}
	if (m_pPhotoImage.valid())
	{
		unsigned int iWidth = m_pPhotoImage->s();
		unsigned int iHeight = m_pPhotoImage->t();
		unsigned int s = fX * iWidth;
		unsigned int t = fY * iHeight;

		return osg::Vec4f(
			m_pPhotoImage->getColor(s, t).r(),
			m_pPhotoImage->getColor(s, t).g(),
			m_pPhotoImage->getColor(s, t).b(),
			m_pPhotoImage->getColor(s, t).a());
	}
	return osg::Vec4f(0,0,0,0);
}

/**
* _CreateTexture2D
* 加载2D纹理
* @author LiuTao
* @since 2020.06.16
* @param fileName: 图片文件路径
* @param iChannelNum: 1、2、3、4分别代表R、RG、RGB、RGBA
* @return osg::Texture* 返回纹理指针
*/
osg::Texture* CGMGalaxy::_CreateTexture2D(const std::string & fileName, const int iChannelNum)
{
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	texture->setImage(osgDB::readImageFile(fileName));
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

osg::Texture* CGMGalaxy::_ReadCubeMap(const std::string& strFolder, const std::string& strFilePrefix)
{
	osg::TextureCubeMap* cubemap = new osg::TextureCubeMap;

	std::string strFile = strFolder + strFilePrefix;
	osg::Image* imagePosX = osgDB::readImageFile(strFile + "X+.jpg");
	osg::Image* imageNegX = osgDB::readImageFile(strFile + "X-.jpg");
	osg::Image* imagePosY = osgDB::readImageFile(strFile + "Y+.jpg");
	osg::Image* imageNegY = osgDB::readImageFile(strFile + "Y-.jpg");
	osg::Image* imagePosZ = osgDB::readImageFile(strFile + "Z+.jpg");
	osg::Image* imageNegZ = osgDB::readImageFile(strFile + "Z-.jpg");

	if (imagePosX && imageNegX && imagePosY && imageNegY && imagePosZ && imageNegZ)
	{
		cubemap->setImage(osg::TextureCubeMap::POSITIVE_X, imagePosX);
		cubemap->setImage(osg::TextureCubeMap::NEGATIVE_X, imageNegX);
		cubemap->setImage(osg::TextureCubeMap::POSITIVE_Y, imagePosY);
		cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Y, imageNegY);
		cubemap->setImage(osg::TextureCubeMap::POSITIVE_Z, imagePosZ);
		cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Z, imageNegZ);

		cubemap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		cubemap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
		cubemap->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);

		cubemap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		cubemap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

		cubemap->setInternalFormat(GL_RGB8);
		cubemap->setSourceFormat(GL_RGB);
		cubemap->setSourceType(GL_UNSIGNED_BYTE);
	}

	return cubemap;
}

osg::Vec3f CGMGalaxy::_getRandomStarColor()
{
	// 0.0-4.0，分成“256*4”段
	float fCenter = (m_iRandom() % 1024) / 256.0f;
	// 0.5-1.0，用来随机色相
	float fScale = (m_iRandom() % 128) / 256.0f + 0.5f;

	float fR = osg::clampBetween(fScale * (2.0f - abs(1.0f - fCenter)), 0.0f, 1.0f);
	float fG = osg::clampBetween(fScale * (2.0f - abs(2.0f - fCenter)), 0.0f, 1.0f);
	float fB = osg::clampBetween(fScale * (2.0f - abs(3.0f - fCenter)), 0.0f, 1.0f);
	return osg::Vec3f(fR, fG, fB);
}
