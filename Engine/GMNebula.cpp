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
#include "GMEngine.h"

#include <osg/BlendFunc>
#include <osg/CullFace>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osgDB/ReadFile>

using namespace GM;

/*************************************************************************
CGMNebula Methods
*************************************************************************/

/** @brief 构造 */
CGMNebula::CGMNebula(): CGMVolumeBasic(),
	m_strNebulaShaderPath("Shaders/NebulaShader/"), m_strCoreNebulaTexPath("Textures/Nebula/"),
	m_vWorldEyePos(osg::Vec3d(10.0, 0.0, 0.0)), m_dTimeLastFrame(0.0),
	m_fGalaxyAlphaUniform(new osg::Uniform("galaxyAlpha", 1.0f)),
	m_vStarHiePosUniform(new osg::Uniform("starWorldPos", osg::Vec3f(0.0f, 0.0f, 0.0f))),
	m_eVolumeState(EGM_VS_Galaxy),
	m_mDeltaVPMatrixUniform(new osg::Uniform("deltaViewProjMatrix", osg::Matrixf())),
	m_fOortVisibleUniform(new osg::Uniform("oortVisible", 1.0f))
{
}

/** @brief 析构 */
CGMNebula::~CGMNebula()
{
}

/** @brief 初始化 */
bool CGMNebula::Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform)
{
	CGMVolumeBasic::Init(pKernelData, pConfigData, pCommonUniform);

	m_3DShapeTex = _Load3DShapeNoise();
	m_3DErosionTex = _Load3DErosionNoise();
	m_2DNoiseTex = _CreateTexture2D(m_pConfigData->strCorePath + m_strCoreNebulaTexPath + "2DNoise.tga", 4);

	if (EGMRENDER_LOW != pConfigData->eRenderQuality)
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

		m_pSsOortDecFace = new osg::StateSet();
		m_pSsOortDecEdge = new osg::StateSet();
		m_pSsOortDecVert = new osg::StateSet();
		_InitOortStateSet(m_pSsOortDecFace.get(), "OortFace");
		_InitOortStateSet(m_pSsOortDecEdge.get(), "OortEdge");
		_InitOortStateSet(m_pSsOortDecVert.get(), "OortVert");
		m_pSsOortDecEdge->setDefine("RAYS_2", osg::StateAttribute::ON);
		m_pSsOortDecVert->setDefine("RAYS_2", osg::StateAttribute::ON);
		m_pSsOortDecVert->setDefine("RAYS_3", osg::StateAttribute::ON);
	}

	return true;
}

/** @brief 更新 */
bool CGMNebula::Update(double dDeltaTime)
{
	if (EGMRENDER_LOW == m_pConfigData->eRenderQuality) return true;

	if (4 == m_pKernelData->iHierarchy)
	{
		double fDistance = GM_ENGINE.GetHierarchyTargetDistance();
		if (fDistance > 0.01)
		{
			if (EGM_VS_Galaxy != m_eVolumeState)
			{
				m_eVolumeState = EGM_VS_Galaxy;
				m_pDodecahedronFace->setStateSet(m_pSsMilkyWayDecFace.get());
				m_pDodecahedronEdge->setStateSet(m_pSsMilkyWayDecEdge.get());
				m_pDodecahedronVert->setStateSet(m_pSsMilkyWayDecVert.get());
			}
		}
		else
		{
			if (EGM_VS_Oort != m_eVolumeState)
			{
				m_eVolumeState = EGM_VS_Oort;
				m_pDodecahedronFace->setStateSet(m_pSsOortDecFace.get());
				m_pDodecahedronEdge->setStateSet(m_pSsOortDecEdge.get());
				m_pDodecahedronVert->setStateSet(m_pSsOortDecVert.get());
			}
		}
	}

	CGMVolumeBasic::Update(dDeltaTime);
	return true;
}

