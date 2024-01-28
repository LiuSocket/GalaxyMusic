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
 SGMXmlNode Methods
*************************************************************************/

bool CGMXmlNode::GetName(std::string & nodeName)
{
	if (m_pNode == nullptr)
		return false;
	nodeName = m_pNode->Value();
	return true;
}

/** @brief 添加子节点 */
CGMXmlNode CGMXmlNode::AddChild(const std::string& strChildName)
{
	if (m_pNode == nullptr)
		return CGMXmlNode();

	TiXmlNode* pEle = new TiXmlElement(strChildName.c_str());
	//m_pNode->InsertEndChild(*pEle);
	//return CGMXmlNode(m_pXml, pEle->ToElement());
	return CGMXmlNode(m_pXml, (m_pNode->InsertEndChild(*pEle))->ToElement());
}

/** @brief 设置String属性 */
void CGMXmlNode::SetPropStr(const std::string& strPropertyName, const char * strPropertyValue)
{
	if (m_pNode == nullptr)
		return;
	m_pNode->SetAttribute(strPropertyName.c_str(), strPropertyValue);
}

void CGMXmlNode::SetPropWStr(const std::string & strPropertyName, const wchar_t * strPropertyValue)
{
	if (m_pNode == nullptr)
		return;
	m_pNode->SetAttribute(strPropertyName.c_str(), _WCharToChar(strPropertyValue));
}

/** @brief 设置Bool属性 */
void CGMXmlNode::SetPropBool(const std::string& strPropertyName, bool bPropertyValue)
{
	if (m_pNode == nullptr)
		return;
	m_pNode->SetAttribute(strPropertyName.c_str(), bPropertyValue ? "true" : "false");
}

/** @brief 设置Int属性 */
void CGMXmlNode::SetPropInt(const std::string& strPropertyName, int iPropertyValue)
{
	if (m_pNode == nullptr)
		return;
	m_pNode->SetAttribute(strPropertyName.c_str(), iPropertyValue);
}

/** @brief 设置Unsigned Int属性 */
void CGMXmlNode::SetPropUInt(const std::string& strPropertyName, unsigned int iPropertyValue)
{
	if (m_pNode == nullptr)
		return;
	m_pNode->SetAttribute(strPropertyName.c_str(), iPropertyValue);
}

/** @brief 设置Float属性 */
void CGMXmlNode::SetPropFloat(const std::string& strPropertyName, float fPropertyValue)
{
	if (m_pNode == nullptr)
		return;
	m_pNode->SetDoubleAttribute(strPropertyName.c_str(), fPropertyValue);
}

/** @brief 设置Double属性 */
void CGMXmlNode::SetPropDouble(const std::string& strPropertyName, double fPropertyValue)
{
	if (m_pNode == nullptr)
		return;
	m_pNode->SetDoubleAttribute(strPropertyName.c_str(), fPropertyValue);
}

/** @brief 设置Vector2属性 */
void CGMXmlNode::SetPropVector2(const std::string& strPropertyName, const SGMVector2& vPropertyValue)
{
	if (m_pNode == nullptr)
		return;
	std::string strPropertyValue = std::to_string(vPropertyValue.x) + " " + std::to_string(vPropertyValue.y);
	m_pNode->SetAttribute(strPropertyName.c_str(), strPropertyValue.c_str());
}
/** @brief 设置Vector2i属性 */
void CGMXmlNode::SetPropVector2(const std::string& strPropertyName, const SGMVector2i& vPropertyValue)
{
	if (m_pNode == nullptr)
		return;
	std::string strPropertyValue = std::to_string(vPropertyValue.x) + " " + std::to_string(vPropertyValue.y);
	m_pNode->SetAttribute(strPropertyName.c_str(), strPropertyValue.c_str());
}
/** @brief 设置Vector2f属性 */
void CGMXmlNode::SetPropVector2(const std::string& strPropertyName, const SGMVector2f& vPropertyValue)
{
	if (m_pNode == nullptr)
		return;
	std::string strPropertyValue = std::to_string(vPropertyValue.x) + " " + std::to_string(vPropertyValue.y);
	m_pNode->SetAttribute(strPropertyName.c_str(), strPropertyValue.c_str());
}

