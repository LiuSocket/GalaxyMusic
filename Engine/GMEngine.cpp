//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMVolumeBasic.h
/// @brief		Galaxy-Music Engine - GMEngine.cpp
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.10
//////////////////////////////////////////////////////////////////////////

#include "GMEngine.h"
#include "GMViewWidget.h"
#include "GMDataManager.h"
#include "GMCommonUniform.h"
#include "GMXml.h"
#include "GMGalaxy.h"
#include "GMAudio.h"
#include "GMPost.h"
#include "GMCameraManipulator.h"
#include <osgViewer/ViewerEventHandlers>
#include <osgQt/GraphicsWindowQt>
#include <QtCore/QTimer>

#include <iostream>

using namespace GM;

/*************************************************************************
Global Constants
*************************************************************************/
static const std::string g_strGMConfigFile = "GalaxyMusic.cfg";	//!< 配置文件名

/*************************************************************************
 Macro Defines
*************************************************************************/
#define GM_NEARFAR_RATIO			(1e-6)

/*************************************************************************
 CGMEngine Methods
*************************************************************************/

class ResizeEventHandler : public osgGA::GUIEventHandler
{
public:
	ResizeEventHandler(CGMEngine* pEngine): _pEngine(pEngine){}

	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&)
	{
		if (ea.getEventType() == osgGA::GUIEventAdapter::RESIZE && _pEngine)
		{
			_pEngine->ResizeScreen(ea.getWindowWidth(), ea.getWindowHeight());
			return true;
		}
		return false;
	}
private:
	CGMEngine* _pEngine = nullptr;
};

template<> CGMEngine* CGMSingleton<CGMEngine>::msSingleton = nullptr;

/** @brief 获取单例 */
CGMEngine& CGMEngine::getSingleton(void)
{
	if (!msSingleton)
		msSingleton = GM_NEW(CGMEngine);
	assert(msSingleton);
	return (*msSingleton);
}
CGMEngine* CGMEngine::getSingletonPtr(void)
{
	if (!msSingleton)
		msSingleton = GM_NEW(CGMEngine);
	assert(msSingleton);
	return msSingleton;
}

/** @brief 构造 */
CGMEngine::CGMEngine():
	m_pKernelData(nullptr), m_pConfigData(nullptr), m_pDataManager(nullptr), m_pManipulator(nullptr),
	m_bInit(false),
	m_dTimeLastFrame(0.0), m_fDeltaStep(0.0f), m_fConstantStep(0.1f),
	m_fGalaxyDiameter(1e21),
	m_pGalaxy(nullptr), m_pAudio(nullptr), m_pPost(nullptr),
	m_ePlayMode(EGMA_MOD_CIRCLE),
	m_pSceneTex(nullptr), m_pBackgroundTex(nullptr), m_pForegroundTex(nullptr)
{
	Init();
}

/** @brief 析构 */
CGMEngine::~CGMEngine()
{
	GM_DELETE(msSingleton);
}

