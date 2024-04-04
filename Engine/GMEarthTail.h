//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMEarthTail.h
/// @brief		Galaxy-Music Engine - GMEarthTail.h
/// @version	1.0
/// @author		LiuTao
/// @date		2023.08.06
//////////////////////////////////////////////////////////////////////////
#pragma once
#include "GMVolumeBasic.h"
#include "GMCommonUniform.h"

namespace GM
{
	/*************************************************************************
	 Macro Defines
	*************************************************************************/

	/*************************************************************************
	constexpr
	*************************************************************************/
	constexpr double EARTH_TAIL_RADIUS = 7.2e6;		// 尾迹半径，单位：米
	constexpr double EARTH_TAIL_LENGTH = 4e7;		// 尾迹长度（从地球球心开始算起），单位：米

	/*************************************************************************
	Enums
	*************************************************************************/

	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/

	/*
	** 地球尾迹画布盒的访问器
	*/
	class CEarthTailBoxVisitor : public osg::NodeVisitor
	{
	public:
		CEarthTailBoxVisitor() : NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
			_fUnit(1e10), _iPeriod(0) {}

		void SetUnit(const double fUnit)
		{
			_fUnit = fUnit;
		}

		/**
		* @brief 设置流浪地球计划的阶段
		* @param iWanderPeriod
		* 0 = 前太阳时代
		* 1 = 刹车时代
		* 2 = 加速转向时代
		* 3 = 减速转向时代
		* 4 = 逃逸时代
		*/
		void SetPeriod(const int iPeriod)
		{
			_iPeriod = iPeriod;
		}

		void apply(osg::Node& node) { traverse(node); }
		void apply(osg::Geode& node)
		{
			osg::Geometry* geom = dynamic_cast<osg::Geometry*>(node.getDrawable(0));
			if (!geom) return;

			osg::ref_ptr<osg::Vec3Array> pVert = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
			if (!pVert.valid()) return;

			switch (_iPeriod)
			{
			case 1:
			{
				float fR = 1.2e7 / _fUnit;
				float fZ = 6.4e6 / _fUnit;

				pVert->at(0) = osg::Vec3(-fR, -fR, -fZ); // 0
				pVert->at(1) = osg::Vec3(fR, -fR, -fZ); // 1
				pVert->at(2) = osg::Vec3(fR, fR, -fZ); // 2
				pVert->at(3) = osg::Vec3(-fR, fR, -fZ); // 3
				pVert->at(4) = osg::Vec3(-fR, -fR, fZ); // 4
				pVert->at(5) = osg::Vec3(fR, -fR, fZ); // 5
				pVert->at(6) = osg::Vec3(fR, fR, fZ); // 6
				pVert->at(7) = osg::Vec3(-fR, fR, fZ); // 7
			}
			break;
			case 2:
			case 3:
			{
				float fR = 1e7 / _fUnit;

				pVert->at(0) = osg::Vec3(-fR, -fR, -fR); // 0
				pVert->at(1) = osg::Vec3(fR, -fR, -fR); // 1
				pVert->at(2) = osg::Vec3(fR, fR, -fR); // 2
				pVert->at(3) = osg::Vec3(-fR, fR, -fR); // 3
				pVert->at(4) = osg::Vec3(-fR, -fR, fR); // 4
				pVert->at(5) = osg::Vec3(fR, -fR, fR); // 5
				pVert->at(6) = osg::Vec3(fR, fR, fR); // 6
				pVert->at(7) = osg::Vec3(-fR, fR, fR); // 7
			}
			break;
			case 4:
			{
				float fR = EARTH_TAIL_RADIUS / _fUnit;
				float fL = EARTH_TAIL_LENGTH / _fUnit;

				pVert->at(0) = osg::Vec3(-fR, -fR, -fR); // 0
				pVert->at(1) = osg::Vec3(fR, -fR, -fR); // 1
				pVert->at(2) = osg::Vec3(fR, fR, -fR); // 2
				pVert->at(3) = osg::Vec3(-fR, fR, -fR); // 3
				pVert->at(4) = osg::Vec3(-fR, -fR, fL); // 4
				pVert->at(5) = osg::Vec3(fR, -fR, fL); // 5
				pVert->at(6) = osg::Vec3(fR, fR, fL); // 6
				pVert->at(7) = osg::Vec3(-fR, fR, fL); // 7
			}
			break;
			default:
				break;
			}

			pVert->dirty();

			traverse(node);
		}
	private:
		double				_fUnit;
		int					_iPeriod; // 流浪地球计划所处阶段
	};

