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

#include "../GMCommon.h"
#include "../GMKernel.h"
#include <osg/MatrixTransform>
#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/Geometry>

namespace GM
{
	/*************************************************************************
	Structs
	*************************************************************************/
	// 面类型枚举（用于扩张操作）
	enum FaceType
	{
		FACE_VERTEX,   // 顶点面（原顶点拉伸形成的三角形）
		FACE_ORIGINAL, // 原面面（原面拉伸形成的多边形）
		FACE_EDGE      // 棱面（原棱拉伸形成的四边形）
	};

	// 带类型的面结构
	struct FaceWithType
	{
		std::vector<int> indices;
		FaceType type;
	};
	/*************************************************************************
	Class
	*************************************************************************/
	class CGMCommonUniform;

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
		* @brief 修改屏幕尺寸时,子类调用此函数
		* @param width: 屏幕宽度
		* @param height: 屏幕高度
		*/
		void ResizeScreen(const int width, const int height);

		/**
		* 设置渲染质量
		* @param eQuality:	渲染质量枚举
		*/
		inline void SetRenderQuality(EGMRENDER_QUALITY eQuality)
		{ 
			m_pConfigData->eRenderQuality = eQuality;
		}
		/**
		* 获取渲染质量
		* @return EGMRENDER_QUALITY 渲染质量
		*/
		inline EGMRENDER_QUALITY GetRenderQuality() const
		{
			return m_pConfigData->eRenderQuality;
		}

		/** @brief 获取体渲染颜色图，不含Alpha */
		inline osg::Texture2D* GetRayMarchColorTex() const
		{
			return m_rayMarchColorTex.get();
		}
		/** @brief 获取体渲染alpha图，RGBA分别输出“左上、右上、左下、右下”4个子像素的alpha */
		inline osg::Texture2D* GetRayMarchAlphaTex() const
		{
			return m_rayMarchAlphaTex.get();
		}

		// 函数
	protected:
		/** @brief 析构 */
		virtual ~CGMVolumeBasic();

		/** @brief 初始化 */
		void Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief 更新(在主相机更新姿态之前) */
		void Update(double dDeltaTime);
		/** @brief 更新(在主相机更新姿态之后，体渲染模块UpdateLater之前) */
		void UpdateLater(double dDeltaTime);

		/**
		* @brief 加载2D纹理
		* @param fileName: 图片文件路径
		* @param iChannelNum: 1、2、3、4分别代表R、RG、RGB、RGBA
		* @return osg::Texture* 返回纹理指针
		*/
		osg::Texture* _CreateTexture2D(const std::string& fileName, const int iChannelNum);

		/**
		* @brief 生成体积云需要的柏拉图实体（介于12面体和20面体之间的实体）
		* @param pFaceGeom:		创建的柏拉图实体的12个五边形的指针
		* @param pEdgeGeom:		创建的柏拉图实体的30个四边形的指针
		* @param pVertGeom:		创建的柏拉图实体的20个三角形的指针
		*/
		void CreatePlatonicSolids(
			osg::Geometry** pFaceGeom,
			osg::Geometry** pEdgeGeom,
			osg::Geometry** pVertGeom) const;

		/**
		* @brief 生成足球（C60-富勒烯）实体（12个五边形和20个六边形以及他们之间的融合带组成的实体）
		* @param pFaceGeom:		12个五边形+20个六边形
		* @param pEdgeGeom:		90个四边形
		* @param pVertGeom:		60个三角形
		*/
		void CreateFootballSolids(
			osg::Geometry** pFaceGeom,
			osg::Geometry** pEdgeGeom,
			osg::Geometry** pVertGeom) const;

