//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMStructs.h
/// @brief		Galaxy-Music Engine -Kernel
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.09
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <osg/Group>
#include "GMViewWidget.h"

namespace GM
{ 
	/*************************************************************************
	 Macro Defines
	*************************************************************************/
	#define GM_Root					m_pKernelData->vRoot
	#define GM_View					m_pKernelData->vView
	#define GM_Viewer				m_pKernelData->vViewer

	/*************************************************************************
	 Struct
	*************************************************************************/
	/*!
	 *  @struct SGMKernelData
	 *  @brief 内核数据
	 */
	struct SGMKernelData
	{
		bool										bInited;			//!< 是否初始化
		osg::ref_ptr<osg::Group>					vRoot;				//!< 根节点
		osg::ref_ptr<osgViewer::View>				vView;				//!< 视口
		osg::ref_ptr<CGMViewWidget>					vViewer;			//!< 视口管理器
		osg::ref_ptr<osg::Camera>					pBackgroundCam;		//!< 背景RTT相机
		osg::ref_ptr<osg::Camera>					pForegroundCam;		//!< 前景RTT相机

		int											iHierarchy;			//!< 当前空间层级编号
		osg::ref_ptr<osg::DoubleArray>				fUnitArray;			//!< 所有空间层级单位
		osg::ref_ptr<osg::Vec3dArray>				vEyePosArray;		//!< 眼点最后时刻的空间坐标
	};

	/*!
	*  @brief 渲染顺序
	*/
	constexpr auto BIN_COSMOS						= 10;
	constexpr auto BIN_BACKGROUND_GALAXY			= 20;
	constexpr auto BIN_BACKGROUND_SUN				= 30;
	constexpr auto BIN_ULTRACLUSTER					= 40;
	constexpr auto BIN_SUPERCLUSTER					= 50;
	constexpr auto BIN_GALAXIES						= 60;
	constexpr auto BIN_GALAXY_PLANE					= 70;
	constexpr auto BIN_MILKYWAY						= 80;
	constexpr auto BIN_STARS_0						= 90;
	constexpr auto BIN_STARS_1						= 91;
	constexpr auto BIN_AUDIO_POINT					= 100;
	constexpr auto BIN_REGION						= 110;
	constexpr auto BIN_PLANET_LINE					= 120;
	constexpr auto BIN_PLANETS						= 121;
	constexpr auto BIN_SUN_BLOOM					= 130;
	constexpr auto BIN_STAR_PLAYING					= 140;
	constexpr auto BIN_HANDLE						= 150;
	constexpr auto BIN_RING_BACK					= 160;
	constexpr auto BIN_ATMOSPHERE					= 170;
	constexpr auto BIN_ATMOS_TAIL					= 171;
	constexpr auto BIN_ROCKSPHERE					= 180;
	constexpr auto BIN_CLOUD						= 190;
	constexpr auto BIN_PLANET_JET					= 200;
	constexpr auto BIN_PLANET_POINT					= 210;
	constexpr auto BIN_RING_FRONT					= 220;
	constexpr auto BIN_PLANET_TAIL					= 300;
}	// GM
