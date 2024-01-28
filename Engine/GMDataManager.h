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
#include "GMDispatchCompute.h"

#include <osg/Texture2D>

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
		bool Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData);
		/** @brief ���� */
		bool Update(double dDeltaTime);
		/** @brief ���� */
		bool Save();

		/**
		* GetAudioNum
		* ��ȡ��Ƶ���������֧��65536�׸�
		* @author LiuTao
		* @since 2021.06.20
		* @return int ��Ƶ����
		*/
		inline int GetAudioNum(){ return int(m_audioDataMap.size()); }

		/**
		* GetAudioDataMap
		* ��ȡ��Ƶ����map�����֧��65536�׸�
		* @author LiuTao
		* @since 2021.06.27
		* @return bool �ɹ�����true��ʧ�ܷ���false
		*/
		bool GetAudioDataMap(std::map<unsigned int, SGMAudioData>& dataMap);

		/**
		* FindAudio(std::wstring& strName)
		* ��ѯ��Ƶ�ļ�
		* @author LiuTao
		* @since 2021.06.22
		* @return bool ���ڷ���true�������ڷ���false
		*/
		bool FindAudio(const std::wstring& strName);

		/**
		* FindAudio(double& fX, double& fY, double& fZ, std::wstring& strName)
		* �������������ѯ��������Ƶ�ļ�����
		* @author LiuTao
		* @since 2021.06.22
		* @param fX,fY,fZ:	��Ҫ��ѯ����������[-1,1]�������ѯ�������޸�Ϊ�����Ƶ����������
		* @param strName:	��ѯ������Ƶ�����ƣ����ʧ�ܣ����ؿ��ַ��� L""
		* @return bool ���ڷ���true�������ڷ���false
		*/
		bool FindAudio(double& fX, double& fY, double& fZ, std::wstring& strName);

		/**
		* FindAudio(const unsigned int iUID)
		* ������ƵID��ѯ��Ƶ�ļ�
		* @author LiuTao
		* @since 2021.07.02
		* @param iID:	��Ҫ��ѯ��ID����ѯǰӦ���Ȼ�ȡ��Ƶ������������
		* @return wstring ���ڷ�����Ƶ�ļ����ƣ��������򷵻ؿ��ַ��� L""
		*/
		std::wstring FindAudio(const unsigned int iUID);

		/**
		* GetLastAudio()
		* ��ѯ��һ����Ƶ�ļ����ƣ����д��ۣ��ص���ȥ�۲���ʷ��Ϣ��δ�����ı�
		* @author LiuTao
		* @since 2021.11.07
		* @param void
		* @return wstring ���ڷ�����Ƶ�ļ����ƣ��������򷵻ؿ��ַ��� L""
		*/
		std::wstring GetLastAudio();

		/**
		* GetCurrentAudio() const
		* ��ȡ��ǰ��Ƶ����
		* @author LiuTao
		* @since 2022.03.27
		* @param void
		* @return std::wstring��	��ǰ��Ƶ����
		*/
		inline std::wstring GetCurrentAudio() const
		{
			return m_strCurrentAudio;
		};

		/**
		* GetCurrentAudioID() const
		* ��ȡ��ǰ��ƵID
		* @author LiuTao
		* @since 2021.10.23
		* @param void
		* @return int	��ǰ��ƵID
		*/
		int GetCurrentAudioID() const;

		/**
		* GetAudioColor
		* ������Ƶ�ռ����꣬��ȡ�����ֵ���ɫ��
		* @author LiuTao
		* @since 2021.07.03
		* @param audioCoord��	��Ƶ�ռ�����
		* @return osg::Vec4f��	���������Ͷ�Ӧ����ɫ
		*/
		osg::Vec4f GetAudioColor(const SGMAudioCoord& audioCoord) const;

		/**
		* GetAudioColor const
		* �����ǳ����꣬��ȡ�����ֵ���ɫ��
		* @author LiuTao
		* @since 2021.07.03
		* @param iUID��				��ƵUID
		* @return osg::Vec4f��		���������Ͷ�Ӧ����ɫ
		*/
		osg::Vec4f GetAudioColor(const unsigned int iUID) const;

		/**
		* Angle2Color const
		* �����ֵġ������Ƕȡ�ת����ɫ����
		* @author LiuTao
		* @since 2022.09.04
		* @param fEmotionAngle��	�����Ƕȣ�[0.0,2*PI)
		* @return osg::Vec4f��		���������Ͷ�Ӧ����ɫ,[0.0,1.0]
		*/
		osg::Vec4f Angle2Color(const float fEmotionAngle) const;

		/**
		* GetUID() const
		* ��ȡ��ǰ���ŵ������ֵ�UID��
		* @author LiuTao
		* @since 2021.07.04
		* @param void
		* @return unsigned int��	��ƵUID
		*/
		unsigned int GetUID() const;

		/**
		* GetUID const
		* �������������ƣ���ȡ�����ֵ��ǳ����ꡣ
		* @author LiuTao
		* @since 2021.07.04
		* @param strName:			����������
		* @return unsigned int��	��ƵUID
		*/
		unsigned int GetUID(const std::wstring& strName) const;

		/**
		* GetAudioCoord const
		* �������������ƣ���ȡ�����ֵ���Ƶ�ռ����ꡣ
		* @author LiuTao
		* @since 2021.07.04
		* @param strName:			����������
		* @return SGMAudioCoord��	��Ƶ�ռ�����
		*/
		SGMAudioCoord GetAudioCoord(const std::wstring& strName) const;

		/**
		* GetGalaxyCoord const
		* �������������ƣ���ȡ�����ֵ��������ꡣ
		* @author LiuTao
		* @since 2021.07.04
		* @param strName:			����������
		* @return SGMGalaxyCoord��	��������
		*/
		SGMGalaxyCoord GetGalaxyCoord(const std::wstring& strName) const;

		/**
		* EditAudioData
		* @brief �޸����е�ĳ�������ֵ�λ�����ݡ�������Ϸ��������ֲ����ڣ��򷵻�false
			���λ������֪��Ƶ���غϣ����Զ�Ǩ�Ƶ�����λ�ã��޸�sData������
		* @author LiuTao
		* @since 2021.07.04
		* @param sData��			���������ݽṹ
		* @return bool��			�ɹ�true��ʧ��false
		*/
		bool EditAudioData(SGMAudioData& sData);

		/**
		* SavePlayingOrder
		* �����ϵ���Ƶ�Ĳ���˳��д��Data/Core/Users/AudioPlayingOrder.xml
		* @author LiuTao
		* @since 2022.03.26
		* @param void��		��
		* @return bool��	�ɹ�true��ʧ��false
		*/
		bool SavePlayingOrder() const;

		/**
		* GetPlayingOrder const
		* ��ȡ���������Ƶ��list��backΪ���¸���
		* @author LiuTao
		* @since 2022.03.26
		* @param void��							��
		* @return std::vector<std::wstring>��	���������Ƶ��list
		*/
		inline const std::vector<std::wstring> GetPlayingOrder() const
		{
			return m_playingOrder;
		}

		/**
		* AudioCoord2GalaxyCoord const
		* ��Ƶ�ռ�����ת��������
		* @author LiuTao
		* @since 2021.07.23
		* @param audioCoord:		��Ƶ�ռ�����
		* @return SGMGalaxyCoord:	��������
		*/
		SGMGalaxyCoord AudioCoord2GalaxyCoord(const SGMAudioCoord& audioCoord) const;

	private:

		/**
		* _RefreshAudioCoordinates
		* ��ȡData/Core/Users/AudioData.xml��������Ƶ���걣����m_audioDataMap��
		* @author LiuTao
		* @since 2021.06.14
		* @return void
		*/
		void _RefreshAudioCoordinates();

		/**
		* _SaveAudioCoordinates const
		* ����Ƶ����д��Data/Core/Users/AudioData.xml
		* @author LiuTao
		* @since 2021.06.27
		* @return bool �ɹ�true��ʧ��false
		*/
		bool _SaveAudioCoordinates()const;

		/**
		* _RefreshAudioFiles
		* ɨ��Data/Media/Music·��������֧�ֵ���Ƶ�ļ��������ļ���������m_audioDataMap��
		* @author LiuTao
		* @since 2021.06.14
		* @return void
		*/
		void _RefreshAudioFiles();

		/**
		* _DeleteOverdueAudios
		* ɾ�����ڵ��ļ��б��������ָ���ļ�����û�иø���
		* �ú����ܺ�ʱ
		* @author LiuTao
		* @since 2022.04.23
		* @return void
		*/
		void _DeleteOverdueAudios();

		/**
		* _UpdateAudioList
		* �������������Ƶ��list
		* @author LiuTao
		* @since 2022.03.26
		* @param strName��		����ѯ��Ҳ���ǲ��ţ�����Ƶ����
		* @return void��		��
		*/
		void _UpdateAudioList(const std::wstring& strName);

		/**
		* _LoadPlayingOrder
		* ��Data/Core/Users/AudioPlayingOrder.xml ��ȡ�Ѿ��������Ƶ����˳��
		* @author LiuTao
		* @since 2022.03.27
		* @param void��			��
		* @return void��		��
		*/ 
		void _LoadPlayingOrder();

		/**
		* @brief ��Ϻ���,�ο� glsl �е� mix(a,b,x)
		* @author LiuTao
		* @since 2022.01.09
		* @param fMin, fMax:			��Χ
		* @param fX:					���ϵ��
		* @return double:				��Ϻ��ֵ
		*/
		inline double _Mix(const double fMin, const double fMax, const double fX) const
		{
			return fMin * (1 - fX) + fMax * fX;
		}

		/**
		* @brief ���������AudioData����map��m_audioDataMap����Ӻ��޸Ķ�����ʹ���������
			�����͸���FreeUID����֤FreeUID��һֱ���п��õ�
		* @author LiuTao
		* @since 2022.08.21
		* @param sData:				�����AudioData
		* @return bool:				����������ӣ���true�����������޸ģ���false
		*/
		bool _AddAudioData2Map(SGMAudioData& sData);

		/**
		* ��ʼ����ʹ��compute shader��������Ŀɲ�����Ƶ���洢��ͼƬ��
		*/
		void _InitComputeNearUID();

		/**
		* ������Ⱦ��
		* @brief ��Ļ������С�������棬�Ⱦ���Ч��Ҫ��һЩ
		* @author LiuTao
		* @since 2020.09.01
		* @param width: ��Ч���εĿ��
		* @param height: ��Ч���εĸ߶�
		* @return osg::Geometry* ���ؼ��νڵ�ָ��
		*/
		osg::Geometry* _CreateScreenTriangle(const int width, const int height);

		// ����
	private:
		SGMKernelData*								m_pKernelData;					//!< �ں�����
		SGMConfigData*								m_pConfigData;					//!< ��������

		std::wstring								m_strAudioPath;					//!< ��Ƶ���·��
		std::wstring								m_strCurrentAudio;				//!< ��ǰ���ŵ���Ƶ����
		std::vector<std::wstring>					m_formatVector;					//!< ֧�ֵ��ļ����ͣ�����mp3
		std::vector<std::wstring>					m_playingOrder;					//!< �����ϵ���Ƶ����˳�򣨲�������
		std::map<unsigned int, SGMAudioData>		m_audioDataMap;					//!< AudioData.xml���е�����map
		std::list<std::wstring>						m_historyList;					//!< ��Ƶ������ʷ�б���������
		unsigned int								m_iFreeUID;						//!< ��ǰ���õ�UID������ʱ����

		osg::ref_ptr<osg::Uniform>					m_pAudioNumUniform;				//!< ��Ƶ����Uniform

		/** m_pGalaxyCoordImage �洢������Ƶ�����������ͼƬ��
		** m_pGalaxyCoordTex �洢������Ƶ���������������
		** ����ΪR32F		��Ƶ��λ�ã��������ر�ʾһ����Ƶ������
		 */
		osg::ref_ptr<osg::Image>					m_pGalaxyCoordImage;
		osg::ref_ptr<osg::Texture2D>				m_pGalaxyCoordTex;
		/** m_pNearUIDImage �洢�����������Ƶλ����Ϣ��ͼƬ��
		** m_pNearUIDTex ��Ӧ������
		** RGB8		��ƵUID��24λ�������Ͽ��Ա��16777216��һǧ���򣩸���Ƶ
		** A8		������Ϣ */
		osg::ref_ptr<osg::Image>					m_pNearUIDImage;
		osg::ref_ptr<osg::Texture2D>				m_pNearUIDTex;
		osg::ref_ptr<CGMDispatchCompute>			m_pNearComputeNode;				//!< �������UID��CS�ڵ�
		osg::ref_ptr<osg::Camera>					m_pReadPixelCam;				//!< ���ڶ�ȡ���ص����
		CReadPixelFinishCallback*					m_pReadPixelFinishCallback;
	};
}	// GM