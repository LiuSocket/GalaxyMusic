//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMCameraManipulator.h
/// @brief		Galaxy-Music Engine - GMCameraManipulator
/// @version	1.0
/// @author		LiuTao
/// @date		2021.05.30
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommon.h"
#include <osgGA/CameraManipulator>

namespace GM
{
	/*************************************************************************
	 Global Constants
	*************************************************************************/

	/*************************************************************************
	 Enums
	*************************************************************************/
	// 当前相机飞行模式
	enum EGMC_FLY
	{
		EGMA_FLY_FREE,				// 自由漫游模式
		EGMA_FLY_CONTROL,			// 受控飞行模式,鼠标控制
		EGMA_FLY_GOTO,				// 目的飞行模式，飞往当前播放的音频星
		EGMA_FLY_HOME,				// 回家模式，回到初始地点（家）
		EGMA_FLY_JUMP,				// 跳跃模式，先回到初始地点（家），再跳跃到当前播放的音频星
		EGMA_FLY_ULTIMATE			// 终极模式，平滑地增大相机焦距，直到终极位置
	};

	/*************************************************************************
	 Class
	*************************************************************************/

	class CGMCameraManipulator : public osgGA::CameraManipulator
	{
	public:
		CGMCameraManipulator();

		/**
		 * Converts screen coordinates to world position.
		 *
		 * @param x, y
		 *      Viewport coordinates, unit:pixel, >= 0
		 * @param view
		 *      View for which to calculate world coordinates
		 * @param vHierarchy
		 *      Output Hierarchy position (only valid if the method returns true)
		 */
		bool screen2Hierarchy(float x, float y, osg::View* view, osg::Vec3d& vHierarchy) const;

		/**
		* GetHierarchyTargetDistance
		* 获取当前层级下的目标点距离
		* @author LiuTao
		* @since 2021.10.05
		* @param void
		* @return double		目标点距离，单位：当前层级单位
		*/
		double GetHierarchyTargetDistance();

	public: // osgGA::CameraManipulator

		virtual const char* className() const { return "GMCameraManipulator"; }

		/** set the position of the matrix manipulator using a 4x4 Matrix.*/
		virtual void setByMatrix(const osg::Matrixd& matrix);

		/** set the position of the matrix manipulator using a 4x4 Matrix.*/
		virtual void setByInverseMatrix(const osg::Matrixd& matrix) { setByMatrix(osg::Matrixd::inverse(matrix)); }

		/** get the position of the manipulator as 4x4 Matrix.*/
		virtual osg::Matrixd getMatrix() const;

		/** get the position of the manipulator as a inverse matrix of the manipulator, typically used as a model view matrix.*/
		virtual osg::Matrixd getInverseMatrix() const;

		//所有操作在这里响应
		virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	protected:

		virtual ~CGMCameraManipulator();

	private:
		/**
		* _InnerUpdate
		* 间隔更新
		* @author LiuTao
		* @since 2021.07.04
		* @param updateStep 两次间隔更新的时间差，单位s
		* @return void
		*/
		void _InnerUpdate(float updateStep);

		/**
		* @brief 进入特定飞行模式
		* @author LiuTao
		* @since 2021.08.07
		* @param eMode:			飞行模式
		* @return void:
		*/
		void _SetFlyMode(EGMC_FLY eMode);

		/**
		* @brief 映射函数
		* @author LiuTao
		* @since 2021.06.06
		* @param fX:					映射前的值
		* @param fMin, fMax:			映射前的范围
		* @param fMinNew, fMaxNew:		映射后的范围
		* @return double:				映射后的值
		*/
		inline double _Remap(const double fX,
			const double fMin, const double fMax,
			const double fMinNew, const double fMaxNew)
		{
			double fY = (osg::clampBetween(fX, fMin, fMax) - fMin) / (fMax - fMin);
			return fMinNew + fY * (fMaxNew - fMinNew);
		}

		/**
		* @brief 平滑过渡函数
		* @author LiuTao
		* @since 2021.09.03
		* @param fMin, fMax:			范围
		* @param fX:					映射前的值
		* @return double:				映射后的值
		*/
		inline double _Smoothstep(const double fMin, const double fMax, const double fX)
		{
			double y = osg::clampBetween((fX - fMin) / (fMax - fMin), 0.0, 1.0);
			return y * y * (3 - 2 * y);
		}

