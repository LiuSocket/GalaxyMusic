//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMGalaxy.h
/// @brief		Galaxy-Music Engine - GMGalaxy
/// @version	1.0
/// @author		LiuTao
/// @date		2020.11.27
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMNebula.h"
#include <random>
#include <osg/Node>
#include <osg/AutoTransform>
#include <osg/Texture>
#include <osg/Texture2DArray>
#include <osg/Depth>
#include <osgUtil/CullVisitor>
#include <osg/CullFace>
#include <osg/BlendEquation>

namespace GM
{
	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/
	class osg::EllipsoidModel;

	class CCosmosBox : public osg::Transform
	{
	public:
		CCosmosBox()
		{
			setReferenceFrame(osg::Transform::ABSOLUTE_RF);
			osg::ref_ptr<osg::StateSet> pStateSetCosmosBox = getOrCreateStateSet();
			pStateSetCosmosBox->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			pStateSetCosmosBox->setMode(GL_BLEND, osg::StateAttribute::ON);
			osg::ref_ptr<osg::BlendEquation> blendEqua = new osg::BlendEquation(osg::BlendEquation::RGBA_MAX);
			pStateSetCosmosBox->setAttributeAndModes(blendEqua.get(), osg::StateAttribute::ON);
			pStateSetCosmosBox->setAttributeAndModes(new osg::CullFace());
			pStateSetCosmosBox->setAttributeAndModes(new osg::Depth(osg::Depth::GEQUAL, 1.0, 1.0));
		}

		CCosmosBox(const CCosmosBox& copy, osg::CopyOp copyop =
			osg::CopyOp::SHALLOW_COPY)
			: osg::Transform(copy, copyop) {}
		META_Node(osg, CCosmosBox);

		virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const
		{
			if (nv && nv->getVisitorType() ==
				osg::NodeVisitor::CULL_VISITOR)
			{
				osgUtil::CullVisitor* cv =
					static_cast<osgUtil::CullVisitor*>(nv);
				matrix.preMult(osg::Matrix::translate(cv->getEyeLocal()));
				return true;
			}
			else
				return osg::Transform::computeLocalToWorldMatrix(matrix, nv);
		}

		virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix,
			osg::NodeVisitor* nv) const
		{
			if (nv && nv->getVisitorType() ==
				osg::NodeVisitor::CULL_VISITOR)
			{
				osgUtil::CullVisitor* cv =
					static_cast<osgUtil::CullVisitor*>(nv);
				matrix.postMult(osg::Matrix::translate(
					-cv->getEyeLocal()));
				return true;
			}
			else
				return osg::Transform::computeWorldToLocalMatrix(matrix, nv);
		}

