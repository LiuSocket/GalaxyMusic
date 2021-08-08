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
#include <osg/Texture>

namespace GM
{
	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/

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
		* @param fDiameter:	��ϵֱ��
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool CreateGalaxy(float fDiameter);

		/**
		* SetMousePosition
		* ����ǰ�������ռ����괫��
		* @author LiuTao
		* @since 2021.07.04
		* @param vPos:			��ǰ֡�������ռ�����
		* @return void
		*/
		void SetMousePosition(const osg::Vec3f& vPos);

		/**
		* SetCurrentStar
		* ���õ�ǰ����ĺ���
		* @author LiuTao
		* @since 2021.07.04
		* @param vPos:			��ǰ����ĺ��ǵ�����ռ�����
		* @param wstrName��		��ǰ���ŵ���Ƶ���ƣ�XXX.mp3
		* @return void
		*/
		void SetCurrentStar(const osg::Vec3f& vPos, const std::wstring& wstrName);

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

	private:
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
		* _CreateGalaxyPlane
		* ����ƽ����ϵ������Զ��
		* @author LiuTao
		* @since 2020.12.05
		* @return bool:			�ɹ�true��ʧ��false
		*/
		bool _CreateGalaxyPlane();

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
		* _GetGalaxyValue
		* ��ȡ��ϵͼ��RGBAͨ��ֵ��channelΪ0/1/2/3����ӦRGBA��
		* @author LiuTao
		* @since 2020.11.27
		* @param fX:		ͼ��x����,[0,1]
		* @param fY:		ͼ��y����,[0,1]
		* @param iChannel:	0��1��2��3����ӦR��G��B��Aͨ��
		* @param bLinear:	�Ƿ�˫���Բ�ֵ��true = ˫���ԣ�false = �ٽ�ֵ
		* @return float [0.0,1.0]
		*/
		float _GetGalaxyValue(float fX, float fY, int iChannel, bool bLinear = false);

		/**
		* ����2D����
		* @author LiuTao
		* @since 2020.06.16
		* @param fileName: ͼƬ�ļ�·��
		* @param iChannelNum: 1��2��3��4�ֱ����R��RG��RGB��RGBA
		* @return osg::Texture* ��������ָ��
		*/
		osg::Texture* _CreateTexture2D(const std::string& fileName, const int iChannelNum);

		// ����
	private:

		float											m_fGalaxyRadius;				//!< ��ϵ�뾶
		/** ��ϵ����ͼƬ��
		** RGBͨ��		��ɫ
		** Alphaͨ��	�ܶ�
		*/
		osg::ref_ptr<osg::Image>						m_pGalaxyImage;
		std::default_random_engine						m_iRandom;

		std::string										m_strGalaxyShaderPath;			//!< galaxy shader ��ɫ��·��
		std::string										m_strGalaxyTexPath;				//!< galaxy texture ��ͼ·��
		std::wstring									m_strPlayingStarName;			//!< �������Ƶ���ļ���,XXX.mp3
		osg::ref_ptr<osg::Transform>					m_pPlayingStarTransform;		//!< �������Ƶ�ǵı任���
		SGMAudioCoord									m_vPlayingAudioCoord;			//!< �������Ƶ�ǵ���Ƶ�ռ�����
		osg::Vec3d										m_vPlayingStarWorldPos;			//!< �������Ƶ������ռ�����
		osg::ref_ptr<osg::Geode>						m_pGeodeHelpLine;				//!< �����޸���Ƶ�������Geode	
		osg::ref_ptr<osg::Geode>						m_pGeodeAudio;					//!< δ�������Ƶ��Geode					
		osg::ref_ptr<osg::StateSet>						m_pStateSetGalaxy;				//!< ��ģʽ��ϵ��״̬��
		osg::ref_ptr<osg::StateSet>						m_pStateSetPlayingAudio;		//!< �������Ƶ��״̬��
		osg::ref_ptr<osg::StateSet>						m_pStateSetPlane;				//!< ���������״̬��
		osg::ref_ptr<osg::Uniform>						m_pMousePosUniform;				//!< ��������ռ�����Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarWorldPosUniform;			//!< �������Ƶ����������Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarAudioPosUniform;			//!< ��ǰ�������Ƶ�ռ�����Uniform
		osg::ref_ptr<osg::Uniform>						m_pTimesUniform;				//!< ʱ��Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarColorUniform;			//!< ��ǰ��ɫUniform
		osg::ref_ptr<osg::Uniform>						m_pLevelArrayUniform;			//!< �������Uniform
		bool											m_bCapture;						//!< �Ƿ�����׽ģʽ
		bool											m_bWelcome;						//!< �Ƿ����û�ӭ����
		double											m_fWelcomeTime;					//!< ����ӭ״̬�������˶೤ʱ��
	};
}	// GM