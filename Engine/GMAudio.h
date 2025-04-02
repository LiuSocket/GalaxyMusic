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
		*/
		void Welcome();

		/**
		* @brief “欢迎效果”是否结束
		* @return bool 结束则返回true，否则false
		*/
		inline bool IsWelcomeFinished() const { return m_bWelcomeEnd;}

		/**
		* @brief 根据文件名，设置当前音频
		* @param strAudioFile:	当前音频文件名称，例如：xxx.mp3
		* @return bool 设置成功返回true，失败或者文件未找到返回false
		*/
		bool SetCurrentAudio(const std::wstring& strAudioFile);

		/**
		* @brief 获取当前音频文件名称
		* @return std::wstring 当前音频文件名称，例如：xxx.mp3,或者L""
		*/
		inline std::wstring GetCurrentAudio() const{ return m_strCurrentFile; }

		/**
		* @brief 音频播放控制
		* @param command:	指令类型枚举
		*/
		void AudioControl(EGMA_COMMAND command);

		/**
		* @brief 判断音频是否播放完毕
		* @return bool 完毕返回true，未完毕返回false
		*/
		bool IsAudioOver() const;

		/**
		* @brief 设置音量（注意，和瞬时振幅值不是一个概念）
		* @param fLevel 音量 [0.0f,1.0f]
		* @return bool 合法返回true，不合法或失败返回false
		*/
		bool SetVolume(float fLevel);
		/**
		* @brief 获取音量（注意，和瞬时振幅值不是一个概念）
		* @return float 音量 [0.0f,1.0f]
		*/
		inline float GetVolume() const { return m_fVolume;}

		/**
		* @brief 获取当前帧振幅值
		* @return float 当前帧振幅 [0.0f,1.0f]
		*/
		float GetLevel() const;

		/**
		* @brief 获取当前播放音频的时长，单位：ms
		* @return int 音频的时长，单位：ms
		*/
		inline int GetAudioDuration() const { return m_iAudioDuration;}

		/**
		* @brief 设置音频的播放位置，单位：ms
		* @param iTime: 音频的播放位置
		* @return bool 成功true， 失败false
		*/
		bool SetAudioCurrentTime(int iTime);
		/**
		* @brief 获取当前音频的播放位置，单位：ms
		* @return int 音频播放位置，单位：ms
		*/
		inline int GetAudioCurrentTime() const { return m_iAudioCurrentTime;}

	private:

		/**
		* @brief 间隔更新
		* @param updateStep 两次间隔更新的时间差，单位s
		*/
		void _InnerUpdate(float updateStep);

		/** @brief 初始化BASS音频库 */
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
		* @brief 获取当前播放到的位置（时域坐标）
		* @return int 当前音频播放位置的时域坐标，单位：ms
		*/
		int _GetAudioCurrentTime() const;

		/**
		* @brief 设置开始播放的时域坐标
		* @param time 时域坐标，单位ms
		* @return bool 成功true， 失败false
		*/
		bool _SeekTo(int iTime);

		/** @brief 为“欢迎效果”做准备工作 */
		void _PreWelcome();

		// 变量
	private:
		SGMConfigData*						m_pConfigData = nullptr;				//!< 配置数据
		std::string							m_strCoreAudioPath = "Audios/";			//!< 核心音频存放路径
		std::wstring						m_strAudioPath = L"Music/";				//!< 音乐存放路径
		std::wstring						m_strCurrentFile = L"";					//!< 正在播放的文件名,XXX.mp3
		HSTREAM								m_streamAudio = 0;
		EGMA_STATE							m_eAudioState = EGMA_STA_MUTE;			//!< 当前播放状态
		int									m_iAudioLastTime = 0;					//!< 上一帧音频时域坐标,单位ms
		int									m_iAudioCurrentTime = 0;				//!< 当前帧音频时域坐标,单位ms
		int									m_iAudioDuration = 0;					//!< 当前播放的音频时长,单位ms

		bool								m_bWelcomeStart = false;				//!< 欢迎功能开始标志
		bool								m_bWelcomeEnd = false;					//!< 欢迎功能结束标志
		int									m_iWelcomeDuration = 5000;				//!< 欢迎音频时长,单位ms

		float								m_fVolume = 0.5f;						//!< 软件音量
	};
}	// GM
