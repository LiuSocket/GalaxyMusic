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
	class CGMXmlNode;
	typedef std::vector<CGMXmlNode>			VGMXmlNodeVec;

	/*************************************************************************
	 Class
	*************************************************************************/
	class CGMXml;
	/*!
	 *  @class CGMXmlNode
	 *  @brief Galaxy-Music Xml Node
	 */
	class CGMXmlNode
	{
		friend CGMXml;
		// 函数
	public:
		/** @brief 构造 */
		CGMXmlNode(CGMXml* pXML = nullptr, TiXmlElement* pNode = nullptr)
			: m_pXml(pXML), m_pNode(pNode)
		{}
		/** @brief 析构 */
		~CGMXmlNode(){}

		/** @brief 是否为空 */
		bool IsEmpty() { return m_pNode == nullptr ? true : false; }
		/** @brief 获取节点name,如果节点为空，返回false */
		bool GetName(std::string& nodeName);
		/** @brief 设置子节点 */
		CGMXmlNode AddChild(const std::string& strChildName);

		/** @brief 设置char *属性 */
		void SetPropStr(const std::string& strPropertyName, const char* strPropertyValue);
		/** @brief 设置wchar_t *属性 */
		void SetPropWStr(const std::string& strPropertyName, const wchar_t* strPropertyValue);
		/** @brief 设置Bool属性 */
		void SetPropBool(const std::string& strPropertyName, bool bPropertyValue);
		/** @brief 设置Int属性 */
		void SetPropInt(const std::string& strPropertyName, int iPropertyValue);
		/** @brief 设置Unsigned Int属性 */
		void SetPropUInt(const std::string& strPropertyName, unsigned int iPropertyValue);
		/** @brief 设置Float属性 */
		void SetPropFloat(const std::string& strPropertyName, float fPropertyValue);
		/** @brief 设置Double属性 */
		void SetPropDouble(const std::string& strPropertyName, double fPropertyValue);
		/** @brief 设置Vector2属性 */
		void SetPropVector2(const std::string& strPropertyName, const SGMVector2 & vPropertyValue);
		void SetPropVector2(const std::string& strPropertyName, const SGMVector2i& vPropertyValue);
		void SetPropVector2(const std::string& strPropertyName, const SGMVector2f& vPropertyValue);
		/** @brief 设置Vector3属性 */
		void SetPropVector3(const std::string& strPropertyName, const SGMVector3 & vPropertyValue);
		void SetPropVector3(const std::string& strPropertyName, const SGMVector3i& vPropertyValue);
		void SetPropVector3(const std::string& strPropertyName, const SGMVector3f& vPropertyValue);
		/** @brief 设置Vector4属性 */
		void SetPropVector4(const std::string& strPropertyName, const SGMVector4 & vPropertyValue);
		void SetPropVector4(const std::string& strPropertyName, const SGMVector4i& vPropertyValue);
		void SetPropVector4(const std::string& strPropertyName, const SGMVector4f& vPropertyValue);

		/** @brief 设置Enum属性 */
		template<typename T>
		void SetPropEnum(const std::string& strPropertyName, T ePropertyValue)
		{
			SetPropUInt(strPropertyName, (unsigned int)ePropertyValue);
		}

		/** @brief 获取子节点 */
		CGMXmlNode GetChild(const std::string& strChildName) const;
		/** @brief 获取子节点组 */
		VGMXmlNodeVec GetChildren(const std::string& strChildName) const;
		/** @brief 节点是否设置了某属性  */
		bool HasProperty(const std::string& propertyName) const;
		/** @brief 获取String属性 */
		const char* GetPropStr(const std::string& strPropertyName, const char* strDefault = "") const;
		/** @brief 获取WString属性 */
		const wchar_t* GetPropWStr(const std::string& strPropertyName, const wchar_t* strDefault = L"") const;
		/** @brief 获取Bool属性 */
		bool GetPropBool(const std::string& strPropertyName, bool bDefault = false) const;
		/** @brief 获取Int属性 */
		int GetPropInt(const std::string& strPropertyName, int nDefault = 0) const;
		/** @brief 获取Unsigned Int属性 */
		unsigned int GetPropUInt(const std::string& strPropertyName, unsigned int nDefault = 0) const;
		/** @brief 获取Float属性 */
		float GetPropFloat(const std::string& strPropertyName, float fDefault = 0) const;
		/** @brief 获取Double属性 */
		double GetPropDouble(const std::string& strPropertyName, double fDefault = 0) const;
		/** @brief 获取Vector2属性 */
		SGMVector2  GetPropVector2(const std::string& strPropertyName, const SGMVector2&  vDefault = SGMVector2()) const;
		SGMVector2i GetPropVector2i(const std::string& strPropertyName, const SGMVector2i& vDefault = SGMVector2i()) const;
		SGMVector2f GetPropVector2f(const std::string& strPropertyName, const SGMVector2f& vDefault = SGMVector2f()) const;
		/** @brief 获取Vector3属性 */
		SGMVector3  GetPropVector3(const std::string& strPropertyName, const SGMVector3&  vDefault = SGMVector3()) const;
		SGMVector3i GetPropVector3i(const std::string& strPropertyName, const SGMVector3i& vDefault = SGMVector3i()) const;
		SGMVector3f GetPropVector3f(const std::string& strPropertyName, const SGMVector3f& vDefault = SGMVector3f()) const;
		/** @brief 获取Vector4f属性 */
		SGMVector4  GetPropVector4(const std::string& strPropertyName, const SGMVector4&  vDefault = SGMVector4()) const;
		SGMVector4i GetPropVector4i(const std::string& strPropertyName, const SGMVector4i& vDefault = SGMVector4i()) const;
		SGMVector4f GetPropVector4f(const std::string& strPropertyName, const SGMVector4f& vDefault = SGMVector4f()) const;
		/** @brief 获取Enum属性 */
		template<typename T>
		T GetPropEnum(const std::string& strPropertyName, T eDefault) const
		{
			return (T)GetPropUInt(strPropertyName, (unsigned int)eDefault);
		}

		// 函数
	private:
		/**
		* 字符串转换 char* to wchar_t*
		* @param cstr:	多字节字符串
		* @return wchar_t 宽字节字符串
		*/
		const wchar_t* _CharToWChar(const char* cstr) const;
		/**
		* 字符串转换 wchar_t* to char*
		* @param wstr:	宽字节字符串
		* @return char 多字节字符串
		*/
		const char* _WCharToChar(const wchar_t* wstr) const;

		// 变量
	private:
		CGMXml*							m_pXml;			//!< XML指针
		TiXmlElement*					m_pNode;		//!< 节点指针
	};

	/*!
 	 *  @class CGMXml
	 *  @brief GalaxyMusic Xml Manager
	 */
	class CGMXml
	{
	// 函数
	public:
		/** @brief 构造 */
		CGMXml();
		/** @brief 析构 */
		~CGMXml();

		/** @brief 创建 */
		bool Create(const std::string& strPathName, const std::string& strRootName);
		/** @brief 加载 */
		bool Load(const std::string& strPathName, const std::string& strRootName);
		/** @brief 保存 */
		bool Save();

		/** @brief 添加子节点 */
		CGMXmlNode AddChild(const std::string& strChildName);
		/** @brief 获取第一个符合条件的子节点 */
		CGMXmlNode GetChild(const std::string& strChildName);
		/** @brief 获取子节点组 */
		VGMXmlNodeVec GetChildren(const std::string& strChildName);

	// 变量
	private:
		TiXmlDocument*				m_pDoc;		//!< 文档指针
		TiXmlElement*				m_pRoot;	//!< 根节点
	};

}	// GM
