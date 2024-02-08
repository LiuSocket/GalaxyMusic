//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMEarthTail.cpp
/// @brief		Galaxy-Music Engine - GMEarthTail.cpp
/// @version	1.0
/// @author		LiuTao
/// @date		2023.08.06
//////////////////////////////////////////////////////////////////////////

#include "GMEarthTail.h"
#include "GMKit.h"
#include "GMEngine.h"

#include <osg/Depth>
#include <osg/BlendFunc>
#include <osg/CullFace>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/PositionAttitudeTransform>
#include <osgDB/ReadFile>

using namespace GM;

/*************************************************************************
CGMEarthTail Methods
*************************************************************************/

/** @brief 构造 */
CGMEarthTail::CGMEarthTail(): CGMVolumeBasic(),
	m_strCoreGalaxyTexPath("Textures/Galaxy/"),
	m_strEarthShaderPath("Shaders/EarthShader/"), m_strGalaxyShaderPath("Shaders/GalaxyShader/"),
	m_bVisible(false),
	m_fTailVisibleUniform(new osg::Uniform("tailVisible", 1.0f)),
	m_mWorld2TailMatUniform(new osg::Uniform("world2ModelMatrix", osg::Matrixf()))
{
}

/** @brief 析构 */
CGMEarthTail::~CGMEarthTail()
{
}

/** @brief 初始化 */
bool CGMEarthTail::Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform)
{
	CGMVolumeBasic::Init(pKernelData, pConfigData, pCommonUniform);

	if (EGMRENDER_LOW != pConfigData->eRenderQuality)
	{

	}

	return true;
}

/** @brief 更新 */
bool CGMEarthTail::Update(double dDeltaTime)
{
	if (EGMRENDER_LOW == m_pConfigData->eRenderQuality) return true;

	if (2 == m_pKernelData->iHierarchy)
	{
		double fDistance = GM_ENGINE.GetHierarchyTargetDistance();
		if (fDistance > 0.2)
		{
			// 隐藏节点
			if (0 != m_rayMarchCamera->getNodeMask())
			{
				m_rayMarchCamera->setNodeMask(0);
				m_TAACamera->setNodeMask(0);
				m_pTailTransform2->setNodeMask(0);
			}
		}
		else if(3 == GM_ENGINE.GetCelestialIndex())// 地球
		{
			// 显示节点
			if (0 == m_rayMarchCamera->getNodeMask())
			{
				m_rayMarchCamera->setNodeMask(~0);
				m_TAACamera->setNodeMask(~0);
				m_pTailTransform2->setNodeMask(~0);
			}
		}
		else{}
	}

	CGMVolumeBasic::Update(dDeltaTime);
	return true;
}

/** @brief 更新(在主相机更新姿态之后) */
bool CGMEarthTail::UpdateLater(double dDeltaTime)
{
	if (EGMRENDER_LOW != m_pConfigData->eRenderQuality &&
		m_rayMarchCamera.valid() && m_rayMarchCamera->getNodeMask() &&
		m_TAACamera.valid())
	{
		osg::Matrixd mMainViewMatrix = GM_View->getCamera()->getViewMatrix();
		osg::Matrixd mMainProjMatrix = GM_View->getCamera()->getProjectionMatrix();
		double fFovy, fAspectRatio, fZNear, fZFar;
		GM_View->getCamera()->getProjectionMatrixAsPerspective(fFovy, fAspectRatio, fZNear, fZFar);
		SetPixelLength(fFovy, m_iScreenHeight);

		// 设置 Dodecahedron 的位置
		osg::Matrixd mPos;
		osg::Matrixd mMainViewInverseMatrix = osg::Matrixd::inverse(mMainViewMatrix);
		mPos.setTrans(mMainViewInverseMatrix.getTrans());
		m_pDodecahedronTrans->setMatrix(mPos);
		m_rayMarchCamera->setViewMatrix(mMainViewMatrix);
		m_rayMarchCamera->setProjectionMatrix(mMainProjMatrix);

		int iHie = m_pKernelData->iHierarchy;
		if(2 == iHie)
		{
			double fDistance = GM_ENGINE.GetHierarchyTargetDistance();
			float fEarthTailAlpha = exp2(-fDistance*5.0);
			m_fTailVisibleUniform->set(fEarthTailAlpha);
		}
		else
		{
			m_fTailVisibleUniform->set(0.0f);
		}
	}

	CGMVolumeBasic::UpdateLater(dDeltaTime);
	return true;
}

