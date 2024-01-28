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

	// ��ǰ���������ö��
	enum EGMC_MANIPULATOR
	{
		// �۲�������������������0-6�����⴩�󣬳�ʱ��
		EGMM_WATCH,
		// ��ʻ���������ο����Ǽʹ��񡷵ķɴ���ʻ��ʽ
		// WSADǰ�������ƶ���QE���ҹ�ת��X��ɲ��
		// Shift���٣�Ctrl���͸߶ȣ�Spacȩ���߶�
		// ���������º��ƶ������Ƴ���
		EGMM_DRIVE
	};

	// ��ǰ����۲�״̬
	enum EGMC_WATCH
	{
		// ����ģʽ
		EGMC_WATCH_FREE,
		// ����ģʽ���������λ��Ϊ���ģ�������Զ���
		// ��������������գ���Ŀ�����С��ĳ��ֵ��Ͳ�����С�ռ�㼶
		// ������������ڲ�ʱ����������ƶ��ٶ�
		EGMC_WATCH_ZOOM,
		// �ƶ�ģʽ��������ƽ�����
		EGMC_WATCH_MOVE,
		// Ŀ�괩��ģʽ��������ǰ���ŵ���Ƶ��
		EGMC_WATCH_GOTO,
		// �ؼ�ģʽ���ص���ʼ�ص㣨�ң�
		EGMC_WATCH_HOME,
		// ��Ծģʽ���Ȼص���ʼ�ص㣨�ң�������Ծ����ǰ���ŵ���Ƶ��
		EGMC_WATCH_JUMP,
		// �ռ�ģʽ��ƽ��������������ֱ࣬���ռ�λ��
		EGMC_WATCH_ULTIMATE
	};

	/*************************************************************************
	Structs
	*************************************************************************/
	// ���ݵĲ����ṹ��
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

		// �ٶȣ��ɴ���������ϵ�£�xyz = ���Ϻ󣬵�λ��m/s
		osg::Vec3d					vSpeed;
		// ������ǰ�ļ��ٶȣ���λ��m/s2��>= 0
		double						fAccelerationFront;
		// �������ļ��ٶȣ���λ��m/s2��>= 0
		double						fAccelerationBack;
		// ��������ļ��ٶȣ���λ��m/s2��>= 0
		double						fAccelerationLeft;
		// �������ҵļ��ٶȣ���λ��m/s2��>= 0
		double						fAccelerationRight;
		// �������ϵļ��ٶȣ���λ��m/s2��>= 0
		double						fAccelerationUp;
		// �������µļ��ٶȣ���λ��m/s2��>= 0
		double						fAccelerationDown;
		// ��ת���ٶȣ���λ������/s
		double						fRoll;
		// �������ٶȣ���λ������/s
		double						fPitch;
		// ƫ�����ٶȣ���λ������/s
		double						fYaw;
		// �Ƿ��ڸ�����ƫ��
		double						bPitchOrYaw;
		// �Ƿ����ת
		bool						bRollLeft;
		// �Ƿ��ҹ�ת
		bool						bRollRight;
		// �Ƿ����
		bool						bShift;
		// �����ƶ�ģʽ�������������������ٶȽ���0
		bool						bBrakeMode;

		// ��ת�����ٶȣ���λ������/s
		double						fRoll_Max;
		// ����ٶȣ��ɴ�����ϵ�£�xyz = ���Ϻ󣬵�λ��m/s
		osg::Vec3d					vSpeed_Max;
		// Ѳ�����ٶȣ��ɴ�����ϵ�£�xyz = ���Ϻ󣬵�λ��m/s2
		osg::Vec3d					vAcceleration_Normal;
		// �����ٶȣ��ɴ�����ϵ�£�xyz = ���Ϻ󣬵�λ��m/s2
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
		* ��ȡ��ǰ�㼶�µ�Ŀ������
		* @return double		Ŀ�����룬��λ����ǰ�㼶��λ
		*/
		inline double GetHierarchyTargetDistance() const
		{
			return m_fCameraRadius;
		}
		/**
		* GetHierarchyTargetPos
		* ��ȡ��ǰ�㼶��Ŀ���λ��
		* @return osg::Vec3d	Ŀ���λ�ã���λ����ǰ�㼶��λ
		*/
		inline osg::Vec3d GetHierarchyTargetPos() const
		{
			return m_vTargetHierarchyPos;
		}

		/**
		* GetHierarchyEyePos
		* ��ȡ��ǰ�㼶���۵�λ�ã��㼶�л�ʱ����λ�������λ�ÿ��ܻ����һ֡
		* @return osg::Vec3d	Ŀ���λ�ã���λ����ǰ�㼶��λ
		*/
		inline osg::Vec3d GetHierarchyEyePos() const
		{
			return m_vTranslate;
		}
		
		/**
		* ChangeCenter
		* @brief ������������ı���ı������̬��Ϣ
		* @author LiuTao
		* @since 2023.01.16
		* @param vDeltaPos: ����ı�ǰ���λ�ò�ֵ����λ����
		* @return void:
		*/
		void ChangeCenter(const SGMVector3& vDeltaPos);

		/**
		* UpdateHierarchy
		* @brief ���ڿռ�㼶�仯�����³���
		* @author LiuTao
		* @since 2022.11.20
		* @param void
		* @return bool:		����л��˿ռ�㼶�򷵻�true��û���л��򷵻�false
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

		//���в�����������Ӧ
		virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	protected:

		virtual ~CGMCameraManipulator();

	private:
		/**
		* _InnerUpdate
		* �������
		* @author LiuTao
		* @since 2021.07.04
		* @param updateStepTime ���μ�����µ�ʱ����λs
		* @return void
		*/
		void _InnerUpdate(const float updateStepTime);

		/**
		* _ObjectiveUpdate
		* �͹���ʵ�ĸ��º����������ģʽ�޹�
		* @author LiuTao
		* @since 2023.04.22
		* @param ea
		* @param aa
		* @return void
		*/
		void _ObjectiveUpdate(const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa);
		/**
		* @brief ��������ģʽ�£��������µ�����
		* @param ea
		* @param aa
		*/
		void _FlyUpdate(const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa);
		/**
		* @brief ���ò�����ģʽ
		* @param eMode:		������ģʽö��
		*/
		void _SetManipulatorMode(const EGMC_MANIPULATOR eMode);
		/**
		* @brief �����ض��۲�״̬
		* @param eWatch:		�۲�״̬
		*/
		void _SetWatchState(const EGMC_WATCH eWatch);
		/**
		* @brief ���ø�����ƫ����
		* @param fXnormalized:		��һ����ĻX���꣬[-1.0,1.0]
		* @param fYnormalized:		��һ����ĻY���꣬[-1.0,1.0]
		*/
		void _SetPitchYaw(const float fXnormalized, const float fYnormalized);

		/**
		* @brief ��ȡ���ɱ���
		* @param fT:			ʱ�������[0.0, 1.0]
		* @param eMode:			����ģʽ
		*/
		double _GetMix(const double fT, const EGMC_WATCH eMode);
		/**
		* @brief ���ù��ɺ���
		* @param fTScale:			ʱ�����ű�����������
		*/
		void _SetMix(const double fTScale);

		/**
		* @brief ӳ�亯��
		* @param fX:					ӳ��ǰ��ֵ
		* @param fMin, fMax:			ӳ��ǰ�ķ�Χ
		* @param fMinNew, fMaxNew:		ӳ���ķ�Χ
		* @return double:				ӳ����ֵ
		*/
		inline double _Remap(const double fX,
			const double fMin, const double fMax,
			const double fMinNew, const double fMaxNew)
		{
			double fY = (osg::clampBetween(fX, fMin, fMax) - fMin) / (fMax - fMin);
			return fMinNew + fY * (fMaxNew - fMinNew);
		}

		/**
		* @brief ƽ�����ɺ���
		* @author LiuTao
		* @since 2021.09.03
		* @param fMin, fMax:			��Χ
		* @param fX:					ӳ��ǰ��ֵ
		* @return double:				ӳ����ֵ
		*/
		inline double _Smoothstep(const double fMin, const double fMax, const double fX)
		{
			double y = osg::clampBetween((fX - fMin) / (fMax - fMin), 0.0, 1.0);
			return y * y * (3 - 2 * y);
		}

		/**
		* @brief ��Ϻ���,�ο� glsl �е� mix(a,b,x)
		* @param fMin, fMax:			��Χ
		* @param fX:					���ϵ��
		* @return double/Vec3d:			��Ϻ��ֵ
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
		* @brief ��ȡ���ƽ��ֵ
		* @return osg::Vec3d: ����ƽ��ֵ
		*/
		osg::Vec3d _GetTranslate() const;
		/**
		* @brief Ŀ����Ƿ�Ϊ���
		* @param iHie:			��ǰ�ռ�㼶
		* @return bool:			Ŀ����Ƿ�Ϊ���
		*/
		bool _IsTargetInVacuum(const int iHie) const;
		/**
		* @brief ��������㴩�أ�������أ���ǿ������
		* @param vHiePos:		��ǰ�ռ�㼶�µĵ�����
		* @return bool:			���Ƿ��Ѿ�����
		*/
		bool _IsIntoGround(osg::Vec3d& vHiePos) const;

		/**
		* @brief SGMVector3 ת osg::Vec3d
		* @param vGM:				�����GM����
		* @return osg::Vec3d:		�����osg����
		*/
		inline osg::Vec3d _GM2OSG(const SGMVector3& vGM) const
		{
			return osg::Vec3d(vGM.x, vGM.y, vGM.z);
		}
		/**
		* @brief SGMVector4 ת osg::Vec4d
		* @param vGM:				�����GM����
		* @return osg::Vec4d:		�����osg����
		*/
		inline osg::Vec4d _GM2OSG(const SGMVector4& vGM) const
		{
			return osg::Vec4d(vGM.x, vGM.y, vGM.z, vGM.w);
		}
		/**
		* @brief SGMVector3 ת osg::Vec3d
		* @param vGM:				�����GM����
		* @return osg::Vec3d:		�����osg����
		*/
		inline SGMVector3 _OSG2GM(const osg::Vec3d& vOSG) const
		{
			return SGMVector3(vOSG.x(), vOSG.y(), vOSG.z());
		}
		/**
		* @brief SGMVector4 ת osg::Vec4d
		* @param vGM:				�����GM����
		* @return osg::Vec4d:		�����osg����
		*/
		inline SGMVector4 _OSG2GM(const osg::Vec4d& vOSG) const
		{
			return SGMVector4(vOSG.x(), vOSG.y(), vOSG.z(), vOSG.w());
		}

	private:
		osg::Vec3d				m_vTranslate;               // ƽ��  
		osg::Quat				m_qRotate;					// ��ת
		float					m_fDeltaStep;				// ��λ��s
		float					m_fConstantStep;			// �ȼ�����µ�ʱ��,��λs
		float					m_fCursorStillTime;			// ��꾲ֹ�ĳ���ʱ��,��λs
		double					m_fTimeLastFrame;			// ��һ֡ʱ�䣬��λ��s
		double					m_fTimeJumpStart;			// ��ʼ��Ծ���е�ʱ�䣬��λ��s
		double					m_fTimeSinceJump;			// �����󾭹���ʱ��
		double					m_fJumpDuration;			// ��Ծ��ʱ����������Ծ�����仯
		double					m_fCameraRadius;			// �����������뾶
		double					m_fCameraStartRadius;		// ��ʼ��Ծʱ�����������뾶
		double					m_fRadiusAcceleration;		// ���ٶ�
		double					m_fRadiusWeakRatio;			// ������ɵļ��ٱ���
		double					m_fFreeYawSpeed;			// ����״̬�µ�ƫת�ٶ�
		double					m_fYawSpeed;				// ��ǰƫת�ٶ�
		double					m_fPitchSpeed;				// �����ٶ�
		double					m_fTerminalPitch;			// ���޸�����
		double					m_fTerminalYaw;				// ����ƫ����
		double					m_fOriginPitch;				// ��ʼ������
		double					m_fOriginYaw;				// ��ʼ����ǣ���ʱ�룬[0, 2*Pi]
		double					m_fSpinAcceleration;		// �����Ǽ��ٶȣ�+=˳ʱ�룬-=��ʱ��
		double					m_fSpinWeakRatio;			// ����������ɵ��������ٱ���
		/*
		* ������������ƶ����ٶ�,��Ļ�ռ�
		* x = ��Ļ�ռ��ҷ�����ƶ����ٶ�
		* y = ��Ļ�ռ�ˮƽ��ǰ�ƶ����ٶ� */
		osg::Vec2d				m_vMoveOnSurfaceSpeed;

		SGMDriveParam			m_sDrive;					// ���ݲ���
		/*
		* x = ���ϵ��������ٶȵľ���ֵ�����
		* y = ���ϵ�������ֵ */
		osg::Vec2d				m_vMix;						

		osg::Vec3d				m_vEyeDeltaMove;				// ���ƽ��ʸ��
		osg::Vec3d				m_vEyeUp;						// ����Ϸ���
		osg::Vec3d				m_vEyeFront;					// ���ǰ����
		osg::Vec3d				m_vEyeLeft;						// �������
		osg::Vec3d				m_vTargetHierarchyPos;			// ��ǰĿ���ĵ�ǰ�㼶����
		osg::Vec3d				m_vTerminalHierarchyPos;		// ����Ŀ���ĵ�ǰ�㼶����
		osg::Vec3d				m_vOriginHierarchyPos;			// ��ʼĿ���ĵ�ǰ�㼶����
		osg::Vec3d				m_vLastMouseHierarchyPos;		// �����һ֡�������ϵĲ㼶����
		osg::Vec3d				m_vStarWorldPos;				// ���ŵ���Ƶ������ռ�λ��
		osg::Vec2f				m_vMousePushScreenPos;			// ���������ĻXY����
		osg::Vec2f				m_vMouseLastScreenPos;			// ��һ֡���������ĻXY����
		osg::Vec2f				m_vMouseDragLastScreenPos;		// ��קʱ����һ֡������ĻXY����
		EGMC_MANIPULATOR		m_eManipulator;					// ���������ģʽ
		EGMC_WATCH				m_eWatchState;					// �۲�ģʽ�£�����Ĵ���״̬
		bool					m_bEdit;						// �Ƿ��ڱ༭ģʽ
		bool					m_bEditAudio;					// �Ƿ����ڱ༭
		bool					m_bTarget2CelestialBody;		// Ŀ����Ƿ���������
		bool					m_bDoubleClick;					// �Ƿ���˫��״̬

		osg::Vec3d				m_vPlanetSolarPos;				// ��ǰ������̫��ϵ����ϵ�µ�λ��
		double					m_fPlanetPeriod;				// ��ǰ���ǵĹ�ת���ڣ�����Ǻ�����Ϊ0����λ����
	};

}	// DP
