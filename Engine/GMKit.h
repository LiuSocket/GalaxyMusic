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
		* @param strShaderName:		shader������
		* @param bPixelLighting:	�Ƿ������ع��գ�����ǣ���Ҫ������������
		* @return bool:				�ɹ�Ϊtrue������false
		*/
		static bool LoadShader(
			osg::StateSet* pStateSet,
			const std::string& vertFilePath,
			const std::string& fragFilePath,
			const std::string& geomFilePath,
			const std::string& strShaderName,
			bool bPixelLighting = false);

		/*!
		* @brief ����Shader by StateSet
		* @param pStateSet:				osg::StateSet
		* @param vertFilePath:			the vertex glsl file path
		* @param fragFilePath:			the fragment glsl file path
		* @param fragCommonFilePath:	the optional common fragment file path
		* @param shaderName:			name of shader
		* @param bPixelLighting:		be pixel lighting(for tangent & binormal)
		* @return bool:					�ɹ�Ϊtrue������false
		*/
		static bool LoadShaderWithCommonFrag(
			osg::StateSet* pStateSet
			, const std::string& vertFilePath
			, const std::string& fragFilePath
			, const std::string& fragCommonFilePath
			, const std::string& shaderName
			, const bool bPixelLighting = false);

		/**
		* ����shader
		* @param pStateSet:			��Ҫ��shader��״̬��ָ��
		* @param vertFilePath:		����shader·��
		* @param fragFilePath:		ƬԪshader·��
		* @param strShaderName:		shader������
		* @return bool:				�ɹ�Ϊtrue������false
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
		* @param pStateSet:			��Ҫ��shader��״̬��ָ��
		* @param compFilePath:		compute shader·��
		* @param shaderName:		shader������
		* @return bool:				�ɹ�Ϊtrue������false
		*/
		static bool LoadComputeShader(
			osg::StateSet* pStateSet,
			const std::string& compFilePath,
			const std::string& shaderName = "");

		/**
		* ��״̬�����������
		* @author LiuTao
		* @since 2022.07.29
		* @param pStateSet:			��Ҫ��shader��״̬��ָ��
		* @param pTex:				��Ҫ��ӵ�����ָ��
		* @param texName:			������glsl�е�����
		* @param iUnit:				����Ԫ
		* @return bool:				�ɹ�Ϊtrue������false
		*/
		static bool AddTexture(osg::StateSet* pStateSet, osg::Texture* pTex, const char* texName, const int iUnit);

		/**
		* @brief ��ȡͼƬ��RGBAͨ��ֵ
		* @param pImg:		ͼƬָ��
		* @param fX:		ͼ��x����,[0,1]
		* @param fY:		ͼ��y����,[0,1]
		* @param bLinear:	�Ƿ�˫���Բ�ֵ��true = ˫���ԣ�false = �ٽ�ֵ
		* @return Vec4f��	RGBAͨ��ֵ,[0.0,1.0]
		*/
		static osg::Vec4f GetImageColor(
			const osg::Image* pImg,
			const float fX, const float fY,
			const bool bLinear = false);

		/**
		* @brief 16F ת 32F
		* @param x:			16F
		* @return float��	32F
		*/
		static float Half_2_Float(const unsigned short x);
		/**
		* @brief 32F ת 16F 
		* @param x:					32F
		* @return unsigned short��	16F
		*/
		static unsigned short Float_2_Half(const float x);

		/**
		* @brief ��Ϻ���,�ο� glsl �е� mix(a,b,x)
		* @param fA, fB:				��Χ
		* @param fX:					���ϵ��
		* @return ��������:				��Ϻ��ֵ
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
