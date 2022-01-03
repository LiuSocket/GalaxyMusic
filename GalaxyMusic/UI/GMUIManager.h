//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMUIManager.h
/// @brief		UI Manager ���������
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
*  @brief ���������
*/ 
class CGMUIManager : public QObject, public GM::CGMSingleton<CGMUIManager>
{
	Q_OBJECT

public:
	CGMUIManager();
	~CGMUIManager();

	/** @brief ��ȡ���� */
	static CGMUIManager& getSingleton(void);
	static CGMUIManager* getSingletonPtr(void);

	/** @brief ��ʼ�� */
	void Init();
	/** @brief �ͷ� */
	bool Release();
	/** @brief ������С */
	void Resize();

	/** @brief ��ȡ�����С */
	void GetScreenSize(int& nWidth,int& nHight);
	
	/** @brief ���� */
	void Update();

	/**
	* SetFullScreen
	* @brief ����ȫ���л�
	* @author LiuTao
	* @since 2021.09.05
	* @param bFull trueΪȫ����false Ϊ��ȫ��
	* @return void
	*/
	void SetFullScreen(const bool bFull);
	/**
	* GetFullScreen
	* @brief ��ȡ����ȫ��״̬
	* @author LiuTao
	* @since 2021.10.23
	* @return bool		�Ƿ���ȫ��״̬
	*/
	bool GetFullScreen();

	/**
	* UpdateAudioInfo
	* ������Ƶ��������Ϣ
	* @author LiuTao
	* @since 2021.09.05
	* @return void
	*/
	void UpdateAudioInfo();

	/**
	* SetCursorVisible
	* ���������ʾ/����
	* @author LiuTao
	* @since 2021.09.11
	* @param bVisible:			����Ƿ���ʾ
	* @return void
	*/
	void SetCursorVisible(bool bVisible);

private:

signals:


private slots:



private:
	bool											m_bInit;						//!< ��ʼ����ʶ
	CGMMainWindow*									m_pMainWindow;					//!< ������
};