/** @brief 初始化 */
bool CGMEngine::Init()
{
	if (m_bInit) return true;

	m_iRandom.seed(time(0));

	//!< 配置数据
	_LoadConfig();

	//!< 内核数据
	m_pKernelData = new SGMKernelData();
	//!< 当前空间层级编号默认4级，这样刚好看到银河系
	m_pKernelData->iHierarchy = 4;
	//!< 空间层级单位，不能修改，6级是整个宇宙，0级是人类尺度，级数编号越大，级数单位越大
	m_pKernelData->fUnitArray = new osg::DoubleArray;
	m_pKernelData->fUnitArray->reserve(7);
	m_pKernelData->fUnitArray->push_back(1.0);	// 0级空间层级单位
	m_pKernelData->fUnitArray->push_back(1e5);	// 1级空间层级单位
	m_pKernelData->fUnitArray->push_back(1e10);	// 2级空间层级单位
	m_pKernelData->fUnitArray->push_back(1e15);	// 3级空间层级单位
	m_pKernelData->fUnitArray->push_back(1e20);	// 4级空间层级单位
	m_pKernelData->fUnitArray->push_back(1e25);	// 5级空间层级单位
	m_pKernelData->fUnitArray->push_back(1e30);	// 6级空间层级单位
	//!< 眼点最后时刻的空间坐标，默认都在(1,1,1)位置
	m_pKernelData->vEyePosArray = new osg::Vec3dArray;
	m_pKernelData->vEyePosArray->reserve(7);
	m_pKernelData->vEyePosArray->push_back(osg::Vec3d(1, 1, 1));	// 0级空间
	m_pKernelData->vEyePosArray->push_back(osg::Vec3d(1, 1, 1));	// 1级空间
	m_pKernelData->vEyePosArray->push_back(osg::Vec3d(1, 1, 1));	// 2级空间
	m_pKernelData->vEyePosArray->push_back(osg::Vec3d(1, 1, 1));	// 3级空间
	m_pKernelData->vEyePosArray->push_back(osg::Vec3d(1, 1, 1));	// 4级空间
	m_pKernelData->vEyePosArray->push_back(osg::Vec3d(1, 1, 1));	// 5级空间
	m_pKernelData->vEyePosArray->push_back(osg::Vec3d(1, 1, 1));	// 6级空间

	GM_Root = new osg::Group();
	GM_View = new osgViewer::View();
	GM_View->setSceneData(GM_Root.get());

	m_pSceneTex = new osg::Texture2D();
	m_pSceneTex->setTextureSize(m_pConfigData->iScreenWidth, m_pConfigData->iScreenHeight);
	m_pSceneTex->setInternalFormat(GL_RGBA8);
	m_pSceneTex->setSourceFormat(GL_RGBA);
	m_pSceneTex->setSourceType(GL_UNSIGNED_BYTE);
	m_pSceneTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_pSceneTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_pSceneTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_pSceneTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_pSceneTex->setBorderColor(osg::Vec4d(0, 0, 0, 0));
	m_pSceneTex->setDataVariance(osg::Object::DYNAMIC);
	m_pSceneTex->setResizeNonPowerOfTwoHint(false);

	// 初始化背景相关节点
	_InitBackground();
	// 初始化前景相关节点
	_InitForeground();

	m_pCommonUniform = new CGMCommonUniform();
	m_pManipulator = new CGMCameraManipulator();
	m_pDataManager = new CGMDataManager();
	m_pGalaxy = new CGMGalaxy();
	m_pAudio = new CGMAudio();
	m_pPost = new CGMPost();

	m_pCommonUniform->Init(m_pKernelData, m_pConfigData);
	m_pDataManager->Init(m_pKernelData, m_pConfigData);
	m_pGalaxy->Init(m_pKernelData, m_pConfigData, m_pCommonUniform, m_pDataManager);
	m_pAudio->Init(m_pConfigData);
	m_pPost->Init(m_pKernelData, m_pConfigData, m_pCommonUniform);

	m_pGalaxy->CreateGalaxy(m_fGalaxyDiameter);

	GM_View->setCameraManipulator(m_pManipulator);
	//状态信息
	osgViewer::StatsHandler* pStatsHandler = new osgViewer::StatsHandler;
	pStatsHandler->setKeyEventTogglesOnScreenStats(osgGA::GUIEventAdapter::KEY_F1);
	GM_View->addEventHandler(pStatsHandler);

	m_bInit = true;
	m_pKernelData->bInited = m_bInit;

	GM_View->addEventHandler(new ResizeEventHandler(this));

	return true;
}


/** @brief 释放 */
void CGMEngine::Release()
{
	setlocale(LC_ALL, "C");
	if (GM_Viewer.valid())
	{
		GM_Viewer->stopThreading();
		GM_Viewer = 0L;
	}

	GM_DELETE(m_pAudio);
	GM_DELETE(m_pGalaxy);

	GM_DELETE(m_pDataManager);
	GM_DELETE(m_pCommonUniform);
	GM_DELETE(m_pConfigData);
	GM_DELETE(m_pKernelData);
	GM_DELETE(msSingleton);
}

/** @brief 更新 */
bool CGMEngine::Update()
{
	if (!m_bInit)
		return false;

	if (GM_Viewer->done())
		return true;

	if (GM_Viewer->getRunFrameScheme() == osgViewer::ViewerBase::CONTINUOUS || GM_Viewer->checkNeedToDoFrame())
	{
		double timeCurrFrame = osg::Timer::instance()->time_s();
		double deltaTime = timeCurrFrame - m_dTimeLastFrame; //单位:秒
		m_dTimeLastFrame = timeCurrFrame;

		float fInnerDeltaTime = deltaTime;
		fInnerDeltaTime += m_fDeltaStep;
		float updateStep = m_fConstantStep;
		while (fInnerDeltaTime >= updateStep)
		{
			_InnerUpdate(updateStep);
			fInnerDeltaTime -= updateStep;
		}
		m_fDeltaStep = fInnerDeltaTime;

		m_pCommonUniform->Update(deltaTime);
		m_pDataManager->Update(deltaTime);
		m_pGalaxy->Update(deltaTime);
		m_pAudio->Update(deltaTime);

		// 更新涟漪效果
		m_pCommonUniform->SetAudioLevel(m_pAudio->GetLevel());

		GM_Viewer->advance(deltaTime);
		GM_Viewer->eventTraversal();
		GM_Viewer->updateTraversal();

		// 在主相机改变位置后再更新
		_UpdateLater(deltaTime);

		GM_Viewer->renderingTraversals();

		// 渲染结束后再更新场景层级信息，否则会在临界点闪烁
		_UpdateScenes();
	}
	return true;
}

/** @brief 加载 */
bool CGMEngine::Load()
{
	m_pGalaxy->Load();
	m_pPost->Load();
	return true;
}

/** @brief 保存 */
bool CGMEngine::Save()
{
	m_pDataManager->Save();
	return false;
}

bool CGMEngine::SaveSolarData()
{
	m_pGalaxy->SaveSolarData();
	return true;
}

