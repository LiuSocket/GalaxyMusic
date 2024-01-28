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
#include "GMKit.h"
#include <osgDB/ReadFile>
#include <io.h>
using namespace GM;

/*************************************************************************
 Macro Defines
*************************************************************************/
#define GM_LIST_MAX					(50)	// ��������б����󳤶�
#define LUT_WIDTH					8192	// ���ұ�������

/*************************************************************************
Structs
*************************************************************************/

/*************************************************************************
Class
*************************************************************************/

/*************************************************************************
CGMDataManager Methods
*************************************************************************/

/** @brief ���� */
CGMDataManager::CGMDataManager() :
	m_pKernelData(nullptr), m_pConfigData(nullptr),
	m_strAudioPath(L"Music/"), m_strCurrentAudio(L""),
	m_formatVector({ L"mp3", L"wma", L"wav", L"ogg" }),
	m_iFreeUID(1),
	m_pAudioNumUniform(new osg::Uniform("audioNum", 4096.0f))
{
}

/** @brief ���� */
CGMDataManager::~CGMDataManager()
{
	m_audioDataMap.clear();
}

/** @brief ��ʼ�� */
bool CGMDataManager::Init(SGMKernelData* pKernelData, SGMConfigData * pConfigData)
{
	m_pKernelData = pKernelData;
	m_pConfigData = pConfigData;

	// ��ȡ�Ѿ��������Ƶ����
	_RefreshAudioCoordinates();
	// ɨ��Data/Media/Music·��������֧�ֵ���Ƶ�ļ��������ļ���������m_fileVector��
	_RefreshAudioFiles();

	// ��ȡ�Ѿ��������Ƶ����˳��
	_LoadPlayingOrder();

	// ��ʼ��compute shader
	_InitComputeNearUID();

	return true;
}

bool CGMDataManager::Update(double dDeltaTime)
{
	if (m_pNearComputeNode->getDirty())
	{
		if (!(GM_Root->containsNode(m_pReadPixelCam.get())))
		{
			GM_Root->addChild(m_pReadPixelCam.get());
		}
		m_pNearComputeNode->setDirty(false);
		m_pReadPixelFinishCallback->SetReady(true);
	}
	if (m_pReadPixelFinishCallback->GetWritten())
	{
		GM_Root->removeChild(m_pReadPixelCam.get());
	}
	return true;
}

/** @brief ���� */
bool CGMDataManager::Save()
{
	//_SaveAudioCoordinates();
	return SavePlayingOrder();
}

bool CGMDataManager::GetAudioDataMap(std::map<unsigned int, SGMAudioData>& dataMap)
{
	dataMap = m_audioDataMap;
	return true;
}

bool CGMDataManager::FindAudio(const std::wstring & strName)
{
	for (auto itr : m_audioDataMap)
	{
		if (strName == itr.second.name)
		{
			return true;
		}
	}
	return false;
}

bool CGMDataManager::FindAudio(double& fX, double& fY, double& fZ, std::wstring& strName)
{
	if (m_pNearUIDImage.valid())
	{
		unsigned int iWidth = m_pNearUIDImage->s();
		unsigned int iHeight = m_pNearUIDImage->t();
		unsigned int s = osg::clampBetween(fX * 0.5 + 0.5, 0.0, 1.0) * iWidth;
		unsigned int t = osg::clampBetween(fY * 0.5 + 0.5, 0.0, 1.0) * iHeight;

		float fR = m_pNearUIDImage->getColor(s, t).r();
		float fG = m_pNearUIDImage->getColor(s, t).g();
		float fB = m_pNearUIDImage->getColor(s, t).b();
		float fA = m_pNearUIDImage->getColor(s, t).a();

		if (1.0f > fA) // ���ε�̫Զ����Ƶ��
		{
			int iUID = (fR + (fG + fB * 255) * 255) * 255;
			auto itr = m_audioDataMap.find(iUID);
			if (itr != m_audioDataMap.end())
			{
				SGMGalaxyCoord sGC = itr->second.galaxyCoord;
				// �޸�fX,fY,fZ��ֵ
				fX = sGC.x;
				fY = sGC.y;
				fZ = sGC.z;
				m_strCurrentAudio = itr->second.name;
				// ��ѯ�������Ƶ���ƺ󣬸��²���˳���б����ʷ��¼
				_UpdateAudioList(m_strCurrentAudio);
				strName = m_strCurrentAudio;
				return true;
			}
		}
	}

	strName = L"";
	return false;
}

