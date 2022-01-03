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

		int											iHierarchy;			//!< ��ǰ�ռ�㼶���
		osg::ref_ptr<osg::DoubleArray>				fUnitArray;			//!< ���пռ�㼶��λ
		osg::ref_ptr<osg::Vec3dArray>				vEyePosArray;		//!< �۵����ʱ�̵Ŀռ�����
		osg::ref_ptr<osg::Vec3dArray>				vTargetPosArray;	//!< Ŀ������ʱ�̵Ŀռ�����
	};

	/*!
	*  @brief ��Ⱦ˳��
	*/
	constexpr auto BIN_COSMOS						= 89;
	constexpr auto BIN_GALAXY_BACKGROUND			= 90;
	constexpr auto BIN_ULTRACLUSTER					= 91;
	constexpr auto BIN_SUPERCLUSTER					= 92;
	constexpr auto BIN_GALAXIES						= 93;
	constexpr auto BIN_NEBULA						= 94;
	constexpr auto BIN_GALAXY_PLANE					= 95;
	constexpr auto BIN_STARS						= 96;
	constexpr auto BIN_AUDIO_POINT					= 97;
	constexpr auto BIN_HELP_LINE					= 98;
	constexpr auto BIN_PLANET_LINE					= 99;
	constexpr auto BIN_PLANETS						= 100;
	constexpr auto BIN_SUN_BLOOM					= 101;
	constexpr auto BIN_STAR_PLAYING					= 102;
	constexpr auto BIN_SUPERNOVA					= 103;
}	// GM
