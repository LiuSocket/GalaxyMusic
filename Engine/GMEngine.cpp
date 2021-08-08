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
#include "GMDataManager.h"
#include "GMGalaxy.h"
#include "GMAudio.h"
#include "GMCameraManipulator.h"
#include <osgViewer/ViewerEventHandlers>

using namespace GM;

/*************************************************************************
 CGMEngine Methods
*************************************************************************/

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
	m_pKernelData(nullptr), m_pConfigData(nullptr), m_bInit(false), m_bHasWelcomed(false),
	m_dTimeLastFrame(0.0), m_fDeltaStep(0.0f), m_fConstantStep(0.1f),
	m_fGalaxyDiameter(10.0f),
	m_pGalaxy(nullptr), m_pAudio(nullptr), m_ePlayMode(EGMA_MOD_SINGLE)
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

	//!< 内核数据
	m_pKernelData = new SGMKernelData();
	GM_Root = new osg::Group();
	GM_Viewer = new osgViewer::Viewer();

	//!< 配置数据
	m_pConfigData = new SGMConfigData();

	m_pDataManager = new CGMDataManager();
	m_pGalaxy = new CGMGalaxy();
	m_pAudio = new CGMAudio();

	m_pDataManager->Init(m_pConfigData);
	m_pGalaxy->Init(m_pKernelData, m_pConfigData);
	m_pAudio->Init(m_pConfigData);

	m_pGalaxy->CreateGalaxy(m_fGalaxyDiameter);

	GM_Viewer->setCameraManipulator(new CGMCameraManipulator());
	//状态信息
	osgViewer::StatsHandler* pStatsHandler = new osgViewer::StatsHandler;
	pStatsHandler->setKeyEventTogglesOnScreenStats(osgGA::GUIEventAdapter::KEY_F1);
	GM_Viewer->addEventHandler(pStatsHandler);
	GM_Viewer->realize();

	m_bInit = true;
	m_pKernelData->bInited = m_bInit;

	// 临时代码
	m_ePlayMode = EGMA_MOD_RANDOM;

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

		m_pGalaxy->Update(deltaTime);
		m_pAudio->Update(deltaTime);

		// 更新涟漪效果
		m_pGalaxy->SetAudioLevel(m_pAudio->GetLevel());

		GM_Viewer->advance(deltaTime);
		GM_Viewer->eventTraversal();
		GM_Viewer->updateTraversal();

		// 为实现TAA相机和其他相机同步抖动，需要每帧更新static成员变量m_iShakeCount
		CGMVolumeBasic::AddShakeCount();
		m_pGalaxy->UpdateLater(deltaTime);

		GM_Viewer->renderingTraversals();
	}
	return true;
}

/** @brief 加载 */
bool CGMEngine::Load()
{
	m_pGalaxy->Load();

	return true;
}

/** @brief 保存 */
bool CGMEngine::Save()
{
	m_pDataManager->Save();
	return false;
}

void CGMEngine::SetEditMode(const bool bEnable)
{
	m_pGalaxy->SetEditMode(bEnable);
	m_ePlayMode = bEnable ? EGMA_MOD_SINGLE : EGMA_MOD_RANDOM;
}

void CGMEngine::SetCapture(const bool bEnable)
{
	m_pGalaxy->SetCapture(bEnable);
}