std::wstring CGMDataManager::FindAudio(const unsigned int iUID)
{
	auto itr = m_audioDataMap.find(iUID);
	if (itr != m_audioDataMap.end())
	{
		m_strCurrentAudio = itr->second.name;
		// ��ѯ�������Ƶ���ƺ󣬸��²���˳���б����ʷ��¼
		_UpdateAudioList(m_strCurrentAudio);
		return m_strCurrentAudio;
	}
	return L"";
}

std::wstring CGMDataManager::GetLastAudio()
{
	// ���ֻ��һ����Ƶ������û����Ƶ���򷵻ؿ��ַ���
	if (L"" == m_strCurrentAudio || 2 > m_historyList.size())
	{
		return L"";
	}

	auto itr = m_historyList.end();
	itr--; itr--;
	m_strCurrentAudio = itr->data();

	// һ���ص���ȥ�۲���ʷ��Ϣ��δ�����ı�
	m_historyList.pop_back();
	return itr->data();
}

int CGMDataManager::GetCurrentAudioID() const
{
	if (L"" == m_strCurrentAudio || m_playingOrder.empty())
	{
		return -1;
	}

	int i = 0;
	for (auto itr = m_audioDataMap.begin();
		itr != m_audioDataMap.end();
		itr++, i++)
	{
		if (m_strCurrentAudio == itr->second.name)
		{
			return i;
		}
	}
	return 0;
}

osg::Vec4f CGMDataManager::GetAudioColor(const SGMAudioCoord& audioCoord) const
{
	return Angle2Color(audioCoord.angle);
}

osg::Vec4f CGMDataManager::GetAudioColor(const unsigned int iUID) const
{
	//if (m_pConfigData->bWanderingEarth)
	//{
	//	return osg::Vec4f(1.0, 0.9, 0.1, 1.0);
	//}

	auto itr = m_audioDataMap.find(iUID);
	if (itr != m_audioDataMap.end())
	{
		return GetAudioColor(itr->second.audioCoord);
	}
	return osg::Vec4f(1, 1, 1, 1);
}

osg::Vec4f CGMDataManager::Angle2Color(const float fEmotionAngle) const
{
	// 0.0f - 1.0f
	float fAngle = fmod(fEmotionAngle / (2.0f * osg::PI), 1.0f);
	return osg::Vec4f(
		osg::clampBetween(std::abs(4.0f * fAngle - 2.5f) - 0.5f, 0.0f, 1.0f),
		osg::clampBetween(1.5f - std::abs(4.0f * fAngle - 1.5f), 0.0f, 1.0f),
		1.0f - osg::clampBetween(std::abs(4.0f*fAngle - 3.0f), 0.0f, 1.0f),
		1);
}

unsigned int CGMDataManager::GetUID() const
{
	if (L"" == m_strCurrentAudio)
	{
		return 0;
	}
	else
	{
		return GetUID(m_strCurrentAudio);
	}
}

unsigned int CGMDataManager::GetUID(const std::wstring& strName) const
{
	for (auto itr : m_audioDataMap)
	{
		if (strName == itr.second.name)
		{
			return itr.second.UID;
		}
	}
	return 0;
}