/** @brief 加载 */
bool CGMEarthTail::Load()
{
	std::string strEarthShader = m_pConfigData->strCorePath + m_strEarthShaderPath;

	if (m_pSsTailDecFace.valid() && m_pSsTailDecEdge.valid() && m_pSsTailDecVert.valid())
	{
		std::string strVertPath = strEarthShader + "EarthTail.Vert";
		std::string strFragPath = strEarthShader + "EarthTail.Frag";
		CGMKit::LoadShader(m_pSsTailDecFace, strVertPath, strFragPath, "EarthTailFace");
		CGMKit::LoadShader(m_pSsTailDecEdge, strVertPath, strFragPath, "EarthTailEdge");
		CGMKit::LoadShader(m_pSsTailDecVert, strVertPath, strFragPath, "EarthTailVert");
	}
	if (m_statesetTAA.valid())
	{
		CGMKit::LoadShader(m_statesetTAA,
			m_pConfigData->strCorePath + m_strVolumeShaderPath + "TAAVert.glsl",
			m_pConfigData->strCorePath + m_strVolumeShaderPath + "TAAFrag.glsl",
			"TAA");
	}
	if (m_pTailEnvelopeGeode2.valid())
	{
		CGMKit::LoadShader(m_pTailEnvelopeGeode2->getStateSet(),
			strEarthShader + "TailEnvelope.vert",
			strEarthShader + "TailEnvelope.frag",
			"TailEnvelope");
	}

	return true;
}