	/*!
	*  @class CGMEarthTail
	*  @brief Galaxy-Music EarthTail Module
	*/
	class CGMEarthTail : public CGMVolumeBasic
	{
		// 函数
	public:
		/** @brief 构造 */
		CGMEarthTail();
		/** @brief 析构 */
		~CGMEarthTail();

		/** @brief 初始化 */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief 更新 */
		bool Update(double dDeltaTime);
		/** @brief 更新(在主相机更新姿态之后) */
		bool UpdateLater(double dDeltaTime);
		/** @brief 加载 */
		bool Load();
		/**
		* @brief 创建流浪地球尾迹
		*/
		void MakeEarthTail();

		/**
		* @brief 由于空间层级变化而更新场景
		* @param iHierarchy:		更新后的空间层级编号
		* @return bool:				成功true，失败false
		*/
		bool UpdateHierarchy(int iHieNew);

		/**
		* @brief 显示、隐藏流浪地球尾迹
		* @param bVisible: 是否可见
		*/
		void SetVisible(const bool bVisible);

		/**
		* @brief 设置流浪地球的尾迹在第2空间层级下的旋转
		* @param fSpin： 自转，单位：弧度
		* @param fObliquity： 自转轴倾角，单位：弧度
		* @param fNorthYaw： 自转轴偏航角，单位：弧度
		*/
		void SetEarthTailRotate(const double fSpin, const double fObliquity, const double fNorthYaw);

		/* @brief 传递Uniform */
		void SetUniform(
			osg::Uniform* pViewLight,
			osg::Uniform* pEngineStartRatio,
			osg::Uniform* pWorld2ECEF,
			osg::Uniform* pView2ECEF,
			osg::Uniform* pWanderProgress);

	private:
		/**
		* @brief 北极轴旋转时，发动机喷出的气体形成的螺旋
		* @param fRadius			地球半径
		* @param bPositive			气体是否用于正向加速地球旋转
		* @return Geometry			返回创建的几何体指针
		*/
		osg::Geometry* _MakeSpiralGeometry(const float fRadius, const bool bPositive) const;

		/**
		* @brief 创建地球尾迹长方体
		* @param fLength			地球尾迹长度
		* @param fRadius			地球大气头部半径
		* @return Geometry			返回创建的几何体指针
		*/
		osg::Geometry* _MakeTailBoxGeometry(const float fLength, const float fRadius) const;
		/**
		* @brief 创建尾迹包络
		* @param fLength			尾迹长度
		* @param fRadius			大气头部半径
		* @return Geometry			返回创建的几何体指针
		*/
		osg::Geometry* _MakeTailEnvelopeGeometry(const float fLength, const float fRadius) const;
		/**
		* @brief 创建圆柱体尾迹
		* @param fLength			尾迹长度
		* @param fRadius			尾迹半径
		* @return Geometry			返回创建的几何体指针
		*/
		osg::Geometry* _MakeTailCylinderGeometry(const float fLength, const float fRadius) const;
		/**
		* @brief 创建交叉面片尾迹
		* @param fLength			尾迹长度
		* @param fRadius			尾迹半径
		* @return Geometry			返回创建的几何体指针
		*/
		osg::Geometry* _MakeTailXGeometry(const float fLength, const float fRadius) const;
		/**
		* @brief 刹车时代，发动机喷出的气体形成的地球环
		* @param fRadius			地球半径
		* @return Geometry			返回创建的几何体指针
		*/
		osg::Geometry* _MakeEarthRingGeometry(const float fRadius);

		/**
		* @brief 初始化流浪地球尾迹状态集
		* @param pSS				需要设置的状态集指针
		* @param strShaderName		shader的名字
		* @return bool				成功true，错误false
		*/
		bool _InitEarthTailStateSet(osg::StateSet* pSS, const std::string strShaderName);

