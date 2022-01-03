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
		TiXmlElement AddChild(const std::string& strChildName,
			const char* strText, const double fRadius, const double fAngle);
		/** @brief 获取第一个符合条件的子节点 */
		TiXmlElement GetChild(const std::string& strChildName);
		/** @brief 获取子节点组 */
		VGMXmlElementVec GetChildren(const std::string& strChildName);

	// 变量
	private:
		TiXmlDocument*				m_pDoc;		//!< 文档指针
		TiXmlElement*				m_pRoot;	//!< 根节点
	};

}	// GM
