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

#include "GMNebula.h"
#include <random>
#include <osg/Node>
#include <osg/AutoTransform>
#include <osg/Texture>
#include <osg/Texture2DArray>
#include <osg/Depth>
#include <osgUtil/CullVisitor>
#include <osg/CullFace>
#include <osg/BlendEquation>

namespace GM
{
	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/
	class osg::EllipsoidModel;

	class CCosmosBox : public osg::Transform
	{
	public:
		CCosmosBox()
		{
			setReferenceFrame(osg::Transform::ABSOLUTE_RF);
			osg::ref_ptr<osg::StateSet> pStateSetCosmosBox = getOrCreateStateSet();
			pStateSetCosmosBox->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			pStateSetCosmosBox->setMode(GL_BLEND, osg::StateAttribute::ON);
			osg::ref_ptr<osg::BlendEquation> blendEqua = new osg::BlendEquation(osg::BlendEquation::RGBA_MAX);
			pStateSetCosmosBox->setAttributeAndModes(blendEqua.get(), osg::StateAttribute::ON);
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
	class CGMGalaxy : public CGMNebula
	{
		// 函数
	public:
		/** @brief 构造 */
		CGMGalaxy();

		/** @brief 析构 */
		~CGMGalaxy();

		/** @brief 初始化 */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData);
		/** @brief 更新 */
		bool Update(double dDeltaTime);
		/** @brief 更新(在主相机更新姿态之后) */
		bool UpdateLater(double dDeltaTime);
		/** @brief 加载 */
		bool Load();

		/** @brief 开启/关闭编辑模式 */
		void SetEditMode(const bool bEnable);
		/** @brief 获取编辑模式 */
		bool GetEditMode();
		/** @brief 开启/关闭捕获功能 */
		void SetCapture(const bool bEnable);

		/**
		* Welcome
		* @brief 开启“欢迎效果”
		* @brief 每次开启软件，Engine调用此函数以实现“欢迎功能”
		* @author LiuTao
		* @since 2021.07.24
		* @param void
		* @return void
		*/
		void Welcome();

		/**
		* CreateGalaxy
		* 创建星系
		* @author LiuTao
		* @since 2020.11.27
		* @param fDiameter:	星系直径,单位：米
		* @return bool 成功true， 失败false
		*/
		bool CreateGalaxy(double fDiameter);

		/**
		* SetMousePosition
		* 将当前鼠标空间层级坐标传入
		* @author LiuTao
		* @since 2021.07.04
		* @param vWorldPos:		当前帧鼠标的空间层级坐标
		* @return void
		*/
		void SetMousePosition(const osg::Vec3d& vHierarchyPos);

		/**
		* SetCurrentStar
		* 设置当前激活的恒星
		* @author LiuTao
		* @since 2021.07.04
		* @param vWorldPos:		当前激活的恒星的世界空间坐标，单位：米
		* @param wstrName：		当前播放的音频名称，XXX.mp3
		* @return void
		*/
		void SetCurrentStar(const osg::Vec3d& vWorldPos, const std::wstring& wstrName);

		/**
		* SetAudioLevel
		* 设置当前帧音频的振幅值
		* @author LiuTao
		* @since 2021.07.04
		* @param fLevel 振幅值 [0.0f,1.0f]
		* @return void
		*/
		void SetAudioLevel(float fLevel);

		/**
		* GetStarWorldPos
		* 获取当前播放的音频星世界空间坐标
		* @author LiuTao
		* @since 2021.07.30
		* @param void
		* @return osg::Vec3d	当前播放的音频星世界空间坐标
		*/
		osg::Vec3d GetStarWorldPos();

		/**
		* UpdateHierarchy
		* @brief 由于空间层级变化而更新场景
		* @author LiuTao
		* @since 2021.09.15
		* @param iHierarchy:		空间层级编号
		* @return bool:				成功true，失败false
		*/
		bool UpdateHierarchy(int iHierarchy);

	private:

		/**
		* _CreateSupernova
		* 创建超新星，一颗非常明亮的恒星
		* 不属于任何层级，播放音频时,在12345层级空间，始终可见
		* @author LiuTao
		* @since 2021.10.06
		* @return bool:			成功true，失败false
		*/
		bool _CreateSupernova();

		/**
		* _CreateStarCube
		* 创建PointSprite恒星盒，用于0123级空间
		* 在一个正方体中创建点模式恒星
		* @author LiuTao
		* @since 2021.10.01
		* @return bool:			成功true，失败false
		*/
		bool _CreateStarCube();

		/**
		* _CreateSun
		* 创建1、2层级的太阳
		* @author LiuTao
		* @since 2022.01.02
		* @param void：			无
		* @return bool:			成功true，失败false
		*/
		bool _CreateSun();

		/**
		* _CreatePlanets
		* 创建2、3层级的行星,围绕恒星旋转
		* @author LiuTao
		* @since 2021.11.28
		* @param void：			无
		* @return bool:			成功true，失败false
		*/
		bool _CreatePlanets();

		/**
		* _AddPlanet
		* 添加行星，围绕恒星旋转
		* @author LiuTao
		* @since 2022.01.01
		* @param fRadius：			公转半径，单位：米
		* @param fOrbitalPeriod:	公转周期，单位：年
		* @param fStartPos:			公转起始位置，单位：弧度
		* @return bool:				成功true，失败false
		*/
		bool _AddPlanet(
			const double fRadius,
			const double fOrbitalPeriod,
			const double fStartPos);

		/**
		* _CreateOortCloud
		* 创建奥尔特云，用于3级空间
		* 一个球面，半径2光年
		* @author LiuTao
		* @since 2021.10.12
		* @return bool:			成功true，失败false
		*/
		bool _CreateOortCloud();

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
		* @author LiuTao
		* @since 2021.10.06
		* @return bool:			成功true，失败false
		*/
		bool _CreateGalaxies_4();

		/**
		* _CreateGalaxyPlane_5
		* 创建平面星系，用于5级空间
		* @author LiuTao
		* @since 2021.09.17
		* @return bool:			成功true，失败false
		*/
		bool _CreateGalaxyPlane_5();

		/**
		* _CreateGalaxies_5
		* 创建PointSprite星系群，用于5级空间
		* 在一个正方体中创建星系群，然后根据相机位置平移每个星系
		* @author LiuTao
		* @since 2021.10.06
		* @return bool:			成功true，失败false
		*/
		bool _CreateGalaxies_5();

		/**
		* _CreateSupercluster
		* 创建超星系团
		* 在一个正方体中创建超星系团
		* @author LiuTao
		* @since 2021.10.16
		* @return bool:			成功true，失败false
		*/
		bool _CreateSupercluster();
		/**
		* _CreateUltracluster
		* 创建究极星系团（瞎编的名称）
		* 在一个正方体中创建究极星系团
		* @author LiuTao
		* @since 2021.10.16
		* @return bool:			成功true，失败false
		*/
		bool _CreateUltracluster();

		/**
		* _CreateMyWorld
		* 创建我的世界
		* @author LiuTao
		* @since 2021.10.16
		* @return bool:			成功true，失败false
		*/
		bool _CreateMyWorld();

		/**
		* _CreateGalaxyBackground
		* 创建背景银河系环形
		* @author LiuTao
		* @since 2021.10.08
		* @return bool:			成功true，失败false
		*/
		bool _CreateGalaxyBackground();

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
		* _CreateAudioGeometry
		* 读取数据并创建未激活状态的音频星几何体
		* @author LiuTao
		* @since 2021.07.11
		* @param coordVector：		音频星点星辰坐标Vector
		* @return Geometry*:		创建的几何体节点指针，未成功则返回nullptr
		*/
		osg::Geometry* _CreateAudioGeometry(std::vector<SGMStarCoord>& coordVector);

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
		* _CreateHelpLineGeometry
		* 创建辅助线,辅助修改音频坐标
		* @author LiuTao
		* @since 2021.07.24
		* @param void：				无
		* @return Geometry*:		创建的几何体节点指针，未成功则返回nullptr
		*/
		osg::Geometry* _CreateHelpLineGeometry();

		/**
		* _MakeEllipsoidGeometry
		* @author LiuTao
		* @since 2020.06.22
		* @param ellipsoid				用于描述天体的椭球模型
		* @param iLonSegments			经度分段数
		* @param iLatSegments			纬度分段数
		* @param fHae					外延半径（类似于海拔高度）
		* @param bGenTexCoords			是否生成UV坐标
		* @param bWholeMap				是否显示完整贴图(在半球或者复杂曲面时)
		* @param bFlipNormal			true 法线向内，false 法线向外
		* @param fLonStart				经度开始位置，单位：角度 °
		* @param fLonEnd				经度结束位置，单位：角度 °
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
			float						fLonStart = -180.0,
			float						fLonEnd = 180.0,
			float						fLatStart = -90.0,
			float						fLatEnd = 90.0);

		/**
		* _GetGalaxyValue
		* 获取星系图的RGBA通道值，channel为0/1/2/3，对应RGBA。
		* @author LiuTao
		* @since 2020.11.27
		* @param fX:		图像x坐标,[0,1]
		* @param fY:		图像y坐标,[0,1]
		* @param iChannel:	0、1、2、3，对应R、G、B、A通道
		* @param bLinear:	是否双线性插值，true = 双线性，false = 临近值
		* @return float：	[0.0,1.0]
		*/
		float _GetGalaxyValue(float fX, float fY, int iChannel, bool bLinear = false);

		/**
		* _GetGalaxyHeight
		* 获取星系的厚度信息
		* @author LiuTao
		* @since 2020.11.27
		* @param fX:		图像x坐标,[0,1]
		* @param fY:		图像y坐标,[0,1]
		* @return float：	[0.0,1.0]
		*/
		float _GetGalaxyHeight(float fX, float fY);

		/**
		* _Get3DValue
		* 获取三维图片的某位置的线性插值后的值，repeat模式
		* @author LiuTao
		* @since 2021.09.26
		* @param fX:		图像x坐标,无限制
		* @param fY:		图像y坐标,无限制
		* @param fZ:		图像z坐标,无限制
		* @return float：	[0.0,1.0]
		*/
		float _Get3DValue(float fX, float fY, float fZ);

		/**
		* _Get3DValue
		* 获取三维图片的某unsigned int位置的值。只能获取“noiseShape128.tga”这张特殊的三维噪声图。
		* 这张图片为128*4096，rgba分别表示4段三维噪声值，先遍历每一层，再遍历每个通道
		* @author LiuTao
		* @since 2021.09.26
		* @param iX:		图像x坐标,[0,127]
		* @param iY:		图像y坐标,[0,127]
		* @param iZ:		图像z坐标,[0,127]
		* @return float：	[0.0,1.0]
		*/
		float _Get3DValue(unsigned int iX, unsigned int iY, unsigned int iZ);

		/**
		* _GetPhotoColor
		* 获取照片的颜色，不插值
		* @author LiuTao
		* @since 2021.10.16
		* @param fX:			图像x坐标,[0,1]
		* @param fY:			图像y坐标,[0,1]
		* @return osg::Vec4f：	Vec3[0.0,1.0]
		*/
		osg::Vec4f _GetPhotoColor(float fX, float fY);

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
		* 加载cubeMap纹理，自动加载文件夹下的指定命名规则的六张jpg图片
		* @author LiuTao
		* @since 2020.06.16
		* @param strFolder:			图片文件夹路径,例如 ./XX/
		* @param strFilePrefix:		图片文件名公用前缀
		* @return osg::Texture*		返回cubeMap纹理指针
		*/
		osg::Texture* _ReadCubeMap(const std::string& strFolder, const std::string& strFilePrefix);

		/**
		* _getRandomStarColor
		* @brief 获取一个随机且合理的恒星颜色
		* @author LiuTao
		* @since 2021.10.05
		* @return osg::Vec3f		返回一个随机且合理的恒星颜色，[0.0,1.0]
		*/
		osg::Vec3f _getRandomStarColor();

		// 变量
	private:
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

		std::default_random_engine						m_iRandom;

		std::string										m_strGalaxyShaderPath;			//!< galaxy shader 着色器路径
		std::string										m_strGalaxyTexPath;				//!< galaxy texture 贴图路径
		std::wstring									m_strPlayingStarName;			//!< 激活的音频星文件名,XXX.mp3
		osg::ref_ptr<osg::Transform>					m_pPlayingStarTransform;		//!< 激活的音频星的变换结点
		osg::ref_ptr<osg::Transform>					m_pStar_2_Transform;			//!< 第2层级中心恒星、行星变换结点
		osg::ref_ptr<osg::Transform>					m_pStar_3_Transform;			//!< 第3层级中心恒星、奥尔特云变换结点
		osg::ref_ptr<osg::Transform>					m_pOort_4_Transform;			//!< 第4层级奥尔特云变换结点
		osg::ref_ptr<osg::Transform>					m_pEye_4_Transform;				//!< 第4层级眼点的变换结点
		osg::ref_ptr<osg::Transform>					m_pEye_5_Transform;				//!< 第5层级眼点的变换结点
		osg::ref_ptr<osg::Transform>					m_pGalaxyGroup_Transform;		//!< 最小星系群的变换结点
		osg::ref_ptr<osg::AutoTransform>				m_pSunBloomTransform;			//!< 太阳辉光的变换结点
		osg::ref_ptr<osg::AutoTransform>				m_pMyWorld_5_AutoTrans;			//!< 5级空间我的世界自动变换结点
		osg::ref_ptr<osg::AutoTransform>				m_pMyWorld_6_AutoTrans;			//!< 6级空间我的世界自动变换结点
		osg::ref_ptr<osg::AutoTransform>				m_pSupernovaTransform;			//!< 超新星的变换结点
		SGMAudioCoord									m_vPlayingAudioCoord;			//!< 激活的音频星的音频空间坐标
		osg::Vec3d										m_vPlayingStarWorldPos;			//!< 激活的音频星世界空间坐标
		osg::ref_ptr<osg::Geode>						m_pGeodeHelpLine;				//!< 辅助修改音频坐标的线Geode	
		osg::ref_ptr<osg::Geode>						m_pGeodeAudio;					//!< 未激活的音频星Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeSun_2;					//!< 第2层级太阳Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePlanets_2;				//!< 第2层级行星点精灵Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePlanetsLine_2;			//!< 第2层级行星轨道线Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePlanets_3;				//!< 第3层级行星点精灵Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePlanetsLine_3;			//!< 第3层级行星轨道线Geode
		osg::ref_ptr<osg::Geode>						m_pOortCloudGeode_3;			//!< 第3层级奥尔特云Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeStarCube;				//!< 第3层级恒星盒的Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePointsN_4;				//!< 第4层级N倍点星星的Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeStarCube_4;				//!< 第4层级恒星盒的Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeGalaxyGroup_4;			//!< 第4层级星系群的Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeGalaxyGroup_5;			//!< 第5层级最小星系群的Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeGalaxies_5;				//!< 第5层级级联星系的Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeSupercluster;			//!< 超星系团的Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeUltracluster;			//!< 究极星系团的Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeMyWorld_5;				//!< 我的5级世界的Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeMyWorld_6;				//!< 我的6级世界的Geode
		osg::ref_ptr<osg::Geode>						m_pGalaxyBackgroundGeode;		//!< 银河系背景的Geode
		osg::ref_ptr<osg::Geode>						m_pCosmosBoxGeode;				//!< 宇宙盒的Geode
		osg::ref_ptr<osg::StateSet>						m_pStateSetGalaxy;				//!< 点模式星系的状态集
		osg::ref_ptr<osg::StateSet>						m_pStateSetPlayingAudio;		//!< 激活的音频星状态集
		osg::ref_ptr<osg::StateSet>						m_pStateSetPlane;				//!< 银河盘面的状态集	
		osg::ref_ptr<osg::Uniform>						m_pMousePosUniform;				//!< 鼠标的世界空间坐标Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarHiePosUniform;			//!< 激活的音频星的当前空间坐标Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarAudioPosUniform;			//!< 当前激活的音频空间坐标Uniform
		osg::ref_ptr<osg::Uniform>						m_pTimesUniform;				//!< 时间Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarColorUniform;			//!< 当前颜色Uniform
		osg::ref_ptr<osg::Uniform>						m_pLevelArrayUniform;			//!< 振幅数组Uniform
		osg::ref_ptr<osg::Uniform>						m_pGalaxyRadiusUniform;			//!< 第4层级单位下星系半径Uniform
		osg::ref_ptr<osg::Uniform>						m_pEyePos4Uniform;				//!< 第4层级眼点坐标Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarsCubeInfoUniform;		//!< cube恒星传入参数Uniform
		osg::ref_ptr<osg::Uniform>						m_pCubeCenterUniform;			//!< 3层级cube星中心偏移Uniform
		osg::ref_ptr<osg::Uniform>						m_pShapeUVWUniform;				//!< 三维噪声UVW系数的Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarAlphaUniform;			//!< 远处恒星的alpha的Uniform
		osg::ref_ptr<osg::Uniform>						m_pGalaxiesAlphaUniform;		//!< 远处星系的alpha的Uniform
		osg::ref_ptr<osg::Uniform>						m_pGalaxiesInfoUniform;			//!< cube星系传入参数Uniform
		osg::ref_ptr<osg::Uniform>						m_pSupernovaLightUniform;		//!< 控制超新星的亮度的Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarDistanceUniform;			//!< 控制0123层级的恒星的最远距离Uniform
		osg::ref_ptr<osg::Uniform>						m_pUnitRatioUniform;			//!< 当前层级与第3层级的单位比例Uniform
		osg::ref_ptr<osg::Uniform>						m_pMyWorldAlphaUniform;			//!< 我的世界的alpha的Uniform
		osg::ref_ptr<osg::Texture>						m_pGalaxyColorTex;				//!< 银河系颜色和alpha纹理
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
		osg::ref_ptr<osg::Vec3Array>					m_pPlanetLineVerts_2;			//!< 第2层级行星轨迹线顶点数组
		osg::ref_ptr<osg::Vec3Array>					m_pPlanetLineCoords_2;			//!< 第2层级行星轨迹线UVW数组
		osg::ref_ptr<osg::DrawElementsUShort>			m_pPlanetLineElement_2;			//!< 第2层级行星轨迹线元素
		osg::ref_ptr<osg::Vec3Array>					m_pPlanetLineVerts_3;			//!< 第3层级行星轨迹线顶点数组
		osg::ref_ptr<osg::Vec3Array>					m_pPlanetLineCoords_3;			//!< 第3层级行星轨迹线UVW数组
		osg::ref_ptr<osg::DrawElementsUShort>			m_pPlanetLineElement_3;			//!< 第3层级行星轨迹线元素
		osg::ref_ptr<osg::Vec4Array>					m_pPlanetVertArray_2;			//!< 第2层级行星顶点数组
		osg::ref_ptr<osg::DrawElementsUShort>			m_pPlanetElement_2;				//!< 第2层级行星元素
		osg::ref_ptr<osg::Vec4Array>					m_pPlanetVertArray_3;			//!< 第3层级行星顶点数组
		osg::ref_ptr<osg::DrawElementsUShort>			m_pPlanetElement_3;				//!< 第3层级行星元素

		int												m_iPlanetCount;					//!< 行星数量计数器

		bool											m_bEdit;						//!< 是否开启编辑模式
		bool											m_bCapture;						//!< 是否开启捕捉模式
		bool											m_bWelcome;						//!< 是否启用欢迎功能
	};
}	// GM