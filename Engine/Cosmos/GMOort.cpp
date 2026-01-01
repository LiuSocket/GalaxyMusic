//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMOort.cpp
/// @brief		Galaxy-Music Engine - GMOort.cpp
/// @version	1.0
/// @author		LiuTao
/// @date		2023.08.06
//////////////////////////////////////////////////////////////////////////

#include "GMOort.h"
#include "../GMKit.h"
#include "../GMEngine.h"

#include <osg/BlendFunc>
#include <osg/CullFace>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/PositionAttitudeTransform>
#include <osgDB/ReadFile>

using namespace GM;

/*************************************************************************
CGMOort Methods
*************************************************************************/

/** @brief 构造 */
CGMOort::CGMOort(): CGMVolumeBasic(),
	m_strCoreGalaxyTexPath("Textures/Galaxy/"), m_strGalaxyShaderPath("Shaders/GalaxyShader/"),
	m_fOortVisibleUniform(new osg::Uniform("oortVisible", 1.0f)),
	m_fUnitUniform(new osg::Uniform("unit", 1e-5f)),
	m_mAttitudeUniform(new osg::Uniform("attitudeMatrix", osg::Matrixf()))
{
}

/** @brief 析构 */
CGMOort::~CGMOort()
{
}

/** @brief 初始化 */
bool CGMOort::Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform)
{
	CGMVolumeBasic::Init(pKernelData, pConfigData, pCommonUniform);

	if (EGMRENDER_LOWEST != pConfigData->eRenderQuality)
	{

	}

	return true;
}

/** @brief 更新 */
bool CGMOort::Update(double dDeltaTime)
{
	if (EGMRENDER_LOWEST == m_pConfigData->eRenderQuality) return true;

	if (4 == m_pKernelData->iHierarchy)
	{
		double fDistance = GM_ENGINE.GetHierarchyTargetDistance();
		if (fDistance > 0.01)
		{
			// 隐藏节点
			if (0 != m_rayMarchCamera->getNodeMask())
			{
				m_rayMarchCamera->setNodeMask(0);
				m_pOortTransform4->setNodeMask(0);
			}
		}
		else
		{
			// 显示节点
			if (0 == m_rayMarchCamera->getNodeMask())
			{
				m_rayMarchCamera->setNodeMask(~0);
				m_pOortTransform4->setNodeMask(~0);
			}
		}
	}
	else if (3 == m_pKernelData->iHierarchy)
	{
		double fDistance = GM_ENGINE.GetHierarchyTargetDistance();
		if (fDistance < 0.01)
		{
			// 隐藏节点
			if (0 != m_rayMarchCamera->getNodeMask())
			{
				m_rayMarchCamera->setNodeMask(0);
				m_pGeodeOort3->setNodeMask(0);
			}
		}
		else
		{
			// 显示节点
			if (0 == m_rayMarchCamera->getNodeMask())
			{
				m_rayMarchCamera->setNodeMask(~0);
				m_pGeodeOort3->setNodeMask(~0);
			}
		}
	}
	else{}

	CGMVolumeBasic::Update(dDeltaTime);
	return true;
}

/** @brief 更新(在主相机更新姿态之后) */
bool CGMOort::UpdateLater(double dDeltaTime)
{
	if (EGMRENDER_LOWEST != m_pConfigData->eRenderQuality &&
		m_rayMarchCamera.valid() && m_rayMarchCamera->getNodeMask())
	{
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
	}

	CGMVolumeBasic::UpdateLater(dDeltaTime);
	return true;
}

/** @brief 加载 */
bool CGMOort::Load()
{
	std::string strShader = m_pConfigData->strCorePath + m_strGalaxyShaderPath;

	if (m_pSsOortDecFace.valid() && m_pSsOortDecEdge.valid() && m_pSsOortDecVert.valid())
	{
		std::string strVertPath = strShader + "Oort.Vert";
		std::string strFragPath = strShader + "Oort.Frag";
		CGMKit::LoadShader(m_pSsOortDecFace.get(), strVertPath, strFragPath,true);
		CGMKit::LoadShader(m_pSsOortDecEdge.get(), strVertPath, strFragPath);
		CGMKit::LoadShader(m_pSsOortDecVert.get(), strVertPath, strFragPath);
	}
	return true;
}

