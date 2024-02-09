//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMSolar.h
/// @brief		Galaxy-Music Engine - GMSolar
/// @version	1.0
/// @author		LiuTao
/// @date		2020.11.27
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommon.h"
#include "GMCommonUniform.h"
#include "GMKernel.h"
#include "GMDispatchCompute.h"

#include <random>
#include <osg/Node>
#include <osg/AutoTransform>
#include <osg/Texture>
#include <osg/Texture2DArray>
#include <osg/Depth>
#include <osg/BlendFunc>
#include <osg/CullFace>
#include <osg/CoordinateSystemNode>

namespace GM
{
	/*************************************************************************
	 Enums
	*************************************************************************/

	/*************************************************************************
	Structs
	*************************************************************************/
	// ����ṹ�壬���ǣ����ǣ����ǵȶ���������
	struct SGMCelestialBody
	{
		SGMCelestialBody()
			: fOrbitalRadius(1e11), fOrbitalPeriod(1.0), fTrueAnomaly(0.0),
			fEquatorRadius(0.01), fPolarRadius(0.01), fObliquity(0.0), fSpinPeriod(1.0),
			fSpin(0.0), fGroundTop(1000.0f), fCloudTop(8e3f), eAtmosHeight(EGMAH_0),
			vAtmosColor(osg::Vec4f(1, 1, 1, 1)),
			fRingMinRadius(0), fRingMaxRadius(0)
		{}
		SGMCelestialBody(double orbitalRadius, double period, double startTrueAnomaly,
			double equatorRadius,double polarRadius,double obliquity,double spinPeriod,
			float groundTop, float cloudTop, EGMAtmosHeight atmosH,
			osg::Vec4f atmosColor = osg::Vec4f(1, 1, 1, 1),
			double ringMinRadius = 0, double ringMaxRadius = 0)
			: fOrbitalRadius(orbitalRadius), fOrbitalPeriod(period), fTrueAnomaly(startTrueAnomaly),
			fEquatorRadius(equatorRadius), fPolarRadius(polarRadius), fObliquity(obliquity), fSpinPeriod(spinPeriod),
			fSpin(0.0), fGroundTop(groundTop), fCloudTop(cloudTop), eAtmosHeight(atmosH), vAtmosColor(atmosColor),
			fRingMinRadius(ringMinRadius), fRingMaxRadius(ringMaxRadius)	
		{}

		double fOrbitalRadius;			// ��ת����뾶����λ����
		double fOrbitalPeriod;			// ��ת���ڣ���λ��s
		double fTrueAnomaly;			// �����ǣ���λ������
		double fEquatorRadius;			// �������뾶����λ����
		double fPolarRadius;			// ���������뾶����λ����
		double fObliquity;				// �������ת��ƽ����Ƶ���нǣ����򣩣���λ������
		double fSpinPeriod;				// �������ת���ڣ���λ����
		double fSpin;					// ���嵱ǰ֡����ת�Ƕȣ���λ������
		float fGroundTop;				// �������ɽ���ĺ��θߣ���λ����
		float fCloudTop;				// ��ʯ���ǵ��ƶ����θߣ���λ����
		EGMAtmosHeight eAtmosHeight;	// ����Ĵ�����ȣ���λ��ǧ��
		osg::Vec4f vAtmosColor;			// ���������ɫ
		double fRingMinRadius;			// ����Ĺ⻷�ھ�����λ����
		double fRingMaxRadius;			// ����Ĺ⻷�⾶����λ����
	};

	/*************************************************************************
	Class
	*************************************************************************/
	class CGMTerrain;
	class CGMAtmosphere;
	class CGMEarth;
	class CGMPlanet;
	class CGMOort;
	class CGMDataManager;
	class CGMCelestialScaleVisitor;

	/*!
	*  @class CGMSolar
	*  @brief Galaxy-Music GMSolar
	*/
	class CGMSolar
	{
		// ����
	public:
		/** @brief ���� */
		CGMSolar();

