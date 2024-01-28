//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMSystemManager.cpp
/// @brief		system Manager 系统管理器
/// @version	1.0
/// @author		LiuTao
/// @date		2021.08.14
//////////////////////////////////////////////////////////////////////////

#include "GMSystemManager.h"
#include "UI/GMUIManager.h"

#include <thread>
#include <QDesktopWidget>
#include <QApplication>
#include <QSettings>
#include <QTextCodec>
#include <QDatetime.h>
#include <QKeyEvent>

using namespace GM;

/*************************************************************************
Macro Defines
*************************************************************************/

#define FRAME_UPDATE	30 	// 多少帧更新一次信息

/*************************************************************************
CGMSystemManager Methods
*************************************************************************/

CGMSystemManager::CGMSystemManager()
	: m_bInit(false), m_bFirst(true),
	m_bVolumeHiding(false), m_iVolumeCount(0),
	m_iFrameCount(0), m_iRhythmCount(0),
	m_iTimeRhythmStart(0), m_iTimeRhythmEnd(20), m_nKeyMask(0)
{
}

CGMSystemManager::~CGMSystemManager()
{

}

CGMSystemManager* CGMSingleton<CGMSystemManager>::msSingleton = nullptr;

/** @brief 获取单例 */
CGMSystemManager& CGMSystemManager::getSingleton(void)
{
	if (!msSingleton)
		msSingleton = GM_NEW(CGMSystemManager);
	assert(msSingleton);
	return (*msSingleton);
}

CGMSystemManager* CGMSystemManager::getSingletonPtr(void)
{
	if (!msSingleton)
		msSingleton = GM_NEW(CGMSystemManager);
	assert(msSingleton);
	return msSingleton;
}

/** @brief 初始化 */
bool CGMSystemManager::Init()
{
	if (m_bInit)
		return true;

	// 初始化引擎
	GM_ENGINE_PTR->Init();

	// 初始化界面
	GM_UI_MANAGER_PTR->Init();

	// 启动定时器
	startTimer(8);

	m_bInit = true;
	return true;
}

/** @brief 释放 */
bool CGMSystemManager::Release()
{
	if (!m_bInit)
		return true;

	GM_ENGINE_PTR->Release();
	m_bInit = false;
	return true;
}

/** @brief 键盘按键按下事件 */
bool CGMSystemManager::GMKeyDown(EGMKeyCode eKC)
{
	switch (eKC)
	{
	case EGMKeyCode::EGM_KC_F11:
	{
		GM_UI_MANAGER_PTR->SetFullScreen(!GM_UI_MANAGER_PTR->GetFullScreen());
	}
	break;
	case EGMKeyCode::EGM_KC_Escape:
	{
		GM_UI_MANAGER_PTR->SetFullScreen(false);
	}
	break;
	case EGMKeyCode::EGM_KC_F7:
	{
		if (GM_ENGINE_PTR->GetEditMode())
		{
			m_iRhythmCount = 0;
		}
	}
	break;
	case EGMKeyCode::EGM_KC_F8:
	{
		if (GM_ENGINE_PTR->GetEditMode())
		{
			if (m_iRhythmCount < 20 && m_iRhythmCount >= 0)
			{
				if (0 == m_iRhythmCount)
				{
					m_iTimeRhythmStart = QDateTime::currentDateTime().currentMSecsSinceEpoch()/1000;
				}
				m_iRhythmCount++;
			}
			else if(m_iRhythmCount >= 20)// 满20个节拍（周期）
			{
				m_iTimeRhythmEnd = QDateTime::currentDateTime().currentMSecsSinceEpoch()/1000;
				double fBPM = 60 / ((m_iTimeRhythmEnd - m_iTimeRhythmStart) / 20);
				SGMAudioCoord vAudioCoord = GM_ENGINE_PTR->GetCurrentStarAudioCoord();
				vAudioCoord.BPM = fBPM;
				GM_ENGINE_PTR->SetCurrentStarAudioCoord(vAudioCoord);
				m_iRhythmCount = -1;
			}		
		}
	}
	break;
	case EGMKeyCode::EGM_KC_Up:
	case EGMKeyCode::EGM_KC_Down:
	{
		m_bVolumeHiding = false;
		m_iVolumeCount = 0;
		GM_UI_MANAGER_PTR->SetVolumeVisible(true);
		if (EGM_KC_Up == eKC)
		{
			GM_ENGINE_PTR->SetVolume(GM_ENGINE_PTR->GetVolume() + 0.057);
		}
		else
		{
			GM_ENGINE_PTR->SetVolume(GM_ENGINE_PTR->GetVolume() - 0.057);
		}
	}
	break;
	case EGMKeyCode::EGM_KC_Left:
	{
		GM_ENGINE_PTR->Last();
	}
	break;
	case EGMKeyCode::EGM_KC_Right:
	{
		GM_ENGINE_PTR->Next();
	}
	break;
	}
	return true;
}

/** @brief 键盘按键弹起事件 */
bool CGMSystemManager::GMKeyUp(EGMKeyCode eKC)
{
	switch (eKC)
	{
	case EGMKeyCode::EGM_KC_Up:
	case EGMKeyCode::EGM_KC_Down:
	{
		m_bVolumeHiding = true;
	}
	break;
	}
	return true;
}

void CGMSystemManager::SetCursorVisible(bool bVisible)
{
	GM_UI_MANAGER_PTR->SetCursorVisible(bVisible);
}

/** @brief 定时器更新 */
void CGMSystemManager::timerEvent(QTimerEvent *event)
{
	if (m_bFirst)
	{
		GM_ENGINE_PTR->Welcome();
		m_bFirst = false;
	}

	// 间隔更新
	m_iFrameCount++;
	if (FRAME_UPDATE <= m_iFrameCount)
	{
		if (GM_ENGINE_PTR->IsWelcomeFinished())
		{
			GM_UI_MANAGER_PTR->UpdateAudioInfo();
		}

		m_iFrameCount = 0;
	}

	if (m_bVolumeHiding)
	{
		m_iVolumeCount++;
		if (60 <= m_iVolumeCount)
		{
			m_iVolumeCount = 0;
			m_bVolumeHiding = false;
			GM_UI_MANAGER_PTR->SetVolumeVisible(false);
		}
	}

	_Render();
}

void CGMSystemManager::_Render()
{
	GM_ENGINE_PTR->Update();
	GM_UI_MANAGER_PTR->Update();
}