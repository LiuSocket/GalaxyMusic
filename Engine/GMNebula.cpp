//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMVolumeBasic.h
/// @brief		Galaxy-Music Engine - GMNebula.cpp
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.10
//////////////////////////////////////////////////////////////////////////

#include "GMNebula.h"
#include "GMKit.h"

#include <osg/BlendFunc>
#include <osg/CullFace>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osgDB/ReadFile>

using namespace GM;

/*************************************************************************
CGMNebula Methods
*************************************************************************/

class NebulaResizeEventHandler : public osgGA::GUIEventHandler
{
public:
	NebulaResizeEventHandler(CGMNebula*	pNebula)
		: _pNebula(pNebula) {}

	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&)
	{
		if (ea.getEventType() == osgGA::GUIEventAdapter::RESIZE)
		{
			int vp_Width = ea.getWindowWidth();
			int vp_Height = ea.getWindowHeight();
			if (_pNebula)
			{
				_pNebula->ResizeScreen(vp_Width, vp_Height);
			}
			return true;
		}
		return false;
	}
private:
	CGMNebula* _pNebula = nullptr;
};

/** @brief 构造 */
CGMNebula::CGMNebula()
	: CGMVolumeBasic()
	, m_strNebulaShaderPath("Shaders/NebulaShader/"), m_strCoreNebulaTexPath("Textures/Nebula/")
	, m_iUnitRayMarch(2), m_iUnitFinal(1), m_dTimeLastFrame(0.0), m_vLastShakeVec(osg::Vec2f(0.0f,0.0f))
	, m_vDeltaShakeUniform(new osg::Uniform("deltaShakeVec", osg::Vec2f(0.0f, 0.0f)))
	, m_fCountUniform(new osg::Uniform("countNum", 0.0f))
	, m_pDeltaVPMatrixUniform(new osg::Uniform("deltaViewProjMatrix", osg::Matrixf()))
{
}

/** @brief 析构 */
CGMNebula::~CGMNebula()
{
}

/** @brief 初始化 */
bool CGMNebula::Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData)
{
	CGMVolumeBasic::Init(pKernelData, pConfigData);

	m_3DShapeTex_128 = _Load3DShapeNoise();
	m_3DErosionTex = _Load3DErosionNoise();
	m_2DNoiseTex = _CreateTexture2D(m_pConfigData->strCorePath + m_strCoreNebulaTexPath + "2DNoise.jpg", 3);

	return true;
}

/** @brief 更新 */
bool CGMNebula::Update(double dDeltaTime)
{
	return true;
}