void CGMOort::MakeOort()
{
	double fOortRadius = 1e16; // 单位：米
	double fRadiusHie3 = fOortRadius / m_pKernelData->fUnitArray->at(3);
	double fRadiusHie4 = fOortRadius / m_pKernelData->fUnitArray->at(4);

	std::string strTexturePath = m_pConfigData->strCorePath + m_strCoreGalaxyTexPath;
	m_galaxyTex = _CreateTexture2D(strTexturePath + "milkyWay_color.tga", 4);
	m_galaxyHeightTex = _CreateTexture2D(strTexturePath + "milkyWay_height.tga", 4);

	// 正十二面体方法绘制奥尔特星云
	osg::Geometry* pDodecahedronFaceGeom = nullptr;
	osg::Geometry* pDodecahedronEdgeGeom = nullptr;
	osg::Geometry* pDodecahedronVertGeom = nullptr;

	CreatePlatonicSolids(&pDodecahedronFaceGeom, &pDodecahedronEdgeGeom, &pDodecahedronVertGeom);

	// make the transform of Dodecahedron :
	m_pDodecahedronTrans = new osg::MatrixTransform();
	m_pDodecahedronTrans->setMatrix(osg::Matrixd());
	m_rayMarchCamera->addChild(m_pDodecahedronTrans.get());

	m_pDodecahedronFace = new osg::Geode();
	m_pDodecahedronFace->addDrawable(pDodecahedronFaceGeom);
	m_pDodecahedronTrans->addChild(m_pDodecahedronFace.get());
	m_pSsOortDecFace = new osg::StateSet();
	_InitOortStateSet(m_pSsOortDecFace.get());
	m_pDodecahedronFace->setStateSet(m_pSsOortDecFace.get());

	m_pDodecahedronEdge = new osg::Geode();
	m_pDodecahedronEdge->addDrawable(pDodecahedronEdgeGeom);
	m_pDodecahedronTrans->addChild(m_pDodecahedronEdge.get());
	m_pSsOortDecEdge = new osg::StateSet();
	_InitOortStateSet(m_pSsOortDecEdge.get());
	m_pSsOortDecEdge->setDefine("RAYS_2", osg::StateAttribute::ON);
	m_pDodecahedronEdge->setStateSet(m_pSsOortDecEdge.get());

	m_pDodecahedronVert = new osg::Geode();
	m_pDodecahedronVert->addDrawable(pDodecahedronVertGeom);
	m_pDodecahedronTrans->addChild(m_pDodecahedronVert.get());
	m_pSsOortDecVert = new osg::StateSet();
	_InitOortStateSet(m_pSsOortDecVert.get());
	m_pSsOortDecVert->setDefine("RAYS_2", osg::StateAttribute::ON);
	m_pSsOortDecVert->setDefine("RAYS_3", osg::StateAttribute::ON);
	m_pDodecahedronVert->setStateSet(m_pSsOortDecVert.get());

	// new stateSet for final Oort sphere
	osg::ref_ptr<osg::StateSet> pSSOort = new osg::StateSet();
	pSSOort->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSOort->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSOort->setAttributeAndModes(new osg::CullFace());
	pSSOort->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pSSOort->setRenderBinDetails(BIN_MILKYWAY, "DepthSortedBin"); // to do
	CGMKit::AddTexture(pSSOort, m_rayMarchColorTex, "colorTex", 0);
	CGMKit::AddTexture(pSSOort, m_rayMarchAlphaTex, "alphaTex", 1);
	pSSOort->addUniform(m_pCommonUniform->GetScreenSize());
	std::string strShader = m_pConfigData->strCorePath + m_strGalaxyShaderPath;
	CGMKit::LoadShader(pSSOort.get(), strShader + "Default.vert", strShader + "Default.frag");

	// final sphere to show the TAA result
	m_pGeodeOort3 = new osg::Geode;
	m_pGeodeOort3->setNodeMask(0);
	m_pGeodeOort3->addDrawable(_MakeSphereGeometry(fRadiusHie3));
	m_pGeodeOort3->setStateSet(pSSOort.get());
	m_pVolumeRoot->addChild(m_pGeodeOort3.get());

	m_pOortTransform4 = new osg::PositionAttitudeTransform();
	osg::ref_ptr<osg::Geode> pGeodeOort4 = new osg::Geode;
	m_pOortTransform4->addChild(pGeodeOort4.get());
	pGeodeOort4->addDrawable(_MakeSphereGeometry(fRadiusHie4));
	m_pOortTransform4->setStateSet(pSSOort.get());
	m_pVolumeRoot->addChild(m_pOortTransform4.get());
}

