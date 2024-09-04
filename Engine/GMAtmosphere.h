//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMAtmosphere.h
/// @brief		Galaxy-Music Engine - GMAtmosphere
/// @version	1.0
/// @author		LiuTao
/// @date		2023.11.30
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommon.h"
#include "GMCommonUniform.h"
#include "GMKernel.h"
#include "GMDispatchCompute.h"

#include <random>
#include <osg/Node>
#include <osg/Texture>

namespace GM
{
	/*************************************************************************
	Macro Defines
	*************************************************************************/

	/*************************************************************************
	constexpr
	*************************************************************************/

	constexpr double ATMOS_FADE_R = 5.802e-6;		// �����ĺ��ɢ��ϵ��
	constexpr double ATMOS_FADE_G = 1.3558e-5; 		// �������̹�ɢ��ϵ��
	constexpr double ATMOS_FADE_B = 3.31e-5; 		// ����������ɢ��ϵ��

	constexpr double ATMOS_RAYLEIGH_H = 0.132; 		// ����������ɢ���߱���
	constexpr double ATMOS_MIE_H = 0.019; 			// ����������ɢ���߱���
	constexpr int ATMOS_MIN = 16;					// ��С�Ĵ�����ȣ���λ��km

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
	*  @class CGMAtmosphere
	*  @brief Galaxy-Music CGMAtmosphere
	*/
	class CGMAtmosphere
	{
		// ����
	public:
		/** @brief ���� */
		CGMAtmosphere();

		/** @brief ���� */
		~CGMAtmosphere();

		/** @brief ��ʼ�� */
		bool Init(SGMConfigData* pConfigData);

		/**
		* @brief ���ݡ�������ȡ��͡�����뾶������ȡ��Ӧ��ɢ������
		* @param fAtmosH:			������ȣ���λ����
		* @param fRadius:			����뾶����λ����
		* @return osg::Texture3D*:	��Ӧ��ɢ������
		*/
		osg::Texture3D* GetInscattering(const EGMAtmosHeight eAtmosH, const double& fRadius);

		/**
		* @brief ���ݡ�������ȡ�ö��ֵ����������
		* @param eAtmosHeight:		�������ö��
		* @return float:			�������,��λ����
		*/
		inline float GetAtmosHeight(const EGMAtmosHeight eAtmosHeight) const
		{
			return ATMOS_MIN * 1e3 * exp2(int(eAtmosHeight) - 1);
		};

		/**
		* @brief ���ݡ������ܺ�ȡ��͡�����뾶���������й������"DotUL"��Сֵ��dot(upDir, lightDir)
		* @param fAtmosThick:		�����ܺ�ȣ���λ����
		* @param fRadius:			����뾶����λ����
		* @return float:			�й������"DotUL"��Сֵ,��Χ��(-1.0f, 0.0f)
		*/
		inline float GetMinDotUL(const double& fAtmosThick, const double& fRadius) const
		{
			float fSinUL = fRadius / (fAtmosThick + fRadius);
			return -sqrt(std::fmax(0.0f, 1.0f - fSinUL * fSinUL));
		};

	private:

		/**
		* @brief ���ɴ�����͸���ʡ�����
		*/
		void _MakeAtmosTransmittance();

		/**
		* @brief ���ɴ��������նȡ�����
		*/
		void _MakeAtmosIrradiance();

		/**
		* @brief ���ɴ�������ɢ�䡱����
		*/
		void _MakeAtmosInscattering();

		/**
		* @brief ���������͸���ʡ�
		*/
		osg::Vec3d _Transmittance(const double& fAtmosDens,
			const double& fR, const double& fAtmosThick,
			const osg::Vec2d& vP0, const osg::Vec2d& vP1);

		/**
		* @brief ���ݡ����θ߶ȡ��͡������ܺ�ȡ��������λ�õ�����ɢ��ϵ��
		* @param fAlt:				���θ߶ȣ���λ����
		* @param fAtmosThick:		�����ܺ�ȣ���λ����
		* @return osg::Vec3d:		��λ�õ�����ɢ��ϵ��,(0,1]
		*/
		inline osg::Vec3d _RayleighCoefficient(const double& fAlt, const double& fAtmosThick) const
		{
			// �����ܶ���߶�˥�������е����������ɢ��ı�ߣ�8500m
			double fEarthH = fAtmosThick * ATMOS_RAYLEIGH_H;
			return osg::Vec3d(ATMOS_FADE_R, ATMOS_FADE_G, ATMOS_FADE_B) * exp2(-std::fmax(0, fAlt) / fEarthH);
		};
		/**
		* @brief ���ݡ����θ߶ȡ��͡������ܺ�ȡ��������λ�õ�����ɢ��ϵ��
		* @param fAlt:				���θ߶ȣ���λ����
		* @param fAtmosThick:		�����ܺ�ȣ���λ����
		* @return double:			��λ�õ�����ɢ��ϵ��,(0,1]
		*/
		inline double _MieCoefficient(const double& fAlt, const double& fAtmosThick) const
		{
			// �����ܶ���߶�˥�������е����������ɢ��ı�ߣ�1200m
			double fEarthH = fAtmosThick * ATMOS_MIE_H;
			return 3.996e-6 * exp2(-std::fmax(0, fAlt) / fEarthH);
		};