		/** @brief ���� */
		~CGMSolar();

		/** @brief ��ʼ�� */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData,
			CGMCommonUniform* pCommonUniform, CGMDataManager* pDataManager);
		/** @brief ���� */
		bool Update(double dDeltaTime);
		/** @brief ����(�������������̬֮��) */
		bool UpdateLater(double dDeltaTime);
		/** @brief ���� */
		bool Load();
		/** @brief ����̫��ϵ�˿̵���Ϣ */
		bool SaveSolarData();

		/**
		* @brief �޸���Ļ�ߴ�ʱ���ô˺���
		* @param iW: ��Ļ���
		* @param iH: ��Ļ�߶�
		*/
		void ResizeScreen(const int iW, const int iH);

		/**
		* @brief ����̫��ϵ
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool CreateSolarSystem();

		/**
		* @brief ���������������ڲ㼶�ռ�+1�����
		* @return Matrix: ��������
		*/
		osg::Matrix HierarchyAddMatrix() const;
		/**
		* @brief ���㽵���������ڲ㼶�ռ�-1�����
		* @return Matrix: ��������
		*/
		osg::Matrix HierarchySubMatrix() const;
		/**
		* @brief ��ȡ��ѯλ�ø��������һ�����壨���ǻ���ǣ���λ��
		* @param vSearchHiePos ��ѯλ�ã���ǰ�ռ�㼶����ϵ
		* @param vPlanetHiePos �������������λ�ã���ǰ�ռ�㼶����ϵ
		* @param fOrbitalPeriod ���ش����ǹ�ת������ڣ�����Ǻ�����Ϊ0����λ����
		* @return bool ��3�㼶�ռ��£����λ�ø�����������true������false
		*/
		bool GetNearestCelestialBody(const SGMVector3& vSearchHiePos,
			SGMVector3& vPlanetHiePos, double& fOrbitalPeriod);

		/**
		* @brief ��ȡĿ��㸽����������壨���ǻ���ǣ���λ��
		* @param vPlanetPos �������������λ�ã���λ����
		* @param fOrbitalPeriod ���ش����ǹ�ת������ڣ���λ����
		*/
		void GetCelestialBody(SGMVector3& vPlanetPos, double& fOrbitalPeriod);

		/**
		* @brief ��ȡĿ��㸽����������壨���ǻ���ǣ���ƽ���뾶
		* @return double ����ƽ���뾶����λ����
		*/
		double GetCelestialMeanRadius() const;
		/**
		* @brief ��ȡĿ��㸽����������壨���ǻ���ǣ���ָ��γ�ȵĺ�ƽ�������ľ���
		* @param fLatitude γ�ȣ���λ���㣬��Χ��[-90.0, 90.0]
		* @return double ָ��γ�ȵĺ�ƽ�������ľ��룬��λ����
		*/
		double GetCelestialRadius(const double fLatitude) const;
		/**
		* @brief ��ȡĿ��㸽����������壨���ǻ���ǣ��ı����ᣨ��ǰ�㼶�ռ��£�
		* @return SGMVector3 �����ᣨ��ǰ�㼶�ռ��£�
		*/
		SGMVector3 GetCelestialNorth() const;
		/**
		* @brief ��ȡĿ��㸽����������壨���ǻ���ǣ��ڱ�����ϵ�е�����
		* @return unsigned int 0������ǣ�1����ת����뾶��С������
		*/
		unsigned int GetCelestialIndex() const;
		/**
		* @brief ��ȡ�۾����θ߶�
		* @return double �۾����θ߶ȣ���λ����
		*/
		double GetEyeAltitude() const;

		/**
		* @brief ��ȡ̫��ϵ��������ӵ���ת��̬
		* @return Quat ��ת��̬��Ԫ��
		*/
		inline osg::Quat GetSolarRotate() const
		{
			return m_qSolarRotate;
		}

