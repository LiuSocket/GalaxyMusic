//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMEarth.h
/// @brief		Galaxy-Music Engine - GMEarth
/// @version	1.0
/// @author		LiuTao
/// @date		2023.11.29
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommonUniform.h"
#include "GMPlanet.h"

#include <osg/Texture2DArray>

namespace GM
{
	/*************************************************************************
	Class
	*************************************************************************/
	class CGMEarthTail;
	class CGMEarthEngine;

	/*!
	*  @class CGMEarth
	*  @brief Galaxy-Music GMEarth
	*/
	class CGMEarth: public CGMPlanet
	{
		// ����
	public:
		/** @brief ���� */
		CGMEarth();

		/** @brief ���� */
		~CGMEarth();

		/** @brief ��ʼ�� */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief ���� */
		bool Update(double dDeltaTime);
		/** @brief ����(�������������̬֮��) */
		bool UpdateLater(double dDeltaTime);
		/** @brief ���� */
		bool Load();

		/* @brief ����Uniform */
		void SetUniform(
			osg::Uniform* pPlanetRadius,
			osg::Uniform* pViewLigh,
			osg::Uniform* pCloudTop,
			osg::Uniform* pGroundTop,
			osg::Uniform* pAtmosHeight,
			osg::Uniform* pMinDotUL,
			osg::Uniform* pEyeAltitude,
			osg::Uniform* pWorld2ECEF,
			osg::Uniform* pView2ECEF);

		/**
		* @brief �޸���Ļ�ߴ�ʱ���ô˺���
		* @param iW: ��Ļ���
		* @param iH: ��Ļ�߶�
		*/
		void ResizeScreen(const int iW, const int iH);

		/**
		* @brief ��ȡָ���㼶�ĸ��ڵ㣬��������ڣ��ͷ��ؿ�
		* @param iHie: �ռ�㼶��0,1,2
		* @return osg::Node* ��Ӧ�㼶�ĸ��ڵ�ָ��
		*/
		osg::Node* GetEarthRoot(const int iHie) const;

		/**
		* @brief ��ȡ�Ƹ߶�
		* @return float �Ƹ߶ȣ���λ����
		*/
		inline float GetCloudBottomHeight() const
		{
			return m_fCloudBottom;
		}
		inline float GetCloudTopHeight() const
		{
			return m_fCloudTop;
		}

		/**
		* @brief ��ʾ�����ص���
		* @param bVisible: �Ƿ�ɼ�
		*/
		void SetVisible(const bool bVisible);
		/**
		* @brief ���õ����ڵ�2�ռ�㼶�µ���ת
		* @param fSpin�� ��ת����λ������
		* @param fObliquity�� ��ת����ǣ���λ������
		* @param fNorthYaw�� ��ת��ƫ���ǣ���λ������
		*/
		void SetEarthRotate(const double fSpin, const double fObliquity, const double fNorthYaw);
		/**
		* @brief ���á����˵���ƻ����Ľ�չ
		* @param fProgress ��չ�ٷֱȣ�[0.0, 1.0]
		*/
		void SetWanderingEarthProgress(const float fProgress);

		/**
		* @brief ��������
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool CreateEarth();

		/**
		* @brief ���ڿռ�㼶�仯�����³���
		* @param iHierarchy:		���º�Ŀռ�㼶���
		* @return bool:				�ɹ�true��ʧ��false
		*/
		bool UpdateHierarchy(int iHierarchy);
		/**
		* @brief ��ȡ�������ת����ǣ����˵���ģʽ�»�仯
		* @return double: �������ת����ǣ���λ������
		*/
		inline double GetCurrentObliquity() const
		{
			return m_fCurrentObliquity;
		}
		/**
		* @brief ���õ������ת�������ת�Ľ��ٶȣ������˵���ģʽ�ã����������Ϊ0
		* @return NorthRotateSpeed: �������ת�������ת�Ľ��ٶȣ���λ������/��
		*/
		inline void SetNorthRotateSpeed(const double fNorthRotateSpeed)
		{
			m_fNorthRotateSpeed = fNorthRotateSpeed;
		}
		/**
		* @brief ��ȡ�������ת�������ת�Ľ��ٶȣ������˵���ģʽ�ã����������Ϊ0
		* @return double: �������ת�������ת�Ľ��ٶȣ���λ������/��
		*/
		inline double GetNorthRotateSpeed() const
		{
			return m_fNorthRotateSpeed;
		}

	private:

		/**
		* @brief ����ȫ���Ʋ�Ե���Ӱ
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateGlobalCloudShadow();

		/**
		* @brief ������1�㼶�ռ�ĵ��򣬰������棬�Ʋ㣬����
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateEarth_1();
		/**
		* @brief ������2�㼶�ռ�ĵ��򣬰������棬�Ʋ㣬����
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateEarth_2();

		/**
		* @brief ������2�㼶�ռ�����˵���
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateWanderingEarth();

		/**
		* @brief ����2D����
		* @param fileName: ͼƬ�ļ�·��
		* @param iChannelNum: 1��2��3��4�ֱ����R��RG��RGB��RGBA
		* @return osg::Texture* ��������ָ��
		*/
		osg::Texture* _CreateTexture2D(const std::string& fileName, const int iChannelNum) const;
		/**
		* @brief ����2D����
		* @param pImg:		ͼƬָ��
		* @param iChannelNum: 1��2��3��4�ֱ����R��RG��RGB��RGBA
		* @return osg::Texture* ��������ָ��
		*/
		osg::Texture* _CreateTexture2D(osg::Image* pImg, const int iChannelNum) const;

		/**
		* @brief ����DDS����
		* @param fileName: ͼƬ�ļ�·��
		* @param eWrap_S: ��/X/U�������ģʽ
		* @param eWrap_T: ��/Y/V�������ģʽ
		* @param bFlip: �Ƿ�ת���£�dds��Ҫ�����Ƿ�ת��
		* @return osg::Texture* ��������ָ��
		*/
		osg::Texture* _CreateDDSTexture(const std::string& fileName,
			osg::Texture::WrapMode eWrap_S, osg::Texture::WrapMode eWrap_T,
			bool bFlip = false) const;
		/**
		* @brief ����DDS��ά��������
		* @param filePreName: ͼƬ�ļ�·�������������ֺ�.dds��
		* @param bFlip: �Ƿ�ת���£�dds��Ҫ�����Ƿ�ת��
		* @return osg::Texture* ��������ָ��
		*/
		osg::Texture2DArray* _CreateDDSTex2DArray(const std::string& filePreName, bool bFlip = true) const;
		/**
		* @brief ׷��DDS��ά��������
		* @param pTex��	����ָ��
		* @param filePreName: ͼƬ�ļ�·�������������ֺ�.dds��
		* @param bFlip: �Ƿ�ת���£�dds��Ҫ�����Ƿ�ת��
		* @return bool �ɹ�true���������ָ��Ϊ�ջ������ӵ�ͼƬ�ߴ������ͼ�ߴ粻ͬ���򷵻�false
		*/
		bool _AddTex2DArray(osg::Texture2DArray* pTex, const std::string& filePreName, bool bFlip = true);

		/**
		* @brief ��ʱ��ӵ��������˵���汾�ĸ�����ͼ�Ĺ��ߺ���
		* @param strPath0: ��ͼ0����������źͺ�׺��
		* @param strPath1: ��ͼ1��ע��Ҫˮƽ��ת����������źͺ�׺��
		* @param strOut: �������ͼ·������������źͺ�׺��
		* @param iType: ��ͬ�ĵ��ӷ�ʽ
		*/
		void _MixWEETexture(
			const std::string& strPath0, const std::string& strPath1, const std::string& strOut,
			const int iType);

		/**
		* @brief ���Ƿ��������ƫת���ƫ��λ�ã��봹ֱ����������ȣ�
		* @param vDir: �����������䷽��ECEF��
		* @param vUp: ���������Ϸ���ECEF��
		* @return osg::Vec3: ���ƫת���ƫ��λ�ã���λ����
		*/
		inline osg::Vec3 _NozzlePos(const osg::Vec3& vDir, const osg::Vec3& vUp) const
		{
			return vDir * 1500.0 - vUp * 2000.0;
		}

		// ����
	private:
		SGMKernelData*									m_pKernelData;					//!< �ں�����
		CGMCommonUniform*								m_pCommonUniform;				//!< ����Uniform

		std::string										m_strGalaxyShaderPath;			//!< galaxy shader ·��
		std::string										m_strEarthShaderPath;			//!< Earth shader ·��

		double											m_fCurrentObliquity;			//!< ��ת����ǣ���λ������
		double											m_fNorthRotateSpeed;			//!< ��ת�������ת�Ľ��ٶȣ�����/��
		float											m_fCloudBottom;					//!< �Ƶ׸߶�
		float											m_fCloudTop;					//!< �ƶ��߶�

		osg::ref_ptr<osg::Transform>					m_pShadow_2_Transform;			//!< ��2�㼶��ǰȫ����Ӱ�任���
		osg::ref_ptr<osg::Group>						m_pEarthRoot_1;					//!< ��1�㼶������ڵ�
		osg::ref_ptr<osg::Group>						m_pEarthRoot_2;					//!< ��2�㼶������ڵ�
		osg::ref_ptr<osg::Geometry>						m_pEarthGeom_1;					//!< ��1�㼶���򼸺νڵ�
		osg::ref_ptr<osg::Geometry>						m_pEarthGeom_2;					//!< ��2�㼶���򼸺νڵ�
		osg::ref_ptr<osg::Geode>						m_pEarthCloud_1;				//!< 1�㼶�����Ʋ�ڵ�
		osg::ref_ptr<osg::Geode>						m_pEarthAtmos_1;				//!< 1�㼶��������ڵ�
		osg::ref_ptr<osg::Geode>						m_pEarthGround_2;				//!< 2�㼶�������ڵ�
		osg::ref_ptr<osg::Geode>						m_pEarthCloud_2;				//!< 2�㼶�����Ʋ�ڵ�
		osg::ref_ptr<osg::Geode>						m_pEarthAtmos_2;				//!< 2�㼶��������ڵ�
		osg::ref_ptr<osg::StateSet>						m_pSSEarthGround_1;				//!< 1�㼶�������״̬��
		osg::ref_ptr<osg::StateSet>						m_pSSEarthCloud_1;				//!< 1�㼶�����Ʋ�״̬��
		osg::ref_ptr<osg::StateSet>						m_pSSEarthAtmos_1;				//!< 1�㼶�������״̬��
		osg::ref_ptr<osg::StateSet>						m_pSSEarthGround_2;				//!< 2�㼶�������״̬��
		osg::ref_ptr<osg::StateSet>						m_pSSEarthCloud_2;				//!< 2�㼶�����Ʋ�״̬��
		osg::ref_ptr<osg::StateSet>						m_pSSEarthAtmos_2;				//!< 2�㼶�������״̬��
		osg::ref_ptr<osg::StateSet>						m_pSSGlobalShadow;				//!< ȫ����Ӱ״̬��

		osg::ref_ptr<osg::Uniform>						m_vPlanetRadiusUniform;			//!< ���ǵ������뾶���ڶ��㼶��
		osg::ref_ptr<osg::Uniform>						m_vViewLightUniform;			//!< view�ռ�Ĺ�Դ����
		osg::ref_ptr<osg::Uniform>						m_fCloudTopUniform;				//!< �Ʋ㶥��
		osg::ref_ptr<osg::Uniform>						m_fGroundTopUniform;			//!< �������ɽ���ĺ��θ�
		osg::ref_ptr<osg::Uniform>						m_fAtmosHeightUniform;			//!< ������߶�
		osg::ref_ptr<osg::Uniform>						m_fMinDotULUniform;				//!< �й��������СDotULֵ(-1,0)
		osg::ref_ptr<osg::Uniform>						m_fEyeAltitudeUniform;			//!< �۵㺣��Uniform
		osg::ref_ptr<osg::Uniform>						m_mWorld2ECEFUniform;			//!< ��2������ռ䡱ת ECEF �ľ���
		osg::ref_ptr<osg::Uniform>						m_mView2ECEFUniform;			//!< view�ռ�תECEF�ľ���
		osg::ref_ptr<osg::Uniform>						m_vEarthCoordScaleUniform;		//!< ������ͼ��������������
		osg::ref_ptr<osg::Uniform>						m_fWanderProgressUniform;		//!< ���˵���ƻ���չUniform

		osg::ref_ptr<osgDB::Options>					m_pDDSOptions;					//!< dds���������
		osg::ref_ptr<osg::Texture2DArray>				m_aEarthBaseTex;				//!< ����base color����
		osg::ref_ptr<osg::Texture2DArray>				m_aEarthCloudTex;				//!< �����Ʋ�����
		osg::ref_ptr<osg::Texture2DArray>				m_aIllumTex;					//!< ��������Է���+�������Է���
		osg::ref_ptr<osg::Texture2DArray>				m_aDEMTex;						//!< ����DEM
		//osg::ref_ptr<osg::Texture2DArray>				m_aAuroraTex;					//!< ��������
		osg::ref_ptr<osg::Texture>						m_pCloudDetailTex;				//!< ��ϸ������
		osg::ref_ptr<osg::Texture3D>					m_pInscatteringTex;

		osg::ref_ptr<osg::EllipsoidModel>				m_pEllipsoid;					//!< ����ģ��
		std::map<int,osg::ref_ptr<osg::Geode>>			m_mShadowEarthGeode;			//!< ����ȫ����Ӱ��Ⱦ�Ľڵ�vector
		osg::ref_ptr<osg::Texture2D>					m_pGlobalShadowTex;				//!< ȫ���Ʋ���Ӱ��ͼ
		osg::ref_ptr<osg::Camera>						m_pGlobalShadowCamera;			//!< ȫ���Ʋ���Ӱ��RTT���

		CGMEarthTail*									m_pEarthTail;					//!< ���˵���β������Ⱦģ��
		CGMEarthEngine*									m_pEarthEngine;					//!< ���Ƿ�����ģ��
	};
}	// GM