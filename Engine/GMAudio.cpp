//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMAudio.cpp
/// @brief		Galaxy-Music Engine - GMAudio
/// @version	1.0
/// @author		LiuTao
/// @date		2021.05.29
//////////////////////////////////////////////////////////////////////////

#include "GMAudio.h"

using namespace GM;

/*************************************************************************
Macro Defines
*************************************************************************/

/*************************************************************************
Structs
*************************************************************************/

/*************************************************************************
CGMAudio Methods
*************************************************************************/

/** @brief 构造 */
CGMAudio::CGMAudio():
	m_pConfigData(nullptr),m_streamAudio(0), 
	m_strCoreAudioPath("Audio/"), m_strAudioPath(L"Music/"), m_strCurrentFile(L""),
	m_eAudioState(EGMA_STA_MUTE),
	m_iAudioLastTime(0), m_iAudioCurrentTime(0), m_iAudioDuration(0),
	m_fDeltaStep(0.0f), m_fConstantStep(0.1f), m_bWelcome(false), m_fWelcomeTime(0.0)
{
}

/** @brief 析构 */
CGMAudio::~CGMAudio()
{
}

/** @brief 初始化 */
bool CGMAudio::Init(SGMConfigData * pConfigData)
{
	m_pConfigData = pConfigData;

	_InitBASS();

	return true;
}

/** @brief 更新 */
bool CGMAudio::Update(double dDeltaTime)
{
	if (m_bWelcome)
	{
		if (GM_WELCOME_MAX_TIME > m_fWelcomeTime)
		{
			m_fWelcomeTime += dDeltaTime;
		}
		else
		{
			BASS_StreamFree(m_streamAudio);
			m_eAudioState = EGMA_STA_MUTE;

			m_bWelcome = false;
		}
	}

	float fDeltaTime = float(dDeltaTime);
	fDeltaTime += m_fDeltaStep;
	float updateStep = m_fConstantStep;
	while (fDeltaTime >= updateStep)
	{
		_InnerUpdate(updateStep);
		fDeltaTime -= updateStep;
	}
	m_fDeltaStep = fDeltaTime;

	return true;
}

void CGMAudio::Welcome()
{
	m_bWelcome = true;

	std::string strFile = m_pConfigData->strCorePath + m_strCoreAudioPath + "Start.mp3";
	m_streamAudio = BASS_StreamCreateFile(FALSE, strFile.c_str(), 0, 0, 0);

	// 获取当前音频时长
	QWORD length_bytes = BASS_ChannelGetLength(m_streamAudio, BASS_POS_BYTE);
	double length_sec = BASS_ChannelBytes2Seconds(m_streamAudio, length_bytes);
	m_iAudioDuration = static_cast<int>(std::fmax(0.0, length_sec) * 1000);
	BASS_ChannelPlay(m_streamAudio, FALSE);
	m_eAudioState = EGMA_STA_PLAY;
}

bool CGMAudio::SetCurrentAudio(std::wstring& strAudioFile)
{
	if (m_bWelcome) return false;

	if (L"" != strAudioFile)
	{
		AudioControl(EGMA_CMD_CLOSE);
		m_strCurrentFile = strAudioFile;
		AudioControl(EGMA_CMD_OPEN);
		return true;
	}
	else
	{
		return false;
	}
}

std::wstring CGMAudio::GetCurrentAudio()
{
	return m_strCurrentFile;
}

