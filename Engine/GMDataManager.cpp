//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMDataManager.cpp
/// @brief		Galaxy-Music Engine - GMDataManager
/// @version	1.0
/// @author		LiuTao
/// @date		2021.06.20
//////////////////////////////////////////////////////////////////////////

#include "GMDataManager.h"
#include "GMXml.h"
#include <io.h>

using namespace GM;

/*************************************************************************
Structs
*************************************************************************/

/*************************************************************************
CGMDataManager Methods
*************************************************************************/

/** @brief 构造 */
CGMDataManager::CGMDataManager() :
	m_pConfigData(nullptr),
	m_strAudioPath(L"Music/"), m_strAudioName(L""),
	m_formatVector({ L"mp3", L"wma", L"wav", L"ogg" }),
	m_vOffsetVector({ osg::Vec2i(0,0),
		osg::Vec2i(1,0), osg::Vec2i(-1,0), osg::Vec2i(0,1), osg::Vec2i(0,-1),
		osg::Vec2i(1,1), osg::Vec2i(-1,-1), osg::Vec2i(-1,1), osg::Vec2i(1,-1),
		osg::Vec2i(2,0), osg::Vec2i(-2,0), osg::Vec2i(0,2), osg::Vec2i(0,-2), 
		osg::Vec2i(2,1), osg::Vec2i(-2,-1), osg::Vec2i(-1,2), osg::Vec2i(1,-2), 
		osg::Vec2i(2,-1), osg::Vec2i(-2,1), osg::Vec2i(-1,-2), osg::Vec2i(1,2), 
		osg::Vec2i(2,2), osg::Vec2i(-2,-2), osg::Vec2i(2,-2), osg::Vec2i(-2,2), 
		osg::Vec2i(3,0), osg::Vec2i(-3,0), osg::Vec2i(0,3), osg::Vec2i(0,-3), 
		osg::Vec2i(3,1), osg::Vec2i(-3,-1), osg::Vec2i(-1,3), osg::Vec2i(1,-3), 
		osg::Vec2i(3,-1), osg::Vec2i(-3,1), osg::Vec2i(1,3), osg::Vec2i(-1,-3),
		osg::Vec2i(4,0), osg::Vec2i(-4,0), osg::Vec2i(0,4), osg::Vec2i(0,-4),
		osg::Vec2i(3,2), osg::Vec2i(-3,-2), osg::Vec2i(-2,3), osg::Vec2i(2,-3),
		osg::Vec2i(2,3), osg::Vec2i(-2,-3), osg::Vec2i(-3,2), osg::Vec2i(3,-2),
		osg::Vec2i(4,1), osg::Vec2i(-4,-1), osg::Vec2i(-1,4), osg::Vec2i(1,-4),
		osg::Vec2i(4,-1), osg::Vec2i(-4,1), osg::Vec2i(-1,-4), osg::Vec2i(1,4)
		})
{
}

/** @brief 析构 */
CGMDataManager::~CGMDataManager()
{
	m_audioDataMap.clear();
}

/** @brief 初始化 */
bool CGMDataManager::Init(SGMConfigData * pConfigData)
{
	m_pConfigData = pConfigData;

	// 读取已经保存的音频坐标
	_RefreshAudioCoordinates();
	// 扫描Data/Media/Music路径下所有支持的音频文件，并将文件名保存在m_fileVector中
	_RefreshAudioFiles();
	return true;
}

/** @brief 保存 */
bool CGMDataManager::Save()
{
	return _SaveAudioCoordinates();
}

bool CGMDataManager::GetStarCoordVector(std::vector<SGMStarCoord>& coordV)
{
	coordV = m_storedCoordVector;
	return true;
}