void CGMOort::ResizeScreen(const int width, const int height)
{
	CGMVolumeBasic::ResizeScreen(width, height);
}

bool CGMOort::UpdateHierarchy(int iHieNew)
{
	if (EGMRENDER_LOWEST != m_pConfigData->eRenderQuality && m_rayMarchCamera.valid())
	{
		if (4 == iHieNew || 3 == iHieNew)
		{
			if (0 == m_rayMarchCamera->getNodeMask())
			{
				m_rayMarchCamera->setNodeMask(~0);
			}

			if (3 == iHieNew)
			{
				m_fUnitUniform->set(1.0f);

				// 设置奥尔特云的旋转姿态
				osg::Matrixf mAttitude = osg::Matrixd::inverse(osg::Matrixd::identity());
				m_mAttitudeUniform->set(mAttitude);

				m_pGeodeOort3->setNodeMask(~0);
				m_pOortTransform4->setNodeMask(0);
			}
			else // 4 == iHieNes
			{
				m_fUnitUniform->set(1e-5f);

				// 设置奥尔特云的旋转姿态
				osg::Matrixf mAttitude = osg::Matrixd::inverse(
					osg::Matrixd(m_pOortTransform4->asPositionAttitudeTransform()->getAttitude()));
				m_mAttitudeUniform->set(mAttitude);

				m_pGeodeOort3->setNodeMask(0);
				m_pOortTransform4->setNodeMask(~0);
			}
		}
		else
		{
			if (0 != m_rayMarchCamera->getNodeMask())
			{
				m_rayMarchCamera->setNodeMask(0);
				m_pGeodeOort3->setNodeMask(0);
				m_pOortTransform4->setNodeMask(0);
			}
		}
	}

	return false;
}

void CGMOort::SetOortHiePos4(const osg::Vec3f& vHiePos)
{
	if(m_pOortTransform4.valid())
		m_pOortTransform4->asPositionAttitudeTransform()->setPosition(vHiePos);
}

void CGMOort::SetOortAttitude(const osg::Quat& qRotate)
{
	if (m_pOortTransform4.valid())
		m_pOortTransform4->asPositionAttitudeTransform()->setAttitude(qRotate);

	osg::Matrixf mAttitude = osg::Matrixd::inverse(osg::Matrixd(qRotate));
	m_mAttitudeUniform->set(mAttitude);
}

osg::Geometry* CGMOort::_MakeSphereGeometry(const float fRadius, const int iLatSegment) const
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

bool CGMOort::_InitOortStateSet(osg::StateSet * pSS)
{
	if (!pSS) return false;

	pSS->setMode(GL_BLEND, osg::StateAttribute::OFF);
	pSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSS->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK));

	pSS->addUniform(m_fUnitUniform.get());
	pSS->addUniform(m_fOortVisibleUniform.get());
	pSS->addUniform(m_mAttitudeUniform.get());
	pSS->addUniform(m_pCommonUniform->GetLevelArray());
	pSS->addUniform(m_pCommonUniform->GetTime());
	pSS->addUniform(m_pCommonUniform->GetScreenSize());
	pSS->addUniform(m_pCommonUniform->GetStarHiePos());
	pSS->addUniform(m_pCommonUniform->GetStarColor());
	pSS->addUniform(m_pCommonUniform->GetMainInvProjMatrix());

	int iUnit = 0;
	CGMKit::AddTexture(pSS, m_blueNoiseTex.get(), "blueNoiseTex", iUnit++);
	CGMKit::AddTexture(pSS, m_3DShapeTex.get(), "noiseShapeTex", iUnit++);
	CGMKit::AddTexture(pSS, m_3DErosionTex.get(), "noiseErosionTex", iUnit++);

	std::string strShader = m_pConfigData->strCorePath + m_strGalaxyShaderPath;
	CGMKit::LoadShader(pSS, strShader + "Oort.vert", strShader + "Oort.frag");

	return true;
}
