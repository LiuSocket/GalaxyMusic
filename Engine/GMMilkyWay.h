//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMMilkyWay.h
/// @brief		Galaxy-Music Engine - GMMilkyWay.h
/// @version	1.0
/// @author		LiuTao
/// @date		2023.08.06
//////////////////////////////////////////////////////////////////////////
#pragma once
#include "GMVolumeBasic.h"

namespace GM
{
	/*************************************************************************
	 Macro Defines
	*************************************************************************/

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
	*  @class CGMMilkyWay
	*  @brief Galaxy-Music Nebula Module
	*/
	class CGMMilkyWay : public CGMVolumeBasic
	{
		// 函数
	public:
		/** @brief 构造 */
		CGMMilkyWay();
		/** @brief 析构 */
		~CGMMilkyWay();

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
		* @brief 修改屏幕尺寸时调用此函数
		* @param width: 屏幕宽度
		* @param height: 屏幕高度
		*/
		void ResizeScreen(const int width, const int height);
		/**
		* @brief 由于空间层级变化而更新场景
		* @param iHierarchy:		更新后的空间层级编号
		* @return bool:				成功true，失败false
		*/
		bool UpdateHierarchy(int iHieNew);

		inline osg::Texture* GetDistanceMap() const
		{
			return m_distanceMap.get();
		}

	private:
		/**
		* @brief 创建长方体
		* @param fLength			长度
		* @param fWidth				宽度
		* @param fHeight			高度
		* @return Geometry			返回创建的几何体指针
		*/
		osg::Geometry* _MakeBoxGeometry(
			const float fLength = 10.0f,
			const float fWidth = 10.0f,
			const float fHeight = 2.0f) const;

		/**
		* @brief 初始化银河系状态集
		* @param pSS				需要设置的状态集指针
		* @param sVR				星系体积范围
		* @param strShaderName		shader的名字
		* @return bool				成功true，错误false
		*/
		bool _InitMilkyWayStateSet(osg::StateSet* pSS,const SGMVolumeRange& sVR, const std::string strShaderName);

	// 变量
	private:
		std::string										m_strCoreGalaxyTexPath;			//!< 星系核心贴图路径
		std::string										m_strGalaxyShaderPath;			//!< 星系着色器路径
		osg::ref_ptr<osg::Texture>						m_galaxyTex;					//!< the galaxy texture
		osg::ref_ptr<osg::Texture>						m_galaxyHeightTex;				//!< the galaxy height texture
		osg::ref_ptr<osg::Texture>						m_2DNoiseTex;					//!< the 2D noise texture
		osg::ref_ptr<osg::Texture2D>					m_distanceMap;					//!< 星系体渲染出的深度相关的信息

		osg::ref_ptr<osg::Geode>						m_pGeodeMilkyWay;				//!< 银河系体渲染box的Geode

		osg::ref_ptr<osg::StateSet>						m_pSsMilkyWayDecFace;			//!< 银河系12面体面状态集
		osg::ref_ptr<osg::StateSet>						m_pSsMilkyWayDecEdge;			//!< 银河系12面体边状态集
		osg::ref_ptr<osg::StateSet>						m_pSsMilkyWayDecVert;			//!< 银河系12面体点状态集
	};

}	// GM