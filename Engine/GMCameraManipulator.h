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
		EGMA_FLY_HOME				// �ؼ�ģʽ���ص���ʼ�ص㣨�ң�
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
		 * @param vWorldPos
		 *      Output world position (only valid if the method returns true)
		 */
		bool screenToWorld(float x, float y, osg::View* view, osg::Vec3d& vWorldPos) const;

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
		* @brief ��ʼ�����ض�ģʽ
		* @author LiuTao
		* @since 2021.08.07
		* @param eMode:			ģʽ����ʱֻ����Ծ�ͻع飩
		* @return void:
		*/
		void _StartToJump(EGMC_FLY eMode);

		/**
		* @brief ӳ�亯��
		* @author LiuTao
		* @since 2021.06.06
		* @param fX:					ӳ��ǰ��ֵ
		* @param fMin, fMax:			ӳ��ǰ�ķ�Χ
		* @param fMinNew, fMaxNew:		ӳ���ķ�Χ
		* @return float:				ӳ����ֵ
		*/
		double _Remap(const double fX,
			const double fMin, const double fMax,
			const double fMinNew, const double fMaxNew);

		/**
		* @brief �����������ֵ��ȡ���ƽ��ֵ
		* @author LiuTao
		* @since 2021.06.06
		* @param vDirection:			�������λ����
		* @return osg::Vec3d:			����ƽ��ֵ
		*/
		osg::Vec3d _GetTranslate(const osg::Vec3d vDirection);

		/**
		* @brief ������������ռ�����꣬��ȡ���ٶȣ������ڿ��Ʋ�������ת�ٶ�
		* @author LiuTao
		* @since 2021.06.06
		* @param vPos:					��ǰ֡λ��
		* @param vLastPos:				��һ֡λ��
		* @return double:				���ؽ��ٶ�
		*/
		double _GetAngularVelocity(const osg::Vec3d vPos, const osg::Vec3d vLastPos);

	private:
		osg::Vec3d				m_vTranslate;               // ƽ��  
		osg::Quat				m_qRotate;					// ��ת
		float					m_fDeltaStep;				// ��λs
		float					m_fConstantStep;			// �ȼ�����µ�ʱ��,��λs
		float					m_fSignYaw;					// ƫ���Ƿ��������Ҹ�����ǰ
		double					m_fTimeLastFrame;			// ��һ֡ʱ�䣬��λ��s
		double					m_fTimeJumpStart;			// ��ʼ��Ծ���е�ʱ�䣬��λ��s
		double					m_fTimePushFrame;			// ��갴�µ�ʱ�̣���λ��s
		double					m_fCameraRadius;			// �����������뾶
		double					m_fRadiusAcceleration;		// ���ٶ�
		double					m_fRadiusWeakRatio;			// ������ɵļ��ٱ���
		double					m_fConstSpinSpeed;			// ���������ٶȣ����������ƣ�
		double					m_fSpinSpeed;				// ��ǰ�����ٶ�
		double					m_fPitchSpeed;				// �����ٶ�
		double					m_fTerminalPitch;			// ���޸�����
		double					m_fOriginPitch;				// ��ʼ������
		double					m_fSpinAcceleration;		// �����Ǽ��ٶȣ�+=˳ʱ�룬-=��ʱ��
		double					m_fSpinWeakRatio;			// ����������ɵ��������ٱ���
		double					m_fTargetMix;				// Ŀ�����ϵ��
		osg::Vec3d				m_vTargetWorldPos;			// ��ǰĿ��������ռ�λ��
		osg::Vec3d				m_vTerminalWorldPos;		// ����Ŀ��������ռ�λ��
		osg::Vec3d				m_vOriginWorldPos;			// ��ʼĿ��������ռ�λ��
		osg::Vec3d				m_vStarWorldPos;			// ���ŵ���Ƶ������ռ�λ��
		osg::Vec3d				m_vMouseWorldPos;			// ��ǰ����������ϵ�λ��
		osg::Vec3d				m_vLastMouseWorldPos;		// ��һ֡����������ϵ�λ��
		osg::Vec2f				m_vMousePushPos;			// ��갴�µ�XY����
		osg::Vec2f				m_vMouseLastPos;			// ��һ֡���XY����
		EGMC_FLY				m_eFlyMode;					// ����ķ���ģʽ
		bool					m_bEdit;					// �Ƿ��ڱ༭ģʽ
	};

}	// DP
