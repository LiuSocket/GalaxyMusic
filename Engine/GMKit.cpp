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

bool CGMKit::LoadShader(
	osg::StateSet* pStateSet,
	const std::string& vertFilePath,
	const std::string& fragFilePath,
	const std::string& geomFilePath,
	const std::string& strShaderName,
	bool bPixelLighting)
{
	osg::ref_ptr<osg::Program> pProgram = new osg::Program;
	pProgram->setName(strShaderName);

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

bool CGMKit::LoadShaderWithCommonFrag(osg::StateSet * pStateSet,
	const std::string & vertFilePath,
	const std::string & fragFilePath,
	const std::string & fragCommonFilePath,
	const std::string & shaderName,
	const bool bPixelLighting)
{
	osg::ref_ptr<osg::Program> pProgram = new osg::Program;
	pProgram->setName(shaderName);

	osg::Shader *pVertShader = new osg::Shader;
	pVertShader->setType(osg::Shader::VERTEX);
	std::string vertOut = _ReadShaderFile(vertFilePath);
	pVertShader->setShaderSource(vertOut);

	osg::Shader *pFragShader = new osg::Shader;
	pFragShader->setType(osg::Shader::FRAGMENT);
	std::string fragOut = _ReadShaderFile(fragFilePath);

	if ("" != fragCommonFilePath)
	{
		std::string fragCommonOut = _ReadShaderFile(fragCommonFilePath);
		fragOut = fragCommonOut + fragOut;
	}
	pFragShader->setShaderSource(fragOut);

	if (bPixelLighting)
	{
		pProgram->addBindAttribLocation("tangent", 6);
		pProgram->addBindAttribLocation("binormal", 7);
	}
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

bool CGMKit::LoadShader(
	osg::StateSet* pStateSet,
	const std::string& vertFilePath,
	const std::string& fragFilePath,
	const std::string& strShaderName)
{
	osg::ref_ptr<osg::Program> pProgram = new osg::Program;
	pProgram->setName(strShaderName);

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

bool CGMKit::LoadComputeShader(
	osg::StateSet * pStateSet,
	const std::string& compFilePath,
	const std::string& shaderName)
{
	if (!pStateSet || "" == compFilePath) return false;

	std::string strComputeSrc = _ReadShaderFile(compFilePath);
	if ("" == strComputeSrc) return false;

	osg::ref_ptr<osg::Program> pProgram = new osg::Program;
	pProgram->setName(shaderName);
	pProgram->addShader(new osg::Shader(osg::Shader::COMPUTE, strComputeSrc));
	pStateSet->setAttributeAndModes(pProgram.get(), osg::StateAttribute::ON);

	return true;
}

bool CGMKit::AddTexture(osg::StateSet* pStateSet, osg::Texture* pTex, const char* texName, const int iUnit)
{
	if (!pStateSet || !pTex || ("" == texName) || (iUnit < 0)) 	return false;

	pStateSet->setTextureAttributeAndModes(iUnit, pTex);
	osg::ref_ptr<osg::Uniform> pUniform = new osg::Uniform(texName, iUnit);
	pStateSet->addUniform(pUniform.get());

	return true;
}

osg::Vec4f CGMKit::GetImageColor(const osg::Image* pImg, const float fX, const float fY, const bool bLinear)
{
	if (fX < 0 || fX > 1 || fY < 0 || fY > 1 || !pImg)
	{
		return osg::Vec4f(0, 0, 0, 0);
	}

	unsigned int iWidth = pImg->s();
	unsigned int iHeight = pImg->t();

	float fS = fX * (iWidth - 1);
	float fT = fY * (iHeight - 1);

	float fDeltaS = fS - (int)fS;
	float fDeltaT = fT - (int)fT;
	unsigned int s = (unsigned int)fS;
	unsigned int t = (unsigned int)fT;
	osg::Vec4f vValue = pImg->getColor(s, t);
	if (bLinear)
	{
		unsigned int s_next = (s == iWidth - 1) ? (iWidth - 1) : (s + 1);
		unsigned int t_next = (t == iHeight - 1) ? (iHeight - 1) : (t + 1);

		osg::Vec4f vValue_01 = pImg->getColor(s_next, t);
		osg::Vec4f vValue_10 = pImg->getColor(s, t_next);
		osg::Vec4f vValue_11 = pImg->getColor(s_next, t_next);

		vValue = Mix(Mix(vValue, vValue_01, fDeltaS), Mix(vValue_10, vValue_11, fDeltaS), fDeltaT);
	}
	return vValue;
}

float CGMKit::Half_2_Float(const unsigned short x)
{ // IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
	const unsigned int e = (x & 0x7C00) >> 10; // exponent
	const unsigned int m = (x & 0x03FF) << 13; // mantissa
	const unsigned int v = AsUint((float)m) >> 23; // evil log2 bit hack to count leading zeros in denormalized format
	return AsFloat((x & 0x8000) << 16 | (e != 0) * ((e + 112) << 23 | m) | ((e == 0) & (m != 0)) * ((v - 37) << 23 | ((m << (150 - v)) & 0x007FE000))); // sign : normalized : denormalized
}

unsigned short CGMKit::Float_2_Half(const float x)
{ // IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
	const unsigned int b = AsUint(x) + 0x00001000; // round-to-nearest-even: add last bit after truncated mantissa
	const unsigned int e = (b & 0x7F800000) >> 23; // exponent
	const unsigned int m = b & 0x007FFFFF; // mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
	return (b & 0x80000000) >> 16 | (e > 112) * ((((e - 112) << 10) & 0x7C00) | m >> 13) | ((e < 113) & (e > 101)) * ((((0x007FF000 + m) >> (125 - e)) + 1) >> 1) | (e > 143) * 0x7FFF; // sign : normalized : denormalized : saturate
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

std::string CGMKit::_ReadShaderFile(const std::string& filePath)
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
	// 将字符串中的 "\r" 替换成 ""，并在结尾加 "\n"
	_ReplaceIn(out, "\r", "");
	out += "\n";

	return out;
}