/** @brief 更新(在主相机更新姿态之后) */
bool CGMNebula::UpdateLater(double dDeltaTime)
{
	if (m_rayMarchCamera.valid() && m_cameraTAA.valid() && m_rayMarchCamera->getCullMask())
	{
		double fLeft, fRight, fBottom, fTop, fZNear, fZFar;
		bool bFrustum = GM_View->getCamera()->getProjectionMatrixAsFrustum(fLeft, fRight, fBottom, fTop, fZNear, fZFar);
		if (bFrustum)
		{
			unsigned int iShakeCount = GetShakeCount();
			m_fCountUniform->set(float(iShakeCount));

			unsigned int iAttribute = osg::StateAttribute::ON | osg::StateAttribute::PROTECTED;
			if (iShakeCount % 2)
			{
				m_stateSetRayMarch->setTextureAttributeAndModes(0, m_rayMarchMap_0.get(), iAttribute);
				m_stateSetRayMarch->setTextureAttributeAndModes(1, m_vectorMap_0.get(), iAttribute);
				m_rayMarchCamera->attach(osg::Camera::COLOR_BUFFER0, m_rayMarchMap_1.get());
				m_rayMarchCamera->attach(osg::Camera::COLOR_BUFFER1, m_vectorMap_1.get());
				m_rayMarchCamera->dirtyAttachmentMap();

				m_statesetTAA->setTextureAttributeAndModes(0, m_TAAMap_0.get(), iAttribute);
				m_statesetTAA->setTextureAttributeAndModes(1, m_TAADistanceMap_0.get(), iAttribute);
				m_statesetTAA->setTextureAttributeAndModes(m_iUnitColor, m_rayMarchMap_1.get(), iAttribute);
				m_statesetTAA->setTextureAttributeAndModes(m_iUnitVelocity, m_vectorMap_1.get(), iAttribute);

				m_cameraTAA->attach(osg::Camera::COLOR_BUFFER0, m_TAAMap_1.get());
				m_cameraTAA->attach(osg::Camera::COLOR_BUFFER1, m_TAADistanceMap_1.get());
				m_cameraTAA->dirtyAttachmentMap();

				m_stateSetFinal->setTextureAttributeAndModes(0, m_TAAMap_1.get(), iAttribute);
			}
			else
			{
				m_stateSetRayMarch->setTextureAttributeAndModes(0, m_rayMarchMap_1.get(), iAttribute);
				m_stateSetRayMarch->setTextureAttributeAndModes(1, m_vectorMap_1.get(), iAttribute);
				m_rayMarchCamera->attach(osg::Camera::COLOR_BUFFER0, m_rayMarchMap_0.get());
				m_rayMarchCamera->attach(osg::Camera::COLOR_BUFFER1, m_vectorMap_0.get());
				m_rayMarchCamera->dirtyAttachmentMap();

				m_statesetTAA->setTextureAttributeAndModes(0, m_TAAMap_1.get(), iAttribute);
				m_statesetTAA->setTextureAttributeAndModes(1, m_TAADistanceMap_1.get(), iAttribute);
				m_statesetTAA->setTextureAttributeAndModes(m_iUnitColor, m_rayMarchMap_0.get(), iAttribute);
				m_statesetTAA->setTextureAttributeAndModes(m_iUnitVelocity, m_vectorMap_0.get(), iAttribute);
				m_cameraTAA->attach(osg::Camera::COLOR_BUFFER0, m_TAAMap_0.get());
				m_cameraTAA->attach(osg::Camera::COLOR_BUFFER1, m_TAADistanceMap_0.get());
				m_cameraTAA->dirtyAttachmentMap();

				m_stateSetFinal->setTextureAttributeAndModes(0, m_TAAMap_0.get(), iAttribute);
			}

			float fShake0, fShake1;
			GetShakeParameters(fShake0, fShake1);

			osg::Vec2f vShakeVec = osg::Vec2f(fShake0, fShake1);
			osg::Vec2f vDeltaShake = vShakeVec - m_vLastShakeVec;
			m_vShakeVectorUniform->set(vShakeVec);
			m_vDeltaShakeUniform->set(vDeltaShake);

			// 修改m_rayMarchCamera的投影矩阵，实现抖动，使最终渲染的分辨率提高四倍
			osg::Matrixd mMainViewMatrix = GM_View->getCamera()->getViewMatrix();
			osg::Matrixd mMainProjMatrix = GM_View->getCamera()->getProjectionMatrix();
			osg::Matrixd VP = mMainViewMatrix*mMainProjMatrix;

			int iWidth = m_TAAMap_0->getTextureWidth();
			int iHeight = m_TAAMap_0->getTextureHeight();
			double fWidthShake = (fRight - fLeft) / double(iWidth);
			double fHeightShake = (fTop - fBottom) / double(iHeight);
			double fShakeLeft = fLeft + fShake0 * fWidthShake;
			double fShakeRight = fRight + fShake0 * fWidthShake;
			double fShakeBottom = fBottom + fShake1 * fHeightShake;
			double fShakeTop = fTop + fShake1 * fHeightShake;
			m_rayMarchCamera->setViewMatrix(mMainViewMatrix);
			m_rayMarchCamera->setProjectionMatrixAsFrustum(fShakeLeft, fShakeRight, fShakeBottom, fShakeTop, fZNear, fZFar);

			if (m_pDeltaVPMatrixUniform.valid())
			{
				// 修改差值矩阵
				osg::Matrixf deltaVP = osg::Matrixf(
					VP(0, 0) - m_mLastVP(0, 0), VP(0, 1) - m_mLastVP(0, 1), VP(0, 2) - m_mLastVP(0, 2), VP(0, 3) - m_mLastVP(0, 3),
					VP(1, 0) - m_mLastVP(1, 0), VP(1, 1) - m_mLastVP(1, 1), VP(1, 2) - m_mLastVP(1, 2), VP(1, 3) - m_mLastVP(1, 3),
					VP(2, 0) - m_mLastVP(2, 0), VP(2, 1) - m_mLastVP(2, 1), VP(2, 2) - m_mLastVP(2, 2), VP(2, 3) - m_mLastVP(2, 3),
					VP(3, 0) - m_mLastVP(3, 0), VP(3, 1) - m_mLastVP(3, 1), VP(3, 2) - m_mLastVP(3, 2), VP(3, 3) - m_mLastVP(3, 3)
				);
				m_pDeltaVPMatrixUniform->set(deltaVP);
			}

			m_mLastVP = VP;
			m_vLastShakeVec = vShakeVec;
		}
	}

	CGMVolumeBasic::UpdateLater(dDeltaTime);
	return true;
}

