//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMVolumeBasic.h
/// @brief		Galaxy-Music Engine - GMEngine.h
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.10
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "GMCommon.h"
#include "GMKernel.h"
#include <random>

namespace GM
{
	/*************************************************************************
	Macro Defines
	*************************************************************************/
	#define GM_ENGINE					CGMEngine::getSingleton()
	#define GM_ENGINE_PTR				CGMEngine::getSingletonPtr()

	/*************************************************************************
	Enums
	*************************************************************************/

	// ��ǰ��Ƶ����ģʽ
	enum EGMA_MODE
	{
		EGMA_MOD_SINGLE,			// ����ѭ��
		EGMA_MOD_CIRCLE,			// �б�ѭ��
		EGMA_MOD_RANDOM,			// �������
		EGMA_MOD_ORDER				// ˳�򲥷�
	};

	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/
	class CGMDataManager;
	class CGMGalaxy;
	class CGMAudio;

	/*!
	*  @class CGMEngine
	*  @brief Galaxy-Music GMEngine
	*/
	class CGMEngine : public CGMSingleton<CGMEngine>
	{
		// ����
	protected:
		/** @brief ���� */
		CGMEngine();
		/** @brief ���� */
		~CGMEngine();

	public:
		/** @brief ��ȡ���� */
		static CGMEngine& getSingleton(void);
		static CGMEngine* getSingletonPtr(void);

		/** @brief ��ʼ�� */
		bool Init();
		/** @brief �ͷ� */
		void Release();
		/** @brief ���� */
		bool Update();
		/** @brief ���� */
		bool Load();
		/** @brief ���� */
		bool Save();

		/** @brief ����/�رձ༭ģʽ */
		void SetEditMode(const bool bEnable);
		/** @brief ����/�رղ����� */
		void SetCapture(const bool bEnable);

		/**
		* SetAudio
		* @brief ѡ����Ƶ
		* @author LiuTao
		* @since 2021.05.30
		* @param fX, fY: �����λ�õ�����ռ�����
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool SetAudio(const float fX, const float fY);
		/** @brief ���� */
		bool Play();
		/** @brief ��ͣ */
		bool Pause();
		/** @brief ֹͣ */
		bool Stop();
		/** @brief ��һ�� */
		bool Next();

		/**
		* SetMousePosition
		* ����ǰ�������ռ����괫��
		* @author LiuTao
		* @since 2021.07.04
		* @param vPos:			��ǰ֡�������ռ�����
		* @return void
		*/
		void SetMousePosition(const osg::Vec3f vPos);

		/**
		* GetCurrentStarWorldPos
		* ��ȡ��ǰ���ŵ���Ƶ������ռ�����
		* @author LiuTao
		* @since 2021.07.30
		* @param void
		* @return osg::Vec3d	��ǰ���ŵ���Ƶ������ռ�����
		*/
		osg::Vec3d GetCurrentStarWorldPos();

		/** @brief �����ӿڹ����� */
		osgViewer::Viewer* CreateViewer();
		/** @brief ��ȡ�ں����� */
		inline SGMKernelData* GetKernelData() { return m_pKernelData; }
		/** @brief ��ȡ�������� */
		inline SGMConfigData* GetConfigData() { return m_pConfigData; }

		/** @brief ��ȡ��Ƶ���ݹ����� */
		inline CGMDataManager* GetDataManager() { return m_pDataManager; }

	private:
		/**
		* _Next
		* ������һ��
		* @author LiuTao
		* @since 2021.07.23
		* @param void
		* @return void
		*/
		void _Next();

		/**
		* _InnerUpdate
		* �������
		* @author LiuTao
		* @since 2021.07.04
		* @param updateStep ���μ�����µ�ʱ����λs
		* @return void
		*/
		void _InnerUpdate(float updateStep);

		/**
		* _World2GalaxyCoord
		* @brief ����ռ����� ת ���ӿռ�����
		* @author LiuTao
		* @since 2021.06.22
		* @param fX, fY: ����ռ�����
		* @param &fGalaxyX, &fGalaxyY: ���ӿռ�����[-1,1]
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool _World2GalaxyCoord(const float fX, const float fY, float& fGalaxyX, float& fGalaxyY);

		/**
		* _World2GalaxyCoord
		* @brief ���ӿռ����� ת ����ռ�����
		* @author LiuTao
		* @since 2021.07.04
		* @param fGalaxyX, fGalaxyY: ���ӿռ�����[-1,1]
		* @param &fX, &fY: ����ɹ������������ռ�����
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool _GalaxyCoord2World(const float fGalaxyX, const float fGalaxyY, float& fX, float& fY);

		/**
		* _StarCoord2World
		* @brief �ǳ����� ת ����ռ�����
		* @author LiuTao
		* @since 2021.07.04
		* @param starCoord: �ǳ�����
		* @param &fX, &fY: ����ɹ������������ռ�����
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool _StarCoord2World(const SGMStarCoord& starCoord, float& fX, float& fY);

		// ����
	private:

		SGMKernelData*						m_pKernelData;				//!< �ں�����
		SGMConfigData*						m_pConfigData;				//!< ��������
		CGMDataManager*						m_pDataManager;				//!< ���ݹ���

		bool								m_bInit;					//!< ��ʼ����־
		bool								m_bHasWelcomed;				//!< �Ƿ�չʾ����ӭ����
		double								m_dTimeLastFrame;			//!< ��һ֡ʱ��
		float								m_fDeltaStep;				//!< ��λs
		float								m_fConstantStep;			//!< �ȼ�����µ�ʱ��,��λs
		float								m_fGalaxyDiameter;			//!< ��ϵֱ��
		CGMGalaxy*							m_pGalaxy;					//!< ��ϵ
		CGMAudio*							m_pAudio;					//!< ��Ƶ
		EGMA_MODE							m_ePlayMode;				//!< ��ǰ����ģʽ
		std::default_random_engine			m_iRandom;
	};
}	// GM