bool CGMEngine::SetAudio(const float fX, const float fY)
{
	// 世界转星辰空间坐标
	float fGalaxyX = 0.0f;
	float fGalaxyY = 0.0f;
	float fGalaxyZ = 0.0f;
	_World2GalaxyCoord(fX, fY, fGalaxyX, fGalaxyY);

	std::wstring wstrName = m_pDataManager->FindAudio(fGalaxyX, fGalaxyY, fGalaxyZ);
	if (L"" != wstrName)
	{
		m_pAudio->SetCurrentAudio(wstrName);
		m_pAudio->AudioControl(EGMA_CMD_PLAY);

		float fWorldX, fWorldY;
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
	if (L"" == m_pAudio->GetCurrentAudio())
	{
		int iMaxNum = m_pDataManager->GetAudioNum();
		m_iRandom.seed(time(0));
		int iNext = m_iRandom() % iMaxNum;
		std::wstring wstrCurrentFile = m_pDataManager->FindAudio(iNext);
		m_pAudio->SetCurrentAudio(wstrCurrentFile);
		m_pAudio->AudioControl(EGMA_CMD_OPEN);
		SGMStarCoord starCoord = m_pDataManager->GetStarCoord(wstrCurrentFile);
		float fWorldX, fWorldY;
		_StarCoord2World(starCoord, fWorldX, fWorldY);
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

/** @brief 下一首 */
bool CGMEngine::Next()
{
	_Next();
	return true;
}

void CGMEngine::SetMousePosition(const osg::Vec3f vPos)
{
	if (m_bHasWelcomed)
	{
		m_pGalaxy->SetMousePosition(vPos);
	}
}

osg::Vec3d CGMEngine::GetCurrentStarWorldPos()
{
	return m_pGalaxy->GetStarWorldPos();
}

/** @brief 创建视口管理器 */
osgViewer::Viewer* CGMEngine::CreateViewer()
{
	GM_Viewer->setSceneData(GM_Root.get());
	GM_Viewer->getCamera()->setClearColor(osg::Vec4(0.0, 0.02, 0.04, 1.0));
	return GM_Viewer.get();
}

void CGMEngine::_Next()
{
	switch (m_ePlayMode)
	{
	case EGMA_MOD_SINGLE:
	{
		m_pAudio->AudioControl(EGMA_CMD_PLAY);
	}
	break;
	case EGMA_MOD_CIRCLE:
	{
		m_pAudio->AudioControl(EGMA_CMD_CLOSE);
	}
	break;
	case EGMA_MOD_RANDOM:
	{
		m_pAudio->AudioControl(EGMA_CMD_CLOSE);

		int iMaxNum = m_pDataManager->GetAudioNum();
		m_iRandom.seed(time(0));
		int iNext = m_iRandom() % iMaxNum;
		std::wstring wstrCurrentFile = m_pDataManager->FindAudio(iNext);
		while (L"" == wstrCurrentFile)
		{
			iNext = m_iRandom() % iMaxNum;
			wstrCurrentFile = m_pDataManager->FindAudio(iNext);
		}
		m_pAudio->SetCurrentAudio(wstrCurrentFile);
		m_pAudio->AudioControl(EGMA_CMD_OPEN);
		m_pAudio->AudioControl(EGMA_CMD_PLAY);

		SGMStarCoord starCoord = m_pDataManager->GetStarCoord(wstrCurrentFile);
		float fWorldX, fWorldY;
		_StarCoord2World(starCoord, fWorldX, fWorldY);
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

void CGMEngine::_InnerUpdate(float updateStep)
{
	if (!m_bHasWelcomed)
	{
		m_pGalaxy->Welcome();
		m_pAudio->Welcome();
		m_bHasWelcomed = true;
	}

	if (m_pAudio->IsAudioOver())
	{
		_Next();
	}
}

bool CGMEngine::_World2GalaxyCoord(const float fX, const float fY, float & fGalaxyX, float & fGalaxyY)
{
	float fRadius = 0.5*m_fGalaxyDiameter;
	fGalaxyX = fX / fRadius;
	fGalaxyY = fY / fRadius;
	return true;
}

bool CGMEngine::_GalaxyCoord2World(const float fGalaxyX, const float fGalaxyY, float & fX, float & fY)
{
	float fRadius = 0.5*m_fGalaxyDiameter;
	fX = fGalaxyX * fRadius;
	fY = fGalaxyY * fRadius;
	return true;
}

bool CGMEngine::_StarCoord2World(const SGMStarCoord& starCoord, float & fX, float & fY)
{
	float fRadius = 0.5*m_fGalaxyDiameter;
	fX = fRadius * float(starCoord.x) / GM_COORD_MAX;
	fY = fRadius * float(starCoord.y) / GM_COORD_MAX;
	return true;
}