void CGMEngine::ResizeScreen(const int iW, const int iH)
{
	osg::ref_ptr<osg::Camera> pMainCam = GM_View->getCamera();
	if (pMainCam.valid())
	{
		double fovy, aspectRatio, zNear, zFar;
		pMainCam->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

		pMainCam->resize(iW, iH);
		double fAspetRatio = double(iW) / double(iH);
		pMainCam->setProjectionMatrixAsPerspective(fovy, fAspetRatio, zNear, zFar);
		pMainCam->dirtyAttachmentMap();
	}

	if (m_pKernelData->pBackgroundCam.valid())
		m_pKernelData->pBackgroundCam->resize(iW, iH);
	if (m_pKernelData->pForegroundCam.valid())
		m_pKernelData->pForegroundCam->resize(iW, iH);
	if (m_pCommonUniform)
		m_pCommonUniform->ResizeScreen(iW, iH);
	if (m_pGalaxy)
		m_pGalaxy->ResizeScreen(iW, iH);
	if (m_pPost)
		m_pPost->ResizeScreen(iW, iH);
}

void CGMEngine::SetEditMode(const bool bEnable)
{
	m_pGalaxy->SetEditMode(bEnable);
}

bool CGMEngine::GetEditMode() const
{
	return 	m_pGalaxy->GetEditMode();
}

void CGMEngine::SetHandleHover(const bool bHover)
{
	m_pGalaxy->SetHandleHover(bHover);
}

void CGMEngine::SetCapture(const bool bEnable)
{
	m_pGalaxy->SetCapture(bEnable);
}

bool CGMEngine::SetAudio(const double fX, const double fY)
{
	// 世界转星辰空间坐标
	double fGalaxyX = 0.0f;
	double fGalaxyY = 0.0f;
	double fGalaxyZ = 0.0f;
	_World2GalaxyCoord(fX, fY, fGalaxyX, fGalaxyY);

	std::wstring wstrName;
	if (m_pDataManager->FindAudio(fGalaxyX, fGalaxyY, fGalaxyZ, wstrName))
	{
		m_pAudio->SetCurrentAudio(wstrName);
		m_pAudio->AudioControl(EGMA_CMD_PLAY);

		double fWorldX, fWorldY;
		_GalaxyCoord2World(fGalaxyX, fGalaxyY, fWorldX, fWorldY);
		m_pGalaxy->SetCurrentStar(osg::Vec3f(fWorldX, fWorldY, 0.0f), wstrName);
		return true;
	}
	else
	{
		return false;
	}
}

bool CGMEngine::Play()
{
	std::wstring wstrCurrentFile = m_pAudio->GetCurrentAudio();
	if (L"" == wstrCurrentFile)
	{
		// 第一次播放
		wstrCurrentFile = m_pDataManager->GetCurrentAudio();
		m_pAudio->SetCurrentAudio(wstrCurrentFile);

		SGMGalaxyCoord vGC = m_pDataManager->GetGalaxyCoord(wstrCurrentFile);
		double fWorldX, fWorldY;
		_GalaxyCoord2World(vGC.x, vGC.y, fWorldX, fWorldY);
		m_pGalaxy->SetCurrentStar(osg::Vec3f(fWorldX, fWorldY, 0.0f), wstrCurrentFile);
	}

	m_pAudio->AudioControl(EGMA_CMD_PLAY);

	return true;
}

/** @brief 暂停 */
bool CGMEngine::Pause()
{
	m_pAudio->AudioControl(EGMA_CMD_PAUSE);
	return true;
}

/** @brief 停止 */
bool CGMEngine::Stop()
{
	m_pAudio->AudioControl(EGMA_CMD_STOP);
	return true;
}

bool CGMEngine::Last()
{
	if (!m_pGalaxy->GetEditMode())
	{
		std::wstring wstrCurrentFile = m_pDataManager->GetLastAudio();
		if (L"" == wstrCurrentFile) return false;

		m_pAudio->AudioControl(EGMA_CMD_CLOSE);
		m_pAudio->SetCurrentAudio(wstrCurrentFile);
		m_pAudio->AudioControl(EGMA_CMD_OPEN);
		m_pAudio->AudioControl(EGMA_CMD_PLAY);

		SGMGalaxyCoord vGC = m_pDataManager->GetGalaxyCoord(wstrCurrentFile);
		double fWorldX, fWorldY;
		_GalaxyCoord2World(vGC.x, vGC.y, fWorldX, fWorldY);
		m_pGalaxy->SetCurrentStar(osg::Vec3f(fWorldX, fWorldY, 0.0f), wstrCurrentFile);

		return true;
	}

	return false;
}

/** @brief 下一首 */
bool CGMEngine::Next()
{
	if (m_pGalaxy->GetEditMode())
	{
		_Next(EGMA_MOD_SINGLE);
	}
	else
	{
		_Next(m_ePlayMode);
	}

	return true;
}

bool CGMEngine::SetVolume(const float fVolume)
{
	return m_pAudio->SetVolume(osg::clampBetween(fVolume, 0.0f, 1.0f));
}

float CGMEngine::GetVolume() const
{
	return m_pAudio->GetVolume();
}

bool CGMEngine::SetPlayMode(EGMA_MODE eMode)
{
	m_ePlayMode = eMode;
	return true;
}

std::wstring CGMEngine::GetAudioName() const
{
	return m_pAudio->GetCurrentAudio();
}

bool CGMEngine::SetAudioCurrentTime(const int iTime)
{	
	return m_pAudio->SetAudioCurrentTime(iTime);
}

