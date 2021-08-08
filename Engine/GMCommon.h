//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMStructs.h
/// @brief		Galaxy-Music Engine - Common
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.09
//////////////////////////////////////////////////////////////////////////
#pragma once
#include <Windows.h>
#include "GMStructs.h"

namespace GM
{ 
	/*************************************************************************
	 Macro Defines
	*************************************************************************/

	#define GM_WELCOME_MAX_TIME				(5.0)

	/*************************************************************************
	 Class
	*************************************************************************/


	/*************************************************************************
	 Struct
	*************************************************************************/
	/*!
	 *  @struct SGMConfigData
	 *  @brief ��������
	 */
	struct SGMConfigData
	{
		SGMConfigData()
			: strCorePath("../../Data/Core/"), strMediaPath(L"../../Data/Media/")
			, fFov(45.0f)
		{}

		std::string						strCorePath;			//!< ������Դ·��
		std::wstring					strMediaPath;			//!< �ⲿ��Դ·��
		float							fFov;					//!< ��ֱ�ӳ���
	};
}	// GM
