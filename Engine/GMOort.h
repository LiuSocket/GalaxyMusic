//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMOort.h
/// @brief		Galaxy-Music Engine - GMOort.h
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
	*  @class CGMOort
	*  @brief Galaxy-Music Oort Module
	*/
	class CGMOort : public CGMVolumeBasic
	{
		// ����
	public:
		/** @brief ���� */
		CGMOort();
		/** @brief ���� */
		~CGMOort();

		/** @brief ��ʼ�� */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief ���� */
		bool Update(double dDeltaTime);
		/** @brief ����(�������������̬֮��) */
		bool UpdateLater(double dDeltaTime);
		/** @brief ���� */
		bool Load();
		/**
		* @brief �����¶�������
		*/
		void MakeOort();
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
		* @brief ���ð¶������ڵ�4�ռ�㼶�µ�λ��
		* @param vHiePos λ�ã���λ����4�ռ�㼶��λ
		*/
		void SetOortHiePos4(const osg::Vec3f& vHiePos);
		/**
		* @brief ���ð¶�������������ӵ���ת��̬
		* @param qRotate ��ת��̬��Ԫ��
		*/
		void SetOortAttitude(const osg::Quat& qRotate);

	private:
		/**
		* @brief ��������
		* @param fRadius			�뾶
		* @param iLatSegment		γ�ȷֶ��������ȷֶ�����������
		* @return Geometry			���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeSphereGeometry(const float fRadius = 10.0f, const int iLatSegment = 32) const;

		/**
		* @brief ��ʼ���¶�����״̬��
		* @param pSS				��Ҫ���õ�״̬��ָ��
		* @param strShaderName		shader������
		* @return bool				�ɹ�true������false
		*/
		bool _InitOortStateSet(osg::StateSet* pSS, const std::string strShaderName);

	// ����
	private:
		std::string										m_strCoreGalaxyTexPath;			//!< ��ϵ������ͼ·��
		std::string										m_strGalaxyShaderPath;			//!< ��ϵ��ɫ��·��
		osg::ref_ptr<osg::Texture>						m_galaxyTex;					//!< the galaxy texture
		osg::ref_ptr<osg::Texture>						m_galaxyHeightTex;				//!< the galaxy height texture

		osg::ref_ptr<osg::Geode>						m_pGeodeOort3;					//!< ��3�㼶�¶����Ƶ�Geode
		osg::ref_ptr<osg::Transform>					m_pOortTransform4;				//!< ��4�㼶�¶����Ƶı任���

		osg::ref_ptr<osg::StateSet>						m_pSsOortDecFace;				//!< �¶�����12������״̬��
		osg::ref_ptr<osg::StateSet>						m_pSsOortDecEdge;				//!< �¶�����12�����״̬��
		osg::ref_ptr<osg::StateSet>						m_pSsOortDecVert;				//!< �¶�����12�����״̬��

		osg::ref_ptr<osg::Uniform>						m_fOortVisibleUniform;			//!< �¶����ƿɼ��ȵ�Uniform
		osg::ref_ptr<osg::Uniform>						m_fUnitUniform;					//!< ��3�㼶��λ:��ǰ�㼶��λ
		osg::ref_ptr<osg::Uniform>						m_mAttitudeUniform;				//!< ����ϵ����ϵ�µ���ת��̬����
	};

}	// GM