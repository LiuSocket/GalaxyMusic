//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMEarthEngine.h
/// @brief		Galaxy-Music Engine - GMEarthEngine
/// @version	1.0
/// @author		LiuTao
/// @date		2024.02.11
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommonUniform.h"

namespace GM
{
	/*************************************************************************
	Class
	*************************************************************************/
	class CEEControlVisitor;

	/*!
	*  @class CGMEarthEngine
	*  @brief Galaxy-Music GMEarthEngine
	*/
	class CGMEarthEngine
	{
		// ����
	public:
		/** @brief ���� */
		CGMEarthEngine();

		/** @brief ���� */
		~CGMEarthEngine();

		/** @brief ��ʼ�� */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief ���� */
		bool Update(double dDeltaTime);
		/** @brief ����(�������������̬֮��) */
		bool UpdateLater(double dDeltaTime);
		/** @brief ���� */
		bool Load();

		/* @brief �������� */
		void SetTex(osg::Texture* pEarthTailTex, osg::Texture* pInscatteringTex);
		/* @brief ����Uniform */
		void SetUniform(
			osg::Uniform* pViewLigh,
			osg::Uniform* pGroundTop,
			osg::Uniform* pAtmosHeight,
			osg::Uniform* pMinDotUL,
			osg::Uniform* pEyeAltitude,
			osg::Uniform* pWanderProgress);

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
		osg::Node* GetEarthEngineRoot(const int iHie) const;
		/**
		* @brief ��ʾ������
		* @param bVisible: �Ƿ�ɼ�
		*/
		void SetVisible(const bool bVisible);
		/**
		* @brief ���õ����ڵ�2�ռ�㼶�µ���ת�Ƕ�
		* @param fSpin�� ��ת�Ƕȣ���λ������
		*/
		inline void SetEarthSpin(const double fSpin)
		{
			m_fEarthSpin = fSpin;
		}
		/**
		* @brief �������Ƿ�����
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool CreateEngine();

		/**
		* @brief ���ڿռ�㼶�仯�����³���
		* @param iHierarchy:		���º�Ŀռ�㼶���
		* @return bool:				�ɹ�true��ʧ��false
		*/
		bool UpdateHierarchy(int iHierarchy);

		/* @brief ��ȡ���Ƿ���������������x=ת��y=�ƽ� */
		inline osg::Uniform* GetEngineStartRatioUniform() const
		{
			return m_vEngineStartRatioUniform.get();
		}

	private:
		/**
		* @brief �������˵������Ƿ���������������㣬������̫���ӽ�
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _GenEarthEnginePoint_1();
		bool _GenEarthEnginePoint_2();
		/**
		* @brief �������˵������Ƿ���������������������̫���ӽ�
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _GenEarthEngineJetLine_1();
		bool _GenEarthEngineJetLine_2();
		/**
		* @brief �������˵������Ƿ����������壬LOD��4�������ڽ����ӽ�
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _GenEarthEngineBody_1();
		bool _GenEarthEngineBody_2();
		/**
		* @brief �������˵������Ƿ���������״��������Ƭ��LOD��4�������ڽ����ӽ�
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _GenEarthEngineStream();

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
		* @brief �������Ƿ���������γ���ߡ��ߴ���Ϣ�����ɺ�Ҫ�ٵ���
		*/
		void _GenEarthEngineData();

		/**
		* @brief ���ɡ����Ƿ���������ͼ���͡���Χbloomͼ����������������ͼ�����ɺ�Ҫ�ٵ���
		*/
		void _GenEarthEngineTexture();

