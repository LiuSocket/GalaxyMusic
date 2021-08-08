//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMVolumeBasic.cpp
/// @brief		Galaxy-Music Engine - Volume basic
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.09
//////////////////////////////////////////////////////////////////////////
#include "GMVolumeBasic.h"
#include "GMKit.h"

#include <osg/Texture2D>
#include <osgDB/ReadFile>

using namespace GM;

unsigned int CGMVolumeBasic::m_iShakeCount = 0;

/** @brief ���� */
CGMVolumeBasic::CGMVolumeBasic()
	: m_pKernelData(nullptr), m_pConfigData(nullptr)
	, m_strShaderPath("Shaders/VolumeShader/"), m_strCoreTexturePath("Textures/Volume/"), m_strMediaTexturePath("Volume/")
	, m_bHigh(false), m_iWidthFull(1920), m_iHeightFull(1080), m_iUnitColor(0), m_iUnitVelocity(0)
	, m_mLastVP(osg::Matrixf())
	, m_fNoiseNumUniform(new osg::Uniform("noiseNum", 1.0f))
	, m_vScreenSizeUniform(new osg::Uniform("screenSize", osg::Vec3f(1920.0f, 1080.0f, 0.25f)))
	, m_vShakeVectorUniform(new osg::Uniform("shakeVec", osg::Vec2f(-1.0f, -1.0f)))
	, m_iUnitTAA(2)
{}

/**
* GetShakeParameters
* ���������������������TAA���ɽ��ֱ������4��
* @author LiuTao
* @since 2020.08.17
* @param fShake0:	����ֵ0��-1.5/-0.5/+0.5/+1.5
* @param fShake1:	����ֵ1��-1.5/-0.5/+0.5/+1.5
* @return void
*/
void CGMVolumeBasic::GetShakeParameters(float & fShake0, float & fShake1)
{
	// 4 ֡һ��ѭ��
	fShake0 = ((m_iShakeCount / 2) % 2)*2.0 - 1.0;
	fShake1 = (((m_iShakeCount + 1) / 2) % 2)*2.0 - 1.0;

	// 16 ֡һ��ѭ��
	fShake0 += (((m_iShakeCount / 4) / 2) % 2) - 0.5;
	fShake1 += (((m_iShakeCount / 4) + 1) % 2) - 0.5;
}

/**
* AddShakeCount
* �������ͬ�������ļ���
* @author LiuTao
* @since 2020.08.18
* @return void
*/
void CGMVolumeBasic::AddShakeCount()
{
	m_iShakeCount++;
}

/**
* GetShakeCount
* ��ȡ���ͬ�������ļ���
* @author LiuTao
* @since 2020.08.18
* @return m_iShakeCount ����ͬ�������ļ���
*/
unsigned int CGMVolumeBasic::GetShakeCount()
{
	return m_iShakeCount;
}

/** @brief ���� */
CGMVolumeBasic::~CGMVolumeBasic()
{
}

/** @brief ��ʼ�� */
void CGMVolumeBasic::Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData)
{
	m_pKernelData = pKernelData;
	m_pConfigData = pConfigData;

	std::string strTexturePath = pConfigData->strCorePath + m_strCoreTexturePath;
	m_blueNoiseTex = _CreateTexture2D(strTexturePath + "BlueNoise.jpg", 1);

	_InitTAA(pConfigData->strCorePath);
}

/** @brief ����(�������������̬֮��) */
void CGMVolumeBasic::UpdateLater(double dDeltaTime)
{
	if (m_fNoiseNumUniform.valid())
		m_fNoiseNumUniform->set(0.01f * float((m_iRandom() % 100)));
}

/**
* ActiveTAA
* ��TAA�ڵ������Texture��������TAA
* @author LiuTao
* @since 2020.09.01
* @param pTex ��ҪTAA����������ָ��
* @param pVectorTex �ٶ�ʸ������ָ��
* @param pDistanceTex ���ơ����塢�����ľ�����Ϣ����ָ��
* @return bool �ɹ�Ϊtrue��ʧ��Ϊfalse
*/
bool CGMVolumeBasic::ActiveTAA(osg::Texture* pTex, osg::Texture* pVectorTex, osg::Texture* pDistanceTex)
{
	if (!m_statesetTAA.valid()) return false;

	m_iUnitColor = m_iUnitTAA;
	m_statesetTAA->setTextureAttributeAndModes(m_iUnitColor, pTex);
	m_statesetTAA->addUniform(new osg::Uniform("currentTex", m_iUnitColor));
	m_iUnitTAA++;
	m_iUnitVelocity = m_iUnitTAA;
	m_statesetTAA->setTextureAttributeAndModes(m_iUnitVelocity, pVectorTex);
	m_statesetTAA->addUniform(new osg::Uniform("velocityTex", m_iUnitVelocity));
	m_iUnitTAA++;
	m_statesetTAA->setTextureAttributeAndModes(m_iUnitTAA, pDistanceTex);
	m_statesetTAA->addUniform(new osg::Uniform("currentDistanceTex", m_iUnitTAA));
	m_iUnitTAA++;

	GM_Root->addChild(m_cameraTAA.get());
	return true;
}

