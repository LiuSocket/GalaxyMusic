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

	// 当前音频播放模式
	enum EGMA_MODE
	{
		EGMA_MOD_SINGLE,			// 单曲循环
		EGMA_MOD_CIRCLE,			// 列表循环
		EGMA_MOD_RANDOM,			// 随机播放
		EGMA_MOD_ORDER				// 顺序播放
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
		// 函数
	protected:
		/** @brief 构造 */
		CGMEngine();
		/** @brief 析构 */
		~CGMEngine();

	public:
		/** @brief 获取单例 */
		static CGMEngine& getSingleton(void);
		static CGMEngine* getSingletonPtr(void);

		/** @brief 初始化 */
		bool Init();
		/** @brief 释放 */
		void Release();
		/** @brief 更新 */
		bool Update();
		/** @brief 加载 */
		bool Load();
		/** @brief 保存 */
		bool Save();

		/** @brief 开启/关闭编辑模式 */
		void SetEditMode(const bool bEnable);
		/** @brief 开启/关闭捕获功能 */
		void SetCapture(const bool bEnable);

		/**
		* SetAudio
		* @brief 选择音频
		* @author LiuTao
		* @since 2021.05.30
		* @param fX, fY: 鼠标点击位置的世界空间坐标
		* @return bool 成功true， 失败false
		*/
		bool SetAudio(const float fX, const float fY);
		/** @brief 播放 */
		bool Play();
		/** @brief 暂停 */
		bool Pause();
		/** @brief 停止 */
		bool Stop();
		/** @brief 下一首 */
		bool Next();

		/**
		* SetMousePosition
		* 将当前鼠标世界空间坐标传入
		* @author LiuTao
		* @since 2021.07.04
		* @param vPos:			当前帧鼠标世界空间坐标
		* @return void
		*/
		void SetMousePosition(const osg::Vec3f vPos);

		/**
		* GetCurrentStarWorldPos
		* 获取当前播放的音频星世界空间坐标
		* @author LiuTao
		* @since 2021.07.30
		* @param void
		* @return osg::Vec3d	当前播放的音频星世界空间坐标
		*/
		osg::Vec3d GetCurrentStarWorldPos();

		/** @brief 创建视口管理器 */
		osgViewer::Viewer* CreateViewer();
		/** @brief 获取内核数据 */
		inline SGMKernelData* GetKernelData() { return m_pKernelData; }
		/** @brief 获取配置数据 */
		inline SGMConfigData* GetConfigData() { return m_pConfigData; }

		/** @brief 获取音频数据管理类 */
		inline CGMDataManager* GetDataManager() { return m_pDataManager; }

	private:
		/**
		* _Next
		* 播放下一首
		* @author LiuTao
		* @since 2021.07.23
		* @param void
		* @return void
		*/
		void _Next();

		/**
		* _InnerUpdate
		* 间隔更新
		* @author LiuTao
		* @since 2021.07.04
		* @param updateStep 两次间隔更新的时间差，单位s
		* @return void
		*/
		void _InnerUpdate(float updateStep);

		/**
		* _World2GalaxyCoord
		* @brief 世界空间坐标 转 银河空间坐标
		* @author LiuTao
		* @since 2021.06.22
		* @param fX, fY: 世界空间坐标
		* @param &fGalaxyX, &fGalaxyY: 银河空间坐标[-1,1]
		* @return bool 成功true， 失败false
		*/
		bool _World2GalaxyCoord(const float fX, const float fY, float& fGalaxyX, float& fGalaxyY);

		/**
		* _World2GalaxyCoord
		* @brief 银河空间坐标 转 世界空间坐标
		* @author LiuTao
		* @since 2021.07.04
		* @param fGalaxyX, fGalaxyY: 银河空间坐标[-1,1]
		* @param &fX, &fY: 如果成功，则代表世界空间坐标
		* @return bool 成功true， 失败false
		*/
		bool _GalaxyCoord2World(const float fGalaxyX, const float fGalaxyY, float& fX, float& fY);

		/**
		* _StarCoord2World
		* @brief 星辰坐标 转 世界空间坐标
		* @author LiuTao
		* @since 2021.07.04
		* @param starCoord: 星辰坐标
		* @param &fX, &fY: 如果成功，则代表世界空间坐标
		* @return bool 成功true， 失败false
		*/
		bool _StarCoord2World(const SGMStarCoord& starCoord, float& fX, float& fY);

		// 变量
	private:

		SGMKernelData*						m_pKernelData;				//!< 内核数据
		SGMConfigData*						m_pConfigData;				//!< 配置数据
		CGMDataManager*						m_pDataManager;				//!< 数据管理

		bool								m_bInit;					//!< 初始化标志
		bool								m_bHasWelcomed;				//!< 是否展示过欢迎功能
		double								m_dTimeLastFrame;			//!< 上一帧时间
		float								m_fDeltaStep;				//!< 单位s
		float								m_fConstantStep;			//!< 等间隔更新的时间,单位s
		float								m_fGalaxyDiameter;			//!< 星系直径
		CGMGalaxy*							m_pGalaxy;					//!< 星系
		CGMAudio*							m_pAudio;					//!< 音频
		EGMA_MODE							m_ePlayMode;				//!< 当前播放模式
		std::default_random_engine			m_iRandom;
	};
}	// GM