//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMUIManager.cpp
/// @brief		UI Manager ���������
/// @version	1.0
/// @author		LiuTao
/// @date		2021.08.14
//////////////////////////////////////////////////////////////////////////

#include "GMUIManager.h"
#include "../GMSystemManager.h"

#include <QFile>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QMenu>
#include <QtWidgets/QStyle>

#include "../Engine/GMEngine.h"

#include "GMMainWindow.h"

using namespace GM;

/*************************************************************************
Global Constants
*************************************************************************/


CGMUIManager::CGMUIManager():
	m_bInit(false), m_pMainWindow(nullptr)
{
}

CGMUIManager::~CGMUIManager()
{
}

CGMUIManager* CGMSingleton<CGMUIManager>::msSingleton = nullptr;

/** @brief ��ȡ���� */
CGMUIManager& CGMUIManager::getSingleton(void)
{
	if (!msSingleton)
		msSingleton = GM_NEW(CGMUIManager);
	assert(msSingleton);
	return (*msSingleton);
}

CGMUIManager* CGMUIManager::getSingletonPtr(void)
{
	if (!msSingleton)
		msSingleton = GM_NEW(CGMUIManager);
	assert(msSingleton);
	return msSingleton;
}

/** @brief ��ʼ�� */
void CGMUIManager::Init()
{
	if (!m_pMainWindow)
	{
		m_pMainWindow = new CGMMainWindow();
		m_pMainWindow->Init();
		m_pMainWindow->show();
	}

	m_bInit = true;
}

/** @brief �ͷ� */
bool CGMUIManager::Release()
{
	if (!m_bInit)
		return true;

	m_bInit = false;
	return true;
}

/** @brief ������С */
void CGMUIManager::Resize()
{
	if (!m_pMainWindow)
		return;

	int nScreenWidth = m_pMainWindow->width();
	int nScreenHeight = m_pMainWindow->height();
}

/** @brief ��ȡ�����С */
void CGMUIManager::GetScreenSize(int& nWidth, int& nHight)
{
	nWidth = 1920;
	nHight = 1080;
}

/** @brief ���� */
void CGMUIManager::Update()
{
	m_pMainWindow->Update();
}

void CGMUIManager::SetFullScreen(const bool bFull)
{
	m_pMainWindow->SetFullScreen(bFull);
}

bool CGMUIManager::GetFullScreen()
{
	return m_pMainWindow->GetFullScreen();
}

void CGMUIManager::UpdateAudioInfo()
{
	m_pMainWindow->UpdateAudioInfo();
}

void CGMUIManager::SetVolumeVisible(const bool bVisible)
{
	m_pMainWindow->SetVolumeVisible(bVisible);
}

void CGMUIManager::SetCursorVisible(bool bVisible)
{
	if (bVisible)
	{
		m_pMainWindow->setCursor(Qt::ArrowCursor);
	}
	else
	{
		m_pMainWindow->setCursor(Qt::BlankCursor);
	}
}
