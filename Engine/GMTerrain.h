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

namespace GM
{
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
		/** @brief 加载 */
		bool Load();

		/**
		* @brief 修改屏幕尺寸时调用此函数
		* @param iW: 屏幕宽度
		* @param iH: 屏幕高度
		*/
		void ResizeScreen(const int iW, const int iH);

		/**
		* @brief 获取指定层级的根节点，如果不存在，就返回空
		* @param iHie: 空间层级，0,1,2
		* @return osg::Node* 对应层级的根节点指针
		*/
		osg::Node* GetTerrainRoot(const int iHie) const;

		/**
		* @brief 创建地形
		* @return bool 成功true， 失败false
		*/
		bool CreateTerrain();

		/**
		* @brief 由于空间层级变化而更新场景
		* @param iHierarchy:		更新后的空间层级编号
		* @return bool:				成功true，失败false */
		bool UpdateHierarchy(int iHierarchy);

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

		// 变量
	private:
		SGMKernelData*								m_pKernelData;					//!< 内核数据
		SGMConfigData*								m_pConfigData;					//!< 配置数据
		CGMCommonUniform*							m_pCommonUniform;				//!< 公共Uniform
		std::vector<osg::ref_ptr<osg::Group>>		m_pHieTerrainRootVector;		//!< 01空间层级的根节点

		std::string									m_strGalaxyShaderPath;			//!< galaxy shader 路径
		std::string									m_strTerrainShaderPath;			//!< Terrain shader 路径

		osg::ref_ptr<osg::Group>					m_pTerrainRoot_1;				//!< 第1层级地形共用球体
	};
}	// GM