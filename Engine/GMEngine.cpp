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
#include "GMGalaxy.h"
#include "GMAudio.h"
#include "GMCameraManipulator.h"
#include <osgViewer/ViewerEventHandlers>
#include <osgQt/GraphicsWindowQt>
#include <QtCore/QTimer>

#include <iostream>

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
	m_pKernelData(nullptr), m_pConfigData(nullptr), m_pDataManager(nullptr), m_pManipulator(nullptr),
	m_bInit(false), m_bDirty(false),
	m_dTimeLastFrame(0.0), m_fDeltaStep(0.0f), m_fConstantStep(0.1f),
	m_fGalaxyDiameter(1e21),
	m_pGalaxy(nullptr), m_pAudio(nullptr), m_ePlayMode(EGMA_MOD_CIRCLE)
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
	//!< 当前空间层级编号默认4级，这样刚好看到银河系
	m_pKernelData->iHierarchy = 4;
	//!< 空间层级单位，不能修改，0级是整个宇宙，6级是人类尺度，级数编号越大，级数单位越小
	m_pKernelData->fUnitArray = new osg::DoubleArray;
	m_pKernelData->fUnitArray->push_back(1.0);	// 0级空间层级单位
	m_pKernelData->fUnitArray->push_back(1e5);	// 1级空间层级单位
	m_pKernelData->fUnitArray->push_back(1e10);	// 2级空间层级单位
	m_pKernelData->fUnitArray->push_back(1e15);	// 3级空间层级单位
	m_pKernelData->fUnitArray->push_back(1e20);	// 4级空间层级单位
	m_pKernelData->fUnitArray->push_back(1e25);	// 5级空间层级单位
	m_pKernelData->fUnitArray->push_back(1e30);	// 6级空间层级单位
	//!< 眼点最后时刻的空间坐标，默认都在(1,1,1)位置
	m_pKernelData->vEyePosArray = new osg::Vec3dArray;
	m_pKernelData->vEyePosArray->push_back(osg::Vec3d(1, 1, 1));	// 0级空间
	m_pKernelData->vEyePosArray->push_back(osg::Vec3d(1, 1, 1));	// 1级空间
	m_pKernelData->vEyePosArray->push_back(osg::Vec3d(1, 1, 1));	// 2级空间
	m_pKernelData->vEyePosArray->push_back(osg::Vec3d(1, 1, 1));	// 3级空间
	m_pKernelData->vEyePosArray->push_back(osg::Vec3d(1, 1, 1));	// 4级空间
	m_pKernelData->vEyePosArray->push_back(osg::Vec3d(1, 1, 1));	// 5级空间
	m_pKernelData->vEyePosArray->push_back(osg::Vec3d(1, 1, 1));	// 6级空间
	//!< 目标点最后时刻的空间坐标，默认都在(0,0,0)位置
	m_pKernelData->vTargetPosArray = new osg::Vec3dArray;
	m_pKernelData->vTargetPosArray->push_back(osg::Vec3d(0, 0, 0));	// 0级空间
	m_pKernelData->vTargetPosArray->push_back(osg::Vec3d(0, 0, 0));	// 1级空间
	m_pKernelData->vTargetPosArray->push_back(osg::Vec3d(0, 0, 0));	// 2级空间
	m_pKernelData->vTargetPosArray->push_back(osg::Vec3d(0, 0, 0));	// 3级空间
	m_pKernelData->vTargetPosArray->push_back(osg::Vec3d(0, 0, 0));	// 4级空间
	m_pKernelData->vTargetPosArray->push_back(osg::Vec3d(0, 0, 0));	// 5级空间
	m_pKernelData->vTargetPosArray->push_back(osg::Vec3d(0, 0, 0));	// 6级空间



	GM_Root = new osg::Group();
	GM_View = new osgViewer::View();
	GM_View->setSceneData(GM_Root.get());

	//!< 配置数据
	m_pConfigData = new SGMConfigData();
	m_pManipulator = new CGMCameraManipulator();
	m_pDataManager = new CGMDataManager();
	m_pGalaxy = new CGMGalaxy();
	m_pAudio = new CGMAudio();

	m_pDataManager->Init(m_pConfigData);
	m_pGalaxy->Init(m_pKernelData, m_pConfigData);
	m_pAudio->Init(m_pConfigData);

	m_pGalaxy->CreateGalaxy(m_fGalaxyDiameter);

	GM_View->setCameraManipulator(m_pManipulator);
	//状态信息
	osgViewer::StatsHandler* pStatsHandler = new osgViewer::StatsHandler;
	pStatsHandler->setKeyEventTogglesOnScreenStats(osgGA::GUIEventAdapter::KEY_F1);
	GM_View->addEventHandler(pStatsHandler);

	m_bInit = true;
	m_pKernelData->bInited = m_bInit;

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

		_UpdateScenes();

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

	std::wstring wstrName = m_pDataManager->FindAudio(fGalaxyX, fGalaxyY, fGalaxyZ);
	if (L"" != wstrName)
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
	if (L"" == m_pAudio->GetCurrentAudio())
	{
		// 说明是第一首歌
		if (m_pGalaxy->GetEditMode())
		{
			_Next(EGMA_MOD_SINGLE);
		}
		else
		{
			_Next(m_ePlayMode);
		}
	}
	else
	{
		m_pAudio->AudioControl(EGMA_CMD_PLAY);
	}

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
	std::wstring wstrCurrentFile = m_pDataManager->FindLastAudio();
	if (L"" == wstrCurrentFile) return false;

	m_pAudio->AudioControl(EGMA_CMD_CLOSE);
	m_pAudio->SetCurrentAudio(wstrCurrentFile);
	m_pAudio->AudioControl(EGMA_CMD_OPEN);
	m_pAudio->AudioControl(EGMA_CMD_PLAY);

	SGMStarCoord starCoord = m_pDataManager->GetStarCoord(wstrCurrentFile);
	double fWorldX, fWorldY;
	_StarCoord2World(starCoord, fWorldX, fWorldY);
	m_pGalaxy->SetCurrentStar(osg::Vec3f(fWorldX, fWorldY, 0.0f), wstrCurrentFile);

	return true;
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

