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
	// ����
	public:
		/** @brief ���� */
		CGMKit() {}

		/**
		* LoadShader
		* ����shader
		* @author LiuTao
		* @since 2020.12.05
		* @param pStateSet:			��Ҫ��shader��״̬��ָ��
		* @param vertFilePath:		����shader·��
		* @param fragFilePath:		ƬԪshader·��
		* @param geomFilePath:		����shader·��
		* @param bPixelLighting:	�Ƿ������ع��գ�����ǣ���Ҫ������������
		* @return bool:				�ɹ�Ϊtrue������false
		*/
		static bool LoadShader(
			osg::StateSet* pStateSet
			, std::string& vertFilePath
			, std::string& fragFilePath
			, std::string& geomFilePath
			, bool bPixelLighting = false);

		/**
		* LoadShader
		* ����shader
		* @author LiuTao
		* @since 2020.12.05
		* @param pStateSet:			��Ҫ��shader��״̬��ָ��
		* @param vertFilePath:		����shader·��
		* @param fragFilePath:		ƬԪshader·��
		* @return bool:				�ɹ�Ϊtrue������false
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