SGMAudioCoord CGMDataManager::GetAudioCoord(const std::wstring& strName) const
{
	for (auto itr : m_audioDataMap)
	{
		if (strName == itr.second.name)
		{
			return itr.second.audioCoord;
		}
	}
	return SGMAudioCoord();
}

SGMGalaxyCoord CGMDataManager::GetGalaxyCoord(const std::wstring & strName) const
{
	for (auto itr : m_audioDataMap)
	{
		if (strName == itr.second.name)
		{
			return itr.second.galaxyCoord;
		}
	}
	return SGMGalaxyCoord();
}

bool CGMDataManager::EditAudioData(SGMAudioData& sData)
{
	if (sData.audioCoord.angle < 0.0 ||
		sData.audioCoord.angle >= (osg::PI*2.0) ||
		sData.audioCoord.BPM > 600.0 ||
		sData.audioCoord.BPM < 20.0 ||
		sData.galaxyCoord.x > 1.0 || sData.galaxyCoord.y > 1.0 || sData.galaxyCoord.z > 1.0 ||
		sData.galaxyCoord.x < -1.0 || sData.galaxyCoord.y < -1.0 || sData.galaxyCoord.z < -1.0)
	{
		return false;
	}

	// ��������������Ƶ�����Ƿ��غϣ�������κ�һ���غϾ�����Ӧ�޸�
	auto iter = m_audioDataMap.begin();
	while (iter != m_audioDataMap.end())
	{
		if ((sData.galaxyCoord == iter->second.galaxyCoord) || (sData.audioCoord == iter->second.audioCoord))
		{
			// �����غ�
			if (sData.galaxyCoord == iter->second.galaxyCoord)
			{
				sData.galaxyCoord.z += fmod(sData.galaxyCoord.z + 1.011f, 2.0f) - 1.0f;
			}
			if (sData.audioCoord == iter->second.audioCoord)
			{
				sData.audioCoord.rank += 1;
			}
			// �ص���ʼ�����¼���Ƿ��غ�
			iter = m_audioDataMap.begin();
		}
		else
		{
			iter++;
		}
	}

	bool bExist = false;
	for (auto& itr : m_audioDataMap)
	{
		if (sData.name == itr.second.name)
		{
			itr.second = sData;
			bExist = true;
			break;
		}
	}

	if (bExist)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool CGMDataManager::SavePlayingOrder() const
{
	CGMXml aXML;
	aXML.Create(m_pConfigData->strCorePath + "Users/AudioPlayingOrder.xml", "Order");
	for (auto &itr : m_playingOrder)
	{
		CGMXmlNode sNode = aXML.AddChild("Audio");
		sNode.SetPropWStr("name", itr.c_str());
	}
	return aXML.Save();
}

SGMGalaxyCoord CGMDataManager::AudioCoord2GalaxyCoord(const SGMAudioCoord & audioCoord) const
{
	double fA = audioCoord.angle + osg::PI*1.25;

	SGMGalaxyCoord vGalaxyCoord = SGMGalaxyCoord();
	vGalaxyCoord.z = 0.01*audioCoord.rank;

	// BPM����600����Ƶ�����϶�Ϊ���޽�����Ƶ������������λ��
	if ((600.0 < audioCoord.BPM) || (m_pConfigData->fMinBPM > audioCoord.BPM))
	{
		vGalaxyCoord.x = 0.1*std::cos(fA);
		vGalaxyCoord.y = 0.1*std::sin(fA);
		return vGalaxyCoord;
	}

	/*
	BPM��Ӧ�ĵȽ����ߵĽ����꣺[0, 90]����λ���㣬��һ��Ҫע�⣺�������Թ�ϵ
	��Ϊ��ƵBPM��Ҫ������100����������Ϊ�����ۣ���ƵBPM�ͽ�����Theta�ġ�2�ο����������Թ�ϵ
	*/
	double fRadiusRatio = sqrt(m_pConfigData->fMinBPM / audioCoord.BPM);
	double fEmotion = fmod(fA / osg::PI, 1.0);
	double fTheta = fRadiusRatio * osg::PI * 2.5 - fEmotion * osg::PI_2 + osg::PI;
	if (audioCoord.angle < osg::PI*1.75 && audioCoord.angle > osg::PI*0.75)
	{
		fTheta += osg::PI;
	}
	double fRadius = (exp((fRadiusRatio * 5.0 + fEmotion)/6.0)-1.0) / (exp(1.0)-1.0);
	vGalaxyCoord.x = fRadius * std::cos(fTheta);
	vGalaxyCoord.y = fRadius * std::sin(fTheta);

	return vGalaxyCoord;
}

void CGMDataManager::_RefreshAudioCoordinates()
{
	CGMXml aXML;
	if (aXML.Load(m_pConfigData->strCorePath + "Users/AudioData.xml", "Data"))
	{
		// ��ʱ��¼UID�������Ƶ����Vector
		std::vector<SGMAudioData> tempAudioVector;

		VGMXmlNodeVec vAudioVec = aXML.GetChildren("Audio");
		for (auto audioItr : vAudioVec)
		{
			const std::wstring wStr = audioItr.GetPropWStr("name");
			double fUID = audioItr.GetPropDouble("UID");
			double fBPM = audioItr.GetPropDouble("BPM", 100);
			double fAngle = audioItr.GetPropDouble("angle");
			double fRank = audioItr.GetPropDouble("rank");

			SGMAudioCoord vAudioCoord(fBPM, fAngle, int(fRank));
			SGMGalaxyCoord vGalaxyCoord = AudioCoord2GalaxyCoord(vAudioCoord);

			if (0 == fUID)
			{
				SGMAudioData sTempData(fUID, wStr, vAudioCoord, vGalaxyCoord);
				tempAudioVector.push_back(sTempData);
			}
			else
			{
				if (m_audioDataMap.empty())
				{
					// vectorΪ�գ�ֱ�Ӳ���Ϸ�����
					SGMAudioData sData(fUID, wStr, vAudioCoord, vGalaxyCoord);
					_AddAudioData2Map(sData);
					
				}
				else
				{
					bool bExist = false;
					for (auto iter : m_audioDataMap)
					{
						if (fUID == iter.second.UID && wStr == iter.second.name)
						{
							// UID�����ƶ���ͬ��˵����Ƶ�Ѵ���
							bExist = true;
							break;
						}
						else if (fUID == iter.second.UID && wStr != iter.second.name)
						{
							// UID��ͬ�����Ʋ�ͬ��˵��UID����
							// ��Ҫ��ʱ��UID�޸�Ϊ0���ȵ���Ƶ�ļ�ȫ���������ͳһ�޸����д����UID
							bExist = true;
							fUID = 0;
							SGMAudioData sTempData(fUID, wStr, vAudioCoord, vGalaxyCoord);
							tempAudioVector.push_back(sTempData);
						}
						else if (fUID != iter.second.UID && wStr == iter.second.name)
						{
							//  UID��ͬ��������ͬ��˵���ļ��е���Ƶ�ظ�������
							bExist = true;
							break;
						}
					}
					if (!bExist)
					{
						auto iter = m_audioDataMap.begin();
						while (iter != m_audioDataMap.end())
						{
							if ((vGalaxyCoord == iter->second.galaxyCoord) || (vAudioCoord == iter->second.audioCoord))
							{
								// �����غ�
								if (vGalaxyCoord == iter->second.galaxyCoord)
								{
									vGalaxyCoord.z += fmod(vGalaxyCoord.z + 1.011f, 2.0f) - 1.0f;
								}
								if (vAudioCoord == iter->second.audioCoord)
								{
									vAudioCoord.rank += 1;
								}
								// �ص���ʼ�����¼���Ƿ��غ�
								iter = m_audioDataMap.begin();
							}
							else
							{
								iter++;
							}
						}
						SGMAudioData sData(fUID, wStr, vAudioCoord, vGalaxyCoord);
						_AddAudioData2Map(sData);
					}
				}
			}
		}

		// ����ʱ����Ĵ���UID����Ƶ�ļ���ͳһ���úϷ���UID�����뵽map
		//tempAudioVector -��m_audioDataMap
		for (auto& itr : tempAudioVector)
		{
			itr.UID = m_iFreeUID;
			_AddAudioData2Map(itr);
		}
		tempAudioVector.clear();
	}
}

bool CGMDataManager::_SaveAudioCoordinates() const
{
	CGMXml aXML;
	aXML.Create(m_pConfigData->strCorePath + "Users/AudioData.xml", "Data");
	for (auto &itr : m_audioDataMap)
	{
		CGMXmlNode sNode = aXML.AddChild("Audio");
		sNode.SetPropUInt("UID", itr.second.UID);
		sNode.SetPropWStr("name", itr.second.name.c_str());
		sNode.SetPropDouble("BPM", itr.second.audioCoord.BPM);
		sNode.SetPropDouble("angle", itr.second.audioCoord.angle);
		sNode.SetPropDouble("rank",itr.second.audioCoord.rank);
	}
	return aXML.Save();
}

void CGMDataManager::_RefreshAudioFiles()
{
	//�ļ���� 
	intptr_t hFile = 0;
	//�ļ���Ϣ����Unicode����ʹ��_wfinddata_t�����ֽ��ַ���ʹ��_finddata_t��
	_wfinddata_t fileinfo;
	std::wstring strFilePath;
	std::wstring strFile = m_pConfigData->strMediaPath + m_strAudioPath;

	double fAngle = 0.01;
	for (auto formatItr : m_formatVector)
	{
		if ((hFile = _wfindfirst(strFilePath.assign(strFile).append(L"/*.").append(formatItr).c_str(), &fileinfo)) != -1)
		{
			do{
				if (m_audioDataMap.size() >= 65536) break;
				std::wstring strFileName = strFilePath.assign(fileinfo.name);

				bool bFind = false;
				for (auto iter : m_audioDataMap)
				{
					if (strFileName == iter.second.name)
					{
						bFind = true;
						break;
					}
				}
				if (!bFind)
				{
					// û���ҵ����������µ���Ƶ�ļ�������map��������BPM����
					SGMAudioCoord vAudioCoord = SGMAudioCoord(0, fAngle, 1);
					// �����Ƶ�����Ƿ����غϣ�����о��޸�
					auto iter = m_audioDataMap.begin();
					while (iter != m_audioDataMap.end())
					{
						if (vAudioCoord == iter->second.audioCoord)
						{
							// �����غ�
							vAudioCoord.angle = fmod(vAudioCoord.angle + 0.01234, osg::PI * 2);
							// �ص���ʼ�����¼���Ƿ��غ�
							iter = m_audioDataMap.begin();
						}
						else
						{
							iter++;
						}
					}
					// ��Ƶ�ռ�����ת��������
					SGMGalaxyCoord vGalaxyCoord = AudioCoord2GalaxyCoord(vAudioCoord);

					// �ں��ʵ�λ�ò�����Ƶ����
					SGMAudioData sData(m_iFreeUID, strFileName, vAudioCoord, vGalaxyCoord);
					_AddAudioData2Map(sData);
				}
			} while (_wfindnext(hFile, &fileinfo) == 0);
		}
		_findclose(hFile);
	}
}

void CGMDataManager::_DeleteOverdueAudios()
{
	//�ļ���� 
	intptr_t hFile = 0;
	//�ļ���Ϣ����Unicode����ʹ��_wfinddata_t�����ֽ��ַ���ʹ��_finddata_t��
	_wfinddata_t fileinfo;
	std::wstring strFilePath;
	std::wstring strFile = m_pConfigData->strMediaPath + m_strAudioPath;

	for (auto itr = m_audioDataMap.begin(); itr != m_audioDataMap.end(); )
	{
		if ((hFile = _wfindfirst(strFilePath.assign(strFile).append(itr->second.name.c_str()).c_str(), &fileinfo)) != -1)
		{
			itr++;
		}
		else
		{
			for (auto it = m_audioDataMap.begin(); it != m_audioDataMap.end(); )
			{
				if (it->second.name == itr->second.name)
				{
					it = m_audioDataMap.erase(it);
					break;
				}
				else
				{
					it++;
				}
			}
			itr = m_audioDataMap.erase(itr);
		}
		_findclose(hFile);
	}

	m_pAudioNumUniform->set(float(m_audioDataMap.size()));
}

void CGMDataManager::_UpdateAudioList(const std::wstring & strName)
{
	// ����б��в�����������֣���������ּ�����������б�
	auto itr = std::find(m_playingOrder.begin(), m_playingOrder.end(), strName);
	if (itr == m_playingOrder.end())
	{
		if (GM_LIST_MAX <= m_playingOrder.size())
		{
			// ɾ����һ��Ԫ��
			m_playingOrder.erase(m_playingOrder.begin());
		}
		m_playingOrder.push_back(strName);
	}

	// ������ʷ��¼
	if (m_historyList.empty())
	{
		m_historyList.push_back(strName);
	}
	else if (strName != m_historyList.back())
	{
		m_historyList.push_back(strName);
	}
}

void CGMDataManager::_LoadPlayingOrder()
{
	// ��������в����б�
	m_playingOrder.clear();
	m_historyList.clear();

	CGMXml aXML;
	if (aXML.Load(m_pConfigData->strCorePath + "Users/AudioPlayingOrder.xml", "Order"))
	{
		VGMXmlNodeVec vAudioVec = aXML.GetChildren("Audio");
		int i = 0;
		for (auto audioItr : vAudioVec)
		{
			const std::wstring wStr = audioItr.GetPropWStr("name");
			m_playingOrder.push_back(wStr);
			m_historyList.push_back(wStr);

			i++;
			if (vAudioVec.size() == i)
			{
				// ����ǰ���ŵ���Ƶ�޸�Ϊ�ϴβ��ŵ����һ����Ƶ
				m_strCurrentAudio = wStr;
			}
		}
	}
}

bool CGMDataManager::_AddAudioData2Map(SGMAudioData & sData)
{
	if (m_iFreeUID <= sData.UID)
	{
		m_audioDataMap[sData.UID] = sData;
		if (m_iFreeUID == sData.UID)
		{
			m_iFreeUID++;
		}

		m_pAudioNumUniform->set(float(m_audioDataMap.size()));
		return true;
	}
	else
	{
		m_audioDataMap.at(sData.UID) = sData;
		return false;
	}
}

void CGMDataManager::_InitComputeNearUID()
{
	int iSize = 1024;
	m_pNearComputeNode = new CGMDispatchCompute(iSize / 16, iSize / 16, 1);
	osg::ref_ptr<osg::Geode> pNearCSGeode = new osg::Geode();
	pNearCSGeode->setCullingActive(false);
	pNearCSGeode->addDrawable(m_pNearComputeNode.get());
	GM_Root->addChild(pNearCSGeode.get());
	osg::ref_ptr<osg::StateSet> pNearSS = m_pNearComputeNode->getOrCreateStateSet();

	const std::string strImgPath = m_pConfigData->strCorePath + "Users/NearUID.tga";
	if (!m_pNearUIDImage.valid()) m_pNearUIDImage = osgDB::readImageFile(strImgPath);
	// �����ͼƬ�����ڣ�������ͼƬ
	if (!m_pNearUIDImage.valid())
	{
		m_pNearUIDImage = new osg::Image;
		unsigned int* pNearUIDData = new unsigned int[iSize*iSize];
		for (int j = 0; j < iSize*iSize; j++)
		{
			pNearUIDData[j] = 0;
		}
		m_pNearUIDImage->setImage(iSize, iSize, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)pNearUIDData, osg::Image::USE_NEW_DELETE);

		// ��ʼ��ʱִ��һ�Σ�������ɺ��Զ��ر�
		m_pNearComputeNode->setDispatch(true);
	}

	// ʹ�� compute shader �����������ƵUID���洢
	// LUTͼ�������,����UID��С�����˳�򣬼�¼��Ƶ�����ӿռ��ά����
	float* pDataLUT = new float[LUT_WIDTH];
	for (int j = 0; j < LUT_WIDTH; j++)
	{
		pDataLUT[j] = 0.0f;
	}
	int i = 0;
	for (auto& itr = m_audioDataMap.begin();
		(itr != m_audioDataMap.end()) && (i+1 < LUT_WIDTH);
		itr++, i += 2)
	{
		pDataLUT[i] = itr->second.galaxyCoord.x;
		pDataLUT[i+1] = itr->second.galaxyCoord.y;
	}
	// Create Galaxy Coord LUT image 
	m_pGalaxyCoordImage = new osg::Image;
	m_pGalaxyCoordImage->setImage(LUT_WIDTH, 1, 1, GL_R32F, GL_RED, GL_FLOAT, (unsigned char*)pDataLUT, osg::Image::USE_NEW_DELETE);
	// Create Galaxy Coord LUT texture 
	m_pGalaxyCoordTex = new osg::Texture2D;
	m_pGalaxyCoordTex->setImage(m_pGalaxyCoordImage.get());
	m_pGalaxyCoordTex->setName("galaxyCoordTex");
	m_pGalaxyCoordTex->setInternalFormat(GL_R32F);
	m_pGalaxyCoordTex->setSourceFormat(GL_RED);
	m_pGalaxyCoordTex->setSourceType(GL_FLOAT);
	m_pGalaxyCoordTex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
	m_pGalaxyCoordTex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
	m_pGalaxyCoordTex->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER);
	m_pGalaxyCoordTex->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER);
	m_pGalaxyCoordTex->setDataVariance(osg::Object::DYNAMIC);
	m_pGalaxyCoordTex->setTextureSize(LUT_WIDTH, 1);
	m_pGalaxyCoordTex->setBorderColor(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));

	int iUnit = 0;

	m_pGalaxyCoordTex->bindToImageUnit(iUnit, osg::Texture::READ_ONLY);
	m_pGalaxyCoordTex->setUnRefImageDataAfterApply(false);
	pNearSS->setTextureAttribute(iUnit, m_pGalaxyCoordTex.get());
	pNearSS->addUniform(new osg::Uniform("galaxyCoordImg", iUnit));
	iUnit++;

	m_pNearUIDTex = new osg::Texture2D();
	m_pNearUIDTex->setTextureSize(iSize, iSize);
	m_pNearUIDTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
	m_pNearUIDTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
	m_pNearUIDTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
	m_pNearUIDTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
	m_pNearUIDTex->setBorderColor(osg::Vec4(0, 0, 0, 0));
	m_pNearUIDTex->setInternalFormat(GL_RGBA8);
	m_pNearUIDTex->setSourceFormat(GL_RGBA);
	m_pNearUIDTex->setSourceType(GL_UNSIGNED_BYTE);
	m_pNearUIDTex->bindToImageUnit(iUnit, osg::Texture::WRITE_ONLY);
	m_pNearUIDTex->setUnRefImageDataAfterApply(false);

	osg::ref_ptr<osg::Uniform> pTargetUniform = new osg::Uniform("targetImg", iUnit);
	pNearSS->addUniform(pTargetUniform.get());
	pNearSS->setTextureAttribute(iUnit, m_pNearUIDTex.get());
	iUnit++;

	m_pAudioNumUniform->set(float(m_audioDataMap.size()));
	pNearSS->addUniform(m_pAudioNumUniform.get());

	// galaxy shader ��ɫ��·��
	std::string strGalaxyShaderPath = "Shaders/GalaxyShader/";
	std::string strCompPath = m_pConfigData->strCorePath + strGalaxyShaderPath + "NearMap.comp";
	CGMKit::LoadComputeShader(pNearSS.get(), strCompPath, "CloudShadowCS");

	m_pReadPixelCam = new osg::Camera;
	m_pReadPixelCam->setName("ReadPixelCamera");
	m_pReadPixelCam->setReferenceFrame(osg::Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT);
	m_pReadPixelCam->setClearMask(GL_COLOR_BUFFER_BIT);
	m_pReadPixelCam->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
	m_pReadPixelCam->setViewport(0, 0, iSize, iSize);
	m_pReadPixelCam->setRenderOrder(osg::Camera::POST_RENDER);
	m_pReadPixelCam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	m_pReadPixelCam->attach(osg::Camera::COLOR_BUFFER, m_pNearUIDImage.get());
	m_pReadPixelCam->setAllowEventFocus(false);
	m_pReadPixelCam->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	m_pReadPixelCam->setViewMatrix(osg::Matrix::identity());
	m_pReadPixelCam->setProjectionMatrixAsOrtho2D(0, iSize, 0, iSize);
	m_pReadPixelCam->setProjectionResizePolicy(osg::Camera::FIXED);

	m_pReadPixelFinishCallback = new CReadPixelFinishCallback(m_pNearUIDImage.get());

	m_pReadPixelFinishCallback->SetPath(strImgPath);
	m_pReadPixelCam->setFinalDrawCallback(m_pReadPixelFinishCallback);

	osg::ref_ptr<osg::Geode> pGeode = new osg::Geode();
	pGeode->addDrawable(_CreateScreenTriangle(iSize, iSize));
	m_pReadPixelCam->addChild(pGeode.get());

	osg::ref_ptr<osg::StateSet> pStateset = pGeode->getOrCreateStateSet();
	pStateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pStateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

	int iReadPixelUnit = 0;
	CGMKit::AddTexture(pStateset.get(), m_pNearUIDTex.get(), "inputTex", iReadPixelUnit++);

	std::string strVertPath = m_pConfigData->strCorePath + strGalaxyShaderPath + "ReadPixel.vert";
	std::string strFragPath = m_pConfigData->strCorePath + strGalaxyShaderPath + "ReadPixel.frag";
	CGMKit::LoadShader(pStateset.get(), strVertPath, strFragPath, "ReadPixel");
}

osg::Geometry* CGMDataManager::_CreateScreenTriangle(const int width, const int height)
{
	osg::Geometry* pGeometry = new osg::Geometry();

	osg::ref_ptr<osg::Vec3Array> verArray = new osg::Vec3Array;
	verArray->push_back(osg::Vec3(0, 0, 0));
	verArray->push_back(osg::Vec3(2 * width, 0, 0));
	verArray->push_back(osg::Vec3(0, 2 * height, 0));
	pGeometry->setVertexArray(verArray);

	osg::ref_ptr<osg::Vec2Array> textArray = new osg::Vec2Array;
	textArray->push_back(osg::Vec2(0, 0));
	textArray->push_back(osg::Vec2(2, 0));
	textArray->push_back(osg::Vec2(0, 2));
	pGeometry->setTexCoordArray(0, textArray);

	osg::ref_ptr <osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 1, 0));
	pGeometry->setNormalArray(normal);
	pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

	pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, 3));

	pGeometry->setUseVertexBufferObjects(true);
	pGeometry->setUseDisplayList(false);
	pGeometry->setDataVariance(osg::Object::DYNAMIC);

	return pGeometry;
}