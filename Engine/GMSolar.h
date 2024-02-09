//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMSolar.h
/// @brief		Galaxy-Music Engine - GMSolar
/// @version	1.0
/// @author		LiuTao
/// @date		2020.11.27
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommon.h"
#include "GMCommonUniform.h"
#include "GMKernel.h"
#include "GMDispatchCompute.h"

#include <random>
#include <osg/Node>
#include <osg/AutoTransform>
#include <osg/Texture>
#include <osg/Texture2DArray>
#include <osg/Depth>
#include <osg/BlendFunc>
#include <osg/CullFace>
#include <osg/CoordinateSystemNode>

namespace GM
{
	/*************************************************************************
	 Enums
	*************************************************************************/

	/*************************************************************************
	Structs
	*************************************************************************/
	// 天体结构体，恒星，行星，卫星等都属于天体
	struct SGMCelestialBody
	{
		SGMCelestialBody()
			: fOrbitalRadius(1e11), fOrbitalPeriod(1.0), fTrueAnomaly(0.0),
			fEquatorRadius(0.01), fPolarRadius(0.01), fObliquity(0.0), fSpinPeriod(1.0),
			fSpin(0.0), fGroundTop(1000.0f), fCloudTop(8e3f), eAtmosHeight(EGMAH_0),
			vAtmosColor(osg::Vec4f(1, 1, 1, 1)),
			fRingMinRadius(0), fRingMaxRadius(0)
		{}
		SGMCelestialBody(double orbitalRadius, double period, double startTrueAnomaly,
			double equatorRadius,double polarRadius,double obliquity,double spinPeriod,
			float groundTop, float cloudTop, EGMAtmosHeight atmosH,
			osg::Vec4f atmosColor = osg::Vec4f(1, 1, 1, 1),
			double ringMinRadius = 0, double ringMaxRadius = 0)
			: fOrbitalRadius(orbitalRadius), fOrbitalPeriod(period), fTrueAnomaly(startTrueAnomaly),
			fEquatorRadius(equatorRadius), fPolarRadius(polarRadius), fObliquity(obliquity), fSpinPeriod(spinPeriod),
			fSpin(0.0), fGroundTop(groundTop), fCloudTop(cloudTop), eAtmosHeight(atmosH), vAtmosColor(atmosColor),
			fRingMinRadius(ringMinRadius), fRingMaxRadius(ringMaxRadius)	
		{}

		double fOrbitalRadius;			// 公转轨道半径，单位：米
		double fOrbitalPeriod;			// 公转周期，单位：s
		double fTrueAnomaly;			// 真近点角，单位：弧度
		double fEquatorRadius;			// 天体赤道半径，单位：米
		double fPolarRadius;			// 天体两级半径，单位：米
		double fObliquity;				// 天体的自转轴平面与黄道面夹角（有向），单位：弧度
		double fSpinPeriod;				// 天体的自转周期，单位：秒
		double fSpin;					// 天体当前帧的自转角度，单位：弧度
		float fGroundTop;				// 地面最高山顶的海拔高，单位：米
		float fCloudTop;				// 岩石行星的云顶海拔高，单位：米
		EGMAtmosHeight eAtmosHeight;	// 天体的大气厚度，单位：千米
		osg::Vec4f vAtmosColor;			// 天体大气颜色
		double fRingMinRadius;			// 天体的光环内径，单位：米
		double fRingMaxRadius;			// 天体的光环外径，单位：米
	};

	/*************************************************************************
	Class
	*************************************************************************/
	class CGMTerrain;
	class CGMAtmosphere;
	class CGMEarth;
	class CGMPlanet;
	class CGMOort;
	class CGMDataManager;
	class CGMCelestialScaleVisitor;

	/*!
	*  @class CGMSolar
	*  @brief Galaxy-Music GMSolar
	*/
	class CGMSolar
	{
		// 函数
	public:
		/** @brief 构造 */
		CGMSolar();

		/** @brief 析构 */
		~CGMSolar();

		/** @brief 初始化 */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData,
			CGMCommonUniform* pCommonUniform, CGMDataManager* pDataManager);
		/** @brief 更新 */
		bool Update(double dDeltaTime);
		/** @brief 更新(在主相机更新姿态之后) */
		bool UpdateLater(double dDeltaTime);
		/** @brief 加载 */
		bool Load();
		/** @brief 保存太阳系此刻的信息 */
		bool SaveSolarData();

