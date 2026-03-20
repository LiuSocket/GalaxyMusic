//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMMilkyWay.h
/// @brief		Galaxy-Music Engine - GMMilkyWay.cpp
/// @version	1.0
/// @author		LiuTao
/// @date		2023.08.06
//////////////////////////////////////////////////////////////////////////

#include "GMMilkyWay.h"
#include "../GMKit.h"
#include "../GMEngine.h"
#include "../GMCommonUniform.h"

#include <osg/BlendFunc>
#include <osg/CullFace>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osgDB/ReadFile>

using namespace GM;

/*************************************************************************
CGMMilkyWay Methods
*************************************************************************/

/** @brief 构造 */
CGMMilkyWay::CGMMilkyWay(): CGMVolumeBasic(),
	m_strCoreGalaxyTexPath("Textures/Galaxy/"), m_strGalaxyShaderPath("Shaders/GalaxyShader/")
{
}

/** @brief 析构 */
CGMMilkyWay::~CGMMilkyWay()
{
}

/** @brief 初始化 */
bool CGMMilkyWay::Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform)
{
	CGMVolumeBasic::Init(pKernelData, pConfigData, pCommonUniform);

	m_2DNoiseTex = _CreateTexture2D(m_pConfigData->strCorePath + m_strCoreGalaxyTexPath + "2DNoise.tga", 4);

	if (EGMRENDER_LOWEST != pConfigData->eRenderQuality)
	{
		m_distanceMap = new osg::Texture2D;
		m_distanceMap->setName("distanceMap");
		m_distanceMap->setTextureSize(pConfigData->iScreenWidth / 2, pConfigData->iScreenHeight / 2);
		m_distanceMap->setInternalFormat(GL_RGBA8);
		m_distanceMap->setSourceFormat(GL_RGBA);
		m_distanceMap->setSourceType(GL_UNSIGNED_BYTE);
		m_distanceMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		m_distanceMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
		m_distanceMap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		m_distanceMap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
		m_distanceMap->setDataVariance(osg::Object::DYNAMIC);
		m_distanceMap->setResizeNonPowerOfTwoHint(false);
	}

	return true;
}

/** @brief 更新 */
bool CGMMilkyWay::Update(double dDeltaTime)
{
	if (EGMRENDER_LOWEST == m_pConfigData->eRenderQuality) return true;

	if (4 == m_pKernelData->iHierarchy)
	{
		double fDistance = GM_ENGINE.GetHierarchyTargetDistance();
		if (fDistance > 0.01)
		{
			// 显示节点
			if (0 == m_rayMarchCamera->getNodeMask())
			{
				m_rayMarchCamera->setNodeMask(~0);
				m_pGeodeMilkyWay->setNodeMask(~0);
			}
		}
		else
		{
			// 隐藏节点
			if (0 != m_rayMarchCamera->getNodeMask())
			{
				m_rayMarchCamera->setNodeMask(0);
				m_pGeodeMilkyWay->setNodeMask(0);
			}
		}
	}

	CGMVolumeBasic::Update(dDeltaTime);
	return true;
}

/** @brief 更新(在主相机更新姿态之后) */
bool CGMMilkyWay::UpdateLater(double dDeltaTime)
{
	if (EGMRENDER_LOWEST != m_pConfigData->eRenderQuality &&
		m_rayMarchCamera.valid() && m_rayMarchCamera->getNodeMask())
	{
		// 实现抖动抗锯齿
		osg::Matrixd mMainViewMatrix = GM_View->getCamera()->getViewMatrix();
		osg::Matrixd mMainProjMatrix = GM_View->getCamera()->getProjectionMatrix();
		double fFovy, fAspectRatio, fZNear, fZFar;
		GM_View->getCamera()->getProjectionMatrixAsPerspective(fFovy, fAspectRatio, fZNear, fZFar);

		// 设置 Dodecahedron 的位置
		osg::Matrixd mPos;
		osg::Matrixd mMainViewInverseMatrix = osg::Matrixd::inverse(mMainViewMatrix);
		mPos.setTrans(mMainViewInverseMatrix.getTrans());
		m_pDodecahedronTrans->setMatrix(mPos);
		m_rayMarchCamera->setViewMatrix(mMainViewMatrix);
		m_rayMarchCamera->setProjectionMatrix(mMainProjMatrix);
	}

	CGMVolumeBasic::UpdateLater(dDeltaTime);
	return true;
}

