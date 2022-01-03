//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMUIManager.h
/// @brief		UI Manager 界面管理器
/// @version	1.0
/// @author		LiuTao
/// @date		2021.08.14
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <QObject>

#include "../Engine/GMPrerequisites.h"
#include "../Engine/GMStructs.h"

/*************************************************************************
Macro Defines
*************************************************************************/
#define GM_UI_MANAGER					CGMUIManager::getSingleton()
#define GM_UI_MANAGER_PTR				CGMUIManager::getSingletonPtr()

/*************************************************************************
Struct
*************************************************************************/

/*************************************************************************
Class
*************************************************************************/
class QWidget;
class QMenu;
class QAction;

class CGMMainWindow;

/*!
*  @class CGMUIManager
*  @brief 界面管理器
*/ 
class CGMUIManager : public QObject, public GM::CGMSingleton<CGMUIManager>
{
	Q_OBJECT

public:
	CGMUIManager();
	~CGMUIManager();

	/** @brief 获取单例 */
	static CGMUIManager& getSingleton(void);
	static CGMUIManager* getSingletonPtr(void);

	/** @brief 初始化 */
	void Init();
	/** @brief 释放 */
	bool Release();
	/** @brief 调整大小 */
	void Resize();

	/** @brief 获取界面大小 */
	void GetScreenSize(int& nWidth,int& nHight);
	
	/** @brief 更新 */
	void Update();

	/**
	* SetFullScreen
	* @brief 界面全屏切换
	* @author LiuTao
	* @since 2021.09.05
	* @param bFull true为全屏，false 为非全屏
	* @return void
	*/
	void SetFullScreen(const bool bFull);
	/**
	* GetFullScreen
	* @brief 获取界面全屏状态
	* @author LiuTao
	* @since 2021.10.23
	* @return bool		是否处于全屏状态
	*/
	bool GetFullScreen();

	/**
	* UpdateAudioInfo
	* 更新音频的所有信息
	* @author LiuTao
	* @since 2021.09.05
	* @return void
	*/
	void UpdateAudioInfo();

	/**
	* SetCursorVisible
	* 设置鼠标显示/隐藏
	* @author LiuTao
	* @since 2021.09.11
	* @param bVisible:			鼠标是否显示
	* @return void
	*/
	void SetCursorVisible(bool bVisible);

private:

signals:


private slots:



private:
	bool											m_bInit;						//!< 初始化标识
	CGMMainWindow*									m_pMainWindow;					//!< 主界面
};