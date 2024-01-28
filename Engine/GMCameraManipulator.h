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

	// 当前相机操作器枚举
	enum EGMC_MANIPULATOR
	{
		// 观察操作器，可以在宇宙的0-6级任意穿梭，超时空
		EGMM_WATCH,
		// 驾驶操作器，参考《星际公民》的飞船驾驶方式
		// WSAD前后左右移动，QE左右滚转，X急刹车
		// Shift加速，Ctrl降低高度，Space抬升高度
		// 鼠标左键按下后移动，控制朝向
		EGMM_DRIVE
	};

	// 当前相机观察状态
	enum EGMC_WATCH
	{
		// 自由模式
		EGMC_WATCH_FREE,
		// 缩放模式，以鼠标点击位置为中心，拉进拉远相机
		// 当缩放中心是真空，且目标距离小于某个值后就不在缩小空间层级
		// 当相机在银盘内部时会限制最大移动速度
		EGMC_WATCH_ZOOM,
		// 移动模式，鼠标控制平移相机
		EGMC_WATCH_MOVE,
		// 目标穿梭模式，飞往当前播放的音频星
		EGMC_WATCH_GOTO,
		// 回家模式，回到初始地点（家）
		EGMC_WATCH_HOME,
		// 跳跃模式，先回到初始地点（家），再跳跃到当前播放的音频星
		EGMC_WATCH_JUMP,
		// 终极模式，平滑地增大相机焦距，直到终极位置
		EGMC_WATCH_ULTIMATE
	};

	/*************************************************************************
	Structs
	*************************************************************************/
	// 座驾的参数结构体
	struct SGMDriveParam
	{
		SGMDriveParam()
			: vSpeed(0.0, 0.0, 0.0),
			fAccelerationFront(0.0), fAccelerationBack(0.0),
			fAccelerationLeft(0.0), fAccelerationRight(0.0),
			fAccelerationUp(0.0), fAccelerationDown(0.0),
			fRoll(0.0), fPitch(0.0), fYaw(0.0),
			bPitchOrYaw(false), bRollLeft(false), bRollRight(false),
			bShift(false), bBrakeMode(false), 
			fRoll_Max(osg::PI*2), vSpeed_Max(3e7, 3e7, 3e7),
			vAcceleration_Normal(98, 98, 200), vAcceleration_Max(980, 980, 2000)
		{}

		// 速度，飞船右手坐标系下，xyz = 右上后，单位：m/s
		osg::Vec3d					vSpeed;
		// 驱动向前的加速度，单位：m/s2，>= 0
		double						fAccelerationFront;
		// 驱动向后的加速度，单位：m/s2，>= 0
		double						fAccelerationBack;
		// 驱动向左的加速度，单位：m/s2，>= 0
		double						fAccelerationLeft;
		// 驱动向右的加速度，单位：m/s2，>= 0
		double						fAccelerationRight;
		// 驱动向上的加速度，单位：m/s2，>= 0
		double						fAccelerationUp;
		// 驱动向下的加速度，单位：m/s2，>= 0
		double						fAccelerationDown;
		// 滚转角速度，单位：弧度/s
		double						fRoll;
		// 俯仰角速度，单位：弧度/s
		double						fPitch;
		// 偏航角速度，单位：弧度/s
		double						fYaw;
		// 是否在俯仰或偏航
		double						bPitchOrYaw;
		// 是否左滚转
		bool						bRollLeft;
		// 是否右滚转
		bool						bRollRight;
		// 是否加速
		bool						bShift;
		// 紧急制动模式，开启后会以最大动力将速度降到0
		bool						bBrakeMode;

		// 滚转最大角速度，单位：弧度/s
		double						fRoll_Max;
		// 最大速度，飞船坐标系下，xyz = 右上后，单位：m/s
		osg::Vec3d					vSpeed_Max;
		// 巡航加速度，飞船坐标系下，xyz = 右上后，单位：m/s2
		osg::Vec3d					vAcceleration_Normal;
		// 最大加速度，飞船坐标系下，xyz = 右上后，单位：m/s2
		osg::Vec3d					vAcceleration_Max;
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
		 * @param vHieOutPos
		 *      Output Hierarchy position of(x,y)
		 */
		bool screen2Hierarchy(const float x, const float y, osg::View* view, osg::Vec3d& vHieOutPos) const;

		/**
		* GetHierarchyTargetDistance
		* 获取当前层级下的目标点距离
		* @return double		目标点距离，单位：当前层级单位
		*/
		inline double GetHierarchyTargetDistance() const
		{
			return m_fCameraRadius;
		}
		/**
		* GetHierarchyTargetPos
		* 获取当前层级的目标点位置
		* @return osg::Vec3d	目标点位置，单位：当前层级单位
		*/
		inline osg::Vec3d GetHierarchyTargetPos() const
		{
			return m_vTargetHierarchyPos;
		}

		/**
		* GetHierarchyEyePos
		* 获取当前层级的眼点位置，层级切换时，该位置与相机位置可能会相差一帧
		* @return osg::Vec3d	目标点位置，单位：当前层级单位
		*/
		inline osg::Vec3d GetHierarchyEyePos() const
		{
			return m_vTranslate;
		}
		
		/**
		* ChangeCenter
		* @brief 由于中心天体改变而改变相机姿态信息
		* @author LiuTao
		* @since 2023.01.16
		* @param vDeltaPos: 天体改变前后的位置差值，单位：米
		* @return void:
		*/
		void ChangeCenter(const SGMVector3& vDeltaPos);

		/**
		* UpdateHierarchy
		* @brief 由于空间层级变化而更新场景
		* @author LiuTao
		* @since 2022.11.20
		* @param void
		* @return bool:		如果切换了空间层级则返回true；没有切换则返回false
		*/
		bool UpdateHierarchy();

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
		* @param updateStepTime 两次间隔更新的时间差，单位s
		* @return void
		*/
		void _InnerUpdate(const float updateStepTime);

		/**
		* _ObjectiveUpdate
		* 客观事实的更新函数，与飞行模式无关
		* @author LiuTao
		* @since 2023.04.22
		* @param ea
		* @param aa
		* @return void
		*/
		void _ObjectiveUpdate(const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa);
		/**
		* @brief 各个飞行模式下，单独更新的内容
		* @param ea
		* @param aa
		*/
		void _FlyUpdate(const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa);
		/**
		* @brief 设置操作器模式
		* @param eMode:		操作器模式枚举
		*/
		void _SetManipulatorMode(const EGMC_MANIPULATOR eMode);
		/**
		* @brief 进入特定观察状态
		* @param eWatch:		观察状态
		*/
		void _SetWatchState(const EGMC_WATCH eWatch);
		/**
		* @brief 设置俯仰和偏航角
		* @param fXnormalized:		归一化屏幕X坐标，[-1.0,1.0]
		* @param fYnormalized:		归一化屏幕Y坐标，[-1.0,1.0]
		*/
		void _SetPitchYaw(const float fXnormalized, const float fYnormalized);

		/**
		* @brief 获取过渡比例
		* @param fT:			时间比例，[0.0, 1.0]
		* @param eMode:			飞行模式
		*/
		double _GetMix(const double fT, const EGMC_WATCH eMode);
		/**
		* @brief 设置过渡函数
		* @param fTScale:			时间缩放比例，大于零
		*/
		void _SetMix(const double fTScale);

		/**
		* @brief 映射函数
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
		* @param fMin, fMax:			范围
		* @param fX:					混合系数
		* @return double/Vec3d:			混合后的值
		*/
		inline double _Mix(const double fMin, const double fMax, const double fX)
		{
			return fMin*(1 - fX) + fMax*fX;
		}
		inline osg::Vec3d _Mix(const osg::Vec3d& vMin, const osg::Vec3d& vMax, const double fX)
		{
			return vMin * (1 - fX) + vMax * fX;
		}

		/**
		* @brief 获取相机平移值
		* @return osg::Vec3d: 返回平移值
		*/
		osg::Vec3d _GetTranslate() const;
		/**
		* @brief 目标点是否为真空
		* @param iHie:			当前空间层级
		* @return bool:			目标点是否为真空
		*/
		bool _IsTargetInVacuum(const int iHie) const;
		/**
		* @brief 避免输入点穿地，如果穿地，就强行拉起
		* @param vHiePos:		当前空间层级下的点坐标
		* @return bool:			点是否已经穿地
		*/
		bool _IsIntoGround(osg::Vec3d& vHiePos) const;

		/**
		* @brief SGMVector3 转 osg::Vec3d
		* @param vGM:				输入的GM向量
		* @return osg::Vec3d:		输出的osg向量
		*/
		inline osg::Vec3d _GM2OSG(const SGMVector3& vGM) const
		{
			return osg::Vec3d(vGM.x, vGM.y, vGM.z);
		}
		/**
		* @brief SGMVector4 转 osg::Vec4d
		* @param vGM:				输入的GM向量
		* @return osg::Vec4d:		输出的osg向量
		*/
		inline osg::Vec4d _GM2OSG(const SGMVector4& vGM) const
		{
			return osg::Vec4d(vGM.x, vGM.y, vGM.z, vGM.w);
		}
		/**
		* @brief SGMVector3 转 osg::Vec3d
		* @param vGM:				输入的GM向量
		* @return osg::Vec3d:		输出的osg向量
		*/
		inline SGMVector3 _OSG2GM(const osg::Vec3d& vOSG) const
		{
			return SGMVector3(vOSG.x(), vOSG.y(), vOSG.z());
		}
		/**
		* @brief SGMVector4 转 osg::Vec4d
		* @param vGM:				输入的GM向量
		* @return osg::Vec4d:		输出的osg向量
		*/
		inline SGMVector4 _OSG2GM(const osg::Vec4d& vOSG) const
		{
			return SGMVector4(vOSG.x(), vOSG.y(), vOSG.z(), vOSG.w());
		}

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
		double					m_fCameraStartRadius;		// 开始跳跃时相机所在球体半径
		double					m_fRadiusAcceleration;		// 加速度
		double					m_fRadiusWeakRatio;			// 阻力造成的减速比例
		double					m_fFreeYawSpeed;			// 自由状态下的偏转速度
		double					m_fYawSpeed;				// 当前偏转速度
		double					m_fPitchSpeed;				// 俯仰速度
		double					m_fTerminalPitch;			// 极限俯仰角
		double					m_fTerminalYaw;				// 极限偏航角
		double					m_fOriginPitch;				// 起始俯仰角
		double					m_fOriginYaw;				// 起始航向角，逆时针，[0, 2*Pi]
		double					m_fSpinAcceleration;		// 自旋角加速度，+=顺时针，-=逆时针
		double					m_fSpinWeakRatio;			// 自旋阻力造成的自旋减速比例
		/*
		* 在星球表面上移动的速度,屏幕空间
		* x = 屏幕空间右方向的移动的速度
		* y = 屏幕空间水平向前移动的速度 */
		osg::Vec2d				m_vMoveOnSurfaceSpeed;

		SGMDriveParam			m_sDrive;					// 座驾参数
		/*
		* x = 混合系数，与加速度的绝对值负相关
		* y = 混合系数的最大值 */
		osg::Vec2d				m_vMix;						

		osg::Vec3d				m_vEyeDeltaMove;				// 相机平移矢量
		osg::Vec3d				m_vEyeUp;						// 相机上方向
		osg::Vec3d				m_vEyeFront;					// 相机前方向
		osg::Vec3d				m_vEyeLeft;						// 相机左方向
		osg::Vec3d				m_vTargetHierarchyPos;			// 当前目标点的当前层级坐标
		osg::Vec3d				m_vTerminalHierarchyPos;		// 极限目标点的当前层级坐标
		osg::Vec3d				m_vOriginHierarchyPos;			// 起始目标点的当前层级坐标
		osg::Vec3d				m_vLastMouseHierarchyPos;		// 鼠标上一帧在银盘上的层级坐标
		osg::Vec3d				m_vStarWorldPos;				// 播放的音频星世界空间位置
		osg::Vec2f				m_vMousePushScreenPos;			// 鼠标点击的屏幕XY坐标
		osg::Vec2f				m_vMouseLastScreenPos;			// 上一帧鼠标点击的屏幕XY坐标
		osg::Vec2f				m_vMouseDragLastScreenPos;		// 拖拽时，上一帧鼠标的屏幕XY坐标
		EGMC_MANIPULATOR		m_eManipulator;					// 相机操作器模式
		EGMC_WATCH				m_eWatchState;					// 观察模式下，相机的穿梭状态
		bool					m_bEdit;						// 是否处于编辑模式
		bool					m_bEditAudio;					// 是否正在编辑
		bool					m_bTarget2CelestialBody;		// 目标点是否在天体上
		bool					m_bDoubleClick;					// 是否处于双击状态

		osg::Vec3d				m_vPlanetSolarPos;				// 当前行星在太阳系坐标系下的位置
		double					m_fPlanetPeriod;				// 当前行星的公转周期，如果是恒星则为0。单位：秒
	};

}	// DP
