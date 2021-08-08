//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMVolumeBasic.h
/// @brief		Galaxy-Music Engine - GMNebula.h
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.10
//////////////////////////////////////////////////////////////////////////
#pragma once
#include "GMVolumeBasic.h"
#include <osg/MatrixTransform>

namespace GM
{
	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/

	/*!
	*  @class CGMNebula
	*  @brief Galaxy-Music Nebula Module
	*/
	class CGMNebula : public CGMVolumeBasic
	{
		// 函数
	public:
		/** @brief 构造 */
		CGMNebula();
		/** @brief 析构 */
		~CGMNebula();

		/** @brief 初始化 */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData);
		/** @brief 更新 */
		bool Update(double dDeltaTime);
		/** @brief 更新(在主相机更新姿态之后) */
		bool UpdateLater(double dDeltaTime);
		/** @brief 加载 */
		bool Load();
		/**
		* MakeNebula;
		* @brief 创建星云
		* @author LiuTao
		* @since 2020.12.12
		* @param fLength			长度
		* @param fWidth				宽度
		* @param fHeight			高度
		* @param fX					中心点的X坐标
		* @param fY					中心点的Y坐标
		* @param fZ					中心点的Z坐标
		* @return void
		*/
		void MakeNebula(float fLength = 10.0f, float fWidth = 10.0f, float fHeight = 0.5f
		, float fX = 0.0f, float fY = 0.0f, float fZ = 0.0f);

		/** @brief 供其他模块调用 */
	public:

		/** @brief: Show or hide the nebula */
		void SetNebulaEnabled(const bool bEnable);
		/** @brief Get the No.0 switching texture for TAA */
		osg::Texture2D* GetTAAMap_0();
		/** @brief Get the No.1 switching texture for TAA */
		osg::Texture2D* GetTAAMap_1();

		/**
		* 修改屏幕尺寸时调用此函数
		* @author LiuTao
		* @since 2020.12.07
		* @param width: 屏幕宽度
		* @param height: 屏幕高度
		* @return void
		*/
		void ResizeScreen(const int width, const int height);

	private:

		osg::Texture* _Load3DShapeNoise();
		osg::Texture* _Load3DErosionNoise();
		osg::Texture* _Load3DCurlNoise();

		/**
		* _MakeSphereGeometry;
		* @brief 创建球体
		* @author LiuTao
		* @since 2020.06.22
		* @param fRadius			半径
		* @param iLatSegment		纬度分段数，经度分段数是其两倍
		* @return Geometry			返回创建的几何体指针
		*/
		osg::Geometry* _MakeSphereGeometry(float fRadius = 10.0f, int iLatSegment = 32);

		/**
		* _MakeBoxGeometry;
		* @brief 创建长方体
		* @author LiuTao
		* @since 2020.12.12
		* @param fLength			长度
		* @param fWidth				宽度
		* @param fHeight			高度
		* @return Geometry			返回创建的几何体指针
		*/
		osg::Geometry* _MakeBoxGeometry(float fLength = 10.0f, float fWidth = 10.0f, float fHeight = 2.0f);

	// 变量
	private:
		std::string											m_strNebulaShaderPath;			//!< Nebula shader 星云着色器路径
		std::string											m_strCoreNebulaTexPath;			//!< Nebula texture 星云核心贴图路径
		osg::ref_ptr<osg::Texture>							m_galaxyTex;					//!< the galaxy texture
		osg::ref_ptr<osg::Texture>							m_galaxyHeightTex;				//!< the galaxy height texture
		osg::ref_ptr<osg::Texture>							m_2DNoiseTex;					//!< the 2D noise texture
		osg::ref_ptr<osg::Texture>							m_3DShapeTex_128;				//!< 三维无缝噪声，128^3
		osg::ref_ptr<osg::Texture2D>						m_rayMarchMap_0;				//!< the No.0 1/16 size texture by ray march
		osg::ref_ptr<osg::Texture2D>						m_rayMarchMap_1;				//!< the No.1 1/16 size texture by ray march
		osg::ref_ptr<osg::Texture2D>						m_vectorMap_0;					//!< the No.0 1/16 size vector texture
		osg::ref_ptr<osg::Texture2D>						m_vectorMap_1;					//!< the No.1 1/16 size vector texture
		osg::ref_ptr<osg::Texture2D>						m_distanceMap;					//!< the 1/16 size distance texture
		osg::ref_ptr<osg::MatrixTransform>					m_nebulaRayMarchTransform;		//!< the Nebula raymarching Transform node
		osg::ref_ptr<osg::MatrixTransform>					m_nebulaTransform;				//!< the Nebula Transform node
		osg::ref_ptr<osg::Node>								m_nebulaNode;
		osg::ref_ptr<osg::Camera>							m_rayMarchCamera;				//!< the camera for raymarching cloud
		osg::ref_ptr<osg::StateSet>							m_stateSetRayMarch;
		osg::ref_ptr<osg::StateSet>							m_stateSetFinal;
		int													m_iUnitRayMarch;				//!< the unit of Terrain map for cloud ray marching
		int													m_iUnitFinal;					//!< the unit of Terrain map for cloud of final step
		double												m_dTimeLastFrame;				//!< 上一帧时间
		osg::Vec2f											m_vLastShakeVec;				//!< 上一帧抖动向量
		osg::ref_ptr<osg::Uniform>							m_vDeltaShakeUniform;			//!< 相机插值抖动向量Uniform
		osg::ref_ptr<osg::Uniform>							m_fCountUniform;				//!< 每帧+1 计数器
		osg::ref_ptr<osg::Uniform>							m_pDeltaVPMatrixUniform;		//!< 相机插值VP矩阵Uniform
	};

}	// GM