		/**
		* @brief ����ɢ����λ����
		* @param fCosVL:		viewDir��LightSource�ļн�����ֵ
		* @return double:		����ɢ����λ����
		*/
		inline double _RayleighPhase(const double& fCosVL) const
		{
			return 3.0 / (16 * osg::PI) * (1 + fCosVL * fCosVL);
		};
		/**
		* @brief ����ɢ����λ����
		* @param fCosVL:		viewDir��LightSource�ļн�����ֵ
		* @return double:		����ɢ����λ����
		*/
		inline double _MiePhase(const double& fCosVL) const
		{
			constexpr double g = 0.8;
			constexpr double g2 = g * g;
			const double a = 3.0 / (8 * osg::PI);
			constexpr double b = (1 - g2) / (2 + g2);
			double c = 1.0 + fCosVL * fCosVL;
			double d = pow(1 + g2 - 2 * g * fCosVL, 1.5);
			return a * b * (c / d);
		};

		/**
		* @brief ����ɢ�䵼�µ�����
		* @param fAlt:				���θ߶ȣ���λ����
		* @param fAtmosThick:		�����ܺ�ȣ���λ����
		* @return osg::Vec3d:		����ɢ�䵼�µ����ձ���
		*/
		inline osg::Vec3d _MieAbsorption(const double& fAlt, const double& fAtmosThick) const
		{
			// �����ܶ���߶�˥�������е����������ɢ��ı�ߣ�1200m
			double fEarthH = fAtmosThick * ATMOS_MIE_H;
			double fMie = 4.4e-6 * exp2(-std::fmax(0, fAlt) / fEarthH);
			return osg::Vec3d(fMie, fMie, fMie);
		};
		/**
		* @brief �����������
		* @param fAlt:				���θ߶ȣ���λ����
		* @param fAtmosThick:		�����ܺ�ȣ���λ����
		* @return osg::Vec3d:		����������ձ���
		*/
		inline osg::Vec3d _OzoneAbsorption(const double& fAlt, const double& fAtmosThick) const
		{
			// �����еĳ�����ozone��Ҳ�������ߵ����գ������Բ�ͬ�����Ĺ����Ų�ͬ������Ч�ʣ���������ɢ��û�й���
			// ��������һ���ض��߶ȵĲ㣬����ͨ�������ĸ߶ȡ��͡����ȡ�������������������������зֱ�ȡ 25km �� 15km
			// �����������̫���ӣ���û�����ۿ��������ԾͲ�������Щ���أ������͵������һ����ֻ��������ɫ�б仯
			double fCenterHeight = fAtmosThick * 0.39;
			double fHalfThick = fAtmosThick * 0.234;
			return osg::Vec3d(0.65e-6, 1.881e-6, 0.085e-6) * std::fmax(0, 1- std::abs(fAlt- fCenterHeight) / fHalfThick);
		};

		/**
		* @brief ���ݡ������ܺ�ȡ�������ر����Դ����ܶȣ��涨����ƽ������ܶ�Ϊ1
		* @param fAtmosThick:		�����ܺ�ȣ���λ����
		* @return double:			�ر����Դ����ܶ�
		*/
		inline double _GetAtmosBottomDens(const double& fAtmosThick) const
		{
			// ��֤����ƽ������ܶ�Ϊ1
			return fAtmosThick / 64000.0;
		};

		// ����
	private:
		SGMConfigData*									m_pConfigData;					//!< ��������

		std::default_random_engine						m_iRandom;						//!< ���ֵ
		std::string										m_strCoreModelPath;				//!< ����ģ����Դ·��
		std::vector<osg::ref_ptr<osg::Texture3D>>		m_pInscatteringTexVector;		//!< ��ɢ����������
	};
}	// GM