/** @brief 更新(在主相机更新姿态之后) */
bool CGMNebula::UpdateLater(double dDeltaTime)
{
	if (EGMRENDER_LOW != m_pConfigData->eRenderQuality &&
		m_rayMarchCamera.valid() && m_rayMarchCamera->getNodeMask() &&
		m_cameraTAA.valid())
	{
		// 修改m_rayMarchCamera的投影矩阵，实现抖动，使最终渲染的分辨率提高四倍
		osg::Matrixd mMainViewMatrix = GM_View->getCamera()->getViewMatrix();
		osg::Matrixd mMainProjMatrix = GM_View->getCamera()->getProjectionMatrix();
		double fFovy, fAspectRatio, fZNear, fZFar;
		GM_View->getCamera()->getProjectionMatrixAsPerspective(fFovy, fAspectRatio, fZNear, fZFar);

		osg::Vec3d vWorldEyeCenter, vWorldEyeUpDir;
		mMainViewMatrix.getLookAt(m_vWorldEyePos, vWorldEyeCenter, vWorldEyeUpDir);
		// 相机正前方单位向量（世界空间）
		osg::Vec3d vWorldEyeFrontDir = vWorldEyeCenter - m_vWorldEyePos;
		vWorldEyeFrontDir.normalize();
		osg::Vec3d vWorldEyeRightDir = vWorldEyeFrontDir ^ vWorldEyeUpDir;
		vWorldEyeRightDir.normalize();

		m_vEyeUpDirUniform->set(osg::Vec3f(vWorldEyeUpDir));		// 相机上方向在世界空间下的单位向量
		m_vEyeRightDirUniform->set(osg::Vec3f(vWorldEyeRightDir));	// 相机右方向在世界空间下的单位向量
		m_vEyeFrontDirUniform->set(osg::Vec3f(vWorldEyeFrontDir));	// 相机前方向在世界空间下的单位向量

		// 设置 Dodecahedron 的位置
		osg::Matrixd mPos;
		osg::Matrixd mMainViewInverseMatrix = osg::Matrixd::inverse(mMainViewMatrix);
		mPos.setTrans(mMainViewInverseMatrix.getTrans());
		m_pDodecahedronTrans->setMatrix(mPos);

		osg::Matrixd VP = mMainViewMatrix*mMainProjMatrix;
		SetPixelLength(fFovy, m_iScreenHeight);

		m_rayMarchCamera->setViewMatrix(mMainViewMatrix);
		m_rayMarchCamera->setProjectionMatrix(mMainProjMatrix);

		osg::Matrixf mInvProjMatrix = osg::Matrixd::inverse(mMainProjMatrix);
		m_mMainInvProjUniform->set(mInvProjMatrix);

		int iHie = m_pKernelData->iHierarchy;
		if (4 == iHie)
		{
			m_fOortVisibleUniform->set(1.0f);
		}
		else if(3 == iHie)
		{
			double fDistance = GM_ENGINE.GetHierarchyTargetDistance();
			float fOortAlpha = 1.0 - exp2(-fDistance*10.0);
			m_fOortVisibleUniform->set(fOortAlpha);
		}
		else
		{
			m_fOortVisibleUniform->set(0.0f);
		}


		if (m_mDeltaVPMatrixUniform.valid())
		{
			// 修改差值矩阵
			osg::Matrixf deltaVP = osg::Matrixf(
				VP(0, 0) - m_mLastVP(0, 0), VP(0, 1) - m_mLastVP(0, 1), VP(0, 2) - m_mLastVP(0, 2), VP(0, 3) - m_mLastVP(0, 3),
				VP(1, 0) - m_mLastVP(1, 0), VP(1, 1) - m_mLastVP(1, 1), VP(1, 2) - m_mLastVP(1, 2), VP(1, 3) - m_mLastVP(1, 3),
				VP(2, 0) - m_mLastVP(2, 0), VP(2, 1) - m_mLastVP(2, 1), VP(2, 2) - m_mLastVP(2, 2), VP(2, 3) - m_mLastVP(2, 3),
				VP(3, 0) - m_mLastVP(3, 0), VP(3, 1) - m_mLastVP(3, 1), VP(3, 2) - m_mLastVP(3, 2), VP(3, 3) - m_mLastVP(3, 3)
			);
			m_mDeltaVPMatrixUniform->set(deltaVP);
		}

		m_mLastVP = VP;
	}

	CGMVolumeBasic::UpdateLater(dDeltaTime);
	return true;
}

