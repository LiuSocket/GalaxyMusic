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
#include "GMEarthTail.h"
#include "GMEarthEngine.h"
#include "GMKit.h"
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

/*************************************************************************
Class
*************************************************************************/

/*************************************************************************
CGMEarth Methods
*************************************************************************/

/** @brief ���� */
CGMEarth::CGMEarth() : CGMPlanet(), m_pKernelData(nullptr),m_pCommonUniform(nullptr),
	m_strGalaxyShaderPath("Shaders/GalaxyShader/"),
	m_strEarthShaderPath("Shaders/EarthShader/"),
	m_fCurrentObliquity(osg::DegreesToRadians(23.44)), m_fNorthRotateSpeed(0.0),
	m_fCloudBottom(5e3f), m_fCloudTop(1e4f),
	m_vEarthCoordScaleUniform(new osg::Uniform("coordScale_Earth", osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f))),//UV���ţ��Ĳ㹻����
	m_fWanderProgressUniform(new osg::Uniform("wanderProgress", 0.0f)),
	m_pEarthTail(nullptr), m_pEarthEngine(nullptr)
{
	m_pEarthRoot_1 = new osg::Group();
	m_pEarthRoot_2 = new osg::Group();

	m_pEllipsoid = new osg::EllipsoidModel();
	m_pEarthTail = new CGMEarthTail();
	m_pEarthEngine = new CGMEarthEngine();
}

/** @brief ���� */
CGMEarth::~CGMEarth()
{
}

/** @brief ��ʼ�� */
bool CGMEarth::Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform)
{
	CGMPlanet::Init(pConfigData);

	Panorama2CubeMap();

	m_pKernelData = pKernelData;
	m_pCommonUniform = pCommonUniform;

	// ��ȡddsʱ��Ҫ��ֱ��ת
	m_pDDSOptions = new osgDB::Options("dds_flip");
	std::string strSphereTexPath = m_pConfigData->strCorePath + "Textures/Sphere/";
	std::string strVolumeTexPath = m_pConfigData->strCorePath + "Textures/Volume/";

	if (m_pConfigData->bWanderingEarth)
	{
		// ��ʼ�����˵���β��
		m_pEarthTail->Init(pKernelData, pConfigData, pCommonUniform);
		m_pEarthEngine->Init(pKernelData, pConfigData, pCommonUniform);

		m_pEarthRoot_1->addChild(m_pEarthEngine->GetEarthEngineRoot(1));
		m_pEarthRoot_2->addChild(m_pEarthEngine->GetEarthEngineRoot(2));
	}

	m_aEarthBaseTex = _CreateDDSTex2DArray(strSphereTexPath + "Earth/Earth_base_");
	m_aEarthCloudTex = _CreateDDSTex2DArray(strSphereTexPath + "Earth/Earth_cloud_");
	if (m_pConfigData->bWanderingEarth)
	{
		_AddTex2DArray(m_aEarthBaseTex, strSphereTexPath + "Earth/wanderingEarth_base_");
		_AddTex2DArray(m_aEarthCloudTex, strSphereTexPath + "Earth/wanderingEarth_cloud_");
	}

	m_aIllumTex = _CreateDDSTex2DArray(strSphereTexPath + "Earth/Earth_illum_");

	// ����DEM
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

	//// ����
	//m_aAuroraTex = _CreateDDSTex2DArray(strSphereTexPath + "aurora.dds");
	
	// �Ƶ�ϸ������
	m_pCloudDetailTex = _CreateDDSTexture(strVolumeTexPath + "CloudDetail.dds", osg::Texture::REPEAT, osg::Texture::REPEAT);
	// ɢ����ά����
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

	// ��ȡ��ǰ������ͼ�ĳߴ磬�����޸�UV����ϵ��
	float fBaseTexSize = m_aEarthBaseTex->getTextureWidth();
	float fCloudTexSize = m_aEarthCloudTex->getTextureWidth();
	float fIlluTexSize = m_aIllumTex->getTextureWidth();
	float fDEMTexSize = m_aDEMTex->getTextureWidth();
	// ������ͼ��С���޸�m_vCoordScaleUniform
	// Ϊ�˱��������α�Ե����ʱ�з죬������ÿ�������ͼ��Ե����һ�����صı������أ��͸��������غ�
	// ���Բ���ʱ��Ҫ������ͼ��С����ÿ������Ե�������Сһ������
	// x = baseTex; y = cloudTex; z = illumTex;
	osg::Vec4f vCoordScale(
		(fBaseTexSize - 2.0f) / fBaseTexSize,
		(fCloudTexSize - 2.0f) / fCloudTexSize,
		(fIlluTexSize - 2.0f) / fIlluTexSize,
		(fDEMTexSize - 1.0f) / fDEMTexSize);
	m_vEarthCoordScaleUniform->set(vCoordScale);

	return true;
}

