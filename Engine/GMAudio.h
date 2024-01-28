//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMAudio.h
/// @brief		Galaxy-Music Engine - GMAudio
/// @version	1.0
/// @author		LiuTao
/// @date		2021.05.29
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommon.h"
#include "bass.h"
#include <osg/Vec2f>
namespace GM
{
	/*************************************************************************
	Enums
	*************************************************************************/

	// ��Ƶ��������ö��
	enum EGMA_COMMAND
	{
		EGMA_CMD_OPEN,				// ����Ƶ
		EGMA_CMD_PLAY,				// ������Ƶ
		EGMA_CMD_CLOSE,				// �ر���Ƶ
		EGMA_CMD_PAUSE,				// ��ͣ��Ƶ
		EGMA_CMD_STOP				// ֹͣ��Ƶ
	};

	// ��ǰ����״̬
	enum EGMA_STATE
	{
		EGMA_STA_MUTE,				// �Ѿ���
		EGMA_STA_PLAY,				// ���ڲ���
		EGMA_STA_PAUSE,				// ����ͣ
	};

	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/
	class CGMDataManager;

	/*!
	*  @class CGMAudio
	*  @brief Galaxy-Music CGMAudio
	*/
	class CGMAudio
	{
		// ����
	public:
		/** @brief ���� */
		CGMAudio();

		/** @brief ���� */
		~CGMAudio();

		/** @brief ��ʼ�� */
		bool Init(SGMConfigData* pConfigData);
		/** @brief ���� */
		bool Update(double dDeltaTime);

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
		* IsWelcomeFinished
		* @brief ����ӭЧ�����Ƿ����
		* @author LiuTao
		* @since 2021.09.11
		* @param void
		* @return bool �����򷵻�true������false
		*/
		bool IsWelcomeFinished();

		/**
		* SetCurrentAudio
		* �����ļ��������õ�ǰ��Ƶ
		* @author LiuTao
		* @since 2021.05.29
		* @param strAudioFile:	��ǰ��Ƶ�ļ����ƣ����磺xxx.mp3
		* @return bool ���óɹ�����true��ʧ�ܻ����ļ�δ�ҵ�����false
		*/
		bool SetCurrentAudio(std::wstring& strAudioFile);

		/**
		* GetCurrentAudio
		* ��ȡ��ǰ��Ƶ�ļ�����
		* @author LiuTao
		* @since 2021.07.24
		* @param void	
		* @return std::wstring ��ǰ��Ƶ�ļ����ƣ����磺xxx.mp3,����L""
		*/
		inline std::wstring GetCurrentAudio()
		{
			return m_strCurrentFile;
		}

		/**
		* @brief ��Ƶ���ſ���
		* @param command:	ָ������ö��
		*/
		void AudioControl(EGMA_COMMAND command);

		/**
		* @brief �ж���Ƶ�Ƿ񲥷����
		* @return bool ��Ϸ���true��δ��Ϸ���false
		*/
		bool IsAudioOver();

		/**
		* @brief ����������ע�⣬��˲ʱ���ֵ����һ�����
		* @param fLevel ���� [0.0f,1.0f]
		* @return bool �Ϸ�����true�����Ϸ���ʧ�ܷ���false
		*/
		bool SetVolume(float fLevel);
		/**
		* GetVolume
		* ��ȡ������ע�⣬��˲ʱ���ֵ����һ�����
		* @author LiuTao
		* @since 2021.10.23
		* @param void 
		* @return float ���� [0.0f,1.0f]
		*/
		inline float GetVolume()
		{
			return m_fVolume;
		}

		/**
		* GetLevel
		* ��ȡ��ǰ֡���ֵ
		* @author LiuTao
		* @since 2021.07.04
		* @return float ��ǰ֡��� [0.0f,1.0f]
		*/
		float GetLevel();

		/**
		* GetAudioDuration
		* ��ȡ��ǰ������Ƶ��ʱ������λ��ms
		* @author LiuTao
		* @since 2021.09.05
		* @return int ��Ƶ��ʱ������λ��ms
		*/
		int GetAudioDuration();

		/**
		* SetAudioCurrentTime
		* @brief ������Ƶ�Ĳ���λ�ã���λ��ms
		* @author LiuTao
		* @since 2021.09.05
		* @param iTime: ��Ƶ�Ĳ���λ��
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool SetAudioCurrentTime(int iTime);
		/**
		* GetAudioCurrentTime
		* ��ȡ��ǰ��Ƶ�Ĳ���λ�ã���λ��ms
		* @author LiuTao
		* @since 2021.09.05
		* @return int ��Ƶ����λ�ã���λ��ms
		*/
		int GetAudioCurrentTime();

	private:

		/**
		* _InnerUpdate
		* �������
		* @author LiuTao
		* @since 2021.05.29
		* @param updateStep ���μ�����µ�ʱ����λs
		* @return void
		*/
		void _InnerUpdate(float updateStep);

		/**
		* _InitBASS
		* ��ʼ��BASS��Ƶ��
		* @author LiuTao
		* @since 2021.05.29
		* @param ��
		* @return void
		*/
		inline void _InitBASS()
		{
			//��ʼ��BASS��Ƶ��
			BASS_Init(
				-1,//Ĭ���豸
				44100,//���������44100������ֵ��
				BASS_DEVICE_CPSPEAKERS,//�źţ�BASS_DEVICE_CPSPEAKERS 
				NULL,//���򴰿�,0���ڿ���̨����
				nullptr//���ʶ��,0ʹ��Ĭ��ֵ
			);
		}

		/**
		* _GetAudioCurrentTime
		* ��ȡ��ǰ���ŵ���λ�ã�ʱ�����꣩
		* @author LiuTao
		* @since 2021.05.29
		* @param ��
		* @return int ��ǰ��Ƶ����λ�õ�ʱ�����꣬��λ��ms
		*/
		int _GetAudioCurrentTime();

		/**
		* _SeekTo
		* ���ÿ�ʼ���ŵ�ʱ������
		* @author LiuTao
		* @since 2021.05.29
		* @param time ʱ�����꣬��λms
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool _SeekTo(int iTime);

		/**
		* _PreWelcome
		* @brief Ϊ����ӭЧ������׼������
		* @author LiuTao
		* @since 2021.08.28
		* @param void
		* @return void
		*/
		void _PreWelcome();

		// ����
	private:
		SGMConfigData*								m_pConfigData;					//!< ��������

		HSTREAM										m_streamAudio;
		std::string									m_strCoreAudioPath;				//!< ������Ƶ���·��
		std::wstring								m_strAudioPath;					//!< ���ִ��·��
		std::wstring								m_strCurrentFile;				//!< ���ڲ��ŵ��ļ���,XXX.mp3
		EGMA_STATE									m_eAudioState;					//!< ��ǰ����״̬
		int											m_iAudioLastTime;				//!< ��һ֡��Ƶʱ������,��λms
		int											m_iAudioCurrentTime;			//!< ��ǰ֡��Ƶʱ������,��λms
		int											m_iAudioDuration;				//!< ��ǰ���ŵ���Ƶʱ��,��λms

		float										m_fDeltaStep;					//!< ��λs
		float										m_fConstantStep;				//!< �ȼ�����µ�ʱ��,��λs
		bool										m_bWelcomeStart;				//!< ��ӭ���ܿ�ʼ��־
		bool										m_bWelcomeEnd;					//!< ��ӭ���ܽ�����־
		int											m_iWelcomeDuration;				//!< ��ӭ��Ƶʱ��,��λms

		float										m_fVolume;						//!< �������
	};
}	// GM
