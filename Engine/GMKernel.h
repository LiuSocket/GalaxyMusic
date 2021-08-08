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
#include <osgViewer/Viewer>

namespace GM
{ 
	/*************************************************************************
	 Macro Defines
	*************************************************************************/
	#define GM_Root					m_pKernelData->vRoot
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
		bool										bInited;	//!< �Ƿ��ʼ��
		osg::ref_ptr<osg::Group>					vRoot;		//!< ���ڵ�
		osg::ref_ptr<osgViewer::Viewer>				vViewer;	//!< �ӿڹ�����
	};

	/*!
	*  @brief ��Ⱦ˳��
	*/
	constexpr auto BIN_NEBULA						= 95;
	constexpr auto BIN_GALAXY_PLANE					= 96;
	constexpr auto BIN_GALAXY_POINT					= 97;
	constexpr auto BIN_AUDIO_POINT					= 98;
	constexpr auto BIN_HELP_LINE					= 99;
	constexpr auto BIN_STAR_PLAYING					= 100;
}	// GM
