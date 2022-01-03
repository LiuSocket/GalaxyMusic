//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMKit.cpp
/// @brief		Galaxy-Music Engine - GMGalaxy
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.05
//////////////////////////////////////////////////////////////////////////

#include "GMKit.h"
#include <osgDB/ReadFile>

using namespace GM;

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
bool CGMKit::LoadShader(
	osg::StateSet* pStateSet
	, std::string& vertFilePath
	, std::string& fragFilePath
	, std::string& geomFilePath
	, bool bPixelLighting)
{
	osg::ref_ptr<osg::Program> pProgram = new osg::Program;

	osg::Shader *pVertShader = new osg::Shader;
	pVertShader->setType(osg::Shader::VERTEX);
	std::string vertOut = _ReadShaderFile(vertFilePath);
	pVertShader->setShaderSource(vertOut);

	osg::Shader *pFragShader = new osg::Shader;
	pFragShader->setType(osg::Shader::FRAGMENT);
	std::string fragOut = _ReadShaderFile(fragFilePath);
	pFragShader->setShaderSource(fragOut);

	osg::Shader *pGeomShader = new osg::Shader;
	pGeomShader->setType(osg::Shader::GEOMETRY);
	std::string geomOut = _ReadShaderFile(geomFilePath);
	pGeomShader->setShaderSource(geomOut);

	if (bPixelLighting)
	{
		pProgram->addBindAttribLocation("tangent", 6);
		pProgram->addBindAttribLocation("binormal", 7);
	}
	if (pVertShader && pFragShader && pGeomShader)
	{
		pProgram->addShader(pVertShader);
		pProgram->addShader(pFragShader);
		pProgram->addShader(pGeomShader);
		pStateSet->setAttributeAndModes(pProgram.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		return true;
	}
	else
	{
		return false;
	}
}

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
bool CGMKit::LoadShader(osg::StateSet* pStateSet, std::string& vertFilePath, std::string& fragFilePath)
{
	osg::ref_ptr<osg::Program> pProgram = new osg::Program;

	osg::Shader *pVertShader = new osg::Shader;
	pVertShader->setType(osg::Shader::VERTEX);
	std::string vertOut = _ReadShaderFile(vertFilePath);
	pVertShader->setShaderSource(vertOut);

	osg::Shader *pFragShader = new osg::Shader;
	pFragShader->setType(osg::Shader::FRAGMENT);
	std::string fragOut = _ReadShaderFile(fragFilePath);
	pFragShader->setShaderSource(fragOut);

	if (pVertShader && pFragShader)
	{
		pProgram->addShader(pVertShader);
		pProgram->addShader(pFragShader);
		pStateSet->setAttributeAndModes(pProgram.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		return true;
	}
	else
	{
		return false;
	}
}

/** Replaces all the instances of "sub" with "other" in "s". */
std::string& CGMKit::_ReplaceIn(std::string& s, const std::string& sub, const std::string& other)
{
	if (sub.empty()) return s;
	size_t b = 0;
	for (; ; )
	{
		b = s.find(sub, b);
		if (b == s.npos) break;
		s.replace(b, sub.size(), other);
		b += other.size();
	}
	return s;
}

std::string CGMKit::_ReadShaderFile(std::string& filePath)
{
	FILE* vertStream = fopen(filePath.data(), "rb");
	if (vertStream == NULL) return std::string();

	fseek(vertStream, 0, SEEK_END);
	int fileLen = ftell(vertStream);
	char* strShader = (char *)malloc(sizeof(char) * (fileLen + 1));
	strShader[fileLen] = 0;
	fseek(vertStream, 0, SEEK_SET);
	fread(strShader, fileLen, sizeof(char), vertStream);
	fclose(vertStream);

	std::string out = strShader;
	// ���ַ����е� "\r" �滻�� ""�����ڽ�β�� "\n"
	_ReplaceIn(out, "\r", "");
	out += "\n";

	return out;
}