		/**
		* @brief 修改屏幕尺寸时调用此函数
		* @param iW: 屏幕宽度
		* @param iH: 屏幕高度
		*/
		void ResizeScreen(const int iW, const int iH);

		/**
		* @brief 创建太阳系
		* @return bool 成功true， 失败false
		*/
		bool CreateSolarSystem();

		/**
		* @brief 计算升级矩阵，用于层级空间+1的情况
		* @return Matrix: 升级矩阵
		*/
		osg::Matrix HierarchyAddMatrix() const;
		/**
		* @brief 计算降级矩阵，用于层级空间-1的情况
		* @return Matrix: 降级矩阵
		*/
		osg::Matrix HierarchySubMatrix() const;
		/**
		* @brief 获取查询位置附近最近的一颗天体（行星或恒星）的位置
		* @param vSearchHiePos 查询位置，当前空间层级坐标系
		* @param vPlanetHiePos 返回最近的天体位置，当前空间层级坐标系
		* @param fOrbitalPeriod 返回此行星公转轨道周期，如果是恒星则为0，单位：秒
		* @return bool 第3层级空间下，点击位置附近有天体则true，否则false
		*/
		bool GetNearestCelestialBody(const SGMVector3& vSearchHiePos,
			SGMVector3& vPlanetHiePos, double& fOrbitalPeriod);

		/**
		* @brief 获取目标点附近最近的天体（行星或恒星）的位置
		* @param vPlanetPos 返回最近的天体位置，单位：米
		* @param fOrbitalPeriod 返回此行星公转轨道周期，单位：秒
		*/
		void GetCelestialBody(SGMVector3& vPlanetPos, double& fOrbitalPeriod);

		/**
		* @brief 获取目标点附近最近的天体（行星或恒星）的平均半径
		* @return double 天体平均半径，单位：米
		*/
		double GetCelestialMeanRadius() const;
		/**
		* @brief 获取目标点附近最近的天体（行星或恒星）的指定纬度的海平面与球心距离
		* @param fLatitude 纬度，单位：°，范围：[-90.0, 90.0]
		* @return double 指定纬度的海平面与球心距离，单位：米
		*/
		double GetCelestialRadius(const double fLatitude) const;
		/**
		* @brief 获取目标点附近最近的天体（行星或恒星）的北极轴（当前层级空间下）
		* @return SGMVector3 北极轴（当前层级空间下）
		*/
		SGMVector3 GetCelestialNorth() const;
		/**
		* @brief 获取目标点附近最近的天体（行星或恒星）在本恒星系中的索引
		* @return unsigned int 0代表恒星，1代表公转轨道半径最小的天体
		*/
		unsigned int GetCelestialIndex() const;
		/**
		* @brief 获取眼睛海拔高度
		* @return double 眼睛海拔高度，单位：米
		*/
		double GetEyeAltitude() const;

		/**
		* @brief 获取太阳系相对于银河的旋转姿态
		* @return Quat 旋转姿态四元数
		*/
		inline osg::Quat GetSolarRotate() const
		{
			return m_qSolarRotate;
		}

		/**
		* @brief 设置超新星在当前空间层级下的位置
		* @param vHiePos 超新星位置，单位：当前空间层级单位
		*/
		void SetSupernovaHiePos(const osg::Vec3f& vHiePos);
		/**
		* @brief 设置“流浪地球计化”的进展
		* @param fProgress 进展百分比，[0.0, 1.0]
		*/
		void SetWanderingEarthProgress(const float fProgress);

		/**
		* @brief 由于空间层级变化而更新场景
		* @param iHierarchy:		更新后的空间层级编号
		* @return bool:				成功true，失败false */
		bool UpdateHierarchy(int iHierarchy);
		/**
		* @brief 更新当前行星/卫星相关信息，返回天体改变前后的位置差值
		* @param vTargetHiePos:		目标点在当前空间的位置
		* @return SGMVector3:		天体改变前后的位置差值，单位：米 */
		SGMVector3 UpdateCelestialBody(const SGMVector3& vTargetHiePos);

	private:
		/**
		* @brief 创建0/1/2层级空间的背景太阳
		*/
		void _CreateBackgroundSun();