int CGMEngine::GetAudioCurrentTime() const
{
	return m_pAudio->GetAudioCurrentTime();
}

int CGMEngine::GetAudioDuration() const
{
	return m_pAudio->GetAudioDuration();
}

void CGMEngine::Welcome()
{
	m_pGalaxy->Welcome();
	m_pAudio->Welcome();
}

bool CGMEngine::IsWelcomeFinished() const
{
	return m_pAudio->IsWelcomeFinished();
}

const std::vector<std::wstring> GM::CGMEngine::GetPlayingOrder() const
{
	return m_pDataManager->GetPlayingOrder();
}

void CGMEngine::SetMousePosition(const SGMVector3& vHiePos)
{
	osg::Vec3d vHierarchyPos = osg::Vec3d(vHiePos.x, vHiePos.y, vHiePos.z);
	m_pGalaxy->SetMousePosition(vHierarchyPos);
}

void CGMEngine::SetCurrentStarAudioCoord(const SGMAudioCoord& vAudioCoord)
{
	m_pGalaxy->SetPlayingStarAudioCoord(vAudioCoord);
}

SGMAudioCoord CGMEngine::GetCurrentStarAudioCoord() const
{
	return m_pGalaxy->GetPlayingStarAudioCoord();
}

SGMVector3 CGMEngine::GetCurrentStarWorldPos() const
{
	osg::Vec3d vPos = m_pGalaxy->GetStarWorldPos();
	return SGMVector3(vPos.x(), vPos.y(), vPos.z());
}

SGMAudioCoord CGMEngine::GetAudioCoord(const std::wstring & strName) const
{
	return m_pDataManager->GetAudioCoord(strName);
}

double CGMEngine::GetHierarchyTargetDistance() const
{
	return m_pManipulator->GetHierarchyTargetDistance();
}

double CGMEngine::GetUnit(const int iHierarchy) const
{
	if (iHierarchy >= 0 && iHierarchy < m_pKernelData->fUnitArray->size())
	{
		return m_pKernelData->fUnitArray->at(iHierarchy);
	}
	else
	{
		return 0;
	}
}

bool CGMEngine::GetSmallerHierarchyCoord(SGMVector3 & vX, SGMVector3 & vY, SGMVector3 & vZ) const
{
	vX = _OSG2GM(osg::Vec3d(1, 0, 0));
	vY = _OSG2GM(osg::Vec3d(0, 1, 0));
	vZ = _OSG2GM(osg::Vec3d(0, 0, 1));

	int iHieSmaller = m_pKernelData->iHierarchy - 1;
	switch (iHieSmaller)
	{
	case 1:	// 2->1
	{
	}
	break;
	case 3:	// 4->3
	{
		osg::Matrix mMatrix = osg::Matrix();
		mMatrix.preMultRotate(m_pGalaxy->GetNearStarRotate());
		vX = _OSG2GM(mMatrix.preMult(osg::Vec3d(1, 0, 0)));
		vY = _OSG2GM(mMatrix.preMult(osg::Vec3d(0, 1, 0)));
		vZ = _OSG2GM(mMatrix.preMult(osg::Vec3d(0, 0, 1)));
		vX.Normalize();
		vY.Normalize();
		vZ.Normalize();
	}
	break;
	case 0:	// 1->0
	case 2:	// 3->2
	case 4:	// 5->4
	case 5: // 6->5

	default:
		return false;
	}

	return true;
}

bool CGMEngine::GetBiggerHierarchyCoord(SGMVector3& vX, SGMVector3& vY, SGMVector3& vZ) const
{
	vX = _OSG2GM(osg::Vec3d(1, 0, 0));
	vY = _OSG2GM(osg::Vec3d(0, 1, 0));
	vZ = _OSG2GM(osg::Vec3d(0, 0, 1));

	int iHieBigger = m_pKernelData->iHierarchy + 1;
	switch (iHieBigger)
	{
	case 2:	// 1->2
	{
	}
	break;
	case 4:	// 3->4
	{
		osg::Matrix mMatrix = osg::Matrix();
		mMatrix.preMultRotate(m_pGalaxy->GetNearStarRotate().inverse());
		vX = _OSG2GM(mMatrix.preMult(osg::Vec3d(1, 0, 0)));
		vY = _OSG2GM(mMatrix.preMult(osg::Vec3d(0, 1, 0)));
		vZ = _OSG2GM(mMatrix.preMult(osg::Vec3d(0, 0, 1)));
		vX.Normalize();
		vY.Normalize();
		vZ.Normalize();
	}
	break;
	case 1:	// 0->1
	case 3:	// 2->3
	case 5: // 4->5
	case 6:	// 5->6
	{
	}
	break;
	default:
		return false;
	}

	return true;
}

