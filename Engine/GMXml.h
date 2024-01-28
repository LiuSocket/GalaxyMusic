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
		// ����
	public:
		/** @brief ���� */
		CGMXmlNode(CGMXml* pXML = nullptr, TiXmlElement* pNode = nullptr)
			: m_pXml(pXML), m_pNode(pNode)
		{}
		/** @brief ���� */
		~CGMXmlNode(){}

		/** @brief �Ƿ�Ϊ�� */
		bool IsEmpty() { return m_pNode == nullptr ? true : false; }
		/** @brief ��ȡ�ڵ�name,����ڵ�Ϊ�գ�����false */
		bool GetName(std::string& nodeName);
		/** @brief �����ӽڵ� */
		CGMXmlNode AddChild(const std::string& strChildName);

		/** @brief ����char *���� */
		void SetPropStr(const std::string& strPropertyName, const char* strPropertyValue);
		/** @brief ����wchar_t *���� */
		void SetPropWStr(const std::string& strPropertyName, const wchar_t* strPropertyValue);
		/** @brief ����Bool���� */
		void SetPropBool(const std::string& strPropertyName, bool bPropertyValue);
		/** @brief ����Int���� */
		void SetPropInt(const std::string& strPropertyName, int iPropertyValue);
		/** @brief ����Unsigned Int���� */
		void SetPropUInt(const std::string& strPropertyName, unsigned int iPropertyValue);
		/** @brief ����Float���� */
		void SetPropFloat(const std::string& strPropertyName, float fPropertyValue);
		/** @brief ����Double���� */
		void SetPropDouble(const std::string& strPropertyName, double fPropertyValue);
		/** @brief ����Vector2���� */
		void SetPropVector2(const std::string& strPropertyName, const SGMVector2 & vPropertyValue);
		void SetPropVector2(const std::string& strPropertyName, const SGMVector2i& vPropertyValue);
		void SetPropVector2(const std::string& strPropertyName, const SGMVector2f& vPropertyValue);
		/** @brief ����Vector3���� */
		void SetPropVector3(const std::string& strPropertyName, const SGMVector3 & vPropertyValue);
		void SetPropVector3(const std::string& strPropertyName, const SGMVector3i& vPropertyValue);
		void SetPropVector3(const std::string& strPropertyName, const SGMVector3f& vPropertyValue);
		/** @brief ����Vector4���� */
		void SetPropVector4(const std::string& strPropertyName, const SGMVector4 & vPropertyValue);
		void SetPropVector4(const std::string& strPropertyName, const SGMVector4i& vPropertyValue);
		void SetPropVector4(const std::string& strPropertyName, const SGMVector4f& vPropertyValue);

		/** @brief ����Enum���� */
		template<typename T>
		void SetPropEnum(const std::string& strPropertyName, T ePropertyValue)
		{
			SetPropUInt(strPropertyName, (unsigned int)ePropertyValue);
		}

		/** @brief ��ȡ�ӽڵ� */
		CGMXmlNode GetChild(const std::string& strChildName) const;
		/** @brief ��ȡ�ӽڵ��� */
		VGMXmlNodeVec GetChildren(const std::string& strChildName) const;
		/** @brief �ڵ��Ƿ�������ĳ����  */
		bool HasProperty(const std::string& propertyName) const;
		/** @brief ��ȡString���� */
		const char* GetPropStr(const std::string& strPropertyName, const char* strDefault = "") const;
		/** @brief ��ȡWString���� */
		const wchar_t* GetPropWStr(const std::string& strPropertyName, const wchar_t* strDefault = L"") const;
		/** @brief ��ȡBool���� */
		bool GetPropBool(const std::string& strPropertyName, bool bDefault = false) const;
		/** @brief ��ȡInt���� */
		int GetPropInt(const std::string& strPropertyName, int nDefault = 0) const;
		/** @brief ��ȡUnsigned Int���� */
		unsigned int GetPropUInt(const std::string& strPropertyName, unsigned int nDefault = 0) const;
		/** @brief ��ȡFloat���� */
		float GetPropFloat(const std::string& strPropertyName, float fDefault = 0) const;
		/** @brief ��ȡDouble���� */
		double GetPropDouble(const std::string& strPropertyName, double fDefault = 0) const;
		/** @brief ��ȡVector2���� */
		SGMVector2  GetPropVector2(const std::string& strPropertyName, const SGMVector2&  vDefault = SGMVector2()) const;
		SGMVector2i GetPropVector2i(const std::string& strPropertyName, const SGMVector2i& vDefault = SGMVector2i()) const;
		SGMVector2f GetPropVector2f(const std::string& strPropertyName, const SGMVector2f& vDefault = SGMVector2f()) const;
		/** @brief ��ȡVector3���� */
		SGMVector3  GetPropVector3(const std::string& strPropertyName, const SGMVector3&  vDefault = SGMVector3()) const;
		SGMVector3i GetPropVector3i(const std::string& strPropertyName, const SGMVector3i& vDefault = SGMVector3i()) const;
		SGMVector3f GetPropVector3f(const std::string& strPropertyName, const SGMVector3f& vDefault = SGMVector3f()) const;
		/** @brief ��ȡVector4f���� */
		SGMVector4  GetPropVector4(const std::string& strPropertyName, const SGMVector4&  vDefault = SGMVector4()) const;
		SGMVector4i GetPropVector4i(const std::string& strPropertyName, const SGMVector4i& vDefault = SGMVector4i()) const;
		SGMVector4f GetPropVector4f(const std::string& strPropertyName, const SGMVector4f& vDefault = SGMVector4f()) const;
		/** @brief ��ȡEnum���� */
		template<typename T>
		T GetPropEnum(const std::string& strPropertyName, T eDefault) const
		{
			return (T)GetPropUInt(strPropertyName, (unsigned int)eDefault);
		}

		// ����
	private:
		/**
		* �ַ���ת�� char* to wchar_t*
		* @param cstr:	���ֽ��ַ���
		* @return wchar_t ���ֽ��ַ���
		*/
		const wchar_t* _CharToWChar(const char* cstr) const;
		/**
		* �ַ���ת�� wchar_t* to char*
		* @param wstr:	���ֽ��ַ���
		* @return char ���ֽ��ַ���
		*/
		const char* _WCharToChar(const wchar_t* wstr) const;

		// ����
	private:
		CGMXml*							m_pXml;			//!< XMLָ��
		TiXmlElement*					m_pNode;		//!< �ڵ�ָ��
	};

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
		CGMXmlNode AddChild(const std::string& strChildName);
		/** @brief ��ȡ��һ�������������ӽڵ� */
		CGMXmlNode GetChild(const std::string& strChildName);
		/** @brief ��ȡ�ӽڵ��� */
		VGMXmlNodeVec GetChildren(const std::string& strChildName);

	// ����
	private:
		TiXmlDocument*				m_pDoc;		//!< �ĵ�ָ��
		TiXmlElement*				m_pRoot;	//!< ���ڵ�
	};

}	// GM