bool CGMDataManager::FindAudio(const std::wstring & strName)
{
	auto itr = std::find(m_storedNameVector.begin(), m_storedNameVector.end(), strName);
	if (itr != m_storedNameVector.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

std::wstring CGMDataManager::FindAudio(double& fX, double& fY, double& fZ)
{
	int iX = fX * GM_COORD_MAX;
	int iY = fY * GM_COORD_MAX;
	for (auto offsetItr : m_vOffsetVector)
	{
		SGMStarCoord starCoord(iX + offsetItr.x(), iY + offsetItr.y(), 0);
		auto itr = m_audioDataMap.find(starCoord);
		if (itr != m_audioDataMap.end())
		{
			fX = double(starCoord.x) / GM_COORD_MAX;
			fY = double(starCoord.y) / GM_COORD_MAX;

			m_strAudioName = itr->second.name;
			if (m_playedVector.empty() || m_strAudioName != m_playedVector.back())
			{
				m_playedVector.push_back(m_strAudioName);
			}
			return m_strAudioName;
		}
	}
	return L"";
}

std::wstring CGMDataManager::FindAudio(const int iID)
{
	if (iID < m_storedNameVector.size())
	{
		m_strAudioName = m_storedNameVector.at(iID);
		if (m_playedVector.empty() || m_strAudioName != m_playedVector.back())
		{
			m_playedVector.push_back(m_strAudioName);
		}
		return m_strAudioName;
	}
	else
	{
		return L"";
	}
}

std::wstring CGMDataManager::FindLastAudio()
{
	if (m_playedVector.empty())
	{
		return L"";
	}

	auto itr = std::find(m_playedVector.begin(), m_playedVector.end(), m_strAudioName);
	if (itr != m_playedVector.end() && itr->data() != m_playedVector.front())
	{
		m_strAudioName = (itr - 1)->data();
		return m_strAudioName;
	}
	else
	{
		return L"";
	}
}

int CGMDataManager::GetCurrentAudioID()
{
	if (L"" == m_strAudioName)
	{
		return -1;
	}

	int i = 0;
	for (auto itr = m_storedNameVector.begin();
		itr != m_storedNameVector.end();
		itr++, i++)
	{
		if (m_strAudioName == itr->data())
		{
			return i;
		}
	}
	return 0;
}

osg::Vec4f CGMDataManager::GetAudioColor(const SGMAudioCoord audioCoord)
{
	// 0.0f - 1.0f
	float fAngle = audioCoord.angle / (2*osg::PI);
	return osg::Vec4f(
		osg::clampBetween(std::abs(4.0f * fAngle - 2.5f) - 0.5f, 0.0f, 1.0f),
		osg::clampBetween(1.5f - std::abs(4.0f * fAngle - 1.5f), 0.0f, 1.0f),
		1.0f - osg::clampBetween(std::abs(4.0f*fAngle - 3.0f), 0.0f, 1.0f),
		1);
}

osg::Vec4f CGMDataManager::GetAudioColor(const SGMStarCoord starCoord)
{
	auto itr = m_audioDataMap.find(starCoord);
	if (itr != m_audioDataMap.end())
	{
		SGMAudioCoord audioCoord = itr->second.audioCoord;
		return GetAudioColor(audioCoord);
	}
	return osg::Vec4f(1, 1, 1, 1);
}

SGMStarCoord CGMDataManager::GetStarCoord()
{
	if (L"" == m_strAudioName)
	{
		return SGMStarCoord();
	}
	else
	{
		return GetStarCoord(m_strAudioName);
	}
}

SGMStarCoord CGMDataManager::GetStarCoord(const std::wstring & strName)
{
	int i = 0;
	for (auto itr = m_storedNameVector.begin();
		itr != m_storedNameVector.end();
		itr++,i++)
	{
		if (strName == itr->data())
		{
			return m_storedCoordVector.at(i);
		}
	}
	return SGMStarCoord();
}

SGMAudioCoord CGMDataManager::GetAudioCoord(const std::wstring & strName)
{
	for(auto itr = m_audioDataMap.begin(); itr != m_audioDataMap.end(); itr++)
	{
		if (strName == itr->second.name)
		{
			return itr->second.audioCoord;
		}
	}
	return SGMAudioCoord();
}

bool CGMDataManager::SetAudioData(const SGMAudioData& sData)
{
	if (sData.audioCoord.angle < 0.0 ||
		sData.audioCoord.angle >= (osg::PI*2.0) ||
		sData.audioCoord.radius > 1.0 ||
		sData.audioCoord.radius <= 0.0)
	{
		return false;
	}

	SGMStarCoord vOldStarCoord = SGMStarCoord(0, 0, 0);
	SGMStarCoord vStarCoord = _AudioCoord2StarCoord(sData.audioCoord);
	int i = 0;
	auto itr = m_storedNameVector.begin();
	for ( ; itr != m_storedNameVector.end(); itr++, i++)
	{
		if (sData.name == itr->data())
		{
			vOldStarCoord = m_storedCoordVector.at(i);
			m_storedCoordVector.at(i) = vStarCoord;
		}
	}
	if (itr == m_storedNameVector.end()) return false;

	m_audioDataMap.erase(vOldStarCoord);
	m_audioDataMap.insert(std::make_pair(vStarCoord, sData));

	return true;
}

void CGMDataManager::_RefreshAudioCoordinates()
{
	CGMXml aXML;
	if (aXML.Load(m_pConfigData->strCorePath + "Coordinates/AudioData.xml", "Data"))
	{
		VGMXmlElementVec vAudioVec = aXML.GetChildren("Audio");
		for (auto audioItr : vAudioVec)
		{
			const char* cStr = audioItr.Attribute("name");
			double fRadius, fAngle;
			audioItr.Attribute("radius", &fRadius);
			audioItr.Attribute("angle", &fAngle);

			// insert the starCoord and file name
			SGMAudioCoord audioCoord(fRadius, fAngle);
			SGMStarCoord starCoord = _AudioCoord2StarCoord(audioCoord);
			// 处理UTF-8
			const std::wstring wStr = _CharToWstring(cStr);
			auto iter = std::find(m_storedNameVector.begin(), m_storedNameVector.end(), wStr);
			if (iter == m_storedNameVector.end())
			{
				m_storedNameVector.push_back(wStr);
				m_storedCoordVector.push_back(starCoord);
				SGMAudioData sData(wStr, audioCoord);
				m_audioDataMap.insert(std::make_pair(starCoord, sData));
			}
		}
	}
}

bool CGMDataManager::_SaveAudioCoordinates()
{
	CGMXml aXML;
	aXML.Create(m_pConfigData->strCorePath + "Coordinates/AudioData.xml", "Data");
	for (auto &itr : m_audioDataMap)
	{
		const std::string str = _WcharToChar(itr.second.name.c_str());
		aXML.AddChild("Audio", str.c_str(), itr.second.audioCoord.radius, itr.second.audioCoord.angle);
	}
	return aXML.Save();
}

void CGMDataManager::_RefreshAudioFiles()
{
	//文件句柄 
	intptr_t hFile = 0;
	//文件信息（用Unicode保存使用_wfinddata_t，多字节字符集使用_finddata_t）
	_wfinddata_t fileinfo;
	std::wstring strFilePath;
	std::wstring strFile = m_pConfigData->strMediaPath + m_strAudioPath;

	for (auto formatItr : m_formatVector)
	{
		if ((hFile = _wfindfirst(strFilePath.assign(strFile).append(L"/*.").append(formatItr).c_str(), &fileinfo)) != -1)
		{
			do{
				if (m_fileVector.size() >= 65536) break;
				std::wstring strFileName = strFilePath.assign(fileinfo.name);
				m_fileVector.push_back(strFileName);  //将文件名保存

				auto iter = std::find(m_storedNameVector.begin(), m_storedNameVector.end(), strFileName);
				if (iter == m_storedNameVector.end())
				{
					m_iRandom.seed(time(0) + m_fileVector.size());
					SGMAudioCoord audioCoord(0.0, 0.0);
					SGMStarCoord starCoord(0,0,0);
					do {
						const int iLargeNum = 10000;
						// 用一个随机数iRadiusFilter来过滤半径
						int iRadiusFilter = -1;
						double fRadius = 0;
						do {
							// 半径 == 音频的节拍，半径越大，节拍越慢
							// [0,GM_COORD_MAX]
							int iRadius = m_iRandom() % GM_COORD_MAX;
							// [0.0,1.0]
							fRadius = double(iRadius) / GM_COORD_MAX;

							iRadiusFilter = m_iRandom() % iLargeNum;
						}while(iRadiusFilter > (1-std::exp(-fRadius*6))*std::exp2(-fRadius * 6)*4*iLargeNum);

						// 用另一个随机数iAngleFilter来过滤角度
						int iAngleFilter = -1;
						double fAngle = -osg::PI_2;
						do {
							// 旋转角度 == 音频的类型
							// [0,GM_COORD_MAX*16]
							int iAngle = m_iRandom() % GM_COORD_MAX_16;
							// [0.0,PI * 2.0]
							fAngle = osg::PI * 2.0 * double(iAngle) / GM_COORD_MAX_16;

							iAngleFilter = m_iRandom() % iLargeNum;
						} while (iAngleFilter > iLargeNum*(std::cos(fAngle*2)+3)*0.25*
							std::powf(1-std::abs(std::sin(fAngle*2)),4.0*fRadius));

						// 音频空间坐标转星辰坐标
						audioCoord = SGMAudioCoord(fRadius, fAngle);
						starCoord = _AudioCoord2StarCoord(audioCoord);

					} while ((starCoord.x == 0 && starCoord.y == 0) ||
						(m_audioDataMap.find(starCoord) != m_audioDataMap.end()));

					// insert the coord and file name			
					m_storedNameVector.push_back(strFileName);
					m_storedCoordVector.push_back(starCoord);
					SGMAudioData sData(strFileName, audioCoord);
					m_audioDataMap.insert(std::make_pair(starCoord, sData));
				}
			} while (_wfindnext(hFile, &fileinfo) == 0);
		}
		_findclose(hFile);
	}
}

const std::wstring CGMDataManager::_CharToWstring(const char *cstr)
{
	if (!cstr) return std::wstring();
	int length = int(strlen(cstr)) + 1;
	wchar_t *t = (wchar_t*)malloc(sizeof(wchar_t) * length);
	if (!t) return std::wstring();
	memset(t, 0, length * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, cstr, int(strlen(cstr)), t, length);
	std::wstring wstr = t;
	free(t);
	return wstr;
}

const std::string CGMDataManager::_WcharToChar(const wchar_t* wstr)
{
	int length = int(wcslen(wstr));
	int destLen = WideCharToMultiByte(CP_UTF8, 0, wstr, length, 0, 0, 0, 0);
	if (destLen <= 0)
	{
		return std::string();
	}
	std::string sDest(destLen, '\0');
	destLen = WideCharToMultiByte(CP_UTF8, 0, wstr, length, &sDest[0], destLen, 0, 0);
	if (destLen <= 0)
	{
		return std::string();
	}
	return sDest;
}

SGMStarCoord CGMDataManager::_AudioCoord2StarCoord(const SGMAudioCoord& audioCoord)
{
	double fAudioX = audioCoord.radius * std::cos(audioCoord.angle);
	double fAudioY = audioCoord.radius * std::sin(audioCoord.angle);

	// Y值向上下偏移
	double fOffset = 0.22;
	if (fAudioX - fAudioY > 0)
	{
		fOffset *= -1.0f;
	}

	osg::Vec4 vOffset = osg::Vec4(fAudioX, fAudioY*(1.0f - std::abs(fOffset)) + fOffset, 0.0f, 0.0f);
	double fAngle = -osg::PI_4 + audioCoord.radius*osg::PI*2.0f;
	osg::Quat qRotate = osg::Quat(
		0, osg::Vec3d(1.0, 0.0, 0.0),
		0, osg::Vec3d(0.0, 1.0, 0.0),
		fAngle, osg::Vec3d(0.0, 0.0, 1.0));
	osg::Matrix mRotate;
	mRotate.setRotate(qRotate);
	osg::Vec4 vRotate = vOffset * mRotate;

	SGMStarCoord starCoord;
	starCoord.x = vRotate.x() * GM_COORD_MAX;
	starCoord.y = vRotate.y() * GM_COORD_MAX;
	starCoord.z = vRotate.z();
	return starCoord;
}