bool CGMEngine::AddHierarchy(const SGMVector3& vHieEyePos, const SGMVector3& vHieTargetPos)
{
	std::cout << "iHierarchy：" << m_pKernelData->iHierarchy << " -> " << (m_pKernelData->iHierarchy + 1) << std::endl;

	if (GM_HIERARCHY_MAX == m_pKernelData->iHierarchy)
	{
		return false;
	}
	else
	{
		const osg::Vec3d vOldHierarchyEyePos = osg::Vec3d(vHieEyePos.x, vHieEyePos.y, vHieEyePos.z);
		const osg::Vec3d vOldHierarchyTargetPos = osg::Vec3d(vHieTargetPos.x, vHieTargetPos.y, vHieTargetPos.z);

		// 记录当前层级眼点空间坐标
		m_pKernelData->vEyePosArray->at(m_pKernelData->iHierarchy) = vOldHierarchyEyePos;

		int iHieNew = ++(m_pKernelData->iHierarchy);
		// 计算升级后空间的眼点空间坐标
		m_pKernelData->vEyePosArray->at(iHieNew) = _AfterAddHierarchy(osg::Vec4d(vOldHierarchyEyePos, 1.0));

		return true;
	}
}

bool CGMEngine::SubHierarchy(const SGMVector3& vHieEyePos, const SGMVector3& vHieTargetPos)
{
	std::cout << "iHierarchy：" << m_pKernelData->iHierarchy << " -> " << (m_pKernelData->iHierarchy - 1) << std::endl;

	if (0 >= m_pKernelData->iHierarchy)
	{
		return false;
	}
	else
	{
		const osg::Vec3d vOldHierarchyEyePos = osg::Vec3d(vHieEyePos.x, vHieEyePos.y, vHieEyePos.z);
		const osg::Vec3d vOldHierarchyTargetPos = osg::Vec3d(vHieTargetPos.x, vHieTargetPos.y, vHieTargetPos.z);

		// 记录当前层级眼点空间坐标
		m_pKernelData->vEyePosArray->at(m_pKernelData->iHierarchy) = vOldHierarchyEyePos;

		int iHieNew = --(m_pKernelData->iHierarchy);
		// 计算降级后空间的眼点空间坐标
		m_pKernelData->vEyePosArray->at(iHieNew) = _AfterSubHierarchy(osg::Vec4d(vOldHierarchyEyePos, 1.0));

		return true;
	}
}

SGMVector3 CGMEngine::AfterAddHierarchy(const SGMVector4& vBefore) const
{
	osg::Vec3d vPosAfterAdd = _AfterAddHierarchy(osg::Vec4d(vBefore.x, vBefore.y, vBefore.z, vBefore.w));
	return SGMVector3(vPosAfterAdd.x(), vPosAfterAdd.y(), vPosAfterAdd.z());
}

SGMVector3 CGMEngine::AfterSubHierarchy(const SGMVector4& vBefore) const
{
	osg::Vec3d vPosAfterSub = _AfterSubHierarchy(osg::Vec4d(vBefore.x, vBefore.y, vBefore.z, vBefore.w));
	return SGMVector3(vPosAfterSub.x(), vPosAfterSub.y(), vPosAfterSub.z());
}

bool CGMEngine::GetNearestCelestialBody(const SGMVector3& vSearchHiePos,
	SGMVector3& vPlanetHiePos, double& fOrbitalPeriod) const
{
	return m_pGalaxy->GetNearestCelestialBody(vSearchHiePos, vPlanetHiePos, fOrbitalPeriod);
}

void CGMEngine::GetCelestialBody(SGMVector3& vPlanetPos, double& fOrbitalPeriod) const
{
	m_pGalaxy->GetCelestialBody(vPlanetPos, fOrbitalPeriod);
}

double CGMEngine::GetCelestialMeanRadius() const
{
	return m_pGalaxy->GetCelestialMeanRadius();
}

double CGMEngine::GetCelestialRadius(const double fLatitude) const
{
	return m_pGalaxy->GetCelestialRadius(fLatitude);
}

SGMVector3 CGMEngine::GetCelestialNorth() const
{
	return m_pGalaxy->GetCelestialNorth();
}

unsigned int CGMEngine::GetCelestialIndex() const
{
	return m_pGalaxy->GetCelestialIndex();
}

SGMVector3 CGMEngine::GetNearStarWorldPos() const
{
	osg::Vec3d vNearStarWorldPos = m_pGalaxy->GetNearStarWorldPos();
	return SGMVector3(vNearStarWorldPos.x(), vNearStarWorldPos.y(), vNearStarWorldPos.z());
}

SGMVector4f CGMEngine::Angle2Color(const float fEmotionAngle) const
{
	osg::Vec4f vColor = m_pDataManager->Angle2Color(fEmotionAngle);
	return SGMVector4f(vColor.r(), vColor.g(), vColor.b(), vColor.a());
}

CGMViewWidget* CGMEngine::CreateViewWidget(QWidget* parent)
{
	GM_Viewer = new CGMViewWidget(GM_View, parent);
	GM_View->getCamera()->setProjectionMatrixAsPerspective(
		m_pConfigData->fFovy,
		static_cast<double>(m_pConfigData->iScreenWidth) / static_cast<double>(m_pConfigData->iScreenHeight),
		0.0003, 30.0);

	m_pPost->CreatePost(m_pSceneTex.get(), m_pBackgroundTex.get(), m_pForegroundTex.get());
	if (EGMRENDER_LOW != m_pConfigData->eRenderQuality)
	{
		//m_pPost->SetVolumeEnable(true, m_pGalaxy->GetTAATex());
	}
	return GM_Viewer.get();
}

