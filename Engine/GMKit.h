//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMKit.h
/// @brief		Galaxy-Music Engine - GMGalaxy
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.05
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMCommon.h"
#include <osg/StateSet>

namespace GM
{
	/*************************************************************************
	Class
	*************************************************************************/
	/*!
	*  @Class CGMKit
	*  @brief toolkits for Galaxy-Music
	*/
	class CGMKit
	{
	// 函数
	public:
		/** @brief 构造 */
		CGMKit() {}

		/**
		* LoadShader
		* 加载shader
		* @author LiuTao
		* @since 2020.12.05
		* @param pStateSet:			需要加shader的状态集指针
		* @param vertFilePath:		顶点shader路径
		* @param fragFilePath:		片元shader路径
		* @param geomFilePath:		几何shader路径
		* @param strShaderName:		shader的名称
		* @param bPixelLighting:	是否逐像素光照（如果是，则要处理切向量）
		* @return bool:				成功为true，否则false
		*/
		static bool LoadShader(
			osg::StateSet* pStateSet,
			const std::string& vertFilePath,
			const std::string& fragFilePath,
			const std::string& geomFilePath,
			const std::string& strShaderName,
			bool bPixelLighting = false);

		/*!
		* @brief 加载Shader by StateSet
		* @param pStateSet:				osg::StateSet
		* @param vertFilePath:			the vertex glsl file path
		* @param fragFilePath:			the fragment glsl file path
		* @param fragCommonFilePath:	the optional common fragment file path
		* @param shaderName:			name of shader
		* @param bPixelLighting:		be pixel lighting(for tangent & binormal)
		* @return bool:					成功为true，否则false
		*/
		static bool LoadShaderWithCommonFrag(
			osg::StateSet* pStateSet
			, const std::string& vertFilePath
			, const std::string& fragFilePath
			, const std::string& fragCommonFilePath
			, const std::string& shaderName
			, const bool bPixelLighting = false);

		/**
		* 加载shader
		* @param pStateSet:			需要加shader的状态集指针
		* @param vertFilePath:		顶点shader路径
		* @param fragFilePath:		片元shader路径
		* @param strShaderName:		shader的名称
		* @return bool:				成功为true，否则false
		*/
		static bool LoadShader(
			osg::StateSet* pStateSet,
			const std::string& vertFilePath,
			const std::string& fragFilePath,
			const std::string& strShaderName);

		/**
		* Load compute shader by stateSet
		* @author LiuTao
		* @since 2022.07.25
		* @param pStateSet:			需要加shader的状态集指针
		* @param compFilePath:		compute shader路径
		* @param shaderName:		shader的名称
		* @return bool:				成功为true，否则false
		*/
		static bool LoadComputeShader(
			osg::StateSet* pStateSet,
			const std::string& compFilePath,
			const std::string& shaderName = "");

		/**
		* 向状态集中添加纹理
		* @author LiuTao
		* @since 2022.07.29
		* @param pStateSet:			需要加shader的状态集指针
		* @param pTex:				需要添加的纹理指针
		* @param texName:			纹理在glsl中的名称
		* @param iUnit:				纹理单元
		* @return bool:				成功为true，否则false
		*/
		static bool AddTexture(osg::StateSet* pStateSet, osg::Texture* pTex, const char* texName, const int iUnit);

		/**
		* @brief 获取图片的RGBA通道值
		* @param pImg:		图片指针
		* @param fX:		图像x坐标,[0,1]
		* @param fY:		图像y坐标,[0,1]
		* @param bLinear:	是否双线性插值，true = 双线性，false = 临近值
		* @return Vec4f：	RGBA通道值,[0.0,1.0]
		*/
		static osg::Vec4f GetImageColor(
			const osg::Image* pImg,
			const float fX, const float fY,
			const bool bLinear = false);

		/**
		* @brief 16F 转 32F
		* @param x:			16F
		* @return float：	32F
		*/
		static float Half_2_Float(const unsigned short x);
		/**
		* @brief 32F 转 16F 
		* @param x:					32F
		* @return unsigned short：	16F
		*/
		static unsigned short Float_2_Half(const float x);

		/**
		* @brief 混合函数,参考 glsl 中的 mix(a,b,x)
		* @param fA, fB:				范围
		* @param fX:					混合系数
		* @return 各种向量:				混合后的值
		*/
		inline static float Mix(const float fA, const float fB, const float fX)
		{
			return fA * (1 - fX) + fB * fX;
		}
		inline static double Mix(const double fA, const double fB, const double fX)
		{
			return fA * (1 - fX) + fB * fX;
		}
		inline static osg::Vec4f Mix(const osg::Vec4f vA, const osg::Vec4f vB, const double fX)
		{
			return vA * (1 - fX) + vB * fX;
		}

	private:
		/** Replaces all the instances of "sub" with "other" in "s". */
		static std::string& _ReplaceIn(std::string& s, const std::string& sub, const std::string& other);
		static std::string _ReadShaderFile(const std::string& filePath);

		inline static unsigned int AsUint(const float x) { return *(unsigned int*)&x; }
		inline static float AsFloat(const unsigned int x) { return *(float*)&x; }
	};

}	// GM