/** @brief 加载 */
bool CGMNebula::Load()
{
	if (m_stateSetRayMarch.valid() && m_stateSetFinal.valid())
	{
		std::string strNebulaVertPath = m_pConfigData->strCorePath + m_strNebulaShaderPath + "NebulaVert.glsl";
		std::string strNebulaFragPath = m_pConfigData->strCorePath + m_strNebulaShaderPath + "NebulaFrag.glsl";
		CGMKit::LoadShader(m_stateSetRayMarch.get(), strNebulaVertPath, strNebulaFragPath);
		CGMKit::LoadShader(m_stateSetFinal.get(), strNebulaVertPath, strNebulaFragPath);
	}
	if (m_statesetTAA.valid())
	{
		std::string strTAAVertPath = m_pConfigData->strCorePath + m_strShaderPath + "TAAVert.glsl";
		std::string strTAAFragPath = m_pConfigData->strCorePath + m_strShaderPath + "TAAFrag.glsl";
		CGMKit::LoadShader(m_statesetTAA.get(), strTAAVertPath, strTAAFragPath);
	}
	return true;
}

// make Nebula for the galaxy
void CGMNebula::MakeNebula(double fLength, double fWidth, double fHeight, double fX, double fY, double fZ)
{
	double fUnit = m_pKernelData->fUnitArray->at(4);
	double fLengthHie = fLength / fUnit;
	double fWidthHie = fWidth / fUnit;
	double fHeightHie = fHeight / fUnit;

	double fXHie = fX / fUnit;
	double fYHie = fY / fUnit;
	double fZHie = fZ / fUnit;

	std::string strTexturePath = m_pConfigData->strCorePath + m_strCoreNebulaTexPath;
	m_galaxyTex = _CreateTexture2D(strTexturePath + "milkyWay_color.tga", 4);
	m_galaxyHeightTex = _CreateTexture2D(strTexturePath + "milkyWay_height.tga", 4);

	osg::ref_ptr<osg::Geometry> pBoxGeom = MakeBoxGeometry(fLengthHie, fWidthHie, fHeightHie);
	osg::ref_ptr<osg::Geode> pBoxGeode = new osg::Geode();
	pBoxGeode->addDrawable(pBoxGeom.get());
	m_nebulaRayMarchTransform = new osg::MatrixTransform();
	m_nebulaRayMarchTransform->addChild(pBoxGeode.get());

	// First step: create the 1/16 ray marching map
	int iWidth = 480;
	int iHeight = 270;

	m_rayMarchMap_0 = new osg::Texture2D;
	m_rayMarchMap_0->setName("rayMarchMap_0");
	m_rayMarchMap_0->setTextureSize(iWidth, iHeight);
	m_rayMarchMap_0->setInternalFormat(GL_RGBA8);
	m_rayMarchMap_0->setSourceFormat(GL_RGBA);
	m_rayMarchMap_0->setSourceType(GL_UNSIGNED_BYTE);
	m_rayMarchMap_0->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_rayMarchMap_0->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_rayMarchMap_0->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_rayMarchMap_0->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_rayMarchMap_0->setDataVariance(osg::Object::DYNAMIC);
	m_rayMarchMap_0->setResizeNonPowerOfTwoHint(false);

	m_rayMarchMap_1 = new osg::Texture2D;
	m_rayMarchMap_1->setName("rayMarchMap_1");
	m_rayMarchMap_1->setTextureSize(iWidth, iHeight);
	m_rayMarchMap_1->setInternalFormat(GL_RGBA8);
	m_rayMarchMap_1->setSourceFormat(GL_RGBA);
	m_rayMarchMap_1->setSourceType(GL_UNSIGNED_BYTE);
	m_rayMarchMap_1->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_rayMarchMap_1->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_rayMarchMap_1->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_rayMarchMap_1->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_rayMarchMap_1->setDataVariance(osg::Object::DYNAMIC);
	m_rayMarchMap_1->setResizeNonPowerOfTwoHint(false);

	m_vectorMap_0 = new osg::Texture2D;
	m_vectorMap_0->setName("vectorMap_0");
	m_vectorMap_0->setTextureSize(iWidth, iHeight);
	m_vectorMap_0->setInternalFormat(GL_RGB32F_ARB);
	m_vectorMap_0->setSourceFormat(GL_RGB);
	m_vectorMap_0->setSourceType(GL_FLOAT);
	m_vectorMap_0->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_vectorMap_0->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_vectorMap_0->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_vectorMap_0->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_vectorMap_0->setDataVariance(osg::Object::DYNAMIC);
	m_vectorMap_0->setResizeNonPowerOfTwoHint(false);

	m_vectorMap_1 = new osg::Texture2D;
	m_vectorMap_1->setName("vectorMap_1");
	m_vectorMap_1->setTextureSize(iWidth, iHeight);
	m_vectorMap_1->setInternalFormat(GL_RGB32F_ARB);
	m_vectorMap_1->setSourceFormat(GL_RGB);
	m_vectorMap_1->setSourceType(GL_FLOAT);
	m_vectorMap_1->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_vectorMap_1->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_vectorMap_1->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_vectorMap_1->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_vectorMap_1->setDataVariance(osg::Object::DYNAMIC);
	m_vectorMap_1->setResizeNonPowerOfTwoHint(false);

	m_distanceMap = new osg::Texture2D;
	m_distanceMap->setName("distanceMap");
	m_distanceMap->setTextureSize(iWidth, iHeight);
	m_distanceMap->setInternalFormat(GL_RGB32F_ARB);
	m_distanceMap->setSourceFormat(GL_RGB);
	m_distanceMap->setSourceType(GL_FLOAT);
	m_distanceMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_distanceMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_distanceMap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_distanceMap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_distanceMap->setDataVariance(osg::Object::DYNAMIC);
	m_distanceMap->setResizeNonPowerOfTwoHint(false);

	// Create its camera and render to it
	m_rayMarchCamera = new osg::Camera;
	m_rayMarchCamera->setName("rayMarchCamera");
	m_rayMarchCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT);
	m_rayMarchCamera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	m_rayMarchCamera->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
	m_rayMarchCamera->setViewport(0, 0, iWidth, iHeight);
	m_rayMarchCamera->setRenderOrder(osg::Camera::PRE_RENDER, 1);
	m_rayMarchCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	m_rayMarchCamera->attach(osg::Camera::COLOR_BUFFER0, m_rayMarchMap_0.get());
	m_rayMarchCamera->attach(osg::Camera::COLOR_BUFFER1, m_vectorMap_0.get());
	m_rayMarchCamera->attach(osg::Camera::COLOR_BUFFER2, m_distanceMap.get());
	m_rayMarchCamera->setAllowEventFocus(false);
	m_rayMarchCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	m_rayMarchCamera->addChild(m_nebulaRayMarchTransform.get());

	m_stateSetRayMarch = pBoxGeode->getOrCreateStateSet();
	m_stateSetRayMarch->setMode(GL_BLEND, osg::StateAttribute::ON);
	m_stateSetRayMarch->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	m_stateSetRayMarch->setAttributeAndModes(new osg::CullFace());
	m_stateSetRayMarch->setDefine("RAYMARCHING", osg::StateAttribute::ON);

	SGMVolumeRange sVR;
	sVR.fXMin = fXHie - fLengthHie * 0.5;
	sVR.fXMax = fXHie + fLengthHie * 0.5;
	sVR.fYMin = fYHie - fWidthHie * 0.5;
	sVR.fYMax = fYHie + fWidthHie * 0.5;
	sVR.fZMin = fZHie - fHeightHie * 0.5;
	sVR.fZMax = fZHie + fHeightHie * 0.5;

	osg::Vec3f vRangeMin = osg::Vec3f(sVR.fXMin, sVR.fYMin, sVR.fZMin);
	osg::ref_ptr<osg::Uniform> pRangeMin = new osg::Uniform("rangeMin", vRangeMin);
	m_stateSetRayMarch->addUniform(pRangeMin.get());
	osg::Vec3f vRangeMax = osg::Vec3f(sVR.fXMax, sVR.fYMax, sVR.fZMax);
	osg::ref_ptr<osg::Uniform> pRangeMax = new osg::Uniform("rangeMax", vRangeMax);
	m_stateSetRayMarch->addUniform(pRangeMax.get());

	m_stateSetRayMarch->addUniform(m_pDeltaVPMatrixUniform.get());
	m_stateSetRayMarch->addUniform(m_fCountUniform.get());
	m_stateSetRayMarch->addUniform(m_fNoiseNumUniform.get());
	m_stateSetRayMarch->addUniform(m_vScreenSizeUniform.get());
	m_stateSetRayMarch->addUniform(m_vDeltaShakeUniform.get());

	m_stateSetRayMarch->setTextureAttributeAndModes(0, m_rayMarchMap_1.get());
	m_stateSetRayMarch->addUniform(new osg::Uniform("lastRayMarchTex", 0));
	m_stateSetRayMarch->setTextureAttributeAndModes(1, m_vectorMap_1.get());
	m_stateSetRayMarch->addUniform(new osg::Uniform("lastVectorTex", 1));

	m_stateSetRayMarch->setTextureAttributeAndModes(m_iUnitRayMarch, m_galaxyTex.get());
	m_stateSetRayMarch->addUniform(new osg::Uniform("galaxyTex", m_iUnitRayMarch));
	m_iUnitRayMarch++;
	m_stateSetRayMarch->setTextureAttributeAndModes(m_iUnitRayMarch, m_galaxyHeightTex.get());
	m_stateSetRayMarch->addUniform(new osg::Uniform("galaxyHeightTex", m_iUnitRayMarch));
	m_iUnitRayMarch++;
	m_stateSetRayMarch->setTextureAttributeAndModes(m_iUnitRayMarch, m_2DNoiseTex.get());
	m_stateSetRayMarch->addUniform(new osg::Uniform("noise2DTex", m_iUnitRayMarch));
	m_iUnitRayMarch++;
	m_stateSetRayMarch->setTextureAttributeAndModes(m_iUnitRayMarch, m_blueNoiseTex.get());
	m_stateSetRayMarch->addUniform(new osg::Uniform("blueNoiseSampler", m_iUnitRayMarch));
	m_iUnitRayMarch++;
	m_stateSetRayMarch->setTextureAttributeAndModes(m_iUnitRayMarch, _Load3DCurlNoise());
	m_stateSetRayMarch->addUniform(new osg::Uniform("noiseCurlTex", m_iUnitRayMarch));
	m_iUnitRayMarch++;
	m_stateSetRayMarch->setTextureAttributeAndModes(m_iUnitRayMarch, m_3DShapeTex_128.get());
	m_stateSetRayMarch->addUniform(new osg::Uniform("noiseShapeTex", m_iUnitRayMarch));
	m_iUnitRayMarch++;
	m_stateSetRayMarch->setTextureAttributeAndModes(m_iUnitRayMarch, m_3DErosionTex.get());
	m_stateSetRayMarch->addUniform(new osg::Uniform("noiseErosionTex", m_iUnitRayMarch));
	m_iUnitRayMarch++;

	std::string strNebulaVertPath = m_pConfigData->strCorePath + m_strNebulaShaderPath + "NebulaVert.glsl";
	std::string strNebulaFragPath = m_pConfigData->strCorePath + m_strNebulaShaderPath + "NebulaFrag.glsl";
	CGMKit::LoadShader(m_stateSetRayMarch.get(), strNebulaVertPath, strNebulaFragPath);

	m_pHierarchyRootVector.at(4)->addChild(m_rayMarchCamera.get());

	/////////////////////
	// Second step: get and mix the last frame by TAA(temporal anti-aliasing)
	// Add texture to TAA board,and active TAA
	ActiveTAA(m_rayMarchMap_0.get(), m_vectorMap_0.get(), m_distanceMap.get());

	/////////////////////
	// Third step: render the final cloud texture
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	geode->addDrawable(pBoxGeom.get());
	m_nebulaRayMarchTransform = new osg::MatrixTransform();
	m_nebulaRayMarchTransform->addChild(geode.get());
	//m_nebulaRayMarchTransform->addChild(osgDB::readNodeFile(m_pConfigData->strMediaPath + "Models/test_1.fbx"));
	m_pHierarchyRootVector.at(4)->addChild(m_nebulaRayMarchTransform.get());

	// configure the state set:
	m_stateSetFinal = geode->getOrCreateStateSet();
	m_stateSetFinal->setAttributeAndModes(new osg::BlendFunc());
	m_stateSetFinal->setMode(GL_BLEND, osg::StateAttribute::ON);
	m_stateSetFinal->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	m_stateSetFinal->setAttributeAndModes(new osg::CullFace());
	m_stateSetFinal->setRenderBinDetails(BIN_NEBULA, "DepthSortedBin");
	m_stateSetFinal->setDefine("RAYMARCHING", osg::StateAttribute::OFF);

	m_stateSetFinal->addUniform(pRangeMin.get());
	m_stateSetFinal->addUniform(pRangeMax.get());
	m_stateSetFinal->addUniform(m_vScreenSizeUniform.get());

	m_stateSetFinal->setTextureAttributeAndModes(0, m_TAAMap_0.get());
	m_stateSetFinal->addUniform(new osg::Uniform("TAATex", 0));

	m_stateSetFinal->setTextureAttributeAndModes(m_iUnitFinal, m_galaxyTex.get());
	m_stateSetFinal->addUniform(new osg::Uniform("galaxyTex", m_iUnitFinal));
	m_iUnitFinal++;

	CGMKit::LoadShader(m_stateSetFinal.get(), strNebulaVertPath, strNebulaFragPath);

	GM_View->addEventHandler(new NebulaResizeEventHandler(this));
}

