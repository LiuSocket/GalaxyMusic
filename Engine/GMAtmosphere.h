//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMAtmosphere.h
/// @brief		Galaxy-Music Engine - GMAtmosphere
/// @version	1.0
/// @author		LiuTao
/// @date		2023.11.30
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommon.h"
#include "GMCommonUniform.h"
#include "GMKernel.h"
#include "GMDispatchCompute.h"

#include <random>
#include <osg/Node>
#include <osg/Texture>

namespace GM
{
	/*************************************************************************
	Macro Defines
	*************************************************************************/

	/*************************************************************************
	constexpr
	*************************************************************************/

	constexpr double ATMOS_FADE_R = 5.802e-6;		// 大气的红光散射系数
	constexpr double ATMOS_FADE_G = 1.3558e-5; 		// 大气的绿光散射系数
	constexpr double ATMOS_FADE_B = 3.31e-5; 		// 大气的蓝光散射系数

	constexpr double ATMOS_RAYLEIGH_H = 0.132; 		// 大气的瑞丽散射标高比例
	constexpr double ATMOS_MIE_H = 0.019; 			// 大气的米氏散射标高比例
	constexpr int ATMOS_MIN = 16;					// 最小的大气厚度，单位：km

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
	*  @class CGMAtmosphere
	*  @brief Galaxy-Music CGMAtmosphere
	*/
	class CGMAtmosphere
	{
		// 函数
	public:
		/** @brief 构造 */
		CGMAtmosphere();

		/** @brief 析构 */
		~CGMAtmosphere();

		/** @brief 初始化 */
		bool Init(SGMConfigData* pConfigData);

		/**
		* @brief 根据“大气厚度”和“星球半径”，获取对应的散射纹理
		* @param fAtmosH:			大气厚度，单位：米
		* @param fRadius:			星球半径，单位：米
		* @return osg::Texture3D*:	对应的散射纹理
		*/
		osg::Texture3D* GetInscattering(const EGMAtmosHeight eAtmosH, const double& fRadius);

		/**
		* @brief 根据“大气厚度”枚举值计算大气厚度
		* @param eAtmosHeight:		大气厚度枚举
		* @return float:			大气厚度,单位：米
		*/
		inline float GetAtmosHeight(const EGMAtmosHeight eAtmosHeight) const
		{
			return ATMOS_MIN * 1e3 * exp2(int(eAtmosHeight) - 1);
		};

		/**
		* @brief 根据“大气总厚度”和“星球半径”，计算有光区域的"DotUL"最小值，dot(upDir, lightDir)
		* @param fAtmosThick:		大气总厚度，单位：米
		* @param fRadius:			星球半径，单位：米
		* @return float:			有光区域的"DotUL"最小值,范围：(-1.0f, 0.0f)
		*/
		inline float GetMinDotUL(const double& fAtmosThick, const double& fRadius) const
		{
			float fSinUL = fRadius / (fAtmosThick + fRadius);
			return -sqrt(std::fmax(0.0f, 1.0f - fSinUL * fSinUL));
		};

	private:

		/**
		* @brief 生成大气“透过率”纹理
		*/
		void _MakeAtmosTransmittance();

		/**
		* @brief 生成大气“辐照度”纹理
		*/
		void _MakeAtmosIrradiance();

		/**
		* @brief 生成大气“内散射”纹理
		*/
		void _MakeAtmosInscattering();

		/**
		* @brief 计算大气“透过率”
		*/
		osg::Vec3d _Transmittance(const double& fAtmosDens,
			const double& fR, const double& fAtmosThick,
			const osg::Vec2d& vP0, const osg::Vec2d& vP1);

