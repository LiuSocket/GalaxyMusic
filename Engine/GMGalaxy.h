//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMGalaxy.h
/// @brief		Galaxy-Music Engine - GMGalaxy
/// @version	1.0
/// @author		LiuTao
/// @date		2020.11.27
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommon.h"
#include "GMKernel.h"

#include <random>
#include <osg/Node>
#include <osg/AutoTransform>
#include <osg/Texture>
#include <osg/Texture2DArray>
#include <osg/Depth>
#include <osg/BlendFunc>
#include <osg/CullFace>
#include <osgUtil/CullVisitor>

namespace GM
{
	/*************************************************************************
	 Enums
	*************************************************************************/
	/*!
	 *  @enum EGMArrowDir
	 *  @brief 箭头方向枚举
	 */
	enum EGMArrowDir
	{
		EGMAD_NONE,				//!< 未指定朝向
		EGMAD_RADIUS_OUT,		//!< 朝向 半径 外方向
		EGMAD_RADIUS_IN,		//!< 朝向 半径 内方向
		EGMAD_ANGLE_PLUS,		//!< 朝向 角度 正方向
		EGMAD_ANGLE_MINUS		//!< 朝向 角度 负方向
	};

	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/
	class osg::EllipsoidModel;
	class CGMMilkyWay;
	class CGMSolar;
	class CGMDataManager;
	class CGMCommonUniform;

	class CCosmosBox : public osg::Transform
	{
	public:
		CCosmosBox()
		{
			setReferenceFrame(osg::Transform::ABSOLUTE_RF);
			osg::ref_ptr<osg::StateSet> pStateSetCosmosBox = getOrCreateStateSet();
			pStateSetCosmosBox->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			pStateSetCosmosBox->setMode(GL_BLEND, osg::StateAttribute::ON);
			pStateSetCosmosBox->setAttributeAndModes(new osg::BlendFunc(
				GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
			), osg::StateAttribute::ON);
			pStateSetCosmosBox->setAttributeAndModes(new osg::CullFace());
			pStateSetCosmosBox->setAttributeAndModes(new osg::Depth(osg::Depth::GEQUAL, 1.0, 1.0));
		}

		CCosmosBox(const CCosmosBox& copy, osg::CopyOp copyop =
			osg::CopyOp::SHALLOW_COPY)
			: osg::Transform(copy, copyop) {}
		META_Node(osg, CCosmosBox);

		virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const
		{
			if (nv && nv->getVisitorType() ==
				osg::NodeVisitor::CULL_VISITOR)
			{
				osgUtil::CullVisitor* cv =
					static_cast<osgUtil::CullVisitor*>(nv);
				matrix.preMult(osg::Matrix::translate(cv->getEyeLocal()));
				return true;
			}
			else
				return osg::Transform::computeLocalToWorldMatrix(matrix, nv);
		}

		virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix,
			osg::NodeVisitor* nv) const
		{
			if (nv && nv->getVisitorType() ==
				osg::NodeVisitor::CULL_VISITOR)
			{
				osgUtil::CullVisitor* cv =
					static_cast<osgUtil::CullVisitor*>(nv);
				matrix.postMult(osg::Matrix::translate(
					-cv->getEyeLocal()));
				return true;
			}
			else
				return osg::Transform::computeWorldToLocalMatrix(matrix, nv);
		}

