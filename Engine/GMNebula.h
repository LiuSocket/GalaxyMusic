//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMVolumeBasic.h
/// @brief		Galaxy-Music Engine - GMNebula.h
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.10
//////////////////////////////////////////////////////////////////////////
#pragma once
#include "GMVolumeBasic.h"
#include <osg/MatrixTransform>

namespace GM
{
	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/

	/*!
	*  @class CGMNebula
	*  @brief Galaxy-Music Nebula Module
	*/
	class CGMNebula : public CGMVolumeBasic
	{
		// ����
	public:
		/** @brief ���� */
		CGMNebula();
		/** @brief ���� */
		~CGMNebula();

		/** @brief ��ʼ�� */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData);
		/** @brief ���� */
		bool Update(double dDeltaTime);
		/** @brief ����(�������������̬֮��) */
		bool UpdateLater(double dDeltaTime);
		/** @brief ���� */
		bool Load();
		/**
		* MakeNebula;
		* @brief ��������
		* @author LiuTao
		* @since 2020.12.12
		* @param fLength			����
		* @param fWidth				���
		* @param fHeight			�߶�
		* @param fX					���ĵ��X����
		* @param fY					���ĵ��Y����
		* @param fZ					���ĵ��Z����
		* @return void
		*/
		void MakeNebula(float fLength = 10.0f, float fWidth = 10.0f, float fHeight = 0.5f
		, float fX = 0.0f, float fY = 0.0f, float fZ = 0.0f);

		/** @brief ������ģ����� */
	public:

		/** @brief: Show or hide the nebula */
		void SetNebulaEnabled(const bool bEnable);
		/** @brief Get the No.0 switching texture for TAA */
		osg::Texture2D* GetTAAMap_0();
		/** @brief Get the No.1 switching texture for TAA */
		osg::Texture2D* GetTAAMap_1();

		/**
		* �޸���Ļ�ߴ�ʱ���ô˺���
		* @author LiuTao
		* @since 2020.12.07
		* @param width: ��Ļ���
		* @param height: ��Ļ�߶�
		* @return void
		*/
		void ResizeScreen(const int width, const int height);

	private:

		osg::Texture* _Load3DShapeNoise();
		osg::Texture* _Load3DErosionNoise();
		osg::Texture* _Load3DCurlNoise();

		/**
		* _MakeSphereGeometry;
		* @brief ��������
		* @author LiuTao
		* @since 2020.06.22
		* @param fRadius			�뾶
		* @param iLatSegment		γ�ȷֶ��������ȷֶ�����������
		* @return Geometry			���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeSphereGeometry(float fRadius = 10.0f, int iLatSegment = 32);

		/**
		* _MakeBoxGeometry;
		* @brief ����������
		* @author LiuTao
		* @since 2020.12.12
		* @param fLength			����
		* @param fWidth				���
		* @param fHeight			�߶�
		* @return Geometry			���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeBoxGeometry(float fLength = 10.0f, float fWidth = 10.0f, float fHeight = 2.0f);

	// ����
	private:
		std::string											m_strNebulaShaderPath;			//!< Nebula shader ������ɫ��·��
		std::string											m_strCoreNebulaTexPath;			//!< Nebula texture ���ƺ�����ͼ·��
		osg::ref_ptr<osg::Texture>							m_galaxyTex;					//!< the galaxy texture
		osg::ref_ptr<osg::Texture>							m_galaxyHeightTex;				//!< the galaxy height texture
		osg::ref_ptr<osg::Texture>							m_2DNoiseTex;					//!< the 2D noise texture
		osg::ref_ptr<osg::Texture>							m_3DShapeTex_128;				//!< ��ά�޷�������128^3
		osg::ref_ptr<osg::Texture2D>						m_rayMarchMap_0;				//!< the No.0 1/16 size texture by ray march
		osg::ref_ptr<osg::Texture2D>						m_rayMarchMap_1;				//!< the No.1 1/16 size texture by ray march
		osg::ref_ptr<osg::Texture2D>						m_vectorMap_0;					//!< the No.0 1/16 size vector texture
		osg::ref_ptr<osg::Texture2D>						m_vectorMap_1;					//!< the No.1 1/16 size vector texture
		osg::ref_ptr<osg::Texture2D>						m_distanceMap;					//!< the 1/16 size distance texture
		osg::ref_ptr<osg::MatrixTransform>					m_nebulaRayMarchTransform;		//!< the Nebula raymarching Transform node
		osg::ref_ptr<osg::MatrixTransform>					m_nebulaTransform;				//!< the Nebula Transform node
		osg::ref_ptr<osg::Node>								m_nebulaNode;
		osg::ref_ptr<osg::Camera>							m_rayMarchCamera;				//!< the camera for raymarching cloud
		osg::ref_ptr<osg::StateSet>							m_stateSetRayMarch;
		osg::ref_ptr<osg::StateSet>							m_stateSetFinal;
		int													m_iUnitRayMarch;				//!< the unit of Terrain map for cloud ray marching
		int													m_iUnitFinal;					//!< the unit of Terrain map for cloud of final step
		double												m_dTimeLastFrame;				//!< ��һ֡ʱ��
		osg::Vec2f											m_vLastShakeVec;				//!< ��һ֡��������
		osg::ref_ptr<osg::Uniform>							m_vDeltaShakeUniform;			//!< �����ֵ��������Uniform
		osg::ref_ptr<osg::Uniform>							m_fCountUniform;				//!< ÿ֡+1 ������
		osg::ref_ptr<osg::Uniform>							m_pDeltaVPMatrixUniform;		//!< �����ֵVP����Uniform
	};

}	// GM