/** @brief 加载 */
bool CGMNebula::Load()
{
	std::string strNebulaShader = m_pConfigData->strCorePath + m_strNebulaShaderPath;

	if (m_pSsMilkyWayDecFace.valid() && m_pSsMilkyWayDecEdge.valid() && m_pSsMilkyWayDecVert.valid())
	{
		std::string strVertPath = strNebulaShader + "NebulaVert.glsl";
		std::string strFragPath = strNebulaShader + "NebulaFrag.glsl";
		CGMKit::LoadShader(m_pSsMilkyWayDecFace.get(), strVertPath, strFragPath, "NebulaFace");
		CGMKit::LoadShader(m_pSsMilkyWayDecEdge.get(), strVertPath, strFragPath, "NebulaEdge");
		CGMKit::LoadShader(m_pSsMilkyWayDecVert.get(), strVertPath, strFragPath, "NebulaVert");
	}
	if (m_pSsOortDecFace.valid() && m_pSsOortDecEdge.valid() && m_pSsOortDecVert.valid())
	{
		std::string strVertPath = strNebulaShader + "Oort.Vert";
		std::string strFragPath = strNebulaShader + "Oort.Frag";
		CGMKit::LoadShader(m_pSsOortDecFace.get(), strVertPath, strFragPath, "OortFace");
		CGMKit::LoadShader(m_pSsOortDecEdge.get(), strVertPath, strFragPath, "OortEdge");
		CGMKit::LoadShader(m_pSsOortDecVert.get(), strVertPath, strFragPath, "OortVert");
	}
	if (m_statesetTAA.valid())
	{
		std::string strTAAVertPath = m_pConfigData->strCorePath + m_strShaderPath + "TAAVert.glsl";
		std::string strTAAFragPath = m_pConfigData->strCorePath + m_strShaderPath + "TAAFrag.glsl";
		CGMKit::LoadShader(m_statesetTAA.get(), strTAAVertPath, strTAAFragPath, "TAA");
	}
	return true;
}

