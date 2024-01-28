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

#include "GMCommon.h"
#include "GMKernel.h"

#include <random>
#include <osg/Node>
#include <osg/AutoTransform>
#include <osg/Texture>
#include <osg/Texture2DArray>
#include <osg/Depth>
#include <osg/BlendFunc>
#include <osg/CullFace>
#include <osgUtil/CullVisitor>

namespace GM
{
	/*************************************************************************
	 Enums
	*************************************************************************/
	/*!
	 *  @enum EGMArrowDir
	 *  @brief ��ͷ����ö��
	 */
	enum EGMArrowDir
	{
		EGMAD_NONE,				//!< δָ������
		EGMAD_RADIUS_OUT,		//!< ���� �뾶 �ⷽ��
		EGMAD_RADIUS_IN,		//!< ���� �뾶 �ڷ���
		EGMAD_ANGLE_PLUS,		//!< ���� �Ƕ� ������
		EGMAD_ANGLE_MINUS		//!< ���� �Ƕ� ������
	};

	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/
	class osg::EllipsoidModel;
	class CGMMilkyWay;
	class CGMSolar;
	class CGMDataManager;
	class CGMCommonUniform;

	class CCosmosBox : public osg::Transform
	{
	public:
		CCosmosBox()
		{
			setReferenceFrame(osg::Transform::ABSOLUTE_RF);
			osg::ref_ptr<osg::StateSet> pStateSetCosmosBox = getOrCreateStateSet();
			pStateSetCosmosBox->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			pStateSetCosmosBox->setMode(GL_BLEND, osg::StateAttribute::ON);
			pStateSetCosmosBox->setAttributeAndModes(new osg::BlendFunc(
				GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
			), osg::StateAttribute::ON);
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
	class CGMGalaxy
	{
		// ����
	public:
		/** @brief ���� */
		CGMGalaxy();

		/** @brief ���� */
		~CGMGalaxy();

		/** @brief ��ʼ�� */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData,
			CGMCommonUniform* pCommonUniform, CGMDataManager* pDataManager);
		/** @brief ���� */
		bool Update(double dDeltaTime);
		/** @brief ����(�������������̬֮��) */
		bool UpdateLater(double dDeltaTime);
		/** @brief ���� */
		bool Load();
		/** @brief ���� */
		bool SaveSolarData();

		/**
		* �޸���Ļ�ߴ�ʱ���ô˺���
		* @author LiuTao
		* @since 2022.11.19
		* @param iW: ��Ļ���
		* @param iH: ��Ļ�߶�
		* @return void
		*/
		void ResizeScreen(const int iW, const int iH);

		/** @brief ����/�رձ༭ģʽ */
		void SetEditMode(const bool bEnable);
		/** @brief ��ȡ�༭ģʽ */
		inline bool GetEditMode() const
		{
			return m_bEdit;
		}
		/** @brief ����/�ر�ץ�ֵ�����״̬ */
		void SetHandleHover(const bool bHover);
		/** @brief ����/�رղ�����,�����������Բ���ǰ���ŵ���Ƶ�� */
		void SetCapture(const bool bEnable);

		/**
		* @brief ��������ӭЧ����
		* ÿ�ο��������Engine���ô˺�����ʵ�֡���ӭ���ܡ�
		*/
		void Welcome();

		/**
		* @brief ������ϵ
		* @param fDiameter:	��ϵֱ��,��λ����
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool CreateGalaxy(double fDiameter);

		/**
		* @brief ����ǰ���ռ�㼶���괫��
		* @param vWorldPos:		��ǰ֡���Ŀռ�㼶����
		*/
		void SetMousePosition(const osg::Vec3d& vHierarchyPos);

		/**
		* @brief ���õ�ǰ����ĺ���
		* @param vWorldPos:		��ǰ����ĺ��ǵ�����ռ����꣬��λ����
		* @param wstrName��		��ǰ���ŵ���Ƶ���ƣ�XXX.mp3
		*/
		void SetCurrentStar(const osg::Vec3d& vWorldPos, const std::wstring& wstrName);

		/**
		* @brief ���õ�ǰ���ŵĺ��ǵ���Ƶ�ռ�����
		* @param vAudioCoord:	��ǰ���ŵĺ��ǵ���Ƶ�ռ�����
		*/
		void SetPlayingStarAudioCoord(const SGMAudioCoord& vAudioCoord);

		/**
		* ��ȡ��ǰ���ŵĺ��ǵ���Ƶ�ռ�����
		* @return vAudioCoord:	��ǰ���ŵĺ��ǵ���Ƶ�ռ�����
		*/
		inline SGMAudioCoord GetPlayingStarAudioCoord() const
		{
			return m_vPlayingAudioCoord;
		}

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
		* @brief ��ȡ��ǰ���ŵ���Ƶ������ռ�����
		* @return osg::Vec3d	��ǰ���ŵ���Ƶ������ռ�����
		*/
		inline osg::Vec3d GetStarWorldPos() const
		{
			return m_vPlayingStarWorld4Pos;
		}

		/**
		* @brief ���ڿռ�㼶�仯�����³���
		* @param iHierarchyNew:		���º�Ŀռ�㼶���
		* @return bool:				�ɹ�true��ʧ��false
		*/
		bool UpdateHierarchy(int iHierarchyNew);
		/**
		* @brief ���µ�ǰ����/���������Ϣ����������ı�ǰ���λ�ò�ֵ
		* @param vTargetHiePos:		Ŀ����ڵ�ǰ�ռ��λ��
		* @return SGMVector3:		����ı�ǰ���λ�ò�ֵ����λ����
		*/
		SGMVector3 UpdateCelestialBody(const SGMVector3& vTargetHiePos);
		/**
		* @brief ˢ������ĺ���������ϵ����ϵ��4���ռ����������ϵ����λ��
		*/
		void RefreshNearStarWorldPos();
		/**
		* @brief ��ȡ����ĺ���������ϵ����ϵ��4���ռ����������ϵ����λ��
		* @return osg::Vec3d:		����ĺ���������ϵ����ϵ��4���ռ����������ϵ����λ��
		*/
		inline osg::Vec3d GetNearStarWorldPos() const
		{
			return m_vNearStarWorld4Pos;
		}
		/**
		* @brief ��ȡ����ĺ���������ϵ����ϵ��4���ռ����������ϵ������̬����ת��
		* @return osg::Quat:		��̬��Ԫ��
		*/
		osg::Quat GetNearStarRotate() const;

	private:
		/**
		* _InitBackground
		* ��ʼ��������ؽڵ�
		* @author LiuTao
		* @since 2022.08.07
		* @return void:
		*/
		void _InitBackground();

		/** _CreateStarCube
		* ����PointSprite���ǺУ�����0123���ռ�
		* ��һ���������д�����ģʽ����
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateStarCube();

		/**
		* _CreateHandle
		* ���������ƶ����ǵ�Բ��״�İ���
		* @author LiuTao
		* @since 2022.01.23
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateHandle();

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
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateGalaxies_4();

		/**
		* _CreateGalaxyPlane_5
		* ����ƽ����ϵ������5���ռ�
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateGalaxyPlane_5();

		/**
		* _CreateGalaxies_5
		* ����PointSprite��ϵȺ������5���ռ�
		* ��һ���������д�����ϵȺ��Ȼ��������λ��ƽ��ÿ����ϵ
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateGalaxies_5();

		/**
		* _CreateSupercluster
		* ��������ϵ��
		* ��һ���������д�������ϵ��
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateSupercluster();
		/**
		* _CreateUltracluster
		* ����������ϵ�ţ�Ϲ������ƣ�
		* ��һ���������д���������ϵ��
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateUltracluster();

		/**
		* _CreateMyWorld
		* �����ҵ�����
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateMyWorld();

		/** _CreateBackgroundGalaxy
		* ������������ϵ���νṹ
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateBackgroundGalaxy();

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
		* _UpdatePlayingStarInformation
		* @brief �����������Ƶ�ռ����꣬���µ�ǰ���ŵ���Ƶ����Ϣ
		* @author LiuTao
		* @since 2022.02.16
		* @param sAudioCoord:		��Ƶ�ռ�����
		* @return bool:				�ɹ�true��������Ϸ����򷵻�false
		*/
		bool _UpdatePlayingStarInformation(const SGMAudioCoord& sAudioCoord);

		/**
		* _CreateAudioGeometry
		* ��ȡ���ݲ�������Ƶ�Ǽ�����
		* @author LiuTao
		* @since 2021.07.11
		* @param audioMap��			��������Ƶ����map
		* @param iDiscardUID:		��Ϊ�������Ҫ���޳�����Ƶ��UID��Ĭ��0��ʾû���޳�����Ƶ
		* @return Geometry*:		�����ļ�����ڵ�ָ�룬δ�ɹ��򷵻�nullptr
		*/
		osg::Geometry* _CreateAudioGeometry(
			std::map<unsigned int, SGMAudioData>& audioMap,
			const unsigned int iDiscardUID = 0);

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
		* _CreateSquareGeometry
		* ����������
		* @author LiuTao
		* @since 2022.01.23
		* @param fWidth��			�����α߳�
		* @param bCorner��			���������ĵ��Ƿ��ڽ��ϣ�true == �ڽ��ϣ�false == ������
		* @return Geometry*:		�����ļ�����ڵ�ָ�룬δ�ɹ��򷵻�nullptr
		*/
		osg::Geometry* _CreateSquareGeometry(const float fWidth, const bool bCorner = false);

		/**
		* _CreateRegionGeometry
		* ������Ƶ����������
		* @return Geometry*:		�����ļ�����ڵ�ָ�룬δ�ɹ��򷵻�nullptr
		*/
		osg::Geometry* _CreateRegionGeometry();

		/**
		* @brief ������Բ
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
		osg::Geometry* _MakeEllipsoidGeometry(
			const osg::EllipsoidModel*	ellipsoid,
			int							iLonSegments,
			int							iLatSegments,
			float						fHae,
			bool						bGenTexCoords,
			bool						bWholeMap = false,
			bool						bFlipNormal = false,
			float						fLatStart = -90.0,
			float						fLatEnd = 90.0) const;

		/**
		* @brief ����������
		* @param fLength			����
		* @param fWidth				���
		* @param fHeight			�߶�
		* @return Geometry			���ش����ļ�����ָ��
		*/
		osg::Geometry* _MakeBoxGeometry(
			const float fLength = 10.0f,
			const float fWidth = 10.0f,
			const float fHeight = 2.0f) const;

		/**
		* @brief ��ȡ��άͼƬ��ĳλ�õ����Բ�ֵ���ֵ��repeatģʽ
		* @param fX:		ͼ��x����,������
		* @param fY:		ͼ��y����,������
		* @param fZ:		ͼ��z����,������
		* @return float��	[0.0,1.0]
		*/
		float _Get3DValue(float fX, float fY, float fZ);

		/**
		* @brief ��ȡ��άͼƬ��ĳunsigned intλ�õ�ֵ��ֻ�ܻ�ȡ��noiseShape128.tga�������������ά����ͼ��
		* ����ͼƬΪ128*4096��rgba�ֱ��ʾ4����ά����ֵ���ȱ���ÿһ�㣬�ٱ���ÿ��ͨ��
		* @param iX:		ͼ��x����,[0,127]
		* @param iY:		ͼ��y����,[0,127]
		* @param iZ:		ͼ��z����,[0,127]
		* @return float��	[0.0,1.0]
		*/
		float _Get3DValue(unsigned int iX, unsigned int iY, unsigned int iZ);

		/**
		* @brief ��Ƶ�ռ�����ת��Ƶ����UV
		* @param fX:			ͼ��x����,[0,1]
		* @return osg::Vec2f		����һ������Һ���ĺ�����ɫ��[0.0,1.0]
		*/
		osg::Vec2f _AudioCoord2UV(const SGMAudioCoord& sAudioCoord) const;

		/**
		* ����2D����
		* @author LiuTao
		* @since 2020.06.16
		* @param fileName: ͼƬ�ļ�·��
		* @param iChannelNum: 1��2��3��4�ֱ����R��RG��RGB��RGBA
		* @return osg::Texture* ��������ָ��
		*/
		osg::Texture* _CreateTexture2D(const std::string& fileName, const int iChannelNum) const;
		/**
		* ����2D����
		* @author LiuTao
		* @since 2023.03.04
		* @param pImg:		ͼƬָ��
		* @param iChannelNum: 1��2��3��4�ֱ����R��RG��RGB��RGBA
		* @return osg::Texture* ��������ָ��
		*/
		osg::Texture* _CreateTexture2D(osg::Image* pImg, const int iChannelNum) const;

		/**
		* ����DDS����
		* @author LiuTao
		* @since 2023.01.08
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
		* ����cubeMap�����Զ������ļ����µ�ָ���������������jpgͼƬ
		* @author LiuTao
		* @since 2020.06.16
		* @param strFolder:			ͼƬ�ļ���·��,���� ./XX/
		* @param strFilePrefix:		ͼƬ�ļ�������ǰ׺
		* @return osg::Texture*		����cubeMap����ָ��
		*/
		osg::Texture* _ReadCubeMap(const std::string& strFolder, const std::string& strFilePrefix) const;

		/**
		* _GetRandomStarColor
		* @brief ��ȡһ������Һ���ĺ�����ɫ
		* @author LiuTao
		* @since 2021.10.05
		* @return osg::Vec3f		����һ������Һ���ĺ�����ɫ��[0.0,1.0]
		*/
		osg::Vec3f _GetRandomStarColor();

		/**
		* @brief ��ȡ�����Ƕ�֮��ļн�
		* @author LiuTao
		* @since 2022.02.10
		* @param fA:			�Ƕ�A����λ������ (-PI, PI]
		* @param fB:			�Ƕ�B����λ������ (-PI, PI]
		* @return double:		A - B �ļнǣ���λ������ (-PI, PI]
		*/
		double _IncludedAngle(const double fA, const double fB) const;

		/**
		* @brief ����α�������ͼ
		* @author LiuTao
		* @since 2022.05.15
		* @param void:
		* @return void:
		*/
		void _MakePseudoNoise();

		osg::Texture* _Load3DShapeNoise() const;

		/**
		* @brief SGMVector3 ת osg::Vec3d
		* @param vGM:				�����GM����
		* @return osg::Vec3d:		�����osg����
		*/
		inline osg::Vec3d _GM2OSG(const SGMVector3& vGM) const
		{
			return osg::Vec3d(vGM.x, vGM.y, vGM.z);
		}

		// ����
	private:
		SGMKernelData*									m_pKernelData;					//!< �ں�����
		SGMConfigData*									m_pConfigData;					//!< ��������
		CGMCommonUniform*								m_pCommonUniform;				//!< ����Uniform
		CGMDataManager*									m_pDataManager;					//!< ���ݹ���ģ��
		std::vector<osg::ref_ptr<osg::Group>>			m_pHierarchyRootVector;			//!< ÿ���ռ�㼶�ĸ��ڵ�

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

		std::string										m_strGalaxyShaderPath;			//!< galaxy shader ��ɫ��·��
		std::string										m_strGalaxyTexPath;				//!< galaxy texture ��ͼ·��
		std::string										m_strCoreModelPath;				//!< ����ģ����Դ·��
		std::wstring									m_strPlayingStarName;			//!< �������Ƶ���ļ���,XXX.mp3
		osg::ref_ptr<osg::Transform>					m_pStarInfoTransform;			//!< ���������Ϣչʾ�ı任���
		osg::ref_ptr<osg::Transform>					m_pStar_4_Transform;			//!< ��4�㼶���ֵı任���
		osg::ref_ptr<osg::Transform>					m_pEye_4_Transform;				//!< ��4�㼶�۵�ı任���
		osg::ref_ptr<osg::Transform>					m_pEye_5_Transform;				//!< ��5�㼶�۵�ı任���
		osg::ref_ptr<osg::Transform>					m_pGalaxyGroup_Transform;		//!< ��С��ϵȺ�ı任���
		osg::ref_ptr<osg::Transform>					m_pBackgroundGalaxyTransform;	//!< ��������ϵ�ı任���
		osg::ref_ptr<osg::Transform>					m_pBackgroundStarTransform;		//!< �������ǵı任���
		osg::ref_ptr<osg::AutoTransform>				m_pMyWorld_5_AutoTrans;			//!< 5���ռ��ҵ������Զ��任���
		osg::ref_ptr<osg::AutoTransform>				m_pMyWorld_6_AutoTrans;			//!< 6���ռ��ҵ������Զ��任���
		osg::ref_ptr<osg::Switch>						m_pHandleSwitch;				//!< ���ֵĿ��ؽ��

		unsigned int									m_iPlayingAudioUID;				//!< �������Ƶ�ǵ�UID
		SGMAudioCoord									m_vPlayingAudioCoord;			//!< �������Ƶ�ǵ���Ƶ�ռ�����
		osg::Vec3d										m_vPlayingStarWorld4Pos;		//!< �������Ƶ��4������ռ�����
		osg::Vec3d										m_vNearStarWorld4Pos;			//!< �������4������ռ�λ��
		osg::Vec3d										m_vMouseWorldPos;				//!< �������ռ�����
		osg::Vec3d										m_vMouseLastWorldPos;			//!< �����һ֡����ռ�����
		osg::Matrixd									m_mLastVP;						//!< ��һ֡Raymarch�����ViewProjectionMatrix

		osg::ref_ptr<osg::Geode>						m_pGeodeRegion;					//!< ��Ƶ����Geode	
		osg::ref_ptr<osg::Geode>						m_pGeodeAudio;					//!< δ�������Ƶ��Geode
		osg::ref_ptr<osg::Geode>						m_pGeodePointsN_4;				//!< ��4�㼶N�������ǵ�Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeStarCube_4;				//!< ��4�㼶���Ǻе�Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeGalaxyGroup_4;			//!< ��4�㼶��ϵȺ��Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeGalaxyGroup_5;			//!< ��5�㼶��С��ϵȺ��Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeGalaxies_5;				//!< ��5�㼶������ϵ��Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeSupercluster;			//!< ����ϵ�ŵ�Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeUltracluster;			//!< ������ϵ�ŵ�Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeMyWorld_5;				//!< �ҵ�5�������Geode
		osg::ref_ptr<osg::Geode>						m_pGeodeMyWorld_6;				//!< �ҵ�6�������Geode
		osg::ref_ptr<osg::Geode>						m_pCosmosBoxGeode;				//!< ����е�Geode
		osg::ref_ptr<osg::StateSet>						m_pStateSetGalaxy;				//!< ��ģʽ��ϵ��״̬��
		osg::ref_ptr<osg::StateSet>						m_pStateSetPlayingAudio;		//!< �������Ƶ��״̬��
		osg::ref_ptr<osg::StateSet>						m_pStateSetPlane;				//!< ���������״̬��

		osg::ref_ptr<osg::Uniform>						m_pMousePosUniform;				//!< ��������ռ�����Uniform
		osg::ref_ptr<osg::Uniform>						m_pAudioUVUniform;				//!< ��ǰ��Ƶ�������ڵ�UV
		osg::ref_ptr<osg::Uniform>						m_pGalaxyRadiusUniform;			//!< ��4�㼶��λ����ϵ�뾶Uniform
		osg::ref_ptr<osg::Uniform>						m_pEyePos4Uniform;				//!< ��4�㼶�۵�����Uniform
		osg::ref_ptr<osg::Uniform>						m_fGalaxyHeightUniform;			//!< ��ϵ�ĵ��߸߶�Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarsCubeInfoUniform;		//!< cube���Ǵ������Uniform
		osg::ref_ptr<osg::Uniform>						m_pCubeCenterUniform;			//!< ��3�㼶cube��������ƫ��
		osg::ref_ptr<osg::Uniform>						m_pShapeUVWUniform;				//!< ��ά����UVWϵ����Uniform
		osg::ref_ptr<osg::Uniform>						m_fStarAlphaUniform;			//!< Զ�����ǵ�alpha��Uniform
		osg::ref_ptr<osg::Uniform>						m_fGalaxiesAlphaUniform;		//!< Զ����ϵ��alpha��Uniform
		osg::ref_ptr<osg::Uniform>						m_pGalaxiesInfoUniform;			//!< cube��ϵ�������Uniform
		osg::ref_ptr<osg::Uniform>						m_fStarDistanceUniform;			//!< ����0123�㼶�ĺ��ǵ���Զ����
		osg::ref_ptr<osg::Uniform>						m_fMyWorldAlphaUniform;			//!< �ҵ������alpha��Uniform

		osg::ref_ptr<osgDB::Options>					m_pDDSOptions;					//!< dds���������
		osg::ref_ptr<osg::Texture>						m_pGalaxyColorTex;				//!< ����ϵ��ɫ��alpha����
		osg::ref_ptr<osg::Texture>						m_3DShapeTex;					//!< ��ά�޷�shape������RGBA��128^3
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
				
		float											m_fArrowAngle;					//!< ָʾ��ͷ����ת�Ƕȣ�����
		float											m_fPRPA;						//!< +Radius+Angle���ƽ��ֵ������
		EGMArrowDir										m_eArrowDir;					//!< ��ͷ����ö��
		bool											m_bEdit;						//!< �Ƿ����༭ģʽ
		bool											m_bHandleHover;					//!< ץ���Ƿ�������״̬
		bool											m_bCapture;						//!< �Ƿ�����׽ģʽ
		bool											m_bWelcome;						//!< �Ƿ����û�ӭ����
		std::default_random_engine						m_iRandom;						//!< ���ֵ

		CGMMilkyWay*									m_pMilkyWay;					//!< ����ϵ������Ⱦģ��
		CGMSolar*										m_pSolarSystem;					//!< ̫��ϵģ��
	};
}	// GM