		/**
		* @brief 根据“海拔高度”和“大气总厚度”，计算该位置的瑞丽散射系数
		* @param fAlt:				海拔高度，单位：米
		* @param fAtmosThick:		大气总厚度，单位：米
		* @return osg::Vec3d:		该位置的瑞丽散射系数,(0,1]
		*/
		inline osg::Vec3d _RayleighCoefficient(const double& fAlt, const double& fAtmosThick) const
		{
			// 大气密度随高度衰减，其中地球大气瑞丽散射的标高：8500m
			double fEarthH = fAtmosThick * ATMOS_RAYLEIGH_H;
			return osg::Vec3d(ATMOS_FADE_R, ATMOS_FADE_G, ATMOS_FADE_B) * exp2(-std::fmax(0, fAlt) / fEarthH);
		};
		/**
		* @brief 根据“海拔高度”和“大气总厚度”，计算该位置的米氏散射系数
		* @param fAlt:				海拔高度，单位：米
		* @param fAtmosThick:		大气总厚度，单位：米
		* @return double:			该位置的米氏散射系数,(0,1]
		*/
		inline double _MieCoefficient(const double& fAlt, const double& fAtmosThick) const
		{
			// 大气密度随高度衰减，其中地球大气米氏散射的标高：1200m
			double fEarthH = fAtmosThick * ATMOS_MIE_H;
			return 3.996e-6 * exp2(-std::fmax(0, fAlt) / fEarthH);
		};

		/**
		* @brief 瑞丽散射相位函数
		* @param fCosVL:		viewDir与LightSource的夹角余弦值
		* @return double:		瑞丽散射相位函数
		*/
		inline double _RayleighPhase(const double& fCosVL) const
		{
			return 3.0 / (16 * osg::PI) * (1 + fCosVL * fCosVL);
		};
		/**
		* @brief 米氏散射相位函数
		* @param fCosVL:		viewDir与LightSource的夹角余弦值
		* @return double:		米氏散射相位函数
		*/
		inline double _MiePhase(const double& fCosVL) const
		{
			constexpr double g = 0.8;
			constexpr double g2 = g * g;
			const double a = 3.0 / (8 * osg::PI);
			constexpr double b = (1 - g2) / (2 + g2);
			double c = 1.0 + fCosVL * fCosVL;
			double d = pow(1 + g2 - 2 * g * fCosVL, 1.5);
			return a * b * (c / d);
		};

		/**
		* @brief 米氏散射导致的吸收
		* @param fAlt:				海拔高度，单位：米
		* @param fAtmosThick:		大气总厚度，单位：米
		* @return osg::Vec3d:		米氏散射导致的吸收比例
		*/
		inline osg::Vec3d _MieAbsorption(const double& fAlt, const double& fAtmosThick) const
		{
			// 大气密度随高度衰减，其中地球大气米氏散射的标高：1200m
			double fEarthH = fAtmosThick * ATMOS_MIE_H;
			double fMie = 4.4e-6 * exp2(-std::fmax(0, fAlt) / fEarthH);
			return osg::Vec3d(fMie, fMie, fMie);
		};
		/**
		* @brief 臭氧层的吸收
		* @param fAlt:				海拔高度，单位：米
		* @param fAtmosThick:		大气总厚度，单位：米
		* @return osg::Vec3d:		臭氧层的吸收比例
		*/
		inline osg::Vec3d _OzoneAbsorption(const double& fAlt, const double& fAtmosThick) const
		{
			// 空气中的臭氧（ozone）也会参与光线的吸收，臭氧对不同波长的光有着不同的吸收效率，但臭氧对散射没有贡献
			// 臭氧层是一个特定高度的层，可以通过“中心高度”和“半厚度”两个参数来描述，地球大气中分别取 25km 和 15km
			// 其他星球大气太复杂，且没人亲眼看过，所以就不考虑那些因素，当做和地球大气一样，只是最终颜色有变化
			double fCenterHeight = fAtmosThick * 0.39;
			double fHalfThick = fAtmosThick * 0.234;
			return osg::Vec3d(0.65e-6, 1.881e-6, 0.085e-6) * std::fmax(0, 1- std::abs(fAlt- fCenterHeight) / fHalfThick);
		};

		/**
		* @brief 根据“大气总厚度”，计算地表的相对大气密度，规定地球海平面大气密度为1
		* @param fAtmosThick:		大气总厚度，单位：米
		* @return double:			地表的相对大气密度
		*/
		inline double _GetAtmosBottomDens(const double& fAtmosThick) const
		{
			// 保证地球海平面大气密度为1
			return fAtmosThick / 64000.0;
		};

		// 变量
	private:
		SGMConfigData*									m_pConfigData;					//!< 配置数据

		std::default_random_engine						m_iRandom;						//!< 随机值
		std::string										m_strCoreModelPath;				//!< 核心模型资源路径
		std::vector<osg::ref_ptr<osg::Texture3D>>		m_pInscatteringTexVector;		//!< 内散射纹理数组
	};
}	// GM