/** @brief 加载配置 */
bool CGMEngine::_LoadConfig()
{
	CGMXml hXML;
	hXML.Load(g_strGMConfigFile, "Config");
	m_pConfigData = new SGMConfigData;

	// 解析系统配置
	CGMXmlNode sNode = hXML.GetChild("System");

	m_pConfigData->strCorePath = sNode.GetPropStr("corePath", m_pConfigData->strCorePath.c_str());
	m_pConfigData->strMediaPath = sNode.GetPropWStr("mediaPath", m_pConfigData->strMediaPath.c_str());
	m_pConfigData->eRenderQuality = EGMRENDER_QUALITY(sNode.GetPropInt("renderQuality", m_pConfigData->eRenderQuality));
	m_pConfigData->bPhoto = sNode.GetPropBool("photo", m_pConfigData->bPhoto);
	m_pConfigData->bWanderingEarth = sNode.GetPropBool("wanderingEarth", m_pConfigData->bWanderingEarth);
	m_pConfigData->fFovy = sNode.GetPropFloat("fovy", m_pConfigData->fFovy);
	m_pConfigData->fVolume = sNode.GetPropFloat("volume", m_pConfigData->fVolume);
	m_pConfigData->fMinBPM = sNode.GetPropDouble("minBPM", m_pConfigData->fMinBPM);

	return true;
}

void CGMEngine::_InitBackground()
{
	m_pBackgroundTex = new osg::Texture2D();
	m_pBackgroundTex->setTextureSize(m_pConfigData->iScreenWidth, m_pConfigData->iScreenHeight);
	m_pBackgroundTex->setInternalFormat(GL_RGBA8);
	m_pBackgroundTex->setSourceFormat(GL_RGBA);
	m_pBackgroundTex->setSourceType(GL_UNSIGNED_BYTE);
	m_pBackgroundTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_pBackgroundTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_pBackgroundTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_pBackgroundTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_pBackgroundTex->setBorderColor(osg::Vec4d(0, 0, 0, 0));
	m_pBackgroundTex->setDataVariance(osg::Object::DYNAMIC);
	m_pBackgroundTex->setResizeNonPowerOfTwoHint(false);

	m_pKernelData->pBackgroundCam = new osg::Camera;
	m_pKernelData->pBackgroundCam->setName("backgroundCamera");
	m_pKernelData->pBackgroundCam->setReferenceFrame(osg::Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT);
	m_pKernelData->pBackgroundCam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_pKernelData->pBackgroundCam->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
	m_pKernelData->pBackgroundCam->setViewport(0, 0, m_pConfigData->iScreenWidth, m_pConfigData->iScreenHeight);
	m_pKernelData->pBackgroundCam->setRenderOrder(osg::Camera::PRE_RENDER, 0);
	m_pKernelData->pBackgroundCam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	m_pKernelData->pBackgroundCam->attach(osg::Camera::COLOR_BUFFER, m_pBackgroundTex.get());
	m_pKernelData->pBackgroundCam->setAllowEventFocus(false);
	m_pKernelData->pBackgroundCam->setProjectionMatrixAsPerspective(
		m_pConfigData->fFovy,
		double(m_pConfigData->iScreenWidth) / double(m_pConfigData->iScreenHeight),
		1.0, 1e5);
	m_pKernelData->pBackgroundCam->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	GM_Root->addChild(m_pKernelData->pBackgroundCam.get());
}

void CGMEngine::_InitForeground()
{
	m_pForegroundTex = new osg::Texture2D();
	m_pForegroundTex->setTextureSize(m_pConfigData->iScreenWidth, m_pConfigData->iScreenHeight);
	m_pForegroundTex->setInternalFormat(GL_RGBA8);
	m_pForegroundTex->setSourceFormat(GL_RGBA);
	m_pForegroundTex->setSourceType(GL_UNSIGNED_BYTE);
	m_pForegroundTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_pForegroundTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_pForegroundTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_pForegroundTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_pForegroundTex->setBorderColor(osg::Vec4d(0, 0, 0, 0));
	m_pForegroundTex->setDataVariance(osg::Object::DYNAMIC);
	m_pForegroundTex->setResizeNonPowerOfTwoHint(false);
	
	m_pKernelData->pForegroundCam = new osg::Camera;
	m_pKernelData->pForegroundCam->setName("foregroundCamera");
	m_pKernelData->pForegroundCam->setReferenceFrame(osg::Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT);
	m_pKernelData->pForegroundCam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_pKernelData->pForegroundCam->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
	m_pKernelData->pForegroundCam->setViewport(0, 0, m_pConfigData->iScreenWidth, m_pConfigData->iScreenHeight);
	m_pKernelData->pForegroundCam->setRenderOrder(osg::Camera::PRE_RENDER, 0);
	m_pKernelData->pForegroundCam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	m_pKernelData->pForegroundCam->attach(osg::Camera::COLOR_BUFFER, m_pForegroundTex.get());
	m_pKernelData->pForegroundCam->setAllowEventFocus(false);
	m_pKernelData->pForegroundCam->setProjectionMatrixAsPerspective(
		m_pConfigData->fFovy,
		double(m_pConfigData->iScreenWidth) / double(m_pConfigData->iScreenHeight),
		1.0, 1e5);
	m_pKernelData->pForegroundCam->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	GM_Root->addChild(m_pKernelData->pForegroundCam.get());
}