/** @brief: Show or hide the Nebula*/
void
CGMNebula::SetNebulaEnabled(const bool bEnable)
{
	unsigned int iMask = bEnable ? ~0 : 0;

	if (m_rayMarchCamera.valid())
	{
		m_rayMarchCamera->setCullMask(iMask);
	}
}

/** @brief Get the No.0 switching texture for TAA */
osg::Texture2D* CGMNebula::GetTAAMap_0()
{
	return m_TAAMap_0.get();
}

/** @brief Get the No.1switching texture for TAA */
osg::Texture2D* CGMNebula::GetTAAMap_1()
{
	return m_TAAMap_1.get();
}

/**
* 修改屏幕尺寸时调用此函数
* @author LiuTao
* @since 2020.12.07
* @param width: 屏幕宽度
* @param height: 屏幕高度
* @return void
*/
void CGMNebula::ResizeScreen(const int width, const int height)
{
	float fRTTRatio = m_bHigh ? 0.5f : 0.25f;
	int iW = std::ceil(fRTTRatio*width);
	int iH = std::ceil(fRTTRatio*height);
	if (m_rayMarchCamera.valid())
	{
		m_rayMarchCamera->resize(iW, iH);

		m_rayMarchMap_0->setTextureSize(iW, iH);
		m_rayMarchMap_0->dirtyTextureObject();
		m_rayMarchMap_1->setTextureSize(iW, iH);
		m_rayMarchMap_1->dirtyTextureObject();

		m_vectorMap_0->setTextureSize(iW, iH);
		m_vectorMap_0->dirtyTextureObject();
		m_vectorMap_1->setTextureSize(iW, iH);
		m_vectorMap_1->dirtyTextureObject();

		m_distanceMap->setTextureSize(iW, iH);
		m_distanceMap->dirtyTextureObject();
	}
	CGMVolumeBasic::ResizeScreen(width, height);
}

