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

	// 音频播放命令枚举
	enum EGMA_COMMAND
	{
		EGMA_CMD_OPEN,				// 打开音频
		EGMA_CMD_PLAY,				// 播放音频
		EGMA_CMD_CLOSE,				// 关闭音频
		EGMA_CMD_PAUSE,				// 暂停音频
		EGMA_CMD_STOP				// 停止音频
	};

	// 当前播放状态
	enum EGMA_STATE
	{
		EGMA_STA_MUTE,				// 已静音
		EGMA_STA_PLAY,				// 正在播放
		EGMA_STA_PAUSE,				// 已暂停
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
		// 函数
	public:
		/** @brief 构造 */
		CGMAudio();

		/** @brief 析构 */
		~CGMAudio();

		/** @brief 初始化 */
		bool Init(SGMConfigData* pConfigData);
		/** @brief 更新 */
		bool Update(double dDeltaTime);

		/**
		* Welcome
		* @brief 开启“欢迎效果”
		* @brief 每次开启软件，Engine调用此函数以实现“欢迎功能”
		* @author LiuTao
		* @since 2021.07.24
		* @param void
		* @return void
		*/
		void Welcome();

		/**
		* SetCurrentAudio
		* 根据文件名，设置当前音频
		* @author LiuTao
		* @since 2021.05.29
		* @param strAudioFile:	当前音频文件名称，例如：xxx.mp3
		* @return bool 设置成功返回true，失败或者文件未找到返回false
		*/
		bool SetCurrentAudio(std::wstring& strAudioFile);

		/**
		* GetCurrentAudio
		* 获取当前音频文件名称
		* @author LiuTao
		* @since 2021.07.24
		* @param void	
		* @return std::wstring 当前音频文件名称，例如：xxx.mp3,或者L""
		*/
		std::wstring GetCurrentAudio();

		/**
		* AudioControl
		* 音频播放控制
		* @author LiuTao
		* @since 2021.05.29
		* @param command:	指令类型枚举
		* @return void
		*/
		void AudioControl(EGMA_COMMAND command);

		/**
		* IsAudioOver
		* 判断音频是否播放完毕
		* @author LiuTao
		* @since 2021.05.29
		* @param 无
		* @return bool 完毕返回true，未完毕返回false
		*/
		bool IsAudioOver();

		/**
		* GetLevel
		* 获取当前帧振幅值
		* @author LiuTao
		* @since 2021.07.04
		* @return float 当前帧振幅 [0.0f,1.0f]
		*/
		float CGMAudio::GetLevel();

	private:

		/**
		* _InnerUpdate
		* 间隔更新
		* @author LiuTao
		* @since 2021.05.29
		* @param updateStep 两次间隔更新的时间差，单位s
		* @return void
		*/
		void _InnerUpdate(float updateStep);

		/**
		* _InitBASS
		* 初始化BASS音频库
		* @author LiuTao
		* @since 2021.05.29
		* @param 无
		* @return void
		*/
		inline void _InitBASS()
		{
			//初始化BASS音频库
			BASS_Init(
				-1,//默认设备
				44100,//输出采样率44100（常用值）
				BASS_DEVICE_CPSPEAKERS,//信号，BASS_DEVICE_CPSPEAKERS 
				NULL,//程序窗口,0用于控制台程序
				nullptr//类标识符,0使用默认值
			);
		}

		/**
		* GetAudioCurrentPos
		* 获取当前播放到的位置（时域坐标）
		* @author LiuTao
		* @since 2021.05.29
		* @param 无
		* @return int 当前音频播放位置的时域坐标，单位：ms
		*/
		int _GetAudioCurrentTime();

		/**
		* _SeekTo
		* 设置开始播放的时域坐标
		* @author LiuTao
		* @since 2021.05.29
		* @param time 时域坐标，单位ms
		* @return void
		*/
		void _SeekTo(int iTime);

		// 变量
	private:
		SGMConfigData*								m_pConfigData;					//!< 配置数据

		HSTREAM										m_streamAudio;
		std::string									m_strCoreAudioPath;				//!< 核心音频存放路径
		std::wstring								m_strAudioPath;					//!< 音乐存放路径
		std::wstring								m_strCurrentFile;				//!< 正在播放的文件名,XXX.mp3
		EGMA_STATE									m_eAudioState;					//!< 当前播放状态
		int											m_iAudioLastTime;				//!< 上一帧音频时域坐标,单位ms
		int											m_iAudioCurrentTime;			//!< 当前帧音频时域坐标,单位ms
		int											m_iAudioDuration;				//!< 当前播放的音频时长,单位ms

		float										m_fDeltaStep;					//!< 单位s
		float										m_fConstantStep;				//!< 等间隔更新的时间,单位s
		bool										m_bWelcome;						//!< 是否启用欢迎功能
		double										m_fWelcomeTime;					//!< “欢迎状态”持续了多长时间
	};
}	// GM
