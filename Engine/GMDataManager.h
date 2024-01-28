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
#include "GMDispatchCompute.h"

#include <osg/Texture2D>

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
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData);
		/** @brief 更新 */
		bool Update(double dDeltaTime);
		/** @brief 保存 */
		bool Save();

		/**
		* GetAudioNum
		* 获取音频总数，最多支持65536首歌
		* @author LiuTao
		* @since 2021.06.20
		* @return int 音频总数
		*/
		inline int GetAudioNum(){ return int(m_audioDataMap.size()); }

		/**
		* GetAudioDataMap
		* 获取音频数据map，最多支持65536首歌
		* @author LiuTao
		* @since 2021.06.27
		* @return bool 成功返回true，失败返回false
		*/
		bool GetAudioDataMap(std::map<unsigned int, SGMAudioData>& dataMap);

		/**
		* FindAudio(std::wstring& strName)
		* 查询音频文件
		* @author LiuTao
		* @since 2021.06.22
		* @return bool 存在返回true，不存在返回false
		*/
		bool FindAudio(const std::wstring& strName);

		/**
		* FindAudio(double& fX, double& fY, double& fZ, std::wstring& strName)
		* 根据银河坐标查询附近的音频文件名称
		* @author LiuTao
		* @since 2021.06.22
		* @param fX,fY,fZ:	需要查询的银河坐标[-1,1]，如果查询到，则修改为最近音频的银河坐标
		* @param strName:	查询到的音频星名称，如果失败，返回空字符串 L""
		* @return bool 存在返回true，不存在返回false
		*/
		bool FindAudio(double& fX, double& fY, double& fZ, std::wstring& strName);

		/**
		* FindAudio(const unsigned int iUID)
		* 根据音频ID查询音频文件
		* @author LiuTao
		* @since 2021.07.02
		* @param iID:	需要查询的ID，查询前应该先获取音频总数量做上限
		* @return wstring 存在返回音频文件名称，不存在则返回空字符串 L""
		*/
		std::wstring FindAudio(const unsigned int iUID);

		/**
		* GetLastAudio()
		* 查询上一首音频文件名称，但有代价：回到过去观察历史信息，未来便会改变
		* @author LiuTao
		* @since 2021.11.07
		* @param void
		* @return wstring 存在返回音频文件名称，不存在则返回空字符串 L""
		*/
		std::wstring GetLastAudio();

		/**
		* GetCurrentAudio() const
		* 获取当前音频名称
		* @author LiuTao
		* @since 2022.03.27
		* @param void
		* @return std::wstring：	当前音频名称
		*/
		inline std::wstring GetCurrentAudio() const
		{
			return m_strCurrentAudio;
		};

		/**
		* GetCurrentAudioID() const
		* 获取当前音频ID
		* @author LiuTao
		* @since 2021.10.23
		* @param void
		* @return int	当前音频ID
		*/
		int GetCurrentAudioID() const;

		/**
		* GetAudioColor
		* 根据音频空间坐标，获取星音乐的颜色。
		* @author LiuTao
		* @since 2021.07.03
		* @param audioCoord：	音频空间坐标
		* @return osg::Vec4f：	星音乐类型对应的颜色
		*/
		osg::Vec4f GetAudioColor(const SGMAudioCoord& audioCoord) const;

		/**
		* GetAudioColor const
		* 根据星辰坐标，获取星音乐的颜色。
		* @author LiuTao
		* @since 2021.07.03
		* @param iUID：				音频UID
		* @return osg::Vec4f：		星音乐类型对应的颜色
		*/
		osg::Vec4f GetAudioColor(const unsigned int iUID) const;

		/**
		* Angle2Color const
		* 星音乐的“情绪角度”转“颜色”。
		* @author LiuTao
		* @since 2022.09.04
		* @param fEmotionAngle：	情绪角度，[0.0,2*PI)
		* @return osg::Vec4f：		星音乐类型对应的颜色,[0.0,1.0]
		*/
		osg::Vec4f Angle2Color(const float fEmotionAngle) const;

		/**
		* GetUID() const
		* 获取当前播放的星音乐的UID。
		* @author LiuTao
		* @since 2021.07.04
		* @param void
		* @return unsigned int：	音频UID
		*/
		unsigned int GetUID() const;

		/**
		* GetUID const
		* 根据星音乐名称，获取星音乐的星辰坐标。
		* @author LiuTao
		* @since 2021.07.04
		* @param strName:			星音乐名称
		* @return unsigned int：	音频UID
		*/
		unsigned int GetUID(const std::wstring& strName) const;

		/**
		* GetAudioCoord const
		* 根据星音乐名称，获取星音乐的音频空间坐标。
		* @author LiuTao
		* @since 2021.07.04
		* @param strName:			星音乐名称
		* @return SGMAudioCoord：	音频空间坐标
		*/
		SGMAudioCoord GetAudioCoord(const std::wstring& strName) const;

		/**
		* GetGalaxyCoord const
		* 根据星音乐名称，获取星音乐的银河坐标。
		* @author LiuTao
		* @since 2021.07.04
		* @param strName:			星音乐名称
		* @return SGMGalaxyCoord：	银河坐标
		*/
		SGMGalaxyCoord GetGalaxyCoord(const std::wstring& strName) const;

		/**
		* EditAudioData
		* @brief 修改已有的某个星音乐的位置数据。如果不合法或者音乐不存在，则返回false
			如果位置与已知音频有重合，则自动迁移到合适位置，修改sData的坐标
		* @author LiuTao
		* @since 2021.07.04
		* @param sData：			星音乐数据结构
		* @return bool：			成功true，失败false
		*/
		bool EditAudioData(SGMAudioData& sData);

		/**
		* SavePlayingOrder
		* 界面上的音频的播放顺序，写入Data/Core/Users/AudioPlayingOrder.xml
		* @author LiuTao
		* @since 2022.03.26
		* @param void：		无
		* @return bool：	成功true，失败false
		*/
		bool SavePlayingOrder() const;

		/**
		* GetPlayingOrder const
		* 获取最近播放音频的list，back为最新歌曲
		* @author LiuTao
		* @since 2022.03.26
		* @param void：							无
		* @return std::vector<std::wstring>：	最近播放音频的list
		*/
		inline const std::vector<std::wstring> GetPlayingOrder() const
		{
			return m_playingOrder;
		}

		/**
		* AudioCoord2GalaxyCoord const
		* 音频空间坐标转银河坐标
		* @author LiuTao
		* @since 2021.07.23
		* @param audioCoord:		音频空间坐标
		* @return SGMGalaxyCoord:	银河坐标
		*/
		SGMGalaxyCoord AudioCoord2GalaxyCoord(const SGMAudioCoord& audioCoord) const;

	private:

		/**
		* _RefreshAudioCoordinates
		* 读取Data/Core/Users/AudioData.xml，并将音频坐标保存在m_audioDataMap中
		* @author LiuTao
		* @since 2021.06.14
		* @return void
		*/
		void _RefreshAudioCoordinates();

		/**
		* _SaveAudioCoordinates const
		* 将音频坐标写入Data/Core/Users/AudioData.xml
		* @author LiuTao
		* @since 2021.06.27
		* @return bool 成功true，失败false
		*/
		bool _SaveAudioCoordinates()const;

		/**
		* _RefreshAudioFiles
		* 扫描Data/Media/Music路径下所有支持的音频文件，并将文件名保存在m_audioDataMap中
		* @author LiuTao
		* @since 2021.06.14
		* @return void
		*/
		void _RefreshAudioFiles();

		/**
		* _DeleteOverdueAudios
		* 删除过期的文件列表项，过期是指：文件夹中没有该歌曲
		* 该函数很耗时
		* @author LiuTao
		* @since 2022.04.23
		* @return void
		*/
		void _DeleteOverdueAudios();

		/**
		* _UpdateAudioList
		* 更新最近播放音频的list
		* @author LiuTao
		* @since 2022.03.26
		* @param strName：		最后查询（也就是播放）的音频名称
		* @return void：		无
		*/
		void _UpdateAudioList(const std::wstring& strName);

		/**
		* _LoadPlayingOrder
		* 从Data/Core/Users/AudioPlayingOrder.xml 读取已经保存的音频播放顺序
		* @author LiuTao
		* @since 2022.03.27
		* @param void：			无
		* @return void：		无
		*/ 
		void _LoadPlayingOrder();

		/**
		* @brief 混合函数,参考 glsl 中的 mix(a,b,x)
		* @author LiuTao
		* @since 2022.01.09
		* @param fMin, fMax:			范围
		* @param fX:					混合系数
		* @return double:				混合后的值
		*/
		inline double _Mix(const double fMin, const double fMax, const double fX) const
		{
			return fMin * (1 - fX) + fMax * fX;
		}

		/**
		* @brief 根据输入的AudioData更新map，m_audioDataMap的添加和修改都必须使用这个函数
			并检查和更新FreeUID，保证FreeUID是一直空闲可用的
		* @author LiuTao
		* @since 2022.08.21
		* @param sData:				输入的AudioData
		* @return bool:				新数据是添加，则true，新数据是修改，则false
		*/
		bool _AddAudioData2Map(SGMAudioData& sData);

		/**
		* 初始化：使用compute shader计算最近的可播放音频，存储在图片中
		*/
		void _InitComputeNearUID();

		/**
		* 创建渲染面
		* @brief 屏幕两倍大小的三角面，比矩形效率要高一些
		* @author LiuTao
		* @since 2020.09.01
		* @param width: 等效矩形的宽度
		* @param height: 等效矩形的高度
		* @return osg::Geometry* 返回几何节点指针
		*/
		osg::Geometry* _CreateScreenTriangle(const int width, const int height);

		// 变量
	private:
		SGMKernelData*								m_pKernelData;					//!< 内核数据
		SGMConfigData*								m_pConfigData;					//!< 配置数据

		std::wstring								m_strAudioPath;					//!< 音频存放路径
		std::wstring								m_strCurrentAudio;				//!< 当前播放的音频名称
		std::vector<std::wstring>					m_formatVector;					//!< 支持的文件类型，例如mp3
		std::vector<std::wstring>					m_playingOrder;					//!< 界面上的音频播放顺序（不重名）
		std::map<unsigned int, SGMAudioData>		m_audioDataMap;					//!< AudioData.xml已有的数据map
		std::list<std::wstring>						m_historyList;					//!< 音频播放历史列表（可重名）
		unsigned int								m_iFreeUID;						//!< 当前可用的UID，会随时更新

		osg::ref_ptr<osg::Uniform>					m_pAudioNumUniform;				//!< 音频总数Uniform

		/** m_pGalaxyCoordImage 存储所有音频的银河坐标的图片：
		** m_pGalaxyCoordTex 存储所有音频的银河坐标的纹理
		** 纹理为R32F		音频的位置，两个像素表示一个音频的坐标
		 */
		osg::ref_ptr<osg::Image>					m_pGalaxyCoordImage;
		osg::ref_ptr<osg::Texture2D>				m_pGalaxyCoordTex;
		/** m_pNearUIDImage 存储距离最近的音频位置信息的图片：
		** m_pNearUIDTex 相应的纹理
		** RGB8		音频UID，24位，理论上可以标记16777216（一千多万）个音频
		** A8		距离信息 */
		osg::ref_ptr<osg::Image>					m_pNearUIDImage;
		osg::ref_ptr<osg::Texture2D>				m_pNearUIDTex;
		osg::ref_ptr<CGMDispatchCompute>			m_pNearComputeNode;				//!< 计算最近UID的CS节点
		osg::ref_ptr<osg::Camera>					m_pReadPixelCam;				//!< 用于读取像素的相机
		CReadPixelFinishCallback*					m_pReadPixelFinishCallback;
	};
}	// GM