// make Nebula for the galaxy
void CGMNebula::MakeMilkyWay(double fLength, double fWidth, double fHeight, double fX, double fY, double fZ)
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

	m_nebulaTex = new osg::Texture2D;
	m_nebulaTex->setName("nebulaTex");
	m_nebulaTex->setTextureSize(iWidth, iHeight);
	m_nebulaTex->setInternalFormat(GL_RGBA8);
	m_nebulaTex->setSourceFormat(GL_RGBA);
	m_nebulaTex->setSourceType(GL_UNSIGNED_BYTE);
	m_nebulaTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_nebulaTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_nebulaTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_nebulaTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_nebulaTex->setDataVariance(osg::Object::DYNAMIC);
	m_nebulaTex->setResizeNonPowerOfTwoHint(false);

	// Create its camera and render to it
	m_rayMarchCamera = new osg::Camera;
	m_rayMarchCamera->setName("rayMarchCamera");
	m_rayMarchCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT);
	m_rayMarchCamera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	m_rayMarchCamera->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
	m_rayMarchCamera->setViewport(0, 0, iWidth, iHeight);
	m_rayMarchCamera->setRenderOrder(osg::Camera::PRE_RENDER, 1);
	m_rayMarchCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	m_rayMarchCamera->attach(osg::Camera::COLOR_BUFFER0, m_vectorMap_0.get());
	m_rayMarchCamera->attach(osg::Camera::COLOR_BUFFER1, m_nebulaTex.get());
	m_rayMarchCamera->attach(osg::Camera::COLOR_BUFFER2, m_distanceMap.get());
	m_rayMarchCamera->setAllowEventFocus(false);
	m_rayMarchCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

	// Raymarch交换buffer的回调函数指针
	SwitchFBOCallback* pRaymarchFBOCallback = new SwitchFBOCallback(m_vectorMap_1.get(), m_vectorMap_0.get());
	m_rayMarchCamera->setPostDrawCallback(pRaymarchFBOCallback);

	GM_Root->addChild(m_rayMarchCamera.get());

	// 正十二面体方法绘制的星云
	osg::Geometry* pDodecahedronFaceGeom = nullptr;
	osg::Geometry* pDodecahedronEdgeGeom = nullptr;
	osg::Geometry* pDodecahedronVertGeom = nullptr;

	CreatePlatonicSolids(&pDodecahedronFaceGeom, &pDodecahedronEdgeGeom, &pDodecahedronVertGeom);

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
	_InitMilkyWayStateSet(m_pSsMilkyWayDecFace.get(), sVR, "NebulaFace");
	m_pDodecahedronFace->setStateSet(m_pSsMilkyWayDecFace.get());

	m_pDodecahedronEdge = new osg::Geode();
	m_pDodecahedronEdge->addDrawable(pDodecahedronEdgeGeom);
	m_pDodecahedronTrans->addChild(m_pDodecahedronEdge.get());
	m_pSsMilkyWayDecEdge = new osg::StateSet();
	_InitMilkyWayStateSet(m_pSsMilkyWayDecEdge.get(), sVR, "NebulaEdge");
	m_pSsMilkyWayDecEdge->setDefine("RAYS_2", osg::StateAttribute::ON);
	m_pDodecahedronEdge->setStateSet(m_pSsMilkyWayDecEdge.get());

	m_pDodecahedronVert = new osg::Geode();
	m_pDodecahedronVert->addDrawable(pDodecahedronVertGeom);
	m_pDodecahedronTrans->addChild(m_pDodecahedronVert.get());
	m_pSsMilkyWayDecVert = new osg::StateSet();
	_InitMilkyWayStateSet(m_pSsMilkyWayDecVert.get(), sVR, "NebulaVert");
	m_pSsMilkyWayDecVert->setDefine("RAYS_2", osg::StateAttribute::ON);
	m_pSsMilkyWayDecVert->setDefine("RAYS_3", osg::StateAttribute::ON);
	m_pDodecahedronVert->setStateSet(m_pSsMilkyWayDecVert.get());

	// get and mix the last frame by TAA(temporal anti-aliasing)
	// Add texture to TAA board,and active TAA
	ActiveTAA(m_nebulaTex.get(), m_vectorMap_0.get());
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
	int iW = std::ceil(0.5*width);
	int iH = std::ceil(0.5*height);
	if (m_rayMarchCamera.valid())
	{
		m_rayMarchCamera->resize(iW, iH);

		m_vectorMap_0->setTextureSize(iW, iH);
		m_vectorMap_0->dirtyTextureObject();
		m_vectorMap_1->setTextureSize(iW, iH);
		m_vectorMap_1->dirtyTextureObject();

		m_nebulaTex->setTextureSize(iW, iH);
		m_nebulaTex->dirtyTextureObject();

		m_distanceMap->setTextureSize(iW, iH);
		m_distanceMap->dirtyTextureObject();
	}
	CGMVolumeBasic::ResizeScreen(width, height);
}

