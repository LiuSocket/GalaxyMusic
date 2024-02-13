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
	*  @brief 各个模块常用的Uniform
	*/
	class CGMCommonUniform
	{
		// 函数
	public:
		/** @brief 构造 */
		CGMCommonUniform();
		/** @brief 析构 */
		~CGMCommonUniform();
		/** @brief 初始化 */
		void Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData);
		/** @brief 更新(在主相机更新姿态之前) */
		void Update(double dDeltaTime);
		/** @brief 更新(在主相机更新姿态之后，所有其他模块UpdateLater之前) */
		void UpdateLater(double dDeltaTime);
		/**
		* @brief 由于空间层级变化而更新场景
		* @param iHierarchyNew:		更新后的空间层级编号
		* @return bool:				成功true，失败false */
		bool UpdateHierarchy(int iHierarchyNew);
		/**
		* 修改屏幕尺寸时调用此函数
		* @param width: 屏幕宽度
		* @param height: 屏幕高度
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
		* 设置当前帧音频的振幅值
		* @param fLevel 振幅值 [0.0f,1.0f]
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

		// 变量
	private:
		SGMKernelData*					m_pKernelData;				//!< 内核数据

		osg::ref_ptr<osg::Uniform> m_vScreenSizeUniform;			//!< vec3(屏幕长，屏幕宽，RTT比例)
		osg::ref_ptr<osg::Uniform> m_fTimeUniform;					//!< 时间，单位：秒
		osg::ref_ptr<osg::Uniform> m_vStarColorUniform;				//!< 当前颜色
		osg::ref_ptr<osg::Uniform> m_fLevelArrayUniform;			//!< 振幅数组
		osg::ref_ptr<osg::Uniform> m_fUnitUniform;					//!< 当前层级单位长度
		osg::ref_ptr<osg::Uniform> m_vStarHiePosUniform;			//!< 音频星当前层级空间坐标
		osg::ref_ptr<osg::Uniform> m_fGalaxyAlphaUniform;			//!< 星系alpha
		osg::ref_ptr<osg::Uniform> m_mDeltaVPMatrixUniform;			//!< 相机插值VP矩阵
		osg::ref_ptr<osg::Uniform> m_mMainInvProjUniform;			//!< 主相机的ProjectionMatrix的逆矩阵
		osg::ref_ptr<osg::Uniform> m_vEyeFrontDirUniform;			//!< 主相机前方单位向量，在世界空间
		osg::ref_ptr<osg::Uniform> m_vEyeRightDirUniform;			//!< 主相机右方单位向量，在世界空间
		osg::ref_ptr<osg::Uniform> m_vEyeUpDirUniform;				//!< 主相机上方单位向量，在世界空间

		osg::ref_ptr<osg::Uniform> m_vViewUpUniform;				//!< 眼点view空间Up向量，指向天空

		double m_fRenderingTime;									//!< 三维渲染的持续时间，不是程序运行时长
	};

}	// GM