/** @brief 设置Vector3属性 */
void CGMXmlNode::SetPropVector3(const std::string& strPropertyName, const SGMVector3& vPropertyValue)
{
	if (m_pNode == nullptr)
		return;
	std::string strPropertyValue =
		std::to_string(vPropertyValue.x) + " " +
		std::to_string(vPropertyValue.y) + " " +
		std::to_string(vPropertyValue.z);
	m_pNode->SetAttribute(strPropertyName.c_str(), strPropertyValue.c_str());
}
/** @brief 设置Vector3i属性 */
void CGMXmlNode::SetPropVector3(const std::string& strPropertyName, const SGMVector3i& vPropertyValue)
{
	if (m_pNode == nullptr)
		return;
	std::string strPropertyValue =
		std::to_string(vPropertyValue.x) + " " +
		std::to_string(vPropertyValue.y) + " " +
		std::to_string(vPropertyValue.z);
	m_pNode->SetAttribute(strPropertyName.c_str(), strPropertyValue.c_str());
}
/** @brief 设置Vector3f属性 */
void CGMXmlNode::SetPropVector3(const std::string& strPropertyName, const SGMVector3f& vPropertyValue)
{
	if (m_pNode == nullptr)
		return;
	std::string strPropertyValue =
		std::to_string(vPropertyValue.x) + " " +
		std::to_string(vPropertyValue.y) + " " +
		std::to_string(vPropertyValue.z);
	m_pNode->SetAttribute(strPropertyName.c_str(), strPropertyValue.c_str());
}

/** @brief 设置Vector4属性 */
void CGMXmlNode::SetPropVector4(const std::string& strPropertyName, const SGMVector4& vPropertyValue)
{
	if (m_pNode == nullptr)
		return;
	std::string strPropertyValue =
		std::to_string(vPropertyValue.x) + " " +
		std::to_string(vPropertyValue.y) + " " +
		std::to_string(vPropertyValue.z) + " " +
		std::to_string(vPropertyValue.w);
	m_pNode->SetAttribute(strPropertyName.c_str(), strPropertyValue.c_str());
}
/** @brief 设置Vector4i属性 */
void CGMXmlNode::SetPropVector4(const std::string& strPropertyName, const SGMVector4i& vPropertyValue)
{
	if (m_pNode == nullptr)
		return;
	std::string strPropertyValue =
		std::to_string(vPropertyValue.x) + " " +
		std::to_string(vPropertyValue.y) + " " +
		std::to_string(vPropertyValue.z) + " " +
		std::to_string(vPropertyValue.w);
	m_pNode->SetAttribute(strPropertyName.c_str(), strPropertyValue.c_str());
}
/** @brief 设置Vector4f属性 */
void CGMXmlNode::SetPropVector4(const std::string& strPropertyName, const SGMVector4f& vPropertyValue)
{
	if (m_pNode == nullptr)
		return;
	std::string strPropertyValue =
		std::to_string(vPropertyValue.x) + " " +
		std::to_string(vPropertyValue.y) + " " +
		std::to_string(vPropertyValue.z) + " " +
		std::to_string(vPropertyValue.w);
	m_pNode->SetAttribute(strPropertyName.c_str(), strPropertyValue.c_str());
}

/** @brief 获取子节点 */
CGMXmlNode CGMXmlNode::GetChild(const std::string& strChildName) const
{
	if (m_pNode == nullptr)
		return CGMXmlNode();

	TiXmlNode* pEle = 0;
	while ((pEle = m_pNode->IterateChildren(pEle)) != 0)
	{
		const std::string strValue = pEle->Value();
		if (strChildName == strValue)
		{
			break;
		}
	}
	return CGMXmlNode(m_pXml, pEle->ToElement());
}

/** @brief 获取子节点组 */
VGMXmlNodeVec CGMXmlNode::GetChildren(const std::string& strChildName) const
{
	VGMXmlNodeVec sList;
	if (m_pNode)
	{
		TiXmlNode* pEle = 0;
		while ((pEle = m_pNode->IterateChildren(pEle)) != 0)
		{
			const std::string strValue = pEle->Value();
			if (strChildName == strValue)
			{
				sList.push_back(CGMXmlNode(m_pXml, pEle->ToElement()));
			}
		}
	}
	return sList;
}

bool CGMXmlNode::HasProperty(const std::string & propertyName) const
{
	if (m_pNode)
	{
		const char* value = m_pNode->Attribute(propertyName.c_str());
		if (value)
			return true;
	}
	return false;
}

/** @brief 获取String属性 */
const char* CGMXmlNode::GetPropStr(const std::string& strPropertyName, const char* strDefault) const
{
	if (m_pNode)
	{
		const char* value = m_pNode->Attribute(strPropertyName.c_str());
		if (value)
			return value;
	}
	return strDefault;
}

const wchar_t* CGMXmlNode::GetPropWStr(const std::string & strPropertyName, const wchar_t* strDefault) const
{
	if (m_pNode)
	{
		const char* value = m_pNode->Attribute(strPropertyName.c_str());
		if (value)
			return _CharToWChar(value);
	}
	return strDefault;
}