bool CGMNebula::UpdateHierarchy(int iHieNew)
{
	if (EGMRENDER_LOW != m_pConfigData->eRenderQuality && m_rayMarchCamera.valid())
	{
		if (4 == iHieNew || 3 == iHieNew)
		{
			if (0 == m_rayMarchCamera->getNodeMask())
				m_rayMarchCamera->setNodeMask(~0);

			if (3 == iHieNew)
			{
				m_pSsOortDecFace->setDefine("LOW_HIERARCHY", osg::StateAttribute::ON);
				m_pSsOortDecEdge->setDefine("LOW_HIERARCHY", osg::StateAttribute::ON);
				m_pSsOortDecVert->setDefine("LOW_HIERARCHY", osg::StateAttribute::ON);
			}
			else
			{
				m_pSsOortDecFace->setDefine("LOW_HIERARCHY", osg::StateAttribute::OFF);
				m_pSsOortDecEdge->setDefine("LOW_HIERARCHY", osg::StateAttribute::OFF);
				m_pSsOortDecVert->setDefine("LOW_HIERARCHY", osg::StateAttribute::OFF);
			}
		}
		else
		{
			if (0 != m_rayMarchCamera->getNodeMask())
				m_rayMarchCamera->setNodeMask(0);
		}
	}

	return false;
}

osg::Geometry* CGMNebula::MakeSphereGeometry(const float fRadius, const int iLatSegment) const
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

osg::Geometry* CGMNebula::MakeBoxGeometry(
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

osg::Texture* CGMNebula::_Load3DShapeNoise() const
{
	std::string strTexturePath = m_pConfigData->strCorePath + m_strCoreTexturePath + "noiseShape.raw";
	osg::ref_ptr<osg::Image> shapeImg = osgDB::readImageFile(strTexturePath);
	shapeImg->setImage(128, 128, 128, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, shapeImg->data(), osg::Image::NO_DELETE);
	osg::Texture3D* tex3d = new osg::Texture3D;
	tex3d->setImage(shapeImg.get());
	tex3d->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	tex3d->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	tex3d->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	tex3d->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	tex3d->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
	tex3d->setInternalFormat(GL_RGBA8);
	tex3d->setSourceFormat(GL_RGBA);
	tex3d->setSourceType(GL_UNSIGNED_BYTE);
	tex3d->allocateMipmapLevels();
	return tex3d;
}

osg::Texture* CGMNebula::_Load3DErosionNoise() const
{
	std::string strTexturePath = m_pConfigData->strCorePath + m_strCoreTexturePath + "noiseErosion.raw";
	osg::Image* img = osgDB::readImageFile(strTexturePath);
	img->setImage(32, 32, 32, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, img->data(), osg::Image::NO_DELETE);
	osg::Texture3D* tex3d = new osg::Texture3D;
	tex3d->setImage(img);
	tex3d->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	tex3d->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	tex3d->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	tex3d->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	tex3d->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
	tex3d->setInternalFormat(GL_RGB8);
	tex3d->setSourceFormat(GL_RGB);
	tex3d->setSourceType(GL_UNSIGNED_BYTE);
	tex3d->allocateMipmapLevels();
	return tex3d;
}

osg::Texture* CGMNebula::_Load3DCurlNoise() const
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

bool CGMNebula::_InitMilkyWayStateSet(osg::StateSet * pSS, const SGMVolumeRange& sVR, const std::string strShaderName)
{
	if (!pSS) return false;

	pSS->setMode(GL_BLEND, osg::StateAttribute::OFF);
	pSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSS->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK));

	pSS->addUniform(m_fGalaxyAlphaUniform.get());
	pSS->addUniform(m_fPixelLengthUniform.get());
	pSS->addUniform(m_vShakeVectorUniform.get());
	pSS->addUniform(m_vDeltaShakeUniform.get());
	pSS->addUniform(m_pCommonUniform->GetScreenSize());
	pSS->addUniform(m_vEyeFrontDirUniform.get());
	pSS->addUniform(m_vEyeRightDirUniform.get());
	pSS->addUniform(m_vEyeUpDirUniform.get());
	pSS->addUniform(m_vNoiseUniform.get());
	pSS->addUniform(m_mMainInvProjUniform.get());
	pSS->addUniform(m_mDeltaVPMatrixUniform.get());

	osg::Vec3f vRangeMin = osg::Vec3f(sVR.fXMin, sVR.fYMin, sVR.fZMin);
	osg::ref_ptr<osg::Uniform> pRangeMin = new osg::Uniform("rangeMin", vRangeMin);
	pSS->addUniform(pRangeMin.get());
	osg::Vec3f vRangeMax = osg::Vec3f(sVR.fXMax, sVR.fYMax, sVR.fZMax);
	osg::ref_ptr<osg::Uniform> pRangeMax = new osg::Uniform("rangeMax", vRangeMax);
	pSS->addUniform(pRangeMax.get());

	int iUnit = 0;
	CGMKit::AddTexture(pSS, m_vectorMap_1.get(), "lastVectorTex", iUnit++);
	CGMKit::AddTexture(pSS, m_galaxyTex.get(), "galaxyTex", iUnit++);
	CGMKit::AddTexture(pSS, m_galaxyHeightTex.get(), "galaxyHeightTex", iUnit++);
	CGMKit::AddTexture(pSS, m_2DNoiseTex.get(), "noise2DTex", iUnit++);
	CGMKit::AddTexture(pSS, m_blueNoiseTex.get(), "blueNoiseTex", iUnit++);
	CGMKit::AddTexture(pSS, m_3DShapeTex.get(), "noiseShapeTex", iUnit++);
	CGMKit::AddTexture(pSS, m_3DErosionTex.get(), "noiseErosionTex", iUnit++);
	CGMKit::AddTexture(pSS, _Load3DCurlNoise(), "noiseCurlTex", iUnit++);

	std::string strNebulaVertPath = m_pConfigData->strCorePath + m_strNebulaShaderPath + "NebulaVert.glsl";
	std::string strNebulaFragPath = m_pConfigData->strCorePath + m_strNebulaShaderPath + "NebulaFrag.glsl";
	CGMKit::LoadShader(pSS, strNebulaVertPath, strNebulaFragPath, strShaderName);

	return true;
}

