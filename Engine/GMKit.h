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
		* @param bPixelLighting:	是否逐像素光照（如果是，则要处理切向量）
		* @return bool:				成功为true，否则false
		*/
		static bool LoadShader(
			osg::StateSet* pStateSet
			, std::string& vertFilePath
			, std::string& fragFilePath
			, std::string& geomFilePath
			, bool bPixelLighting = false);

		/**
		* LoadShader
		* 加载shader
		* @author LiuTao
		* @since 2020.12.05
		* @param pStateSet:			需要加shader的状态集指针
		* @param vertFilePath:		顶点shader路径
		* @param fragFilePath:		片元shader路径
		* @return bool:				成功为true，否则false
		*/
		static bool LoadShader(
			osg::StateSet* pStateSet
			, std::string& vertFilePath
			, std::string& fragFilePath);

	private:
		/** Replaces all the instances of "sub" with "other" in "s". */
		static std::string& _ReplaceIn(std::string& s, const std::string& sub, const std::string& other);
		static std::string _ReadShaderFile(std::string& filePath);
	};

}	// GM
