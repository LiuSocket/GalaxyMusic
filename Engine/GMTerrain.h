//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMTerrain.h
/// @brief		Galaxy-Music Engine - GMTerrain
/// @version	1.0
/// @author		LiuTao
/// @date		2023.12.31
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommonUniform.h"
#include "GMCelestialScaleVisitor.h"

namespace GM
{
	/*!
	*  @class CGMTerrain
	*  @brief Galaxy-Music GMTerrain
	*/
	class CGMTerrain
	{
		// ����
	public:
		/** @brief ���� */
		CGMTerrain();

		/** @brief ���� */
		~CGMTerrain();

		/** @brief ��ʼ�� */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief ���� */
		bool Update(double dDeltaTime);
		/** @brief ����(�������������̬֮��) */
		bool UpdateLater(double dDeltaTime);
		/** @brief ���� */
		bool Load();

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
		osg::Node* GetTerrainRoot(const int iHie) const;

		/**
		* @brief ��������
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool CreateTerrain();

		/**
		* @brief ���ڿռ�㼶�仯�����³���
		* @param iHierarchy:		���º�Ŀռ�㼶���
		* @return bool:				�ɹ�true��ʧ��false */
		bool UpdateHierarchy(int iHierarchy);

	private:

		/**
		* @brief ������0�㼶�ռ�ĵ���
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateTerrain_0();
		/**
		* @brief ������1�㼶�ռ�ĵ���
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateTerrain_1();

		/**
		* @brief ����������ϸ�ֺ�������һ������ķ�֮һ�Ĳ��֣�ÿ�����㶼�з��ߺ�UV
		* UV0.xy = WGS84��Ӧ��UV��[0.0, 1.0]
		* UV1.xy = ��������ͼUV��[0.0, 1.0]
		* UV1.z = ������ID��0,1,2,3,4,5
		* @param bPolar:			�Ƿ��Ǽ�������
		* @param iHalfSegment:		�ķ�֮һ����ı߳��ķֶ�����Ҳ����һ��������İ�߳��ķֶ���
		* @return Geometry:			���ؼ�����ָ��
		*/
		osg::Geometry* _MakeHexahedronQuaterGeometry(const bool bPolar, int iHalfSegment = 64) const;

		/**
		* @brief ���ݶ������Ϣ��ȡ����������������⴦��������ڱ�����ϵĶ���
		* @param iX��iY: �����XYλ��
		* @param iHalfSeg: �ķ�֮һ����ı߳��ķֶ�����Ҳ����һ��������İ�߳��ķֶ���
		* @return int: ���������
		*/
		inline int _GetVertIndex(const int iX, const int iY, const int iHalfSeg) const
		{
			return iY * (iHalfSeg + 1) + iX;
		}

		// ����
	private:
		SGMKernelData*								m_pKernelData;					//!< �ں�����
		SGMConfigData*								m_pConfigData;					//!< ��������
		CGMCommonUniform*							m_pCommonUniform;				//!< ����Uniform
		std::vector<osg::ref_ptr<osg::Group>>		m_pHieTerrainRootVector;		//!< 01�ռ�㼶�ĸ��ڵ�

		std::string									m_strGalaxyShaderPath;			//!< galaxy shader ·��
		std::string									m_strTerrainShaderPath;			//!< Terrain shader ·��

		std::vector<osg::ref_ptr<osg::Transform>>	m_pTerrainPolarTransVec;		//!< �ķ�֮һ��ļ��ؽڵ�vector
		std::vector<osg::ref_ptr<osg::Transform>>	m_pTerrainEquatorTransVec;		//!< �ķ�֮һ��ĳ���ڵ�vector
		CGMCelestialScaleVisitor*					m_pCelestialScaleVisitor;		//!< ���ڿ��������С
	};
}	// GM