void CGMEarthTail::MakeEarthTail()
{
	// create the ray marching texture
	int iWidth = 960;
	int iHeight = 540;

	m_vectorMap_0 = new osg::Texture2D;
	m_vectorMap_0->setName("vectorMap_0");
	m_vectorMap_0->setTextureSize(iWidth, iHeight);
	m_vectorMap_0->setInternalFormat(GL_RGB16F_ARB);
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
	m_vectorMap_1->setInternalFormat(GL_RGB16F_ARB);
	m_vectorMap_1->setSourceFormat(GL_RGB);
	m_vectorMap_1->setSourceType(GL_FLOAT);
	m_vectorMap_1->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_vectorMap_1->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_vectorMap_1->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_vectorMap_1->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_vectorMap_1->setDataVariance(osg::Object::DYNAMIC);
	m_vectorMap_1->setResizeNonPowerOfTwoHint(false);

	m_rayMarchTex = new osg::Texture2D;
	m_rayMarchTex->setName("EarthTailTex");
	m_rayMarchTex->setTextureSize(iWidth, iHeight);
	m_rayMarchTex->setInternalFormat(GL_RGBA8);
	m_rayMarchTex->setSourceFormat(GL_RGBA);
	m_rayMarchTex->setSourceType(GL_UNSIGNED_BYTE);
	m_rayMarchTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_rayMarchTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_rayMarchTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_rayMarchTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_rayMarchTex->setDataVariance(osg::Object::DYNAMIC);
	m_rayMarchTex->setResizeNonPowerOfTwoHint(false);

	// Create its camera and render to it
	m_rayMarchCamera = new osg::Camera;
	m_rayMarchCamera->setName("rayMarchCamera");
	m_rayMarchCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT);
	m_rayMarchCamera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	m_rayMarchCamera->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
	m_rayMarchCamera->setViewport(0, 0, iWidth, iHeight);
	m_rayMarchCamera->setRenderOrder(osg::Camera::PRE_RENDER, 1);
	m_rayMarchCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	m_rayMarchCamera->attach(osg::Camera::COLOR_BUFFER0, m_vectorMap_0);
	m_rayMarchCamera->attach(osg::Camera::COLOR_BUFFER1, m_rayMarchTex);
	m_rayMarchCamera->setAllowEventFocus(false);
	m_rayMarchCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

	// Raymarch交换buffer的回调函数指针
	SwitchFBOCallback* pRaymarchFBOCallback = new SwitchFBOCallback(m_vectorMap_1, m_vectorMap_0);
	m_rayMarchCamera->setPostDrawCallback(pRaymarchFBOCallback);

	GM_Root->addChild(m_rayMarchCamera);

	// 正十二面体方法绘制流浪地球尾迹
	osg::Geometry* pDodecahedronFaceGeom = nullptr;
	osg::Geometry* pDodecahedronEdgeGeom = nullptr;
	osg::Geometry* pDodecahedronVertGeom = nullptr;

	CreatePlatonicSolids(&pDodecahedronFaceGeom, &pDodecahedronEdgeGeom, &pDodecahedronVertGeom);

	// make the transform of Dodecahedron :
	m_pDodecahedronTrans = new osg::MatrixTransform();
	m_pDodecahedronTrans->setMatrix(osg::Matrixd());
	m_rayMarchCamera->addChild(m_pDodecahedronTrans);

	m_pDodecahedronFace = new osg::Geode();
	m_pDodecahedronFace->addDrawable(pDodecahedronFaceGeom);
	m_pDodecahedronTrans->addChild(m_pDodecahedronFace);
	m_pSsTailDecFace = new osg::StateSet();
	_InitEarthTailStateSet(m_pSsTailDecFace, "EarthTailFace");
	m_pDodecahedronFace->setStateSet(m_pSsTailDecFace);

	m_pDodecahedronEdge = new osg::Geode();
	m_pDodecahedronEdge->addDrawable(pDodecahedronEdgeGeom);
	m_pDodecahedronTrans->addChild(m_pDodecahedronEdge);
	m_pSsTailDecEdge = new osg::StateSet();
	_InitEarthTailStateSet(m_pSsTailDecEdge, "EarthTailEdge");
	m_pSsTailDecEdge->setDefine("RAYS_2", osg::StateAttribute::ON);
	m_pDodecahedronEdge->setStateSet(m_pSsTailDecEdge);

	m_pDodecahedronVert = new osg::Geode();
	m_pDodecahedronVert->addDrawable(pDodecahedronVertGeom);
	m_pDodecahedronTrans->addChild(m_pDodecahedronVert);
	m_pSsTailDecVert = new osg::StateSet();
	_InitEarthTailStateSet(m_pSsTailDecVert, "EarthTailVert");
	m_pSsTailDecVert->setDefine("RAYS_2", osg::StateAttribute::ON);
	m_pSsTailDecVert->setDefine("RAYS_3", osg::StateAttribute::ON);
	m_pDodecahedronVert->setStateSet(m_pSsTailDecVert);

	// get and mix the last frame by TAA(temporal anti-aliasing)
	// Add texture to TAA board,and active TAA
	ActiveTAA(m_rayMarchTex, m_vectorMap_0);

	m_pTailTransform2 = new osg::PositionAttitudeTransform();
	m_pTailTransform2->setNodeMask(0);
	GM_Root->addChild(m_pTailTransform2);

	// new stateSet for final EarthTail box
	osg::ref_ptr<osg::StateSet> pSSEarthTail = new osg::StateSet();
	pSSEarthTail->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSEarthTail->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSEarthTail->setAttributeAndModes(new osg::CullFace());
	pSSEarthTail->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pSSEarthTail->setRenderBinDetails(BIN_ATMOS_TAIL, "DepthSortedBin");
	CGMKit::AddTexture(pSSEarthTail, m_TAATex_0, "mainTex", 0);
	pSSEarthTail->addUniform(m_pCommonUniform->GetScreenSize());
	CGMKit::LoadShader(pSSEarthTail,
		m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Default.vert",
		m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Default.frag",
		"EarthTailBox2");

	// final sphere to show the TAA result
	double fUnit2 = m_pKernelData->fUnitArray->at(2);
	double fEarthTailRadius = 7.2e6; // 单位：米
	double fEarthTailLength = 4e7; // 单位：米
	double fRadiusHie2 = fEarthTailRadius / fUnit2;
	double fLengthHie2 = fEarthTailLength / fUnit2;

	m_pTailBoxGeode2 = new osg::Geode;
	m_pTailTransform2->addChild(m_pTailBoxGeode2);
	m_pTailBoxGeode2->addDrawable(_MakeTailBoxGeometry(fLengthHie2, fRadiusHie2));
	m_pTailBoxGeode2->setStateSet(pSSEarthTail);

	// 创建尾迹包络面
	osg::ref_ptr<osg::StateSet> pSSTailEnvelope = new osg::StateSet();
	pSSTailEnvelope->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSTailEnvelope->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSTailEnvelope->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false)); // no zbuffer
	pSSTailEnvelope->setAttributeAndModes(new osg::CullFace());
	pSSTailEnvelope->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pSSTailEnvelope->setRenderBinDetails(BIN_PLANET_TAIL, "DepthSortedBin");

	pSSTailEnvelope->addUniform(m_pCommonUniform->GetUnit());
	pSSTailEnvelope->addUniform(m_pCommonUniform->GetTime());
	pSSTailEnvelope->addUniform(m_vViewLightUniform);
	pSSTailEnvelope->addUniform(m_fEngineStartRatioUniform);

	osg::ref_ptr<osg::Texture2D> pNoise2DTex = new osg::Texture2D;
	pNoise2DTex->setImage(osgDB::readImageFile(m_pConfigData->strCorePath + "Textures/Volume/noise2D.dds"));
	pNoise2DTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	pNoise2DTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	pNoise2DTex->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	pNoise2DTex->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	pNoise2DTex->setSourceType(GL_UNSIGNED_BYTE);
	CGMKit::AddTexture(pSSTailEnvelope, pNoise2DTex, "noise2DTex", 0);

	CGMKit::LoadShader(pSSTailEnvelope,
		m_pConfigData->strCorePath + m_strEarthShaderPath + "TailEnvelope.vert",
		m_pConfigData->strCorePath + m_strEarthShaderPath + "TailEnvelope.frag",
		"TailEnvelope");

	m_pTailEnvelopeGeode2 = new osg::Geode;
	m_pTailEnvelopeGeode2->setStateSet(pSSTailEnvelope);
	m_pTailTransform2->addChild(m_pTailEnvelopeGeode2);
	m_pTailEnvelopeGeode2->addDrawable(_MakeTailEnvelopeGeometry(1.2e7 / fUnit2, fRadiusHie2));
	m_pTailEnvelopeGeode2->addDrawable(_MakeTailCylinderGeometry(1.5e8 / fUnit2, 1.7e6 / fUnit2));
	m_pTailEnvelopeGeode2->addDrawable(_MakeTailXGeometry(1.5e8 / fUnit2, 1.7e6 / fUnit2));
}