/** @brief ���� */
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
	{
		m_pEarthTail->Update(dDeltaTime);
		m_pEarthEngine->Update(dDeltaTime);
	}
	return true;
}

/** @brief ����(�������������̬֮��) */
bool CGMEarth::UpdateLater(double dDeltaTime)
{
	int iHie = m_pKernelData->iHierarchy;

	osg::Matrixd mViewMatrix = GM_View->getCamera()->getViewMatrix();
	osg::Matrixd mProjMatrix = GM_View->getCamera()->getProjectionMatrix();

	// �޸�ȫ����Ӱ���
	m_pGlobalShadowCamera->setViewMatrix(mViewMatrix);
	m_pGlobalShadowCamera->setProjectionMatrix(mProjMatrix);

	if (m_pConfigData->bWanderingEarth)
	{
		m_pEarthTail->UpdateLater(dDeltaTime);
		m_pEarthEngine->UpdateLater(dDeltaTime);
	}

	return true;
}

/** @brief ���� */
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

	if (m_pSSGlobalShadow.valid())
	{
		CGMKit::LoadShader(m_pSSGlobalShadow,
			strEarthShader + "GlobalShadow.vert",
			strEarthShader + "GlobalShadow.frag",
			"Global Shadow");
	}

	if (m_pConfigData->bWanderingEarth)
	{
		m_pEarthTail->Load();
		m_pEarthEngine->Load();
	}

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
		m_pEarthTail->SetUniform(
			pViewLight,
			m_pEarthEngine->GetEngineStartRatioUniform(),
			pView2ECEF,
			m_fWanderProgressUniform);
		m_pEarthEngine->SetUniform(
			pViewLight,
			pGroundTop,
			pAtmosHeight,
			pMinDotUL,
			pEyeAltitude,
			m_fWanderProgressUniform);
	}
}

void CGMEarth::ResizeScreen(const int iW, const int iH)
{
	m_pGlobalShadowCamera->resize(iW, iH);

	if (m_pConfigData->bWanderingEarth)
	{
		m_pEarthTail->ResizeScreen(iW, iH);
		m_pEarthEngine->ResizeScreen(iW, iH);
	}
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

	// ���˵���
	if (m_pConfigData->bWanderingEarth)
	{
		m_pEarthTail->SetVisible(bVisible);
		m_pEarthEngine->SetVisible(bVisible);
	}
}

void CGMEarth::SetEarthRotate(const double fSpin, const double fObliquity, const double fNorthYaw)
{
	m_fCurrentObliquity = fObliquity;

	osg::Quat qPlanetSpin = osg::Quat(fSpin, osg::Vec3d(0, 0, 1));
	osg::Quat qPlanetInclination = osg::Quat(fObliquity, osg::Vec3d(1, 0, 0));
	osg::Quat qPlanetNorthYaw = osg::Quat(fNorthYaw, osg::Vec3d(0, 0, 1));
	osg::Quat qRotate = qPlanetSpin * qPlanetInclination * qPlanetNorthYaw;
	m_pShadow_2_Transform->asPositionAttitudeTransform()->setAttitude(qRotate);

	if (m_pConfigData->bWanderingEarth)
	{
		m_pEarthEngine->SetEarthSpin(fSpin);
		m_pEarthTail->SetEarthTailRotate(fSpin, fObliquity, fNorthYaw);
	}
}

void CGMEarth::SetWanderingEarthProgress(const float fProgress)
{
	if (!m_pConfigData->bWanderingEarth) return;

	m_fWanderProgressUniform->set(osg::clampBetween(fProgress, 0.0f, 1.0f));
}

bool CGMEarth::CreateEarth()
{
	_CreateGlobalCloudShadow();

	// ������������1���ռ�
	_CreateEarth_1();
	// ������������2���ռ�
	_CreateEarth_2();

	// �������˵�����صĶ���
	if (m_pConfigData->bWanderingEarth)
	{
		_CreateWanderingEarth();
	}

	return true;
}

