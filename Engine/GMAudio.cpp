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
	m_fDeltaStep(0.0f), m_fConstantStep(0.1f), m_bWelcomeStart(false), m_bWelcomeEnd(false),
	m_iWelcomeDuration(5000), m_fVolume(0.5f)
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

	m_fVolume = m_pConfigData->fVolume;

	_InitBASS();
	// 为“欢迎效果”做准备工作
	_PreWelcome();

	return true;
}

/** @brief 更新 */
bool CGMAudio::Update(double dDeltaTime)
{
	if (m_bWelcomeStart && !m_bWelcomeEnd)
	{
		QWORD pos_bytes;
		pos_bytes = BASS_ChannelGetPosition(m_streamAudio, BASS_POS_BYTE);
		double pos_sec;
		pos_sec = BASS_ChannelBytes2Seconds(m_streamAudio, pos_bytes);

		if (m_iWelcomeDuration <= static_cast<int>(pos_sec * 1000))
		{
			BASS_StreamFree(m_streamAudio);
			m_eAudioState = EGMA_STA_MUTE;

			m_bWelcomeEnd = true;
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
	BASS_ChannelPlay(m_streamAudio, FALSE);
	m_eAudioState = EGMA_STA_PLAY;

	m_bWelcomeStart = true;
}

bool CGMAudio::IsWelcomeFinished()
{
	return m_bWelcomeEnd;
}

bool CGMAudio::SetCurrentAudio(std::wstring& strAudioFile)
{
	if (!m_bWelcomeEnd) return false;

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
	if (!m_bWelcomeEnd) return;

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

		//const char* tag = BASS_ChannelGetTags(m_streamAudio, BASS_TAG_ID3V2);
		// https://blog.csdn.net/u013401219/article/details/48103315
		// https://github.com/alkatrazstudio/meson-sound-engine/blob/0e289ad770a892cda7999431ebe51eebbec51ef5/modules/MesonSoundEngine/mse/sources/source_stream.cpp

		m_eAudioState = EGMA_STA_MUTE;
	}
	break;
	case EGMA_CMD_PLAY:
	{
		BASS_ChannelSetAttribute(m_streamAudio, BASS_ATTRIB_VOL, m_fVolume);
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
		&& m_iAudioCurrentTime > m_iAudioDuration - 100)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool CGMAudio::SetVolume(float fLevel)
{
	m_fVolume = fLevel;
	return BASS_ChannelSetAttribute(m_streamAudio, BASS_ATTRIB_VOL, m_fVolume);
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

int CGMAudio::GetAudioDuration()
{
	return m_iAudioDuration;
}

bool CGMAudio::SetAudioCurrentTime(int iTime)
{
	_SeekTo(iTime);
	return true;
}

int CGMAudio::GetAudioCurrentTime()
{
	return m_iAudioCurrentTime;
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

int CGMAudio::_GetAudioCurrentTime()
{
	QWORD pos_bytes;
	pos_bytes = BASS_ChannelGetPosition(m_streamAudio, BASS_POS_BYTE);
	double pos_sec;
	pos_sec = BASS_ChannelBytes2Seconds(m_streamAudio, pos_bytes);
	return static_cast<int>(pos_sec * 1000);
}

bool CGMAudio::_SeekTo(int iTime)
{
	if (iTime > m_iAudioDuration)
	{
		iTime = m_iAudioDuration;
	}
	m_iAudioCurrentTime = iTime;
	double pos_sec = static_cast<double>(iTime) / 1000.0;
	QWORD pos_bytes = BASS_ChannelSeconds2Bytes(m_streamAudio, pos_sec);
	return BASS_ChannelSetPosition(m_streamAudio, pos_bytes, BASS_POS_BYTE);
}

void CGMAudio::_PreWelcome()
{
	std::string strFile = m_pConfigData->strCorePath + m_strCoreAudioPath + "Start.mp3";
	m_streamAudio = BASS_StreamCreateFile(FALSE, strFile.c_str(), 0, 0, 0);

	// 获取当前音频时长
	QWORD length_bytes = BASS_ChannelGetLength(m_streamAudio, BASS_POS_BYTE);
	double length_sec = BASS_ChannelBytes2Seconds(m_streamAudio, length_bytes);
	m_iWelcomeDuration = static_cast<int>(std::fmax(0.0, length_sec) * 1000);
}
