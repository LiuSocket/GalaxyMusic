//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMGalaxy.h
/// @brief		Galaxy-Music Engine - GMGalaxy
/// @version	1.0
/// @author		LiuTao
/// @date		2020.11.27
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMNebula.h"
#include <random>
#include <osg/Node>
#include <osg/Texture>

namespace GM
{
	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/

	/*!
	*  @class CGMGalaxy
	*  @brief Galaxy-Music GMGalaxy
	*/
	class CGMGalaxy : public CGMNebula
	{
		// 函数
	public:
		/** @brief 构造 */
		CGMGalaxy();

		/** @brief 析构 */
		~CGMGalaxy();

		/** @brief 初始化 */
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData);
		/** @brief 更新 */
		bool Update(double dDeltaTime);
		/** @brief 更新(在主相机更新姿态之后) */
		bool UpdateLater(double dDeltaTime);
		/** @brief 加载 */
		bool Load();

		/** @brief 开启/关闭编辑模式 */
		void SetEditMode(const bool bEnable);
		/** @brief 开启/关闭捕获功能 */
		void SetCapture(const bool bEnable);

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
		* CreateGalaxy
		* 创建星系
		* @author LiuTao
		* @since 2020.11.27
		* @param fDiameter:	星系直径
		* @return bool 成功true， 失败false
		*/
		bool CreateGalaxy(float fDiameter);

		/**
		* SetMousePosition
		* 将当前鼠标世界空间坐标传入
		* @author LiuTao
		* @since 2021.07.04
		* @param vPos:			当前帧鼠标世界空间坐标
		* @return void
		*/
		void SetMousePosition(const osg::Vec3f& vPos);

		/**
		* SetCurrentStar
		* 设置当前激活的恒星
		* @author LiuTao
		* @since 2021.07.04
		* @param vPos:			当前激活的恒星的世界空间坐标
		* @param wstrName：		当前播放的音频名称，XXX.mp3
		* @return void
		*/
		void SetCurrentStar(const osg::Vec3f& vPos, const std::wstring& wstrName);

		/**
		* SetAudioLevel
		* 设置当前帧音频的振幅值
		* @author LiuTao
		* @since 2021.07.04
		* @param fLevel 振幅值 [0.0f,1.0f]
		* @return void
		*/
		void SetAudioLevel(float fLevel);

		/**
		* GetStarWorldPos
		* 获取当前播放的音频星世界空间坐标
		* @author LiuTao
		* @since 2021.07.30
		* @param void
		* @return osg::Vec3d	当前播放的音频星世界空间坐标
		*/
		osg::Vec3d GetStarWorldPos();

	private:
		/**
		* _CreateAudioPoints
		* 创建音频星点
		* @author LiuTao
		* @since 2020.12.05
		* @return bool:			成功true，失败false
		*/
		bool _CreateAudioPoints();

		/**
		* _CreateGalaxyPoints
		* 创建PointSprite星系
		* @author LiuTao
		* @since 2020.12.05
		* @return bool:			成功true，失败false
		*/
		bool _CreateGalaxyPoints();

		/**
		* _CreateGalaxyPlane
		* 创建平面星系，用于远处
		* @author LiuTao
		* @since 2020.12.05
		* @return bool:			成功true，失败false
		*/
		bool _CreateGalaxyPlane();

		/**
		* _DetachAudioPoints
		* 将输入的音频星点分离
		* @author LiuTao
		* @since 2021.07.11
		* @return bool:				成功true，失败false
		*/
		bool _DetachAudioPoints();

		/**
		* _AttachAudioPoints
		* 合并当前编辑的音频星点
		* @author LiuTao
		* @since 2021.07.11
		* @return bool:				成功true，失败false
		*/
		bool _AttachAudioPoints();

		/**
		* _CreateAudioGeometry
		* 读取数据并创建未激活状态的音频星几何体
		* @author LiuTao
		* @since 2021.07.11
		* @param coordVector：		音频星点星辰坐标Vector
		* @return Geometry*:		创建的几何体节点指针，未成功则返回nullptr
		*/
		osg::Geometry* _CreateAudioGeometry(std::vector<SGMStarCoord>& coordVector);