		/**
		* @brief �������˵����ϵ����Ƿ����������������
		* @param pEllipsoid				�����������������ģ��
		* @param fUnit					��ǰ�ռ�㼶�ĵ�λ���ȣ���λ����
		* @return Geometry				���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeEnginePointGeometry(const osg::EllipsoidModel* pEllipsoid, const double fUnit) const;
		/**
		* @brief �������˵����ϵ����Ƿ�������������
		* @param pEllipsoid				�����������������ģ��
		* @param fUnit					��ǰ�ռ�㼶�ĵ�λ���ȣ���λ����
		* @return Geometry				���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeEngineJetLineGeometry(const osg::EllipsoidModel* pEllipsoid, const double fUnit) const;
		/**
		* @brief �������˵����ϵ����Ƿ������������������ڽ����ӽ�
		* @param pEllipsoid				�����������������ģ��
		* @param fUnit					��ǰ�ռ�㼶�ĵ�λ���ȣ���λ����
		* @return Geometry				���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeEngineJetStreamGeometry(const osg::EllipsoidModel* pEllipsoid, const double fUnit) const;

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

		// ����
	private:
		SGMKernelData*									m_pKernelData;					//!< �ں�����
		SGMConfigData*									m_pConfigData;					//!< ��������
		CGMCommonUniform*								m_pCommonUniform;				//!< ����Uniform

		std::string										m_strGalaxyShaderPath;			//!< galaxy shader ·��
		std::string										m_strEarthShaderPath;			//!< Earth shader ·��
		std::string										m_strCoreModelPath;				//!< ����ģ����Դ·��

		osg::ref_ptr<osg::Group>						m_pEarthEngineRoot_1;			//!< ��1�㼶���Ƿ��������ڵ�
		osg::ref_ptr<osg::Group>						m_pEarthEngineRoot_2;			//!< ��2�㼶���Ƿ��������ڵ�
		osg::ref_ptr<osg::Geode>						m_pEarthEnginePointNode_1;		//!< 1�㼶���Ƿ��������������
		osg::ref_ptr<osg::Geode>						m_pEarthEnginePointNode_2;		//!< 2�㼶���Ƿ��������������
		osg::ref_ptr<osg::Geode>						m_pEarthEngineJetNode_1;		//!< 1�㼶���Ƿ�����������
		osg::ref_ptr<osg::Geode>						m_pEarthEngineJetNode_2;		//!< 2�㼶���Ƿ�����������
		osg::ref_ptr<osg::Geode>						m_pEarthEngineStream;			//!< ���Ƿ�������������״��Ƭ
		osg::ref_ptr<osg::Node>							m_pEarthEngineBody_1;			//!< 1�㼶���Ƿ���������
		osg::ref_ptr<osg::Node>							m_pEarthEngineBody_2;			//!< 2�㼶���Ƿ���������

		osg::ref_ptr<osg::Texture>						m_pEarthTailTex;				//!< ����β������
		osg::ref_ptr<osg::Texture>						m_pInscatteringTex;				//!< ������ɢ������

		osg::ref_ptr<osg::Uniform>						m_vViewLightUniform;			//!< view�ռ�Ĺ�Դ����
		osg::ref_ptr<osg::Uniform>						m_fCloudTopUniform;				//!< �Ʋ㶥��
		osg::ref_ptr<osg::Uniform>						m_fGroundTopUniform;			//!< �������ɽ���ĺ��θ�
		osg::ref_ptr<osg::Uniform>						m_fAtmosHeightUniform;			//!< ������߶�
		osg::ref_ptr<osg::Uniform>						m_fMinDotULUniform;				//!< �й��������СDotULֵ(-1,0)
		osg::ref_ptr<osg::Uniform>						m_fEyeAltitudeUniform;			//!< �۵㺣��Uniform
		osg::ref_ptr<osg::Uniform>						m_fWanderProgressUniform;		//!< ���˵���ƻ���չUniform
		//!< ����������x=ת��y=�ƽ���z=0��ʾ�����������𲽹رգ�z=1��ʾ�����������𲽿���
		osg::ref_ptr<osg::Uniform>						m_vEngineStartRatioUniform;

		osg::ref_ptr<osgDB::Options>					m_pDDSOptions;					//!< dds���������
		osg::ref_ptr<osg::EllipsoidModel>				m_pEllipsoid;					//!< ����ģ��
		// ���˵������Ƿ���������,xy=��γ�ȣ����ȣ���z=�׸ߣ��ף���w=�������߶ȣ��ף�
		// ͼƬ��ȣ�s��= �������������߶ȣ�t��= 1
		osg::ref_ptr<osg::Image>						m_pEarthEngineDataImg;
		osg::ref_ptr<CEEControlVisitor>					m_pEEDirControl;				//!< ���Ƿ�����������
		double											m_fEarthSpin;					//!< �������ת�Ƕȣ���λ������
	};
}	// GM