bool CGMEngine::SetVolume(float fVolume)
{
	return m_pAudio->SetVolume(osg::clampBetween(fVolume, 0.0f, 1.0f));
}

float CGMEngine::GetVolume()
{
	return m_pAudio->GetVolume();
}

bool CGMEngine::SetPlayMode(EGMA_MODE eMode)
{
	m_ePlayMode = eMode;
	return true;
}

EGMA_MODE CGMEngine::GetPlayMode()
{
	return m_ePlayMode;
}

std::wstring CGMEngine::GetAudioName()
{
	return m_pAudio->GetCurrentAudio();
}

bool CGMEngine::SetAudioCurrentTime(int iTime)
{	
	return m_pAudio->SetAudioCurrentTime(iTime);
}

int CGMEngine::GetAudioCurrentTime()
{
	return m_pAudio->GetAudioCurrentTime();
}

int CGMEngine::GetAudioDuration()
{
	return m_pAudio->GetAudioDuration();
}

void CGMEngine::Welcome()
{
	m_pGalaxy->Welcome();
	m_pAudio->Welcome();
}

bool CGMEngine::IsWelcomeFinished()
{
	return m_pAudio->IsWelcomeFinished();
}

void CGMEngine::SetMousePosition(const osg::Vec3f& vHierarchyPos)
{
	m_pGalaxy->SetMousePosition(vHierarchyPos);
}

osg::Vec3d CGMEngine::GetCurrentStarWorldPos()
{
	return m_pGalaxy->GetStarWorldPos();
}

double CGMEngine::GetGalaxyRadius()
{
	return m_fGalaxyDiameter*0.5;
}

double CGMEngine::GetHierarchyTargetDistance()
{
	return m_pManipulator->GetHierarchyTargetDistance();
}

double CGMEngine::GetUnit(int iHierarchy)
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

osg::Vec3d CGMEngine::GetHierarchyLastEyePos(int iHierarchy)
{
	int iHie = (-1 == iHierarchy) ? m_pKernelData->iHierarchy : osg::clampBetween(iHierarchy, 0, 6);
	return m_pKernelData->vEyePosArray->at(iHie);
}

