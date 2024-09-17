//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMTerrain.h
/// @brief		Galaxy-Music Engine - GMTerrain
/// @version	1.0
/// @author		LiuTao
/// @date		2023.12.31
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommonUniform.h"
#include "GMCelestialScaleVisitor.h"

namespace GM
{
	/*!
	*  @brief 四分之一面的数据结构体
	*/
	struct SQuatorData
	{
		osg::ref_ptr<osg::Transform> pQuatorTrans;	//!< 四分之一面的位置节点
		std::vector<osg::Vec3d> vCenterDirVec;		//!< 所有可能的中心点方向的vector
		int iQuatorID;				//!< 四分之一面象限，0123
		bool bPolar;				//!< 是否是极地区域
	};

	/*!
	*  @class CGMTerrain
	*  @brief Galaxy-Music GMTerrain
	*/
	class CGMTerrain
	{
		// 函数
	public:
		/** @brief 构造 */
		CGMTerrain();

		/** @brief 析构 */
		~CGMTerrain();

		/** @brief 初始化 */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief 更新 */
		bool Update(double dDeltaTime);
		/** @brief 更新(在主相机更新姿态之后) */
		bool UpdateLater(double dDeltaTime);

		/**
		* @brief 修改屏幕尺寸时调用此函数
		* @param iW: 屏幕宽度
		* @param iH: 屏幕高度
		*/
		void ResizeScreen(const int iW, const int iH);
		/**
		* @brief 显示、隐藏行星
		* @param bVisible: 是否可见
		*/
		void SetVisible(const bool bVisible);

		/**
		* @brief 创建地形
		* @return bool 成功true， 失败false
		*/
		bool CreateTerrain();

		/**
		* @brief 由于空间层级变化而更新场景
		* @param iHieNew:			更新后的空间层级编号
		* @return bool:				成功true，失败false */
		bool UpdateHierarchy(int iHieNew);

	private:

		/**
		* @brief 创建第0层级空间的地形
		* @return bool:			成功true，失败false
		*/
		bool _CreateTerrain_0();
		/**
		* @brief 创建第1层级空间的地形
		* @return bool:			成功true，失败false
		*/
		bool _CreateTerrain_1();

		/**
		* @brief 创建六面体细分后的球体的一个面的四分之一的部分，每个顶点都有法线和UV
		* UV0.xy = WGS84对应的UV，[0.0, 1.0]
		* UV1.xy = 六面体贴图UV，[0.0, 1.0]
		* UV1.z = 六面体ID，0,1,2,3,4,5
		* @param bPolar:			是否是极地区域
		* @param iHalfSegment:		四分之一面体的边长的分段数，也就是一个六面体的半边长的分段数
		* @return Geometry:			返回几何体指针
		*/
		osg::Geometry* _MakeHexahedronQuaterGeometry(const bool bPolar, int iHalfSegment = 64) const;

		/**
		* @brief 根据顶点的信息获取顶点的索引，会特殊处理国际日期变更线上的顶点
		* @param iX，iY: 顶点的XY位置
		* @param iHalfSeg: 四分之一面体的边长的分段数，也就是一个六面体的半边长的分段数
		* @return int: 顶点的索引
		*/
		inline int _GetVertIndex(const int iX, const int iY, const int iHalfSeg) const
		{
			return iY * (iHalfSeg + 1) + iX;
		}

		// 变量
	protected:
		SGMKernelData*		m_pKernelData = nullptr;		//!< 内核数据
		SGMConfigData*		m_pConfigData = nullptr;		//!< 配置数据
		CGMCommonUniform*	m_pCommonUniform = nullptr;		//!< 公共Uniform

		std::string m_strGalaxyShaderPath = "Shaders/GalaxyShader/";			//!< galaxy shader 路径
		std::string m_strTerrainShaderPath = "Shaders/TerrainShader/";			//!< Terrain shader 路径

		std::vector<osg::ref_ptr<osg::Group>>		m_pHieTerrainRootVector;		//!< 01空间层级的根节点
		std::vector<SQuatorData>					m_sQuatorVec;					//!< 四分之一面数据vector

		CGMCelestialScaleVisitor*	m_pCelestialScaleVisitor = nullptr;		//!< 用于控制天体大小
	};
}	// GM