	private:
		/**
		 * 截角操作（输出普通面列表）
		 * @param inputVerts  原始顶点数组
		 * @param inputFaces  原始面列表（逆时针顺序）
		 * @param fBevel      切角比例 (0~0.5)
		 * @param outputVerts 输出新顶点数组
		 * @param outputFaces 输出新面列表（普通面，每个面由顶点索引组成）
		 * @param outputEdges 输出边集合（每条边由两个顶点索引组成，无序）
		 */
		void _TruncatePolyhedron(
			const osg::ref_ptr<osg::Vec3Array>& inputVerts,
			const std::vector<std::vector<int>>& inputFaces,
			float fBevel,
			osg::ref_ptr<osg::Vec3Array>& outputVerts,
			std::vector<std::vector<int>>& outputFaces,
			std::vector<std::pair<int, int>>& outputEdges) const;

		/**
		 * 扩张操作（输出带类型的面列表）
		 * @param inputVerts  原始顶点数组
		 * @param inputFaces  原始面列表（逆时针顺序）
		 * @param fOffset     扩张距离（沿法线方向移动量）
		 * @param outputVerts 输出新顶点数组
		 * @param outputFaces 输出新面列表（带类型）
		 * @param outputEdges 输出边集合
		 */
		void _ExpandPolyhedron(
			const osg::ref_ptr<osg::Vec3Array>& inputVerts,
			const std::vector<std::vector<int>>& inputFaces,
			float fOffset,
			osg::ref_ptr<osg::Vec3Array>& outputVerts,
			std::vector<FaceWithType>& outputFaces,
			std::vector<std::pair<int, int>>& outputEdges) const;

		/** @brief raymarching初始化，用于体渲染 */
		void _InitRayMarching();
		/**
		* @brief 设置屏幕上的像素在1米近平面位置的长度,用于抗锯齿等功能
		* @param fFovy : 相机垂直FOV，单位：°
		* @param iHeight : 屏幕高度上的像素数量
		*/
		//void _SetPixelLength(const float fFovy, const int iHeight);

		osg::Texture* _Load3DShapeNoise() const;
		osg::Texture* _Load3DErosionNoise() const;
		osg::Texture* _Load3DCurlNoise() const;

		// 变量
	protected:
		SGMKernelData*										m_pKernelData;					//!< 内核数据
		SGMConfigData*										m_pConfigData;					//!< 配置数据
		CGMCommonUniform*									m_pCommonUniform;				//!< 公共Uniform
		std::string											m_strVolumeShaderPath;			//!< Volume shader 着色器路径
		std::string											m_strCoreTexturePath;			//!< Volume texture 核心贴图路径
		std::string											m_strMediaTexturePath;			//!< Volume texture 非核心贴图路径

		float												m_fResolutionScale;				//!< 体渲染分辨率缩放比例

		//osg::ref_ptr<osg::Uniform>							m_fPixelLengthUniform;			//!< 1米近截面处的像素长度Uniform

		osg::ref_ptr<osg::Group>							m_pVolumeRoot;					//!< 体渲染根节点
		osg::ref_ptr<osg::MatrixTransform>					m_pDodecahedronTrans;			//!< 正12面体变换节点
		osg::ref_ptr<osg::Geode>							m_pDodecahedronFace;			//!< 12面体的“面”几何节点
		osg::ref_ptr<osg::Geode>							m_pDodecahedronEdge;			//!< 12面体的“边”几何节点
		osg::ref_ptr<osg::Geode>							m_pDodecahedronVert;			//!< 12面体的“点”几何节点
		osg::ref_ptr<osg::Texture>							m_3DShapeTex;					//!< 三维无缝shape噪声，RGBA，128^3
		osg::ref_ptr<osg::Texture>							m_3DErosionTex;					//!< 三维无缝erosion噪声，RGB，32^3
		osg::ref_ptr<osg::Texture>							m_3DCurlTex;					//!< 三维无缝curl噪声，R，128*128*4
		osg::ref_ptr<osg::Texture>							m_blueNoiseTex;					//!< 蓝噪声
		osg::ref_ptr<osg::Texture2D>						m_rayMarchColorTex;				//!< rayMarch生成的颜色图
		osg::ref_ptr<osg::Texture2D>						m_rayMarchAlphaTex;				//!< rayMarch生成的alpha图
		osg::ref_ptr<osg::Camera>							m_rayMarchCamera;				//!< the camera for raymarch
	};
}	// GM