void CGMEarthTail::ResizeScreen(const int width, const int height)
{
	CGMVolumeBasic::ResizeScreen(width, height);
}

bool CGMEarthTail::UpdateHierarchy(int iHieNew)
{
	if (EGMRENDER_LOW != m_pConfigData->eRenderQuality && m_rayMarchCamera.valid())
	{
		if (2 == iHieNew)
		{
			if (m_bVisible && 0 == m_rayMarchCamera->getNodeMask())
			{
				m_rayMarchCamera->setNodeMask(~0);
				m_TAACamera->setNodeMask(~0);
				m_pTailTransform2->setNodeMask(~0);
			}
		}
		else
		{
			if (0 != m_rayMarchCamera->getNodeMask())
			{
				m_rayMarchCamera->setNodeMask(0);
				m_TAACamera->setNodeMask(0);
				m_pTailTransform2->setNodeMask(0);
			}
		}
	}

	return false;
}

void CGMEarthTail::SetVisible(const bool bVisible)
{
	m_bVisible = bVisible;
}

void CGMEarthTail::SetEarthTailRotate(const osg::Quat& qRotate)
{
	if (!m_pTailTransform2.valid()) return;

	m_pTailTransform2->asPositionAttitudeTransform()->setAttitude(qRotate);
	osg::Matrixf mWorld2TailMatrix = osg::Matrixd::inverse(osg::Matrixd(qRotate));
	m_mWorld2TailMatUniform->set(mWorld2TailMatrix);
}