		/**
		* @brief 创建超新星，一颗非常明亮的恒星
		*	不属于任何层级，播放音频时,在12345层级空间，始终可见
		* @return bool:			成功true，失败false
		*/
		bool _CreateSupernova();

		/**
		* @brief 创建第二层级空间的太阳
		* @return bool:			成功true，失败false
		*/
		bool _CreateSun();

		/**
		* @brief 创建2、3层级的行星,围绕恒星旋转
		* @author LiuTao
		* @since 2021.11.28
		* @param void：			无
		* @return bool:			成功true，失败false
		*/
		bool _CreatePlanets();

		/**
		* @brief 添加行星，围绕恒星旋转
		* @param fRadius：			公转半径，单位：米
		* @param fOrbitalPeriod:	公转周期，单位：年
		* @param fStartTheta:		公转起始位置，单位：弧度
		* @return bool:				成功true，失败false
		*/
		bool _AddPlanet(
			const double fRadius,
			const double fOrbitalPeriod,
			const double fStartTheta);

		/**
		* @brief 创建第1层级空间的行星系统，每颗行星有多层材质，行星环，卫星等
			区别于上面的_CreatePlanets
		* @return bool:			成功true，失败false
		*/
		bool _CreatePlanetSystem_1();
		/**
		* @brief 创建第2层级空间的行星系统，每颗行星有多层材质，行星环，卫星等
			区别于上面的_CreatePlanets
		* @return bool:			成功true，失败false
		*/
		bool _CreatePlanetSystem_2();

		/**
		* @brief 创建正方形
		* @param fWidth：			正方形边长
		* @param bCorner：			正方形中心点是否在角上，true == 在角上，false == 在中心
		* @return Geometry*:		创建的几何体节点指针，未成功则返回nullptr
		*/
		osg::Geometry* _CreateSquareGeometry(const float fWidth, const bool bCorner = false);

		/**
		* @brief 创建光环的某一块切片
		* @param iID					环的每个切片ID号，0,1,2,3,4,5
		* @return Geometry				返回创建的几何体指针
		*/
		osg::Geometry* _MakeRingGeometry(unsigned int iID);

		/**
		* @brief 修改光环半径
		* @param fRadiusMin				内半径，单位：米
		* @param fRadiusMax				外半径，单位：米
		*/
		void _SetRingRadius(double fRadiusMin, double fRadiusMax);

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

		osg::Texture* _Load3DShapeNoise() const;

		/**
		* @brief 修改天体大气颜色转换矩阵
		* @param vColor:			输入的大气颜色
		* @return osg::Matrixf:		输出的颜色转换矩阵
		*/
		osg::Matrixf _ColorTransMatrix(const osg::Vec4f& vColor) const;

		/**
		* @brief 初始化小行星带
		*/
		void _InitAsteroidBelt();
		/**
		* @brief 清洗小行星带数据，把重叠的小行星加速到一个随机的很高的速度，以实现清理目的
		*/
		void _WashAsteroidBeltData();

		/**
		* @brief 创建渲染面 屏幕两倍大小的三角面，比矩形效率要高一些
		* @param width: 等效矩形的宽度
		* @param height: 等效矩形的高度
		* @return osg::Geometry* 返回几何节点指针
		*/
		osg::Geometry* _CreateScreenTriangle(const int width, const int height);

		/**
		* @brief 更新天体自转相关姿态
		* @param dDeltaTime: 上一帧间隔时间，单位：秒
		*/
		void _UpdatePlanetRotate(double dDeltaTime);
		/**
		* @brief 获取某一时刻当前天体自转四元数
		* @return osg::Quat 当前天体自转四元数
		*/
		osg::Quat _GetPlanetSpin() const;
		/**
		* @brief 获取某一时刻当前天体自转轴倾角四元数
		* @return osg::Quat 当前天体自转轴倾角四元数
		*/
		osg::Quat _GetPlanetInclination() const;
		/**
		* @brief 获取某一时刻当前天体自转轴偏航角四元数（针对流浪地球）
		* @return osg::Quat 当前天体自转轴偏航角四元数
		*/
		osg::Quat _GetPlanetTurn() const;

		// 变量
	private:
		SGMKernelData*									m_pKernelData;					//!< 内核数据
		SGMConfigData*									m_pConfigData;					//!< 配置数据
		CGMCommonUniform*								m_pCommonUniform;				//!< 公共Uniform
		std::vector<osg::ref_ptr<osg::Group>>			m_pHieSolarRootVector;			//!< 0123空间层级的根节点

