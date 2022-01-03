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

	#define GM_UNIT_SCALE				(1e5)		// 每个层级空间之间的尺度比例
	#define GM_HIERARCHY_MAX			(6)			// 宇宙最大层级
	#define GM_STAR_POS_MIN				(1e10)		// 音频星世界空间坐标的最小空间单位
	#define GM_MIN_RADIUS				(5e-4)		// 相机在每个层级空间中的最小焦距
	#define GM_MAX_RADIUS				(GM_MIN_RADIUS*GM_UNIT_SCALE*2)	// 相机在每个层级空间中的最大焦距

	/*************************************************************************
	 Class
	*************************************************************************/


	/*************************************************************************
	 Struct
	*************************************************************************/
	/*!
	 *  @struct SGMConfigData
	 *  @brief 配置数据
	 */
	struct SGMConfigData
	{
		SGMConfigData()
			: strCorePath("../../Data/Core/"), strMediaPath(L"../../Data/Media/")
			, bHighQuality(false), bPhoto(false), fVolume(0.5f)
		{}

		std::string						strCorePath;			//!< 核心资源路径
		std::wstring					strMediaPath;			//!< 外部资源路径
		bool							bHighQuality;			//!< 高画质模式
		bool							bPhoto;					//!< 照片模式开关
		float							fVolume;				//!< 音量
	};
}	// GM