osg::Geometry* CGMNebula::MakeSphereGeometry(float fRadius, int iLatSegment)
{
	int iLonSegment = 2 * iLatSegment;

	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);

	float fSegmentSize = osg::PI / iLatSegment; // 弧度

	osg::Vec3Array* verts = new osg::Vec3Array();
	verts->reserve((iLatSegment + 1) * (iLonSegment + 1));

	osg::Vec2Array* texCoords = 0;
	osg::Vec3Array* normals = 0;

	texCoords = new osg::Vec2Array();
	texCoords->reserve((iLatSegment + 1) * (iLonSegment + 1));
	geom->setTexCoordArray(0, texCoords);

	normals = new osg::Vec3Array();
	normals->reserve((iLatSegment + 1) * (iLonSegment + 1));
	geom->setNormalArray(normals);
	geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

	osg::DrawElementsUShort* el = new osg::DrawElementsUShort(GL_TRIANGLES);
	el->reserve(iLatSegment * iLonSegment * 6);

	for (int y = 0; y <= iLatSegment; ++y)
	{
		float lat = -osg::PI_2 + fSegmentSize * (float)y; // 弧度
		for (int x = 0; x <= iLonSegment; ++x)
		{
			float lon = -osg::PI + fSegmentSize * (float)x; // 弧度
			float sin_latitude = sin(lat);
			float cos_latitude = cos(lat);
			float fU = cos_latitude * sin(lon);
			float fV = sin_latitude;
			float gx = fRadius * fU;
			float gy = fRadius * fV;
			float gz = fRadius * cos_latitude*cos(lon);
			verts->push_back(osg::Vec3(gx, gy, gz));
			texCoords->push_back(osg::Vec2(fU, fV));

			osg::Vec3 normal(-gx, -gy, -gz);
			normal.normalize();
			normals->push_back(normal);

			if ((y < iLatSegment) && (x < iLonSegment))
			{
				int x_plus_1 = x + 1;
				int y_plus_1 = y + 1;
				el->push_back(y*(iLonSegment + 1) + x);
				el->push_back(y_plus_1*(iLonSegment + 1) + x);
				el->push_back(y*(iLonSegment + 1) + x_plus_1);
				el->push_back(y*(iLonSegment + 1) + x_plus_1);
				el->push_back(y_plus_1*(iLonSegment + 1) + x);
				el->push_back(y_plus_1*(iLonSegment + 1) + x_plus_1);
			}
		}
	}

	geom->setVertexArray(verts);
	geom->addPrimitiveSet(el);

	return geom;
}