/** @brief 加载 */
bool CGMMilkyWay::Load()
{
	std::string strShader = m_pConfigData->strCorePath + m_strGalaxyShaderPath;

	if (m_pSsMilkyWayDecFace.valid() && m_pSsMilkyWayDecEdge.valid() && m_pSsMilkyWayDecVert.valid())
	{
		std::string strVertPath = strShader + "MilkyWay.vert";
		std::string strFragPath = strShader + "MilkyWay.frag";
		CGMKit::LoadShader(m_pSsMilkyWayDecFace.get(), strVertPath, strFragPath, true);
		CGMKit::LoadShader(m_pSsMilkyWayDecEdge.get(), strVertPath, strFragPath);
		CGMKit::LoadShader(m_pSsMilkyWayDecVert.get(), strVertPath, strFragPath);
	}
	return true;
}

void CGMMilkyWay::MakeMilkyWay(double fLength, double fWidth, double fHeight, double fX, double fY, double fZ)
{
	double fUnit = m_pKernelData->fUnitArray->at(4);
	double fLengthHie = fLength / fUnit;
	double fWidthHie = fWidth / fUnit;
	double fHeightHie = fHeight / fUnit;

	double fXHie = fX / fUnit;
	double fYHie = fY / fUnit;
	double fZHie = fZ / fUnit;

	std::string strTexturePath = m_pConfigData->strCorePath + m_strCoreGalaxyTexPath;
	m_galaxyTex = _CreateTexture2D(strTexturePath + "milkyWay_color.tga", 4);
	m_galaxyHeightTex = _CreateTexture2D(strTexturePath + "milkyWay_height.tga", 4);

	// 银河体渲染需要再加一张输出图，存储距离信息
	m_rayMarchCamera->attach(osg::Camera::COLOR_BUFFER2, m_distanceMap.get());

	// 正十二面体方法绘制银河系
	osg::Geometry* pDodecahedronFaceGeom = nullptr;
	osg::Geometry* pDodecahedronEdgeGeom = nullptr;
	osg::Geometry* pDodecahedronVertGeom = nullptr;

	//CreatePlatonicSolids(&pDodecahedronFaceGeom, &pDodecahedronEdgeGeom, &pDodecahedronVertGeom);
	CreateFootballSolids(&pDodecahedronFaceGeom, &pDodecahedronEdgeGeom, &pDodecahedronVertGeom);

	// make the transform of Dodecahedron :
	m_pDodecahedronTrans = new osg::MatrixTransform();
	m_pDodecahedronTrans->setMatrix(osg::Matrixd());
	m_rayMarchCamera->addChild(m_pDodecahedronTrans.get());

	SGMVolumeRange sVR;
	sVR.fXMin = fXHie - fLengthHie * 0.5;
	sVR.fXMax = fXHie + fLengthHie * 0.5;
	sVR.fYMin = fYHie - fWidthHie * 0.5;
	sVR.fYMax = fYHie + fWidthHie * 0.5;
	sVR.fZMin = fZHie - fHeightHie * 0.5;
	sVR.fZMax = fZHie + fHeightHie * 0.5;

	m_pDodecahedronFace = new osg::Geode();
	m_pDodecahedronFace->addDrawable(pDodecahedronFaceGeom);
	m_pDodecahedronTrans->addChild(m_pDodecahedronFace.get());
	m_pSsMilkyWayDecFace = new osg::StateSet();
	_InitMilkyWayStateSet(m_pSsMilkyWayDecFace.get(), sVR);
	m_pDodecahedronFace->setStateSet(m_pSsMilkyWayDecFace.get());

	m_pDodecahedronEdge = new osg::Geode();
	m_pDodecahedronEdge->addDrawable(pDodecahedronEdgeGeom);
	m_pDodecahedronTrans->addChild(m_pDodecahedronEdge.get());
	m_pSsMilkyWayDecEdge = new osg::StateSet();
	_InitMilkyWayStateSet(m_pSsMilkyWayDecEdge.get(), sVR);
	m_pSsMilkyWayDecEdge->setDefine("RAYS_2", osg::StateAttribute::ON);
	m_pDodecahedronEdge->setStateSet(m_pSsMilkyWayDecEdge.get());

	m_pDodecahedronVert = new osg::Geode();
	m_pDodecahedronVert->addDrawable(pDodecahedronVertGeom);
	m_pDodecahedronTrans->addChild(m_pDodecahedronVert.get());
	m_pSsMilkyWayDecVert = new osg::StateSet();
	_InitMilkyWayStateSet(m_pSsMilkyWayDecVert.get(), sVR);
	m_pSsMilkyWayDecVert->setDefine("RAYS_2", osg::StateAttribute::ON);
	m_pSsMilkyWayDecVert->setDefine("RAYS_3", osg::StateAttribute::ON);
	m_pDodecahedronVert->setStateSet(m_pSsMilkyWayDecVert.get());

	m_pGeodeMilkyWay = new osg::Geode;
	m_pGeodeMilkyWay->setNodeMask(~0);// 默认显示
	m_pGeodeMilkyWay->addDrawable(_MakeBoxGeometry(fLengthHie, fWidthHie, fHeightHie));
	osg::ref_ptr<osg::StateSet> pSSMilkyWay = m_pGeodeMilkyWay->getOrCreateStateSet();

	pSSMilkyWay->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSMilkyWay->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSMilkyWay->setAttributeAndModes(new osg::CullFace());
	pSSMilkyWay->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pSSMilkyWay->setRenderBinDetails(BIN_MILKYWAY, "DepthSortedBin");

	CGMKit::AddTexture(pSSMilkyWay.get(), m_rayMarchColorTex.get(), "colorTex", 0);
	CGMKit::AddTexture(pSSMilkyWay.get(), m_rayMarchAlphaTex.get(), "alphaTex", 1);
	pSSMilkyWay->addUniform(m_pCommonUniform->GetScreenSize());

	std::string strShader = m_pConfigData->strCorePath + m_strGalaxyShaderPath;
	std::string strVertPath = strShader + "Default.vert";
	std::string strFragPath = strShader + "Default.frag";
	CGMKit::LoadShader(pSSMilkyWay.get(), strVertPath, strFragPath);

	m_pVolumeRoot->addChild(m_pGeodeMilkyWay.get());
}

