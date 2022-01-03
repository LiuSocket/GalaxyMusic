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
#include <random>

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
		// ����
	public:
		/** @brief ���� */
		CGMDataManager();

		/** @brief ���� */
		~CGMDataManager();

		/** @brief ��ʼ�� */
		bool Init(SGMConfigData* pConfigData);

		/** @brief ���� */
		bool Save();

		/**
		* GetAudioNum
		* ��ȡ��Ƶ���������֧��65536�׸�
		* @author LiuTao
		* @since 2021.06.20
		* @return int ��Ƶ����
		*/
		inline int GetAudioNum(){ return int(m_fileVector.size()); }

		/**
		* GetStarCoordVector
		* ��ȡ��Ƶ���ǳ�����Vector�����֧��65536�׸�
		* @author LiuTao
		* @since 2021.06.27
		* @return bool �ɹ�����true��ʧ�ܷ���false
		*/
		bool GetStarCoordVector(std::vector<SGMStarCoord>& coordV);

		/**
		* FindAudio(std::wstring& strName)
		* ��ѯ��Ƶ�ļ�
		* @author LiuTao
		* @since 2021.06.22
		* @return bool ���ڷ���true�������ڷ���false
		*/
		bool FindAudio(const std::wstring& strName);

		/**
		* FindAudio(float& fX, float& fY, float& fZ)
		* �������������ѯ��������Ƶ�ļ�����
		* @author LiuTao
		* @since 2021.06.22
		* @param fX,fY,fZ:	��Ҫ��ѯ��λ�ã��������꣩ [-1,-1]��������ڣ����޸�����
		* @return wstring ���ڷ�����Ƶ�ļ����ƣ��������򷵻ؿ��ַ��� L""
		*/
		std::wstring FindAudio(double& fX, double& fY, double& fZ);

		/**
		* FindAudio(int iID)
		* ������ƵID��ѯ��Ƶ�ļ�
		* @author LiuTao
		* @since 2021.07.02
		* @param iID:	��Ҫ��ѯ��ID����ѯǰӦ���Ȼ�ȡ��Ƶ������������
		* @return wstring ���ڷ�����Ƶ�ļ����ƣ��������򷵻ؿ��ַ��� L""
		*/
		std::wstring FindAudio(const int iID);

		/**
		* FindLastAudio()
		* ��ѯ��һ����Ƶ�ļ�����
		* @author LiuTao
		* @since 2021.11.07
		* @param void
		* @return wstring ���ڷ�����Ƶ�ļ����ƣ��������򷵻ؿ��ַ��� L""
		*/
		std::wstring FindLastAudio();

		/**
		* GetCurrentAudioID
		* ��ȡ��ǰ��ƵID
		* @author LiuTao
		* @since 2021.10.23
		* @param void
		* @return int	��ǰ��ƵID
		*/
		int GetCurrentAudioID();

		/**
		* GetAudioColor
		* ��ȡ���������Ͷ�Ӧ����ɫ��
		* @author LiuTao
		* @since 2021.07.03
		* @param audioCoord:	��Ƶ�ռ�����
		* @return osg::Vec4f	���������Ͷ�Ӧ����ɫ
		*/
		osg::Vec4f GetAudioColor(const SGMAudioCoord audioCoord);

		/**
		* GetAudioColor
		* ��ȡ���������Ͷ�Ӧ����ɫ��
		* @author LiuTao
		* @since 2021.07.03
		* @param starCoord:			�ǳ�����
		* @return osg::Vec4f��		���������Ͷ�Ӧ����ɫ
		*/
		osg::Vec4f GetAudioColor(const SGMStarCoord starCoord);

		/**
		* GetStarCoord
		* ��ȡ��ǰ���ŵ������ֵ��ǳ����ꡣ
		* @author LiuTao
		* @since 2021.07.04
		* @param void
		* @return SGMStarCoord��	�ǳ�����
		*/
		SGMStarCoord GetStarCoord();

		/**
		* GetStarCoord
		* ��ȡ�����ֵ��ǳ����ꡣ
		* @author LiuTao
		* @since 2021.07.04
		* @param strName:			����������
		* @return SGMStarCoord��	�ǳ�����
		*/
		SGMStarCoord GetStarCoord(const std::wstring& strName);

		/**
		* GetAudioCoord
		* ��ȡ�����ֵ���Ƶ�ռ����ꡣ
		* @author LiuTao
		* @since 2021.07.04
		* @param strName:			����������
		* @return SGMAudioCoord��	��Ƶ�ռ�����
		*/
		SGMAudioCoord GetAudioCoord(const std::wstring& strName);

		/**
		* SetAudioData
		* ���������ֵ����ݡ�������Ϸ����򷵻�false
		* @author LiuTao
		* @since 2021.07.04
		* @param sData��			���������ݽṹ
		* @return bool��			�ɹ�true��ʧ��false
		*/
		bool SetAudioData(const SGMAudioData& sData);

	private:

		/**
		* _RefreshAudioCoordinates
		* ��ȡData/Core/Coordinates/AudioData.xml��������Ƶ���걣����m_fileCoordsMap��
		* @author LiuTao
		* @since 2021.06.14
		* @return void
		*/
		void _RefreshAudioCoordinates();

		/**
		* _SaveAudioCoordinates
		* ����Ƶ����д��Data/Core/Coordinates/AudioData.xml
		* @author LiuTao
		* @since 2021.06.27
		* @return bool �ɹ�true��ʧ��false
		*/
		bool _SaveAudioCoordinates();

		/**
		* _RefreshAudioFiles
		* ɨ��Data/Media/Music·��������֧�ֵ���Ƶ�ļ��������ļ���������m_fileVector��
		* @author LiuTao
		* @since 2021.06.14
		* @return void
		*/
		void _RefreshAudioFiles();
		/**
		* _CharToWstring
		* �ַ���ת�� char* to wstring
		* @author LiuTao
		* @since 2021.06.26
		* @param cstr:	���ֽ��ַ���
		* @return wstring ���ֽ��ַ���
		*/
		const std::wstring _CharToWstring(const char* cstr);
		/**
		* _WcharToChar
		* �ַ���ת�� wchar_t* to string
		* @author LiuTao
		* @since 2021.06.26
		* @param wstr:	���ֽ��ַ���
		* @return string:	���ֽ��ַ���
		*/
		const std::string _WcharToChar(const wchar_t* wstr);

		/**
		* _AudioCoord2StarCoord
		* ��Ƶ�ռ�����ת�ǳ�����
		* @author LiuTao
		* @since 2021.07.23
		* @param audioCoord:	��Ƶ�ռ�����
		* @return SGMStarCoord:	�ǳ�����
		*/
		SGMStarCoord _AudioCoord2StarCoord(const SGMAudioCoord& audioCoord);

		// ����
	private:
		SGMConfigData*								m_pConfigData;					//!< ��������

		std::wstring								m_strAudioPath;					//!< ���ִ��·��
		std::wstring								m_strAudioName;					//!< ���һ�β�ѯ���ļ���,XXX.mp3
		std::vector<std::wstring>					m_formatVector;					//!< ֧�ֵ��ļ����ͣ�����mp3
		std::vector<std::wstring>					m_fileVector;					//!< Media�ļ���������֧�ֵ���Ƶ�ļ�
		std::vector<std::wstring>					m_playedVector;					//!< �Ѳ�����Ƶ������Vector
		std::vector<std::wstring>					m_storedNameVector;				//!< AudioData.xml���е�����Vector
		std::vector<SGMStarCoord>					m_storedCoordVector;			//!< AudioData.xml���е�����Vector
		std::map<SGMStarCoord, SGMAudioData>		m_audioDataMap;					//!< �ǳ���������Ƶ���ݵ�ӳ��
		std::default_random_engine					m_iRandom;
		std::vector<osg::Vec2i>						m_vOffsetVector;				//!< ƫ��˳�����ڲ�ѯ��Ƶ
	};
}	// GM