osg::Geometry* CGMNebula::MakeBoxGeometry(float fLength, float fWidth, float fHeight)
{
	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);

	osg::Vec3Array* verts = new osg::Vec3Array();
	verts->reserve(8);

	osg::Vec3Array* texCoords = new osg::Vec3Array();
	texCoords->reserve(8);

	osg::DrawElementsUShort* el = new osg::DrawElementsUShort(GL_TRIANGLES);
	el->reserve(36);

	float fHalfLength = 0.5*fLength;
	float fHalfWidth = 0.5*fWidth;
	float fHalfHeight = 0.5*fHeight;

	verts->push_back(osg::Vec3(-fHalfLength, -fHalfWidth, -fHalfHeight)); // 0
	verts->push_back(osg::Vec3(fHalfLength, -fHalfWidth, -fHalfHeight)); // 1
	verts->push_back(osg::Vec3(fHalfLength, fHalfWidth, -fHalfHeight)); // 2
	verts->push_back(osg::Vec3(-fHalfLength, fHalfWidth, -fHalfHeight)); // 3
	verts->push_back(osg::Vec3(-fHalfLength, -fHalfWidth, fHalfHeight)); // 4
	verts->push_back(osg::Vec3(fHalfLength, -fHalfWidth, fHalfHeight)); // 5
	verts->push_back(osg::Vec3(fHalfLength, fHalfWidth, fHalfHeight)); // 6
	verts->push_back(osg::Vec3(-fHalfLength, fHalfWidth, fHalfHeight)); // 7


	texCoords->push_back(osg::Vec3(0, 0, 0)); // 0
	texCoords->push_back(osg::Vec3(1, 0, 0)); // 1
	texCoords->push_back(osg::Vec3(1, 1, 0)); // 2
	texCoords->push_back(osg::Vec3(0, 1, 0)); // 3
	texCoords->push_back(osg::Vec3(0, 0, 1)); // 4
	texCoords->push_back(osg::Vec3(1, 0, 1)); // 5
	texCoords->push_back(osg::Vec3(1, 1, 1)); // 6
	texCoords->push_back(osg::Vec3(0, 1, 1)); // 7

	// bottom z = -1
	el->push_back(0);
	el->push_back(1);
	el->push_back(2);
	el->push_back(0);
	el->push_back(2);
	el->push_back(3);
	// front x = 1
	el->push_back(1);
	el->push_back(6);
	el->push_back(2);
	el->push_back(1);
	el->push_back(5);
	el->push_back(6);
	// top z = 1
	el->push_back(4);
	el->push_back(6);
	el->push_back(5);
	el->push_back(4);
	el->push_back(7);
	el->push_back(6);
	// back x = -1
	el->push_back(0);
	el->push_back(3);
	el->push_back(7);
	el->push_back(0);
	el->push_back(7);
	el->push_back(4);
	// left y = 1
	el->push_back(2);
	el->push_back(6);
	el->push_back(7);
	el->push_back(2);
	el->push_back(7);
	el->push_back(3);
	// right y = -1
	el->push_back(0);
	el->push_back(4);
	el->push_back(5);
	el->push_back(0);
	el->push_back(5);
	el->push_back(1);

	geom->setVertexArray(verts);
	geom->setTexCoordArray(0, texCoords);
	geom->addPrimitiveSet(el);

	return geom;
}

