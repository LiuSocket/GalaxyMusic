//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMDataManager.h
/// @brief		Galaxy-Music Engine - GMDataManager
/// @version	1.0
/// @author		LiuTao
/// @date		2021.06.20
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommon.h"
#include "GMKernel.h"
#include <random>

namespace GM
{
	/*************************************************************************
	Enums
	*************************************************************************/

	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/

	/*!
	*  @class CGMDataManager
	*  @brief Galaxy-Music CGMDataManager
	*/
	class CGMDataManager
	{
		// 函数
	public:
		/** @brief 构造 */
		CGMDataManager();

		/** @brief 析构 */
		~CGMDataManager();

		/** @brief 初始化 */
		bool Init(SGMConfigData* pConfigData);

		/** @brief 保存 */
		bool Save();

		/**
		* GetAudioNum
		* 获取音频总数，最多支持65536首歌
		* @author LiuTao
		* @since 2021.06.20
		* @return int 音频总数
		*/
		inline int GetAudioNum(){ return int(m_fileVector.size()); }

		/**
		* GetStarCoordVector
		* 获取音频的星辰坐标Vector，最多支持65536首歌
		* @author LiuTao
		* @since 2021.06.27
		* @return bool 成功返回true，失败返回false
		*/
		bool GetStarCoordVector(std::vector<SGMStarCoord>& coordV);

		/**
		* FindAudio(std::wstring& strName)
		* 查询音频文件
		* @author LiuTao
		* @since 2021.06.22
		* @return bool 存在返回true，不存在返回false
		*/
		bool FindAudio(const std::wstring& strName);

		/**
		* FindAudio(float& fX, float& fY, float& fZ)
		* 根据银河坐标查询附近的音频文件名称
		* @author LiuTao
		* @since 2021.06.22
		* @param fX,fY,fZ:	需要查询的位置（银河坐标） [-1,-1]，如果存在，则修改坐标
		* @return wstring 存在返回音频文件名称，不存在则返回空字符串 L""
		*/
		std::wstring FindAudio(double& fX, double& fY, double& fZ);

		/**
		* FindAudio(int iID)
		* 根据音频ID查询音频文件
		* @author LiuTao
		* @since 2021.07.02
		* @param iID:	需要查询的ID，查询前应该先获取音频总数量做上限
		* @return wstring 存在返回音频文件名称，不存在则返回空字符串 L""
		*/
		std::wstring FindAudio(const int iID);

		/**
		* FindLastAudio()
		* 查询上一首音频文件名称
		* @author LiuTao
		* @since 2021.11.07
		* @param void
		* @return wstring 存在返回音频文件名称，不存在则返回空字符串 L""
		*/
		std::wstring FindLastAudio();

		/**
		* GetCurrentAudioID
		* 获取当前音频ID
		* @author LiuTao
		* @since 2021.10.23
		* @param void
		* @return int	当前音频ID
		*/
		int GetCurrentAudioID();

		/**
		* GetAudioColor
		* 获取星音乐类型对应的颜色。
		* @author LiuTao
		* @since 2021.07.03
		* @param audioCoord:	音频空间坐标
		* @return osg::Vec4f	星音乐类型对应的颜色
		*/
		osg::Vec4f GetAudioColor(const SGMAudioCoord audioCoord);

		/**
		* GetAudioColor
		* 获取星音乐类型对应的颜色。
		* @author LiuTao
		* @since 2021.07.03
		* @param starCoord:			星辰坐标
		* @return osg::Vec4f：		星音乐类型对应的颜色
		*/
		osg::Vec4f GetAudioColor(const SGMStarCoord starCoord);

		/**
		* GetStarCoord
		* 获取当前播放的星音乐的星辰坐标。
		* @author LiuTao
		* @since 2021.07.04
		* @param void
		* @return SGMStarCoord：	星辰坐标
		*/
		SGMStarCoord GetStarCoord();

		/**
		* GetStarCoord
		* 获取星音乐的星辰坐标。
		* @author LiuTao
		* @since 2021.07.04
		* @param strName:			星音乐名称
		* @return SGMStarCoord：	星辰坐标
		*/
		SGMStarCoord GetStarCoord(const std::wstring& strName);

		/**
		* GetAudioCoord
		* 获取星音乐的音频空间坐标。
		* @author LiuTao
		* @since 2021.07.04
		* @param strName:			星音乐名称
		* @return SGMAudioCoord：	音频空间坐标
		*/
		SGMAudioCoord GetAudioCoord(const std::wstring& strName);

		/**
		* SetAudioData
		* 设置星音乐的数据。如果不合法，则返回false
		* @author LiuTao
		* @since 2021.07.04
		* @param sData：			星音乐数据结构
		* @return bool：			成功true，失败false
		*/
		bool SetAudioData(const SGMAudioData& sData);

	private:

		/**
		* _RefreshAudioCoordinates
		* 读取Data/Core/Coordinates/AudioData.xml，并将音频坐标保存在m_fileCoordsMap中
		* @author LiuTao
		* @since 2021.06.14
		* @return void
		*/
		void _RefreshAudioCoordinates();

		/**
		* _SaveAudioCoordinates
		* 将音频坐标写入Data/Core/Coordinates/AudioData.xml
		* @author LiuTao
		* @since 2021.06.27
		* @return bool 成功true，失败false
		*/
		bool _SaveAudioCoordinates();

		/**
		* _RefreshAudioFiles
		* 扫描Data/Media/Music路径下所有支持的音频文件，并将文件名保存在m_fileVector中
		* @author LiuTao
		* @since 2021.06.14
		* @return void
		*/
		void _RefreshAudioFiles();
		/**
		* _CharToWstring
		* 字符串转换 char* to wstring
		* @author LiuTao
		* @since 2021.06.26
		* @param cstr:	多字节字符串
		* @return wstring 宽字节字符串
		*/
		const std::wstring _CharToWstring(const char* cstr);
		/**
		* _WcharToChar
		* 字符串转换 wchar_t* to string
		* @author LiuTao
		* @since 2021.06.26
		* @param wstr:	宽字节字符串
		* @return string:	多字节字符串
		*/
		const std::string _WcharToChar(const wchar_t* wstr);

		/**
		* _AudioCoord2StarCoord
		* 音频空间坐标转星辰坐标
		* @author LiuTao
		* @since 2021.07.23
		* @param audioCoord:	音频空间坐标
		* @return SGMStarCoord:	星辰坐标
		*/
		SGMStarCoord _AudioCoord2StarCoord(const SGMAudioCoord& audioCoord);

		// 变量
	private:
		SGMConfigData*								m_pConfigData;					//!< 配置数据

		std::wstring								m_strAudioPath;					//!< 音乐存放路径
		std::wstring								m_strAudioName;					//!< 最后一次查询的文件名,XXX.mp3
		std::vector<std::wstring>					m_formatVector;					//!< 支持的文件类型，例如mp3
		std::vector<std::wstring>					m_fileVector;					//!< Media文件夹下所有支持的音频文件
		std::vector<std::wstring>					m_playedVector;					//!< 已播放音频的名称Vector
		std::vector<std::wstring>					m_storedNameVector;				//!< AudioData.xml已有的名称Vector
		std::vector<SGMStarCoord>					m_storedCoordVector;			//!< AudioData.xml已有的坐标Vector
		std::map<SGMStarCoord, SGMAudioData>		m_audioDataMap;					//!< 星辰坐标与音频数据的映射
		std::default_random_engine					m_iRandom;
		std::vector<osg::Vec2i>						m_vOffsetVector;				//!< 偏移顺序，用于查询音频
	};
}	// GM