/** @brief 获取Bool属性 */
bool CGMXmlNode::GetPropBool(const std::string& strPropertyName, bool bDefault) const
{
	if (m_pNode)
	{
		bool bValue;
		m_pNode->QueryBoolAttribute(strPropertyName.c_str(), &bValue);
		return bValue;
	}
	return bDefault;
}

/** @brief 获取Int属性 */
int CGMXmlNode::GetPropInt(const std::string& strPropertyName, int iDefault) const
{
	if (m_pNode)
	{
		int iValue;
		if (m_pNode->Attribute(strPropertyName.c_str(), &iValue))
			return iValue;
	}
	return iDefault;
}

/** @brief 获取Unsigned Int属性 */
unsigned int CGMXmlNode::GetPropUInt(const std::string& strPropertyName, unsigned int iDefault) const
{
	if (m_pNode)
	{
		int iValue = -1;
		if (m_pNode->Attribute(strPropertyName.c_str(), &iValue))
		{
			if (iValue >= 0)
				return iValue;
			else
				return iDefault;
		}
	}
	return iDefault;
}

/** @brief 获取Float属性 */
float CGMXmlNode::GetPropFloat(const std::string& strPropertyName, float fDefault) const
{
	if (m_pNode)
	{
		double fValue;
		if (m_pNode->Attribute(strPropertyName.c_str(), &fValue))
			return fValue;
	}
	return fDefault;
}

/** @brief 获取Double属性 */
double CGMXmlNode::GetPropDouble(const std::string& strPropertyName, double fDefault) const
{
	if (m_pNode)
	{
		double fValue;
		if (m_pNode->Attribute(strPropertyName.c_str(), &fValue))
			return fValue;
	}
	return fDefault;
}

/** @brief 获取Vector2属性 */
SGMVector2 CGMXmlNode::GetPropVector2(const std::string& strPropertyName, const SGMVector2& vDefault) const
{
	if (m_pNode)
	{
		const char* value = m_pNode->Attribute(strPropertyName.c_str());
		if (value)
		{
			std::string str = value;
			size_t nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;

			SGMVector2 vValue;
			vValue.x = std::stod(str.substr(0, nPos));
			str = str.substr(nPos + 1);
			vValue.y = std::stod(str);
			return vValue;
		}
	}
	return vDefault;
}
/** @brief 获取Vector2i属性 */
SGMVector2i CGMXmlNode::GetPropVector2i(const std::string& strPropertyName, const SGMVector2i& vDefault) const
{
	if (m_pNode)
	{
		const char* value = m_pNode->Attribute(strPropertyName.c_str());
		if (value)
		{
			std::string str = value;
			size_t nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;

			SGMVector2i vValue;
			vValue.x = std::stoi(str.substr(0, nPos));
			str = str.substr(nPos + 1);
			vValue.y = std::stoi(str);
			return vValue;
		}
	}
	return vDefault;
}
/** @brief 获取Vector2f属性 */
SGMVector2f CGMXmlNode::GetPropVector2f(const std::string& strPropertyName, const SGMVector2f& vDefault) const
{
	if (m_pNode)
	{
		const char* value = m_pNode->Attribute(strPropertyName.c_str());
		if (value)
		{
			std::string str = value;
			size_t nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;

			SGMVector2f vValue;
			vValue.x = std::stof(str.substr(0, nPos));
			str = str.substr(nPos + 1);
			vValue.y = std::stof(str);
			return vValue;
		}
	}
	return vDefault;
}

/** @brief 获取Vector3属性 */
SGMVector3 CGMXmlNode::GetPropVector3(const std::string& strPropertyName, const SGMVector3& vDefault) const
{
	if (m_pNode)
	{
		const char* value = m_pNode->Attribute(strPropertyName.c_str());
		if (value)
		{
			std::string str = value;

			size_t nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;
			SGMVector3 vValue;
			vValue.x = std::stod(str.substr(0, nPos));
			str = str.substr(nPos + 1);

			nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;
			vValue.y = std::stod(str.substr(0, nPos));
			str = str.substr(nPos + 1);

			vValue.z = std::stod(str);
			return vValue;
		}
	}
	return vDefault;
}
/** @brief 获取Vector3i属性 */
SGMVector3i CGMXmlNode::GetPropVector3i(const std::string& strPropertyName, const SGMVector3i& vDefault) const
{
	if (m_pNode)
	{
		const char* value = m_pNode->Attribute(strPropertyName.c_str());
		if (value)
		{
			std::string str = value;

			size_t nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;
			SGMVector3i vValue;
			vValue.x = std::stoi(str.substr(0, nPos));
			str = str.substr(nPos + 1);

			nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;
			vValue.y = std::stoi(str.substr(0, nPos));
			str = str.substr(nPos + 1);

			vValue.z = std::stoi(str);
			return vValue;
		}
	}
	return vDefault;
}
/** @brief 获取Vector3f属性 */
SGMVector3f CGMXmlNode::GetPropVector3f(const std::string& strPropertyName, const SGMVector3f& vDefault) const
{
	if (m_pNode)
	{
		const char* value = m_pNode->Attribute(strPropertyName.c_str());
		if (value)
		{
			std::string str = value;

			size_t nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;
			SGMVector3f vValue;
			vValue.x = std::stof(str.substr(0, nPos));
			str = str.substr(nPos + 1);

			nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;
			vValue.y = std::stof(str.substr(0, nPos));
			str = str.substr(nPos + 1);

			vValue.z = std::stof(str);
			return vValue;
		}
	}
	return vDefault;
}