void CGMMilkyWay::ResizeScreen(const int width, const int height)
{
	int iW = std::ceil(0.5*width);
	int iH = std::ceil(0.5*height);
	if (m_distanceMap.valid()) //暂时用不到这张图
	{
		m_distanceMap->setTextureSize(iW, iH);
		m_distanceMap->dirtyTextureObject();
	}
	CGMVolumeBasic::ResizeScreen(width, height);
}

bool CGMMilkyWay::UpdateHierarchy(int iHieNew)
{
	if (EGMRENDER_LOWEST != m_pConfigData->eRenderQuality && m_rayMarchCamera.valid())
	{
		if (4 == iHieNew)
		{
			if (0 == m_rayMarchCamera->getNodeMask())
			{
				m_rayMarchCamera->setNodeMask(~0);
				m_pGeodeMilkyWay->setNodeMask(~0);
			}
		}
		else
		{
			if (0 != m_rayMarchCamera->getNodeMask())
			{
				m_rayMarchCamera->setNodeMask(0);
				m_pGeodeMilkyWay->setNodeMask(0);
			}
		}
	}

	return false;
}

osg::Geometry* CGMMilkyWay::_MakeBoxGeometry(
	const float fLength,
	const float fWidth,
	const float fHeight) const
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

bool CGMMilkyWay::_InitMilkyWayStateSet(osg::StateSet * pSS, const SGMVolumeRange& sVR)
{
	if (!pSS) return false;

	pSS->setMode(GL_BLEND, osg::StateAttribute::OFF);
	pSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSS->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT));

	pSS->addUniform(m_pCommonUniform->GetGalaxyAlpha());
	pSS->addUniform(m_pCommonUniform->GetScreenSize());
	pSS->addUniform(m_pCommonUniform->GetMainInvProjMatrix());

	osg::Vec3f vRangeMin = osg::Vec3f(sVR.fXMin, sVR.fYMin, sVR.fZMin);
	osg::ref_ptr<osg::Uniform> pRangeMin = new osg::Uniform("rangeMin", vRangeMin);
	pSS->addUniform(pRangeMin.get());
	osg::Vec3f vRangeMax = osg::Vec3f(sVR.fXMax, sVR.fYMax, sVR.fZMax);
	osg::ref_ptr<osg::Uniform> pRangeMax = new osg::Uniform("rangeMax", vRangeMax);
	pSS->addUniform(pRangeMax.get());

	int iUnit = 0;
	CGMKit::AddTexture(pSS, m_galaxyTex.get(), "galaxyTex", iUnit++);
	CGMKit::AddTexture(pSS, m_galaxyHeightTex.get(), "galaxyHeightTex", iUnit++);
	CGMKit::AddTexture(pSS, m_2DNoiseTex.get(), "noise2DTex", iUnit++);
	CGMKit::AddTexture(pSS, m_blueNoiseTex.get(), "blueNoiseTex", iUnit++);
	CGMKit::AddTexture(pSS, m_3DShapeTex.get(), "noiseShapeTex", iUnit++);
	CGMKit::AddTexture(pSS, m_3DErosionTex.get(), "noiseErosionTex", iUnit++);
	CGMKit::AddTexture(pSS, m_3DCurlTex.get(), "noiseCurlTex", iUnit++);

	std::string strShader = m_pConfigData->strCorePath + m_strGalaxyShaderPath;
	CGMKit::LoadShader(pSS, strShader + "MilkyWay.vert", strShader + "MilkyWay.frag");

	return true;
}