void CGMEngine::_Next(const EGMA_MODE eMode)
{
	std::wstring wstrCurrentFile = L"";

	switch (eMode)
	{
	case EGMA_MOD_SINGLE:
	{
		m_pAudio->AudioControl(EGMA_CMD_PLAY);
	}
	break;
	case EGMA_MOD_CIRCLE:
	{
		m_pAudio->AudioControl(EGMA_CMD_CLOSE);

		int iMaxNum = m_pDataManager->GetAudioNum();
		unsigned int iNext = (m_pDataManager->GetUID() + 1) % iMaxNum;// wrong to do
		wstrCurrentFile = m_pDataManager->FindAudio(iNext);

		m_pAudio->SetCurrentAudio(wstrCurrentFile);
		m_pAudio->AudioControl(EGMA_CMD_OPEN);
		m_pAudio->AudioControl(EGMA_CMD_PLAY);

		SGMGalaxyCoord vGC = m_pDataManager->GetGalaxyCoord(wstrCurrentFile);
		double fWorldX, fWorldY;
		_GalaxyCoord2World(vGC.x, vGC.y, fWorldX, fWorldY);
		m_pGalaxy->SetCurrentStar(osg::Vec3f(fWorldX, fWorldY, 0.0f), wstrCurrentFile);
	}
	break;
	case EGMA_MOD_RANDOM:
	{
		m_pAudio->AudioControl(EGMA_CMD_CLOSE);

		int iMaxNum = m_pDataManager->GetAudioNum();
		std::uniform_int_distribution<> iPseudoNoise(0, iMaxNum-1);
		int iNext = iPseudoNoise(m_iRandom);
		wstrCurrentFile = m_pDataManager->FindAudio(iNext);
		while (L"" == wstrCurrentFile)
		{
			iNext = iPseudoNoise(m_iRandom);
			wstrCurrentFile = m_pDataManager->FindAudio(iNext);
		}
		m_pAudio->SetCurrentAudio(wstrCurrentFile);
		m_pAudio->AudioControl(EGMA_CMD_OPEN);
		m_pAudio->AudioControl(EGMA_CMD_PLAY);

		SGMGalaxyCoord vGC = m_pDataManager->GetGalaxyCoord(wstrCurrentFile);
		double fWorldX, fWorldY;
		_GalaxyCoord2World(vGC.x, vGC.y, fWorldX, fWorldY);
		m_pGalaxy->SetCurrentStar(osg::Vec3f(fWorldX, fWorldY, 0.0f), wstrCurrentFile);
	}
	break;
	case EGMA_MOD_ORDER:
	{
		m_pAudio->AudioControl(EGMA_CMD_CLOSE);
	}
	break;
	default:
	{
		m_pAudio->AudioControl(EGMA_CMD_CLOSE);
	}
	break;
	}
}

void CGMEngine::_InnerUpdate(const float updateStep)
{
	if (m_pAudio->IsWelcomeFinished() && m_pAudio->IsAudioOver())
	{
		if (m_pGalaxy->GetEditMode())
		{
			_Next(EGMA_MOD_SINGLE);
		}
		else
		{
			_Next(m_ePlayMode);
		}
	}

	if (3 == m_pKernelData->iHierarchy || 2 == m_pKernelData->iHierarchy)
	{
		SGMVector3 vTargetHiePos = _OSG2GM(m_pManipulator->GetHierarchyTargetPos());
		SGMVector3 vCelestialBodyDeltaPos = m_pGalaxy->UpdateCelestialBody(vTargetHiePos);
		if (0 != vCelestialBodyDeltaPos.Length())
		{
			if (2 == m_pKernelData->iHierarchy)
			{
				m_pManipulator->ChangeCenter(vCelestialBodyDeltaPos);
			}
		}
	}
}

bool CGMEngine::_UpdateLater(const double dDeltaTime)
{
	// background camera
	osg::Vec3d vEye, vCenter, vUp;
	GM_View->getCamera()->getViewMatrixAsLookAt(vEye, vCenter, vUp);
	osg::Vec3d vBackCamDir = vCenter - vEye;
	m_pKernelData->pBackgroundCam->setViewMatrixAsLookAt(osg::Vec3d(0, 0, 0), vBackCamDir, vUp);
	//前景相机和主相机有相同的姿态
	m_pKernelData->pForegroundCam->setViewMatrixAsLookAt(vEye, vCenter, vUp);
	double fFovy, fAspectRatio, fZNear, fZFar;
	GM_View->getCamera()->getProjectionMatrixAsPerspective(fFovy, fAspectRatio, fZNear, fZFar);
	m_pKernelData->pBackgroundCam->setProjectionMatrixAsPerspective(fFovy, fAspectRatio, fZNear, fZFar);
	m_pKernelData->pForegroundCam->setProjectionMatrixAsPerspective(fFovy, fAspectRatio, fZNear, fZFar);

	m_pCommonUniform->UpdateLater(dDeltaTime);
	m_pGalaxy->UpdateLater(dDeltaTime);

	return true;
}

