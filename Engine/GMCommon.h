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

	#define GM_UNIT_SCALE				(1e5)		// ÿ���㼶�ռ�֮��ĳ߶ȱ���
	#define GM_HIERARCHY_MAX			(6)			// �������㼶
	#define GM_STAR_POS_MIN				(1e10)		// ��Ƶ������ռ��������С�ռ䵥λ
	#define GM_MIN_RADIUS				(5e-4)		// �����ÿ���㼶�ռ��е���С����
	#define GM_MAX_RADIUS				(GM_MIN_RADIUS*GM_UNIT_SCALE*2)	// �����ÿ���㼶�ռ��е���󽹾�

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
			, bHighQuality(false), bPhoto(false), fVolume(0.5f)
		{}

		std::string						strCorePath;			//!< ������Դ·��
		std::wstring					strMediaPath;			//!< �ⲿ��Դ·��
		bool							bHighQuality;			//!< �߻���ģʽ
		bool							bPhoto;					//!< ��Ƭģʽ����
		float							fVolume;				//!< ����
	};
}	// GM