		osg::ref_ptr<osg::Texture>						m_3DShapeTex;					//!< 三维无缝噪声，RGBA，128^3
		// 天体数组
		std::vector<SGMCelestialBody>					m_sCelestialBodyVector;
		// 当前目标天体在数组中的顺序
		unsigned int									m_iCenterCelestialBody;
		std::default_random_engine						m_iRandom;						//!< 随机值

		std::string										m_strGalaxyShaderPath;			//!< galaxy shader 着色器路径
		std::string										m_strGalaxyTexPath;				//!< galaxy texture 贴图路径
		std::string										m_strCoreModelPath;				//!< 核心模型资源路径
		osg::ref_ptr<osg::Transform>					m_pStar_1_Transform;			//!< 第1层级中心恒星、行星变换结点
		osg::ref_ptr<osg::Transform>					m_pStar_2_Transform;			//!< 第2层级中心恒星、行星变换结点
		osg::ref_ptr<osg::Transform>					m_pPlanet_1_Transform;			//!< 第1层级当前行星变换结点
		osg::ref_ptr<osg::Transform>					m_pPlanet_2_Transform;			//!< 第2层级当前行星变换结点
		osg::ref_ptr<osg::Transform>					m_pRing_2_Transform;			//!< 第2层级行星光环变换结点
		osg::ref_ptr<osg::Transform>					m_pPlanetTailTransform;			//!< 行星尾迹的变换结点
		osg::ref_ptr<osg::AutoTransform>				m_pSunBloomTransform;			//!< 太阳辉光的变换结点
		osg::ref_ptr<osg::AutoTransform>				m_pBackgroundSunTransform;		//!< 0/1/2层级背景太阳变换结点
		osg::ref_ptr<osg::AutoTransform>				m_pSupernovaTransform;			//!< 超新星的变换结点

		osg::Matrixd									m_mView2ECEFMatrix;				//!< view 转 ECEF 的矩阵
		osg::Matrixd									m_mWorld2Shadow;				//!< 世界空间 转 阴影空间 的矩阵
		osg::Vec3d										m_vPlanetNorth;					//!< 世界空间下行星的北极轴
		osg::Vec3d										m_vPlanetAxisX;					//!< 世界空间下行星的X轴
		osg::Quat										m_qSolarRotate;					//!< 太阳系相对于银河系的姿态
		osg::Quat										m_qPlanetRotate;				//!< 当前天体相对于太阳系的姿态
		osg::Vec3d										m_vSolarPos_Hie1;				//!< 太阳在第1层级空间下的坐标
		osg::Vec3d										m_vSolarPos_Hie2;				//!< 太阳在第2层级空间下的坐标
		double											m_fEyeAltitude;					//!< 眼点海拔，单位：米
		float											m_fWanderingEarthProgress;		//!< 流浪地球计划进展[0.0,1.0]

		osg::ref_ptr<osg::Geometry>						m_pPlanetGeom_1;				//!< 第1层级行星共用球体
		osg::ref_ptr<osg::Geometry>						m_pPlanetGeom_2;				//!< 第2层级行星共用球体
		osg::ref_ptr<osg::Geode>						m_pGeodeSun_2;					//!< 第2层级太阳Geode
		osg::ref_ptr<osg::Geode>						m_pRingGeode_2;					//!< 第2层级行星光环Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeAsteroid_2;				//!< 第2层级小行星带点精灵Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePlanets_2;				//!< 第2层级行星点精灵Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePlanetsLine_2;			//!< 第2层级行星轨道线Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeAsteroid_3;				//!< 第3层级小行星带点精灵Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePlanets_3;				//!< 第3层级行星点精灵Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePlanetsLine_3;			//!< 第3层级行星轨道线Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeSupernovaX;				//!< 前景超新星的十字结点
		osg::ref_ptr<osg::Geode>						m_pGeodeSupernovaBloom;			//!< 前景超新星的辉光结点
		osg::ref_ptr<osg::Geode>						m_pGroundRoot;					//!< 行星地面的根节点
		osg::ref_ptr<osg::Geode>						m_pCloudRoot;					//!< 行星云层的根节点
		osg::ref_ptr<osg::Geode>						m_pAtmosRoot;					//!< 行星大气的根节点
		osg::ref_ptr<osg::StateSet>						m_pSSPlanetGround;				//!< 行星地面的状态集
		osg::ref_ptr<osg::StateSet>						m_pSSPlanetCloud;				//!< 行星云层的状态集
		osg::ref_ptr<osg::StateSet>						m_pSSPlanetAtmos;				//!< 行星大气的状态集