		/**
		* @brief 获取螺旋气体表面坐标
		* @param fCoordUV			螺旋气体表面UV坐标，范围[0.0,1.0]
		* @param fRadius			地球半径
		* @param i					螺旋气体是轴对称图形，i=0/1分别代表螺旋的两半
		* @param bPositive			气体是否用于正向加速地球旋转
		* @return osg::Vec3			表面顶点的模型空间的位置
		*/
		osg::Vec3 _GetSpiralSurfacePos(const osg::Vec2 fCoordUV, const float fRadius, const int i, const bool bPositive) const;
		/**
		* @brief 获取尾迹包络的表面坐标
		* @param fCoordUV			沿着尾迹方向的坐标，范围[0.0,1.0]
		* @param fLength			尾迹长度
		* @param fRadius			大气头部半径
		* @return osg::Vec3			表面顶点的模型空间的位置
		*/
		osg::Vec3 _GetTailEnvelopePos(const osg::Vec2 fCoordUV, const float fLength, const float fRadius) const;

		/**
		* @brief 设置流浪地球计划的阶段
		* @param iWanderPeriod
		* 0 = 前太阳时代
		* 1 = 刹车时代
		* 2 = 加速转向时代
		* 3 = 减速转向时代
		* 4 = 逃逸时代
		*/
		void _SetWanderPeriod(const int iWanderPeriod);

	// 变量
	private:
		std::string										m_strCoreGalaxyTexPath;			//!< 星系核心贴图路径
		std::string										m_strEarthShaderPath;			//!< Earth shader 路径
		std::string										m_strGalaxyShaderPath;			//!< 星系着色器路径

		osg::ref_ptr<osg::Transform>					m_pSpiralTransform2;			//!< 第2层级气体螺旋的变换节点
		osg::ref_ptr<osg::Transform>					m_pTailTransform2;				//!< 第2层级流浪地球尾迹的变换节点

		osg::ref_ptr<osg::Geode>						m_pEarthRingGeode2;				//!< 第2层级地球环的几何节点
		osg::ref_ptr<osg::Geode>						m_pSpiralPositiveGeode2;		//!< 第2层级正向螺旋的几何节点
		osg::ref_ptr<osg::Geode>						m_pSpiralNegativeGeode2;		//!< 第2层级反向螺旋的几何节点
		osg::ref_ptr<osg::Geode>						m_pTailBoxGeode2;				//!< 第2层级尾迹画布的几何节点
		osg::ref_ptr<osg::Geode>						m_pTailEnvelopeGeode2;			//!< 第2层级尾迹包络的几何节点

		osg::ref_ptr<osg::StateSet>						m_pSsTailDecFace;				//!< 流浪地球尾迹12面体面状态集
		osg::ref_ptr<osg::StateSet>						m_pSsTailDecEdge;				//!< 流浪地球尾迹12面体边状态集
		osg::ref_ptr<osg::StateSet>						m_pSsTailDecVert;				//!< 流浪地球尾迹12面体点状态集

		bool											m_bVisible;						//!< 流浪地球尾迹是否可见
		osg::ref_ptr<osg::Uniform>						m_fTailVisibleUniform;			//!< 流浪地球尾迹可见度的Uniform
		osg::ref_ptr<osg::Uniform>						m_mWorld2ECEFUniform;			//!< “2级世界空间”转 ECEF 的矩阵
		osg::ref_ptr<osg::Uniform>						m_mView2ECEFUniform;			//!< view空间转ECEF的矩阵
		osg::ref_ptr<osg::Uniform>						m_mWorld2SpiralUniform;			//!< “2级世界空间”转“螺旋尾迹空间”矩阵
		osg::ref_ptr<osg::Uniform>						m_mView2SpiralUniform;			//!< view空间转“螺旋尾迹空间”矩阵
		osg::ref_ptr<osg::Uniform>						m_vViewLightUniform;			//!< view空间的光源方向
		osg::ref_ptr<osg::Uniform>						m_fWanderProgressUniform;		//!< 流浪地球计划进展Uniform
		//!< 开启比例，x=转向，y=推进，z=0表示发动机正在逐步关闭，z=1表示发动机正在逐步开启
		osg::ref_ptr<osg::Uniform>						m_vEngineStartRatioUniform;

		osg::ref_ptr<CEarthTailBoxVisitor>				m_pEarthTailBoxVisitor;			//!< 地球尾迹的画布盒的访问器
	};

}	// GM