/** @brief 获取Vector4属性 */
SGMVector4 CGMXmlNode::GetPropVector4(const std::string& strPropertyName, const SGMVector4& vDefault) const
{
	if (m_pNode)
	{
		const char* value = m_pNode->Attribute(strPropertyName.c_str());
		if (value)
		{
			std::string str = value;

			size_t nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;
			SGMVector4 vValue;
			vValue.x = std::stod(str.substr(0, nPos));
			str = str.substr(nPos + 1);

			nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;
			vValue.y = std::stod(str.substr(0, nPos));
			str = str.substr(nPos + 1);

			nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;
			vValue.z = std::stod(str.substr(0, nPos));
			str = str.substr(nPos + 1);

			vValue.w = std::stod(str);
			return vValue;
		}
	}
	return vDefault;
}

/** @brief 获取Vector4i属性 */
SGMVector4i CGMXmlNode::GetPropVector4i(const std::string& strPropertyName, const SGMVector4i& vDefault) const
{
	if (m_pNode)
	{
		const char* value = m_pNode->Attribute(strPropertyName.c_str());
		if (value)
		{
			std::string str = value;

			size_t nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;
			SGMVector4i vValue;
			vValue.x = std::stoi(str.substr(0, nPos));
			str = str.substr(nPos + 1);

			nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;
			vValue.y = std::stoi(str.substr(0, nPos));
			str = str.substr(nPos + 1);

			nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;
			vValue.z = std::stoi(str.substr(0, nPos));
			str = str.substr(nPos + 1);

			vValue.w = std::stoi(str);
			return vValue;
		}
	}
	return vDefault;
}
/** @brief 获取Vector4f属性 */
SGMVector4f CGMXmlNode::GetPropVector4f(const std::string& strPropertyName, const SGMVector4f& vDefault) const
{
	if (m_pNode)
	{
		const char* value = m_pNode->Attribute(strPropertyName.c_str());
		if (value)
		{
			std::string str = value;

			size_t nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;
			SGMVector4f vValue;
			vValue.x = std::stof(str.substr(0, nPos));
			str = str.substr(nPos + 1);

			nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;
			vValue.y = std::stof(str.substr(0, nPos));
			str = str.substr(nPos + 1);

			nPos = str.find(" ", 0);
			if (nPos == std::string::npos)
				return vDefault;
			vValue.z = std::stof(str.substr(0, nPos));
			str = str.substr(nPos + 1);

			vValue.w = std::stof(str);
			return vValue;
		}
	}
	return vDefault;
}

const wchar_t* CGMXmlNode::_CharToWChar(const char * cstr) const
{
	if (!cstr) return L"";
	int length = int(strlen(cstr)) + 1;
	wchar_t *t = (wchar_t*)malloc(sizeof(wchar_t) * length);
	if (!t) return L"";
	memset(t, 0, length * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, cstr, int(strlen(cstr)), t, length);
	return t;
}

const char* CGMXmlNode::_WCharToChar(const wchar_t * wstr) const
{
	int destLen = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	if (destLen <= 0)
	{
		return "";
	}
	char* sDest = (char*)malloc(sizeof(char) * destLen);
	if (!sDest) return "";
	memset(sDest, 0, destLen * sizeof(char));
	destLen = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, sDest, destLen, NULL, NULL);
	if (destLen <= 0)
	{
		return "";
	}
	return sDest;
}

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
	m_pDoc = new TiXmlDocument(strPathName.c_str());
	const std::string strHead = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
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
		std::cout << "WARNING: File " << strPathName.c_str() << " is not found.\n";
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

CGMXmlNode CGMXml::AddChild(const std::string & strChildName)
{
	return CGMXmlNode(this, m_pRoot).AddChild(strChildName);
}

CGMXmlNode CGMXml::GetChild(const std::string& strChildName)
{
	return CGMXmlNode(this, m_pRoot).GetChild(strChildName);
}

VGMXmlNodeVec CGMXml::GetChildren(const std::string& strChildName)
{
	return CGMXmlNode(this, m_pRoot).GetChildren(strChildName);
}