	protected:
		virtual ~CCosmosBox() {}
	};

	/*!
	*  @class CGMGalaxy
	*  @brief Galaxy-Music GMGalaxy
	*/
	class CGMGalaxy
	{
		// 函数
	public:
		/** @brief 构造 */
		CGMGalaxy();

		/** @brief 析构 */
		~CGMGalaxy();

		/** @brief 初始化 */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData,
			CGMCommonUniform* pCommonUniform, CGMDataManager* pDataManager);
		/** @brief 更新 */
		bool Update(double dDeltaTime);
		/** @brief 更新(在主相机更新姿态之后) */
		bool UpdateLater(double dDeltaTime);
		/** @brief 加载 */
		bool Load();
		/** @brief 保存 */
		bool SaveSolarData();

		/**
		* 修改屏幕尺寸时调用此函数
		* @author LiuTao
		* @since 2022.11.19
		* @param iW: 屏幕宽度
		* @param iH: 屏幕高度
		* @return void
		*/
		void ResizeScreen(const int iW, const int iH);

		/** @brief 开启/关闭编辑模式 */
		void SetEditMode(const bool bEnable);
		/** @brief 获取编辑模式 */
		inline bool GetEditMode() const
		{
			return m_bEdit;
		}
		/** @brief 开启/关闭抓手的悬浮状态 */
		void SetHandleHover(const bool bHover);
		/** @brief 开启/关闭捕获功能,开启后鼠标可以捕获当前播放的音频星 */
		void SetCapture(const bool bEnable);

		/**
		* @brief 开启“欢迎效果”
		* 每次开启软件，Engine调用此函数以实现“欢迎功能”
		*/
		void Welcome();

		/**
		* @brief 创建星系
		* @param fDiameter:	星系直径,单位：米
		* @return bool 成功true， 失败false
		*/
		bool CreateGalaxy(double fDiameter);

		/**
		* @brief 将当前鼠标空间层级坐标传入
		* @param vWorldPos:		当前帧鼠标的空间层级坐标
		*/
		void SetMousePosition(const osg::Vec3d& vHierarchyPos);

		/**
		* @brief 设置当前激活的恒星
		* @param vWorldPos:		当前激活的恒星的世界空间坐标，单位：米
		* @param wstrName：		当前播放的音频名称，XXX.mp3
		*/
		void SetCurrentStar(const osg::Vec3d& vWorldPos, const std::wstring& wstrName);

		/**
		* @brief 设置当前播放的恒星的音频空间坐标
		* @param vAudioCoord:	当前播放的恒星的音频空间坐标
		*/
		void SetPlayingStarAudioCoord(const SGMAudioCoord& vAudioCoord);

		/**
		* 获取当前播放的恒星的音频空间坐标
		* @return vAudioCoord:	当前播放的恒星的音频空间坐标
		*/
		inline SGMAudioCoord GetPlayingStarAudioCoord() const
		{
			return m_vPlayingAudioCoord;
		}

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
		* @brief 获取当前播放的音频星世界空间坐标
		* @return osg::Vec3d	当前播放的音频星世界空间坐标
		*/
		inline osg::Vec3d GetStarWorldPos() const
		{
			return m_vPlayingStarWorld4Pos;
		}

		/**
		* @brief 由于空间层级变化而更新场景
		* @param iHierarchyNew:		更新后的空间层级编号
		* @return bool:				成功true，失败false
		*/
		bool UpdateHierarchy(int iHierarchyNew);
		/**
		* @brief 更新当前行星/卫星相关信息，返回天体改变前后的位置差值
		* @param vTargetHiePos:		目标点在当前空间的位置
		* @return SGMVector3:		天体改变前后的位置差值，单位：米
		*/
		SGMVector3 UpdateCelestialBody(const SGMVector3& vTargetHiePos);
		/**
		* @brief 刷新最近的恒星在银河系坐标系（4级空间的世界坐标系）的位置
		*/
		void RefreshNearStarWorldPos();
		/**
		* @brief 获取最近的恒星在银河系坐标系（4级空间的世界坐标系）的位置
		* @return osg::Vec3d:		最近的恒星在银河系坐标系（4级空间的世界坐标系）的位置
		*/
		inline osg::Vec3d GetNearStarWorldPos() const
		{
			return m_vNearStarWorld4Pos;
		}
		/**
		* @brief 获取最近的恒星在银河系坐标系（4级空间的世界坐标系）的姿态（旋转）
		* @return osg::Quat:		姿态四元数
		*/
		osg::Quat GetNearStarRotate() const;

	private:
		/**
		* _InitBackground
		* 初始化背景相关节点
		* @author LiuTao
		* @since 2022.08.07
		* @return void:
		*/
		void _InitBackground();

		/** _CreateStarCube
		* 创建PointSprite恒星盒，用于0123级空间
		* 在一个正方体中创建点模式恒星
		* @return bool:			成功true，失败false
		*/
		bool _CreateStarCube();

		/**
		* _CreateHandle
		* 创建用于移动恒星的圆盘状的把手
		* @author LiuTao
		* @since 2022.01.23
		* @return bool:			成功true，失败false
		*/
		bool _CreateHandle();

		/**
		* _CreateAudioPoints
		* 创建音频星点
		* @author LiuTao
		* @since 2020.12.05
		* @return bool:			成功true，失败false
		*/
		bool _CreateAudioPoints();

		/**
		* _CreateGalaxyPoints
		* 创建PointSprite星系
		* @author LiuTao
		* @since 2020.12.05
		* @return bool:			成功true，失败false
		*/
		bool _CreateGalaxyPoints();

		/**
		* _CreateStarCube_4
		* 创建PointSprite恒星盒，用于4级空间
		* 在一个正方体中创建星星，然后根据相机位置平移每颗星星
		* @author LiuTao
		* @since 2021.10.01
		* @return bool:			成功true，失败false
		*/
		bool _CreateStarCube_4();

		/**
		* _CreateGalaxyPointsN_4
		* 创建N倍密度的PointSprite星系，用于4级空间
		* 例如：输入2倍密度，星星的范围直径就缩小到星系的1/2,但高度范围不变
		* @author LiuTao
		* @since 2021.09.26
		* @param iDens：		星星的密度倍数，只处理2、4、8、16倍数
		* @return bool:			成功true，失败false
		*/
		bool _CreateGalaxyPointsN_4(int iDens);

		/**
		* _CreateGalaxyPlane_4
		* 创建平面星系，用于4级空间
		* @author LiuTao
		* @since 2020.12.05
		* @return bool:			成功true，失败false
		*/
		bool _CreateGalaxyPlane_4();

		/**
		* _CreateGalaxies_4
		* 创建PointSprite星系群，用于4级空间,和第5级空间的最小星系群无缝衔接
		* 完美融合，实现乾坤大挪移
		* 在一个球面上创建星系群，球面中心始终在相机位置
		* 每个星系都和5级空间的星系完全吻合
		* @return bool:			成功true，失败false
		*/
		bool _CreateGalaxies_4();

		/**
		* _CreateGalaxyPlane_5
		* 创建平面星系，用于5级空间
		* @return bool:			成功true，失败false
		*/
		bool _CreateGalaxyPlane_5();

		/**
		* _CreateGalaxies_5
		* 创建PointSprite星系群，用于5级空间
		* 在一个正方体中创建星系群，然后根据相机位置平移每个星系
		* @return bool:			成功true，失败false
		*/
		bool _CreateGalaxies_5();

		/**
		* _CreateSupercluster
		* 创建超星系团
		* 在一个正方体中创建超星系团
		* @return bool:			成功true，失败false
		*/
		bool _CreateSupercluster();
		/**
		* _CreateUltracluster
		* 创建究极星系团（瞎编的名称）
		* 在一个正方体中创建究极星系团
		* @return bool:			成功true，失败false
		*/
		bool _CreateUltracluster();

		/**
		* _CreateMyWorld
		* 创建我的世界
		* @return bool:			成功true，失败false
		*/
		bool _CreateMyWorld();

		/** _CreateBackgroundGalaxy
		* 创建背景银河系环形结构
		* @return bool:			成功true，失败false
		*/
		bool _CreateBackgroundGalaxy();

		/**
		* _CreateCosmosBox
		* 创建背景宇宙盒，用于背景
		* @author LiuTao
		* @since 2021.08.22
		* @return bool:			成功true，失败false
		*/
		bool _CreateCosmosBox();

		/**
		* _DetachAudioPoints
		* 将输入的音频星点分离
		* @author LiuTao
		* @since 2021.07.11
		* @return bool:				成功true，失败false
		*/
		bool _DetachAudioPoints();

		/**
		* _AttachAudioPoints
		* 合并当前编辑的音频星点
		* @author LiuTao
		* @since 2021.07.11
		* @return bool:				成功true，失败false
		*/
		bool _AttachAudioPoints();

		/**
		* _UpdatePlayingStarInformation
		* @brief 根据输入的音频空间坐标，更新当前播放的音频星信息
		* @author LiuTao
		* @since 2022.02.16
		* @param sAudioCoord:		音频空间坐标
		* @return bool:				成功true，如果不合法，则返回false
		*/
		bool _UpdatePlayingStarInformation(const SGMAudioCoord& sAudioCoord);

		/**
		* _CreateAudioGeometry
		* 读取数据并创建音频星几何体
		* @author LiuTao
		* @since 2021.07.11
		* @param audioMap：			完整的音频数据map
		* @param iDiscardUID:		因为激活而需要被剔除的音频星UID，默认0表示没有剔除的音频
		* @return Geometry*:		创建的几何体节点指针，未成功则返回nullptr
		*/
		osg::Geometry* _CreateAudioGeometry(
			std::map<unsigned int, SGMAudioData>& audioMap,
			const unsigned int iDiscardUID = 0);

		/**
		* _CreateConeGeometry
		* 创建圆锥体
		* @author LiuTao
		* @since 2021.07.18
		* @param void：				无
		* @return Geometry*:		创建的几何体节点指针，未成功则返回nullptr
		*/
		osg::Geometry* _CreateConeGeometry();

		/**
		* _CreateSquareGeometry
		* 创建正方形
		* @author LiuTao
		* @since 2022.01.23
		* @param fWidth：			正方形边长
		* @param bCorner：			正方形中心点是否在角上，true == 在角上，false == 在中心
		* @return Geometry*:		创建的几何体节点指针，未成功则返回nullptr
		*/
		osg::Geometry* _CreateSquareGeometry(const float fWidth, const bool bCorner = false);

		/**
		* _CreateRegionGeometry
		* 创建音频分区几何体
		* @return Geometry*:		创建的几何体节点指针，未成功则返回nullptr
		*/
		osg::Geometry* _CreateRegionGeometry();

		/**
		* @brief 创建椭圆
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
		osg::Geometry* _MakeEllipsoidGeometry(
			const osg::EllipsoidModel*	ellipsoid,
			int							iLonSegments,
			int							iLatSegments,
			float						fHae,
			bool						bGenTexCoords,
			bool						bWholeMap = false,
			bool						bFlipNormal = false,
			float						fLatStart = -90.0,
			float						fLatEnd = 90.0) const;

		/**
		* @brief 创建长方体
		* @param fLength			长度
		* @param fWidth				宽度
		* @param fHeight			高度
		* @return Geometry			返回创建的几何体指针
		*/
		osg::Geometry* _MakeBoxGeometry(
			const float fLength = 10.0f,
			const float fWidth = 10.0f,
			const float fHeight = 2.0f) const;

		/**
		* @brief 获取三维图片的某位置的线性插值后的值，repeat模式
		* @param fX:		图像x坐标,无限制
		* @param fY:		图像y坐标,无限制
		* @param fZ:		图像z坐标,无限制
		* @return float：	[0.0,1.0]
		*/
		float _Get3DValue(float fX, float fY, float fZ);

		/**
		* @brief 获取三维图片的某unsigned int位置的值。只能获取“noiseShape128.tga”这张特殊的三维噪声图。
		* 这张图片为128*4096，rgba分别表示4段三维噪声值，先遍历每一层，再遍历每个通道
		* @param iX:		图像x坐标,[0,127]
		* @param iY:		图像y坐标,[0,127]
		* @param iZ:		图像z坐标,[0,127]
		* @return float：	[0.0,1.0]
		*/
		float _Get3DValue(unsigned int iX, unsigned int iY, unsigned int iZ);

		/**
		* @brief 音频空间坐标转音频区域UV
		* @param fX:			图像x坐标,[0,1]
		* @return osg::Vec2f		返回一个随机且合理的恒星颜色，[0.0,1.0]
		*/
		osg::Vec2f _AudioCoord2UV(const SGMAudioCoord& sAudioCoord) const;

		/**
		* 加载2D纹理
		* @author LiuTao
		* @since 2020.06.16
		* @param fileName: 图片文件路径
		* @param iChannelNum: 1、2、3、4分别代表R、RG、RGB、RGBA
		* @return osg::Texture* 返回纹理指针
		*/
		osg::Texture* _CreateTexture2D(const std::string& fileName, const int iChannelNum) const;
		/**
		* 加载2D纹理
		* @author LiuTao
		* @since 2023.03.04
		* @param pImg:		图片指针
		* @param iChannelNum: 1、2、3、4分别代表R、RG、RGB、RGBA
		* @return osg::Texture* 返回纹理指针
		*/
		osg::Texture* _CreateTexture2D(osg::Image* pImg, const int iChannelNum) const;

		/**
		* 加载DDS纹理
		* @author LiuTao
		* @since 2023.01.08
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
		* 加载cubeMap纹理，自动加载文件夹下的指定命名规则的六张jpg图片
		* @author LiuTao
		* @since 2020.06.16
		* @param strFolder:			图片文件夹路径,例如 ./XX/
		* @param strFilePrefix:		图片文件名公用前缀
		* @return osg::Texture*		返回cubeMap纹理指针
		*/
		osg::Texture* _ReadCubeMap(const std::string& strFolder, const std::string& strFilePrefix) const;

		/**
		* _GetRandomStarColor
		* @brief 获取一个随机且合理的恒星颜色
		* @author LiuTao
		* @since 2021.10.05
		* @return osg::Vec3f		返回一个随机且合理的恒星颜色，[0.0,1.0]
		*/
		osg::Vec3f _GetRandomStarColor();

		/**
		* @brief 获取两个角度之间的夹角
		* @author LiuTao
		* @since 2022.02.10
		* @param fA:			角度A，单位：弧度 (-PI, PI]
		* @param fB:			角度B，单位：弧度 (-PI, PI]
		* @return double:		A - B 的夹角，单位：弧度 (-PI, PI]
		*/
		double _IncludedAngle(const double fA, const double fB) const;

		/**
		* @brief 生成伪随机噪声图
		* @author LiuTao
		* @since 2022.05.15
		* @param void:
		* @return void:
		*/
		void _MakePseudoNoise();

		osg::Texture* _Load3DShapeNoise() const;

		/**
		* @brief SGMVector3 转 osg::Vec3d
		* @param vGM:				输入的GM向量
		* @return osg::Vec3d:		输出的osg向量
		*/
		inline osg::Vec3d _GM2OSG(const SGMVector3& vGM) const
		{
			return osg::Vec3d(vGM.x, vGM.y, vGM.z);
		}

		// 变量
	private:
		SGMKernelData*									m_pKernelData;					//!< 内核数据
		SGMConfigData*									m_pConfigData;					//!< 配置数据
		CGMCommonUniform*								m_pCommonUniform;				//!< 公共Uniform
		CGMDataManager*									m_pDataManager;					//!< 数据管理模块
		std::vector<osg::ref_ptr<osg::Group>>			m_pHierarchyRootVector;			//!< 每个空间层级的根节点

		//!< 星系半径，单位：米
		double											m_fGalaxyRadius;
		/** 星系厚度图片：
		** R通道		/
		** G通道		/
		** B通道		整体厚度
		** Alpha通道	/
		*/
		osg::ref_ptr<osg::Image>						m_pGalaxyHeightImage;
		/** 星系数据图片：
		** RGB通道		颜色
		** Alpha通道	密度
		*/
		osg::ref_ptr<osg::Image>						m_pGalaxyImage;
		/** 三维噪声图片：
		** RGBA通道		噪声灰度
		** 特殊的三维噪声，用二维图片的4个通道存储
		*/
		osg::ref_ptr<osg::Image>						m_shapeImg;
		/** 照片的图片：
		** RGB通道		颜色
		** Alpha通道	无
		*/
		osg::ref_ptr<osg::Image>						m_pPhotoImage;

		std::string										m_strGalaxyShaderPath;			//!< galaxy shader 着色器路径
		std::string										m_strGalaxyTexPath;				//!< galaxy texture 贴图路径
		std::string										m_strCoreModelPath;				//!< 核心模型资源路径
		std::wstring									m_strPlayingStarName;			//!< 激活的音频星文件名,XXX.mp3
		osg::ref_ptr<osg::Transform>					m_pStarInfoTransform;			//!< 激活恒星信息展示的变换结点
		osg::ref_ptr<osg::Transform>					m_pStar_4_Transform;			//!< 第4层级把手的变换结点
		osg::ref_ptr<osg::Transform>					m_pEye_4_Transform;				//!< 第4层级眼点的变换结点
		osg::ref_ptr<osg::Transform>					m_pEye_5_Transform;				//!< 第5层级眼点的变换结点
		osg::ref_ptr<osg::Transform>					m_pGalaxyGroup_Transform;		//!< 最小星系群的变换结点
		osg::ref_ptr<osg::Transform>					m_pBackgroundGalaxyTransform;	//!< 背景银河系的变换结点
		osg::ref_ptr<osg::Transform>					m_pBackgroundStarTransform;		//!< 背景恒星的变换结点
		osg::ref_ptr<osg::AutoTransform>				m_pMyWorld_5_AutoTrans;			//!< 5级空间我的世界自动变换结点
		osg::ref_ptr<osg::AutoTransform>				m_pMyWorld_6_AutoTrans;			//!< 6级空间我的世界自动变换结点
		osg::ref_ptr<osg::Switch>						m_pHandleSwitch;				//!< 把手的开关结点

		unsigned int									m_iPlayingAudioUID;				//!< 激活的音频星的UID
		SGMAudioCoord									m_vPlayingAudioCoord;			//!< 激活的音频星的音频空间坐标
		osg::Vec3d										m_vPlayingStarWorld4Pos;		//!< 激活的音频星4级世界空间坐标
		osg::Vec3d										m_vNearStarWorld4Pos;			//!< 最近恒星4级世界空间位置
		osg::Vec3d										m_vMouseWorldPos;				//!< 鼠标世界空间坐标
		osg::Vec3d										m_vMouseLastWorldPos;			//!< 鼠标上一帧世界空间坐标
		osg::Matrixd									m_mLastVP;						//!< 上一帧Raymarch相机的ViewProjectionMatrix

		osg::ref_ptr<osg::Geode>						m_pGeodeRegion;					//!< 音频区域Geode	
		osg::ref_ptr<osg::Geode>						m_pGeodeAudio;					//!< 未激活的音频星Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePointsN_4;				//!< 第4层级N倍点星星的Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeStarCube_4;				//!< 第4层级恒星盒的Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeGalaxyGroup_4;			//!< 第4层级星系群的Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeGalaxyGroup_5;			//!< 第5层级最小星系群的Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeGalaxies_5;				//!< 第5层级级联星系的Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeSupercluster;			//!< 超星系团的Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeUltracluster;			//!< 究极星系团的Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeMyWorld_5;				//!< 我的5级世界的Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeMyWorld_6;				//!< 我的6级世界的Geode
		osg::ref_ptr<osg::Geode>						m_pCosmosBoxGeode;				//!< 宇宙盒的Geode
		osg::ref_ptr<osg::StateSet>						m_pStateSetGalaxy;				//!< 点模式星系的状态集
		osg::ref_ptr<osg::StateSet>						m_pStateSetPlayingAudio;		//!< 激活的音频星状态集
		osg::ref_ptr<osg::StateSet>						m_pStateSetPlane;				//!< 银河盘面的状态集

		osg::ref_ptr<osg::Uniform>						m_pMousePosUniform;				//!< 鼠标的世界空间坐标Uniform
		osg::ref_ptr<osg::Uniform>						m_pAudioUVUniform;				//!< 当前音频在区域内的UV
		osg::ref_ptr<osg::Uniform>						m_pGalaxyRadiusUniform;			//!< 第4层级单位下星系半径Uniform
		osg::ref_ptr<osg::Uniform>						m_pEyePos4Uniform;				//!< 第4层级眼点坐标Uniform
		osg::ref_ptr<osg::Uniform>						m_fGalaxyHeightUniform;			//!< 星系的单边高度Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarsCubeInfoUniform;		//!< cube恒星传入参数Uniform
		osg::ref_ptr<osg::Uniform>						m_pCubeCenterUniform;			//!< 第3层级cube恒星中心偏移
		osg::ref_ptr<osg::Uniform>						m_pShapeUVWUniform;				//!< 三维噪声UVW系数的Uniform
		osg::ref_ptr<osg::Uniform>						m_fStarAlphaUniform;			//!< 远处恒星的alpha的Uniform
		osg::ref_ptr<osg::Uniform>						m_fGalaxiesAlphaUniform;		//!< 远处星系的alpha的Uniform
		osg::ref_ptr<osg::Uniform>						m_pGalaxiesInfoUniform;			//!< cube星系传入参数Uniform
		osg::ref_ptr<osg::Uniform>						m_fStarDistanceUniform;			//!< 控制0123层级的恒星的最远距离
		osg::ref_ptr<osg::Uniform>						m_fMyWorldAlphaUniform;			//!< 我的世界的alpha的Uniform

		osg::ref_ptr<osgDB::Options>					m_pDDSOptions;					//!< dds的纹理操作
		osg::ref_ptr<osg::Texture>						m_pGalaxyColorTex;				//!< 银河系颜色和alpha纹理
		osg::ref_ptr<osg::Texture>						m_3DShapeTex;					//!< 三维无缝shape噪声，RGBA，128^3
		osg::ref_ptr<osg::Texture2DArray>				m_pGalaxiesTex;					//!< 星系纹理数组
		osg::ref_ptr<osg::Vec4Array>					m_pSphereVertArray;				//!< sphere点模式恒星顶点
		osg::ref_ptr<osg::Vec4Array>					m_pSphereColorArray;			//!< sphere点模式恒星颜色
		osg::ref_ptr<osg::DrawElementsUShort>			m_pSphereElement;				//!< sphere点模式恒星元素
		osg::ref_ptr<osg::Vec4Array>					m_pCubeVertArray;				//!< cube点模式恒星顶点
		osg::ref_ptr<osg::Vec4Array>					m_pCubeColorArray;				//!< cube点模式恒星颜色
		osg::ref_ptr<osg::DrawElementsUShort>			m_pCubeElement;					//!< cube点模式恒星元素
		std::vector<osg::ref_ptr<osg::Geometry>>		m_pStarsGeomVector;				//!< cube点模式恒星几何体容器
		osg::ref_ptr<CCosmosBox>						m_pCosmosBoxNode;				//!< 宇宙盒结点，有裁剪回调
		osg::ref_ptr<osg::Vec4Array>					m_pGalaxiesInfo;				//!< 星系旋转枚举(0123)和纹理编号
		std::vector<osg::ref_ptr<osg::Geometry>>		m_pGalaxiesGeomVector;			//!< 星系几何体容器
				
		float											m_fArrowAngle;					//!< 指示箭头的旋转角度，弧度
		float											m_fPRPA;						//!< +Radius+Angle锐角平均值，弧度
		EGMArrowDir										m_eArrowDir;					//!< 箭头方向枚举
		bool											m_bEdit;						//!< 是否开启编辑模式
		bool											m_bHandleHover;					//!< 抓手是否处于悬浮状态
		bool											m_bCapture;						//!< 是否开启捕捉模式
		bool											m_bWelcome;						//!< 是否启用欢迎功能
		std::default_random_engine						m_iRandom;						//!< 随机值

		CGMMilkyWay*									m_pMilkyWay;					//!< 银河系的体渲染模块
		CGMSolar*										m_pSolarSystem;					//!< 太阳系模块
	};
}	// GM