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
#include "GMEnums.h"

namespace GM
{
	/*************************************************************************
	constexpr
	*************************************************************************/
	constexpr float PROGRESS_0 = 0.1f; 		// ǰ̫��ʱ������������ɲ��ʱ��
	constexpr float PROGRESS_1 = 0.15f; 	// ɲ��ʱ����������ʼ����������̬����ת������
	constexpr float PROGRESS_2 = 0.2f;		// ������̬��������
	constexpr float PROGRESS_3 = 0.25f;		// ������̬����������������������з����෴
	constexpr float PROGRESS_4 = 0.3f;		// ȫ�򷢶��������ɹ�������
	constexpr float PROGRESS_5 = 0.4f;		// ������ȫ���ʿ���

	/*************************************************************************
	 Macro Defines
	*************************************************************************/

	#define GM_UNIT_SCALE				(1e5)		// ÿ���㼶�ռ�֮��ĳ߶ȱ���
	#define GM_HIERARCHY_MAX			(6)			// �������㼶
	#define GM_MIN_RADIUS				(5e-4)		// �����ÿ���㼶�ռ��е���С����
	#define GM_MAX_RADIUS				(GM_MIN_RADIUS*GM_UNIT_SCALE*2)	// �����ÿ���㼶�ռ��е���󽹾�
	#define GM_HANDLE_RADIUS			(0.15)		// ���ְ뾶����λ������꣬1e20

	#define SCAT_COS_NUM				(32)		// ɢ��ͼ��̫�����������߷���ĵ�˲����� [-1,1]
	#define SCAT_LIGHT_NUM				(16)		// ɢ��ͼ��̫���������Ϸ���ĵ�˲����� [-1,1]
	#define SCAT_PITCH_NUM				(256)		// ɢ��ͼ�ĸ����������Ϸ���ĵ�˲����� [-1,1]
	#define SCAT_ALT_NUM				(16)		// ɢ��ͼ�Ĵ�����߶Ȳ����� [0,fAtmosThick]m

	/*************************************************************************
	 Enums
	*************************************************************************/

	/*!
	 *  @enum EGMAtmosHeight
	 *  @brief �������ö�٣�Ϊ������㣬��������������ɼ����޶�ֵ
	 */
	enum EGMAtmosHeight
	{
		EGMAH_0,		//!< û�д���
		EGMAH_16,		//!< 16km���
		EGMAH_32,		//!< 32km���
		EGMAH_64,		//!< 64km���
		EGMAH_128,		//!< 128km���
	};

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
			: strCorePath("../../Data/Core/"), strMediaPath(L"../../Data/Media/"),
			eRenderQuality(EGMRENDER_LOW), bPhoto(false), bWanderingEarth(false),
			fFovy(40.0f), fVolume(0.5f), fMinBPM(23.0),
			iScreenWidth(1920), iScreenHeight(1080)
		{}

		std::string						strCorePath;			//!< ������Դ·��
		std::wstring					strMediaPath;			//!< �ⲿ��Դ·��
		EGMRENDER_QUALITY				eRenderQuality;			//!< �߻���ģʽ
		bool							bPhoto;					//!< ��Ƭģʽ����
		bool							bWanderingEarth;		//!< ���˵���ģʽ����
		float							fFovy;					//!< ����Ĵ�ֱFOV����λ����
		float							fVolume;				//!< ������[0.0,1.0]
		double							fMinBPM;				//!< ��Ƶ����СBPM
		int								iScreenWidth;			//!< ��Ļ��ȣ���λ������
		int								iScreenHeight;			//!< ��Ļ�߶ȣ���λ������
	};
}	// GM