		/**
		* _CreateConeGeometry
		* 创建圆锥体
		* @author LiuTao
		* @since 2021.07.18
		* @param void：				无
		* @return Geometry*:		创建的几何体节点指针，未成功则返回nullptr
		*/
		osg::Geometry* _CreateConeGeometry();

		/**
		* _CreateHelpLineGeometry
		* 创建辅助线,辅助修改音频坐标
		* @author LiuTao
		* @since 2021.07.24
		* @param void：				无
		* @return Geometry*:		创建的几何体节点指针，未成功则返回nullptr
		*/
		osg::Geometry* _CreateHelpLineGeometry();

		/**
		* _GetGalaxyValue
		* 获取星系图的RGBA通道值，channel为0/1/2/3，对应RGBA。
		* @author LiuTao
		* @since 2020.11.27
		* @param fX:		图像x坐标,[0,1]
		* @param fY:		图像y坐标,[0,1]
		* @param iChannel:	0、1、2、3，对应R、G、B、A通道
		* @param bLinear:	是否双线性插值，true = 双线性，false = 临近值
		* @return float [0.0,1.0]
		*/
		float _GetGalaxyValue(float fX, float fY, int iChannel, bool bLinear = false);

		/**
		* 加载2D纹理
		* @author LiuTao
		* @since 2020.06.16
		* @param fileName: 图片文件路径
		* @param iChannelNum: 1、2、3、4分别代表R、RG、RGB、RGBA
		* @return osg::Texture* 返回纹理指针
		*/
		osg::Texture* _CreateTexture2D(const std::string& fileName, const int iChannelNum);

		// 变量
	private:

		float											m_fGalaxyRadius;				//!< 星系半径
		/** 星系数据图片：
		** RGB通道		颜色
		** Alpha通道	密度
		*/
		osg::ref_ptr<osg::Image>						m_pGalaxyImage;
		std::default_random_engine						m_iRandom;

		std::string										m_strGalaxyShaderPath;			//!< galaxy shader 着色器路径
		std::string										m_strGalaxyTexPath;				//!< galaxy texture 贴图路径
		std::wstring									m_strPlayingStarName;			//!< 激活的音频星文件名,XXX.mp3
		osg::ref_ptr<osg::Transform>					m_pPlayingStarTransform;		//!< 激活的音频星的变换结点
		SGMAudioCoord									m_vPlayingAudioCoord;			//!< 激活的音频星的音频空间坐标
		osg::Vec3d										m_vPlayingStarWorldPos;			//!< 激活的音频星世界空间坐标
		osg::ref_ptr<osg::Geode>						m_pGeodeHelpLine;				//!< 辅助修改音频坐标的线Geode	
		osg::ref_ptr<osg::Geode>						m_pGeodeAudio;					//!< 未激活的音频星Geode					
		osg::ref_ptr<osg::StateSet>						m_pStateSetGalaxy;				//!< 点模式星系的状态集
		osg::ref_ptr<osg::StateSet>						m_pStateSetPlayingAudio;		//!< 激活的音频星状态集
		osg::ref_ptr<osg::StateSet>						m_pStateSetPlane;				//!< 银河盘面的状态集
		osg::ref_ptr<osg::Uniform>						m_pMousePosUniform;				//!< 鼠标的世界空间坐标Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarWorldPosUniform;			//!< 激活的音频星世界坐标Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarAudioPosUniform;			//!< 当前激活的音频空间坐标Uniform
		osg::ref_ptr<osg::Uniform>						m_pTimesUniform;				//!< 时间Uniform
		osg::ref_ptr<osg::Uniform>						m_pStarColorUniform;			//!< 当前颜色Uniform
		osg::ref_ptr<osg::Uniform>						m_pLevelArrayUniform;			//!< 振幅数组Uniform
		bool											m_bCapture;						//!< 是否开启捕捉模式
		bool											m_bWelcome;						//!< 是否启用欢迎功能
		double											m_fWelcomeTime;					//!< “欢迎状态”持续了多长时间
	};
}	// GM