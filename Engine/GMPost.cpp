//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMPost.cpp
/// @brief		Galaxy-Music Engine - GMPost
/// @version	1.0
/// @author		LiuTao
/// @date		2022.07.20
//////////////////////////////////////////////////////////////////////////

#include "GMPost.h"
#include "GMKit.h"

using namespace GM;
/*************************************************************************
Macro Defines
*************************************************************************/


/*************************************************************************
Class
*************************************************************************/

/*************************************************************************
CGMGalaxy Methods
*************************************************************************/

/** @brief 构造 */
CGMPost::CGMPost() :
	m_pKernelData(nullptr), m_pConfigData(nullptr),
	m_strShaderPath("Shaders/PostShader/"),
	m_pVolumeTex(nullptr),
	m_iPostUnit(0), m_bVolume(false)
{
}

/** @brief 析构 */
CGMPost::~CGMPost()
{
}

/** @brief 初始化 */
bool CGMPost::Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform)
{
	m_pKernelData = pKernelData;
	m_pConfigData = pConfigData;
	m_pCommonUniform = pCommonUniform;

	m_bVolume = EGMRENDER_LOW != pConfigData->eRenderQuality ? true : false;

	return true;
}

/** @brief 更新 */
bool CGMPost::Update(double dDeltaTime)
{
	double dTime = osg::Timer::instance()->time_s();
	float fTimes = std::fmod((float)dTime, 1000000.0f);

	return true;
}

/** @brief 更新(在主相机更新姿态之后) */
bool CGMPost::UpdateLater(double dDeltaTime)
{
	return true;
}

/** @brief 加载 */
bool CGMPost::Load()
{
	if (m_pPostGeode.valid())
	{
		std::string strPostVertPath = m_pConfigData->strCorePath + m_strShaderPath + "Post.vert";
		std::string strPostFragPath = m_pConfigData->strCorePath + m_strShaderPath + "Post.frag";
		CGMKit::LoadShader(m_pPostGeode->getStateSet(), strPostVertPath, strPostFragPath, "Post");
	}
	return true;
}

void CGMPost::ResizeScreen(const int width, const int height)
{
	if (m_pPostCam.valid())
	{
		m_pPostCam->resize(width, height);
		m_pPostCam->setProjectionMatrixAsOrtho2D(0, width, 0, height);
		m_pPostCam->dirtyAttachmentMap();
	}

	_ResizeScreenTriangle(width, height);
}

bool CGMPost::CreatePost(osg::Texture* pSceneTex,
	osg::Texture* pBackgroundTex,
	osg::Texture* pForegroundTex)
{
	if (m_pPostCam.valid() || !pSceneTex || !pBackgroundTex || !pForegroundTex) return false;

	int width = m_pConfigData->iScreenWidth;
	int height = m_pConfigData->iScreenHeight;

	osg::Camera* pMainCam = GM_View->getCamera();
	osg::Viewport* vp = pMainCam->getViewport();
	vp->setViewport(0, 0, width, height);
	pMainCam->setRenderOrder(osg::Camera::PRE_RENDER, 20);
	pMainCam->attach(osg::Camera::COLOR_BUFFER0, pSceneTex);

	// Create post triangle
	m_pPostGeode = new osg::Geode();
	m_pPostGeode->addDrawable(_CreateScreenTriangle(width, height));
	osg::ref_ptr<osg::StateSet>	pSsPost = m_pPostGeode->getOrCreateStateSet();
	GLenum buffer = pMainCam->getGraphicsContext()->getTraits()->doubleBuffer ? GL_BACK : GL_FRONT;

	m_pPostCam = new osg::Camera;
	m_pPostCam->setName("postCamera");
	m_pPostCam->setGraphicsContext(pMainCam->getGraphicsContext());
	m_pPostCam->setClearMask(GL_COLOR_BUFFER_BIT);
	m_pPostCam->setClearColor(osg::Vec4(0.0, 0.0, 0.0, 0.0));
	m_pPostCam->setViewport(new osg::Viewport(0, 0, width, height));
	m_pPostCam->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
	m_pPostCam->setAllowEventFocus(false);
	m_pPostCam->setRenderOrder(osg::Camera::POST_RENDER, 100);
	m_pPostCam->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	m_pPostCam->setDrawBuffer(buffer);
	m_pPostCam->setReadBuffer(buffer);
	m_pPostCam->setViewMatrix(osg::Matrix::identity());
	m_pPostCam->setProjectionMatrixAsOrtho2D(0, width, 0, height);
	m_pPostCam->addChild(m_pPostGeode.get());

	GM_Root->addChild(m_pPostCam.get());

	pSsPost->addUniform(m_pCommonUniform->GetScreenSize());
	//pSsPost->setDefine("VOLUME", m_bVolume ? osg::StateAttribute::ON : osg::StateAttribute::OFF);

	CGMKit::AddTexture(pSsPost.get(), pSceneTex, "sceneTex", m_iPostUnit++);
	CGMKit::AddTexture(pSsPost.get(), pBackgroundTex, "backgroundTex", m_iPostUnit++);
	CGMKit::AddTexture(pSsPost.get(), pForegroundTex, "foregroundTex", m_iPostUnit++);

	std::string strPostVertPath = m_pConfigData->strCorePath + m_strShaderPath + "Post.vert";
	std::string strPostFragPath = m_pConfigData->strCorePath + m_strShaderPath + "Post.frag";
	return CGMKit::LoadShader(pSsPost.get(), strPostVertPath, strPostFragPath, "Post");
}

