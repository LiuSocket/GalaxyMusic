//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMEarth.h
/// @brief		Galaxy-Music Engine - GMEarth
/// @version	1.0
/// @author		LiuTao
/// @date		2023.11.29
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
	class CGMEarthTail;
	class CGMEarthEngine;

	/*!
	*  @class CGMEarth
	*  @brief Galaxy-Music GMEarth
	*/
	class CGMEarth: public CGMPlanet
	{
		// 函数
	public:
		/** @brief 构造 */
		CGMEarth();

		/** @brief 析构 */
		~CGMEarth();

		/** @brief 初始化 */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief 更新 */
		bool Update(double dDeltaTime);
		/** @brief 更新(在主相机更新姿态之后) */
		bool UpdateLater(double dDeltaTime);
		/** @brief 加载 */
		bool Load();

		/* @brief 传递Uniform */
		void SetUniform(
			osg::Uniform* pPlanetRadius,
			osg::Uniform* pViewLigh,
			osg::Uniform* pCloudTop,
			osg::Uniform* pGroundTop,
			osg::Uniform* pAtmosHeight,
			osg::Uniform* pMinDotUL,
			osg::Uniform* pEyeAltitude,
			osg::Uniform* pWorld2ECEF,
			osg::Uniform* pView2ECEF);

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
		osg::Node* GetEarthRoot(const int iHie) const;

		/**
		* @brief 获取云高度
		* @return float 云高度，单位：米
		*/
		inline float GetCloudBottomHeight() const
		{
			return m_fCloudBottom;
		}
		inline float GetCloudTopHeight() const
		{
			return m_fCloudTop;
		}

		/**
		* @brief 显示、隐藏地球
		* @param bVisible: 是否可见
		*/
		void SetVisible(const bool bVisible);
		/**
		* @brief 设置地球在第2空间层级下的旋转
		* @param fSpin： 自转，单位：弧度
		* @param fObliquity： 自转轴倾角，单位：弧度
		* @param fNorthYaw： 自转轴偏航角，单位：弧度
		*/
		void SetEarthRotate(const double fSpin, const double fObliquity, const double fNorthYaw);
		/**
		* @brief 设置“流浪地球计化”的进展
		* @param fProgress 进展百分比，[0.0, 1.0]
		*/
		void SetWanderingEarthProgress(const float fProgress);

		/**
		* @brief 创建地球
		* @return bool 成功true， 失败false
		*/
		bool CreateEarth();

		/**
		* @brief 由于空间层级变化而更新场景
		* @param iHierarchy:		更新后的空间层级编号
		* @return bool:				成功true，失败false
		*/
		bool UpdateHierarchy(int iHierarchy);
		/**
		* @brief 获取地球的自转轴倾角，流浪地球模式下会变化
		* @return double: 地球的自转轴倾角，单位：弧度
		*/
		inline double GetCurrentObliquity() const
		{
			return m_fCurrentObliquity;
		}
		/**
		* @brief 设置地球的自转轴倾角旋转的角速度，给流浪地球模式用，正常情况下为0
		* @return NorthRotateSpeed: 地球的自转轴倾角旋转的角速度，单位：弧度/秒
		*/
		inline void SetNorthRotateSpeed(const double fNorthRotateSpeed)
		{
			m_fNorthRotateSpeed = fNorthRotateSpeed;
		}
		/**
		* @brief 获取地球的自转轴倾角旋转的角速度，给流浪地球模式用，正常情况下为0
		* @return double: 地球的自转轴倾角旋转的角速度，单位：弧度/秒
		*/
		inline double GetNorthRotateSpeed() const
		{
			return m_fNorthRotateSpeed;
		}

	private:

		/**
		* @brief 创建全球云层对地阴影
		* @return bool:			成功true，失败false
		*/
		bool _CreateGlobalCloudShadow();

		/**
		* @brief 创建第1层级空间的地球，包括地面，云层，大气
		* @return bool:			成功true，失败false
		*/
		bool _CreateEarth_1();
		/**
		* @brief 创建第2层级空间的地球，包括地面，云层，大气
		* @return bool:			成功true，失败false
		*/
		bool _CreateEarth_2();

		/**
		* @brief 创建第2层级空间的流浪地球
		* @return bool:			成功true，失败false
		*/
		bool _CreateWanderingEarth();

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
		* @brief 加载DDS二维纹理数组
		* @param filePreName: 图片文件路径（不包含数字和.dds）
		* @param bFlip: 是否翻转上下（dds需要考虑是否翻转）
		* @return osg::Texture* 返回纹理指针
		*/
		osg::Texture2DArray* _CreateDDSTex2DArray(const std::string& filePreName, bool bFlip = true) const;
		/**
		* @brief 追加DDS二维纹理数组
		* @param pTex：	纹理指针
		* @param filePreName: 图片文件路径（不包含数字和.dds）
		* @param bFlip: 是否翻转上下（dds需要考虑是否翻转）
		* @return bool 成功true，如果纹理指针为空或者增加的图片尺寸和已有图尺寸不同，则返回false
		*/
		bool _AddTex2DArray(osg::Texture2DArray* pTex, const std::string& filePreName, bool bFlip = true);

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
		CGMCommonUniform*								m_pCommonUniform;				//!< 公共Uniform

		std::string										m_strGalaxyShaderPath;			//!< galaxy shader 路径
		std::string										m_strEarthShaderPath;			//!< Earth shader 路径

		double											m_fCurrentObliquity;			//!< 自转轴倾角，单位：弧度
		double											m_fNorthRotateSpeed;			//!< 自转轴倾角旋转的角速度，弧度/秒
		float											m_fCloudBottom;					//!< 云底高度
		float											m_fCloudTop;					//!< 云顶高度

		osg::ref_ptr<osg::Transform>					m_pShadow_2_Transform;			//!< 第2层级当前全球阴影变换结点
		osg::ref_ptr<osg::Group>						m_pEarthRoot_1;					//!< 第1层级地球根节点
		osg::ref_ptr<osg::Group>						m_pEarthRoot_2;					//!< 第2层级地球根节点
		osg::ref_ptr<osg::Geometry>						m_pEarthGeom_1;					//!< 第1层级地球几何节点
		osg::ref_ptr<osg::Geometry>						m_pEarthGeom_2;					//!< 第2层级地球几何节点
		osg::ref_ptr<osg::Geode>						m_pEarthCloud_1;				//!< 1层级地球云层节点
		osg::ref_ptr<osg::Geode>						m_pEarthAtmos_1;				//!< 1层级地球大气节点
		osg::ref_ptr<osg::Geode>						m_pEarthGround_2;				//!< 2层级地球地面节点
		osg::ref_ptr<osg::Geode>						m_pEarthCloud_2;				//!< 2层级地球云层节点
		osg::ref_ptr<osg::Geode>						m_pEarthAtmos_2;				//!< 2层级地球大气节点
		osg::ref_ptr<osg::StateSet>						m_pSSEarthGround_1;				//!< 1层级地球地面状态集
		osg::ref_ptr<osg::StateSet>						m_pSSEarthCloud_1;				//!< 1层级地球云层状态集
		osg::ref_ptr<osg::StateSet>						m_pSSEarthAtmos_1;				//!< 1层级地球大气状态集
		osg::ref_ptr<osg::StateSet>						m_pSSEarthGround_2;				//!< 2层级地球地面状态集
		osg::ref_ptr<osg::StateSet>						m_pSSEarthCloud_2;				//!< 2层级地球云层状态集
		osg::ref_ptr<osg::StateSet>						m_pSSEarthAtmos_2;				//!< 2层级地球大气状态集
		osg::ref_ptr<osg::StateSet>						m_pSSGlobalShadow;				//!< 全球阴影状态集

		osg::ref_ptr<osg::Uniform>						m_vPlanetRadiusUniform;			//!< 行星的两个半径（第二层级）
		osg::ref_ptr<osg::Uniform>						m_vViewLightUniform;			//!< view空间的光源方向
		osg::ref_ptr<osg::Uniform>						m_fCloudTopUniform;				//!< 云层顶高
		osg::ref_ptr<osg::Uniform>						m_fGroundTopUniform;			//!< 地面最高山顶的海拔高
		osg::ref_ptr<osg::Uniform>						m_fAtmosHeightUniform;			//!< 大气层高度
		osg::ref_ptr<osg::Uniform>						m_fMinDotULUniform;				//!< 有光区域的最小DotUL值(-1,0)
		osg::ref_ptr<osg::Uniform>						m_fEyeAltitudeUniform;			//!< 眼点海拔Uniform
		osg::ref_ptr<osg::Uniform>						m_mWorld2ECEFUniform;			//!< “2级世界空间”转 ECEF 的矩阵
		osg::ref_ptr<osg::Uniform>						m_mView2ECEFUniform;			//!< view空间转ECEF的矩阵
		osg::ref_ptr<osg::Uniform>						m_vEarthCoordScaleUniform;		//!< 地球贴图的纹理坐标缩放
		osg::ref_ptr<osg::Uniform>						m_fWanderProgressUniform;		//!< 流浪地球计划进展Uniform

		osg::ref_ptr<osgDB::Options>					m_pDDSOptions;					//!< dds的纹理操作
		osg::ref_ptr<osg::Texture2DArray>				m_aEarthBaseTex;				//!< 地球base color纹理
		osg::ref_ptr<osg::Texture2DArray>				m_aEarthCloudTex;				//!< 地球云层纹理
		osg::ref_ptr<osg::Texture2DArray>				m_aIllumTex;					//!< 地球城市自发光+发动机自发光
		osg::ref_ptr<osg::Texture2DArray>				m_aDEMTex;						//!< 地球DEM
		//osg::ref_ptr<osg::Texture2DArray>				m_aAuroraTex;					//!< 极光纹理
		osg::ref_ptr<osg::Texture>						m_pCloudDetailTex;				//!< 云细节纹理
		osg::ref_ptr<osg::Texture3D>					m_pInscatteringTex;

		osg::ref_ptr<osg::EllipsoidModel>				m_pEllipsoid;					//!< 椭球模型
		std::map<int,osg::ref_ptr<osg::Geode>>			m_mShadowEarthGeode;			//!< 用于全球阴影渲染的节点vector
		osg::ref_ptr<osg::Texture2D>					m_pGlobalShadowTex;				//!< 全球云层阴影贴图
		osg::ref_ptr<osg::Camera>						m_pGlobalShadowCamera;			//!< 全球云层阴影的RTT相机

		CGMEarthTail*									m_pEarthTail;					//!< 流浪地球尾迹体渲染模块
		CGMEarthEngine*									m_pEarthEngine;					//!< 行星发动机模块
	};
}	// GM