//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMPost.h
/// @brief		Galaxy-Music Engine - GMPost
/// @version	1.0
/// @author		LiuTao
/// @date		2022.07.20
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommon.h"
#include "GMCommonUniform.h"
#include "GMKernel.h"
#include <osg/Texture2D>

namespace GM
{
	/*************************************************************************
	 Enums
	*************************************************************************/


	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/

	/*!
	*  @class CGMPost
	*  @brief Galaxy-Music GMPost
	*/
	class CGMPost
	{
		// ����
	public:
		/** @brief ���� */
		CGMPost();

		/** @brief ���� */
		~CGMPost();

		/** @brief ��ʼ�� */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief ���� */
		bool Update(double dDeltaTime);
		/** @brief ����(�������������̬֮��) */
		bool UpdateLater(double dDeltaTime);
		/** @brief ���� */
		bool Load();
		/**
		* �޸���Ļ�ߴ�ʱ���ô˺���
		* @author LiuTao
		* @since 2022.07.21
		* @param width: ��Ļ���
		* @param height: ��Ļ�߶�
		* @return void
		*/
		void ResizeScreen(const int width, const int height);
		/**
		* CreatePost
		* ��������
		* @author LiuTao
		* @since 2022.07.20
		* @param pSceneTex:			������ɫͼ
		* @param pBackgroundTex:	������ɫͼ
		* @param pForegroundTex:	ǰ����ɫͼ
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool CreatePost(osg::Texture* pSceneTex,
			osg::Texture* pBackgroundTex,
			osg::Texture* pForegroundTex);

		/**
		* SetVolumeEnable
		* ��������Ⱦ
		* @author LiuTao
		* @since 2022.07.28
		* @param bEnabled: ������ر�����Ⱦ
		* @param pVolumeTex: ����Ⱦͼ
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool SetVolumeEnable(bool bEnabled, osg::Texture* pVolumeTex = nullptr);

		/**
		* UpdateHierarchy
		* @brief ���ڿռ�㼶�仯�����³���
		* @param iHierarchy:		���º�Ŀռ�㼶���
		* @return bool:				�ɹ�true��ʧ��false
		*/
		bool UpdateHierarchy(int iHieNew);

	private:
		/**
		* ������Ⱦ��
		* @brief ��Ļ������С�������棬�Ⱦ���Ч��Ҫ��һЩ
		* @author LiuTao
		* @since 2020.09.01
		* @param width: ��Ч���εĿ��
		* @param height: ��Ч���εĸ߶�
		* @return osg::Geometry* ���ؼ��νڵ�ָ��
		*/
		osg::Geometry* _CreateScreenTriangle(const int width, const int height);
		/**
		* ����Ⱦ������ߴ�
		* @brief ��Ļ������С�������棬�Ⱦ���Ч��Ҫ��һЩ
		* @author LiuTao
		* @since 2020.12.07
		* @param width: ��Ч���εĿ��
		* @param height: ��Ч���εĸ߶�
		* @return void
		*/
		void _ResizeScreenTriangle(const int width, const int height);

		// ����
	private:
		SGMKernelData*										m_pKernelData;					//!< �ں�����
		SGMConfigData*										m_pConfigData;					//!< ��������
		CGMCommonUniform*									m_pCommonUniform;				//!< ����Uniform

		std::string											m_strShaderPath;				//!< post shader ��ɫ��·��

		osg::ref_ptr<osg::Camera>							m_pPostCam;						//!< ���������
		osg::ref_ptr<osg::Geode>							m_pPostGeode;					//!< ��Ⱦ�ڵ�

		osg::ref_ptr<osg::Texture>							m_pVolumeTex;					//!< ����Ⱦ��ɫͼ

		int													m_iPostUnit;					//!< ������嵱ǰ���õ�����Ԫ
		bool												m_bVolume;						//!< ����Ⱦ����
	};
}	// GM