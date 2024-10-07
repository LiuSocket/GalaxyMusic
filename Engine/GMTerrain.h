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
	*  @brief 6个面的XYZ向量信息的结构体
	*/
	struct SFaceXYZ
	{
		SFaceXYZ(const osg::Vec3d& x, const osg::Vec3d& y, const osg::Vec3d& z) : vX(x), vY(y), vZ(z) {}

		osg::Vec3d vX = osg::Vec3d(1, 0, 0);			// X轴的方向
		osg::Vec3d vY = osg::Vec3d(0, 1, 0);			// Y轴的方向
		osg::Vec3d vZ = osg::Vec3d(0, 0, 1);			// Z轴的方向
	};

	/*!
	*  @brief 瓦片的数据结构体
	*/
	struct STileData
	{
		osg::ref_ptr<osg::Transform> pTileTrans;	// 瓦片的位置节点
		// 数组的0位存储第0层瓦片的6个面（0、1、2、3、4、5），组成一个球体
		// 后面依次存储第1/2/3/4...层瓦片的4象限（0/1/2/3）
		std::vector<int> iTileQuadVec;				
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
		* @brief 创建第1层级空间的地形的瓦片（0.5、1.0、1.5、2.0、2.5...）
		*	0.0级瓦片是六面体细分后的球体的一个面，在行星模块已经创建，不需要在这里创建了，这个瓦片用6张Tile0的纹理
		*	0.5级瓦片是0.0级瓦片的四分之一，1.0级瓦片是0.5级瓦片细分一次，这两个瓦片用24张Tile1的纹理
		*	1.5级瓦片是1.0级瓦片的四分之一，2.0级瓦片是1.5级瓦片细分一次，这两个瓦片用96张Tile1的纹理
		* @return bool:			成功true，失败false
		*/
		bool _CreateTerrain_1();

		/**
		* @brief 创建对应层级瓦片的几何体，每个顶点都有法线和UV
		* UV0.xy = WGS84对应的UV，[0.0, 1.0]
		* UV1.xy = 六面体贴图UV，[0.0, 1.0]
		* UV1.z = 六面体ID，0,1,2,3,4,5
		* @param iTileVec:		瓦片的各级数据
		* @param iSegment:		瓦片体的边长的分段数，建议设置成2^n-1是为了保证高程图的分辨率是2^n
		* @return Geometry:		返回几何体指针
		*/
		osg::Geometry* _MakeTileGeometry(const std::vector<int>& iTileVec, int iSegment) const;

		/**
		* @brief 获取瓦片中心点在ECEF坐标系中的方向
		* @param iTileVec：			瓦片的各级数据
		* @return osg::Vec3d:		返回瓦片的中心点在ECEF坐标系中的方向，出错返回(0,0,0)
		*/
		osg::Vec3d _GetTileCenterDir(const std::vector<int>& iTileVec) const;

		/**
		* @brief 获取瓦片中心点在ECEF坐标系中的方向投影到边长为2的正方体上的坐标
		* @param iTileVec：			瓦片的各级数据
		* @return osg::Vec3d:		返回瓦片的中心点在ECEF坐标系中的方向投影到边长为2的正方体上的坐标，出错返回(0,0,0)
		*/
		osg::Vec3d _GetTileCenterInBox(const std::vector<int>& iTileVec) const;

		/**
		* @brief 根据顶点的信息获取顶点的索引
		* @param iX，iY: 顶点的XY位置
		* @param iHalfSeg: 瓦片体的边长的分段数，也就是一个六面体的半边长的分段数
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
		
		//!< tile1瓦片的经纬度偏移
		osg::ref_ptr<osg::Uniform> m_vTileOffsetLonLatUniform;
		//!< tile1瓦片的索引偏移
		osg::ref_ptr<osg::Uniform> m_vTileOffsetIDUniform;

		std::string m_strGalaxyShaderPath = "Shaders/GalaxyShader/";			//!< galaxy shader 路径
		std::string m_strTerrainShaderPath = "Shaders/TerrainShader/";			//!< Terrain shader 路径

		std::vector<osg::ref_ptr<osg::Group>>		m_pHieRootVector;			//!< 0/1空间层级的根节点
		std::map<float,osg::ref_ptr<osg::Group>>	m_pHie1_TileMap;			//!< 1层空间的各级瓦片根节点
		CGMCelestialScaleVisitor*	m_pCelestialScaleVisitor = nullptr;			//!< 用于控制天体大小

	private:
		std::vector<STileData>						m_sTileVec;					//!< 瓦片数据vector
		std::vector<std::vector<osg::Vec3d>>		m_vTileDirVec;				//!< 所有瓦片中心点方向的vector
		std::vector<SFaceXYZ>						m_vFaceXYZVec;				//!< 6个面XYZ方向信息的vector
	};
}	// GM