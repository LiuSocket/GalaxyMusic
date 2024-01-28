//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMOort.h
/// @brief		Galaxy-Music Engine - GMOort.h
/// @version	1.0
/// @author		LiuTao
/// @date		2023.08.06
//////////////////////////////////////////////////////////////////////////
#pragma once
#include "GMVolumeBasic.h"
#include "GMCommonUniform.h"

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
	*  @class CGMOort
	*  @brief Galaxy-Music Oort Module
	*/
	class CGMOort : public CGMVolumeBasic
	{
		// 函数
	public:
		/** @brief 构造 */
		CGMOort();
		/** @brief 析构 */
		~CGMOort();

		/** @brief 初始化 */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief 更新 */
		bool Update(double dDeltaTime);
		/** @brief 更新(在主相机更新姿态之后) */
		bool UpdateLater(double dDeltaTime);
		/** @brief 加载 */
		bool Load();
		/**
		* @brief 创建奥尔特星云
		*/
		void MakeOort();
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

		/**
		* @brief 设置奥尔特云在第4空间层级下的位置
		* @param vHiePos 位置，单位：第4空间层级单位
		*/
		void SetOortHiePos4(const osg::Vec3f& vHiePos);
		/**
		* @brief 设置奥尔特云相对于银河的旋转姿态
		* @param qRotate 旋转姿态四元数
		*/
		void SetOortAttitude(const osg::Quat& qRotate);

	private:
		/**
		* @brief 创建球体
		* @param fRadius			半径
		* @param iLatSegment		纬度分段数，经度分段数是其两倍
		* @return Geometry			返回创建的几何体指针
		*/
		osg::Geometry* _MakeSphereGeometry(const float fRadius = 10.0f, const int iLatSegment = 32) const;

		/**
		* @brief 初始化奥尔特云状态集
		* @param pSS				需要设置的状态集指针
		* @param strShaderName		shader的名字
		* @return bool				成功true，错误false
		*/
		bool _InitOortStateSet(osg::StateSet* pSS, const std::string strShaderName);

	// 变量
	private:
		std::string										m_strCoreGalaxyTexPath;			//!< 星系核心贴图路径
		std::string										m_strGalaxyShaderPath;			//!< 星系着色器路径
		osg::ref_ptr<osg::Texture>						m_galaxyTex;					//!< the galaxy texture
		osg::ref_ptr<osg::Texture>						m_galaxyHeightTex;				//!< the galaxy height texture

		osg::ref_ptr<osg::Geode>						m_pGeodeOort3;					//!< 第3层级奥尔特云的Geode
		osg::ref_ptr<osg::Transform>					m_pOortTransform4;				//!< 第4层级奥尔特云的变换结点

		osg::ref_ptr<osg::StateSet>						m_pSsOortDecFace;				//!< 奥尔特云12面体面状态集
		osg::ref_ptr<osg::StateSet>						m_pSsOortDecEdge;				//!< 奥尔特云12面体边状态集
		osg::ref_ptr<osg::StateSet>						m_pSsOortDecVert;				//!< 奥尔特云12面体点状态集

		osg::ref_ptr<osg::Uniform>						m_fOortVisibleUniform;			//!< 奥尔特云可见度的Uniform
		osg::ref_ptr<osg::Uniform>						m_fUnitUniform;					//!< 第3层级单位:当前层级单位
		osg::ref_ptr<osg::Uniform>						m_mAttitudeUniform;				//!< 在星系坐标系下的旋转姿态矩阵
	};

}	// GM