bool CGMEngine::_World2GalaxyCoord(const double fX, const double fY, double & fGalaxyX, double & fGalaxyY) const
{
	double fRadius = 0.5*m_fGalaxyDiameter;
	fGalaxyX = fX / fRadius;
	fGalaxyY = fY / fRadius;
	return true;
}

bool CGMEngine::_GalaxyCoord2World(const double fGalaxyX, const double fGalaxyY, double & fX, double & fY) const
{
	double fRadius = 0.5*m_fGalaxyDiameter;
	fX = fGalaxyX * fRadius;
	fY = fGalaxyY * fRadius;
	return true;
}

osg::Vec3d CGMEngine::_AfterAddHierarchy(const osg::Vec4d& vBefore) const
{
	osg::Vec4d vAfter = m_pGalaxy->HierarchyAddMatrix().preMult(vBefore);
	return osg::Vec3d(vAfter.x(), vAfter.y(), vAfter.z());
}

osg::Vec3d CGMEngine::_AfterSubHierarchy(const osg::Vec4d& vBefore) const
{
	if(3 == m_pKernelData->iHierarchy) // 4->3
	{
		// 4->3时更新最近恒星的位置
		m_pGalaxy->RefreshNearStarWorldPos();
	}
	osg::Vec4d vAfter = m_pGalaxy->HierarchySubMatrix().preMult(vBefore);
	return osg::Vec3d(vAfter.x(), vAfter.y(), vAfter.z());
}

void CGMEngine::_UpdateScenes()
{
	// 这行之前，层级还没有变化
	bool bNeedUpdate = m_pManipulator->UpdateHierarchy();
	// 这行之后，层级可能会变化，+1或者-1
	_UpdateNearFar();
	if (!bNeedUpdate) return;

	m_pCommonUniform->UpdateHierarchy(m_pKernelData->iHierarchy);
	m_pGalaxy->UpdateHierarchy(m_pKernelData->iHierarchy);
	m_pPost->UpdateHierarchy(m_pKernelData->iHierarchy);

	switch (m_pKernelData->iHierarchy)
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

	}
	break;
	case 4:
	{

	}
	break;
	case 5:
	{

	}
	break;
	case 6:
	{

	}
	break;
	default:
	break;
	}
}

void CGMEngine::_UpdateNearFar()
{
	osg::Camera* camera = GM_View->getCamera();
	if (!camera) return;

	double fFovy, fAspectRatio, fNear, fFar;
	camera->getProjectionMatrixAsPerspective(fFovy, fAspectRatio, fNear, fFar);

	double fEyeDistance = m_pManipulator->GetHierarchyEyePos().length();
	double fDistance = m_pManipulator->GetHierarchyTargetDistance();
	double fUnit = GM_ENGINE.GetUnit();
	double fFarNew = 10.0;
	double fNearNew = fFarNew * GM_NEARFAR_RATIO;
	switch (m_pKernelData->iHierarchy)
	{
	case 1:
	{
		double fEyeAlt1 = m_pGalaxy->GetEyeAltitude() / fUnit;
		fNearNew = 0.1 * fEyeAlt1;
		fFarNew = fNearNew / GM_NEARFAR_RATIO;
	}
	break;
	case 2:
	{
		fFarNew = fDistance + 1e3;
		fNearNew = fFarNew * GM_NEARFAR_RATIO;
		// 如果近截面太远，则缩小近截面
		double fEyeAlt2 = m_pGalaxy->GetEyeAltitude() / fUnit;
		if (fNearNew > 0.1* fEyeAlt2)
		{
			fNearNew = 0.1* fEyeAlt2;
			fFarNew = fNearNew / GM_NEARFAR_RATIO;
		}
	}
	break;
	case 3:
	{
		fNearNew = osg::clampBetween(fEyeDistance - 2e-2, 1e-5, 0.1);
		fFarNew = fNearNew / GM_NEARFAR_RATIO;
	}
	break;
	case 4:
	{
		if (fEyeDistance < (m_fGalaxyDiameter * 0.5 / fUnit))// 在星系内部
		{
			fFarNew = osg::clampBetween(fEyeDistance + m_fGalaxyDiameter / fUnit, 2.0, 1000.0);
			fNearNew = fFarNew * GM_NEARFAR_RATIO;
		}
		else// 在星系外面
		{
			fNearNew = osg::clampBetween(fEyeDistance - m_fGalaxyDiameter / fUnit, 2e-5, 0.4);
			fFarNew = fNearNew / GM_NEARFAR_RATIO;
		}
	}
	break;
	case 5:
	{
		fNearNew = osg::clampBetween(fEyeDistance - m_fGalaxyDiameter * 0.5 / fUnit,
			1e-4, _Mix(2e-3, 0.05, osg::clampBetween(fEyeDistance / 50.0, 0.0, 1.0)));
		fFarNew = fNearNew / GM_NEARFAR_RATIO;
	}
	break;
	default:
		break;
	}
	camera->setProjectionMatrixAsPerspective(fFovy, fAspectRatio, fNearNew, fFarNew);
}
