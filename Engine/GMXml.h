//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMXml.h
/// @brief		Galaxy-Music Engine - GMXml.h
/// @version	1.0
/// @author		LiuTao
/// @date		2021.06.23
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMStructs.h"
#include "./Assist/tinyxml.h"

namespace GM
{
	/*************************************************************************
	 Type Defines
	*************************************************************************/
	typedef std::vector<TiXmlElement>			VGMXmlElementVec;

	/*************************************************************************
	 Class
	*************************************************************************/

	/*!
 	 *  @class CGMXml
	 *  @brief GalaxyMusic Xml Manager
	 */
	class CGMXml
	{
	// ����
	public:
		/** @brief ���� */
		CGMXml();
		/** @brief ���� */
		~CGMXml();

		/** @brief ���� */
		bool Create(const std::string& strPathName, const std::string& strRootName);
		/** @brief ���� */
		bool Load(const std::string& strPathName, const std::string& strRootName);
		/** @brief ���� */
		bool Save();

		/** @brief ����ӽڵ� */
		TiXmlElement AddChild(const std::string& strChildName,
			const char* strText, const double fRadius, const double fAngle);
		/** @brief ��ȡ��һ�������������ӽڵ� */
		TiXmlElement GetChild(const std::string& strChildName);
		/** @brief ��ȡ�ӽڵ��� */
		VGMXmlElementVec GetChildren(const std::string& strChildName);

	// ����
	private:
		TiXmlDocument*				m_pDoc;		//!< �ĵ�ָ��
		TiXmlElement*				m_pRoot;	//!< ���ڵ�
	};

}	// GM