void CGMEarthTail::SetUniform(osg::Uniform* pViewLight, osg::Uniform* pEngineStartRatio)
{
	m_vViewLightUniform = pViewLight;
	m_fEngineStartRatioUniform = pEngineStartRatio;
}

osg::Geometry* CGMEarthTail::_MakeTailBoxGeometry(const float fLength, const float fRadius) const
{
	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);

	osg::Vec3Array* verts = new osg::Vec3Array();
	verts->reserve(8);

	osg::Vec3Array* texCoords = new osg::Vec3Array();
	texCoords->reserve(8);

	osg::DrawElementsUShort* el = new osg::DrawElementsUShort(GL_TRIANGLES);
	el->reserve(36);

	verts->push_back(osg::Vec3(-fRadius, -fRadius, -fRadius)); // 0
	verts->push_back(osg::Vec3(fRadius, -fRadius, -fRadius)); // 1
	verts->push_back(osg::Vec3(fRadius, fRadius, -fRadius)); // 2
	verts->push_back(osg::Vec3(-fRadius, fRadius, -fRadius)); // 3
	verts->push_back(osg::Vec3(-fRadius, -fRadius, fLength)); // 4
	verts->push_back(osg::Vec3(fRadius, -fRadius, fLength)); // 5
	verts->push_back(osg::Vec3(fRadius, fRadius, fLength)); // 6
	verts->push_back(osg::Vec3(-fRadius, fRadius, fLength)); // 7

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

osg::Geometry* CGMEarthTail::_MakeTailEnvelopeGeometry(const float fLength, const float fRadius) const
{
	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);

	int iLonSegments = 48;
	int iLatSegments = 32;

	osg::Vec3Array* verts = new osg::Vec3Array();
	osg::Vec2Array* texCoords = new osg::Vec2Array();
	osg::Vec3Array* normals = new osg::Vec3Array();
	osg::DrawElementsUShort* el = new osg::DrawElementsUShort(GL_TRIANGLES);

	verts->reserve(iLatSegments * iLonSegments);
	texCoords->reserve(iLatSegments * iLonSegments);
	normals->reserve(iLatSegments * iLonSegments);
	el->reserve(iLatSegments * iLonSegments * 6);

	for (int y = 0; y <= iLatSegments; ++y)
	{
		float t = (float)y / (float)iLatSegments;
		for (int x = 0; x <= iLonSegments; ++x)
		{
			float s = (float)x / (float)iLonSegments;
			osg::Vec3 vPos = _GetTailEnvelopePos(osg::Vec2(s, t), fLength, fRadius);
			verts->push_back(vPos);
			texCoords->push_back(osg::Vec2(s*13, t));

			osg::Vec3 vOut = vPos;
			vOut.normalize();
			// 微分
			osg::Vec3 vPos_1 = _GetTailEnvelopePos(osg::Vec2(s, t + 1e-4), fLength, fRadius);
			osg::Vec3 vTang = vPos_1 - vPos;
			vTang.normalize();
			osg::Vec3 vBinormal = vTang ^ vOut;
			vBinormal.normalize();
			osg::Vec3 normal = vBinormal ^ vTang;
			normal.normalize();
			normals->push_back(normal);

			if ((y < iLatSegments) && (x < iLonSegments))
			{
				int x_plus_1 = x + 1;
				int y_plus_1 = y + 1;
				el->push_back(y * (iLonSegments + 1) + x);
				el->push_back(y * (iLonSegments + 1) + x_plus_1);
				el->push_back(y_plus_1 * (iLonSegments + 1) + x);
				el->push_back(y * (iLonSegments + 1) + x_plus_1);
				el->push_back(y_plus_1 * (iLonSegments + 1) + x_plus_1);
				el->push_back(y_plus_1 * (iLonSegments + 1) + x);
			}
		}
	}

	geom->setVertexArray(verts);
	geom->setTexCoordArray(0, texCoords);
	geom->setNormalArray(normals);
	geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	geom->addPrimitiveSet(el);
	return geom;
}