/**
* _CreateTexture2D
* ����2D����
* @author LiuTao
* @since 2020.06.16
* @param fileName: ͼƬ�ļ�·��
* @param iChannelNum: 1��2��3��4�ֱ����R��RG��RGB��RGBA
* @return osg::Texture* ��������ָ��
*/
osg::Texture* CGMVolumeBasic::_CreateTexture2D(const std::string & fileName, const int iChannelNum)
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

/**
* �޸���Ļ�ߴ�ʱ,������ô˺���
* @author LiuTao
* @since 2020.12.07
* @param width: ��Ļ���
* @param height: ��Ļ�߶�
* @return void
*/
void CGMVolumeBasic::ResizeScreen(const int width, const int height)
{
	_ResizeScreenTriangle(width, height);

	float fRTTRatio = m_bHigh ? 0.5f : 0.25f;
	if (m_cameraTAA.valid())
	{
		m_cameraTAA->resize(width, height);
		m_cameraTAA->setProjectionMatrixAsOrtho2D(0, width, 0, height);
	}
	if (m_TAAMap_0.valid() && m_TAAMap_1.valid())
	{
		m_TAAMap_0->setTextureSize(width, height);
		m_TAAMap_0->dirtyTextureObject();
		m_TAAMap_1->setTextureSize(width, height);
		m_TAAMap_1->dirtyTextureObject();
	}
	if (m_vScreenSizeUniform.valid())
	{
		m_vScreenSizeUniform->set(osg::Vec3f(width, height, fRTTRatio));
	}
}

/**
* _InitTAA
* TAA��ʼ������������Ⱦ
* @author LiuTao
* @since 2020.09.01
* @param strCorePath ����·��
* @return void
*/
void CGMVolumeBasic::_InitTAA(std::string strCorePath)
{
	m_TAAMap_0 = new osg::Texture2D;
	m_TAAMap_0->setName("TAAMap_0");
	m_TAAMap_0->setTextureSize(m_iWidthFull, m_iHeightFull);
	m_TAAMap_0->setInternalFormat(GL_RGBA8);
	m_TAAMap_0->setSourceFormat(GL_RGBA);
	m_TAAMap_0->setSourceType(GL_UNSIGNED_BYTE);
	m_TAAMap_0->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_TAAMap_0->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_TAAMap_0->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_TAAMap_0->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_TAAMap_0->setDataVariance(osg::Object::DYNAMIC);
	m_TAAMap_0->setResizeNonPowerOfTwoHint(false);

	m_TAAMap_1 = new osg::Texture2D;
	m_TAAMap_1->setName("TAAMap_1");
	m_TAAMap_1->setTextureSize(m_iWidthFull, m_iHeightFull);
	m_TAAMap_1->setInternalFormat(GL_RGBA8);
	m_TAAMap_1->setSourceFormat(GL_RGBA);
	m_TAAMap_1->setSourceType(GL_UNSIGNED_BYTE);
	m_TAAMap_1->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_TAAMap_1->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_TAAMap_1->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_TAAMap_1->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_TAAMap_1->setDataVariance(osg::Object::DYNAMIC);
	m_TAAMap_1->setResizeNonPowerOfTwoHint(false);

	m_distanceMap_0 = new osg::Texture2D;
	m_distanceMap_0->setName("distanceMap_0");
	m_distanceMap_0->setTextureSize(m_iWidthFull, m_iHeightFull);
	m_distanceMap_0->setInternalFormat(GL_RGB32F_ARB);
	m_distanceMap_0->setSourceFormat(GL_RGB);
	m_distanceMap_0->setSourceType(GL_FLOAT);
	m_distanceMap_0->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_distanceMap_0->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_distanceMap_0->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_distanceMap_0->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_distanceMap_0->setDataVariance(osg::Object::DYNAMIC);
	m_distanceMap_0->setResizeNonPowerOfTwoHint(false);

	m_distanceMap_1 = new osg::Texture2D;
	m_distanceMap_1->setName("distanceMap_1");
	m_distanceMap_1->setTextureSize(m_iWidthFull, m_iHeightFull);
	m_distanceMap_1->setInternalFormat(GL_RGB32F_ARB);
	m_distanceMap_1->setSourceFormat(GL_RGB);
	m_distanceMap_1->setSourceType(GL_FLOAT);
	m_distanceMap_1->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_distanceMap_1->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_distanceMap_1->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_distanceMap_1->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_distanceMap_1->setDataVariance(osg::Object::DYNAMIC);
	m_distanceMap_1->setResizeNonPowerOfTwoHint(false);

	m_cameraTAA = new osg::Camera;
	m_cameraTAA->setName("TAACamera");
	m_cameraTAA->setReferenceFrame(osg::Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT);
	m_cameraTAA->setClearMask(GL_COLOR_BUFFER_BIT);
	m_cameraTAA->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
	m_cameraTAA->setViewport(0, 0, m_iWidthFull, m_iHeightFull);
	m_cameraTAA->setRenderOrder(osg::Camera::PRE_RENDER,2);
	m_cameraTAA->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	m_cameraTAA->attach(osg::Camera::COLOR_BUFFER0, m_TAAMap_0.get());
	m_cameraTAA->attach(osg::Camera::COLOR_BUFFER1, m_distanceMap_0.get());
	m_cameraTAA->setAllowEventFocus(false);
	m_cameraTAA->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	m_cameraTAA->setViewMatrix(osg::Matrix::identity());
	m_cameraTAA->setProjectionMatrixAsOrtho2D(0, m_iWidthFull, 0, m_iHeightFull);
	m_cameraTAA->setProjectionResizePolicy(osg::Camera::FIXED);

	m_pCloudTAAGeode = new osg::Geode();
	m_pCloudTAAGeode->addDrawable(_CreateScreenTriangle(m_iWidthFull, m_iHeightFull));
	m_cameraTAA->addChild(m_pCloudTAAGeode.get());

	m_statesetTAA = m_pCloudTAAGeode->getOrCreateStateSet();
	m_statesetTAA->addUniform(m_vScreenSizeUniform.get());
	m_statesetTAA->addUniform(m_vShakeVectorUniform.get());

	m_statesetTAA->setTextureAttributeAndModes(0, m_TAAMap_1.get());
	m_statesetTAA->addUniform(new osg::Uniform("lastTex", 0));
	m_statesetTAA->setTextureAttributeAndModes(1, m_distanceMap_1.get());
	m_statesetTAA->addUniform(new osg::Uniform("lastDistanceTex", 1));
	m_statesetTAA->setTextureAttributeAndModes(m_iUnitTAA, m_blueNoiseTex.get());
	m_statesetTAA->addUniform(new osg::Uniform("blueNoiseSampler", m_iUnitTAA));
	m_iUnitTAA++;

	std::string strTAAVertPath = strCorePath + m_strShaderPath + "TAAVert.glsl";
	std::string strTAAFragPath = strCorePath + m_strShaderPath + "TAAFrag.glsl";
	CGMKit::LoadShader(m_statesetTAA.get(), strTAAVertPath, strTAAFragPath);
}

