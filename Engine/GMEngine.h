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
#include "GMEnums.h"
#include <random>

/*************************************************************************
Class
*************************************************************************/
class QWidget;

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
	class CGMCameraManipulator;

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
		* @brief 根据坐标选择需要操作的音频
		* @author LiuTao
		* @since 2021.05.30
		* @param fX, fY: 鼠标点击位置的世界空间坐标,单位：米
		* @return bool 成功true， 失败false
		*/
		bool SetAudio(const double fX, const double fY);
		/** @brief 播放 */
		bool Play();
		/** @brief 暂停 */
		bool Pause();
		/** @brief 停止 */
		bool Stop();
		/** @brief 上一首 */
		bool Last();
		/** @brief 下一首 */
		bool Next();
		/** @brief 设置音量，0.0-1.0 */
		bool SetVolume(float fVolume);
		/** @brief 获取音量，0.0-1.0 */
		float GetVolume();
		/**
		* SetPlayMode
		* @brief 设置播放模式
		* @author LiuTao
		* @since 2021.10.24
		* @param eMode:			播放模式（单曲循环、随机播放、列表循环等）
		* @return bool：		成功true， 失败false
		*/
		bool SetPlayMode(EGMA_MODE eMode);
		/**
		* GetPlayMode
		* @brief 获取播放模式
		* @author LiuTao
		* @since 2021.10.24
		* @param void		
		* @return EGMA_MODE：	播放模式（单曲循环、随机播放、列表循环等）
		*/
		EGMA_MODE GetPlayMode();

		/**
		* GetAudioName
		* 获取当前音频文件名称
		* @author LiuTao
		* @since 2021.09.11
		* @param void
		* @return std::wstring 当前播放的音频文件名称，含后缀名，未播放则返回 L""
		*/
		std::wstring GetAudioName();
		/**
		* SetAudioCurrentTime
		* @brief 设置音频的播放位置，单位：ms
		* @author LiuTao
		* @since 2021.09.05
		* @param iTime: 音频的播放位置
		* @return bool 成功true， 失败false
		*/
		bool SetAudioCurrentTime(int iTime);
		/**
		* GetAudioCurrentTime
		* @brief 获取音频的播放位置，单位：ms
		* @author LiuTao
		* @since 2021.09.05
		* @param void
		* @return int: 音频的播放位置
		*/
		int GetAudioCurrentTime();
		/**
		* GetAudioDuration
		* @brief 获取音频的总时长，单位：ms
		* @author LiuTao
		* @since 2021.09.05
		* @param void
		* @return int: 音频的总时长
		*/
		int GetAudioDuration();

		/**
		* Welcome
		* @brief 开启“欢迎效果”
		* @brief 每次开启软件，调用此函数以实现“欢迎功能”
		* @author LiuTao
		* @since 2021.08.28
		* @param void
		* @return void
		*/
		void Welcome();
		/**
		* WelcomeFinished
		* @brief “欢迎效果”是否结束
		* @author LiuTao
		* @since 2021.09.11
		* @param void
		* @return bool 结束则返回true，否则false
		*/
		bool IsWelcomeFinished();

		/**
		* SetMousePosition
		* 将当前鼠标空间层级坐标传入
		* @author LiuTao
		* @since 2021.07.04
		* @param vPos:			当前帧鼠标在当前空间层级下的坐标
		* @return void
		*/
		void SetMousePosition(const osg::Vec3f& vHierarchyPos);

		/**
		* GetCurrentStarWorldPos
		* 获取当前播放的音频星世界空间坐标
		* 由于本产品的特殊性，需要将银河系中心作为宇宙的中心
		* 这样就不用考虑用double表示恒星在6级空间中的位置的误差问题
		* 而且为了方便计算，只有银河系中会计算恒星，其他星系都看做一个质点
		* 规定：恒星的坐标的最小单位长度GM_STAR_POS_MIN = 1e10米（第3层跳跃到第2层时的相机半径是5e11）
		* 而且这个精度可以在银河系中放置上万亿颗恒星而没有重复感，满足了产品需求
		* @author LiuTao
		* @since 2021.07.30
		* @param void
		* @return osg::Vec3d	当前播放的音频星世界空间坐标,单位：米
		*/
		osg::Vec3d GetCurrentStarWorldPos();

		/**
		* GetGalaxyRadius
		* 获取星系半径
		* @author LiuTao
		* @since 2021.08.22
		* @param void
		* @return double		星系半径，单位：米
		*/
		double GetGalaxyRadius();

		/**
		* GetHierarchyTargetDistance
		* 获取当前层级下的目标点距离
		* @author LiuTao
		* @since 2021.10.05
		* @param void
		* @return double		目标点距离，单位：当前层级单位
		*/
		double GetHierarchyTargetDistance();

		/**
		* GetUnit
		* 获取当前“空间层级单位”，即一个单位代表多少米,初始值为1e20
		* 为了实现从 “1.0”米到“1e30”米的无缝穿梭，必须用6个double变量来记录每个空间层级的系统单位
		* 然后就可以用这些变量整体控制系统所有物体的缩放
		*
		* 每隔5个数量级，跳变一次，即缩放一次系统单位；只要6次跳变，就可以遨游整个宇宙
		* 在本系统中，宇宙分为6级空间，0级-6级，0级是人类尺度，6级是整个宇宙，级数编号越大，级数单位越大
		* 空间层级编号:	0级		1级		2级		3级		4级		5级		6级
		* 空间层级单位:	1.0		1e5		1e10	1e15	1e20	1e25	1e30
		*
		* 下面列举典型的尺度，以便用户建立宇宙尺度的概念
		* 地球直径：					1.275e7 米
		* 太阳直径：					1.392e9 米
		* 天文单位（AU）：				1.496e11 米
		* 盾牌座UY（最大恒星）直径：	2.36e12 米
		* 海王星的轨道半长轴：			5.498e12 米，30.07 AU
		* 光年（ly）：					9.461e15 米
		* 比邻星与我们的距离：			4e16 米，约为 4.22光年
		* 银河系直径：					1e21 米，约为 10万光年
		* 仙女星系与我们的距离：		2.5e22 米，约为 254万光年
		* 本星系群直径：				1e23 米，约为1000万光年
		* 室女座超星系团直径：			1e24 米，约为1.1亿光年
		* 拉尼亚凯亚超星系团直径：		5e24 米，约为5.2亿光年
		* 可观测宇宙范围：				1e27 米，约为930亿光年
		* 在这之外，就是我的世界：		1e30 米
		*
		* @author LiuTao
		* @since 2021.09.11
		* @param void					如果输入为空，则认为输入的是当前空间层级编号
		* @param iHierarchy				空间层级的层级编号
		* @return double				输入的空间层级的“空间层级单位”
		*/
		inline double GetUnit()
		{
			return m_pKernelData->fUnitArray->at(m_pKernelData->iHierarchy);
		}
		double GetUnit(int iHierarchy);

		/**
		* GetHierarchyLastEyePos
		* 获取当前空间层级跳跃前的最后时刻的眼点空间坐标
		* 为了实现从 “1.0”米到“1e30”米的无缝穿梭，必须用14个Vec3d记录眼点和目标点在7个空间层级中进出时的空间坐标
		* 然后用这些坐标整体控制当前空间层级所有物体的平移
		* 每隔5个数量级，跳变一次，即改变一次系统原点；只要6次跳变，就可以遨游整个宇宙
		* 在本系统中，宇宙分为6级空间，0级-6级，0级是人类尺度，6级是整个宇宙，级数编号越大，级数单位越大
		* @author LiuTao
		* @since 2021.09.25
		* @param iHierarchy:			层级编号，-1则表示当前层级
		* @return Vec3d：				眼点在该空间层级跳跃前的最后时刻的空间坐标，初始时全为(1,1,1)
		*/
		osg::Vec3d GetHierarchyLastEyePos(int iHierarchy = -1);

		/**
		* GetHierarchyLastTargetPos
		* 获取当前空间层级跳跃前的最后时刻的目标点空间坐标
		* @author LiuTao
		* @since 2021.09.25
		* @param iHierarchy:			层级编号，-1则表示当前层级
		* @return Vec3d:				目标点在该空间层级跳跃前的最后时刻的空间坐标，初始时全为0
		*/
		osg::Vec3d GetHierarchyLastTargetPos(int iHierarchy = -1);

		/**
		* GetHierarchy
		* 获取当前空间层级的层级编号
		* 在本系统中，宇宙分为6级空间，0级-6级，0级是人类尺度，6级是整个宇宙
		* @author LiuTao
		* @since 2021.09.12
		* @param void
		* @return int				当前空间层级的层级编号
		*/
		inline int GetHierarchy()
		{
			return m_pKernelData->iHierarchy;
		}
		/**
		* AddHierarchy
		* 将当前空间层级的层级编号+1，只能在 0-6 范围内
		* 同时，必须传入眼点在当前空间层级下的空间坐标
		* 用来计算上一层级空间的位置，以及记录以后原路返回该空间时的位置
		* 在本系统中，宇宙分为6级空间，0级-6级，0级是人类尺度，6级是整个宇宙
		* @author LiuTao
		* @since 2021.09.12
		* @param vHierarchyEyePos:		当前空间层级下，眼点在层级跃迁时的空间坐标
		* @param vHierarchyTargetPos:	当前空间层级下，目标点在层级跃迁时的空间坐标
		* @return bool					成功true，如果已到6则false
		*/
		bool AddHierarchy(const osg::Vec3d& vHierarchyEyePos, const osg::Vec3d& vHierarchyTargetPos);
		/**
		* SubHierarchy
		* 将当前空间层级的层级编号-1，只能在 0-6 范围内
		* 同时，必须传入眼点和目标点在当前空间层级下的空间坐标
		* 用来计算下一层级空间的位置，以及记录以后原路返回该空间时的位置
		* 目标点空间坐标分两种情况考虑：
		* 1.当前空间层级为0、1、2、3、4时，子空间原点就是当前空间的目标点
		* 2.当前空间层级为5、6时，子空间原点 == 当前空间原点 == (0,0,0)
		* 在本系统中，宇宙分为6级空间，0级-6级，0级是人类尺度，6级是整个宇宙
		* @author LiuTao
		* @since 2021.09.12
		* @param vHierarchyEyePos:		当前空间层级下，眼点在层级跃迁时的空间坐标
		* @param vHierarchyTargetPos:	当前空间层级下，目标点在层级跃迁时的空间坐标
		* @return bool					成功true，如果已到0则false
		*/
		bool SubHierarchy(const osg::Vec3d& vHierarchyEyePos, const osg::Vec3d& vHierarchyTargetPos);

		/**
		* Hierarchy2World
		* 层级空间坐标转世界空间坐标，由于double精度不够，会有误差
		* @author LiuTao
		* @since 2021.09.19
		* @param vHierarchy：		层级空间坐标
		* @return osg::Vec3d：		世界空间坐标
		*/
		osg::Vec3d Hierarchy2World(osg::Vec3d vHierarchy);

		/**
		* StarWorld2Hierarchy
		* 恒星的世界空间坐标转层级空间坐标，只能用于恒星坐标计算
		* 恒星的世界空间坐标最小距离为1e10,且只计算银河系内的恒星
		* 所以用double可以精确表示每颗恒星的世界空间坐标
		* @author LiuTao
		* @since 2021.09.19
		* @param vStarWorldPos：	恒星的世界空间坐标
		* @return osg::Vec3d：		层级空间坐标
		*/
		osg::Vec3d StarWorld2Hierarchy(osg::Vec3d vStarWorldPos);

		/** @brief 创建视口(QT:QWidget) */
		CGMViewWidget* CreateViewWidget(QWidget* parent);

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
		void _Next(EGMA_MODE eMode);

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
		bool _World2GalaxyCoord(const double fX, const double fY, double& fGalaxyX, double& fGalaxyY);

		/**
		* _World2GalaxyCoord
		* @brief 银河空间坐标 转 世界空间坐标
		* @author LiuTao
		* @since 2021.07.04
		* @param fGalaxyX, fGalaxyY: 银河空间坐标[-1,1]
		* @param &fX, &fY: 如果成功，则代表世界空间坐标
		* @return bool 成功true， 失败false
		*/
		bool _GalaxyCoord2World(const double fGalaxyX, const double fGalaxyY, double& fX, double& fY);

		/**
		* _StarCoord2World
		* @brief 星辰坐标 转 世界空间坐标
		* @author LiuTao
		* @since 2021.07.04
		* @param starCoord: 星辰坐标
		* @param &fX, &fY: 如果成功，则代表世界空间坐标
		* @return bool 成功true， 失败false
		*/
		bool _StarCoord2World(const SGMStarCoord& starCoord, double& fX, double& fY);

		/**
		* _UpdateScenes
		* @brief 由于空间层级变化而更新场景
		* @author LiuTao
		* @since 2021.09.15
		* @param void
		* @return void
		*/
		void _UpdateScenes();

		// 变量
	private:

		SGMKernelData*						m_pKernelData;				//!< 内核数据
		SGMConfigData*						m_pConfigData;				//!< 配置数据
		CGMDataManager*						m_pDataManager;				//!< 数据管理模块
		CGMCameraManipulator*				m_pManipulator;				//!< 相机操作器

		bool								m_bInit;					//!< 初始化标志
		bool								m_bDirty;					//!< 脏标记
		double								m_dTimeLastFrame;			//!< 上一帧时间
		float								m_fDeltaStep;				//!< 单位s
		float								m_fConstantStep;			//!< 等间隔更新的时间,单位s
		double								m_fGalaxyDiameter;			//!< 星系直径，单位：米
		CGMGalaxy*							m_pGalaxy;					//!< 星系模块
		CGMAudio*							m_pAudio;					//!< 音频模块
		EGMA_MODE							m_ePlayMode;				//!< 当前播放模式

		std::default_random_engine			m_iRandom;
	};
}	// GM