bool CGMEarth::UpdateHierarchy(int iHieNew)
{
	switch (iHieNew)
	{
	case 0:
	{
		// ����ȫ���ƶԵ���Ӱ��ص�����ͽڵ�
		m_pGlobalShadowCamera->setNodeMask(0);
	}
	break;
	case 1:
	{
		// ��ʾȫ���ƶԵ���Ӱ��ص�����ͽڵ�
		m_pGlobalShadowCamera->setNodeMask(~0);
		m_mShadowEarthGeode.at(1)->setNodeMask(~0);
		m_mShadowEarthGeode.at(2)->setNodeMask(0);
	}
	break;
	case 2:
	{
		// ��ʾȫ���ƶԵ���Ӱ��ص�����ͽڵ�
		m_pGlobalShadowCamera->setNodeMask(~0);
		m_mShadowEarthGeode.at(1)->setNodeMask(0);
		m_mShadowEarthGeode.at(2)->setNodeMask(~0);
	}
	break;
	case 3:
	{
		// ����ȫ���ƶԵ���Ӱ��ص�����ͽڵ�
		m_pGlobalShadowCamera->setNodeMask(0);
	}
	break;
	default:
		break;
	}

	if (m_pConfigData->bWanderingEarth)
	{
		m_pEarthTail->UpdateHierarchy(iHieNew);
		m_pEarthEngine->UpdateHierarchy(iHieNew);
	}

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
		m_pCelestialScaleVisitor->SetRadius(
			(osg::WGS_84_RADIUS_EQUATOR + m_fCloudTop) / fUnit,
			(osg::WGS_84_RADIUS_POLAR + m_fCloudTop) / fUnit);
		pShadowEarthGeom->accept(*m_pCelestialScaleVisitor);	// �ı��С
		osg::ref_ptr<osg::Geode> pShadowEarth = new osg::Geode();
		m_mShadowEarthGeode[i] = pShadowEarth;
		pShadowEarth->addDrawable(pShadowEarthGeom);
		if (2 == i)
		{
			m_pShadow_2_Transform = new osg::PositionAttitudeTransform();
			m_pShadow_2_Transform->addChild(pShadowEarth);
			m_pGlobalShadowCamera->addChild(m_pShadow_2_Transform);
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
	// ��������
	m_pSSGlobalShadow->setTextureAttributeAndModes(iShadowUnit, m_aEarthCloudTex, osg::StateAttribute::ON);
	osg::ref_ptr<osg::Uniform> pGlobalCloudUniform = new osg::Uniform("cloudTex", iShadowUnit++);
	m_pSSGlobalShadow->addUniform(pGlobalCloudUniform);
	// �Ƶ�ϸ������
	m_pSSGlobalShadow->setTextureAttributeAndModes(iShadowUnit, m_pCloudDetailTex, osg::StateAttribute::ON);
	osg::ref_ptr<osg::Uniform> pCloudDetailUniform = new osg::Uniform("cloudDetailTex", iShadowUnit++);
	m_pSSGlobalShadow->addUniform(pCloudDetailUniform);

	if (m_pConfigData->bWanderingEarth)
	{
		m_pSSGlobalShadow->addUniform(m_fWanderProgressUniform);
		m_pSSGlobalShadow->setDefine("WANDERING", osg::StateAttribute::ON);
	}

	// ���shader
	std::string strShaderPath = m_pConfigData->strCorePath + m_strEarthShaderPath;
	CGMKit::LoadShader(m_pSSGlobalShadow,
		strShaderPath + "GlobalShadow.vert",
		strShaderPath + "GlobalShadow.frag",
		"Global Shadow");

	m_pGlobalShadowCamera->setStateSet(m_pSSGlobalShadow);
	// ��ȫ���ƶԵ���Ӱ����ӵ����ڵ��£�ֻ��1��2�㼶�²���ʾ
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
	m_pCelestialScaleVisitor->SetRadius(osg::WGS_84_RADIUS_EQUATOR / fUnit1, osg::WGS_84_RADIUS_POLAR / fUnit1);
	// �ı��С
	m_pEarthGeom_1->accept(*m_pCelestialScaleVisitor);

	// ������ʯ����
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
	// ����궨��
	m_pSSEarthGround_1->setDefine("EARTH", osg::StateAttribute::ON);
	m_pSSEarthGround_1->setDefine("ATMOS", osg::StateAttribute::ON);

	int iGroundUnit = 0;
	// ������ͼ
	m_pSSEarthGround_1->setTextureAttributeAndModes(iGroundUnit, m_aEarthBaseTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGrundBaseUniform = new osg::Uniform("baseTex", iGroundUnit++);
	m_pSSEarthGround_1->addUniform(pGrundBaseUniform);
	// �Է�����ͼ
	m_pSSEarthGround_1->setTextureAttributeAndModes(iGroundUnit, m_aIllumTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGrundIllumUniform = new osg::Uniform("illumTex", iGroundUnit++);
	m_pSSEarthGround_1->addUniform(pGrundIllumUniform);
	// DEM��ͼ
	m_pSSEarthGround_1->setTextureAttributeAndModes(iGroundUnit, m_aDEMTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pDEMUniform = new osg::Uniform("DEMTex", iGroundUnit++);
	m_pSSEarthGround_1->addUniform(pDEMUniform);
	// ȫ����Ӱ
	m_pSSEarthGround_1->setTextureAttributeAndModes(iGroundUnit, m_pGlobalShadowTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGlobalShadowUniform = new osg::Uniform("globalShadowTex", iGroundUnit++);
	m_pSSEarthGround_1->addUniform(pGlobalShadowUniform);
	// �����ϵĴ�������ɢ�䡱����
	m_pSSEarthGround_1->setTextureAttributeAndModes(iGroundUnit, m_pInscatteringTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGroundInscatteringUniform = new osg::Uniform("inscatteringTex", iGroundUnit++);
	m_pSSEarthGround_1->addUniform(pGroundInscatteringUniform);

	if (m_pConfigData->bWanderingEarth)
	{
		// ���˵���β������ɢ�Ĵ�����
		m_pSSEarthGround_1->setTextureAttributeAndModes(iGroundUnit, m_pEarthTail->GetTAATex(), iOnOverride);
		osg::ref_ptr<osg::Uniform> pGroundTailUniform = new osg::Uniform("tailTex", iGroundUnit++);
		m_pSSEarthGround_1->addUniform(pGroundTailUniform);

		m_pSSEarthGround_1->addUniform(m_pEarthEngine->GetEngineStartRatioUniform());
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

	// ���shader
	CGMKit::LoadShaderWithCommonFrag(m_pSSEarthGround_1,
		strShaderPath + "CelestialGround.vert",
		strShaderPath + "CelestialGround.frag",
		strShaderPath + "CelestialCommon.frag",
		"CelestialGround");

	////////////////////////////////////
	// �����Ʋ�
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
	// �Ʋ�ƫ�ƣ�����͵�����
	osg::ref_ptr<osg::PolygonOffset> pCloudPO = new osg::PolygonOffset(-1, -1);
	m_pSSEarthCloud_1->setAttributeAndModes(pCloudPO, iOnOverride);
	m_pSSEarthCloud_1->setDefine("ATMOS", osg::StateAttribute::ON);
	// Ĭ�ϵ���
	m_pSSEarthCloud_1->setDefine("EARTH", osg::StateAttribute::ON);
	m_pSSEarthCloud_1->setDefine("SATURN", osg::StateAttribute::OFF);

	int iCloudUnit = 0;
	// ������ͼ
	m_pSSEarthCloud_1->setTextureAttributeAndModes(iCloudUnit, m_aEarthCloudTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pCloudBaseUniform = new osg::Uniform("cloudTex", iCloudUnit++);
	m_pSSEarthCloud_1->addUniform(pCloudBaseUniform);
	// �Ƶ�ϸ������
	m_pSSEarthCloud_1->setTextureAttributeAndModes(iCloudUnit, m_pCloudDetailTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pCloudDetailUniform = new osg::Uniform("cloudDetailTex", iCloudUnit++);
	m_pSSEarthCloud_1->addUniform(pCloudDetailUniform);
	// �Ʋ��ϵġ���ɢ�䡱����
	m_pSSEarthCloud_1->setTextureAttributeAndModes(iCloudUnit, m_pInscatteringTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pCloudInscatteringUniform = new osg::Uniform("inscatteringTex", iCloudUnit++);
	m_pSSEarthCloud_1->addUniform(pCloudInscatteringUniform);

	if (m_pConfigData->bWanderingEarth)
	{
		// ���˵���β������ɢ�Ĵ�����
		m_pSSEarthCloud_1->setTextureAttributeAndModes(iCloudUnit, m_pEarthTail->GetTAATex(), iOnOverride);
		osg::ref_ptr<osg::Uniform> pCloudTailUniform = new osg::Uniform("tailTex", iCloudUnit++);
		m_pSSEarthCloud_1->addUniform(pCloudTailUniform);

		m_pSSEarthCloud_1->setTextureAttributeAndModes(iCloudUnit, m_aIllumTex, iOnOverride);
		osg::ref_ptr<osg::Uniform> pCloudIllumUniform = new osg::Uniform("illumTex", iCloudUnit++);
		m_pSSEarthCloud_1->addUniform(pCloudIllumUniform);

		m_pSSEarthCloud_1->addUniform(m_pEarthEngine->GetEngineStartRatioUniform());
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

	// ���shader
	CGMKit::LoadShaderWithCommonFrag(m_pSSEarthCloud_1,
		strShaderPath + "CelestialCloud.vert",
		strShaderPath + "CelestialCloud.frag",
		strShaderPath + "CelestialCommon.frag",
		"CelestialCloud");

	///////////////////////////////////////////////
	// �������
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
	// ����ɢ�䡱��ͼ
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

	// ���shader
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
	// �ı��С
	m_pEarthGeom_2->accept(*m_pCelestialScaleVisitor);

	// ������ʯ����
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
	// ����궨��
	m_pSSEarthGround_2->setDefine("EARTH", osg::StateAttribute::ON);
	m_pSSEarthGround_2->setDefine("ATMOS", osg::StateAttribute::ON);

	int iGroundUnit = 0;
	// ������ͼ
	m_pSSEarthGround_2->setTextureAttributeAndModes(iGroundUnit, m_aEarthBaseTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGrundBaseUniform = new osg::Uniform("baseTex", iGroundUnit++);
	m_pSSEarthGround_2->addUniform(pGrundBaseUniform);
	// �Է�����ͼ
	m_pSSEarthGround_2->setTextureAttributeAndModes(iGroundUnit, m_aIllumTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGrundIllumUniform = new osg::Uniform("illumTex", iGroundUnit++);
	m_pSSEarthGround_2->addUniform(pGrundIllumUniform);
	// DEM��ͼ
	m_pSSEarthGround_2->setTextureAttributeAndModes(iGroundUnit, m_aDEMTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pDEMUniform = new osg::Uniform("DEMTex", iGroundUnit++);
	m_pSSEarthGround_2->addUniform(pDEMUniform);
	// ȫ����Ӱ
	m_pSSEarthGround_2->setTextureAttributeAndModes(iGroundUnit, m_pGlobalShadowTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGlobalShadowUniform = new osg::Uniform("globalShadowTex", iGroundUnit++);
	m_pSSEarthGround_2->addUniform(pGlobalShadowUniform);
	// �����ϵĴ�������ɢ�䡱����
	m_pSSEarthGround_2->setTextureAttributeAndModes(iGroundUnit, m_pInscatteringTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGroundInscatteringUniform = new osg::Uniform("inscatteringTex", iGroundUnit++);
	m_pSSEarthGround_2->addUniform(pGroundInscatteringUniform);

	if (m_pConfigData->bWanderingEarth)
	{
		// ���˵���β������ɢ�Ĵ�����
		m_pSSEarthGround_2->setTextureAttributeAndModes(iGroundUnit, m_pEarthTail->GetTAATex(), iOnOverride);
		osg::ref_ptr<osg::Uniform> pGroundTailUniform = new osg::Uniform("tailTex", iGroundUnit++);
		m_pSSEarthGround_2->addUniform(pGroundTailUniform);

		m_pSSEarthGround_2->addUniform(m_pEarthEngine->GetEngineStartRatioUniform());
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

	// ���shader
	CGMKit::LoadShaderWithCommonFrag(m_pSSEarthGround_2,
		strShaderPath + "CelestialGround.vert",
		strShaderPath + "CelestialGround.frag",
		strShaderPath + "CelestialCommon.frag",
		"CelestialGround");

	////////////////////////////////////
	// �����Ʋ�
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
	// �Ʋ�ƫ�ƣ�����͵�����
	osg::ref_ptr<osg::PolygonOffset> pCloudPO = new osg::PolygonOffset(-1, -1);
	m_pSSEarthCloud_2->setAttributeAndModes(pCloudPO, iOnOverride);
	m_pSSEarthCloud_2->setDefine("ATMOS", osg::StateAttribute::ON);
	// Ĭ�ϵ���
	m_pSSEarthCloud_2->setDefine("EARTH", osg::StateAttribute::ON);
	m_pSSEarthCloud_2->setDefine("SATURN", osg::StateAttribute::OFF);

	int iCloudUnit = 0;
	// ������ͼ
	m_pSSEarthCloud_2->setTextureAttributeAndModes(iCloudUnit, m_aEarthCloudTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pCloudBaseUniform = new osg::Uniform("cloudTex", iCloudUnit++);
	m_pSSEarthCloud_2->addUniform(pCloudBaseUniform);
	// �Ƶ�ϸ������
	m_pSSEarthCloud_2->setTextureAttributeAndModes(iCloudUnit, m_pCloudDetailTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pCloudDetailUniform = new osg::Uniform("cloudDetailTex", iCloudUnit++);
	m_pSSEarthCloud_2->addUniform(pCloudDetailUniform);
	// �Ʋ��ϵġ���ɢ�䡱����
	m_pSSEarthCloud_2->setTextureAttributeAndModes(iCloudUnit, m_pInscatteringTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pCloudInscatteringUniform = new osg::Uniform("inscatteringTex", iCloudUnit++);
	m_pSSEarthCloud_2->addUniform(pCloudInscatteringUniform);

	if (m_pConfigData->bWanderingEarth)
	{
		// ���˵���β������ɢ�Ĵ�����
		m_pSSEarthCloud_2->setTextureAttributeAndModes(iCloudUnit, m_pEarthTail->GetTAATex(), iOnOverride);
		osg::ref_ptr<osg::Uniform> pCloudTailUniform = new osg::Uniform("tailTex", iCloudUnit++);
		m_pSSEarthCloud_2->addUniform(pCloudTailUniform);

		m_pSSEarthCloud_2->setTextureAttributeAndModes(iCloudUnit, m_aIllumTex, iOnOverride);
		osg::ref_ptr<osg::Uniform> pCloudIllumUniform = new osg::Uniform("illumTex", iCloudUnit++);
		m_pSSEarthCloud_2->addUniform(pCloudIllumUniform);

		m_pSSEarthCloud_2->addUniform(m_pEarthEngine->GetEngineStartRatioUniform());
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

	// ���shader
	CGMKit::LoadShaderWithCommonFrag(m_pSSEarthCloud_2,
		strShaderPath + "CelestialCloud.vert",
		strShaderPath + "CelestialCloud.frag",
		strShaderPath + "CelestialCommon.frag",
		"CelestialCloud");

	////////////////////////////////////
	// �������
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
	// ����ɢ�䡱��ͼ
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

	// ���shader
	CGMKit::LoadShader(m_pSSEarthAtmos_2,
		strShaderPath + "CelestialAtmosphere.vert",
		strShaderPath + "CelestialAtmosphere.frag",
		"CelestialAtmosphere");

	return true;
}

bool CGMEarth::_CreateWanderingEarth()
{
	// ��ʱ��ӵ��������˵���汾�ĸ�����ͼ�Ĺ��ߺ���
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

	if ((m_pConfigData->bWanderingEarth))
	{
		m_pEarthTail->MakeEarthTail();

		m_pEarthEngine->SetTex(m_pEarthTail->GetTAATex(), m_pInscatteringTex);
		m_pEarthEngine->CreateEngine();
	}

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
				// Ŀ��ͼƬ��ǰ����Rͨ���ĵ�ַ
				int iAddress = 4 * (pImage0->s() * j + i);
				// ���ݲ�ͬͼƬ����ȡ��ͬ�ĵ����㷨
				switch (iType)
				{
				case 0:
				{
					// base color
					osg::Vec4 c2 = c0;
					c2.r() = CGMKit::Mix(c0.r(), c1.r(), c1.a());
					c2.g() = CGMKit::Mix(c0.g(), c1.g(), c1.a());
					c2.b() = CGMKit::Mix(c0.b(), c1.b(), c1.a());
					c2.a() = CGMKit::Mix(c0.a(), 0, c1.a()); // 0=½�أ�1=����

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