/**
* ������Ⱦ��
* @author LiuTao
* @since 2020.09.01
* @param width: ��Ŀ��
* @param height: ��ĸ߶�
* @return osg::Geometry* ���ؼ��νڵ�ָ��
*/
osg::Geometry* CGMVolumeBasic::_CreateScreenTriangle(const int width, const int height)
{
	osg::Geometry* pGeometry = new osg::Geometry();

	osg::ref_ptr<osg::Vec3Array> verArray = new osg::Vec3Array;
	verArray->push_back(osg::Vec3(0, 0, -1));
	verArray->push_back(osg::Vec3(2*width, 0, -1));
	verArray->push_back(osg::Vec3(0, 2*height, -1));
	pGeometry->setVertexArray(verArray);

	osg::ref_ptr<osg::Vec2Array> textArray = new osg::Vec2Array;
	textArray->push_back(osg::Vec2(0, 0));
	textArray->push_back(osg::Vec2(2, 0));
	textArray->push_back(osg::Vec2(0, 2));
	pGeometry->setTexCoordArray(0, textArray);

	osg::ref_ptr < osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 1, 0));
	pGeometry->setNormalArray(normal);
	pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

	pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, 3));

	pGeometry->setUseVertexBufferObjects(true);
	pGeometry->setUseDisplayList(false);
	pGeometry->setDataVariance(osg::Object::DYNAMIC);
	pGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pGeometry->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

	return pGeometry;
}

/**
* ����Ⱦ������ߴ�
* @brief ��Ļ������С�������棬�Ⱦ���Ч��Ҫ��һЩ
* @author LiuTao
* @since 2020.12.07
* @param width: ��Ч���εĿ��
* @param height: ��Ч���εĸ߶�
* @return void
*/
void CGMVolumeBasic::_ResizeScreenTriangle(const int width, const int height)
{
	osg::ref_ptr<osg::Geometry> pGeometry = m_pCloudTAAGeode->asGeode()->getDrawable(0)->asGeometry();
	osg::ref_ptr<osg::Vec3Array> verArray = new osg::Vec3Array;
	verArray->push_back(osg::Vec3(0, 0, -1));
	verArray->push_back(osg::Vec3(2 * width, 0, -1));
	verArray->push_back(osg::Vec3(0, 2 * height, -1));
	pGeometry->setVertexArray(verArray);
	pGeometry->dirtyBound();
}
