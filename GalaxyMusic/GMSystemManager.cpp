//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMSystemManager.cpp
/// @brief		system Manager ϵͳ������
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

#define FRAME_UPDATE	30 	// ����֡����һ����Ϣ

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

/** @brief ��ȡ���� */
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

/** @brief ��ʼ�� */
bool CGMSystemManager::Init()
{
	if (m_bInit)
		return true;

	// ��ʼ������
	GM_ENGINE.Init();

	// ��ʼ������
	GM_UI_MANAGER.Init();

	// ������ʱ��
	startTimer(8);

	m_bInit = true;
	return true;
}

/** @brief �ͷ� */
bool CGMSystemManager::Release()
{
	if (!m_bInit)
		return true;

	GM_ENGINE.Release();
	m_bInit = false;
	return true;
}

/** @brief ���̰��������¼� */
bool CGMSystemManager::GMKeyDown(EGMKeyCode eKC)
{
	switch (eKC)
	{
	case EGMKeyCode::EGM_KC_F11:
	{
		GM_UI_MANAGER.SetFullScreen(!GM_UI_MANAGER.GetFullScreen());
	}
	break;
	case EGMKeyCode::EGM_KC_Escape:
	{
		GM_UI_MANAGER.SetFullScreen(false);
	}
	break;
	case EGMKeyCode::EGM_KC_F7:
	{
		if (GM_ENGINE.GetEditMode())
		{
			m_iRhythmCount = 0;
		}
	}
	break;
	case EGMKeyCode::EGM_KC_F8:
	{
		if (GM_ENGINE.GetEditMode())
		{
			if (m_iRhythmCount < 20 && m_iRhythmCount >= 0)
			{
				if (0 == m_iRhythmCount)
				{
					m_iTimeRhythmStart = QDateTime::currentDateTime().currentMSecsSinceEpoch()/1000;
				}
				m_iRhythmCount++;
			}
			else if(m_iRhythmCount >= 20)// ��20�����ģ����ڣ�
			{
				m_iTimeRhythmEnd = QDateTime::currentDateTime().currentMSecsSinceEpoch()/1000;
				double fBPM = 60 / ((m_iTimeRhythmEnd - m_iTimeRhythmStart) / 20);
				SGMAudioCoord vAudioCoord = GM_ENGINE.GetCurrentStarAudioCoord();
				vAudioCoord.BPM = fBPM;
				GM_ENGINE.SetCurrentStarAudioCoord(vAudioCoord);
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
		GM_UI_MANAGER.SetVolumeVisible(true);
		if (EGM_KC_Up == eKC)
		{
			GM_ENGINE.SetVolume(GM_ENGINE.GetVolume() + 0.057);
		}
		else
		{
			GM_ENGINE.SetVolume(GM_ENGINE.GetVolume() - 0.057);
		}
	}
	break;
	case EGMKeyCode::EGM_KC_Left:
	{
		GM_ENGINE.Last();
	}
	break;
	case EGMKeyCode::EGM_KC_Right:
	{
		GM_ENGINE.Next();
	}
	break;
	}
	return true;
}

/** @brief ���̰��������¼� */
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
	GM_UI_MANAGER.SetCursorVisible(bVisible);
}

/** @brief ��ʱ������ */
void CGMSystemManager::timerEvent(QTimerEvent *event)
{
	if (m_bFirst)
	{
		GM_ENGINE.Welcome();
		m_bFirst = false;
	}

	// �������
	m_iFrameCount++;
	if (FRAME_UPDATE <= m_iFrameCount)
	{
		if (GM_ENGINE.IsWelcomeFinished())
		{
			GM_UI_MANAGER.UpdateAudioInfo();
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
			GM_UI_MANAGER.SetVolumeVisible(false);
		}
	}

	_Render();
}

void CGMSystemManager::_Render()
{
	GM_ENGINE.Update();
	GM_UI_MANAGER.Update();
}