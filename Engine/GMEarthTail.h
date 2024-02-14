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
	Enums
	*************************************************************************/

	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/

	/*!
	*  @class CGMEarthTail
	*  @brief Galaxy-Music EarthTail Module
	*/
	class CGMEarthTail : public CGMVolumeBasic
	{
		// ����
	public:
		/** @brief ���� */
		CGMEarthTail();
		/** @brief ���� */
		~CGMEarthTail();

		/** @brief ��ʼ�� */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief ���� */
		bool Update(double dDeltaTime);
		/** @brief ����(�������������̬֮��) */
		bool UpdateLater(double dDeltaTime);
		/** @brief ���� */
		bool Load();
		/**
		* @brief �������˵���β��
		*/
		void MakeEarthTail();
		/**
		* @brief �޸���Ļ�ߴ�ʱ���ô˺���
		* @param width: ��Ļ���
		* @param height: ��Ļ�߶�
		*/
		void ResizeScreen(const int width, const int height);

		/**
		* @brief ���ڿռ�㼶�仯�����³���
		* @param iHierarchy:		���º�Ŀռ�㼶���
		* @return bool:				�ɹ�true��ʧ��false
		*/
		bool UpdateHierarchy(int iHieNew);

		/**
		* @brief ��ʾ���������˵���β��
		* @param bVisible: �Ƿ�ɼ�
		*/
		void SetVisible(const bool bVisible);

		/**
		*  @brief �������˵����β���ڵ�2�ռ�㼶�µ���ת
		* @param qRotate ��ת��Ԫ��
		*/
		void SetEarthTailRotate(const osg::Quat& qRotate);

		/* @brief ����Uniform */
		void SetUniform(
			osg::Uniform* pViewLight,
			osg::Uniform* pEngineStartRatio,
			osg::Uniform* pView2ECEF,
			osg::Uniform* pWanderProgress);

	private:
		/**
		* @brief ��������β��������
		* @param fLength			����β������
		* @param fRadius			�������ͷ���뾶
		* @return Geometry			���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeTailBoxGeometry(const float fLength, const float fRadius) const;
		/**
		* @brief ����β������
		* @param fLength			β������
		* @param fRadius			����ͷ���뾶
		* @return Geometry			���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeTailEnvelopeGeometry(const float fLength, const float fRadius) const;
		/**
		* @brief ����Բ����β��
		* @param fLength			β������
		* @param fRadius			β���뾶
		* @return Geometry			���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeTailCylinderGeometry(const float fLength, const float fRadius) const;
		/**
		* @brief ����������Ƭβ��
		* @param fLength			β������
		* @param fRadius			β���뾶
		* @return Geometry			���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeTailXGeometry(const float fLength, const float fRadius) const;

		/**
		* @brief ��ʼ�����˵���β��״̬��
		* @param pSS				��Ҫ���õ�״̬��ָ��
		* @param strShaderName		shader������
		* @return bool				�ɹ�true������false
		*/
		bool _InitEarthTailStateSet(osg::StateSet* pSS, const std::string strShaderName);

		/**
		* @brief ��ȡβ������ı�������
		* @param fCoordV			����β����������꣬��Χ[0.0,1.0]
		* @param fLength			β������
		* @param fRadius			����ͷ���뾶
		* @return osg::Vec3			���涥���ģ�Ϳռ��λ��
		*/
		osg::Vec3 _GetTailEnvelopePos(const osg::Vec2 fCoordUV, const float fLength, const float fRadius) const;
	// ����
	private:
		std::string										m_strCoreGalaxyTexPath;			//!< ��ϵ������ͼ·��
		std::string										m_strEarthShaderPath;			//!< Earth shader ·��
		std::string										m_strGalaxyShaderPath;			//!< ��ϵ��ɫ��·��

		osg::ref_ptr<osg::Transform>					m_pTailTransform2;				//!< ��2�㼶���˵���β���ı任�ڵ�
		osg::ref_ptr<osg::Geode>						m_pTailBoxGeode2;				//!< ��2�㼶���˵���β���ļ��νڵ�
		osg::ref_ptr<osg::Geode>						m_pTailEnvelopeGeode2;			//!< ��2�㼶β������ļ��νڵ�

		osg::ref_ptr<osg::StateSet>						m_pSsTailDecFace;				//!< ���˵���β��12������״̬��
		osg::ref_ptr<osg::StateSet>						m_pSsTailDecEdge;				//!< ���˵���β��12�����״̬��
		osg::ref_ptr<osg::StateSet>						m_pSsTailDecVert;				//!< ���˵���β��12�����״̬��

		bool											m_bVisible;						//!< ���˵���β���Ƿ�ɼ�
		osg::ref_ptr<osg::Uniform>						m_fTailVisibleUniform;			//!< ���˵���β���ɼ��ȵ�Uniform
		osg::ref_ptr<osg::Uniform>						m_mWorld2ECEFUniform;			//!< ��2������ռ䡱ת ECEF �ľ���
		osg::ref_ptr<osg::Uniform>						m_mView2ECEFUniform;			//!< view�ռ�תECEF�ľ���
		osg::ref_ptr<osg::Uniform>						m_vViewLightUniform;			//!< view�ռ�Ĺ�Դ����
		osg::ref_ptr<osg::Uniform>						m_fWanderProgressUniform;		//!< ���˵���ƻ���չUniform
		osg::ref_ptr<osg::Uniform>						m_vEngineStartRatioUniform;		//!< ����������x=ת��y=�ƽ�
	};

}	// GM