		/**
		* @brief ���ó������ڵ�ǰ�ռ�㼶�µ�λ��
		* @param vHiePos ������λ�ã���λ����ǰ�ռ�㼶��λ
		*/
		void SetSupernovaHiePos(const osg::Vec3f& vHiePos);
		/**
		* @brief ���á����˵���ƻ����Ľ�չ
		* @param fProgress ��չ�ٷֱȣ�[0.0, 1.0]
		*/
		void SetWanderingEarthProgress(const float fProgress);

		/**
		* @brief ���ڿռ�㼶�仯�����³���
		* @param iHierarchy:		���º�Ŀռ�㼶���
		* @return bool:				�ɹ�true��ʧ��false */
		bool UpdateHierarchy(int iHierarchy);
		/**
		* @brief ���µ�ǰ����/���������Ϣ����������ı�ǰ���λ�ò�ֵ
		* @param vTargetHiePos:		Ŀ����ڵ�ǰ�ռ��λ��
		* @return SGMVector3:		����ı�ǰ���λ�ò�ֵ����λ���� */
		SGMVector3 UpdateCelestialBody(const SGMVector3& vTargetHiePos);

	private:
		/**
		* @brief ����0/1/2�㼶�ռ�ı���̫��
		*/
		void _CreateBackgroundSun();

		/**
		* @brief ���������ǣ�һ�ŷǳ������ĺ���
		*	�������κβ㼶��������Ƶʱ,��12345�㼶�ռ䣬ʼ�տɼ�
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateSupernova();

		/**
		* @brief �����ڶ��㼶�ռ��̫��
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateSun();

		/**
		* @brief ����2��3�㼶������,Χ�ƺ�����ת
		* @author LiuTao
		* @since 2021.11.28
		* @param void��			��
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreatePlanets();

		/**
		* @brief ������ǣ�Χ�ƺ�����ת
		* @param fRadius��			��ת�뾶����λ����
		* @param fOrbitalPeriod:	��ת���ڣ���λ����
		* @param fStartTheta:		��ת��ʼλ�ã���λ������
		* @return bool:				�ɹ�true��ʧ��false
		*/
		bool _AddPlanet(
			const double fRadius,
			const double fOrbitalPeriod,
			const double fStartTheta);

		/**
		* @brief ������1�㼶�ռ������ϵͳ��ÿ�������ж����ʣ����ǻ������ǵ�
			�����������_CreatePlanets
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreatePlanetSystem_1();
		/**
		* @brief ������2�㼶�ռ������ϵͳ��ÿ�������ж����ʣ����ǻ������ǵ�
			�����������_CreatePlanets
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreatePlanetSystem_2();

		/**
		* @brief ����������
		* @param fWidth��			�����α߳�
		* @param bCorner��			���������ĵ��Ƿ��ڽ��ϣ�true == �ڽ��ϣ�false == ������
		* @return Geometry*:		�����ļ�����ڵ�ָ�룬δ�ɹ��򷵻�nullptr
		*/
		osg::Geometry* _CreateSquareGeometry(const float fWidth, const bool bCorner = false);

		/**
		* @brief �����⻷��ĳһ����Ƭ
		* @param iID					����ÿ����ƬID�ţ�0,1,2,3,4,5
		* @return Geometry				���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeRingGeometry(unsigned int iID);

		/**
		* @brief �޸Ĺ⻷�뾶
		* @param fRadiusMin				�ڰ뾶����λ����
		* @param fRadiusMax				��뾶����λ����
		*/
		void _SetRingRadius(double fRadiusMin, double fRadiusMax);

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

		osg::Texture* _Load3DShapeNoise() const;

		/**
		* @brief �޸����������ɫת������
		* @param vColor:			����Ĵ�����ɫ
		* @return osg::Matrixf:		�������ɫת������
		*/
		osg::Matrixf _ColorTransMatrix(const osg::Vec4f& vColor) const;