osg::Texture* CGMNebula::_Load3DShapeNoise()
{
	std::string strTexturePath = m_pConfigData->strCorePath + m_strCoreTexturePath + "noiseShape128.raw";
	osg::ref_ptr<osg::Image> shapeImg = osgDB::readImageFile(strTexturePath);
	shapeImg->setImage(128, 128, 128, GL_R8, GL_RED, GL_UNSIGNED_BYTE, shapeImg->data(), osg::Image::NO_DELETE);
	osg::Texture3D* tex3d = new osg::Texture3D;
	tex3d->setImage(shapeImg.get());
	tex3d->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	tex3d->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	tex3d->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	tex3d->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	tex3d->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
	tex3d->setInternalFormat(GL_R8);
	tex3d->setSourceFormat(GL_RED);
	tex3d->setSourceType(GL_UNSIGNED_BYTE);
	tex3d->allocateMipmapLevels();
	return tex3d;
}

osg::Texture* CGMNebula::_Load3DErosionNoise()
{
	std::string strTexturePath = m_pConfigData->strCorePath + m_strCoreTexturePath + "noiseErosion.raw";
	osg::Image* img = osgDB::readImageFile(strTexturePath);
	img->setImage(32, 32, 32, GL_R8, GL_RED, GL_UNSIGNED_BYTE, img->data(), osg::Image::NO_DELETE);
	osg::Texture3D* tex3d = new osg::Texture3D;
	tex3d->setImage(img);
	tex3d->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	tex3d->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	tex3d->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	tex3d->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	tex3d->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
	tex3d->setInternalFormat(GL_R8);
	tex3d->setSourceFormat(GL_RED);
	tex3d->setSourceType(GL_UNSIGNED_BYTE);
	tex3d->allocateMipmapLevels();
	return tex3d;
}

osg::Texture* CGMNebula::_Load3DCurlNoise()
{
	std::string strTexturePath = m_pConfigData->strCorePath + m_strCoreTexturePath + "noiseCurl.raw";
	osg::Image* img = osgDB::readImageFile(strTexturePath);
	img->setImage(128, 128, 4, GL_R8, GL_RED, GL_UNSIGNED_BYTE, img->data(), osg::Image::NO_DELETE);
	osg::Texture3D* tex3d = new osg::Texture3D;
	tex3d->setImage(img);
	tex3d->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	tex3d->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	tex3d->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	tex3d->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	tex3d->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
	tex3d->setInternalFormat(GL_R8);
	tex3d->setSourceFormat(GL_RED);
	tex3d->setSourceType(GL_UNSIGNED_BYTE);
	tex3d->allocateMipmapLevels();
	return tex3d;
}