bool CGMPost::SetVolumeEnable(bool bEnabled, osg::Texture* pVolumeTex)
{
	if (EGMRENDER_LOW == m_pConfigData->eRenderQuality) return false;

	osg::ref_ptr<osg::StateSet>	pSsPost = m_pPostGeode->getStateSet();

	if (bEnabled)// 开启
	{
		if (pVolumeTex && !m_pVolumeTex.valid())
		{
			m_pVolumeTex = pVolumeTex;

			CGMKit::AddTexture(pSsPost, m_pVolumeTex, "volumeTex", m_iPostUnit++);
		}

		if (m_pVolumeTex.valid())
		{
			m_bVolume = true;
			return true;
		}
		else
		{
			return false;
		}
	}
	else// 关闭
	{
		m_bVolume = false;
		pSsPost->setDefine("VOLUME", osg::StateAttribute::OFF);
		return true;
	}
}

bool CGMPost::UpdateHierarchy(int iHieNew)
{
	if (EGMRENDER_LOW != m_pConfigData->eRenderQuality)
	{
		osg::ref_ptr<osg::StateSet>	pSsPost = m_pPostGeode->getStateSet();

		//if (m_bVolume && (4 == iHieNew || 3 == iHieNew))
		//{
		//	pSsPost->setDefine("VOLUME", osg::StateAttribute::ON);
		//}
		//else
		//{
		//	pSsPost->setDefine("VOLUME", osg::StateAttribute::OFF);
		//}
	}
	return true;
}

osg::Geometry* CGMPost::_CreateScreenTriangle(const int width, const int height)
{
	osg::Geometry* pGeometry = new osg::Geometry();

	osg::ref_ptr<osg::Vec3Array> verArray = new osg::Vec3Array;
	verArray->push_back(osg::Vec3(0, 0, 0));
	verArray->push_back(osg::Vec3(2 * width, 0, 0));
	verArray->push_back(osg::Vec3(0, 2 * height, 0));
	pGeometry->setVertexArray(verArray);

	osg::ref_ptr<osg::Vec2Array> textArray = new osg::Vec2Array;
	textArray->push_back(osg::Vec2(0, 0));
	textArray->push_back(osg::Vec2(2, 0));
	textArray->push_back(osg::Vec2(0, 2));
	pGeometry->setTexCoordArray(0, textArray);

	osg::ref_ptr <osg::Vec3Array> normal = new osg::Vec3Array;
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

void CGMPost::_ResizeScreenTriangle(const int width, const int height)
{
	osg::ref_ptr<osg::Geometry> pGeometry = m_pPostGeode->asGeode()->getDrawable(0)->asGeometry();
	osg::ref_ptr<osg::Vec3Array> verArray = new osg::Vec3Array;
	verArray->push_back(osg::Vec3(0, 0, 0));
	verArray->push_back(osg::Vec3(2 * width, 0, 0));
	verArray->push_back(osg::Vec3(0, 2 * height, 0));
	pGeometry->setVertexArray(verArray);
	pGeometry->dirtyBound();
}