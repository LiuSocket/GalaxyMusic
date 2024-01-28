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
	 Macro Defines
	*************************************************************************/

	/*************************************************************************
	Enums
	*************************************************************************/

	/*!
	 *  @enum EGMVolumeState
	 *  @brief ��갴��
	 */
	enum EGMVolumeState
	{
		EGM_VS_Oort,					//!< �¶�����
		EGM_VS_Galaxy,					//!< ����ϵ
		EGM_VS_GalaxyGroup,				//!< ��ϵȺ
		EGM_VS_SuperCluster,			//!< ����ϵ��
		EGM_VS_UltraCluster				//!< ������ϵ��
	};

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
		* �޸���Ļ�ߴ�ʱ���ô˺���
		* @author LiuTao
		* @since 2020.12.07
		* @param width: ��Ļ���
		* @param height: ��Ļ�߶�
		* @return void
		*/
		void ResizeScreen(const int width, const int height);

	protected:
		/**
		* UpdateHierarchy
		* @brief ���ڿռ�㼶�仯�����³���
		* @param iHierarchy:		���º�Ŀռ�㼶���
		* @return bool:				�ɹ�true��ʧ��false
		*/
		bool UpdateHierarchy(int iHieNew);

		/**
		* _MakeSphereGeometry;
		* @brief ��������
		* @author LiuTao
		* @since 2020.06.22
		* @param fRadius			�뾶
		* @param iLatSegment		γ�ȷֶ��������ȷֶ�����������
		* @return Geometry			���ش����ļ�����ָ��
		*/
		osg::Geometry* MakeSphereGeometry(const float fRadius = 10.0f, const int iLatSegment = 32) const;

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
		osg::Geometry* MakeBoxGeometry(
			const float fLength = 10.0f,
			const float fWidth = 10.0f,
			const float fHeight = 2.0f) const;

	private:
		osg::Texture* _Load3DShapeNoise() const;
		osg::Texture* _Load3DErosionNoise() const;
		osg::Texture* _Load3DCurlNoise() const;

		/**
		* _InitMilkyWayStateSet
		* @brief ��ʼ������ϵ״̬��
		* @author LiuTao
		* @since 2022.07.26
		* @param pSS				��Ҫ���õ�״̬��ָ��
		* @param sVR				��ϵ�����Χ
		* @param strShaderName		shader������
		* @return bool				�ɹ�true������false
		*/
		bool _InitMilkyWayStateSet(osg::StateSet* pSS,const SGMVolumeRange& sVR, const std::string strShaderName);
		/**
		* _InitOortStateSet
		* @brief ��ʼ���¶�����״̬��
		* @author LiuTao
		* @since 2022.07.26
		* @param pSS				��Ҫ���õ�״̬��ָ��
		* @param strShaderName		shader������
		* @return bool				�ɹ�true������false
		*/
		bool _InitOortStateSet(osg::StateSet* pSS, const std::string strShaderName);

	// ����
	protected:
		osg::ref_ptr<osg::Texture>						m_3DShapeTex;					//!< ��ά�޷�������RGBA��128^3
		osg::ref_ptr<osg::Texture>						m_3DErosionTex;					//!< ��ά�޷�������RGB��32^3
		osg::ref_ptr<osg::Texture>						m_2DNoiseTex;					//!< the 2D noise texture
		osg::ref_ptr<osg::Texture2D>					m_distanceMap;					//!< ��ϵ����Ⱦ���������ص���Ϣ
		std::string										m_strCoreNebulaTexPath;			//!< Nebula texture ���ƺ�����ͼ·��
		osg::ref_ptr<osg::Uniform>						m_fGalaxyAlphaUniform;			//!< ���ӵ�alpha��Uniform
		osg::ref_ptr<osg::Uniform>						m_vStarHiePosUniform;			//!< ��Ƶ�ǵ�ǰ�㼶�ռ�����Uniform

	private:
		std::string										m_strNebulaShaderPath;			//!< Nebula shader ������ɫ��·��
		osg::ref_ptr<osg::Texture>						m_galaxyTex;					//!< the galaxy texture
		osg::ref_ptr<osg::Texture>						m_galaxyHeightTex;				//!< the galaxy height texture
		osg::ref_ptr<osg::Texture2D>					m_nebulaTex;					//!< the nebula texture by ray march
		osg::ref_ptr<osg::Texture2D>					m_rayMarchMap_1;				//!< the No.1 texture by ray march
		osg::ref_ptr<osg::Texture2D>					m_vectorMap_0;					//!< the No.0 vector texture
		osg::ref_ptr<osg::Texture2D>					m_vectorMap_1;					//!< the No.1 vector texture

		osg::ref_ptr<osg::Geode>						m_pDodecahedronFace;			//!< 12����ġ��桱���νڵ�
		osg::ref_ptr<osg::Geode>						m_pDodecahedronEdge;			//!< 12����ġ��ߡ����νڵ�
		osg::ref_ptr<osg::Geode>						m_pDodecahedronVert;			//!< 12����ġ��㡱���νڵ�

		osg::ref_ptr<osg::StateSet>						m_pSsMilkyWayDecFace;			//!< ����ϵ12������״̬��
		osg::ref_ptr<osg::StateSet>						m_pSsMilkyWayDecEdge;			//!< ����ϵ12�����״̬��
		osg::ref_ptr<osg::StateSet>						m_pSsMilkyWayDecVert;			//!< ����ϵ12�����״̬��
		osg::ref_ptr<osg::StateSet>						m_pSsOortDecFace;				//!< �¶�����12������״̬��
		osg::ref_ptr<osg::StateSet>						m_pSsOortDecEdge;				//!< �¶�����12�����״̬��
		osg::ref_ptr<osg::StateSet>						m_pSsOortDecVert;				//!< �¶�����12�����״̬��

		EGMVolumeState									m_eVolumeState;					//!< ����Ⱦ״̬
		osg::ref_ptr<osg::MatrixTransform>				m_pDodecahedronTrans;			//!< ��12����任�ڵ�
		osg::ref_ptr<osg::Camera>						m_rayMarchCamera;				//!< the camera for raymarching cloud
		osg::Vec3d										m_vWorldEyePos;					//!< �۵㵱ǰ֡����ռ�����
		double											m_dTimeLastFrame;				//!< ��һ֡ʱ��
		osg::ref_ptr<osg::Uniform>						m_mDeltaVPMatrixUniform;		//!< �����ֵVP����Uniform
		osg::ref_ptr<osg::Uniform>						m_fOortVisibleUniform;			//!< �¶����ƿɼ��ȵ�Uniform
	};

}	// GM