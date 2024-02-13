//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMCommonUniform.h
/// @brief		Galaxy-Music Engine - Common Uniform
/// @version	1.0
/// @author		LiuTao
/// @date		2022.08.21
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommon.h"
#include "GMKernel.h"
#include <osg/Uniform>

namespace GM
{
	/*************************************************************************
	 Struct
	*************************************************************************/

	/*************************************************************************
	 Class
	*************************************************************************/
	/*!
	*  @class CGMCommonUniform
	*  @brief ����ģ�鳣�õ�Uniform
	*/
	class CGMCommonUniform
	{
		// ����
	public:
		/** @brief ���� */
		CGMCommonUniform();
		/** @brief ���� */
		~CGMCommonUniform();
		/** @brief ��ʼ�� */
		void Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData);
		/** @brief ����(�������������̬֮ǰ) */
		void Update(double dDeltaTime);
		/** @brief ����(�������������̬֮����������ģ��UpdateLater֮ǰ) */
		void UpdateLater(double dDeltaTime);
		/**
		* @brief ���ڿռ�㼶�仯�����³���
		* @param iHierarchyNew:		���º�Ŀռ�㼶���
		* @return bool:				�ɹ�true��ʧ��false */
		bool UpdateHierarchy(int iHierarchyNew);
		/**
		* �޸���Ļ�ߴ�ʱ���ô˺���
		* @param width: ��Ļ���
		* @param height: ��Ļ�߶�
		*/
		void ResizeScreen(const int width, const int height);

		inline osg::Uniform*const GetScreenSize() const
		{
			return m_vScreenSizeUniform.get();
		}
		inline osg::Uniform*const GetTime() const
		{
			return m_fTimeUniform.get();
		}
		inline osg::Uniform*const GetStarColor() const
		{
			return m_vStarColorUniform.get();
		}
		inline void SetStarColor(const osg::Vec4f vColor)
		{
			m_vStarColorUniform->set(vColor);
		}
		inline osg::Uniform*const GetLevelArray() const
		{
			return m_fLevelArrayUniform.get();
		}
		/**
		* SetAudioLevel
		* ���õ�ǰ֡��Ƶ�����ֵ
		* @param fLevel ���ֵ [0.0f,1.0f]
		*/
		void SetAudioLevel(const float fLevel);

		inline osg::Uniform* const GetUnit() const
		{
			return m_fUnitUniform.get();
		}

		inline osg::Uniform*const GetStarHiePos() const
		{
			return m_vStarHiePosUniform.get();
		}
		inline void SetStarHiePos(const osg::Vec3f v)
		{
			m_vStarHiePosUniform->set(v);
		}

		inline osg::Uniform*const GetGalaxyAlpha() const
		{
			return m_fGalaxyAlphaUniform.get();
		}
		inline void SetGalaxyAlpha(const float f)
		{
			m_fGalaxyAlphaUniform->set(f);
		}

		inline osg::Uniform* const GetDeltaVPMatrix() const
		{
			return m_mDeltaVPMatrixUniform.get();
		}
		inline void SetDeltaVPMatrix(const osg::Matrixf m)
		{
			m_mDeltaVPMatrixUniform->set(m);
		}

		inline osg::Uniform* const GetMainInvProjMatrix() const
		{
			return m_mMainInvProjUniform.get();
		}
		inline void SetMainInvProjMatrix(const osg::Matrixf m)
		{
			m_mMainInvProjUniform->set(m);
		}

		inline osg::Uniform* const GetEyeFrontDir() const
		{
			return m_vEyeFrontDirUniform.get();
		}
		inline void SetEyeFrontDir(const osg::Vec3f v)
		{
			m_vEyeFrontDirUniform->set(v);
		}

		inline osg::Uniform* const GetEyeRightDir() const
		{
			return m_vEyeRightDirUniform.get();
		}
		inline void SetEyeRightDir(const osg::Vec3f v)
		{
			m_vEyeRightDirUniform->set(v);
		}

		inline osg::Uniform* const GetEyeUpDir() const
		{
			return m_vEyeUpDirUniform.get();
		}
		inline void SetEyeUpDir(const osg::Vec3f v)
		{
			m_vEyeUpDirUniform->set(v);
		}

		inline osg::Uniform* const GetViewUp() const
		{
			return m_vViewUpUniform.get();
		}
		inline void SetViewUp(const osg::Vec3f v)
		{
			m_vViewUpUniform->set(v);
		}

		// ����
	private:
		SGMKernelData*					m_pKernelData;				//!< �ں�����

		osg::ref_ptr<osg::Uniform> m_vScreenSizeUniform;			//!< vec3(��Ļ������Ļ��RTT����)
		osg::ref_ptr<osg::Uniform> m_fTimeUniform;					//!< ʱ�䣬��λ����
		osg::ref_ptr<osg::Uniform> m_vStarColorUniform;				//!< ��ǰ��ɫ
		osg::ref_ptr<osg::Uniform> m_fLevelArrayUniform;			//!< �������
		osg::ref_ptr<osg::Uniform> m_fUnitUniform;					//!< ��ǰ�㼶��λ����
		osg::ref_ptr<osg::Uniform> m_vStarHiePosUniform;			//!< ��Ƶ�ǵ�ǰ�㼶�ռ�����
		osg::ref_ptr<osg::Uniform> m_fGalaxyAlphaUniform;			//!< ��ϵalpha
		osg::ref_ptr<osg::Uniform> m_mDeltaVPMatrixUniform;			//!< �����ֵVP����
		osg::ref_ptr<osg::Uniform> m_mMainInvProjUniform;			//!< �������ProjectionMatrix�������
		osg::ref_ptr<osg::Uniform> m_vEyeFrontDirUniform;			//!< �����ǰ����λ������������ռ�
		osg::ref_ptr<osg::Uniform> m_vEyeRightDirUniform;			//!< ������ҷ���λ������������ռ�
		osg::ref_ptr<osg::Uniform> m_vEyeUpDirUniform;				//!< ������Ϸ���λ������������ռ�

		osg::ref_ptr<osg::Uniform> m_vViewUpUniform;				//!< �۵�view�ռ�Up������ָ�����

		double m_fRenderingTime;									//!< ��ά��Ⱦ�ĳ���ʱ�䣬���ǳ�������ʱ��
	};

}	// GM
