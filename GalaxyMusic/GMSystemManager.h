//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMSystemManager.h
/// @brief		system Manager ϵͳ������
/// @version	1.0
/// @author		LiuTao
/// @date		2021.08.14
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <windows.h>
#include <QObject>
#include "../Engine/GMPrerequisites.h"
#include "../Engine/GMEngine.h"

/*************************************************************************
Macro Defines
*************************************************************************/
#define GM_SYSTEM_MANAGER					CGMSystemManager::getSingleton()
#define GM_SYSTEM_MANAGER_PTR				CGMSystemManager::getSingletonPtr()

/*************************************************************************
Enums
*************************************************************************/


/*************************************************************************
Struct
*************************************************************************/


/*************************************************************************
Class
*************************************************************************/
/*!
*  @class CGMSystemManager
*  @brief ϵͳ������
*/
class CGMSystemManager : public QObject, public GM::CGMSingleton<CGMSystemManager>
{
	Q_OBJECT

public:
	CGMSystemManager();
	~CGMSystemManager();

	/** @brief ��ʼ�� */
	bool Init();
	/** @brief �ͷ� */
	bool Release();

	/** @brief ���̰��������¼� */
	bool GMKeyDown(GM::EGMKeyCode eKC);
	/** @brief ���̰��������¼� */
	bool GMKeyUp(GM::EGMKeyCode eKC);

	/**
	* SetCursorVisible
	* ���������ʾ/����
	* @author LiuTao
	* @since 2021.09.11
	* @param bVisible:			����Ƿ���ʾ
	* @return void
	*/
	void SetCursorVisible(bool bVisible);

protected:
	/** @brief ��ʱ������ */
	void timerEvent(QTimerEvent *event);

private:
	/** @brief ��Ⱦ���� */
	void _Render();

public:
	/** @brief ��ȡ���� */
	static CGMSystemManager& getSingleton(void);
	static CGMSystemManager* getSingletonPtr(void);

private:
	bool							m_bInit;				//!< ��ʼ����ʶ
	bool							m_bFirst;				//!< �Ƿ��һ֡

	bool							m_bVolumeHiding;		//!< �Ƿ�ʼ���������ؼ�
	int								m_iVolumeCount;			//!< ���������ؼ����ص�֡��������

	int								m_iFrameCount;			//!< ֡����������30֡һѭ��
	int								m_iRhythmCount;			//!< ���ļ�������20һѭ��
	qint64							m_iTimeRhythmStart;		//!< ���ļ�����ʼʱ�䣬��λ��s
	qint64							m_iTimeRhythmEnd;		//!< ���ļ�������ʱ�䣬��λ��s

	uint							m_nKeyMask;
};