		osg::ref_ptr<osg::Uniform>						m_fBackgroundSunScaleUniform;	//!< 背景太阳缩放比例的Uniform
		osg::ref_ptr<osg::Uniform>						m_fBackgroundSunAlphaUniform;	//!< 背景太阳alpha的Uniform
		osg::ref_ptr<osg::Uniform>						m_fSunEdgeUniform;				//!< 太阳辉光板上的太阳边缘位置Uniform
		osg::ref_ptr<osg::Uniform>						m_fSupernovaLightUniform;		//!< 控制超新星的亮度的Uniform
		osg::ref_ptr<osg::Uniform>						m_fSupernovaAlphaUniform;		//!< 眩光（中心，刺状）的alpha
		osg::ref_ptr<osg::Uniform>						m_fSupernovaBloomScaleUniform;	//!< 控制超新星的bloom大小的Uniform
		osg::ref_ptr<osg::Uniform>						m_fSupernovaBloomAlphaUniform;	//!< 控制超新星的bloom不透明度的Uniform
		osg::ref_ptr<osg::Uniform>						m_fPlanetNumUniform;			//!< 行星数量的Uniform
		osg::ref_ptr<osg::Uniform>						m_fPlanetTailAlphaUniform;		//!< 行星尾迹线的alpha的Uniform
		osg::ref_ptr<osg::Uniform>						m_fPlanetLineAlphaUniform;		//!< 行星轨迹线的alpha的Uniform
		osg::ref_ptr<osg::Uniform>						m_vPlanetRadiusUniform;			//!< 行星的两个半径（第二层级）
		osg::ref_ptr<osg::Uniform>						m_fPlanetPointAlphaUniform;		//!< 行星点精灵的alpha的Uniform
		osg::ref_ptr<osg::Uniform>						m_vViewLightUniform;			//!< view空间的光源方向
		osg::ref_ptr<osg::Uniform>						m_mRingShadowMatrixUniform;		//!< 2层级光环转阴影空间的矩阵
		osg::ref_ptr<osg::Uniform>						m_mPlanetShadowMatrixUniform;	//!< 2层级行星转阴影空间的矩阵
		osg::ref_ptr<osg::Uniform>						m_fNorthDotLightUniform;		//!< 行星自转轴与阳光前进方向的点乘
		osg::ref_ptr<osg::Uniform>						m_fCloudTopUniform;				//!< 云层顶高
		osg::ref_ptr<osg::Uniform>						m_fGroundTopUniform;			//!< 地面最高山顶的海拔高
		osg::ref_ptr<osg::Uniform>						m_fAtmosHeightUniform;			//!< 大气层高度
		osg::ref_ptr<osg::Uniform>						m_fMinDotULUniform;				//!< 有光区域的最小DotUL值(-1,0)
		osg::ref_ptr<osg::Uniform>						m_fEyeAltitudeUniform;			//!< 眼点海拔Uniform
		osg::ref_ptr<osg::Uniform>						m_mAtmosColorTransUniform;		//!< 大气颜色转换矩阵
		osg::ref_ptr<osg::Uniform>						m_mView2ECEFUniform;			//!< view空间转ECEF的矩阵
		osg::ref_ptr<osg::Uniform>						m_fStepTimeUniform;				//!< 仿真步长的真实时间，单位：秒
		osg::ref_ptr<osg::Uniform>						m_vJupiterPosUniform;			//!< 木星在太阳系下的坐标，单位：米
		osg::ref_ptr<osg::Uniform>						m_vCoordScaleUniform;			//!< 球体贴图的纹理坐标缩放

