//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMVolumeBasic.h
/// @brief		Galaxy-Music Engine - Volume basic
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.09
//////////////////////////////////////////////////////////////////////////
#pragma once
#include <random>
#include "GMCommon.h"
#include "GMKernel.h"
#include <osg/Texture>
#include <osg/Geometry>

namespace GM
{
	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/

	/*!
	*  @class CGMVolumeBasic
	*  @brief Digital-Planets Volume Basic
	*/
	class CGMVolumeBasic
	{
		// ����
	public:
		/** @brief ���� */
		CGMVolumeBasic();

		/**
		* SetRenderQuality
		* ������Ⱦ����������������Ĭ������
		* @author LiuTao
		* @since 2020.12.27
		* @param bHigh:	�Ƿ������
		* @return void
		*/
		inline void SetRenderQuality(bool bHigh)
		{ 
			m_bHigh = bHigh;
		}
		/**
		* GetRenderQuality
		* ��ȡ��Ⱦ����������������Ĭ������
		* @author LiuTao
		* @since 2020.12.27
		* @return bool �Ƿ������
		*/
		inline bool GetRenderQuality()
		{
			return m_bHigh;
		}

		/**
		* GetShakeParameters
		* ���������������������TAA���ɽ��ֱ������4��
		* @author LiuTao
		* @since 2020.08.17
		* @param fShake0:	����ֵ0��-1.5/-0.5/+0.5/+1.5
		* @param fShake1:	����ֵ1��-1.5/-0.5/+0.5/+1.5
		* @return void
		*/
		void GetShakeParameters(float& fShake0, float& fShake1);

		/**
		* AddShakeCount
		* �������ͬ�������ļ���
		* @author LiuTao
		* @since 2020.08.18
		* @return void
		*/
		static void AddShakeCount();
		/**
		* GetShakeCount
		* ��ȡ���ͬ�������ļ���
		* @author LiuTao
		* @since 2020.08.18
		* @return m_iShakeCount ����ͬ�������ļ���
		*/
		static unsigned int GetShakeCount();

		// ����
	protected:
		/** @brief ���� */
		virtual ~CGMVolumeBasic();

		/** @brief ��ʼ�� */
		void Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData);
		/** @brief ����(�������������̬֮�����󡢻���������Ⱦģ��UpdateLater֮ǰ) */
		void UpdateLater(double dDeltaTime);
		/**
		* ActiveTAA
		* ��TAA�ڵ������Texture��������TAA
		* @author LiuTao
		* @since 2020.09.01
		* @param pTex ��ҪTAA����������ָ��
		* @param pVectorTex �ٶ�ʸ������ָ��
		* @param pDistanceTex ���ơ����塢�����ľ�����Ϣ����ָ��
		* @return bool �ɹ�Ϊtrue��ʧ��Ϊfalse
		*/
		bool ActiveTAA(osg::Texture* pTex, osg::Texture* pVectorTex, osg::Texture* pDistanceTex);

		/**
		* ����2D����
		* @author LiuTao
		* @since 2020.06.16
		* @param fileName: ͼƬ�ļ�·��
		* @param iChannelNum: 1��2��3��4�ֱ����R��RG��RGB��RGBA
		* @return osg::Texture* ��������ָ��
		*/
		osg::Texture* _CreateTexture2D(const std::string& fileName, const int iChannelNum);
		/**
		* �޸���Ļ�ߴ�ʱ,������ô˺���
		* @author LiuTao
		* @since 2020.12.07
		* @param width: ��Ļ���
		* @param height: ��Ļ�߶�
		* @return void
		*/
		void ResizeScreen(const int width, const int height);
	private:
		/**
		* _InitTAA
		* TAA��ʼ������������Ⱦ
		* @author LiuTao
		* @since 2020.09.01
		* @param strCorePath ����·��
		* @return void
		*/
		void _InitTAA(std::string strCorePath);
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
	protected:
		SGMKernelData*										m_pKernelData;					//!< �ں�����
		SGMConfigData*										m_pConfigData;					//!< ��������

		std::vector<osg::ref_ptr<osg::Group>>				m_pHierarchyRootVector;			//!< ÿ���ռ�㼶�ĸ��ڵ�
		std::string											m_strShaderPath;				//!< Volume shader ��ɫ��·��
		std::string											m_strCoreTexturePath;			//!< Volume texture ������ͼ·��
		std::string											m_strMediaTexturePath;			//!< Volume texture �Ǻ�����ͼ·��
		osg::ref_ptr<osg::Geometry>							m_pGeo;							//!< ����Ⱦ������ڵ�
		osg::ref_ptr<osg::Geode>							m_pTAAGeode;					//!< TAA�ڵ�
		osg::ref_ptr<osg::StateSet>							m_statesetTAA;					//!< TAA�ڵ��״̬��
		osg::ref_ptr<osg::Texture>							m_blueNoiseTex;					//!< ������
		osg::ref_ptr<osg::Texture2D>						m_TAAMap_0;						//!< the No.0 switching texture for TAA
		osg::ref_ptr<osg::Texture2D>						m_TAAMap_1;						//!< the No.1 switching texture for TAA
		osg::ref_ptr<osg::Texture2D>						m_TAADistanceMap_0;				//!< the No.0 switching distance texture for TAA
		osg::ref_ptr<osg::Texture2D>						m_TAADistanceMap_1;				//!< the No.1 switching distance texture for TAA
		bool												m_bHigh;						//!< ��Ⱦ������true == ����������������Ⱦʱ�䳤
		int													m_iWidthFull;					//!< ��Ļ���
		int													m_iHeightFull;					//!< ��Ļ�߶�
		int													m_iUnitColor;					//!< TAA�õ�����ɫ����Ԫ
		int													m_iUnitVelocity;				//!< TAA�õ����ٶ�����Ԫ
		osg::ref_ptr<osg::Camera>							m_cameraTAA;					//!< the camera for TAA
		std::default_random_engine							m_iRandom;						//!< ���ֵ
		osg::Matrixd										m_mLastVP;						//!< ��һ֡Raymarching�����ViewProjectionMatrix
		osg::ref_ptr<osg::Uniform>							m_fNoiseNumUniform;				//!< float���������Uniform��0-1
		osg::ref_ptr<osg::Uniform>							m_vScreenSizeUniform;			//!< vec3(��Ļ������Ļ��RTT����)
		osg::ref_ptr<osg::Uniform>							m_vShakeVectorUniform;			//!< TAA�����������

	private:
		int													m_iUnitTAA;						//!< TAA�������Ԫ
		static unsigned int									m_iShakeCount;					//!< TAA��������ļ���
	};
}	// GM
