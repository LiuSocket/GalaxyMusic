//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMPlanet.h
/// @brief		Galaxy-Music Engine - GMPlanet
/// @version	1.0
/// @author		LiuTao
/// @date		2023.09.23
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommon.h"
#include "GMKernel.h"
#include "GMCelestialScaleVisitor.h"

namespace GM
{
	/*************************************************************************
	Class
	*************************************************************************/

	/*!
	*  @class CGMPlanet
	*  @brief Galaxy-Music CGMPlanet
	*/
	class CGMPlanet
	{
		// ����
	public:
		/** @brief ���� */
		CGMPlanet();

		/** @brief ���� */
		~CGMPlanet();

		/** @brief ��ʼ�� */
		bool Init(SGMConfigData* pConfigData);

		/**
		* @brief ����������ϸ�ֺ�����壬ÿ�����㶼�з��ߺ�UV
		* UV.xy = WGS84��Ӧ��UV��[0.0, 1.0]
		* UV.zw = ��������ͼUV��[0.0, 1.0]
		* @param iSegment:			�������ÿ���߳��ķֶ���
		* @return Geometry:			���ؼ�����ָ��
		*/
		osg::Geometry* MakeHexahedronSphereGeometry(int iSegment = 32);

		/**
		* @brief ����������
		* @param ellipsoid				�����������������ģ��
		* @param iLonSegments			���ȷֶ���
		* @param iLatSegments			γ�ȷֶ���
		* @param fHae					���Ӱ뾶�������ں��θ߶ȣ�
		* @param bGenTexCoords			�Ƿ�����UV����
		* @param bWholeMap				�Ƿ���ʾ������ͼ(�ڰ�����߸�������ʱ)
		* @param bFlipNormal			true �������ڣ�false ��������
		* @param fLatStart				γ�ȿ�ʼλ�ã���λ���Ƕ� ��
		* @param fLatEnd				γ�Ƚ���λ�ã���λ���Ƕ� ��
		* @return Geometry				���ش����ļ�����ָ��
		*/
		osg::Geometry* MakeEllipsoidGeometry(
			const osg::EllipsoidModel*	ellipsoid,
			int							iLonSegments,
			int							iLatSegments,
			float						fHae,
			bool						bGenTexCoords,
			bool						bWholeMap = false,
			bool						bFlipNormal = false,
			float						fLatStart = -90.0,
			float						fLatEnd = 90.0) const;

	protected:
		/** @brief ���ߺ�����ƽ������Ҫ���� */
		void Panorama2CubeMap();

	private:
		/**
		* @brief ȫ��DEMתcubemap�汾DEM
		* @param strPanoPath: ȫ��DEM_bed·��
		* @param strCubeMapPath: CubeDEM·��
		* @return bool: �ɹ�true��ʧ��false
		*/
		bool _Panorama_2_CubeDEM(
			const std::string& strPanoPath,
			const std::string& strCubeDEMPath);
		/**
		* @brief ȫ��ͼתcubemap
		* @param strPanoramaPath: ȫ��ͼ·��
		* @param strCubeMapPath: CubeMap·��
		* @return bool: �ɹ�true��ʧ��false
		*/
		bool _Panorama_2_CubeMap(
			const std::string& strPanoramaPath,
			const std::string& strCubeMapPath);

		// ����
	protected:
		SGMConfigData*							m_pConfigData;					//!< ��������
		std::string								m_strCoreModelPath;				//!< ����ģ����Դ·��
		CGMCelestialScaleVisitor*				m_pCelestialScaleVisitor;		//!< ���ڿ��������С
	};
}	// GM