	protected:
		virtual ~CCosmosBox() {}
	};

	/*!
	*  @class CGMGalaxy
	*  @brief Galaxy-Music GMGalaxy
	*/
	class CGMGalaxy : public CGMNebula
	{
		// ����
	public:
		/** @brief ���� */
		CGMGalaxy();

		/** @brief ���� */
		~CGMGalaxy();

		/** @brief ��ʼ�� */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData);
		/** @brief ���� */
		bool Update(double dDeltaTime);
		/** @brief ����(�������������̬֮��) */
		bool UpdateLater(double dDeltaTime);
		/** @brief ���� */
		bool Load();

		/** @brief ����/�رձ༭ģʽ */
		void SetEditMode(const bool bEnable);
		/** @brief ��ȡ�༭ģʽ */
		bool GetEditMode();
		/** @brief ����/�رղ����� */
		void SetCapture(const bool bEnable);

		/**
		* Welcome
		* @brief ��������ӭЧ����
		* @brief ÿ�ο��������Engine���ô˺�����ʵ�֡���ӭ���ܡ�
		* @author LiuTao
		* @since 2021.07.24
		* @param void
		* @return void
		*/
		void Welcome();

		/**
		* CreateGalaxy
		* ������ϵ
		* @author LiuTao
		* @since 2020.11.27
		* @param fDiameter:	��ϵֱ��,��λ����
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool CreateGalaxy(double fDiameter);

		/**
		* SetMousePosition
		* ����ǰ���ռ�㼶���괫��
		* @author LiuTao
		* @since 2021.07.04
		* @param vWorldPos:		��ǰ֡���Ŀռ�㼶����
		* @return void
		*/
		void SetMousePosition(const osg::Vec3d& vHierarchyPos);

		/**
		* SetCurrentStar
		* ���õ�ǰ����ĺ���
		* @author LiuTao
		* @since 2021.07.04
		* @param vWorldPos:		��ǰ����ĺ��ǵ�����ռ����꣬��λ����
		* @param wstrName��		��ǰ���ŵ���Ƶ���ƣ�XXX.mp3
		* @return void
		*/
		void SetCurrentStar(const osg::Vec3d& vWorldPos, const std::wstring& wstrName);

		/**
		* SetAudioLevel
		* ���õ�ǰ֡��Ƶ�����ֵ
		* @author LiuTao
		* @since 2021.07.04
		* @param fLevel ���ֵ [0.0f,1.0f]
		* @return void
		*/
		void SetAudioLevel(float fLevel);

		/**
		* GetStarWorldPos
		* ��ȡ��ǰ���ŵ���Ƶ������ռ�����
		* @author LiuTao
		* @since 2021.07.30
		* @param void
		* @return osg::Vec3d	��ǰ���ŵ���Ƶ������ռ�����
		*/
		osg::Vec3d GetStarWorldPos();

		/**
		* UpdateHierarchy
		* @brief ���ڿռ�㼶�仯�����³���
		* @author LiuTao
		* @since 2021.09.15
		* @param iHierarchy:		�ռ�㼶���
		* @return bool:				�ɹ�true��ʧ��false
		*/
		bool UpdateHierarchy(int iHierarchy);

	private:

		/**
		* _CreateSupernova
		* ���������ǣ�һ�ŷǳ������ĺ���
		* �������κβ㼶��������Ƶʱ,��12345�㼶�ռ䣬ʼ�տɼ�
		* @author LiuTao
		* @since 2021.10.06
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateSupernova();

		/**
		* _CreateStarCube
		* ����PointSprite���ǺУ�����0123���ռ�
		* ��һ���������д�����ģʽ����
		* @author LiuTao
		* @since 2021.10.01
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateStarCube();

		/**
		* _CreateSun
		* ����1��2�㼶��̫��
		* @author LiuTao
		* @since 2022.01.02
		* @param void��			��
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateSun();

		/**
		* _CreatePlanets
		* ����2��3�㼶������,Χ�ƺ�����ת
		* @author LiuTao
		* @since 2021.11.28
		* @param void��			��
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreatePlanets();

		/**
		* _AddPlanet
		* ������ǣ�Χ�ƺ�����ת
		* @author LiuTao
		* @since 2022.01.01
		* @param fRadius��			��ת�뾶����λ����
		* @param fOrbitalPeriod:	��ת���ڣ���λ����
		* @param fStartPos:			��ת��ʼλ�ã���λ������
		* @return bool:				�ɹ�true��ʧ��false
		*/
		bool _AddPlanet(
			const double fRadius,
			const double fOrbitalPeriod,
			const double fStartPos);

		/**
		* _CreateOortCloud
		* �����¶����ƣ�����3���ռ�
		* һ�����棬�뾶2����
		* @author LiuTao
		* @since 2021.10.12
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateOortCloud();

		/**
		* _CreateAudioPoints
		* ������Ƶ�ǵ�
		* @author LiuTao
		* @since 2020.12.05
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateAudioPoints();

		/**
		* _CreateGalaxyPoints
		* ����PointSprite��ϵ
		* @author LiuTao
		* @since 2020.12.05
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateGalaxyPoints();

		/**
		* _CreateStarCube_4
		* ����PointSprite���ǺУ�����4���ռ�
		* ��һ���������д������ǣ�Ȼ��������λ��ƽ��ÿ������
		* @author LiuTao
		* @since 2021.10.01
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateStarCube_4();

		/**
		* _CreateGalaxyPointsN_4
		* ����N���ܶȵ�PointSprite��ϵ������4���ռ�
		* ���磺����2���ܶȣ����ǵķ�Χֱ������С����ϵ��1/2,���߶ȷ�Χ����
		* @author LiuTao
		* @since 2021.09.26
		* @param iDens��		���ǵ��ܶȱ�����ֻ����2��4��8��16����
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateGalaxyPointsN_4(int iDens);

		/**
		* _CreateGalaxyPlane_4
		* ����ƽ����ϵ������4���ռ�
		* @author LiuTao
		* @since 2020.12.05
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateGalaxyPlane_4();

		/**
		* _CreateGalaxies_4
		* ����PointSprite��ϵȺ������4���ռ�,�͵�5���ռ����С��ϵȺ�޷��ν�
		* �����ںϣ�ʵ��Ǭ����Ų��
		* ��һ�������ϴ�����ϵȺ����������ʼ�������λ��
		* ÿ����ϵ����5���ռ����ϵ��ȫ�Ǻ�
		* @author LiuTao
		* @since 2021.10.06
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateGalaxies_4();

		/**
		* _CreateGalaxyPlane_5
		* ����ƽ����ϵ������5���ռ�
		* @author LiuTao
		* @since 2021.09.17
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateGalaxyPlane_5();

		/**
		* _CreateGalaxies_5
		* ����PointSprite��ϵȺ������5���ռ�
		* ��һ���������д�����ϵȺ��Ȼ��������λ��ƽ��ÿ����ϵ
		* @author LiuTao
		* @since 2021.10.06
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateGalaxies_5();

		/**
		* _CreateSupercluster
		* ��������ϵ��
		* ��һ���������д�������ϵ��
		* @author LiuTao
		* @since 2021.10.16
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateSupercluster();
		/**
		* _CreateUltracluster
		* ����������ϵ�ţ�Ϲ������ƣ�
		* ��һ���������д���������ϵ��
		* @author LiuTao
		* @since 2021.10.16
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateUltracluster();

		/**
		* _CreateMyWorld
		* �����ҵ�����
		* @author LiuTao
		* @since 2021.10.16
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateMyWorld();

		/**
		* _CreateGalaxyBackground
		* ������������ϵ����
		* @author LiuTao
		* @since 2021.10.08
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateGalaxyBackground();

		/**
		* _CreateCosmosBox
		* ������������У����ڱ���
		* @author LiuTao
		* @since 2021.08.22
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateCosmosBox();

		/**
		* _DetachAudioPoints
		* ���������Ƶ�ǵ����
		* @author LiuTao
		* @since 2021.07.11
		* @return bool:				�ɹ�true��ʧ��false
		*/
		bool _DetachAudioPoints();

		/**
		* _AttachAudioPoints
		* �ϲ���ǰ�༭����Ƶ�ǵ�
		* @author LiuTao
		* @since 2021.07.11
		* @return bool:				�ɹ�true��ʧ��false
		*/
		bool _AttachAudioPoints();

		/**
		* _CreateAudioGeometry
		* ��ȡ���ݲ�����δ����״̬����Ƶ�Ǽ�����
		* @author LiuTao
		* @since 2021.07.11
		* @param coordVector��		��Ƶ�ǵ��ǳ�����Vector
		* @return Geometry*:		�����ļ�����ڵ�ָ�룬δ�ɹ��򷵻�nullptr
		*/
		osg::Geometry* _CreateAudioGeometry(std::vector<SGMStarCoord>& coordVector);

		/**
		* _CreateConeGeometry
		* ����Բ׶��
		* @author LiuTao
		* @since 2021.07.18
		* @param void��				��
		* @return Geometry*:		�����ļ�����ڵ�ָ�룬δ�ɹ��򷵻�nullptr
		*/
		osg::Geometry* _CreateConeGeometry();

		/**
		* _CreateHelpLineGeometry
		* ����������,�����޸���Ƶ����
		* @author LiuTao
		* @since 2021.07.24
		* @param void��				��
		* @return Geometry*:		�����ļ�����ڵ�ָ�룬δ�ɹ��򷵻�nullptr
		*/
		osg::Geometry* _CreateHelpLineGeometry();

		/**
		* _MakeEllipsoidGeometry
		* @author LiuTao
		* @since 2020.06.22
		* @param ellipsoid				�����������������ģ��
		* @param iLonSegments			���ȷֶ���
		* @param iLatSegments			γ�ȷֶ���
		* @param fHae					���Ӱ뾶�������ں��θ߶ȣ�
		* @param bGenTexCoords			�Ƿ�����UV����
		* @param bWholeMap				�Ƿ���ʾ������ͼ(�ڰ�����߸�������ʱ)
		* @param bFlipNormal			true �������ڣ�false ��������
		* @param fLonStart				���ȿ�ʼλ�ã���λ���Ƕ� ��
		* @param fLonEnd				���Ƚ���λ�ã���λ���Ƕ� ��
		* @param fLatStart				γ�ȿ�ʼλ�ã���λ���Ƕ� ��
		* @param fLatEnd				γ�Ƚ���λ�ã���λ���Ƕ� ��
		* @return Geometry				���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeEllipsoidGeometry(
			const osg::EllipsoidModel*	ellipsoid,
			int							iLonSegments,
			int							iLatSegments,
			float						fHae,
			bool						bGenTexCoords,
			bool						bWholeMap = false,
			bool						bFlipNormal = false,
			float						fLonStart = -180.0,
			float						fLonEnd = 180.0,
			float						fLatStart = -90.0,
			float						fLatEnd = 90.0);

		/**
		* _GetGalaxyValue
		* ��ȡ��ϵͼ��RGBAͨ��ֵ��channelΪ0/1/2/3����ӦRGBA��
		* @author LiuTao
		* @since 2020.11.27
		* @param fX:		ͼ��x����,[0,1]
		* @param fY:		ͼ��y����,[0,1]
		* @param iChannel:	0��1��2��3����ӦR��G��B��Aͨ��
		* @param bLinear:	�Ƿ�˫���Բ�ֵ��true = ˫���ԣ�false = �ٽ�ֵ
		* @return float��	[0.0,1.0]
		*/
		float _GetGalaxyValue(float fX, float fY, int iChannel, bool bLinear = false);

		/**
		* _GetGalaxyHeight
		* ��ȡ��ϵ�ĺ����Ϣ
		* @author LiuTao
		* @since 2020.11.27
		* @param fX:		ͼ��x����,[0,1]
		* @param fY:		ͼ��y����,[0,1]
		* @return float��	[0.0,1.0]
		*/
		float _GetGalaxyHeight(float fX, float fY);

		/**
		* _Get3DValue
		* ��ȡ��άͼƬ��ĳλ�õ����Բ�ֵ���ֵ��repeatģʽ
		* @author LiuTao
		* @since 2021.09.26
		* @param fX:		ͼ��x����,������
		* @param fY:		ͼ��y����,������
		* @param fZ:		ͼ��z����,������
		* @return float��	[0.0,1.0]
		*/
		float _Get3DValue(float fX, float fY, float fZ);

		/**
		* _Get3DValue
		* ��ȡ��άͼƬ��ĳunsigned intλ�õ�ֵ��ֻ�ܻ�ȡ��noiseShape128.tga�������������ά����ͼ��
		* ����ͼƬΪ128*4096��rgba�ֱ��ʾ4����ά����ֵ���ȱ���ÿһ�㣬�ٱ���ÿ��ͨ��
		* @author LiuTao
		* @since 2021.09.26
		* @param iX:		ͼ��x����,[0,127]
		* @param iY:		ͼ��y����,[0,127]
		* @param iZ:		ͼ��z����,[0,127]
		* @return float��	[0.0,1.0]
		*/
		float _Get3DValue(unsigned int iX, unsigned int iY, unsigned int iZ);

		/**
		* _GetPhotoColor
		* ��ȡ��Ƭ����ɫ������ֵ
		* @author LiuTao
		* @since 2021.10.16
		* @param fX:			ͼ��x����,[0,1]
		* @param fY:			ͼ��y����,[0,1]
		* @return osg::Vec4f��	Vec3[0.0,1.0]
		*/
		osg::Vec4f _GetPhotoColor(float fX, float fY);

		/**
		* ����2D����
		* @author LiuTao
		* @since 2020.06.16
		* @param fileName: ͼƬ�ļ�·��
		* @param iChannelNum: 1��2��3��4�ֱ����R��RG��RGB��RGBA
		* @return osg::Texture* ��������ָ��
		*/
		osg::Texture* _CreateTexture2D(const std::string& fileName, const int iChannelNum);

		/**
		* ����cubeMap�����Զ������ļ����µ�ָ���������������jpgͼƬ
		* @author LiuTao
		* @since 2020.06.16
		* @param strFolder:			ͼƬ�ļ���·��,���� ./XX/
		* @param strFilePrefix:		ͼƬ�ļ�������ǰ׺
		* @return osg::Texture*		����cubeMap����ָ��
		*/
		osg::Texture* _ReadCubeMap(const std::string& strFolder, const std::string& strFilePrefix);

		/**
		* _getRandomStarColor
		* @brief ��ȡһ������Һ���ĺ�����ɫ
		* @author LiuTao
		* @since 2021.10.05
		* @return osg::Vec3f		����һ������Һ���ĺ�����ɫ��[0.0,1.0]
		*/
		osg::Vec3f _getRandomStarColor();

		// ����
	private:
		//!< ��ϵ�뾶����λ����
		double											m_fGalaxyRadius;
		/** ��ϵ���ͼƬ��
		** Rͨ��		/
		** Gͨ��		/
		** Bͨ��		������
		** Alphaͨ��	/
		*/
		osg::ref_ptr<osg::Image>						m_pGalaxyHeightImage;
		/** ��ϵ����ͼƬ��
		** RGBͨ��		��ɫ
		** Alphaͨ��	�ܶ�
		*/
		osg::ref_ptr<osg::Image>						m_pGalaxyImage;
		/** ��ά����ͼƬ��
		** RGBAͨ��		�����Ҷ�
		** �������ά�������ö�άͼƬ��4��ͨ���洢
		*/
		osg::ref_ptr<osg::Image>						m_shapeImg;
		/** ��Ƭ��ͼƬ��
		** RGBͨ��		��ɫ
		** Alphaͨ��	��
		*/
		osg::ref_ptr<osg::Image>						m_pPhotoImage;

		std::default_random_engine						m_iRandom;

		std::string										m_strGalaxyShaderPath;			//!< galaxy shader ��ɫ��·��
		std::string										m_strGalaxyTexPath;				//!< galaxy texture ��ͼ·��
		std::wstring									m_strPlayingStarName;			//!< �������Ƶ���ļ���,XXX.mp3
		osg::ref_ptr<osg::Transform>					m_pPlayingStarTransform;		//!< �������Ƶ�ǵı任���
		osg::ref_ptr<osg::Transform>					m_pStar_2_Transform;			//!< ��2�㼶���ĺ��ǡ����Ǳ任���
		osg::ref_ptr<osg::Transform>					m_pStar_3_Transform;			//!< ��3�㼶���ĺ��ǡ��¶����Ʊ任���
		osg::ref_ptr<osg::Transform>					m_pOort_4_Transform;			//!< ��4�㼶�¶����Ʊ任���
		osg::ref_ptr<osg::Transform>					m_pEye_4_Transform;				//!< ��4�㼶�۵�ı任���
		osg::ref_ptr<osg::Transform>					m_pEye_5_Transform;				//!< ��5�㼶�۵�ı任���
		osg::ref_ptr<osg::Transform>					m_pGalaxyGroup_Transform;		//!< ��С��ϵȺ�ı任���
		osg::ref_ptr<osg::AutoTransform>				m_pSunBloomTransform;			//!< ̫���Թ�ı任���
		osg::ref_ptr<osg::AutoTransform>				m_pMyWorld_5_AutoTrans;			//!< 5���ռ��ҵ������Զ��任���
		osg::ref_ptr<osg::AutoTransform>				m_pMyWorld_6_AutoTrans;			//!< 6���ռ��ҵ������Զ��任���
		osg::ref_ptr<osg::AutoTransform>				m_pSupernovaTransform;			//!< �����ǵı任���
		SGMAudioCoord									m_vPlayingAudioCoord;			//!< �������Ƶ�ǵ���Ƶ�ռ�����
		osg::Vec3d										m_vPlayingStarWorldPos;			//!< �������Ƶ������ռ�����
		osg::ref_ptr<osg::Geode>						m_pGeodeHelpLine;				//!< �����޸���Ƶ�������Geode	
		osg::ref_ptr<osg::Geode>						m_pGeodeAudio;					//!< δ�������Ƶ��Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeSun_2;					//!< ��2�㼶̫��Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePlanets_2;				//!< ��2�㼶���ǵ㾫��Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePlanetsLine_2;			//!< ��2�㼶���ǹ����Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePlanets_3;				//!< ��3�㼶���ǵ㾫��Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePlanetsLine_3;			//!< ��3�㼶���ǹ����Geode
		osg::ref_ptr<osg::Geode>						m_pOortCloudGeode_3;			//!< ��3�㼶�¶�����Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeStarCube;				//!< ��3�㼶���Ǻе�Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePointsN_4;				//!< ��4�㼶N�������ǵ�Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeStarCube_4;				//!< ��4�㼶���Ǻе�Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeGalaxyGroup_4;			//!< ��4�㼶��ϵȺ��Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeGalaxyGroup_5;			//!< ��5�㼶��С��ϵȺ��Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeGalaxies_5;				//!< ��5�㼶������ϵ��Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeSupercluster;			//!< ����ϵ�ŵ�Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeUltracluster;			//!< ������ϵ�ŵ�Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeMyWorld_5;				//!< �ҵ�5�������Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeMyWorld_6;				//!< �ҵ�6�������Geode
		osg::ref_ptr<osg::Geode>						m_pGalaxyBackgroundGeode;		//!< ����ϵ������Geode
		osg::ref_ptr<osg::Geode>						m_pCosmosBoxGeode;				//!< ����е�Geode
		osg::ref_ptr<osg::StateSet>						m_pStateSetGalaxy;				//!< ��ģʽ��ϵ��״̬��
		osg::ref_ptr<osg::StateSet>						m_pStateSetPlayingAudio;		//!< �������Ƶ��״̬��
		osg::ref_ptr<osg::StateSet>						m_pStateSetPlane;				//!< ���������״̬��	
		osg::ref_ptr<osg::Uniform>						m_pMousePosUniform;				//!< ��������ռ�����Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarHiePosUniform;			//!< �������Ƶ�ǵĵ�ǰ�ռ�����Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarAudioPosUniform;			//!< ��ǰ�������Ƶ�ռ�����Uniform
		osg::ref_ptr<osg::Uniform>						m_pTimesUniform;				//!< ʱ��Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarColorUniform;			//!< ��ǰ��ɫUniform
		osg::ref_ptr<osg::Uniform>						m_pLevelArrayUniform;			//!< �������Uniform
		osg::ref_ptr<osg::Uniform>						m_pGalaxyRadiusUniform;			//!< ��4�㼶��λ����ϵ�뾶Uniform
		osg::ref_ptr<osg::Uniform>						m_pEyePos4Uniform;				//!< ��4�㼶�۵�����Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarsCubeInfoUniform;		//!< cube���Ǵ������Uniform
		osg::ref_ptr<osg::Uniform>						m_pCubeCenterUniform;			//!< 3�㼶cube������ƫ��Uniform
		osg::ref_ptr<osg::Uniform>						m_pShapeUVWUniform;				//!< ��ά����UVWϵ����Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarAlphaUniform;			//!< Զ�����ǵ�alpha��Uniform
		osg::ref_ptr<osg::Uniform>						m_pGalaxiesAlphaUniform;		//!< Զ����ϵ��alpha��Uniform
		osg::ref_ptr<osg::Uniform>						m_pGalaxiesInfoUniform;			//!< cube��ϵ�������Uniform
		osg::ref_ptr<osg::Uniform>						m_pSupernovaLightUniform;		//!< ���Ƴ����ǵ����ȵ�Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarDistanceUniform;			//!< ����0123�㼶�ĺ��ǵ���Զ����Uniform
		osg::ref_ptr<osg::Uniform>						m_pUnitRatioUniform;			//!< ��ǰ�㼶���3�㼶�ĵ�λ����Uniform
		osg::ref_ptr<osg::Uniform>						m_pMyWorldAlphaUniform;			//!< �ҵ������alpha��Uniform
		osg::ref_ptr<osg::Texture>						m_pGalaxyColorTex;				//!< ����ϵ��ɫ��alpha����
		osg::ref_ptr<osg::Texture2DArray>				m_pGalaxiesTex;					//!< ��ϵ��������
		osg::ref_ptr<osg::Vec4Array>					m_pSphereVertArray;				//!< sphere��ģʽ���Ƕ���
		osg::ref_ptr<osg::Vec4Array>					m_pSphereColorArray;			//!< sphere��ģʽ������ɫ
		osg::ref_ptr<osg::DrawElementsUShort>			m_pSphereElement;				//!< sphere��ģʽ����Ԫ��
		osg::ref_ptr<osg::Vec4Array>					m_pCubeVertArray;				//!< cube��ģʽ���Ƕ���
		osg::ref_ptr<osg::Vec4Array>					m_pCubeColorArray;				//!< cube��ģʽ������ɫ
		osg::ref_ptr<osg::DrawElementsUShort>			m_pCubeElement;					//!< cube��ģʽ����Ԫ��
		std::vector<osg::ref_ptr<osg::Geometry>>		m_pStarsGeomVector;				//!< cube��ģʽ���Ǽ���������
		osg::ref_ptr<CCosmosBox>						m_pCosmosBoxNode;				//!< ����н�㣬�вü��ص�
		osg::ref_ptr<osg::Vec4Array>					m_pGalaxiesInfo;				//!< ��ϵ��תö��(0123)��������
		std::vector<osg::ref_ptr<osg::Geometry>>		m_pGalaxiesGeomVector;			//!< ��ϵ����������
		osg::ref_ptr<osg::Vec3Array>					m_pPlanetLineVerts_2;			//!< ��2�㼶���ǹ켣�߶�������
		osg::ref_ptr<osg::Vec3Array>					m_pPlanetLineCoords_2;			//!< ��2�㼶���ǹ켣��UVW����
		osg::ref_ptr<osg::DrawElementsUShort>			m_pPlanetLineElement_2;			//!< ��2�㼶���ǹ켣��Ԫ��
		osg::ref_ptr<osg::Vec3Array>					m_pPlanetLineVerts_3;			//!< ��3�㼶���ǹ켣�߶�������
		osg::ref_ptr<osg::Vec3Array>					m_pPlanetLineCoords_3;			//!< ��3�㼶���ǹ켣��UVW����
		osg::ref_ptr<osg::DrawElementsUShort>			m_pPlanetLineElement_3;			//!< ��3�㼶���ǹ켣��Ԫ��
		osg::ref_ptr<osg::Vec4Array>					m_pPlanetVertArray_2;			//!< ��2�㼶���Ƕ�������
		osg::ref_ptr<osg::DrawElementsUShort>			m_pPlanetElement_2;				//!< ��2�㼶����Ԫ��
		osg::ref_ptr<osg::Vec4Array>					m_pPlanetVertArray_3;			//!< ��3�㼶���Ƕ�������
		osg::ref_ptr<osg::DrawElementsUShort>			m_pPlanetElement_3;				//!< ��3�㼶����Ԫ��

		int												m_iPlanetCount;					//!< ��������������

		bool											m_bEdit;						//!< �Ƿ����༭ģʽ
		bool											m_bCapture;						//!< �Ƿ�����׽ģʽ
		bool											m_bWelcome;						//!< �Ƿ����û�ӭ����
	};
}	// GM