osg::Geometry* CGMEarthTail::_MakeTailCylinderGeometry(const float fLength, const float fRadius) const
{
	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);

	float fStartLenRatio = 0.05;
	int iLonSegments = 20;
	float lonSegmentSize = osg::PI * 2 / (float)iLonSegments;

	osg::Vec3Array* verts = new osg::Vec3Array();
	osg::Vec2Array* texCoords = new osg::Vec2Array();
	osg::Vec3Array* normals = new osg::Vec3Array();
	osg::DrawElementsUShort* el = new osg::DrawElementsUShort(GL_TRIANGLES);

	verts->reserve(2 * iLonSegments);
	texCoords->reserve(2 * iLonSegments);
	normals->reserve(2 * iLonSegments);
	el->reserve(iLonSegments * 6);

	for (int y = 0; y < 2; ++y)
	{
		for (int x = 0; x <= iLonSegments; ++x)
		{
			float lon = lonSegmentSize * x - osg::PI;
			float cosLon = cos(lon);
			float sinLon = sin(lon);
			float fX = fRadius * cosLon;
			float fY = fRadius * sinLon;
			float fZ = (fStartLenRatio + (1 - fStartLenRatio) * y) * fLength;
			osg::Vec3 vPos = osg::Vec3(fX, fY, fZ);
			verts->push_back(vPos);

			texCoords->push_back(osg::Vec2((float)x / (float)iLonSegments, 1 + y * 3));

			osg::Vec3 normal = osg::Vec3(fX, fY, 0);
			normal.normalize();
			normals->push_back(normal);

			if (x < iLonSegments)
			{
				int x_plus_1 = x + 1;
				int y_plus_1 = y + 1;
				el->push_back(y * (iLonSegments + 1) + x);
				el->push_back(y * (iLonSegments + 1) + x_plus_1);
				el->push_back(y_plus_1 * (iLonSegments + 1) + x);
				el->push_back(y * (iLonSegments + 1) + x_plus_1);
				el->push_back(y_plus_1 * (iLonSegments + 1) + x_plus_1);
				el->push_back(y_plus_1 * (iLonSegments + 1) + x);
			}
		}
	}

	geom->setVertexArray(verts);
	geom->setTexCoordArray(0, texCoords);
	geom->setNormalArray(normals);
	geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	geom->addPrimitiveSet(el);
	return geom;
}

