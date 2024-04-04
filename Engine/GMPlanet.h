//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMPlanet.h
/// @brief		Galaxy-Music Engine - GMPlanet
/// @version	1.0
/// @author		LiuTao
/// @date		2023.09.23
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommon.h"
#include "GMKernel.h"
#include "GMCelestialScaleVisitor.h"

namespace GM
{
	/*************************************************************************
	Class
	*************************************************************************/

	/*!
	*  @class CGMPlanet
	*  @brief Galaxy-Music CGMPlanet
	*/
	class CGMPlanet
	{
		// 函数
	public:
		/** @brief 构造 */
		CGMPlanet();

		/** @brief 析构 */
		~CGMPlanet();

		/** @brief 初始化 */
		bool Init(SGMConfigData* pConfigData);

		/**
		* @brief 创建六面体细分后的球体，每个顶点都有法线和UV
		* UV0.xy = WGS84对应的UV，[0.0, 1.0]
		* UV1.xy = 六面体贴图UV，[0.0, 1.0]
		* UV1.z = 六面体ID，0,1,2,3,4,5
		* @param iSegment:			六面体的每个边长的分段数
		* @return Geometry:			返回几何体指针
		*/
		osg::Geometry* MakeHexahedronSphereGeometry(int iSegment = 32);

		/**
		* @brief 创建椭球体
		* @param ellipsoid				用于描述天体的椭球模型
		* @param iLonSegments			经度分段数
		* @param iLatSegments			纬度分段数
		* @param fHae					外延半径（类似于海拔高度）
		* @param bGenTexCoords			是否生成UV坐标
		* @param bWholeMap				是否显示完整贴图(在半球或者复杂曲面时)
		* @param bFlipNormal			true 法线向内，false 法线向外
		* @param fLatStart				纬度开始位置，单位：角度 °
		* @param fLatEnd				纬度结束位置，单位：角度 °
		* @return Geometry				返回创建的几何体指针
		*/
		osg::Geometry* MakeEllipsoidGeometry(
			const osg::EllipsoidModel*	ellipsoid,
			int							iLonSegments,
			int							iLatSegments,
			float						fHae,
			bool						bGenTexCoords,
			bool						bWholeMap = false,
			bool						bFlipNormal = false,
			float						fLatStart = -90.0,
			float						fLatEnd = 90.0) const;

	protected:
		/** @brief 工具函数，平常不需要调用 */
		void Panorama2CubeMap();

	private:
		/**
		* @brief 全景DEM转cubemap版本DEM
		* @param strPanoPath: 全景DEM_bed路径
		* @param strCubeMapPath: CubeDEM路径
		* @return bool: 成功true，失败false
		*/
		bool _Panorama_2_CubeDEM(
			const std::string& strPanoPath,
			const std::string& strCubeDEMPath);
		/**
		* @brief 全景图转cubemap
		* @param strPanoramaPath: 全景图路径
		* @param strCubeMapPath: CubeMap路径
		* @return bool: 成功true，失败false
		*/
		bool _Panorama_2_CubeMap(
			const std::string& strPanoramaPath,
			const std::string& strCubeMapPath);

		/**
		* @brief 根据顶点的信息获取顶点的索引，会特殊处理国际日期变更线上的顶点
		* @param iFace: 顶点所在的面
		* @param iX，iY: 顶点的XY位置
		* @param iSegment: 六面体的每个边长的分段数
		* @param bEast:	是否是东半球，如果不是，就需要考虑去“变更线”上找索引 
		* @return int: 顶点的索引
		*/
		int _GetVertIndex(const int iFace, const int iX, const int iY, const int iSegment, const bool bEast);

		// 变量
	protected:
		SGMConfigData*							m_pConfigData;					//!< 配置数据
		std::string								m_strCoreModelPath;				//!< 核心模型资源路径
		CGMCelestialScaleVisitor*				m_pCelestialScaleVisitor;		//!< 用于控制天体大小
	};
}	// GM