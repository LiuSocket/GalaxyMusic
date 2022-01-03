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
		// 函数
	public:
		/** @brief 构造 */
		CGMVolumeBasic();

		/**
		* SetRenderQuality
		* 设置渲染质量，高质量或者默认质量
		* @author LiuTao
		* @since 2020.12.27
		* @param bHigh:	是否高质量
		* @return void
		*/
		inline void SetRenderQuality(bool bHigh)
		{ 
			m_bHigh = bHigh;
		}
		/**
		* GetRenderQuality
		* 获取渲染质量，高质量或者默认质量
		* @author LiuTao
		* @since 2020.12.27
		* @return bool 是否高质量
		*/
		inline bool GetRenderQuality()
		{
			return m_bHigh;
		}

		/**
		* GetShakeParameters
		* 设置相机抖动参数，用于TAA，可将分辨率提高4倍
		* @author LiuTao
		* @since 2020.08.17
		* @param fShake0:	抖动值0，-1.5/-0.5/+0.5/+1.5
		* @param fShake1:	抖动值1，-1.5/-0.5/+0.5/+1.5
		* @return void
		*/
		void GetShakeParameters(float& fShake0, float& fShake1);

		/**
		* AddShakeCount
		* 增加相机同步抖动的计数
		* @author LiuTao
		* @since 2020.08.18
		* @return void
		*/
		static void AddShakeCount();
		/**
		* GetShakeCount
		* 获取相机同步抖动的计数
		* @author LiuTao
		* @since 2020.08.18
		* @return m_iShakeCount 返回同步抖动的计数
		*/
		static unsigned int GetShakeCount();

		// 函数
	protected:
		/** @brief 析构 */
		virtual ~CGMVolumeBasic();

		/** @brief 初始化 */
		void Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData);
		/** @brief 更新(在主相机更新姿态之后，气象、环境、体渲染模块UpdateLater之前) */
		void UpdateLater(double dDeltaTime);
		/**
		* ActiveTAA
		* 向TAA节点中添加Texture，并启动TAA
		* @author LiuTao
		* @since 2020.09.01
		* @param pTex 需要TAA操作的纹理指针
		* @param pVectorTex 速度矢量纹理指针
		* @param pDistanceTex 星云、气体、尘埃的距离信息纹理指针
		* @return bool 成功为true，失败为false
		*/
		bool ActiveTAA(osg::Texture* pTex, osg::Texture* pVectorTex, osg::Texture* pDistanceTex);

		/**
		* 加载2D纹理
		* @author LiuTao
		* @since 2020.06.16
		* @param fileName: 图片文件路径
		* @param iChannelNum: 1、2、3、4分别代表R、RG、RGB、RGBA
		* @return osg::Texture* 返回纹理指针
		*/
		osg::Texture* _CreateTexture2D(const std::string& fileName, const int iChannelNum);
		/**
		* 修改屏幕尺寸时,子类调用此函数
		* @author LiuTao
		* @since 2020.12.07
		* @param width: 屏幕宽度
		* @param height: 屏幕高度
		* @return void
		*/
		void ResizeScreen(const int width, const int height);
	private:
		/**
		* _InitTAA
		* TAA初始化，用于体渲染
		* @author LiuTao
		* @since 2020.09.01
		* @param strCorePath 核心路径
		* @return void
		*/
		void _InitTAA(std::string strCorePath);
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
	protected:
		SGMKernelData*										m_pKernelData;					//!< 内核数据
		SGMConfigData*										m_pConfigData;					//!< 配置数据

		std::vector<osg::ref_ptr<osg::Group>>				m_pHierarchyRootVector;			//!< 每个空间层级的根节点
		std::string											m_strShaderPath;				//!< Volume shader 着色器路径
		std::string											m_strCoreTexturePath;			//!< Volume texture 核心贴图路径
		std::string											m_strMediaTexturePath;			//!< Volume texture 非核心贴图路径
		osg::ref_ptr<osg::Geometry>							m_pGeo;							//!< 体渲染几何体节点
		osg::ref_ptr<osg::Geode>							m_pTAAGeode;					//!< TAA节点
		osg::ref_ptr<osg::StateSet>							m_statesetTAA;					//!< TAA节点的状态集
		osg::ref_ptr<osg::Texture>							m_blueNoiseTex;					//!< 蓝噪声
		osg::ref_ptr<osg::Texture2D>						m_TAAMap_0;						//!< the No.0 switching texture for TAA
		osg::ref_ptr<osg::Texture2D>						m_TAAMap_1;						//!< the No.1 switching texture for TAA
		osg::ref_ptr<osg::Texture2D>						m_TAADistanceMap_0;				//!< the No.0 switching distance texture for TAA
		osg::ref_ptr<osg::Texture2D>						m_TAADistanceMap_1;				//!< the No.1 switching distance texture for TAA
		bool												m_bHigh;						//!< 渲染质量，true == 高质量，代价是渲染时间长
		int													m_iWidthFull;					//!< 屏幕宽度
		int													m_iHeightFull;					//!< 屏幕高度
		int													m_iUnitColor;					//!< TAA用到的颜色纹理单元
		int													m_iUnitVelocity;				//!< TAA用到的速度纹理单元
		osg::ref_ptr<osg::Camera>							m_cameraTAA;					//!< the camera for TAA
		std::default_random_engine							m_iRandom;						//!< 随机值
		osg::Matrixd										m_mLastVP;						//!< 上一帧Raymarching相机的ViewProjectionMatrix
		osg::ref_ptr<osg::Uniform>							m_fNoiseNumUniform;				//!< float随机数噪声Uniform：0-1
		osg::ref_ptr<osg::Uniform>							m_vScreenSizeUniform;			//!< vec3(屏幕长，屏幕宽，RTT比例)
		osg::ref_ptr<osg::Uniform>							m_vShakeVectorUniform;			//!< TAA相机抖动向量

	private:
		int													m_iUnitTAA;						//!< TAA面板纹理单元
		static unsigned int									m_iShakeCount;					//!< TAA相机抖动的计数
	};
}	// GM