bool CGMNebula::_InitOortStateSet(osg::StateSet * pSS, const std::string strShaderName)
{
	if (!pSS) return false;

	pSS->setMode(GL_BLEND, osg::StateAttribute::OFF);
	pSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSS->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK));

	pSS->addUniform(m_pCommonUniform->GetLevelArray());
	pSS->addUniform(m_pCommonUniform->GetTime());
	pSS->addUniform(m_fPixelLengthUniform.get());
	pSS->addUniform(m_fOortVisibleUniform.get());
	pSS->addUniform(m_vShakeVectorUniform.get());
	pSS->addUniform(m_vDeltaShakeUniform.get());
	pSS->addUniform(m_pCommonUniform->GetScreenSize());
	pSS->addUniform(m_vEyeFrontDirUniform.get());
	pSS->addUniform(m_vEyeRightDirUniform.get());
	pSS->addUniform(m_vEyeUpDirUniform.get());
	pSS->addUniform(m_vStarHiePosUniform.get());
	pSS->addUniform(m_pCommonUniform->GetStarColor());
	pSS->addUniform(m_mMainInvProjUniform.get());
	pSS->addUniform(m_mDeltaVPMatrixUniform.get());

	int iUnit = 0;
	CGMKit::AddTexture(pSS, m_vectorMap_1.get(), "lastVectorTex", iUnit++);
	CGMKit::AddTexture(pSS, m_blueNoiseTex.get(), "blueNoiseTex", iUnit++);
	CGMKit::AddTexture(pSS, m_3DShapeTex.get(), "noiseShapeTex", iUnit++);
	CGMKit::AddTexture(pSS, m_3DErosionTex.get(), "noiseErosionTex", iUnit++);

	std::string strNebulaVertPath = m_pConfigData->strCorePath + m_strNebulaShaderPath + "Oort.vert";
	std::string strNebulaFragPath = m_pConfigData->strCorePath + m_strNebulaShaderPath + "Oort.frag";
	CGMKit::LoadShader(pSS, strNebulaVertPath, strNebulaFragPath, strShaderName);

	return true;
}
