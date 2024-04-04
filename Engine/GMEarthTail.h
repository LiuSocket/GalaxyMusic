//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMEarthTail.h
/// @brief		Galaxy-Music Engine - GMEarthTail.h
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
	constexpr
	*************************************************************************/
	constexpr double EARTH_TAIL_RADIUS = 7.2e6;		// β���뾶����λ����
	constexpr double EARTH_TAIL_LENGTH = 4e7;		// β�����ȣ��ӵ������Ŀ�ʼ���𣩣���λ����

	/*************************************************************************
	Enums
	*************************************************************************/

	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/

	/*
	** ����β�������еķ�����
	*/
	class CEarthTailBoxVisitor : public osg::NodeVisitor
	{
	public:
		CEarthTailBoxVisitor() : NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
			_fUnit(1e10), _iPeriod(0) {}

		void SetUnit(const double fUnit)
		{
			_fUnit = fUnit;
		}

		/**
		* @brief �������˵���ƻ��Ľ׶�
		* @param iWanderPeriod
		* 0 = ǰ̫��ʱ��
		* 1 = ɲ��ʱ��
		* 2 = ����ת��ʱ��
		* 3 = ����ת��ʱ��
		* 4 = ����ʱ��
		*/
		void SetPeriod(const int iPeriod)
		{
			_iPeriod = iPeriod;
		}

		void apply(osg::Node& node) { traverse(node); }
		void apply(osg::Geode& node)
		{
			osg::Geometry* geom = dynamic_cast<osg::Geometry*>(node.getDrawable(0));
			if (!geom) return;

			osg::ref_ptr<osg::Vec3Array> pVert = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
			if (!pVert.valid()) return;

			switch (_iPeriod)
			{
			case 1:
			{
				float fR = 1.2e7 / _fUnit;
				float fZ = 6.4e6 / _fUnit;

				pVert->at(0) = osg::Vec3(-fR, -fR, -fZ); // 0
				pVert->at(1) = osg::Vec3(fR, -fR, -fZ); // 1
				pVert->at(2) = osg::Vec3(fR, fR, -fZ); // 2
				pVert->at(3) = osg::Vec3(-fR, fR, -fZ); // 3
				pVert->at(4) = osg::Vec3(-fR, -fR, fZ); // 4
				pVert->at(5) = osg::Vec3(fR, -fR, fZ); // 5
				pVert->at(6) = osg::Vec3(fR, fR, fZ); // 6
				pVert->at(7) = osg::Vec3(-fR, fR, fZ); // 7
			}
			break;
			case 2:
			case 3:
			{
				float fR = 1e7 / _fUnit;

				pVert->at(0) = osg::Vec3(-fR, -fR, -fR); // 0
				pVert->at(1) = osg::Vec3(fR, -fR, -fR); // 1
				pVert->at(2) = osg::Vec3(fR, fR, -fR); // 2
				pVert->at(3) = osg::Vec3(-fR, fR, -fR); // 3
				pVert->at(4) = osg::Vec3(-fR, -fR, fR); // 4
				pVert->at(5) = osg::Vec3(fR, -fR, fR); // 5
				pVert->at(6) = osg::Vec3(fR, fR, fR); // 6
				pVert->at(7) = osg::Vec3(-fR, fR, fR); // 7
			}
			break;
			case 4:
			{
				float fR = EARTH_TAIL_RADIUS / _fUnit;
				float fL = EARTH_TAIL_LENGTH / _fUnit;

				pVert->at(0) = osg::Vec3(-fR, -fR, -fR); // 0
				pVert->at(1) = osg::Vec3(fR, -fR, -fR); // 1
				pVert->at(2) = osg::Vec3(fR, fR, -fR); // 2
				pVert->at(3) = osg::Vec3(-fR, fR, -fR); // 3
				pVert->at(4) = osg::Vec3(-fR, -fR, fL); // 4
				pVert->at(5) = osg::Vec3(fR, -fR, fL); // 5
				pVert->at(6) = osg::Vec3(fR, fR, fL); // 6
				pVert->at(7) = osg::Vec3(-fR, fR, fL); // 7
			}
			break;
			default:
				break;
			}

			pVert->dirty();

			traverse(node);
		}
	private:
		double				_fUnit;
		int					_iPeriod; // ���˵���ƻ������׶�
	};

	/*!
	*  @class CGMEarthTail
	*  @brief Galaxy-Music EarthTail Module
	*/
	class CGMEarthTail : public CGMVolumeBasic
	{
		// ����
	public:
		/** @brief ���� */
		CGMEarthTail();
		/** @brief ���� */
		~CGMEarthTail();

		/** @brief ��ʼ�� */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief ���� */
		bool Update(double dDeltaTime);
		/** @brief ����(�������������̬֮��) */
		bool UpdateLater(double dDeltaTime);
		/** @brief ���� */
		bool Load();
		/**
		* @brief �������˵���β��
		*/
		void MakeEarthTail();

		/**
		* @brief ���ڿռ�㼶�仯�����³���
		* @param iHierarchy:		���º�Ŀռ�㼶���
		* @return bool:				�ɹ�true��ʧ��false
		*/
		bool UpdateHierarchy(int iHieNew);

		/**
		* @brief ��ʾ���������˵���β��
		* @param bVisible: �Ƿ�ɼ�
		*/
		void SetVisible(const bool bVisible);

		/**
		* @brief �������˵����β���ڵ�2�ռ�㼶�µ���ת
		* @param fSpin�� ��ת����λ������
		* @param fObliquity�� ��ת����ǣ���λ������
		* @param fNorthYaw�� ��ת��ƫ���ǣ���λ������
		*/
		void SetEarthTailRotate(const double fSpin, const double fObliquity, const double fNorthYaw);

		/* @brief ����Uniform */
		void SetUniform(
			osg::Uniform* pViewLight,
			osg::Uniform* pEngineStartRatio,
			osg::Uniform* pWorld2ECEF,
			osg::Uniform* pView2ECEF,
			osg::Uniform* pWanderProgress);

	private:
		/**
		* @brief ��������תʱ������������������γɵ�����
		* @param fRadius			����뾶
		* @param bPositive			�����Ƿ�����������ٵ�����ת
		* @return Geometry			���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeSpiralGeometry(const float fRadius, const bool bPositive) const;

		/**
		* @brief ��������β��������
		* @param fLength			����β������
		* @param fRadius			�������ͷ���뾶
		* @return Geometry			���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeTailBoxGeometry(const float fLength, const float fRadius) const;
		/**
		* @brief ����β������
		* @param fLength			β������
		* @param fRadius			����ͷ���뾶
		* @return Geometry			���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeTailEnvelopeGeometry(const float fLength, const float fRadius) const;
		/**
		* @brief ����Բ����β��
		* @param fLength			β������
		* @param fRadius			β���뾶
		* @return Geometry			���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeTailCylinderGeometry(const float fLength, const float fRadius) const;
		/**
		* @brief ����������Ƭβ��
		* @param fLength			β������
		* @param fRadius			β���뾶
		* @return Geometry			���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeTailXGeometry(const float fLength, const float fRadius) const;
		/**
		* @brief ɲ��ʱ��������������������γɵĵ���
		* @param fRadius			����뾶
		* @return Geometry			���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeEarthRingGeometry(const float fRadius);

		/**
		* @brief ��ʼ�����˵���β��״̬��
		* @param pSS				��Ҫ���õ�״̬��ָ��
		* @param strShaderName		shader������
		* @return bool				�ɹ�true������false
		*/
		bool _InitEarthTailStateSet(osg::StateSet* pSS, const std::string strShaderName);

		/**
		* @brief ��ȡ���������������
		* @param fCoordUV			�����������UV���꣬��Χ[0.0,1.0]
		* @param fRadius			����뾶
		* @param i					������������Գ�ͼ�Σ�i=0/1�ֱ��������������
		* @param bPositive			�����Ƿ�����������ٵ�����ת
		* @return osg::Vec3			���涥���ģ�Ϳռ��λ��
		*/
		osg::Vec3 _GetSpiralSurfacePos(const osg::Vec2 fCoordUV, const float fRadius, const int i, const bool bPositive) const;
		/**
		* @brief ��ȡβ������ı�������
		* @param fCoordUV			����β����������꣬��Χ[0.0,1.0]
		* @param fLength			β������
		* @param fRadius			����ͷ���뾶
		* @return osg::Vec3			���涥���ģ�Ϳռ��λ��
		*/
		osg::Vec3 _GetTailEnvelopePos(const osg::Vec2 fCoordUV, const float fLength, const float fRadius) const;

		/**
		* @brief �������˵���ƻ��Ľ׶�
		* @param iWanderPeriod
		* 0 = ǰ̫��ʱ��
		* 1 = ɲ��ʱ��
		* 2 = ����ת��ʱ��
		* 3 = ����ת��ʱ��
		* 4 = ����ʱ��
		*/
		void _SetWanderPeriod(const int iWanderPeriod);

	// ����
	private:
		std::string										m_strCoreGalaxyTexPath;			//!< ��ϵ������ͼ·��
		std::string										m_strEarthShaderPath;			//!< Earth shader ·��
		std::string										m_strGalaxyShaderPath;			//!< ��ϵ��ɫ��·��

		osg::ref_ptr<osg::Transform>					m_pSpiralTransform2;			//!< ��2�㼶���������ı任�ڵ�
		osg::ref_ptr<osg::Transform>					m_pTailTransform2;				//!< ��2�㼶���˵���β���ı任�ڵ�

		osg::ref_ptr<osg::Geode>						m_pEarthRingGeode2;				//!< ��2�㼶���򻷵ļ��νڵ�
		osg::ref_ptr<osg::Geode>						m_pSpiralPositiveGeode2;		//!< ��2�㼶���������ļ��νڵ�
		osg::ref_ptr<osg::Geode>						m_pSpiralNegativeGeode2;		//!< ��2�㼶���������ļ��νڵ�
		osg::ref_ptr<osg::Geode>						m_pTailBoxGeode2;				//!< ��2�㼶β�������ļ��νڵ�
		osg::ref_ptr<osg::Geode>						m_pTailEnvelopeGeode2;			//!< ��2�㼶β������ļ��νڵ�

		osg::ref_ptr<osg::StateSet>						m_pSsTailDecFace;				//!< ���˵���β��12������״̬��
		osg::ref_ptr<osg::StateSet>						m_pSsTailDecEdge;				//!< ���˵���β��12�����״̬��
		osg::ref_ptr<osg::StateSet>						m_pSsTailDecVert;				//!< ���˵���β��12�����״̬��

		bool											m_bVisible;						//!< ���˵���β���Ƿ�ɼ�
		osg::ref_ptr<osg::Uniform>						m_fTailVisibleUniform;			//!< ���˵���β���ɼ��ȵ�Uniform
		osg::ref_ptr<osg::Uniform>						m_mWorld2ECEFUniform;			//!< ��2������ռ䡱ת ECEF �ľ���
		osg::ref_ptr<osg::Uniform>						m_mView2ECEFUniform;			//!< view�ռ�תECEF�ľ���
		osg::ref_ptr<osg::Uniform>						m_mWorld2SpiralUniform;			//!< ��2������ռ䡱ת������β���ռ䡱����
		osg::ref_ptr<osg::Uniform>						m_mView2SpiralUniform;			//!< view�ռ�ת������β���ռ䡱����
		osg::ref_ptr<osg::Uniform>						m_vViewLightUniform;			//!< view�ռ�Ĺ�Դ����
		osg::ref_ptr<osg::Uniform>						m_fWanderProgressUniform;		//!< ���˵���ƻ���չUniform
		//!< ����������x=ת��y=�ƽ���z=0��ʾ�����������𲽹رգ�z=1��ʾ�����������𲽿���
		osg::ref_ptr<osg::Uniform>						m_vEngineStartRatioUniform;

		osg::ref_ptr<CEarthTailBoxVisitor>				m_pEarthTailBoxVisitor;			//!< ����β���Ļ����еķ�����
	};

}	// GM