		/**
		* @brief ��ʼ��С���Ǵ�
		*/
		void _InitAsteroidBelt();
		/**
		* @brief ��ϴС���Ǵ����ݣ����ص���С���Ǽ��ٵ�һ������ĺܸߵ��ٶȣ���ʵ������Ŀ��
		*/
		void _WashAsteroidBeltData();

		/**
		* @brief ������Ⱦ�� ��Ļ������С�������棬�Ⱦ���Ч��Ҫ��һЩ
		* @param width: ��Ч���εĿ��
		* @param height: ��Ч���εĸ߶�
		* @return osg::Geometry* ���ؼ��νڵ�ָ��
		*/
		osg::Geometry* _CreateScreenTriangle(const int width, const int height);

		/**
		* @brief ����������ת�����̬
		* @param dDeltaTime: ��һ֡���ʱ�䣬��λ����
		*/
		void _UpdatePlanetRotate(double dDeltaTime);
		/**
		* @brief ��ȡĳһʱ�̵�ǰ������ת��Ԫ��
		* @return osg::Quat ��ǰ������ת��Ԫ��
		*/
		osg::Quat _GetPlanetSpin() const;
		/**
		* @brief ��ȡĳһʱ�̵�ǰ������ת�������Ԫ��
		* @return osg::Quat ��ǰ������ת�������Ԫ��
		*/
		osg::Quat _GetPlanetInclination() const;
		/**
		* @brief ��ȡĳһʱ�̵�ǰ������ת��ƫ������Ԫ����������˵���
		* @return osg::Quat ��ǰ������ת��ƫ������Ԫ��
		*/
		osg::Quat _GetPlanetTurn() const;

		// ����
	private:
		SGMKernelData*									m_pKernelData;					//!< �ں�����
		SGMConfigData*									m_pConfigData;					//!< ��������
		CGMCommonUniform*								m_pCommonUniform;				//!< ����Uniform
		std::vector<osg::ref_ptr<osg::Group>>			m_pHieSolarRootVector;			//!< 0123�ռ�㼶�ĸ��ڵ�

		osg::ref_ptr<osg::Texture>						m_3DShapeTex;					//!< ��ά�޷�������RGBA��128^3
		// ��������
		std::vector<SGMCelestialBody>					m_sCelestialBodyVector;
		// ��ǰĿ�������������е�˳��
		unsigned int									m_iCenterCelestialBody;
		std::default_random_engine						m_iRandom;						//!< ���ֵ

		std::string										m_strGalaxyShaderPath;			//!< galaxy shader ��ɫ��·��
		std::string										m_strGalaxyTexPath;				//!< galaxy texture ��ͼ·��
		std::string										m_strCoreModelPath;				//!< ����ģ����Դ·��
		osg::ref_ptr<osg::Transform>					m_pStar_1_Transform;			//!< ��1�㼶���ĺ��ǡ����Ǳ任���
		osg::ref_ptr<osg::Transform>					m_pStar_2_Transform;			//!< ��2�㼶���ĺ��ǡ����Ǳ任���
		osg::ref_ptr<osg::Transform>					m_pPlanet_1_Transform;			//!< ��1�㼶��ǰ���Ǳ任���
		osg::ref_ptr<osg::Transform>					m_pPlanet_2_Transform;			//!< ��2�㼶��ǰ���Ǳ任���
		osg::ref_ptr<osg::Transform>					m_pRing_2_Transform;			//!< ��2�㼶���ǹ⻷�任���
		osg::ref_ptr<osg::Transform>					m_pPlanetTailTransform;			//!< ����β���ı任���
		osg::ref_ptr<osg::AutoTransform>				m_pSunBloomTransform;			//!< ̫���Թ�ı任���
		osg::ref_ptr<osg::AutoTransform>				m_pBackgroundSunTransform;		//!< 0/1/2�㼶����̫���任���
		osg::ref_ptr<osg::AutoTransform>				m_pSupernovaTransform;			//!< �����ǵı任���

		osg::Matrixd									m_mView2ECEFMatrix;				//!< view ת ECEF �ľ���
		osg::Matrixd									m_mWorld2Shadow;				//!< ����ռ� ת ��Ӱ�ռ� �ľ���
		osg::Vec3d										m_vPlanetNorth;					//!< ����ռ������ǵı�����
		osg::Vec3d										m_vPlanetAxisX;					//!< ����ռ������ǵ�X��
		osg::Quat										m_qSolarRotate;					//!< ̫��ϵ���������ϵ����̬
		osg::Quat										m_qPlanetRotate;				//!< ��ǰ���������̫��ϵ����̬
		osg::Vec3d										m_vSolarPos_Hie1;				//!< ̫���ڵ�1�㼶�ռ��µ�����
		osg::Vec3d										m_vSolarPos_Hie2;				//!< ̫���ڵ�2�㼶�ռ��µ�����
		double											m_fEyeAltitude;					//!< �۵㺣�Σ���λ����
		float											m_fWanderingEarthProgress;		//!< ���˵���ƻ���չ[0.0,1.0]

		osg::ref_ptr<osg::Geometry>						m_pPlanetGeom_1;				//!< ��1�㼶���ǹ�������
		osg::ref_ptr<osg::Geometry>						m_pPlanetGeom_2;				//!< ��2�㼶���ǹ�������
		osg::ref_ptr<osg::Geode>						m_pGeodeSun_2;					//!< ��2�㼶̫��Geode
		osg::ref_ptr<osg::Geode>						m_pRingGeode_2;					//!< ��2�㼶���ǹ⻷Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeAsteroid_2;				//!< ��2�㼶С���Ǵ��㾫��Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePlanets_2;				//!< ��2�㼶���ǵ㾫��Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePlanetsLine_2;			//!< ��2�㼶���ǹ����Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeAsteroid_3;				//!< ��3�㼶С���Ǵ��㾫��Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePlanets_3;				//!< ��3�㼶���ǵ㾫��Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePlanetsLine_3;			//!< ��3�㼶���ǹ����Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeSupernovaX;				//!< ǰ�������ǵ�ʮ�ֽ��
		osg::ref_ptr<osg::Geode>						m_pGeodeSupernovaBloom;			//!< ǰ�������ǵĻԹ���
		osg::ref_ptr<osg::Geode>						m_pGroundRoot;					//!< ���ǵ���ĸ��ڵ�
		osg::ref_ptr<osg::Geode>						m_pCloudRoot;					//!< �����Ʋ�ĸ��ڵ�
		osg::ref_ptr<osg::Geode>						m_pAtmosRoot;					//!< ���Ǵ����ĸ��ڵ�
		osg::ref_ptr<osg::StateSet>						m_pSSPlanetGround;				//!< ���ǵ����״̬��
		osg::ref_ptr<osg::StateSet>						m_pSSPlanetCloud;				//!< �����Ʋ��״̬��
		osg::ref_ptr<osg::StateSet>						m_pSSPlanetAtmos;				//!< ���Ǵ�����״̬��

		osg::ref_ptr<osg::Uniform>						m_fBackgroundSunScaleUniform;	//!< ����̫�����ű�����Uniform
		osg::ref_ptr<osg::Uniform>						m_fBackgroundSunAlphaUniform;	//!< ����̫��alpha��Uniform
		osg::ref_ptr<osg::Uniform>						m_fSunEdgeUniform;				//!< ̫���Թ���ϵ�̫����Եλ��Uniform
		osg::ref_ptr<osg::Uniform>						m_fSupernovaLightUniform;		//!< ���Ƴ����ǵ����ȵ�Uniform
		osg::ref_ptr<osg::Uniform>						m_fSupernovaAlphaUniform;		//!< ѣ�⣨���ģ���״����alpha
		osg::ref_ptr<osg::Uniform>						m_fSupernovaBloomScaleUniform;	//!< ���Ƴ����ǵ�bloom��С��Uniform
		osg::ref_ptr<osg::Uniform>						m_fSupernovaBloomAlphaUniform;	//!< ���Ƴ����ǵ�bloom��͸���ȵ�Uniform
		osg::ref_ptr<osg::Uniform>						m_fPlanetNumUniform;			//!< ����������Uniform
		osg::ref_ptr<osg::Uniform>						m_fPlanetTailAlphaUniform;		//!< ����β���ߵ�alpha��Uniform
		osg::ref_ptr<osg::Uniform>						m_fPlanetLineAlphaUniform;		//!< ���ǹ켣�ߵ�alpha��Uniform
		osg::ref_ptr<osg::Uniform>						m_vPlanetRadiusUniform;			//!< ���ǵ������뾶���ڶ��㼶��
		osg::ref_ptr<osg::Uniform>						m_fPlanetPointAlphaUniform;		//!< ���ǵ㾫���alpha��Uniform
		osg::ref_ptr<osg::Uniform>						m_vViewLightUniform;			//!< view�ռ�Ĺ�Դ����
		osg::ref_ptr<osg::Uniform>						m_mRingShadowMatrixUniform;		//!< 2�㼶�⻷ת��Ӱ�ռ�ľ���
		osg::ref_ptr<osg::Uniform>						m_mPlanetShadowMatrixUniform;	//!< 2�㼶����ת��Ӱ�ռ�ľ���
		osg::ref_ptr<osg::Uniform>						m_fNorthDotLightUniform;		//!< ������ת��������ǰ������ĵ��
		osg::ref_ptr<osg::Uniform>						m_fCloudTopUniform;				//!< �Ʋ㶥��
		osg::ref_ptr<osg::Uniform>						m_fGroundTopUniform;			//!< �������ɽ���ĺ��θ�
		osg::ref_ptr<osg::Uniform>						m_fAtmosHeightUniform;			//!< ������߶�
		osg::ref_ptr<osg::Uniform>						m_fMinDotULUniform;				//!< �й��������СDotULֵ(-1,0)
		osg::ref_ptr<osg::Uniform>						m_fEyeAltitudeUniform;			//!< �۵㺣��Uniform
		osg::ref_ptr<osg::Uniform>						m_mAtmosColorTransUniform;		//!< ������ɫת������
		osg::ref_ptr<osg::Uniform>						m_mView2ECEFUniform;			//!< view�ռ�תECEF�ľ���
		osg::ref_ptr<osg::Uniform>						m_fStepTimeUniform;				//!< ���沽������ʵʱ�䣬��λ����
		osg::ref_ptr<osg::Uniform>						m_vJupiterPosUniform;			//!< ľ����̫��ϵ�µ����꣬��λ����
		osg::ref_ptr<osg::Uniform>						m_vCoordScaleUniform;			//!< ������ͼ��������������

		osg::ref_ptr<osgDB::Options>					m_pDDSOptions;					//!< dds���������
		osg::ref_ptr<osg::Texture2DArray>				m_aMercuryBaseTex;				//!< ˮ��base color����
		osg::ref_ptr<osg::Texture2DArray>				m_aVenusBaseTex;				//!< ����base color����
		osg::ref_ptr<osg::Texture2DArray>				m_aVenusCloudTex;				//!< �����Ʋ�����
		osg::ref_ptr<osg::Texture2DArray>				m_aMoonBaseTex;					//!< ����base color����
		osg::ref_ptr<osg::Texture2DArray>				m_aMarsBaseTex;					//!< ����base color����
		osg::ref_ptr<osg::Texture2DArray>				m_aJupiterCloudTex;				//!< ľ��base color����
		osg::ref_ptr<osg::Texture>						m_pJupiterRingTex;				//!< ľ�ǹ⻷����
		osg::ref_ptr<osg::Texture2DArray>				m_aSaturnCloudTex;				//!< ����base color����
		osg::ref_ptr<osg::Texture>						m_pSaturnRingTex;				//!< ���ǹ⻷����
		osg::ref_ptr<osg::Texture2DArray>				m_aUranusCloudTex;				//!< ������base color����
		osg::ref_ptr<osg::Texture>						m_pUranusRingTex;				//!< �����ǹ⻷����
		osg::ref_ptr<osg::Texture2DArray>				m_aNeptuneCloudTex;				//!< ������base color����
		osg::ref_ptr<osg::Texture>						m_pNeptuneRingTex;				//!< �����ǹ⻷����
		osg::ref_ptr<osg::Texture2DArray>				m_aPlutoBaseTex;				//!< ڤ����base color����
		osg::ref_ptr<osg::Texture2DArray>				m_aCharonBaseTex;				//!< ����base color����
		//osg::ref_ptr<osg::Texture2DArray>				m_aAuroraTex;					//!< ��������
		std::vector<osg::ref_ptr<osg::Vec3Array>>		m_ringVertVector;				//!< �⻷��������
		osg::ref_ptr<osg::Vec3Array>					m_pPlanetLineVerts_2;			//!< ��2�㼶���ǹ켣�߶�������
		osg::ref_ptr<osg::Vec3Array>					m_pPlanetLineCoords_2;			//!< ��2�㼶���ǹ켣��UVW����
		osg::ref_ptr<osg::DrawElementsUShort>			m_pPlanetLineElement_2;			//!< ��2�㼶���ǹ켣��Ԫ��
		osg::ref_ptr<osg::Vec3Array>					m_pPlanetLineVerts_3;			//!< ��3�㼶���ǹ켣�߶�������
		osg::ref_ptr<osg::Vec3Array>					m_pPlanetLineCoords_3;			//!< ��3�㼶���ǹ켣��UVW����
		osg::ref_ptr<osg::DrawElementsUShort>			m_pPlanetLineElement_3;			//!< ��3�㼶���ǹ켣��Ԫ��

		osg::ref_ptr<osg::Vec4Array>					m_pPlanetVertArray_2;			//!< ��2�㼶���Ƕ�������
		osg::ref_ptr<osg::Vec2Array>					m_pPlanetVertCoords_2;			//!< ��2�㼶���Ƕ�����������
		osg::ref_ptr<osg::DrawElementsUShort>			m_pPlanetElement_2;				//!< ��2�㼶����Ԫ��
		osg::ref_ptr<osg::Vec4Array>					m_pPlanetVertArray_3;			//!< ��3�㼶���Ƕ�������
		osg::ref_ptr<osg::Vec2Array>					m_pPlanetVertCoords_3;			//!< ��3�㼶���Ƕ�����������
		osg::ref_ptr<osg::DrawElementsUShort>			m_pPlanetElement_3;				//!< ��3�㼶����Ԫ��

		int												m_iPlanetCount;					//!< ��������������

		osg::ref_ptr<CGMDispatchCompute>				m_pAsteroidComputeNode;			//!< ����С���Ǵ���CS�ڵ�
		osg::ref_ptr<osg::Camera>						m_pReadAsteroidCam;				//!< ���ڶ�ȡС���Ǵ������
		CReadPixelFinishCallback*						m_pReadPixelFinishCallback;
		CGMCelestialScaleVisitor*						m_pCelestialScaleVisitor;		//!< ���ڿ��������С

		CGMTerrain*										m_pTerrain;						//!< ����ģ��
		CGMAtmosphere*									m_pAtmos;						//!< ����ģ��
		CGMEarth*										m_pEarth;						//!< ����ģ��(������ģ��)
		CGMPlanet*										m_pPlanet;						//!< ����ģ�飨���������ǣ�
		CGMOort*										m_pOort;						//!< �¶�����ģ��
	};
}	// GM