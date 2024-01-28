//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMSystemManager.h
/// @brief		system Manager 系统管理器
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
*  @brief 系统管理器
*/
class CGMSystemManager : public QObject, public GM::CGMSingleton<CGMSystemManager>
{
	Q_OBJECT

public:
	CGMSystemManager();
	~CGMSystemManager();

	/** @brief 初始化 */
	bool Init();
	/** @brief 释放 */
	bool Release();

	/** @brief 键盘按键按下事件 */
	bool GMKeyDown(GM::EGMKeyCode eKC);
	/** @brief 键盘按键弹起事件 */
	bool GMKeyUp(GM::EGMKeyCode eKC);

	/**
	* SetCursorVisible
	* 设置鼠标显示/隐藏
	* @author LiuTao
	* @since 2021.09.11
	* @param bVisible:			鼠标是否显示
	* @return void
	*/
	void SetCursorVisible(bool bVisible);

protected:
	/** @brief 定时器更新 */
	void timerEvent(QTimerEvent *event);

private:
	/** @brief 渲染更新 */
	void _Render();

public:
	/** @brief 获取单例 */
	static CGMSystemManager& getSingleton(void);
	static CGMSystemManager* getSingletonPtr(void);

private:
	bool							m_bInit;				//!< 初始化标识
	bool							m_bFirst;				//!< 是否第一帧

	bool							m_bVolumeHiding;		//!< 是否开始隐藏音量控件
	int								m_iVolumeCount;			//!< 控制音量控件隐藏的帧数计数器

	int								m_iFrameCount;			//!< 帧数计数器，30帧一循环
	int								m_iRhythmCount;			//!< 节拍计数器，20一循环
	qint64							m_iTimeRhythmStart;		//!< 节拍计数开始时间，单位：s
	qint64							m_iTimeRhythmEnd;		//!< 节拍计数结束时间，单位：s

	uint							m_nKeyMask;
};