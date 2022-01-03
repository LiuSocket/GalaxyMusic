//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMXml.cpp
/// @brief		Galaxy-Music Engine - GMXml.cpp
/// @version	1.0
/// @author		LiuTao
/// @date		2021.06.23
//////////////////////////////////////////////////////////////////////////
#include "GMXml.h"
#include <Windows.h>
#include <iostream>

using namespace GM;

/*************************************************************************
 CGMXml Methods
*************************************************************************/
/** @brief 构造 */
CGMXml::CGMXml():
	m_pDoc(nullptr), m_pRoot(nullptr)
{
}

/** @brief 析构 */
CGMXml::~CGMXml()
{
	GM_DELETE(m_pDoc);
}

/** @brief 创建 */
bool CGMXml::Create(const std::string & strPathName, const std::string & strRootName)
{
	const std::string strHead = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

	m_pDoc = new TiXmlDocument(strPathName.c_str());
	m_pDoc->Parse(strHead.c_str());

	if (m_pDoc->Error() && m_pDoc->ErrorId() == TiXmlBase::TIXML_ERROR_OPENING_FILE)
	{
		std::cout << "WARNING: File " << strPathName.c_str() << " is not created.\n";
		return false;
	}
	else
	{
		TiXmlElement root(strRootName.c_str());
		m_pDoc->InsertEndChild(root);

		TiXmlHandle docH(m_pDoc);
		m_pRoot = docH.FirstChildElement(strRootName.c_str()).Element();
		return true;
	}
}

/** @brief 加载 */
bool CGMXml::Load(const std::string& strPathName, const std::string& strRootName)
{
	m_pDoc = new TiXmlDocument(strPathName.c_str());
	m_pDoc->LoadFile();
	if (m_pDoc->Error() && m_pDoc->ErrorId() == TiXmlBase::TIXML_ERROR_OPENING_FILE)
	{
		std::cout<<"WARNING: File "<< strPathName.c_str()<<" is not found.\n";
		return false;
	}
	else
	{
		TiXmlHandle docH(m_pDoc);
		m_pRoot = docH.FirstChildElement(strRootName.c_str()).Element();
		return true;
	}
}

/** @brief 保存 */
bool CGMXml::Save()
{
	return m_pDoc->SaveFile();
}

TiXmlElement CGMXml::AddChild(const std::string& strChildName,
	const char* strText, const double fRadius, const double fAngle)
{
	TiXmlElement ele(strChildName.c_str());
	if (m_pRoot)
	{
		ele.SetAttribute("name", strText);
		ele.SetDoubleAttribute("radius", fRadius);
		ele.SetDoubleAttribute("angle", fAngle);
		m_pRoot->InsertEndChild(ele);
	}
	return ele;
}

TiXmlElement CGMXml::GetChild(const std::string& strChildName)
{
	if (m_pRoot)
	{
		TiXmlNode* ele = 0;
		while ((ele = m_pRoot->IterateChildren(ele)) != 0)
		{
			const std::string strValue = ele->Value();
			if (strChildName == strValue)
			{
				return *(ele->ToElement());
			}
		}
	}
	return TiXmlElement(strChildName.c_str());
}

VGMXmlElementVec CGMXml::GetChildren(const std::string& strChildName)
{
	VGMXmlElementVec sList;
	if (m_pRoot)
	{
		TiXmlNode* ele = 0;
		while ((ele = m_pRoot->IterateChildren(ele)) != 0)
		{
			const std::string strValue = ele->Value();
			if (strChildName == strValue)
			{
				sList.push_back(*(ele->ToElement()));
			}
		}
	}
	return sList;
}