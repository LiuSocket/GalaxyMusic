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
	 *  @brief �ں�����
	 */
	struct SGMKernelData
	{
		bool										bInited;			//!< �Ƿ��ʼ��
		osg::ref_ptr<osg::Group>					vRoot;				//!< ���ڵ�
		osg::ref_ptr<osgViewer::View>				vView;				//!< �ӿ�
		osg::ref_ptr<CGMViewWidget>					vViewer;			//!< �ӿڹ�����
		osg::ref_ptr<osg::Camera>					pBackgroundCam;		//!< ����RTT���
		osg::ref_ptr<osg::Camera>					pForegroundCam;		//!< ǰ��RTT���

		int											iHierarchy;			//!< ��ǰ�ռ�㼶���
		osg::ref_ptr<osg::DoubleArray>				fUnitArray;			//!< ���пռ�㼶��λ
		osg::ref_ptr<osg::Vec3dArray>				vEyePosArray;		//!< �۵����ʱ�̵Ŀռ�����
	};

	/*!
	*  @brief ��Ⱦ˳��
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
