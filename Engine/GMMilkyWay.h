//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMMilkyWay.h
/// @brief		Galaxy-Music Engine - GMMilkyWay.h
/// @version	1.0
/// @author		LiuTao
/// @date		2023.08.06
//////////////////////////////////////////////////////////////////////////
#pragma once
#include "GMVolumeBasic.h"

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
	*  @class CGMMilkyWay
	*  @brief Galaxy-Music Nebula Module
	*/
	class CGMMilkyWay : public CGMVolumeBasic
	{
		// ����
	public:
		/** @brief ���� */
		CGMMilkyWay();
		/** @brief ���� */
		~CGMMilkyWay();

		/** @brief ��ʼ�� */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief ���� */
		bool Update(double dDeltaTime);
		/** @brief ����(�������������̬֮��) */
		bool UpdateLater(double dDeltaTime);
		/** @brief ���� */
		bool Load();
		/**
		* @brief ��������ϵ
		* @param fLength			���ȣ���λ����
		* @param fWidth				��ȣ���λ����
		* @param fHeight			�߶ȣ���λ����
		* @param fX					���ĵ��X���꣬��λ����
		* @param fY					���ĵ��Y���꣬��λ����
		* @param fZ					���ĵ��Z���꣬��λ����
		*/
		void MakeMilkyWay(double fLength = 1e21, double fWidth = 1e21, double fHeight = 5e19
		, double fX = 0.0, double fY = 0.0, double fZ = 0.0);

		/** @brief ������ģ����� */
	public:

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

		inline osg::Texture* GetDistanceMap() const
		{
			return m_distanceMap.get();
		}

	private:
		/**
		* @brief ����������
		* @param fLength			����
		* @param fWidth				���
		* @param fHeight			�߶�
		* @return Geometry			���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeBoxGeometry(
			const float fLength = 10.0f,
			const float fWidth = 10.0f,
			const float fHeight = 2.0f) const;

		/**
		* @brief ��ʼ������ϵ״̬��
		* @param pSS				��Ҫ���õ�״̬��ָ��
		* @param sVR				��ϵ�����Χ
		* @param strShaderName		shader������
		* @return bool				�ɹ�true������false
		*/
		bool _InitMilkyWayStateSet(osg::StateSet* pSS,const SGMVolumeRange& sVR, const std::string strShaderName);

	// ����
	private:
		std::string										m_strCoreGalaxyTexPath;			//!< ��ϵ������ͼ·��
		std::string										m_strGalaxyShaderPath;			//!< ��ϵ��ɫ��·��
		osg::ref_ptr<osg::Texture>						m_galaxyTex;					//!< the galaxy texture
		osg::ref_ptr<osg::Texture>						m_galaxyHeightTex;				//!< the galaxy height texture
		osg::ref_ptr<osg::Texture>						m_2DNoiseTex;					//!< the 2D noise texture
		osg::ref_ptr<osg::Texture2D>					m_distanceMap;					//!< ��ϵ����Ⱦ���������ص���Ϣ

		osg::ref_ptr<osg::Geode>						m_pGeodeMilkyWay;				//!< ����ϵ����Ⱦbox��Geode

		osg::ref_ptr<osg::StateSet>						m_pSsMilkyWayDecFace;			//!< ����ϵ12������״̬��
		osg::ref_ptr<osg::StateSet>						m_pSsMilkyWayDecEdge;			//!< ����ϵ12�����״̬��
		osg::ref_ptr<osg::StateSet>						m_pSsMilkyWayDecVert;			//!< ����ϵ12�����״̬��
	};

}	// GM