/**
* MusicControl
* 音乐播放控制
* @author LiuTao
* @since 2021.05.29
* @param command:	指令类型枚举
* @return void
*/
void CGMAudio::AudioControl(EGMA_COMMAND command)
{
	if (m_bWelcome) return;

	switch (command)
	{
	case EGMA_CMD_OPEN:
	{
		std::wstring strFile = m_pConfigData->strMediaPath + m_strAudioPath + m_strCurrentFile;
		m_streamAudio = BASS_StreamCreateFile(FALSE, strFile.c_str(), 0, 0, 0);

		// 获取当前音频时长
		QWORD length_bytes = BASS_ChannelGetLength(m_streamAudio, BASS_POS_BYTE);
		double length_sec = BASS_ChannelBytes2Seconds(m_streamAudio, length_bytes);
		m_iAudioDuration = static_cast<int>(std::fmax(0.0,length_sec) * 1000);

		m_eAudioState = EGMA_STA_MUTE;
	}
	break;
	case EGMA_CMD_PLAY:
	{
		BASS_ChannelPlay(m_streamAudio, FALSE);
		m_eAudioState = EGMA_STA_PLAY;
	}
	break;
	case EGMA_CMD_CLOSE:
	{
		BASS_StreamFree(m_streamAudio);
		m_eAudioState = EGMA_STA_MUTE;
	}
	break;
	case EGMA_CMD_PAUSE:
	{
		BASS_ChannelPause(m_streamAudio);
		m_eAudioState = EGMA_STA_PAUSE;
	}
	break;
	case EGMA_CMD_STOP:
	{
		BASS_ChannelStop(m_streamAudio);
		_SeekTo(0);
		m_eAudioState = EGMA_STA_MUTE;
	}
	break;
	default:
		break;
	}
}

bool CGMAudio::IsAudioOver()
{
	if (EGMA_STA_PLAY == m_eAudioState
		&& m_iAudioCurrentTime == m_iAudioLastTime
		&& m_iAudioCurrentTime != 0
		&& m_iAudioCurrentTime > m_iAudioDuration - 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

float CGMAudio::GetLevel()
{
	if (EGMA_STA_PLAY != m_eAudioState || IsAudioOver())
	{
		return 0.0f;
	}

	DWORD level, left, right;
	level = BASS_ChannelGetLevel(m_streamAudio);
	left = LOWORD(level); // the left level
	right = HIWORD(level); // the right level
	float fLevel = min(1.0f, max(left, right) / 32768.0f);

	//float fft[128]; // fft data buffer
	//BASS_ChannelGetData(m_streamAudio, fft, BASS_DATA_FFT256);
	//float fMax = 0.0f;
	//for (int i = 0; i < 128; i++)
	//{
	//	fMax = std::fmax(fMax, fft[i]);
	//}
	//fMax = min(10*fMax, 1.0f);
	return fLevel;
}

/**
* _InnerUpdate
* 间隔更新
* @author LiuTao
* @since 2021.05.29
* @param updateStep 两次间隔更新的时间差，单位s
* @return void
*/
void CGMAudio::_InnerUpdate(float updateStep)
{
	m_iAudioLastTime = m_iAudioCurrentTime;
	m_iAudioCurrentTime = _GetAudioCurrentTime();
}

/**
* GetAudioCurrentPos
* 获取当前播放到的位置（时域坐标）
* @author LiuTao
* @since 2021.05.29
* @param 无
* @return int 当前音频播放位置的时域坐标，单位：ms
*/
int CGMAudio::_GetAudioCurrentTime()
{
	QWORD pos_bytes;
	pos_bytes = BASS_ChannelGetPosition(m_streamAudio, BASS_POS_BYTE);
	double pos_sec;
	pos_sec = BASS_ChannelBytes2Seconds(m_streamAudio, pos_bytes);
	return static_cast<int>(pos_sec * 1000);
}

/**
* _SeekTo
* 设置开始播放的时域坐标
* @author LiuTao
* @since 2021.05.29
* @param time 时域坐标，单位ms
* @return void
*/
void CGMAudio::_SeekTo(int iTime)
{
	if (iTime > m_iAudioDuration)
	{
		iTime = m_iAudioDuration;
	}
	m_iAudioCurrentTime = iTime;
	double pos_sec = static_cast<double>(iTime) / 1000.0;
	QWORD pos_bytes = BASS_ChannelSeconds2Bytes(m_streamAudio, pos_sec);
	BASS_ChannelSetPosition(m_streamAudio, pos_bytes, BASS_POS_BYTE);
}