osg::Vec3d CGMEngine::GetHierarchyLastTargetPos(int iHierarchy)
{
	int iHie = (-1 == iHierarchy) ? m_pKernelData->iHierarchy : osg::clampBetween(iHierarchy, 0, 6);
	return m_pKernelData->vTargetPosArray->at(iHie);
}

bool CGMEngine::AddHierarchy(const osg::Vec3d& vHierarchyEyePos, const osg::Vec3d& vHierarchyTargetPos)
{
	std::cout << "iHierarchy：" << m_pKernelData->iHierarchy << " ++" << std::endl;

	if (GM_HIERARCHY_MAX == m_pKernelData->iHierarchy)
	{
		return false;
	}
	else
	{
		// 记录当前层级眼点空间坐标
		m_pKernelData->vEyePosArray->at(m_pKernelData->iHierarchy) = vHierarchyEyePos;
		// 记录当前层级目标点空间坐标
		m_pKernelData->vTargetPosArray->at(m_pKernelData->iHierarchy) = vHierarchyTargetPos;

		m_pKernelData->iHierarchy++;
		int iHieNew = m_pKernelData->iHierarchy;

		if (4 < iHieNew)
		{
			// 计算升级后空间的眼点空间坐标
			m_pKernelData->vEyePosArray->at(iHieNew) = vHierarchyEyePos / GM_UNIT_SCALE;
			// 计算升级后空间的目标点空间坐标
			m_pKernelData->vTargetPosArray->at(iHieNew) = vHierarchyTargetPos / GM_UNIT_SCALE;

		}
		else
		{
			// 取出升级后空间的目标点空间坐标
			osg::Vec3d vTargetOld = m_pKernelData->vTargetPosArray->at(iHieNew);
			// 计算升级后空间的眼点空间坐标
			m_pKernelData->vEyePosArray->at(iHieNew) = vTargetOld + vHierarchyEyePos / GM_UNIT_SCALE;
			// 计算升级后空间的目标点空间坐标
			m_pKernelData->vTargetPosArray->at(iHieNew) = vTargetOld + vHierarchyTargetPos / GM_UNIT_SCALE;
		}

		// 激活脏标记
		m_bDirty = true;
		return true;
	}
}

bool CGMEngine::SubHierarchy(const osg::Vec3d& vHierarchyEyePos, const osg::Vec3d& vHierarchyTargetPos)
{
	std::cout << "iHierarchy：" << m_pKernelData->iHierarchy << " --" << std::endl;

	if (0 == m_pKernelData->iHierarchy)
	{
		return false;
	}
	else
	{
		// 记录当前层级眼点空间坐标
		m_pKernelData->vEyePosArray->at(m_pKernelData->iHierarchy) = vHierarchyEyePos;
		// 记录当前层级目标点空间坐标
		m_pKernelData->vTargetPosArray->at(m_pKernelData->iHierarchy) = vHierarchyTargetPos;

		m_pKernelData->iHierarchy--;
		int iHieNew = m_pKernelData->iHierarchy;

		if (4 <= iHieNew)
		{
			// 计算降级后空间的目标点空间坐标
			m_pKernelData->vTargetPosArray->at(iHieNew) = vHierarchyTargetPos * GM_UNIT_SCALE;
			// 计算降级后空间的眼点空间坐标
			m_pKernelData->vEyePosArray->at(iHieNew) = vHierarchyEyePos * GM_UNIT_SCALE;
		}
		else
		{
			// 计算降级后空间的目标点空间坐标
			m_pKernelData->vTargetPosArray->at(iHieNew) = osg::Vec3d(0, 0, 0);
			// 计算降级后空间的眼点空间坐标
			m_pKernelData->vEyePosArray->at(iHieNew) = (vHierarchyEyePos - vHierarchyTargetPos) * GM_UNIT_SCALE;
		}

		// 激活脏标记
		m_bDirty = true;
		return true;
	}
}

osg::Vec3d CGMEngine::Hierarchy2World(osg::Vec3d vHierarchy)
{
	osg::Vec3d vWorldPos = vHierarchy * m_pKernelData->fUnitArray->at(m_pKernelData->iHierarchy);
	//456层级的坐标原点都在银河系中心，所以不需要偏移，直接除以单位长度
	//0123层级的坐标原点可以任意变动，需要乘以单位长度后叠加
	for (int i = 4; i > m_pKernelData->iHierarchy; i--)
	{
		vWorldPos += m_pKernelData->vTargetPosArray->at(i) * m_pKernelData->fUnitArray->at(i);
	}
	return vWorldPos;
}