		osg::ref_ptr<osgDB::Options>					m_pDDSOptions;					//!< dds的纹理操作
		osg::ref_ptr<osg::Texture2DArray>				m_aMercuryBaseTex;				//!< 水星base color纹理
		osg::ref_ptr<osg::Texture2DArray>				m_aVenusBaseTex;				//!< 金星base color纹理
		osg::ref_ptr<osg::Texture2DArray>				m_aVenusCloudTex;				//!< 金星云层纹理
		osg::ref_ptr<osg::Texture2DArray>				m_aMoonBaseTex;					//!< 月球base color纹理
		osg::ref_ptr<osg::Texture2DArray>				m_aMarsBaseTex;					//!< 火星base color纹理
		osg::ref_ptr<osg::Texture2DArray>				m_aJupiterCloudTex;				//!< 木星base color纹理
		osg::ref_ptr<osg::Texture>						m_pJupiterRingTex;				//!< 木星光环纹理
		osg::ref_ptr<osg::Texture2DArray>				m_aSaturnCloudTex;				//!< 土星base color纹理
		osg::ref_ptr<osg::Texture>						m_pSaturnRingTex;				//!< 土星光环纹理
		osg::ref_ptr<osg::Texture2DArray>				m_aUranusCloudTex;				//!< 天王星base color纹理
		osg::ref_ptr<osg::Texture>						m_pUranusRingTex;				//!< 天王星光环纹理
		osg::ref_ptr<osg::Texture2DArray>				m_aNeptuneCloudTex;				//!< 海王星base color纹理
		osg::ref_ptr<osg::Texture>						m_pNeptuneRingTex;				//!< 海王星光环纹理
		osg::ref_ptr<osg::Texture2DArray>				m_aPlutoBaseTex;				//!< 冥王星base color纹理
		osg::ref_ptr<osg::Texture2DArray>				m_aCharonBaseTex;				//!< 卡戎base color纹理
		//osg::ref_ptr<osg::Texture2DArray>				m_aAuroraTex;					//!< 极光纹理
		std::vector<osg::ref_ptr<osg::Vec3Array>>		m_ringVertVector;				//!< 光环顶点数组
		osg::ref_ptr<osg::Vec3Array>					m_pPlanetLineVerts_2;			//!< 第2层级行星轨迹线顶点数组
		osg::ref_ptr<osg::Vec3Array>					m_pPlanetLineCoords_2;			//!< 第2层级行星轨迹线UVW数组
		osg::ref_ptr<osg::DrawElementsUShort>			m_pPlanetLineElement_2;			//!< 第2层级行星轨迹线元素
		osg::ref_ptr<osg::Vec3Array>					m_pPlanetLineVerts_3;			//!< 第3层级行星轨迹线顶点数组
		osg::ref_ptr<osg::Vec3Array>					m_pPlanetLineCoords_3;			//!< 第3层级行星轨迹线UVW数组
		osg::ref_ptr<osg::DrawElementsUShort>			m_pPlanetLineElement_3;			//!< 第3层级行星轨迹线元素

		osg::ref_ptr<osg::Vec4Array>					m_pPlanetVertArray_2;			//!< 第2层级行星顶点数组
		osg::ref_ptr<osg::Vec2Array>					m_pPlanetVertCoords_2;			//!< 第2层级行星顶点纹理坐标
		osg::ref_ptr<osg::DrawElementsUShort>			m_pPlanetElement_2;				//!< 第2层级行星元素
		osg::ref_ptr<osg::Vec4Array>					m_pPlanetVertArray_3;			//!< 第3层级行星顶点数组
		osg::ref_ptr<osg::Vec2Array>					m_pPlanetVertCoords_3;			//!< 第3层级行星顶点纹理坐标
		osg::ref_ptr<osg::DrawElementsUShort>			m_pPlanetElement_3;				//!< 第3层级行星元素

		int												m_iPlanetCount;					//!< 行星数量计数器

		osg::ref_ptr<CGMDispatchCompute>				m_pAsteroidComputeNode;			//!< 计算小行星带的CS节点
		osg::ref_ptr<osg::Camera>						m_pReadAsteroidCam;				//!< 用于读取小行星带的相机
		CReadPixelFinishCallback*						m_pReadPixelFinishCallback;
		CGMCelestialScaleVisitor*						m_pCelestialScaleVisitor;		//!< 用于控制天体大小

		CGMTerrain*										m_pTerrain;						//!< 地形模块
		CGMAtmosphere*									m_pAtmos;						//!< 大气模块
		CGMEarth*										m_pEarth;						//!< 地球模块(单独成模块)
		CGMPlanet*										m_pPlanet;						//!< 行星模块（还包括卫星）
		CGMOort*										m_pOort;						//!< 奥尔特云模块
	};
}	// GM