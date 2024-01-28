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
	 Macro Defines
	*************************************************************************/

	/*************************************************************************
	Enums
	*************************************************************************/

	/*!
	 *  @enum EGMVolumeState
	 *  @brief 鼠标按键
	 */
	enum EGMVolumeState
	{
		EGM_VS_Oort,					//!< 奥尔特云
		EGM_VS_Galaxy,					//!< 银河系
		EGM_VS_GalaxyGroup,				//!< 星系群
		EGM_VS_SuperCluster,			//!< 超星系团
		EGM_VS_UltraCluster				//!< 究极星系团
	};

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
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief 更新 */
		bool Update(double dDeltaTime);
		/** @brief 更新(在主相机更新姿态之后) */
		bool UpdateLater(double dDeltaTime);
		/** @brief 加载 */
		bool Load();
		/**
		* @brief 创建银河系
		* @param fLength			长度，单位：米
		* @param fWidth				宽度，单位：米
		* @param fHeight			高度，单位：米
		* @param fX					中心点的X坐标，单位：米
		* @param fY					中心点的Y坐标，单位：米
		* @param fZ					中心点的Z坐标，单位：米
		*/
		void MakeMilkyWay(double fLength = 1e21, double fWidth = 1e21, double fHeight = 5e19
		, double fX = 0.0, double fY = 0.0, double fZ = 0.0);

		/** @brief 供其他模块调用 */
	public:

		/**
		* 修改屏幕尺寸时调用此函数
		* @author LiuTao
		* @since 2020.12.07
		* @param width: 屏幕宽度
		* @param height: 屏幕高度
		* @return void
		*/
		void ResizeScreen(const int width, const int height);

	protected:
		/**
		* UpdateHierarchy
		* @brief 由于空间层级变化而更新场景
		* @param iHierarchy:		更新后的空间层级编号
		* @return bool:				成功true，失败false
		*/
		bool UpdateHierarchy(int iHieNew);

		/**
		* _MakeSphereGeometry;
		* @brief 创建球体
		* @author LiuTao
		* @since 2020.06.22
		* @param fRadius			半径
		* @param iLatSegment		纬度分段数，经度分段数是其两倍
		* @return Geometry			返回创建的几何体指针
		*/
		osg::Geometry* MakeSphereGeometry(const float fRadius = 10.0f, const int iLatSegment = 32) const;

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
		osg::Geometry* MakeBoxGeometry(
			const float fLength = 10.0f,
			const float fWidth = 10.0f,
			const float fHeight = 2.0f) const;

	private:
		osg::Texture* _Load3DShapeNoise() const;
		osg::Texture* _Load3DErosionNoise() const;
		osg::Texture* _Load3DCurlNoise() const;

		/**
		* _InitMilkyWayStateSet
		* @brief 初始化银河系状态集
		* @author LiuTao
		* @since 2022.07.26
		* @param pSS				需要设置的状态集指针
		* @param sVR				星系体积范围
		* @param strShaderName		shader的名字
		* @return bool				成功true，错误false
		*/
		bool _InitMilkyWayStateSet(osg::StateSet* pSS,const SGMVolumeRange& sVR, const std::string strShaderName);
		/**
		* _InitOortStateSet
		* @brief 初始化奥尔特云状态集
		* @author LiuTao
		* @since 2022.07.26
		* @param pSS				需要设置的状态集指针
		* @param strShaderName		shader的名字
		* @return bool				成功true，错误false
		*/
		bool _InitOortStateSet(osg::StateSet* pSS, const std::string strShaderName);

	// 变量
	protected:
		osg::ref_ptr<osg::Texture>						m_3DShapeTex;					//!< 三维无缝噪声，RGBA，128^3
		osg::ref_ptr<osg::Texture>						m_3DErosionTex;					//!< 三维无缝噪声，RGB，32^3
		osg::ref_ptr<osg::Texture>						m_2DNoiseTex;					//!< the 2D noise texture
		osg::ref_ptr<osg::Texture2D>					m_distanceMap;					//!< 星系体渲染出的深度相关的信息
		std::string										m_strCoreNebulaTexPath;			//!< Nebula texture 星云核心贴图路径
		osg::ref_ptr<osg::Uniform>						m_fGalaxyAlphaUniform;			//!< 银河的alpha的Uniform
		osg::ref_ptr<osg::Uniform>						m_vStarHiePosUniform;			//!< 音频星当前层级空间坐标Uniform

	private:
		std::string										m_strNebulaShaderPath;			//!< Nebula shader 星云着色器路径
		osg::ref_ptr<osg::Texture>						m_galaxyTex;					//!< the galaxy texture
		osg::ref_ptr<osg::Texture>						m_galaxyHeightTex;				//!< the galaxy height texture
		osg::ref_ptr<osg::Texture2D>					m_nebulaTex;					//!< the nebula texture by ray march
		osg::ref_ptr<osg::Texture2D>					m_rayMarchMap_1;				//!< the No.1 texture by ray march
		osg::ref_ptr<osg::Texture2D>					m_vectorMap_0;					//!< the No.0 vector texture
		osg::ref_ptr<osg::Texture2D>					m_vectorMap_1;					//!< the No.1 vector texture

		osg::ref_ptr<osg::Geode>						m_pDodecahedronFace;			//!< 12面体的“面”几何节点
		osg::ref_ptr<osg::Geode>						m_pDodecahedronEdge;			//!< 12面体的“边”几何节点
		osg::ref_ptr<osg::Geode>						m_pDodecahedronVert;			//!< 12面体的“点”几何节点

		osg::ref_ptr<osg::StateSet>						m_pSsMilkyWayDecFace;			//!< 银河系12面体面状态集
		osg::ref_ptr<osg::StateSet>						m_pSsMilkyWayDecEdge;			//!< 银河系12面体边状态集
		osg::ref_ptr<osg::StateSet>						m_pSsMilkyWayDecVert;			//!< 银河系12面体点状态集
		osg::ref_ptr<osg::StateSet>						m_pSsOortDecFace;				//!< 奥尔特云12面体面状态集
		osg::ref_ptr<osg::StateSet>						m_pSsOortDecEdge;				//!< 奥尔特云12面体边状态集
		osg::ref_ptr<osg::StateSet>						m_pSsOortDecVert;				//!< 奥尔特云12面体点状态集

		EGMVolumeState									m_eVolumeState;					//!< 体渲染状态
		osg::ref_ptr<osg::MatrixTransform>				m_pDodecahedronTrans;			//!< 正12面体变换节点
		osg::ref_ptr<osg::Camera>						m_rayMarchCamera;				//!< the camera for raymarching cloud
		osg::Vec3d										m_vWorldEyePos;					//!< 眼点当前帧世界空间坐标
		double											m_dTimeLastFrame;				//!< 上一帧时间
		osg::ref_ptr<osg::Uniform>						m_mDeltaVPMatrixUniform;		//!< 相机插值VP矩阵Uniform
		osg::ref_ptr<osg::Uniform>						m_fOortVisibleUniform;			//!< 奥尔特云可见度的Uniform
	};

}	// GM