osg::Vec3d CGMEngine::StarWorld2Hierarchy(osg::Vec3d vStarWorldPos)
{
	osg::Vec3d vHierarchyCenterWorldPos = osg::Vec3d(0, 0, 0);
	//456层级的坐标原点都在银河系中心，所以直接用世界坐标除以单位长度
	//0123层级的坐标原点可以任意变动，需要减去坐标原点，再除以单位长度
	for (int i = 4; i > m_pKernelData->iHierarchy; i--)
	{
		vHierarchyCenterWorldPos += m_pKernelData->vTargetPosArray->at(i)* m_pKernelData->fUnitArray->at(i);
	}
	return (vStarWorldPos - vHierarchyCenterWorldPos) / m_pKernelData->fUnitArray->at(m_pKernelData->iHierarchy);
}

CGMViewWidget* CGMEngine::CreateViewWidget(QWidget* parent)
{
	GM_Viewer = new CGMViewWidget(GM_View, parent);
	return GM_Viewer.get();
}

void CGMEngine::_Next(EGMA_MODE eMode)
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
		int iNext = (m_pDataManager->GetCurrentAudioID() + 1) % iMaxNum;
		wstrCurrentFile = m_pDataManager->FindAudio(iNext);

		m_pAudio->SetCurrentAudio(wstrCurrentFile);
		m_pAudio->AudioControl(EGMA_CMD_OPEN);
		m_pAudio->AudioControl(EGMA_CMD_PLAY);

		SGMStarCoord starCoord = m_pDataManager->GetStarCoord(wstrCurrentFile);
		double fWorldX, fWorldY;
		_StarCoord2World(starCoord, fWorldX, fWorldY);
		m_pGalaxy->SetCurrentStar(osg::Vec3f(fWorldX, fWorldY, 0.0f), wstrCurrentFile);
	}
	break;
	case EGMA_MOD_RANDOM:
	{
		m_pAudio->AudioControl(EGMA_CMD_CLOSE);

		int iMaxNum = m_pDataManager->GetAudioNum();
		m_iRandom.seed(time(0));
		int iNext = m_iRandom() % iMaxNum;
		wstrCurrentFile = m_pDataManager->FindAudio(iNext);
		while (L"" == wstrCurrentFile)
		{
			iNext = m_iRandom() % iMaxNum;
			wstrCurrentFile = m_pDataManager->FindAudio(iNext);
		}
		m_pAudio->SetCurrentAudio(wstrCurrentFile);
		m_pAudio->AudioControl(EGMA_CMD_OPEN);
		m_pAudio->AudioControl(EGMA_CMD_PLAY);

		SGMStarCoord starCoord = m_pDataManager->GetStarCoord(wstrCurrentFile);
		double fWorldX, fWorldY;
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
	if (m_pAudio->IsAudioOver())
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
}

bool CGMEngine::_World2GalaxyCoord(const double fX, const double fY, double & fGalaxyX, double & fGalaxyY)
{
	double fRadius = 0.5*m_fGalaxyDiameter;
	fGalaxyX = fX / fRadius;
	fGalaxyY = fY / fRadius;
	return true;
}

bool CGMEngine::_GalaxyCoord2World(const double fGalaxyX, const double fGalaxyY, double & fX, double & fY)
{
	double fRadius = 0.5*m_fGalaxyDiameter;
	fX = fGalaxyX * fRadius;
	fY = fGalaxyY * fRadius;
	return true;
}

bool CGMEngine::_StarCoord2World(const SGMStarCoord& starCoord, double & fX, double & fY)
{
	double fRadius = 0.5*m_fGalaxyDiameter;
	fX = fRadius * double(starCoord.x) / GM_COORD_MAX;
	fY = fRadius * double(starCoord.y) / GM_COORD_MAX;
	return true;
}

void CGMEngine::_UpdateScenes()
{
	if (!m_bDirty) return;

	m_pGalaxy->UpdateHierarchy(m_pKernelData->iHierarchy);

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

	m_bDirty = false;
}
