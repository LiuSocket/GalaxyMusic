//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMEarthEngine.h
/// @brief		Galaxy-Music Engine - GMEarthEngine
/// @version	1.0
/// @author		LiuTao
/// @date		2024.02.11
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommonUniform.h"
#include "GMPlanet.h"

#include <osg/Texture2DArray>

namespace GM
{
	/*************************************************************************
	Class
	*************************************************************************/
	class CEngineDirControlVisitor;

	/*!
	*  @class CGMEarthEngine
	*  @brief Galaxy-Music GMEarthEngine
	*/
	class CGMEarthEngine
	{
		// 函数
	public:
		/** @brief 构造 */
		CGMEarthEngine();

		/** @brief 析构 */
		~CGMEarthEngine();

		/** @brief 初始化 */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief 更新 */
		bool Update(double dDeltaTime);
		/** @brief 更新(在主相机更新姿态之后) */
		bool UpdateLater(double dDeltaTime);
		/** @brief 加载 */
		bool Load();

		/* @brief 传递纹理 */
		void SetTex(osg::Texture* pEarthTailTex, osg::Texture* pInscatteringTex);
		/* @brief 传递Uniform */
		void SetUniform(
			osg::Uniform* pViewLigh,
			osg::Uniform* pGroundTop,
			osg::Uniform* pAtmosHeight,
			osg::Uniform* pMinDotUL,
			osg::Uniform* pEyeAltitude,
			osg::Uniform* pWanderProgress);

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
		osg::Node* GetEarthEngineRoot(const int iHie) const;
		/**
		* @brief 显示、隐藏
		* @param bVisible: 是否可见
		*/
		void SetVisible(const bool bVisible);
		/**
		* @brief 设置“流浪地球计化”的进展
		* @param fProgress 进展百分比，[0.0, 1.0]
		*/
		void SetWanderingEarthProgress(const float fProgress);
		/**
		* @brief 创建行星发动机
		* @return bool 成功true， 失败false
		*/
		bool CreateEngine();

		/**
		* @brief 由于空间层级变化而更新场景
		* @param iHierarchy:		更新后的空间层级编号
		* @return bool:				成功true，失败false
		*/
		bool UpdateHierarchy(int iHierarchy);

		/* @brief 获取行星发动机开启比例 */
		inline osg::Uniform* GetEngineStartRatioUniform() const
		{
			return m_fEngineStartRatioUniform.get();
		}

	private:
		/**
		* @brief 创建流浪地球行星发动机的喷射口亮点，仅用于太空视角
		* @return bool:			成功true，失败false
		*/
		bool _GenEarthEnginePoint_1();
		bool _GenEarthEnginePoint_2();
		/**
		* @brief 创建流浪地球行星发动机的喷射流，仅用于太空视角
		* @return bool:			成功true，失败false
		*/
		bool _GenEarthEngineJetLine_1();
		bool _GenEarthEngineJetLine_2();
		/**
		* @brief 创建流浪地球行星发动机的主体，LOD第4级，用于近地视角
		* @return bool:			成功true，失败false
		*/
		bool _GenEarthEngineBody_1();
		bool _GenEarthEngineBody_2();
		/**
		* @brief 创建流浪地球行星发动机的柱状喷射流面片，LOD第4级，用于近地视角
		* @return bool:			成功true，失败false
		*/
		bool _GenEarthEngineStream();

		/**
		* @brief 加载2D纹理
		* @param fileName: 图片文件路径
		* @param iChannelNum: 1、2、3、4分别代表R、RG、RGB、RGBA
		* @return osg::Texture* 返回纹理指针
		*/
		osg::Texture* _CreateTexture2D(const std::string& fileName, const int iChannelNum) const;
		/**
		* @brief 加载2D纹理
		* @param pImg:		图片指针
		* @param iChannelNum: 1、2、3、4分别代表R、RG、RGB、RGBA
		* @return osg::Texture* 返回纹理指针
		*/
		osg::Texture* _CreateTexture2D(osg::Image* pImg, const int iChannelNum) const;

		/**
		* @brief 加载DDS纹理
		* @param fileName: 图片文件路径
		* @param eWrap_S: 横/X/U方向包裹模式
		* @param eWrap_T: 纵/Y/V方向包裹模式
		* @param bFlip: 是否翻转上下（dds需要考虑是否翻转）
		* @return osg::Texture* 返回纹理指针
		*/
		osg::Texture* _CreateDDSTexture(const std::string& fileName,
			osg::Texture::WrapMode eWrap_S, osg::Texture::WrapMode eWrap_T,
			bool bFlip = false) const;
		/**
		* @brief 生成行星发动机经、纬、高、尺寸信息，生成后不要再调用
		*/
		void _GenEarthEngineData();

		/**
		* @brief 生成“行星发动机基座图”和“周围bloom图”，仅用于生成贴图，生成后不要再调用
		*/
		void _GenEarthEngineTexture();

