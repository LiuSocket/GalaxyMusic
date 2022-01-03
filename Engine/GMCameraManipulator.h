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
	// ��ǰ�������ģʽ
	enum EGMC_FLY
	{
		EGMA_FLY_FREE,				// ��������ģʽ
		EGMA_FLY_CONTROL,			// �ܿط���ģʽ,������
		EGMA_FLY_GOTO,				// Ŀ�ķ���ģʽ��������ǰ���ŵ���Ƶ��
		EGMA_FLY_HOME,				// �ؼ�ģʽ���ص���ʼ�ص㣨�ң�
		EGMA_FLY_JUMP,				// ��Ծģʽ���Ȼص���ʼ�ص㣨�ң�������Ծ����ǰ���ŵ���Ƶ��
		EGMA_FLY_ULTIMATE			// �ռ�ģʽ��ƽ��������������ֱ࣬���ռ�λ��
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
		* ��ȡ��ǰ�㼶�µ�Ŀ������
		* @author LiuTao
		* @since 2021.10.05
		* @param void
		* @return double		Ŀ�����룬��λ����ǰ�㼶��λ
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
		* @param updateStep ���μ�����µ�ʱ����λs
		* @return void
		*/
		void _InnerUpdate(float updateStep);

		/**
		* @brief �����ض�����ģʽ
		* @author LiuTao
		* @since 2021.08.07
		* @param eMode:			����ģʽ
		* @return void:
		*/
		void _SetFlyMode(EGMC_FLY eMode);

		/**
		* @brief ӳ�亯��
		* @author LiuTao
		* @since 2021.06.06
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
		* @author LiuTao
		* @since 2021.08.29
		* @param fMin, fMax:			��Χ
		* @param fX:					���ϵ��
		* @return double:				��Ϻ��ֵ
		*/
		inline double _Mix(const double fMin, const double fMax, const double fX)
		{
			return fMin*(1 - fX) + fMax*fX;
		}

		/**
		* @brief �����������ֵ��ȡ���ƽ��ֵ
		* @author LiuTao
		* @since 2021.06.06
		* @param vDirection:			�������λ����
		* @return osg::Vec3d:			����ƽ��ֵ
		*/
		osg::Vec3d _GetTranslate(const osg::Vec3d vDirection);

		/**
		* @brief ����������ǰ�ռ�㼶�����꣬��ȡ���ٶȣ������ڿ��Ʋ�������ת�ٶ�
		* @author LiuTao
		* @since 2021.06.06
		* @param vHierarchyPos:			��ǰ֡��ǰ�ռ�㼶������
		* @param vLastHierarchyPos:		��һ֡��ǰ�ռ�㼶������
		* @return double:				���ؽ��ٶ�
		*/
		double _GetAngularVelocity(const osg::Vec3d vHierarchyPos, const osg::Vec3d vLastHierarchyPos);

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
		double					m_fCameraStartRadius;		// ��ʼ��Ծʱ�������������뾶
		double					m_fRadiusAcceleration;		// ���ٶ�
		double					m_fRadiusWeakRatio;			// ������ɵļ��ٱ���
		double					m_fConstSpinSpeed;			// ���������ٶȣ����������ƣ�
		double					m_fSpinSpeed;				// ��ǰ�����ٶ�
		double					m_fPitchSpeed;				// �����ٶ�
		double					m_fTerminalPitch;			// ���޸�����
		double					m_fTerminalYaw;				// ����ƫ����
		double					m_fOriginPitch;				// ��ʼ������
		double					m_fOriginYaw;				// ��ʼ����ǣ���ʱ�룬[0, 2*Pi]
		double					m_fSpinAcceleration;		// �����Ǽ��ٶȣ�+=˳ʱ�룬-=��ʱ��
		double					m_fSpinWeakRatio;			// ����������ɵ��������ٱ���

		double					m_fMixA;					// ���ϵ��A������ٵ�ǿ�ȸ����
		double					m_fMixB;					// ���ϵ��B������ٵ�ǿ�ȸ����
		double					m_fMixMax;					// ���ϵ�������ֵ

		osg::Vec3d				m_vTargetHierarchyPos;			// ��ǰĿ���ĵ�ǰ�ռ�����
		osg::Vec3d				m_vTerminalHierarchyPos;		// ����Ŀ���ĵ�ǰ�ռ�����
		osg::Vec3d				m_vOriginHierarchyPos;			// ��ʼĿ���ĵ�ǰ�ռ�����
		osg::Vec3d				m_vMouseHierarchyPos;			// ��ǰ֡����������ϵĿռ�����
		osg::Vec3d				m_vLastMouseHierarchyPos;		// ��һ֡����������ϵĿռ�����
		osg::Vec3d				m_vStarWorldPos;				// ���ŵ���Ƶ������ռ�λ��
		osg::Vec2f				m_vMousePushScreenPos;			// ��갴�µ�XY����
		osg::Vec2f				m_vMouseLastScreenPos;			// ��һ֡���XY����
		osg::Vec2f				m_vMouseDragLastScreenPos;		// ��һ֡�����ק��XY����
		EGMC_FLY				m_eFlyMode;						// ����ķ���ģʽ
		bool					m_bEdit;						// �Ƿ��ڱ༭ģʽ
		bool					m_bDrag;						// �����ק���
	};

}	// DP