osg::Geometry* CGMEarthTail::_MakeTailXGeometry(const float fLength, const float fRadius) const
{
	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);

	float fStartLenRatio = 0.05f;
	int iSegments = 32;
	int iDegreeSeg = 6;

	osg::Vec3Array* verts = new osg::Vec3Array();
	osg::Vec2Array* texCoords = new osg::Vec2Array();
	osg::Vec3Array* normals = new osg::Vec3Array();
	osg::DrawElementsUShort* el = new osg::DrawElementsUShort(GL_TRIANGLES);

	verts->reserve(2 * iDegreeSeg * (iSegments+1));
	texCoords->reserve(2 * iDegreeSeg * (iSegments + 1));
	normals->reserve(2 * iDegreeSeg * (iSegments + 1));
	el->reserve(6 * iDegreeSeg * iSegments);

	for (int y = 0; y <= iSegments; ++y)
	{
		for (int x = 0; x < iDegreeSeg; ++x)
		{
			float fR = fRadius * (1 + 2 * std::exp2(- y * 30.0 / iSegments));
			float fLon = (osg::PI * 2 * x) / iDegreeSeg;
			float fX = fR * cos(fLon);
			float fY = fR * sin(fLon);
			float fZ = fLength * (fStartLenRatio + (1 - fStartLenRatio) * float(y) / iSegments);
			verts->push_back(osg::Vec3(-fX, -fY, fZ));
			verts->push_back(osg::Vec3(fX, fY, fZ));

			float fCoordY = 4.0f + 2.0f * float(y) / iSegments;
			texCoords->push_back(osg::Vec2(2 * float(x) / iDegreeSeg, fCoordY));
			texCoords->push_back(osg::Vec2(2 * float(x + 1) / iDegreeSeg, fCoordY));

			osg::Vec3 normal = osg::Vec3(fY, -fX, 0);
			normal.normalize();
			normals->push_back(normal);
			normals->push_back(normal);

			if (y < iSegments)
			{
				int iVertPerSeg = 2 * iDegreeSeg;
				el->push_back(y * iVertPerSeg + x * 2);
				el->push_back(y * iVertPerSeg + x * 2 + 1);
				el->push_back((y + 1) * iVertPerSeg + x * 2);
				el->push_back(y * iVertPerSeg + x * 2 + 1);
				el->push_back((y + 1) * iVertPerSeg + x * 2 + 1);
				el->push_back((y + 1) * iVertPerSeg + x * 2);
			}
		}
	}

	geom->setVertexArray(verts);
	geom->setTexCoordArray(0, texCoords);
	geom->setNormalArray(normals);
	geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	geom->addPrimitiveSet(el);
	return geom;
}

bool CGMEarthTail::_InitEarthTailStateSet(osg::StateSet * pSS, const std::string strShaderName)
{
	if (!pSS) return false;

	pSS->setMode(GL_BLEND, osg::StateAttribute::OFF);
	pSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSS->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK));

	pSS->addUniform(m_fPixelLengthUniform);
	pSS->addUniform(m_fTailVisibleUniform);
	pSS->addUniform(m_vShakeVectorUniform);
	pSS->addUniform(m_vDeltaShakeUniform);
	pSS->addUniform(m_mWorld2TailMatUniform);
	pSS->addUniform(m_pCommonUniform->GetUnit());
	pSS->addUniform(m_pCommonUniform->GetTime());
	pSS->addUniform(m_pCommonUniform->GetScreenSize());
	pSS->addUniform(m_pCommonUniform->GetEyeFrontDir());
	pSS->addUniform(m_pCommonUniform->GetEyeRightDir());
	pSS->addUniform(m_pCommonUniform->GetEyeUpDir());
	pSS->addUniform(m_pCommonUniform->GetMainInvProjMatrix());
	pSS->addUniform(m_pCommonUniform->GetDeltaVPMatrix());
	pSS->addUniform(m_fEngineStartRatioUniform);

	int iUnit = 0;
	CGMKit::AddTexture(pSS, m_vectorMap_1, "lastVectorTex", iUnit++);
	CGMKit::AddTexture(pSS, m_blueNoiseTex, "blueNoiseTex", iUnit++);
	CGMKit::AddTexture(pSS, m_3DShapeTex, "noiseShapeTex", iUnit++);
	CGMKit::AddTexture(pSS, m_3DErosionTex, "noiseErosionTex", iUnit++);

	std::string strPath = m_pConfigData->strCorePath + m_strEarthShaderPath;
	CGMKit::LoadShader(pSS, strPath + "EarthTail.vert", strPath + "EarthTail.frag", strShaderName);

	return true;
}

osg::Vec3 CGMEarthTail::_GetTailEnvelopePos(const osg::Vec2 fCoordUV, const float fLength, const float fRadius) const
{
	float fL = fCoordUV.y() * 2.4 + 0.31;
	float fR = fRadius * (0.4 + sqrt((1 - exp(-fL)) * exp(-fL * fL)));
	float lon = (2 * fCoordUV.x() - 1) * osg::PI;
	float cosLon = cos(lon);
	float sinLon = sin(lon);
	float fX = fR * cosLon;
	float fY = fR * sinLon;
	float fZ = fLength * fCoordUV.y() - 0.1 * fRadius;
	return osg::Vec3(fX, fY, fZ);
}