		/**
		* @brief 创建流浪地球上的行星发动机的喷射口亮点
		* @param pEllipsoid				用于描述天体的椭球模型
		* @param fUnit					当前空间层级的单位长度，单位：米
		* @return Geometry				返回创建的几何体指针
		*/
		osg::Geometry* _MakeEnginePointGeometry(const osg::EllipsoidModel* pEllipsoid, const double fUnit) const;
		/**
		* @brief 创建流浪地球上的行星发动机的喷射流
		* @param pEllipsoid				用于描述天体的椭球模型
		* @param fUnit					当前空间层级的单位长度，单位：米
		* @return Geometry				返回创建的几何体指针
		*/
		osg::Geometry* _MakeEngineJetLineGeometry(const osg::EllipsoidModel* pEllipsoid, const double fUnit) const;
		/**
		* @brief 创建流浪地球上的行星发动机的喷射流，用于近地视角
		* @param pEllipsoid				用于描述天体的椭球模型
		* @param fUnit					当前空间层级的单位长度，单位：米
		* @return Geometry				返回创建的几何体指针
		*/
		osg::Geometry* _MakeEngineJetStreamGeometry(const osg::EllipsoidModel* pEllipsoid, const double fUnit) const;

		/**
		* @brief 临时添加的生成流浪地球版本的各个贴图的工具函数
		* @param strPath0: 贴图0，不包含编号和后缀名
		* @param strPath1: 贴图1，注意要水平翻转，不包含编号和后缀名
		* @param strOut: 输出的贴图路径，不包含编号和后缀名
		* @param iType: 不同的叠加方式
		*/
		void _MixWEETexture(
			const std::string& strPath0, const std::string& strPath1, const std::string& strOut,
			const int iType);

		/**
		* @brief 流浪地球的行星发动机的方向
		* @param vECEFPos: 发动机的ECEF坐标
		* @return osg::Vec3: 发动机方向
		*/
		osg::Vec3 _Pos2Norm(const osg::Vec3& vECEFPos) const;

		/**
		* @brief 行星发动机喷口偏转后的偏移位置（与垂直向上喷射相比）
		* @param vDir: 发动机的喷射方向（ECEF）
		* @param vUp: 发动机的上方向（ECEF）
		* @return osg::Vec3: 喷口偏转后的偏移位置，单位：米
		*/
		inline osg::Vec3 _NozzlePos(const osg::Vec3& vDir, const osg::Vec3& vUp) const
		{
			return vDir * 1500.0 - vUp * 2000.0;
		}

		// 变量
	private:
		SGMKernelData*									m_pKernelData;					//!< 内核数据
		SGMConfigData*									m_pConfigData;					//!< 配置数据
		CGMCommonUniform*								m_pCommonUniform;				//!< 公共Uniform

		std::string										m_strGalaxyShaderPath;			//!< galaxy shader 路径
		std::string										m_strEarthShaderPath;			//!< Earth shader 路径
		std::string										m_strCoreModelPath;				//!< 核心模型资源路径

		osg::ref_ptr<osg::Group>						m_pEarthEngineRoot_1;			//!< 第1层级行星发动机根节点
		osg::ref_ptr<osg::Group>						m_pEarthEngineRoot_2;			//!< 第2层级行星发动机根节点
		osg::ref_ptr<osg::Geode>						m_pEarthEnginePointNode_1;		//!< 1层级行星发动机喷射口亮点
		osg::ref_ptr<osg::Geode>						m_pEarthEnginePointNode_2;		//!< 2层级行星发动机喷射口亮点
		osg::ref_ptr<osg::Geode>						m_pEarthEngineJetNode_1;		//!< 1层级行星发动机喷射流
		osg::ref_ptr<osg::Geode>						m_pEarthEngineJetNode_2;		//!< 2层级行星发动机喷射流
		osg::ref_ptr<osg::Geode>						m_pEarthEngineStream;			//!< 行星发动机喷射流柱状面片
		osg::ref_ptr<osg::Node>							m_pEarthEngineBody_1;			//!< 1层级行星发动机主体
		osg::ref_ptr<osg::Node>							m_pEarthEngineBody_2;			//!< 2层级行星发动机主体

		osg::ref_ptr<osg::Texture>						m_pEarthTailTex;				//!< 地球尾迹纹理
		osg::ref_ptr<osg::Texture>						m_pInscatteringTex;				//!< 大气内散射纹理

		osg::ref_ptr<osg::Uniform>						m_vViewLightUniform;			//!< view空间的光源方向
		osg::ref_ptr<osg::Uniform>						m_fCloudTopUniform;				//!< 云层顶高
		osg::ref_ptr<osg::Uniform>						m_fGroundTopUniform;			//!< 地面最高山顶的海拔高
		osg::ref_ptr<osg::Uniform>						m_fAtmosHeightUniform;			//!< 大气层高度
		osg::ref_ptr<osg::Uniform>						m_fMinDotULUniform;				//!< 有光区域的最小DotUL值(-1,0)
		osg::ref_ptr<osg::Uniform>						m_fEyeAltitudeUniform;			//!< 眼点海拔Uniform
		osg::ref_ptr<osg::Uniform>						m_fWanderProgressUniform;		//!< 流浪地球计划进展Uniform
		osg::ref_ptr<osg::Uniform>						m_fEngineStartRatioUniform;		//!< 发动机开启比例Uniform

		osg::ref_ptr<osgDB::Options>					m_pDDSOptions;					//!< dds的纹理操作

		osg::ref_ptr<CEngineDirControlVisitor>			m_pEngineDirControl;
		osg::ref_ptr<osg::EllipsoidModel>				m_pEllipsoid;					//!< 椭球模型

		// 流浪地球行星发动机数据,xy=经纬度（弧度），z=底高（米），w=发动机高度（米）
		// 图片宽度（s）= 发动机数量，高度（t）= 1
		osg::ref_ptr<osg::Image>						m_pEarthEngineDataImg;
	};
}	// GM