		/**
		* @brief 混合函数,参考 glsl 中的 mix(a,b,x)
		* @author LiuTao
		* @since 2021.08.29
		* @param fMin, fMax:			范围
		* @param fX:					混合系数
		* @return double:				混合后的值
		*/
		inline double _Mix(const double fMin, const double fMax, const double fX)
		{
			return fMin*(1 - fX) + fMax*fX;
		}

		/**
		* @brief 根据相机俯仰值获取相机平移值
		* @author LiuTao
		* @since 2021.06.06
		* @param vDirection:			相机方向单位向量
		* @return osg::Vec3d:			返回平移值
		*/
		osg::Vec3d _GetTranslate(const osg::Vec3d vDirection);

		/**
		* @brief 根据两个当前空间层级的坐标，获取角速度，仅用于控制操作器旋转速度
		* @author LiuTao
		* @since 2021.06.06
		* @param vHierarchyPos:			当前帧当前空间层级的坐标
		* @param vLastHierarchyPos:		上一帧当前空间层级的坐标
		* @return double:				返回角速度
		*/
		double _GetAngularVelocity(const osg::Vec3d vHierarchyPos, const osg::Vec3d vLastHierarchyPos);

	private:
		osg::Vec3d				m_vTranslate;               // 平移  
		osg::Quat				m_qRotate;					// 旋转
		float					m_fDeltaStep;				// 单位：s
		float					m_fConstantStep;			// 等间隔更新的时间,单位s
		float					m_fCursorStillTime;			// 鼠标静止的持续时间,单位s
		double					m_fTimeLastFrame;			// 上一帧时间，单位：s
		double					m_fTimeJumpStart;			// 开始跳跃飞行的时间，单位：s
		double					m_fTimeSinceJump;			// 起跳后经过的时间
		double					m_fJumpDuration;			// 跳跃总时长，根据跳跃点距离变化
		double					m_fCameraRadius;			// 相机所在球体半径
		double					m_fCameraStartRadius;		// 开始跳跃时，相机所在球体半径
		double					m_fRadiusAcceleration;		// 加速度
		double					m_fRadiusWeakRatio;			// 阻力造成的减速比例
		double					m_fConstSpinSpeed;			// 常量自旋速度（不受鼠标控制）
		double					m_fSpinSpeed;				// 当前自旋速度
		double					m_fPitchSpeed;				// 俯仰速度
		double					m_fTerminalPitch;			// 极限俯仰角
		double					m_fTerminalYaw;				// 极限偏航角
		double					m_fOriginPitch;				// 起始俯仰角
		double					m_fOriginYaw;				// 起始航向角，逆时针，[0, 2*Pi]
		double					m_fSpinAcceleration;		// 自旋角加速度，+=顺时针，-=逆时针
		double					m_fSpinWeakRatio;			// 自旋阻力造成的自旋减速比例

		double					m_fMixA;					// 混合系数A，与加速的强度负相关
		double					m_fMixB;					// 混合系数B，与减速的强度负相关
		double					m_fMixMax;					// 混合系数的最大值

		osg::Vec3d				m_vTargetHierarchyPos;			// 当前目标点的当前空间坐标
		osg::Vec3d				m_vTerminalHierarchyPos;		// 极限目标点的当前空间坐标
		osg::Vec3d				m_vOriginHierarchyPos;			// 起始目标点的当前空间坐标
		osg::Vec3d				m_vMouseHierarchyPos;			// 当前帧鼠标在银盘上的空间坐标
		osg::Vec3d				m_vLastMouseHierarchyPos;		// 上一帧鼠标在银盘上的空间坐标
		osg::Vec3d				m_vStarWorldPos;				// 播放的音频星世界空间位置
		osg::Vec2f				m_vMousePushScreenPos;			// 鼠标按下的XY坐标
		osg::Vec2f				m_vMouseLastScreenPos;			// 上一帧鼠标XY坐标
		osg::Vec2f				m_vMouseDragLastScreenPos;		// 上一帧鼠标拖拽的XY坐标
		EGMC_FLY				m_eFlyMode;						// 相机的飞行模式
		bool					m_bEdit;						// 是否处于编辑模式
		bool					m_bDrag;						// 鼠标拖拽标记
	};

}	// DP
