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
		// 函数
	public:
		/** @brief 构造 */
		CGMPost();

		/** @brief 析构 */
		~CGMPost();

		/** @brief 初始化 */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief 更新 */
		bool Update(double dDeltaTime);
		/** @brief 更新(在主相机更新姿态之后) */
		bool UpdateLater(double dDeltaTime);
		/** @brief 加载 */
		bool Load();
		/**
		* 修改屏幕尺寸时调用此函数
		* @author LiuTao
		* @since 2022.07.21
		* @param width: 屏幕宽度
		* @param height: 屏幕高度
		* @return void
		*/
		void ResizeScreen(const int width, const int height);
		/**
		* CreatePost
		* 创建后期
		* @author LiuTao
		* @since 2022.07.20
		* @param pSceneTex:			场景颜色图
		* @param pBackgroundTex:	背景颜色图
		* @param pForegroundTex:	前景颜色图
		* @return bool 成功true， 失败false
		*/
		bool CreatePost(osg::Texture* pSceneTex,
			osg::Texture* pBackgroundTex,
			osg::Texture* pForegroundTex);

		/**
		* SetVolumeEnable
		* 开关体渲染
		* @author LiuTao
		* @since 2022.07.28
		* @param bEnabled: 开启或关闭体渲染
		* @param pVolumeTex: 体渲染图
		* @return bool 成功true， 失败false
		*/
		bool SetVolumeEnable(bool bEnabled, osg::Texture* pVolumeTex = nullptr);

		/**
		* UpdateHierarchy
		* @brief 由于空间层级变化而更新场景
		* @param iHierarchy:		更新后的空间层级编号
		* @return bool:				成功true，失败false
		*/
		bool UpdateHierarchy(int iHieNew);

	private:
		/**
		* 创建渲染面
		* @brief 屏幕两倍大小的三角面，比矩形效率要高一些
		* @author LiuTao
		* @since 2020.09.01
		* @param width: 等效矩形的宽度
		* @param height: 等效矩形的高度
		* @return osg::Geometry* 返回几何节点指针
		*/
		osg::Geometry* _CreateScreenTriangle(const int width, const int height);
		/**
		* 给渲染面重设尺寸
		* @brief 屏幕两倍大小的三角面，比矩形效率要高一些
		* @author LiuTao
		* @since 2020.12.07
		* @param width: 等效矩形的宽度
		* @param height: 等效矩形的高度
		* @return void
		*/
		void _ResizeScreenTriangle(const int width, const int height);

		// 变量
	private:
		SGMKernelData*										m_pKernelData;					//!< 内核数据
		SGMConfigData*										m_pConfigData;					//!< 配置数据
		CGMCommonUniform*									m_pCommonUniform;				//!< 公共Uniform

		std::string											m_strShaderPath;				//!< post shader 着色器路径

		osg::ref_ptr<osg::Camera>							m_pPostCam;						//!< 后期主相机
		osg::ref_ptr<osg::Geode>							m_pPostGeode;					//!< 渲染节点

		osg::ref_ptr<osg::Texture>							m_pVolumeTex;					//!< 体渲染颜色图

		int													m_iPostUnit;					//!< 后期面板当前可用的纹理单元
		bool												m_bVolume;						//!< 体渲染开关
	};
}	// GM