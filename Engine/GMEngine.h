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
	class CGMCommonUniform;
	class CGMDataManager;
	class CGMGalaxy;
	class CGMAudio;
	class CGMPost;
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
		/** @brief 保存太阳系此刻的信息 */
		bool SaveSolarData();
		/**
		* 修改屏幕尺寸时调用此函数
		* @param iW: 屏幕宽度
		* @param iH: 屏幕高度
		*/
		void ResizeScreen(const int iW, const int iH);

		/** @brief 开启/关闭编辑模式 */
		void SetEditMode(const bool bEnable);
		/** @brief 获取编辑模式，是或否 */
		bool GetEditMode() const;
		/** @brief 开启/关闭抓手的悬浮状态 */
		void SetHandleHover(const bool bHover);
		/** @brief 开启/关闭捕获功能 */
		void SetCapture(const bool bEnable);

		/**
		* @brief 根据坐标选择需要操作的音频
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
		/** @brief 查询上一首，但有代价：回到过去观察历史信息，未来便会改变 */
		bool Last();
		/** @brief 下一首 */
		bool Next();
		/** @brief 设置音量，0.0-1.0 */
		bool SetVolume(const float fVolume);
		/** @brief 获取音量，0.0-1.0 */
		float GetVolume() const;
		/**
		* @brief 设置播放模式
		* @param eMode:			播放模式（单曲循环、随机播放、列表循环等）
		* @return bool：		成功true， 失败false
		*/
		bool SetPlayMode(EGMA_MODE eMode);
		/**
		* @brief 获取播放模式	
		* @return EGMA_MODE：	播放模式（单曲循环、随机播放、列表循环等）
		*/
		inline EGMA_MODE GetPlayMode() const
		{
			return m_ePlayMode;
		}

		/**
		* @brief 获取当前音频文件名称
		* @return std::wstring 当前播放的音频文件名称，含后缀名，未播放则返回 L""
		*/
		std::wstring GetAudioName() const;

		/**
		* @brief 设置音频的播放位置，单位：ms
		* @param iTime: 音频的播放位置
		* @return bool 成功true， 失败false
		*/
		bool SetAudioCurrentTime(const int iTime);
		/**
		* @brief 获取音频的播放位置，单位：ms
		* @return int: 音频的播放位置
		*/
		int GetAudioCurrentTime() const;

		/**
		* @brief 获取音频的总时长，单位：ms
		* @return int: 音频的总时长
		*/
		int GetAudioDuration() const;

		/**
		* @brief 开启“欢迎效果”
		* 每次开启软件，调用此函数以实现“欢迎功能”
		*/
		void Welcome();
		/**
		* @brief “欢迎效果”是否结束
		* @return bool 结束则返回true，否则false
		*/
		bool IsWelcomeFinished() const;

		/**
		* @brief 获取音频播放的顺序列表，back位置为最新的音频
		* @return std::vector<std::wstring>：	音频播放的顺序列表
		*/
		const std::vector<std::wstring> GetPlayingOrder() const;

		/**
		* @brief 将当前鼠标空间层级坐标传入
		* @param vHiePos:		当前帧鼠标在当前空间层级下的坐标
		* @return void
		*/
		void SetMousePosition(const SGMVector3& vHiePos);

		/**
		* @brief 设置当前播放的恒星显示的音频空间坐标
		* 注意：不等于 设置恒星数据库中的坐标数据
		* @param vAudioCoord:	当前播放的恒星的音频空间坐标
		* @return void
		*/
		void SetCurrentStarAudioCoord(const SGMAudioCoord& vAudioCoord);

		/**
		* @brief 获取当前播放的恒星显示的音频空间坐标
		* 注意：不一定等于恒星数据库中的坐标数据，有可能正在修改
		* @return SGMAudioCoord:	当前播放的恒星的音频空间坐标
		*/
		SGMAudioCoord GetCurrentStarAudioCoord() const;

		/**
		* @brief 获取当前播放的音频星世界空间坐标
		* 由于本产品的特殊性，需要将银河系中心作为宇宙的中心
		* 这样就不用考虑恒星在6级空间中的位置的误差问题(由double精度不足导致)
		* 而且为了方便计算，只有银河系中会绘制恒星，其他星系都看做一个质点
		* @return SGMVector3	当前播放的音频星世界空间坐标,单位：米
		*/
		SGMVector3 GetCurrentStarWorldPos() const;

		/**
		* @brief 获取指定音频的音频空间坐标
		* 注意：不一定等于恒星数据库中的坐标数据，有可能正在修改
		* @param strName：			音频文件名
		* @return SGMAudioCoord:	音频空间坐标
		*/
		SGMAudioCoord GetAudioCoord(const std::wstring& strName) const;

		/**
		* @brief 获取星系半径
		* @return double		星系半径，单位：米
		*/
		inline double GetGalaxyRadius() const
		{
			return m_fGalaxyDiameter * 0.5;
		}

		/**
		* @brief 获取当前层级下的目标点距离
		* @return double		目标点距离，单位：当前层级单位
		*/
		double GetHierarchyTargetDistance() const;

		/**
		* @brief 获取当前“空间层级单位”，即一个单位代表多少米,初始值为1e20
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
		* @param void					如果输入为空，则认为输入的是当前空间层级编号
		* @param iHierarchy				空间层级的层级编号
		* @return double				输入的空间层级的“空间层级单位”
		*/
		inline double GetUnit() const
		{
			return m_pKernelData->fUnitArray->at(m_pKernelData->iHierarchy);
		}
		double GetUnit(const int iHierarchy) const;

		/**
		* @brief 获取小一级的空间层级“上方向向量”在当前空间层级坐标系下的向量值
		* @param vX,vY,vZ:			归一化的X/Y/Z单位向量
		* @return bool:				成功true，失败false */
		bool GetSmallerHierarchyCoord(SGMVector3& vX, SGMVector3& vY, SGMVector3& vZ) const;
		/**
		* 获取大一级的空间层级“上方向向量”在当前空间层级坐标系下的向量值
		* @param vX,vY,vZ:			归一化的X/Y/Z单位向量
		* @return bool:				成功true，失败false */
		bool GetBiggerHierarchyCoord(SGMVector3& vX, SGMVector3& vY, SGMVector3& vZ) const;
		/**
		* @brief 获取当前空间层级的层级编号
		* 在本系统中，宇宙分为6级空间，0级-6级，0级是人类尺度，6级是整个宇宙
		* @return int				当前空间层级的层级编号
		*/
		inline int GetHierarchy() const
		{
			return m_pKernelData->iHierarchy;
		}
		/**
		* @brief 将当前空间层级的层级编号+1，只能在 0-6 范围内
		* 同时，必须传入眼点在当前空间层级下的空间坐标
		* 用来计算升一级空间的位置，以及记录以后返回该空间时的位置
		* 在本系统中，宇宙分为6级空间，0级-6级，0级是人类尺度，6级是整个宇宙
		* @param vHierarchyEyePos:		当前空间层级下，眼点在层级跃迁时的空间坐标
		* @param vHierarchyTargetPos:	当前空间层级下，目标点在层级跃迁时的空间坐标
		* @return bool					成功true，如果已到6则false
		*/
		bool AddHierarchy(const SGMVector3& vHierarchyEyePos, const SGMVector3& vHierarchyTargetPos);
		/**
		* @brief 将当前空间层级的层级编号-1，只能在 0-6 范围内
		* 同时，必须传入眼点和目标点在当前空间层级下的空间坐标
		* 用来计算降一级空间的位置，以及记录以后返回该空间时的位置
		* 目标点空间坐标分多种情况考虑：
		* 1.当前空间层级为0、1时 to do
		* 2.当前空间层级为2时，子空间原点就是当前最近天体的坐标
		* 3.当前空间层级为4时，子空间原点就是当前最近恒星的坐标
		* 4.当前空间层级为3、5、6时，子空间原点 == 当前空间原点 == (0,0,0)
		* 在本系统中，宇宙分为6级空间，0级-6级，0级是人类尺度，6级是整个宇宙
		* @param vHierarchyEyePos:		当前空间层级下，眼点在层级跃迁时的空间坐标
		* @param vHierarchyTargetPos:	当前空间层级下，目标点在层级跃迁时的空间坐标
		* @return bool					成功true，如果已到0则false
		*/
		bool SubHierarchy(const SGMVector3& vHierarchyEyePos, const SGMVector3& vHierarchyTargetPos);
		/**
		* @brief 获取任意向量在升级后空间下的向量
		* @param vBefore:			该向量在升级前的空间下的值
		* @return SGMVector4:		输入的向量在升级后空间下的值
		*/
		SGMVector3 AfterAddHierarchy(const SGMVector4& vBefore) const;
		/**
		* @brief 获取任意向量在降级后空间下的向量
		* @param vBefore:		该向量在降级前的空间下的值
		* @return SGMVector4:		输入的向量在降级后空间下的值
		*/
		SGMVector3 AfterSubHierarchy(const SGMVector4& vBefore) const;
		/**
		* @brief 获取查询位置附近最近的一颗天体（行星或恒星）的位置
		* @param vSearchHiePos 查询位置，当前空间层级坐标系
		* @param vPlanetHiePos 返回最近的天体位置，当前空间层级坐标系
		* @param fOrbitalPeriod 返回此行星公转轨道周期，单位：秒
		* @return bool 第3层级空间下，点击位置附近有天体则true，否则false
		*/
		bool GetNearestCelestialBody(const SGMVector3& vSearchHiePos,
			SGMVector3& vPlanetHiePos, double& fOrbitalPeriod) const;
		/**
		* @brief 获取目标点附近最近的天体（行星或恒星）的位置
		* @param vPlanetPos 返回最近的天体位置，单位：米
		* @param fOrbitalPeriod 返回此行星公转轨道周期，单位：秒
		*/
		void GetCelestialBody(SGMVector3& vPlanetPos, double& fOrbitalPeriod) const;

		/**
		* @brief 获取目标点附近最近的天体（行星或恒星）的平均半径
		* @return double 天体平均半径，单位：米
		*/
		double GetCelestialMeanRadius() const;
		/**
		* @brief 获取目标点附近最近的天体（行星或恒星）的指定纬度的海平面与球心距离
		* @param fLatitude 纬度，单位：°，范围：[-90.0, 90.0]
		* @return double 指定纬度的海平面与球心距离，单位：米
		*/
		double GetCelestialRadius(const double fLatitude) const;
		/**
		* @brief 获取目标点附近最近的天体（行星或恒星）的北极轴（当前层级空间下）
		* @return SGMVector3 北极轴（当前层级空间下）
		*/
		SGMVector3 GetCelestialNorth() const;

		/**
		* @brief 获取目标点附近最近的天体（行星或恒星）在本恒星系中的索引
		* @return unsigned int 0代表恒星，1代表公转轨道半径最小的天体
		*/
		unsigned int GetCelestialIndex() const;

		/**
		* @brief 获取最近的恒星在银河系坐标系（4级空间的世界坐标系）的位置
		* @author LiuTao
		* @since 2023.01.16
		* @param void:
		* @return SGMVector3:		最近的恒星在银河系坐标系（4级空间的世界坐标系）的位置
		*/
		SGMVector3 GetNearStarWorldPos() const;

		/**
		* Angle2Color const
		* 星音乐的“情绪角度”转“颜色”。
		* @author LiuTao
		* @since 2022.09.04
		* @param fEmotionAngle：	情绪角度，[0.0,2*PI)
		* @return SGMVector4f：		星音乐类型对应的颜色,[0.0,1.0]
		*/
		SGMVector4f Angle2Color(const float fEmotionAngle) const;

		/** @brief 创建视口(QT:QWidget) */
		CGMViewWidget* CreateViewWidget(QWidget* parent);

	private:
		/**
		* @brief 加载配置
		*/
		bool _LoadConfig();
		/**
		* @brief 初始化背景相关节点
		*/
		void _InitBackground();
		/**
		* @brief 初始化前景相关节点
		*/
		void _InitForeground();
		/**
		* @brief 播放下一首
		*/
		void _Next(const EGMA_MODE eMode);
		/**
		* @brief 间隔更新，一秒钟更新10次
		* @param updateStep 两次间隔更新的时间差，单位s
		*/
		void _InnerUpdate(const float updateStep);
		/** @brief 更新(在主相机更新姿态之后) */
		bool _UpdateLater(const double dDeltaTime);
		/**
		* @brief 世界空间坐标 转 银河空间坐标
		* @param fX, fY: 世界空间坐标
		* @param &fGalaxyX, &fGalaxyY: 银河空间坐标[-1,1]
		* @return bool 成功true， 失败false
		*/
		bool _World2GalaxyCoord(const double fX, const double fY, double& fGalaxyX, double& fGalaxyY) const;
		/**
		* @brief 银河空间坐标 转 世界空间坐标
		* @param fGalaxyX, fGalaxyY: 银河空间坐标[-1,1]
		* @param &fX, &fY: 如果成功，则代表世界空间坐标
		* @return bool 成功true， 失败false
		*/
		bool _GalaxyCoord2World(const double fGalaxyX, const double fGalaxyY, double& fX, double& fY) const;

		/**
		* @brief 获取任意向量在升级后空间下的向量
		* @param vBefore:			该向量在升级前的空间下的值
		* @return osg::Vec3d:		输入的点在升级后空间下的值
		*/
		osg::Vec3d _AfterAddHierarchy(const osg::Vec4d& vBefore) const;
		/**
		* @brief 获取任意向量在降级后空间下的向量
		* @param vBefore:			该向量在降级前的空间下的值
		* @return osg::Vec3d:		输入的向量在降级后空间下的值
		*/
		osg::Vec3d _AfterSubHierarchy(const osg::Vec4d& vBefore) const;
		/**
		* @brief 由于空间层级变化而更新场景
		*/
		void _UpdateScenes();
		/**
		* @brief 更新相机远近截面
		*/
		void _UpdateNearFar();
		/**
		* @brief 混合函数,参考 glsl 中的 mix(a,b,x)
		* @param fMin, fMax:			范围
		* @param fX:					混合系数
		* @return double:				混合后的值
		*/
		inline double _Mix(const double fMin, const double fMax, const double fX)
		{
			return fMin * (1 - fX) + fMax * fX;
		}
		/**
		* @brief SGMVector3 转 osg::Vec3d
		* @param vGM:				输入的GM向量
		* @return osg::Vec3d:		输出的osg向量 */
		inline SGMVector3 _OSG2GM(const osg::Vec3d& vOSG) const
		{
			return SGMVector3(vOSG.x(), vOSG.y(), vOSG.z());
		}

		// 变量
	private:

		SGMKernelData*						m_pKernelData;				//!< 内核数据
		SGMConfigData*						m_pConfigData;				//!< 配置数据
		CGMCommonUniform*					m_pCommonUniform;			//!< 公用Uniform
		CGMDataManager*						m_pDataManager;				//!< 数据管理模块
		CGMCameraManipulator*				m_pManipulator;				//!< 相机操作器

		bool								m_bInit;					//!< 初始化标志
		double								m_dTimeLastFrame;			//!< 上一帧时间
		float								m_fDeltaStep;				//!< 单位s
		float								m_fConstantStep;			//!< 等间隔更新的时间,单位s
		double								m_fGalaxyDiameter;			//!< 星系直径，单位：米
		CGMGalaxy*							m_pGalaxy;					//!< 星系模块
		CGMAudio*							m_pAudio;					//!< 音频模块
		CGMPost*							m_pPost;					//!< 后期模块

		EGMA_MODE							m_ePlayMode;				//!< 当前播放模式
		std::default_random_engine			m_iRandom;

		osg::ref_ptr<osg::Texture2D>		m_pSceneTex;				//!< 主场景颜色图
		osg::ref_ptr<osg::Texture2D>		m_pBackgroundTex;			//!< 背景颜色图
		osg::ref_ptr<osg::Texture2D>		m_pForegroundTex;			//!< 前景颜色图
	};
}	// GM