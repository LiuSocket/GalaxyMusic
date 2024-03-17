//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMGalaxy.cpp
/// @brief		Galaxy-Music Engine - GMGalaxy
/// @version	1.0
/// @author		LiuTao
/// @date		2020.11.27
//////////////////////////////////////////////////////////////////////////

#include "GMGalaxy.h"
#include "GMMilkyWay.h"
#include "GMSolar.h"
#include "GMEngine.h"
#include "GMCommonUniform.h"
#include "GMDataManager.h"
#include "GMKit.h"
#include <osg/PointSprite>
#include <osg/LineWidth>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/TextureCubeMap>
#include <osg/PositionAttitudeTransform>
#include <osg/PolygonOffset>
#include <osgDB/ReadFile>

#include <ppl.h>
using namespace concurrency;
using namespace GM;
/*************************************************************************
Macro Defines
*************************************************************************/

#define GM_MIN_STARS_CUBE		(8e18)			// cube恒星的最小尺寸
#define GM_MIN_GALAXIES_CUBE	(2e25)			// cube星系的最小尺寸
#define ID_HANDLE				(0) 			// 把手默认状态下在switch中的索引号
#define ID_HANDLE_HOVER			(1)				// 把手hover状态下在switch中的索引号
#define ID_ARROW				(2) 			// 箭头在switch中的索引号
#define MAX_BPM_RATIO			(25) 			// 最大周期/最小周期

/*************************************************************************
Class
*************************************************************************/

/*************************************************************************
CGMGalaxy Methods
*************************************************************************/

/** @brief 构造 */
CGMGalaxy::CGMGalaxy() :
	m_pKernelData(nullptr), m_pConfigData(nullptr), m_pCommonUniform(nullptr), m_pDataManager(nullptr),
	m_fGalaxyRadius(5e20),
	m_strGalaxyShaderPath("Shaders/GalaxyShader/"), m_strGalaxyTexPath("Textures/Galaxy/"), 
	m_strCoreModelPath("Models/"), m_strPlayingStarName(L""),
	m_iPlayingAudioUID(0),m_vPlayingAudioCoord(SGMAudioCoord(0.5, 0.0)),
	m_vPlayingStarWorld4Pos(0.0, 0.0, 0.0), m_vNearStarWorld4Pos(0, 0, 0),
	m_vMouseWorldPos(0.0, 0.0, 0.0), m_vMouseLastWorldPos(0.0, 0.0, 0.0), m_mLastVP(osg::Matrixf()),
	m_pMousePosUniform(new osg::Uniform("mouseWorldPos", osg::Vec3f(0.0f, 0.0f, 0.0f))),
	m_pAudioUVUniform(new osg::Uniform("audioUV", osg::Vec2f(0.5f, 0.0f))),
	m_pGalaxyRadiusUniform(new osg::Uniform("galaxyRadius", 5.0f)),
	m_pEyePos4Uniform(new osg::Uniform("eyePos4", osg::Vec3f(0.0f, -2.5f, 0.0f))),
	m_fGalaxyHeightUniform(new osg::Uniform("galaxyHeight", 1.0f)),
	m_pCubeCenterUniform(new osg::Uniform("centerOffset", osg::Vec3f(0.0f, 0.0f, 0.0f))),
	m_pShapeUVWUniform(new osg::Uniform("shapeUVW", osg::Vec3f(3.2f, 3.2f, 4.0f))),
	m_fStarAlphaUniform(new osg::Uniform("starAlpha", 1.0f)),
	m_fGalaxiesAlphaUniform(new osg::Uniform("galaxiesAlpha", 1.0f)),
	m_fStarDistanceUniform(new osg::Uniform("starDistance", 1.0f)),
	m_fMyWorldAlphaUniform(new osg::Uniform("myWorldAlpha", 0.0f)),
	m_fArrowAngle(0.0f), m_fPRPA(0.0f), m_eArrowDir(EGMAD_NONE),
	m_bEdit(false), m_bHandleHover(false), m_bCapture(false), m_bWelcome(false),
	m_pMilkyWay(nullptr), m_pSolarSystem(nullptr)
{
	m_pMilkyWay = new CGMMilkyWay();
	m_pSolarSystem = new CGMSolar();

	m_iRandom.seed(0);
}

/** @brief 析构 */
CGMGalaxy::~CGMGalaxy()
{
}

/** @brief 初始化 */
bool CGMGalaxy::Init(SGMKernelData * pKernelData, SGMConfigData * pConfigData,
	CGMCommonUniform* pCommonUniform, CGMDataManager* pDataManager)
{
	m_pKernelData = pKernelData;
	m_pConfigData = pConfigData;
	m_pCommonUniform = pCommonUniform;
	m_pDataManager = pDataManager;

	for (int i = 0; i <= GM_HIERARCHY_MAX; i++)
	{
		osg::ref_ptr<osg::Group> _pRoot = new osg::Group();
		m_pHierarchyRootVector.push_back(_pRoot);
	}
	GM_Root->addChild(m_pHierarchyRootVector.at(4).get());

	m_pMilkyWay->Init(pKernelData, pConfigData, pCommonUniform);
	m_pSolarSystem->Init(pKernelData, pConfigData, pCommonUniform, pDataManager);

	// 读取dds时需要垂直翻转
	m_pDDSOptions = new osgDB::Options("dds_flip");

	// 初始化背景相关节点
	_InitBackground();

	std::string strGalaxyTexPath = pConfigData->strCorePath + m_strGalaxyTexPath;
	m_pGalaxyImage = osgDB::readImageFile(strGalaxyTexPath + "milkyWay.tga");
	m_pGalaxyColorTex = _CreateTexture2D(m_pGalaxyImage.get(), 4);
	m_3DShapeTex = _Load3DShapeNoise();

	// 初始化第三层级

	// 初始化第四层级的奥尔特云、把手的变换节点
	if (!m_pStar_4_Transform.valid())
	{
		m_pStar_4_Transform = new osg::PositionAttitudeTransform();
		m_pHierarchyRootVector.at(4)->addChild(m_pStar_4_Transform.get());
	}

	// 加载星系纹理数组
	const int iTexNum = 32;
	std::string strGalaxiesPath = strGalaxyTexPath + "GalaxyArray/";
	m_pGalaxiesTex = new osg::Texture2DArray();
	m_pGalaxiesTex->setTextureSize(64, 64, iTexNum);
	for (int i = 0; i < iTexNum; i++)
	{
		std::string strName = std::to_string(i);
		if (i < 10)
		{
			strName = "00" + strName;
		}
		else if (i < 100)
		{
			strName = "0" + strName;
		}
		m_pGalaxiesTex->setImage(i, osgDB::readImageFile(strGalaxiesPath + strName + ".dds", m_pDDSOptions.get()));
	}
	m_pGalaxiesTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_NEAREST);
	m_pGalaxiesTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_pGalaxiesTex->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	m_pGalaxiesTex->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	m_pGalaxiesTex->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
	m_pGalaxiesTex->setInternalFormat(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
	m_pGalaxiesTex->setSourceFormat(GL_RGBA);
	m_pGalaxiesTex->setSourceType(GL_UNSIGNED_BYTE);

	size_t iMaxNum = 65536;
	// 初始化立方体恒星数组
	m_pCubeVertArray = new osg::Vec4Array;
	m_pCubeColorArray = new osg::Vec4Array();
	m_pCubeElement = new osg::DrawElementsUShort(GL_POINTS);
	m_pCubeVertArray->reserve(iMaxNum);
	m_pCubeColorArray->reserve(iMaxNum);
	m_pCubeElement->reserve(iMaxNum);

	std::uniform_int_distribution<> iPseudoNoise(0, 9999);

	float fMinAlpha = 0.25f;
	int x = 0;
	while (x < iMaxNum)
	{
		float fU = iPseudoNoise(m_iRandom)*1e-4f;
		float fV = iPseudoNoise(m_iRandom)*1e-4f;
		float fW = iPseudoNoise(m_iRandom)*1e-4f;
		float fAlpha = 1.0f - _Get3DValue(fU*4.0f, fV*4.0f, fW*4.0f);
		fAlpha *= fAlpha;
		if (fAlpha > fMinAlpha)
		{
			float fRandomX = fU - 0.5f;
			float fRandomY = fV - 0.5f;
			float fRandomZ = fW - 0.5f;
			float fX = fRandomX;
			float fY = fRandomY;
			float fZ = fRandomZ;
			osg::Vec3f vColor = _GetRandomStarColor();

			m_pCubeVertArray->push_back(osg::Vec4(fX, fY, fZ, 1.0f));
			m_pCubeColorArray->push_back(osg::Vec4(vColor, (fAlpha - fMinAlpha) / (1.0f - fMinAlpha)));
			m_pCubeElement->push_back(x);
			x++;
		}
	}

	// 初始化球面恒星数组
	m_pSphereVertArray = new osg::Vec4Array;
	m_pSphereColorArray = new osg::Vec4Array;
	m_pSphereElement = new osg::DrawElementsUShort(GL_POINTS);
	m_pSphereVertArray->reserve(iMaxNum);
	m_pSphereColorArray->reserve(iMaxNum);
	m_pSphereElement->reserve(iMaxNum);

	int iEleNum = 0;
	for (size_t i = 0; i < iMaxNum; i++)
	{
		osg::Vec4 vPos = m_pCubeVertArray->at(i);
		osg::Vec3 vVertPos = osg::Vec3(vPos.x(), vPos.y(), vPos.z());
		float fLength = vVertPos.normalize();
		if (fLength <= 0.5f)
		{
			float fFalloff = 1.0f - osg::clampBetween(2.0f*fLength / 0.5f - 1.0f, 0.0f, 1.0f);
			osg::Vec4 vert = osg::Vec4(vVertPos, fFalloff);
			m_pSphereVertArray->push_back(vert);
			osg::Vec4 color = m_pCubeColorArray->at(i);
			m_pSphereColorArray->push_back(color);
			m_pSphereElement->push_back(iEleNum++);
		}
	}

	// 星系旋转枚举(0123)和纹理编号
	// xyz = 旋转枚举，分别代表图片旋转0°、90°、180°、270°
	// 000 = 0°
	// 100 = 90°
	// 010 = 180°
	// 001 = 270°
	// w = 星系纹理编号
	m_pGalaxiesInfo = new osg::Vec4Array;
	m_pGalaxiesInfo->reserve(iMaxNum);

	float fLayerMax = m_pGalaxiesTex->getNumImages();
	float fLayer = 0.0f;
	for (int i = 0; i < iMaxNum; i++)
	{
		float fHue = iPseudoNoise(m_iRandom) / 2500.0f;
		float fAlpha = m_pCubeColorArray->at(i).a();
		int iSpin = iPseudoNoise(m_iRandom) % 4;
		m_pGalaxiesInfo->push_back(osg::Vec4(fHue, iSpin, fLayer, fAlpha));
		if (3 == (i % 4))
		{
			fLayer = std::fmod(fLayer + 1.0f, fLayerMax);
		}
	}

	if (!m_pEye_4_Transform.valid())
	{
		m_pEye_4_Transform = new osg::PositionAttitudeTransform();
		m_pHierarchyRootVector.at(4)->addChild(m_pEye_4_Transform.get());
	}

	return true;
}

/** @brief 更新 */
bool CGMGalaxy::Update(double dDeltaTime)
{
	float fTimes;
	m_pCommonUniform->GetTime()->get(fTimes);

	if (m_bWelcome)
	{
		if (GM_ENGINE.IsWelcomeFinished())
		{
			if (m_pStateSetGalaxy.valid())
			{
				m_pStateSetGalaxy->setDefine("WELCOME", osg::StateAttribute::OFF);
			}
			if (m_pGeodeAudio.valid())
			{
				m_pGeodeAudio->getOrCreateStateSet()->setDefine("WELCOME", osg::StateAttribute::OFF);
			}
			m_bWelcome = false;
		}
	}
	else
	{
		if (m_bCapture)
		{
			if (4 == m_pKernelData->iHierarchy)
			{
				// 半径方向的微小偏移
				SGMAudioCoord vAudioCoord_radius = SGMAudioCoord(
					m_vPlayingAudioCoord.BPM > m_pConfigData->fMinBPM
					? m_vPlayingAudioCoord.BPM - 0.01 : m_vPlayingAudioCoord.BPM + 0.01,
					m_vPlayingAudioCoord.angle);
				// 角度方向的微小偏移
				SGMAudioCoord vAudioCoord_angle = SGMAudioCoord(
					m_vPlayingAudioCoord.BPM,
					std::fmod(m_vPlayingAudioCoord.angle + 0.01, osg::PI * 2));

				SGMGalaxyCoord vGCRadius = m_pDataManager->AudioCoord2GalaxyCoord(vAudioCoord_radius);
				SGMGalaxyCoord vGCAngle = m_pDataManager->AudioCoord2GalaxyCoord(vAudioCoord_angle);

				osg::Vec3d vWorldPos_radius = osg::Vec3d(vGCRadius.x, vGCRadius.y, vGCRadius.z) * m_fGalaxyRadius;
				osg::Vec3d vWorldPos_angle = osg::Vec3d(vGCAngle.x, vGCAngle.y, vGCAngle.z) * m_fGalaxyRadius;
				// 音频坐标的半径变大的方向的单位向量
				osg::Vec3d vRadiusAxis = vWorldPos_radius - m_vPlayingStarWorld4Pos;
				vRadiusAxis.z() = 0;
				if (vWorldPos_radius.length() < m_vPlayingStarWorld4Pos.length()) vRadiusAxis *= -1;
				vRadiusAxis.normalize();
				// 音频坐标的角度变大的方向的单位向量
				osg::Vec3d vAngleAxis = vWorldPos_angle - m_vPlayingStarWorld4Pos;
				vAngleAxis.z() = 0;
				vAngleAxis.normalize();

				// vRadiusAxis 的银盘面旋转角 音频空间半径外方向的旋转角度(-PI,PI]
				float fRadiusTheta = std::atan2(vRadiusAxis.y(), vRadiusAxis.x());
				// vAngleAxis 的银盘面旋转角 音频空间角正方向的旋转角度(-PI,PI]
				float fAngleTheta = std::atan2(vAngleAxis.y(), vAngleAxis.x());

				// + Radius + Angle
				m_fPRPA = 0.5*(fRadiusTheta + fAngleTheta);
				if (std::abs(fRadiusTheta - fAngleTheta) > osg::PI)
				{
					m_fPRPA += osg::PI;
					if (m_fPRPA > osg::PI)
					{
						m_fPRPA -= osg::PI * 2;
					}
				}
				
				double fMouse2CenterRatio = m_vMouseWorldPos.length() / m_fGalaxyRadius;
				osg::Vec3d vMouseMove = m_vMouseWorldPos - m_vMouseLastWorldPos;
				osg::Vec3d vMouseMoveDir = vMouseMove;
				vMouseMoveDir.normalize();

				const double fDeadValue = 0.001;

				// 求vStar2MouseDir的银盘面旋转角
				float fDeltaTheta = _IncludedAngle(std::atan2(vMouseMoveDir.y(), vMouseMoveDir.x()), m_fPRPA);
				double fMouseMove = vMouseMove * vRadiusAxis;
				if (EGMAD_RADIUS_OUT == m_eArrowDir || EGMAD_RADIUS_IN == m_eArrowDir)// 只改变音频半径分量
				{
					if (EGMAD_RADIUS_OUT == m_eArrowDir)
					{
						if ((fDeltaTheta < osg::PI*0.25 && fDeltaTheta > -osg::PI*0.75)
							|| (abs(fMouseMove / m_fGalaxyRadius) < fDeadValue))
						{
							m_fArrowAngle = fRadiusTheta;
							fMouseMove = std::fmax(0.0, fMouseMove);
						}
						else
						{
							m_eArrowDir = EGMAD_RADIUS_IN;
							m_fArrowAngle = std::fmod(fRadiusTheta + osg::PI, osg::PI * 2);
						}
					}
					else if (EGMAD_RADIUS_IN == m_eArrowDir)
					{
						if ((fDeltaTheta > osg::PI*0.25 || fDeltaTheta < -osg::PI*0.75)
							|| (abs(fMouseMove / m_fGalaxyRadius) < fDeadValue))
						{
							m_fArrowAngle = std::fmod(fRadiusTheta + osg::PI, osg::PI * 2);
							fMouseMove = std::fmin(0.0, fMouseMove);
						}
						else
						{
							m_eArrowDir = EGMAD_RADIUS_OUT;
							m_fArrowAngle = fRadiusTheta;
						}
					}
					double fRadiusFact = 0.7 - 0.4*fMouse2CenterRatio;

					m_vPlayingAudioCoord.BPM = max(
						m_vPlayingAudioCoord.BPM * (1-fRadiusFact * fMouseMove / m_fGalaxyRadius),
						m_pConfigData->fMinBPM);
				}
				else if (EGMAD_ANGLE_PLUS == m_eArrowDir || EGMAD_ANGLE_MINUS == m_eArrowDir)// 只改变音频角度分量
				{
					fMouseMove = vMouseMove * vAngleAxis;
					if (EGMAD_ANGLE_PLUS == m_eArrowDir)
					{
						if ((fDeltaTheta > -osg::PI*0.25 && fDeltaTheta < osg::PI*0.75)
							|| (abs(fMouseMove / m_fGalaxyRadius) < fDeadValue))
						{
							m_fArrowAngle = fAngleTheta;
							fMouseMove = std::fmax(0.0, fMouseMove);
						}
						else
						{
							m_eArrowDir = EGMAD_ANGLE_MINUS;
							m_fArrowAngle = std::fmod(fAngleTheta + osg::PI, osg::PI * 2);
						}
					}
					else if (EGMAD_ANGLE_MINUS == m_eArrowDir)
					{
						if ((fDeltaTheta < -osg::PI*0.25 || fDeltaTheta > osg::PI*0.75)
							|| (abs(fMouseMove / m_fGalaxyRadius) < fDeadValue))
						{
							m_fArrowAngle = std::fmod(fAngleTheta + osg::PI, osg::PI * 2);
							fMouseMove = std::fmin(0.0, fMouseMove);
						}
						else
						{
							m_eArrowDir = EGMAD_ANGLE_PLUS;
							m_fArrowAngle = fAngleTheta;
						}
					}
					double fRadiusFact = 10.0 - 6.0*fMouse2CenterRatio;
					double fAngle = m_vPlayingAudioCoord.angle + fRadiusFact * fMouseMove / m_fGalaxyRadius;
					if (fAngle < 0.0)
					{
						fAngle += osg::PI * 2;
					}
					m_vPlayingAudioCoord.angle = std::fmod(fAngle, osg::PI * 2);
				}
			}
			_UpdatePlayingStarInformation(m_vPlayingAudioCoord);
		}
		else
		{
			osg::Vec3d vDir = m_vMouseWorldPos - m_vPlayingStarWorld4Pos;
			vDir.normalize();
			float fDeltaTheta = _IncludedAngle(std::atan2(vDir.y(), vDir.x()), m_fPRPA);
			osg::Quat quat = osg::Quat(m_fPRPA, osg::Vec3d(0, 0, 1));
			if (fDeltaTheta > osg::PI_2)
			{
				m_eArrowDir = EGMAD_RADIUS_OUT;
			}
			else if (fDeltaTheta <= 0.0 && fDeltaTheta > -osg::PI_2)
			{
				m_eArrowDir = EGMAD_RADIUS_IN;
			}
			else
			{
				quat = osg::Quat(m_fPRPA + osg::PI_2, osg::Vec3d(0, 0, 1));
				if (fDeltaTheta <= osg::PI_2 && fDeltaTheta > 0.0)
				{
					m_eArrowDir = EGMAD_ANGLE_PLUS;
				}
				else
				{
					m_eArrowDir = EGMAD_ANGLE_MINUS;
				}
			}
			m_pStar_4_Transform->asPositionAttitudeTransform()->setAttitude(quat);
		}
		m_vMouseLastWorldPos = m_vMouseWorldPos;
	}

	m_pMilkyWay->Update(dDeltaTime);
	m_pSolarSystem->Update(dDeltaTime);

	return true;
}

/** @brief 更新(在主相机更新姿态之后) */
bool CGMGalaxy::UpdateLater(double dDeltaTime)
{
	osg::Vec3d vEye, vCenter, vUp;
	GM_View->getCamera()->getViewMatrixAsLookAt(vEye, vCenter, vUp);
	double fDistance = GM_ENGINE.GetHierarchyTargetDistance();

	int iHie = GM_ENGINE.GetHierarchy();
	double fUnit = m_pKernelData->fUnitArray->at(iHie);

	if (4 == iHie)
	{
		m_pEyePos4Uniform->set(osg::Vec3(vEye));
	}
	else
	{
		osg::Vec3 vEyePos4 = m_pKernelData->vEyePosArray->at(4);
		m_pEyePos4Uniform->set(vEyePos4);
	}

	switch (iHie)
	{
	case 0:
	{

	}
	break;
	case 1:
	{

	}
	break;
	case 2:
	{
	}
	break;
	case 3:
	{
		m_fStarDistanceUniform->set(osg::clampBetween(float(fDistance)*100.0f, 1.0f, 4000.0f));
	}
	break;
	case 4:
	{
		if (m_pEye_4_Transform.valid())
		{
			m_pEye_4_Transform->asPositionAttitudeTransform()->setPosition(osg::Vec3f(vEye));

			// 控制cube恒星的显隐
			double fCubeMinSize = GM_MIN_STARS_CUBE / fUnit;
			for (int i = 0; i < int(m_pStarsGeomVector.size()); i++)
			{
				if (fDistance < fCubeMinSize * std::pow(2, i + 3))
				{
					m_pStarsGeomVector.at(i)->setNodeMask(~0);
				}
				else
				{
					m_pStarsGeomVector.at(i)->setNodeMask(0);
				}
			}
			osg::Vec2f vCubeInfo;
			m_pStarsCubeInfoUniform->get(vCubeInfo);
			vCubeInfo.y() = fDistance;
			m_pStarsCubeInfoUniform->set(vCubeInfo);

			// 获取当前位置的星系单边高度，约束cube恒星的显隐
			double fGalaxyRadius4 = m_fGalaxyRadius / m_pKernelData->fUnitArray->at(4);// 4级空间下的星系半径
			float fU = 0.5 + 0.5 * osg::clampBetween(vEye.x() / fGalaxyRadius4, -1.0, 1.0);
			float fV = 0.5 + 0.5 * osg::clampBetween(vEye.y() / fGalaxyRadius4, -1.0, 1.0);
			float fGalaxH = 0.15f * CGMKit::GetImageColor(m_pGalaxyImage.get(), fU, fV, true).a();
			m_fGalaxyHeightUniform->set(fGalaxH);
		}

		float fStarAlpha = 1.0 - exp2(std::fmin(0.0, 0.001 - fDistance)*100.0);
		m_fStarAlphaUniform->set(fStarAlpha);

		float fGalaxyAlpha = 1.0 - exp2(std::fmin(0.0, 0.01 - fDistance)*10.0);
		m_pCommonUniform->SetGalaxyAlpha(fGalaxyAlpha);

		m_pBackgroundGalaxyTransform->setNodeMask((fDistance > 0.05) ? 0 : ~0);
		m_pCosmosBoxGeode->setNodeMask((fDistance > 0.001) ? ~0 : 0);
	}
	break;
	case 5:
	{
		if (m_pGalaxyGroup_Transform.valid())
		{
			osg::Vec3f vMinGalaxyGroupPos = vEye;
			float fLength = vMinGalaxyGroupPos.normalize();
			vMinGalaxyGroupPos *= std::fmin(fLength, 2 * GM_MIN_RADIUS);
			m_pGalaxyGroup_Transform->asPositionAttitudeTransform()->setPosition(vMinGalaxyGroupPos);
		}

		if (m_pEye_5_Transform.valid())
		{
			m_pEye_5_Transform->asPositionAttitudeTransform()->setPosition(osg::Vec3f(vEye));

			double fGalaxiesCubeMinSize = GM_MIN_GALAXIES_CUBE / m_pKernelData->fUnitArray->at(5);
			for (int i = 0; i < int(m_pGalaxiesGeomVector.size()); i++)
			{
				if (fDistance < fGalaxiesCubeMinSize * std::pow(2, i + 2))
				{
					m_pGalaxiesGeomVector.at(i)->setNodeMask(~0);
				}
				else
				{
					m_pGalaxiesGeomVector.at(i)->setNodeMask(0);
				}
			}
			osg::Vec2f vGalaxiesInfo;
			m_pGalaxiesInfoUniform->get(vGalaxiesInfo);
			vGalaxiesInfo.y() = fDistance;
			m_pGalaxiesInfoUniform->set(vGalaxiesInfo);
		}

		if (m_pGeodeSupercluster.valid())
		{
			m_pGeodeSupercluster->setNodeMask((fDistance < 1) ? ~0 : 0);
		}
		if (m_pGeodeUltracluster.valid())
		{
			m_pGeodeUltracluster->setNodeMask((fDistance < 10) ? ~0 : 0);
		}

		if (m_pConfigData->bPhoto)
		{
			m_fMyWorldAlphaUniform->set(osg::clampBetween(float((fDistance - 1) / 99), 0.0f, 1.0f));
		}
	}
	break;
	case 6:
	{
		if (m_pConfigData->bPhoto)
		{
			m_fMyWorldAlphaUniform->set(osg::clampBetween(float((fDistance - 1e-5) / 9.9e-4), 0.0f, 1.0f));
		}
	}
	break;
	default:
		break;
	}

	if (EGMRENDER_LOW != m_pConfigData->eRenderQuality)
	{
		// 实现抖动抗锯齿
		// 相机正前方单位向量（世界空间）
		osg::Vec3d vWorldEyeFrontDir = vCenter - vEye;
		vWorldEyeFrontDir.normalize();
		osg::Vec3d vWorldEyeRightDir = vWorldEyeFrontDir ^ vUp;
		vWorldEyeRightDir.normalize();

		m_pCommonUniform->SetEyeUpDir(vUp);		// 相机上方向在世界空间下的单位向量
		m_pCommonUniform->SetEyeRightDir(vWorldEyeRightDir);	// 相机右方向在世界空间下的单位向量
		m_pCommonUniform->SetEyeFrontDir(vWorldEyeFrontDir);	// 相机前方向在世界空间下的单位向量

		osg::Matrixd mMainViewMatrix = GM_View->getCamera()->getViewMatrix();
		osg::Matrixd mMainProjMatrix = GM_View->getCamera()->getProjectionMatrix();
		osg::Matrixf mInvProjMatrix = osg::Matrixd::inverse(mMainProjMatrix);
		m_pCommonUniform->SetMainInvProjMatrix(mInvProjMatrix);

		osg::Matrixd VP = mMainViewMatrix * mMainProjMatrix;
		// 修改VP差值矩阵
		osg::Matrixf deltaVP = osg::Matrixf(
			VP(0, 0) - m_mLastVP(0, 0), VP(0, 1) - m_mLastVP(0, 1), VP(0, 2) - m_mLastVP(0, 2), VP(0, 3) - m_mLastVP(0, 3),
			VP(1, 0) - m_mLastVP(1, 0), VP(1, 1) - m_mLastVP(1, 1), VP(1, 2) - m_mLastVP(1, 2), VP(1, 3) - m_mLastVP(1, 3),
			VP(2, 0) - m_mLastVP(2, 0), VP(2, 1) - m_mLastVP(2, 1), VP(2, 2) - m_mLastVP(2, 2), VP(2, 3) - m_mLastVP(2, 3),
			VP(3, 0) - m_mLastVP(3, 0), VP(3, 1) - m_mLastVP(3, 1), VP(3, 2) - m_mLastVP(3, 2), VP(3, 3) - m_mLastVP(3, 3)
		);
		m_pCommonUniform->SetDeltaVPMatrix(deltaVP);
		// 修改上一帧VP矩阵
		m_mLastVP = VP;
	}

	m_pMilkyWay->UpdateLater(dDeltaTime);
	m_pSolarSystem->UpdateLater(dDeltaTime);

	return true;
}

/** @brief 加载 */
bool CGMGalaxy::Load()
{
	std::string strGalaxyShader = m_pConfigData->strCorePath + m_strGalaxyShaderPath;

	if (m_pGeodeStarCube_4.valid())
	{
		CGMKit::LoadShader(m_pGeodeStarCube_4->getStateSet(),
			strGalaxyShader + "StarCube_4_Vert.glsl",
			strGalaxyShader + "StarCube_4_Frag.glsl",
			"StarCube_4");
	}
	if (m_pBackgroundStarTransform.valid())
	{
		CGMKit::LoadShader(m_pBackgroundStarTransform->getStateSet(),
			strGalaxyShader + "StarCube_Vert.glsl",
			strGalaxyShader + "StarCube_Frag.glsl",
			"StarCube");
	}
	if (m_pStateSetGalaxy.valid())
	{
		CGMKit::LoadShader(m_pStateSetGalaxy.get(),
			strGalaxyShader + "GalaxyStarVert.glsl",
			strGalaxyShader + "GalaxyStarFrag.glsl",
			"GalaxyStar");
	}
	if (m_pGeodePointsN_4.valid())
	{
		CGMKit::LoadShader(m_pGeodePointsN_4->getOrCreateStateSet(),
			strGalaxyShader + "StarNVert.glsl",
			strGalaxyShader + "StarNFrag.glsl",
			"StarN");
	}
	if (m_pGeodeAudio.valid())
	{
		CGMKit::LoadShader(m_pGeodeAudio->getStateSet(),
			strGalaxyShader + "AudioVert.glsl",
			strGalaxyShader + "AudioFrag.glsl",
			"AudioStar");
	}
	if (m_pStateSetPlane.valid())
	{
		CGMKit::LoadShader(m_pStateSetPlane.get(),
			strGalaxyShader + "GalaxyPlaneVert.glsl",
			strGalaxyShader + "GalaxyPlaneFrag.glsl",
			"GalaxyPlanet");
	}
	if (m_pStarInfoTransform.valid())
	{
		CGMKit::LoadShader(m_pStarInfoTransform->getStateSet(),
			strGalaxyShader + "PlayingStarVert.glsl",
			strGalaxyShader + "PlayingStarFrag.glsl",
			"PlayingStar");
	}
	if (m_pGeodeRegion.valid())
	{
		CGMKit::LoadShader(m_pGeodeRegion->getStateSet(), 
			strGalaxyShader + "AudioRegionVert.glsl",
			strGalaxyShader + "AudioRegionFrag.glsl",
			"AudioRegion");
	}
	if (m_pGeodeGalaxyGroup_4.valid())
	{
		CGMKit::LoadShader(m_pGeodeGalaxyGroup_4->getStateSet(),
			strGalaxyShader + "Galaxies_4_Vert.glsl",
			strGalaxyShader + "Galaxies_4_Frag.glsl",
			"Galaxies_4");
	}
	if (m_pGeodeGalaxyGroup_5.valid() && m_pGeodeGalaxies_5.valid())
	{
		CGMKit::LoadShader(m_pGeodeGalaxyGroup_5->getStateSet(),
			strGalaxyShader + "Galaxies_5_Vert.glsl", strGalaxyShader + "Galaxies_5_Frag.glsl",
			"GalaxyGroup_5");
		CGMKit::LoadShader(m_pGeodeGalaxies_5->getStateSet(),
			strGalaxyShader + "Galaxies_5_Vert.glsl", strGalaxyShader + "Galaxies_5_Frag.glsl",
			"Galaxies_5");
	}
	if (m_pGeodeSupercluster.valid() && m_pGeodeUltracluster.valid())
	{
		CGMKit::LoadShader(m_pGeodeSupercluster->getStateSet(),
			strGalaxyShader + "SuperclusterVert.glsl", strGalaxyShader + "SuperclusterFrag.glsl",
			"Supercluster");
		CGMKit::LoadShader(m_pGeodeUltracluster->getStateSet(),
			strGalaxyShader + "SuperclusterVert.glsl", strGalaxyShader + "SuperclusterFrag.glsl",
			"Ultracluster");
	}
	if (m_pGeodeMyWorld_5.valid() && m_pGeodeMyWorld_6.valid())
	{
		CGMKit::LoadShader(m_pGeodeMyWorld_5->getStateSet(),
			strGalaxyShader + "MyWorldVert.glsl", strGalaxyShader + "MyWorldFrag.glsl",
			"MyWorld_5");
		CGMKit::LoadShader(m_pGeodeMyWorld_6->getStateSet(),
			strGalaxyShader + "MyWorldVert.glsl", strGalaxyShader + "MyWorldFrag.glsl",
			"MyWorld_6");
	}
	if (m_pBackgroundGalaxyTransform.valid())
	{
		CGMKit::LoadShader(m_pBackgroundGalaxyTransform->getStateSet(),
			strGalaxyShader + "GalaxyBackgroundVert.glsl", strGalaxyShader + "GalaxyBackgroundFrag.glsl",
			"GalaxyBackground");
	}
	if (m_pCosmosBoxGeode.valid())
	{
		CGMKit::LoadShader(m_pCosmosBoxGeode->getStateSet(),
			strGalaxyShader + "CosmosBoxVert.glsl", strGalaxyShader + "CosmosBoxFrag.glsl",
			"CosmosBox");
	}

	m_pMilkyWay->Load();
	m_pSolarSystem->Load();

	return true;
}

bool CGMGalaxy::SaveSolarData()
{
	m_pSolarSystem->SaveSolarData();
	return true;
}

void CGMGalaxy::ResizeScreen(const int iW, const int iH)
{
	m_pMilkyWay->ResizeScreen(iW, iH);
	m_pSolarSystem->ResizeScreen(iW, iH);
}

void CGMGalaxy::SetEditMode(const bool bEnable)
{
	m_bEdit = bEnable;
	m_pStateSetPlane->setDefine("EDIT", bEnable ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
	if (bEnable)
	{
		m_pHandleSwitch->setNodeMask(~0);
		m_pHandleSwitch->setChildValue(m_pHandleSwitch->getChild(ID_HANDLE), true);
		m_pHandleSwitch->setChildValue(m_pHandleSwitch->getChild(ID_HANDLE_HOVER), false);
		m_pHandleSwitch->setChildValue(m_pHandleSwitch->getChild(ID_ARROW), false);

		_DetachAudioPoints();

		// 更新音频的UV
		m_pAudioUVUniform->set(_AudioCoord2UV(m_vPlayingAudioCoord));
	}
	else
	{
		m_pHandleSwitch->setNodeMask(0);
		_AttachAudioPoints();
	}
}

void CGMGalaxy::SetHandleHover(const bool bHover)
{
	if (!m_bCapture && bHover != m_bHandleHover)
	{
		m_bHandleHover = bHover;
		m_pHandleSwitch->setChildValue(m_pHandleSwitch->getChild(ID_HANDLE), !bHover);
		m_pHandleSwitch->setChildValue(m_pHandleSwitch->getChild(ID_HANDLE_HOVER), bHover);

		if (bHover)
		{
			// 在这里必须重新计算精确的“当前恒星世界空间位置”：m_vPlayingStarWorld4Pos
			SGMGalaxyCoord vGC = m_pDataManager->AudioCoord2GalaxyCoord(m_vPlayingAudioCoord);
			m_vPlayingStarWorld4Pos = osg::Vec3d(vGC.x, vGC.y, vGC.z) * m_fGalaxyRadius;

			// 半径方向的微小偏移
			SGMAudioCoord vAudioCoord_radius = SGMAudioCoord(
				m_vPlayingAudioCoord.BPM > m_pConfigData->fMinBPM
				? m_vPlayingAudioCoord.BPM - 0.01 : m_vPlayingAudioCoord.BPM + 0.01,
				m_vPlayingAudioCoord.angle);
			// 角度方向的微小偏移
			SGMAudioCoord vAudioCoord_angle = SGMAudioCoord(
				m_vPlayingAudioCoord.BPM,
				std::fmod(m_vPlayingAudioCoord.angle + 0.01, osg::PI * 2));


			SGMGalaxyCoord vGCRadius = m_pDataManager->AudioCoord2GalaxyCoord(vAudioCoord_radius);
			SGMGalaxyCoord vGCAngle = m_pDataManager->AudioCoord2GalaxyCoord(vAudioCoord_angle);
			osg::Vec3d vWorldPos_radius = osg::Vec3d(vGCRadius.x, vGCRadius.y, vGCRadius.z) * m_fGalaxyRadius;
			osg::Vec3d vWorldPos_angle = osg::Vec3d(vGCAngle.x, vGCAngle.y, vGCAngle.z) * m_fGalaxyRadius;
			osg::Vec3d vRadiusAxis = vWorldPos_radius - m_vPlayingStarWorld4Pos;
			vRadiusAxis.z() = 0;
			if (vWorldPos_radius.length() < m_vPlayingStarWorld4Pos.length()) vRadiusAxis *= -1;
			vRadiusAxis.normalize();
			osg::Vec3d vAngleAxis = vWorldPos_angle - m_vPlayingStarWorld4Pos;
			vAngleAxis.z() = 0;
			vAngleAxis.normalize();

			// vRadiusAxis 的银盘面旋转角 音频空间半径外方向的旋转角度(-PI,PI]
			float fRadiusTheta = std::atan2(vRadiusAxis.y(), vRadiusAxis.x());
			// vAngleAxis 的银盘面旋转角 音频空间角正方向的旋转角度(-PI,PI]
			float fAngleTheta = std::atan2(vAngleAxis.y(), vAngleAxis.x());

			// + Radius + Angle
			m_fPRPA = 0.5*(fRadiusTheta + fAngleTheta);
			if (std::abs(fRadiusTheta - fAngleTheta) > osg::PI)
			{
				m_fPRPA += osg::PI;
				if (m_fPRPA > osg::PI)
				{
					m_fPRPA -= osg::PI * 2;
				}
			}
		}
	}
}

void CGMGalaxy::SetCapture(const bool bEnable)
{
	if (bEnable != m_bCapture)
	{
		m_bCapture = bEnable;
		m_pStateSetPlane->setDefine("CAPTURE", bEnable ? osg::StateAttribute::ON : osg::StateAttribute::OFF);

		if (bEnable && (4 == m_pKernelData->iHierarchy))
		{
			m_pHandleSwitch->setChildValue(m_pHandleSwitch->getChild(ID_HANDLE), false);
			m_pHandleSwitch->setChildValue(m_pHandleSwitch->getChild(ID_HANDLE_HOVER), false);
			m_pHandleSwitch->setChildValue(m_pHandleSwitch->getChild(ID_ARROW), true);
		}
		else if (!bEnable)
		{
			m_eArrowDir = EGMAD_NONE;
			m_pHandleSwitch->setChildValue(m_pHandleSwitch->getChild(ID_HANDLE), !m_bHandleHover);
			m_pHandleSwitch->setChildValue(m_pHandleSwitch->getChild(ID_HANDLE_HOVER), m_bHandleHover);
			m_pHandleSwitch->setChildValue(m_pHandleSwitch->getChild(ID_ARROW), false);

			osg::Quat quat = osg::Quat(m_fPRPA, osg::Vec3d(0, 0, 1));
			m_pStar_4_Transform->asPositionAttitudeTransform()->setAttitude(quat);
		}
	}
}

void CGMGalaxy::Welcome()
{
	m_bWelcome = true;
	if (m_pStateSetGalaxy.valid())
	{
		m_pStateSetGalaxy->setDefine("WELCOME", osg::StateAttribute::ON);
	}
	if (m_pGeodeAudio.valid())
	{
		m_pGeodeAudio->getOrCreateStateSet()->setDefine("WELCOME", osg::StateAttribute::ON);
	}
}

bool CGMGalaxy::CreateGalaxy(double fDiameter)
{
	m_fGalaxyRadius = fDiameter * 0.5;
	m_pGalaxyRadiusUniform->set(float(m_fGalaxyRadius / m_pKernelData->fUnitArray->at(4)));

	// 0123层级空间下都可见的星空cube
	_CreateStarCube();

	// 3层级空间下的物体

	// 4层级空间下的物体
	_CreateAudioPoints();
	_CreateGalaxyPoints();
	_CreateGalaxyPointsN_4(2);	// 2倍密度的星星
	_CreateGalaxyPointsN_4(4);	// 4倍密度的星星
	_CreateGalaxyPointsN_4(8);	// 8倍密度的星星

	_CreateStarCube_4(); // 创建cube恒星，用于4级空间
	_CreateGalaxyPlane_4();
	_CreateGalaxies_4(); // 创建星系群，用于4级空间

	_CreateHandle(); // 创建用于移动恒星的圆盘状的把手

	// 5层级空间下的物体
	_CreateGalaxyPlane_5();
	_CreateGalaxies_5();
	//_CreateSupercluster();
	//_CreateUltracluster();

	if (m_pConfigData->bPhoto)
		_CreateMyWorld();

	// 背景银河
	_CreateBackgroundGalaxy();
	// 背景宇宙
	_CreateCosmosBox();

	if (EGMRENDER_LOW != m_pConfigData->eRenderQuality)
	{
		m_pMilkyWay->MakeMilkyWay(fDiameter, fDiameter, 0.13f * fDiameter);
	}

	m_pSolarSystem->CreateSolarSystem();

	return true;
}

void CGMGalaxy::SetMousePosition(const osg::Vec3d& vHierarchyPos)
{
	if (m_bWelcome) return;

	osg::Vec3f vPosHierarchy = vHierarchyPos;
	m_pMousePosUniform->set(vPosHierarchy);

	if (4 == m_pKernelData->iHierarchy)
	{
		m_vMouseWorldPos = vHierarchyPos * m_pKernelData->fUnitArray->at(4);
	}
	else
	{
		// to do
		//m_vMouseWorldPos = vHierarchyPos * m_pKernelData->fUnitArray->at();
	}
}

void CGMGalaxy::SetCurrentStar(const osg::Vec3d& vWorldPos, const std::wstring& wstrName)
{
	if (m_bWelcome || m_bEdit) return;

	osg::Vec3f vPosHierarchy = vWorldPos / m_pKernelData->fUnitArray->at(4);
	if (4 > m_pKernelData->iHierarchy)
	{
		vPosHierarchy = (vPosHierarchy - m_vPlayingStarWorld4Pos)*GM_UNIT_SCALE;
	}
	else
	{
		vPosHierarchy = vWorldPos / m_pKernelData->fUnitArray->at(m_pKernelData->iHierarchy);
	}
	m_pCommonUniform->SetStarHiePos(vPosHierarchy);
	m_strPlayingStarName = wstrName;

	// 设置激活恒星信息节点的位置
	if (m_pStarInfoTransform.valid())
	{
		m_pStarInfoTransform->asPositionAttitudeTransform()->setPosition(vPosHierarchy);
	}
	// 设置中心恒星、行星轨道等的位置
	if (m_pStar_4_Transform.valid())
	{
		m_pStar_4_Transform->asPositionAttitudeTransform()->setPosition(vPosHierarchy);
	}

	// 设置超新星的位置
	if (3 == m_pKernelData->iHierarchy)
	{
		osg::Vec3f vPosHie = (vPosHierarchy - m_vPlayingStarWorld4Pos)*GM_UNIT_SCALE;
		m_pSolarSystem->SetSupernovaHiePos(vPosHie);
	}
	else if (4 <= m_pKernelData->iHierarchy)
	{
		m_pSolarSystem->SetSupernovaHiePos(vPosHierarchy);
	}
	else {}

	m_vPlayingStarWorld4Pos = vWorldPos;
	// 设置当前音频星颜色
	m_pCommonUniform->SetStarColor(m_pDataManager->GetAudioColor(m_pDataManager->GetUID(wstrName)));
}

void CGMGalaxy::SetPlayingStarAudioCoord(const SGMAudioCoord& vAudioCoord)
{
	m_vPlayingAudioCoord = vAudioCoord;
	_UpdatePlayingStarInformation(vAudioCoord);
}

osg::Matrix CGMGalaxy::HierarchyAddMatrix() const
{
	osg::Vec3d vZeroPos = osg::Vec3d(0, 0, 0);
	osg::Quat qRotate = osg::Quat();
	switch (m_pKernelData->iHierarchy)
	{
	case 3:	// 2->3
	{
		SGMVector3 vPlanetPos;
		double fOrbitalPeriod;
		m_pSolarSystem->GetCelestialBody(vPlanetPos, fOrbitalPeriod);
		vZeroPos = osg::Vec3d(vPlanetPos.x, vPlanetPos.y, vPlanetPos.z) / m_pKernelData->fUnitArray->at(3);
	}
	break;
	case 4:	// 3->4
	{
		vZeroPos = m_vNearStarWorld4Pos / m_pKernelData->fUnitArray->at(4);
		qRotate = m_pSolarSystem->GetSolarRotate();
	}
	break;
	default:
		break;
	}

	if (2 < m_pKernelData->iHierarchy) // 2->3->4->5->6
	{
		osg::Matrix mHierarchyMatrix = osg::Matrix();
		mHierarchyMatrix.preMultTranslate(vZeroPos);
		mHierarchyMatrix.preMultRotate(qRotate);
		mHierarchyMatrix.preMultScale(osg::Vec3d(1.0, 1.0, 1.0) / GM_UNIT_SCALE);
		return mHierarchyMatrix;
	}
	else // 0,1,2
	{
		return m_pSolarSystem->HierarchyAddMatrix();
	}
}

osg::Matrix CGMGalaxy::HierarchySubMatrix() const
{
	osg::Vec3d vZeroPos = osg::Vec3d(0, 0, 0);
	osg::Quat qRotate = osg::Quat();
	switch (m_pKernelData->iHierarchy)
	{
	case 3: // 4->3
	{
		vZeroPos = m_vPlayingStarWorld4Pos / m_pKernelData->fUnitArray->at(4);
		qRotate = m_pSolarSystem->GetSolarRotate();
	}
	break;
	case 2: // 3->2
	{
		SGMVector3 vPlanetPos;
		double fOrbitalPeriod;
		m_pSolarSystem->GetCelestialBody(vPlanetPos, fOrbitalPeriod);
		vZeroPos = osg::Vec3d(vPlanetPos.x, vPlanetPos.y, vPlanetPos.z) / m_pKernelData->fUnitArray->at(3);
	}
	break;
	default:
		break;
	}

	if (1 < m_pKernelData->iHierarchy)// 6->5->4->3->2
	{
		osg::Matrix mHierarchyMatrix = osg::Matrix();
		mHierarchyMatrix.preMultScale(osg::Vec3d(GM_UNIT_SCALE, GM_UNIT_SCALE, GM_UNIT_SCALE));
		mHierarchyMatrix.preMultRotate(qRotate.inverse());
		mHierarchyMatrix.preMultTranslate(-vZeroPos);
		return mHierarchyMatrix;
	}
	else // 0,1
	{
		return m_pSolarSystem->HierarchySubMatrix();
	}
}

bool CGMGalaxy::GetNearestCelestialBody(const SGMVector3& vSearchHiePos,
	SGMVector3& vPlanetHiePos, double& fOrbitalPeriod)
{
	return m_pSolarSystem->GetNearestCelestialBody(vSearchHiePos, vPlanetHiePos, fOrbitalPeriod);
}

void CGMGalaxy::GetCelestialBody(SGMVector3& vPlanetPos, double & fOrbitalPeriod)
{
	m_pSolarSystem->GetCelestialBody(vPlanetPos, fOrbitalPeriod);
}

double CGMGalaxy::GetCelestialMeanRadius() const
{
	return m_pSolarSystem->GetCelestialMeanRadius();
}

double CGMGalaxy::GetCelestialRadius(const double fLatitude) const
{
	return m_pSolarSystem->GetCelestialRadius(fLatitude);
}

SGMVector3 CGMGalaxy::GetCelestialNorth() const
{
	return m_pSolarSystem->GetCelestialNorth();
}

unsigned int CGMGalaxy::GetCelestialIndex() const
{
	return m_pSolarSystem->GetCelestialIndex();
}

double CGMGalaxy::GetEyeAltitude() const
{
	return m_pSolarSystem->GetEyeAltitude();
}

bool CGMGalaxy::UpdateHierarchy(int iHieNew)
{
	// 移除下一空间层级
	if (iHieNew > 0 && GM_Root->containsNode(m_pHierarchyRootVector.at(iHieNew - 1).get()))
	{
		GM_Root->removeChild(m_pHierarchyRootVector.at(iHieNew - 1).get());
	}
	// 移除上一空间层级
	if (iHieNew < 6 && GM_Root->containsNode(m_pHierarchyRootVector.at(iHieNew + 1).get()))
	{
		GM_Root->removeChild(m_pHierarchyRootVector.at(iHieNew + 1).get());
	}
	// 添加当前空间层级
	if (!(GM_Root->containsNode(m_pHierarchyRootVector.at(iHieNew).get())))
	{
		GM_Root->addChild(m_pHierarchyRootVector.at(iHieNew).get());
	}

	switch (iHieNew)
	{
	case 0:
	{
		if (m_pBackgroundStarTransform.valid())
		{
			m_pBackgroundStarTransform->setNodeMask(~0);
			m_fStarDistanceUniform->set(1.0f);
		}
	}
	break;
	case 1:
	{
		if (m_pBackgroundStarTransform.valid())
		{
			m_pBackgroundStarTransform->setNodeMask(~0);
			m_fStarDistanceUniform->set(1.0f);
		}
	}
	break;
	case 2:
	{
		if (m_pBackgroundStarTransform.valid())
		{
			m_pBackgroundStarTransform->setNodeMask(~0);
			m_fStarDistanceUniform->set(1.0f);
		}
		m_pCommonUniform->SetStarHiePos(osg::Vec3f(0, 0, 0)); // to do
	}
	break;
	case 3:
	{
		osg::Vec3f vOffset = osg::Vec3f(
			std::fmod(m_vNearStarWorld4Pos.x(), GM_MIN_STARS_CUBE),
			std::fmod(m_vNearStarWorld4Pos.y(), GM_MIN_STARS_CUBE),
			std::fmod(m_vNearStarWorld4Pos.z(), GM_MIN_STARS_CUBE))
			/ m_pKernelData->fUnitArray->at(3);
		m_pCubeCenterUniform->set(vOffset);

		m_pSolarSystem->SetSupernovaHiePos(osg::Vec3(0, 0, 0));

		if (m_pBackgroundStarTransform.valid())
		{
			m_pBackgroundStarTransform->setNodeMask(~0);
		}
		m_fStarAlphaUniform->set(0.0f);
		m_pCommonUniform->SetStarHiePos(osg::Vec3f(0,0,0));
	}
	break;
	case 4:
	{
		osg::Vec3f vPosHierarchy = m_vPlayingStarWorld4Pos / m_pKernelData->fUnitArray->at(4);
		m_pSolarSystem->SetSupernovaHiePos(vPosHierarchy);
		if (m_pBackgroundStarTransform.valid())
		{
			m_pBackgroundStarTransform->setNodeMask(0);
		}
		if (m_pStar_4_Transform.valid())
		{
			m_pStar_4_Transform->asPositionAttitudeTransform()->setPosition(vPosHierarchy);
		}
		m_fStarDistanceUniform->set(4000.0f);
		m_pCommonUniform->SetStarHiePos(vPosHierarchy);
	}
	break;
	case 5:
	{
		m_pSolarSystem->SetSupernovaHiePos(m_vPlayingStarWorld4Pos / m_pKernelData->fUnitArray->at(5));

		if (m_pBackgroundStarTransform.valid())
		{
			m_pBackgroundStarTransform->setNodeMask(0);
		}
	}
	break;
	case 6:
	{
		if (m_pBackgroundStarTransform.valid())
		{
			m_pBackgroundStarTransform->setNodeMask(0);
		}
	}
	break;
	default:
		break;
	}

	if (iHieNew <= 4)
	{
		osg::Quat quat = osg::Quat();
		for (int i = iHieNew; i < 4; i++)
		{
			// to do
			if (3 == i)
			{
				quat = m_pSolarSystem->GetSolarRotate().inverse();
			}
		}
		m_pBackgroundGalaxyTransform->asPositionAttitudeTransform()->setAttitude(quat);
		m_pBackgroundStarTransform->asPositionAttitudeTransform()->setAttitude(quat);
	}

	m_pMilkyWay->UpdateHierarchy(iHieNew);
	m_pSolarSystem->UpdateHierarchy(iHieNew);

	return true;
}

SGMVector3 CGMGalaxy::UpdateCelestialBody(const SGMVector3& vTargetHiePos)
{
	return m_pSolarSystem->UpdateCelestialBody(vTargetHiePos);
}

osg::Quat CGMGalaxy::GetNearStarRotate() const
{
	return m_pSolarSystem->GetSolarRotate();
}

void CGMGalaxy::SetWanderingEarthProgress(const float fProgress)
{
	m_pSolarSystem->SetWanderingEarthProgress(fProgress);
}

float CGMGalaxy::GetWanderingEarthProgress() const
{
	return m_pSolarSystem->GetWanderingEarthProgress();
}

void CGMGalaxy::_InitBackground()
{
	m_pCosmosBoxNode = new CCosmosBox();
	m_pKernelData->pBackgroundCam->addChild(m_pCosmosBoxNode.get());
}

bool CGMGalaxy::_CreateStarCube()
{
	m_pBackgroundStarTransform = new osg::PositionAttitudeTransform();
	m_pBackgroundStarTransform->setNodeMask(0);
	osg::ref_ptr<osg::StateSet> pSS = m_pBackgroundStarTransform->getOrCreateStateSet();
	pSS->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
	pSS->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	pSS->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSS->setAttributeAndModes(new osg::BlendFunc(
		GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_ALPHA, GL_ONE), osg::StateAttribute::ON);
	pSS->setRenderBinDetails(BIN_STARS_1, "DepthSortedBin");

	pSS->addUniform(m_pCubeCenterUniform.get());
	pSS->addUniform(m_pCommonUniform->GetUnit());
	pSS->addUniform(m_fStarDistanceUniform.get());
	pSS->addUniform(m_pCommonUniform->GetStarColor());

	// 添加shader
	std::string strStarVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarCube_Vert.glsl";
	std::string strStarFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarCube_Frag.glsl";
	CGMKit::LoadShader(pSS.get(), strStarVertPath, strStarFragPath, "StarCube");

	float fCubeSize = GM_MIN_STARS_CUBE / m_pKernelData->fUnitArray->at(3);
	size_t iNum = m_pCubeVertArray->size();
	osg::ref_ptr<osg::Vec4Array> vertArray = new osg::Vec4Array;
	vertArray->reserve(iNum);
	for (size_t i = 0; i < iNum; i++)
	{
		osg::Vec4 vert = m_pCubeVertArray->at(i);
		vertArray->push_back(vert*fCubeSize);
	}

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
	pGeometry->setVertexArray(vertArray.get());
	pGeometry->setColorArray(m_pCubeColorArray.get());
	pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 1, 0));
	pGeometry->setNormalArray(normal.get());
	pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
	pGeometry->addPrimitiveSet(m_pCubeElement.get());

	pGeometry->setUseVertexBufferObjects(true);
	pGeometry->setUseDisplayList(false);
	pGeometry->setDataVariance(osg::Object::DYNAMIC);

	osg::ref_ptr<osg::Geode> pGeodeStarCube = new osg::Geode();
	pGeodeStarCube->addDrawable(pGeometry.get());
	m_pBackgroundStarTransform->addChild(pGeodeStarCube.get());
	m_pCosmosBoxNode->addChild(m_pBackgroundStarTransform.get());
	return true;
}

bool CGMGalaxy::_CreateHandle()
{
	m_pHandleSwitch = new osg::Switch;
	m_pStar_4_Transform->addChild(m_pHandleSwitch.get());
	osg::ref_ptr<osg::StateSet>	pStateSet = m_pHandleSwitch->getOrCreateStateSet();
	pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
	pStateSet->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pStateSet->setRenderBinDetails(BIN_HANDLE, "DepthSortedBin");
	// 添加shader
	std::string strHandleVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Handle_Vert.glsl";
	std::string strHandleFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Handle_Frag.glsl";
	CGMKit::LoadShader(pStateSet.get(), strHandleVertPath, strHandleFragPath, "Handle");

	std::string strGalaxyTexPath = m_pConfigData->strCorePath + m_strGalaxyTexPath;

	const float fWidth = 2.0f*GM_HANDLE_RADIUS;
	osg::ref_ptr<osg::Geode> pHandleGeode = new osg::Geode();
	pHandleGeode->addDrawable(_CreateSquareGeometry(fWidth));
	osg::ref_ptr<osg::StateSet>	pSSHandle = pHandleGeode->getOrCreateStateSet();
	osg::ref_ptr<osg::Texture> pHandleTex = _CreateTexture2D(strGalaxyTexPath + "handle.tga", 4);
	pSSHandle->setTextureAttributeAndModes(0, pHandleTex.get());
	pSSHandle->addUniform(new osg::Uniform("baseTex", 0));
	m_pHandleSwitch->insertChild(ID_HANDLE, pHandleGeode.get(), false);

	osg::ref_ptr<osg::Geode> pHandleHoverGeode = new osg::Geode();
	pHandleHoverGeode->addDrawable(_CreateSquareGeometry(fWidth));
	osg::ref_ptr<osg::StateSet>	pSSHandleHover = pHandleHoverGeode->getOrCreateStateSet();
	osg::ref_ptr<osg::Texture> pHandleHoverTex = _CreateTexture2D(strGalaxyTexPath + "handle_hover.tga", 4);
	pSSHandleHover->setTextureAttributeAndModes(0, pHandleHoverTex.get());
	pSSHandleHover->addUniform(new osg::Uniform("baseTex", 0));
	m_pHandleSwitch->insertChild(ID_HANDLE_HOVER, pHandleHoverGeode.get(), false);

	osg::ref_ptr<osg::Geode> pArrowGeode = new osg::Geode();
	pArrowGeode->addDrawable(_CreateSquareGeometry(GM_HANDLE_RADIUS, true));
	osg::ref_ptr<osg::StateSet>	pSSArrow = pArrowGeode->getOrCreateStateSet();
	osg::ref_ptr<osg::Texture> pArrowTex = _CreateTexture2D(strGalaxyTexPath + "handle_arrow.tga", 4);
	pSSArrow->setTextureAttributeAndModes(0, pArrowTex.get());
	pSSArrow->addUniform(new osg::Uniform("baseTex", 0));
	m_pHandleSwitch->insertChild(ID_ARROW, pArrowGeode.get(), false);

	return true;
}

bool CGMGalaxy::_CreateAudioPoints()
{
	m_pGeodeAudio = new osg::Geode();
	m_pHierarchyRootVector.at(4)->addChild(m_pGeodeAudio.get());

	// 音频数据
	std::map<unsigned int, SGMAudioData> audioDataMap;
	m_pDataManager->GetAudioDataMap(audioDataMap);

	// 从数据管理模块读取数据，创建未激活状态的音频星几何体
	osg::ref_ptr<osg::Geometry> pGeomAudio = _CreateAudioGeometry(audioDataMap);
	if (!pGeomAudio.valid()) return false;
	m_pGeodeAudio->addDrawable(pGeomAudio.get());

	osg::ref_ptr<osg::StateSet> pStateSetAudio = m_pGeodeAudio->getOrCreateStateSet();
	pStateSetAudio->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
	pStateSetAudio->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	pStateSetAudio->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pStateSetAudio->setMode(GL_BLEND, osg::StateAttribute::ON);
	pStateSetAudio->setAttributeAndModes(new osg::BlendFunc(
		GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pStateSetAudio->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pStateSetAudio->setRenderBinDetails(BIN_AUDIO_POINT, "DepthSortedBin");

	pStateSetAudio->addUniform(m_pMousePosUniform.get());
	pStateSetAudio->addUniform(m_pCommonUniform->GetStarHiePos());
	pStateSetAudio->addUniform(m_pCommonUniform->GetLevelArray());
	pStateSetAudio->addUniform(m_fStarAlphaUniform.get());

	// 添加shader
	std::string strAudioVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "AudioVert.glsl";
	std::string strAudioFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "AudioFrag.glsl";
	return CGMKit::LoadShader(pStateSetAudio.get(), strAudioVertPath, strAudioFragPath, "Audio");
}

bool CGMGalaxy::_CreateGalaxyPoints()
{
	// 4级空间下的星系半径
	double fGalaxyRadius4 = m_fGalaxyRadius / m_pKernelData->fUnitArray->at(4);

	osg::ref_ptr<osg::Geode> pGalaxyPointsGeode = new osg::Geode();
	m_pHierarchyRootVector.at(4)->addChild(pGalaxyPointsGeode.get());

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();

	size_t iNum = 65536;
	osg::ref_ptr<osg::Vec3Array> vertArray = new osg::Vec3Array();
	osg::ref_ptr<osg::Vec2Array> texcoordArray = new osg::Vec2Array();
	osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array();
	osg::ref_ptr<osg::DrawElementsUShort> el = new osg::DrawElementsUShort(GL_POINTS);
	vertArray->reserve(iNum);
	texcoordArray->reserve(iNum);
	colorArray->reserve(iNum);
	el->reserve(iNum);

	if (!m_pGalaxyHeightImage.valid())
	{
		std::string strFile = m_pConfigData->strCorePath + m_strGalaxyTexPath + "milkyWay_height.tga";
		m_pGalaxyHeightImage = osgDB::readImageFile(strFile);
	}

	std::uniform_int_distribution<> iPseudoNoise(0, 10000);

	int x = 0;
	while (x < iNum)
	{
		float fRandomX = iPseudoNoise(m_iRandom)*1e-4f - 0.5f;
		float fRandomY = iPseudoNoise(m_iRandom)*1e-4f - 0.5f;
		float fRandomAlpha = iPseudoNoise(m_iRandom)*1e-4f;
		float fX = fGalaxyRadius4 * 2.0f * fRandomX;
		float fY = fGalaxyRadius4 * 2.0f * fRandomY;
		float fU = fRandomX + 0.5f;
		float fV = fRandomY + 0.5f;

		float fA = CGMKit::GetImageColor(m_pGalaxyImage.get(), fU, fV).a();
		if (fRandomAlpha < fA)
		{
			float fRandomR = iPseudoNoise(m_iRandom)*1e-4f - 0.5f;
			float fR = max(0.0f, CGMKit::GetImageColor(m_pGalaxyImage.get(), fU, fV).r() + fRandomR * fRandomR * fRandomR);
			float fG = CGMKit::GetImageColor(m_pGalaxyImage.get(), fU, fV).g();
			float fB = CGMKit::GetImageColor(m_pGalaxyImage.get(), fU, fV).b();

			float fRGBMax = max(max(max(fR, fG), fB), 1e-5);
			fR /= fRGBMax;
			fG /= fRGBMax;
			fB /= fRGBMax;

			float fRandomZ = iPseudoNoise(m_iRandom)*2e-4f - 1.0f;
			float fSignZ = (fRandomZ > 0) ? 1.0f : -1.0f;
			float fSmooth = fSignZ * (3 * fRandomZ*fRandomZ - 2 * abs(fRandomZ*fRandomZ*fRandomZ));
			float fZ = (fRandomAlpha + 0.2f)*0.05f*fGalaxyRadius4*fSmooth;
			float fRandomRadius = iPseudoNoise(m_iRandom)*1e-4f;
			fRandomRadius = (fA + CGMKit::GetImageColor(m_pGalaxyHeightImage.get(), fU, fV, true).z())
				* fRandomRadius * fRandomRadius;
			float fRadiusNow = osg::Vec2(fRandomX, fRandomY).length();
			float fTmp = pow(min(1.0f, 1.03f*(1.0f - fRadiusNow)), 11);
			fZ *= 0.5 + 3 * fTmp*fTmp - 2 * fTmp*fTmp*fTmp;
			vertArray->push_back(osg::Vec3(fX, fY, fZ));
			texcoordArray->push_back(osg::Vec2(fU, fV));
			colorArray->push_back(osg::Vec4(fR, fG, fB, fRandomRadius));
			el->push_back(x);
			x++;
		}
	}

	pGeometry->setVertexArray(vertArray.get());
	pGeometry->setTexCoordArray(0, texcoordArray.get());
	pGeometry->setColorArray(colorArray.get());
	pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 1, 0));
	pGeometry->setNormalArray(normal.get());
	pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
	pGeometry->addPrimitiveSet(el.get());

	pGeometry->setUseVertexBufferObjects(true);
	pGeometry->setUseDisplayList(false);
	pGeometry->setDataVariance(osg::Object::DYNAMIC);

	pGalaxyPointsGeode->addDrawable(pGeometry.get());
	m_pStateSetGalaxy = pGalaxyPointsGeode->getOrCreateStateSet();

	m_pStateSetGalaxy->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);

	m_pStateSetGalaxy->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	m_pStateSetGalaxy->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	m_pStateSetGalaxy->setMode(GL_BLEND, osg::StateAttribute::ON);
	m_pStateSetGalaxy->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	m_pStateSetGalaxy->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	m_pStateSetGalaxy->setRenderBinDetails(BIN_STARS_0, "DepthSortedBin");

	int iUnit = 0;
	if (EGMRENDER_LOW != m_pConfigData->eRenderQuality)
	{
		m_pStateSetGalaxy->setDefine("HIGH_QUALITY", osg::StateAttribute::ON);
		CGMKit::AddTexture(m_pStateSetGalaxy.get(), m_pMilkyWay->GetDistanceMap(), "distanceTex", iUnit++);
		m_pStateSetGalaxy->addUniform(m_pCommonUniform->GetScreenSize());
	}

	m_pStateSetGalaxy->addUniform(m_pCommonUniform->GetStarHiePos());
	m_pStateSetGalaxy->addUniform(m_pCommonUniform->GetLevelArray());
	m_pStateSetGalaxy->addUniform(m_pCommonUniform->GetStarColor());
	m_pStateSetGalaxy->addUniform(m_fStarAlphaUniform.get());

	// 添加shader
	std::string strStarVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyStarVert.glsl";
	std::string strStarFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyStarFrag.glsl";
	return CGMKit::LoadShader(m_pStateSetGalaxy.get(), strStarVertPath, strStarFragPath, "GalaxyStar");
}

bool CGMGalaxy::_CreateStarCube_4()
{
	float fCubeMinSize = GM_MIN_STARS_CUBE / m_pKernelData->fUnitArray->at(4);

	if (!m_pGeodeStarCube_4.valid())
	{
		m_pGeodeStarCube_4 = new osg::Geode();
		m_pEye_4_Transform->addChild(m_pGeodeStarCube_4.get());
		osg::ref_ptr<osg::StateSet> pSS_4 = m_pGeodeStarCube_4->getOrCreateStateSet();

		pSS_4->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
		pSS_4->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
		pSS_4->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pSS_4->setMode(GL_BLEND, osg::StateAttribute::ON);
		pSS_4->setAttributeAndModes(new osg::BlendFunc(
			GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_ALPHA, GL_ONE), osg::StateAttribute::ON);
		pSS_4->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
		pSS_4->setRenderBinDetails(BIN_STARS_1, "DepthSortedBin");

		// x = cubeMinSize, y = targetDistance
		m_pStarsCubeInfoUniform = new osg::Uniform("cubeInfo", osg::Vec2f(fCubeMinSize, 1.0f));
		pSS_4->addUniform(m_pStarsCubeInfoUniform.get());
		pSS_4->addUniform(m_fGalaxyHeightUniform.get());
		pSS_4->addUniform(m_pCommonUniform->GetStarColor());
		pSS_4->addUniform(m_pShapeUVWUniform.get());
		pSS_4->addUniform(m_fStarAlphaUniform.get());

		int iUnit = 1;
		CGMKit::AddTexture(pSS_4.get(), m_3DShapeTex.get(), "shapeNoiseTex", iUnit++);

		// 添加shader
		std::string strStarVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarCube_4_Vert.glsl";
		std::string strStarFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarCube_4_Frag.glsl";
		CGMKit::LoadShader(pSS_4.get(), strStarVertPath, strStarFragPath, "StarCube_4");
	}

	size_t iNum = m_pCubeVertArray->size();
	for (int i = 0; i < 4; i++)
	{
		float fCubeSize = fCubeMinSize * std::pow(2, float(i));

		osg::ref_ptr<osg::Vec4Array> vertArray = new osg::Vec4Array;
		vertArray->reserve(iNum);
		for (size_t j = 0; j < iNum; j++)
		{
			osg::Vec4 vert = m_pCubeVertArray->at(j);
			vertArray->push_back(vert*fCubeSize);
		}

		osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
		pGeometry->setNodeMask(0);
		pGeometry->setVertexArray(vertArray.get());
		pGeometry->setColorArray(m_pCubeColorArray.get());
		pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

		osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
		normal->push_back(osg::Vec3(0, 1, 0));
		pGeometry->setNormalArray(normal.get());
		pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
		pGeometry->addPrimitiveSet(m_pCubeElement.get());

		pGeometry->setUseVertexBufferObjects(true);
		pGeometry->setUseDisplayList(false);
		pGeometry->setDataVariance(osg::Object::DYNAMIC);

		m_pStarsGeomVector.push_back(pGeometry);
		m_pGeodeStarCube_4->addDrawable(pGeometry.get());
	}

	return true;
}

bool CGMGalaxy::_CreateGalaxyPointsN_4(int iDens)
{
	/**
	* 创建N倍密度的PointSprite星系点阵
	* 虽然最终渲染的范围是1/N个星系的尺寸
	* 然后在vertex shader中进行平移和缩放
	* 点阵的位置根据三维纹理设置，保证看不出重复感
	*/
	if (!m_pGeodePointsN_4.valid())
	{
		m_pGeodePointsN_4 = new osg::Geode();
		m_pHierarchyRootVector.at(4)->addChild(m_pGeodePointsN_4.get());
		osg::ref_ptr<osg::StateSet> pSSN = m_pGeodePointsN_4->getOrCreateStateSet();

		pSSN->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
		pSSN->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
		pSSN->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pSSN->setMode(GL_BLEND, osg::StateAttribute::ON);
		pSSN->setAttributeAndModes(new osg::BlendFunc(
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
		), osg::StateAttribute::ON);
		pSSN->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
		pSSN->setRenderBinDetails(BIN_STARS_1, "DepthSortedBin");

		int iUnit = 0;
		if (EGMRENDER_LOW != m_pConfigData->eRenderQuality)
		{
			pSSN->setDefine("HIGH_QUALITY", osg::StateAttribute::ON);
			CGMKit::AddTexture(pSSN.get(), m_pMilkyWay->GetDistanceMap(), "distanceTex", iUnit++);
			pSSN->addUniform(m_pCommonUniform->GetScreenSize());
		}
		CGMKit::AddTexture(pSSN.get(), m_pGalaxyColorTex.get(), "galaxyTex", iUnit++);

		pSSN->addUniform(m_pCommonUniform->GetStarHiePos());
		pSSN->addUniform(m_pCommonUniform->GetLevelArray());
		pSSN->addUniform(m_pCommonUniform->GetStarColor());
		pSSN->addUniform(m_pGalaxyRadiusUniform.get());
		pSSN->addUniform(m_fStarAlphaUniform.get());

		// 添加shader
		std::string strStarVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarNVert.glsl";
		std::string strStarFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarNFrag.glsl";
		CGMKit::LoadShader(pSSN.get(), strStarVertPath, strStarFragPath, "StarN");
	}

	osg::Vec3f vUVW;
	m_pShapeUVWUniform->get(vUVW);

	float fDens = float(osg::clampBetween(iDens, 2, 8));
	// 4级空间下的星系半径
	double fGalaxyRadius4 = m_fGalaxyRadius / m_pKernelData->fUnitArray->at(4);
	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
	size_t iNum = 65536;
	osg::ref_ptr<osg::Vec3Array> vertArray = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> texcoordArray = new osg::Vec4Array;
	osg::ref_ptr<osg::DrawElementsUShort> el = new osg::DrawElementsUShort(GL_POINTS);
	vertArray->reserve(iNum);
	texcoordArray->reserve(iNum);
	el->reserve(iNum);

	std::uniform_int_distribution<> iPseudoNoise(0, 10000);

	float fMinAlpha = 0.25f;
	int x = 0;
	while (x < iNum)
	{
		float fU = iPseudoNoise(m_iRandom)*1e-4f;
		float fV = iPseudoNoise(m_iRandom)*1e-4f;
		float fW = iPseudoNoise(m_iRandom)*1e-4f;
		float fDiameter = fGalaxyRadius4 * 2.0f;
		float fUScale = fDiameter * vUVW.x() / fDens;
		float fVScale = fDiameter * vUVW.y() / fDens;
		float fAlpha = 1.0f - _Get3DValue(fU * fUScale, fV * fVScale, fW);
		fAlpha *= fAlpha;
		if (fAlpha > fMinAlpha)
		{
			float fRandomX = fU - 0.5f;
			float fRandomY = fV - 0.5f;
			float fRandomZ = fW - 0.5f;
			float fX = fDiameter * fRandomX;
			float fY = fDiameter * fRandomY;
			float fZ = fRandomZ / vUVW.z();

			vertArray->push_back(osg::Vec3(fX, fY, fZ));
			// fU、fV不需要传入，所以在UV的位置上存放缩放系数
			texcoordArray->push_back(osg::Vec4(fDens, fDens, fW, (fAlpha - fMinAlpha) / (1.0f - fMinAlpha)));
			el->push_back(x);
			x++;
		}
	}

	pGeometry->setVertexArray(vertArray.get());
	pGeometry->setTexCoordArray(0, texcoordArray.get());

	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 1, 0));
	pGeometry->setNormalArray(normal.get());
	pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
	pGeometry->addPrimitiveSet(el.get());

	pGeometry->setUseVertexBufferObjects(true);
	pGeometry->setUseDisplayList(false);
	pGeometry->setDataVariance(osg::Object::DYNAMIC);

	m_pGeodePointsN_4->addDrawable(pGeometry.get());

	return true;
}

bool CGMGalaxy::_CreateGalaxyPlane_4()
{
	// 4级空间下的星系半径
	double fGalaxyRadius4 = m_fGalaxyRadius / m_pKernelData->fUnitArray->at(4);

	osg::ref_ptr<osg::Geode> pGalaxyPlaneGeode = new osg::Geode();
	m_pHierarchyRootVector.at(4)->addChild(pGalaxyPlaneGeode.get());

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
	osg::ref_ptr<osg::Vec3Array> verArray = new osg::Vec3Array;
	verArray->push_back(osg::Vec3(-fGalaxyRadius4, -fGalaxyRadius4, 0));
	verArray->push_back(osg::Vec3(fGalaxyRadius4, -fGalaxyRadius4, 0));
	verArray->push_back(osg::Vec3(fGalaxyRadius4, fGalaxyRadius4, 0));
	verArray->push_back(osg::Vec3(-fGalaxyRadius4, fGalaxyRadius4, 0));
	pGeometry->setVertexArray(verArray);

	osg::ref_ptr<osg::Vec2Array> textArray = new osg::Vec2Array;
	textArray->push_back(osg::Vec2(0, 0));
	textArray->push_back(osg::Vec2(1, 0));
	textArray->push_back(osg::Vec2(1, 1));
	textArray->push_back(osg::Vec2(0, 1));
	pGeometry->setTexCoordArray(0, textArray);

	osg::ref_ptr < osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 0, 1));
	pGeometry->setNormalArray(normal);
	pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

	pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

	pGeometry->setUseVertexBufferObjects(true);
	pGeometry->setUseDisplayList(false);
	pGeometry->setDataVariance(osg::Object::DYNAMIC);

	pGalaxyPlaneGeode->addDrawable(pGeometry.get());

	m_pStateSetPlane = pGalaxyPlaneGeode->getOrCreateStateSet();
	m_pStateSetPlane->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	m_pStateSetPlane->setMode(GL_BLEND, osg::StateAttribute::ON);
	m_pStateSetPlane->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	m_pStateSetPlane->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	m_pStateSetPlane->setRenderBinDetails(BIN_GALAXY_PLANE, "DepthSortedBin");

	int iUnit = 0;
	CGMKit::AddTexture(m_pStateSetPlane.get(), m_pGalaxyColorTex.get(), "galaxyTex", iUnit++);

	m_pStateSetPlane->addUniform(m_pCommonUniform->GetLevelArray());
	m_pStateSetPlane->addUniform(m_pMousePosUniform.get());

	// 添加shader
	std::string strGalaxyPlaneVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyPlaneVert.glsl";
	std::string strGalaxyPlaneFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyPlaneFrag.glsl";
	return CGMKit::LoadShader(m_pStateSetPlane.get(), strGalaxyPlaneVertPath, strGalaxyPlaneFragPath, "GalaxyPlane");
}

bool CGMGalaxy::_CreateGalaxies_4()
{
	size_t iMaxNum = m_pCubeVertArray->size();

	if (!m_pGeodeGalaxyGroup_4.valid())
	{
		m_pGeodeGalaxyGroup_4 = new osg::Geode();
		m_pEye_4_Transform->addChild(m_pGeodeGalaxyGroup_4.get());
		osg::ref_ptr<osg::StateSet> pSSG_4 = m_pGeodeGalaxyGroup_4->getOrCreateStateSet();

		pSSG_4->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
		pSSG_4->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
		pSSG_4->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pSSG_4->setMode(GL_BLEND, osg::StateAttribute::ON);
		pSSG_4->setAttributeAndModes(new osg::BlendFunc(
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
		), osg::StateAttribute::ON);
		pSSG_4->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
		pSSG_4->setRenderBinDetails(BIN_GALAXIES, "DepthSortedBin");

		pSSG_4->addUniform(m_fStarAlphaUniform.get());

		int iUnit = 1;
		CGMKit::AddTexture(pSSG_4.get(), m_pGalaxiesTex.get(), "galaxiesTex", iUnit++);

		// 添加shader
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxies_4_Vert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxies_4_Frag.glsl";
		CGMKit::LoadShader(pSSG_4.get(), strVertPath, strFragPath, "Galaxies_4");
	}

	osg::ref_ptr<osg::Vec4Array> vertArray = new osg::Vec4Array;
	osg::ref_ptr<osg::Vec4Array> galaxiesInfoArray = new osg::Vec4Array;
	vertArray->reserve(iMaxNum);
	galaxiesInfoArray->reserve(iMaxNum);

	int iEleNum = 0;
	for (size_t i = 0; i < iMaxNum; i++)
	{
		osg::Vec4 vPos = m_pCubeVertArray->at(i);
		osg::Vec3 vVertPos = osg::Vec3(vPos.x(), vPos.y(), vPos.z());
		float fLength = vVertPos.normalize();
		if (fLength <= 0.5f)
		{
			float fFalloff = 0.75f*osg::clampBetween(1.0f - fLength / 0.5f, 0.0f, 1.0f);
			osg::Vec4 vert = osg::Vec4(vVertPos, fFalloff);
			vertArray->push_back(vert);
			osg::Vec4 info = m_pGalaxiesInfo->at(i);
			galaxiesInfoArray->push_back(info);
		}
	}

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
	pGeometry->setVertexArray(vertArray.get());
	pGeometry->setColorArray(galaxiesInfoArray.get());
	pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 1, 0));
	pGeometry->setNormalArray(normal.get());
	pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
	pGeometry->addPrimitiveSet(m_pSphereElement.get());

	pGeometry->setUseVertexBufferObjects(true);
	pGeometry->setUseDisplayList(false);
	pGeometry->setDataVariance(osg::Object::DYNAMIC);

	m_pGeodeGalaxyGroup_4->addDrawable(pGeometry.get());

	return true;
}

bool CGMGalaxy::_CreateGalaxyPlane_5()
{
	// 5级空间下的星系半径
	double fGalaxyRadius5 = m_fGalaxyRadius / m_pKernelData->fUnitArray->at(5);

	osg::ref_ptr<osg::Geode> pGalaxy_5_Geode = new osg::Geode();
	m_pHierarchyRootVector.at(5)->addChild(pGalaxy_5_Geode.get());

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
	osg::ref_ptr<osg::Vec3Array> verArray = new osg::Vec3Array;
	verArray->push_back(osg::Vec3(-fGalaxyRadius5, -fGalaxyRadius5, 0));
	verArray->push_back(osg::Vec3(fGalaxyRadius5, -fGalaxyRadius5, 0));
	verArray->push_back(osg::Vec3(fGalaxyRadius5, fGalaxyRadius5, 0));
	verArray->push_back(osg::Vec3(-fGalaxyRadius5, fGalaxyRadius5, 0));
	pGeometry->setVertexArray(verArray);

	osg::ref_ptr<osg::Vec2Array> textArray = new osg::Vec2Array;
	textArray->push_back(osg::Vec2(0, 0));
	textArray->push_back(osg::Vec2(1, 0));
	textArray->push_back(osg::Vec2(1, 1));
	textArray->push_back(osg::Vec2(0, 1));
	pGeometry->setTexCoordArray(0, textArray);

	osg::ref_ptr< osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 0, 1));
	pGeometry->setNormalArray(normal);
	pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

	pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

	pGeometry->setUseVertexBufferObjects(true);
	pGeometry->setUseDisplayList(false);
	pGeometry->setDataVariance(osg::Object::DYNAMIC);

	pGalaxy_5_Geode->addDrawable(pGeometry.get());

	//!< 5级空间下银河盘面的状态集
	osg::ref_ptr<osg::StateSet>	pStateSet_5 = pGalaxy_5_Geode->getOrCreateStateSet();
	pStateSet_5->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pStateSet_5->setMode(GL_BLEND, osg::StateAttribute::ON);
	pStateSet_5->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pStateSet_5->setRenderBinDetails(BIN_GALAXY_PLANE, "DepthSortedBin");

	int iUnit = 0;
	std::string strGalaxyTexPath = m_pConfigData->strCorePath + m_strGalaxyTexPath;
	CGMKit::AddTexture(pStateSet_5.get(), _CreateTexture2D(m_pGalaxyImage.get(), 4), "galaxyTex", iUnit++);

	// 添加shader
	std::string strGalaxyPlaneVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxy_5_Vert.glsl";
	std::string strGalaxyPlaneFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxy_5_Frag.glsl";
	return CGMKit::LoadShader(pStateSet_5.get(), strGalaxyPlaneVertPath, strGalaxyPlaneFragPath, "Galaxy_5");
}

bool CGMGalaxy::_CreateGalaxies_5()
{
	if (!m_pEye_5_Transform.valid())
	{
		m_pEye_5_Transform = new osg::PositionAttitudeTransform();
		m_pHierarchyRootVector.at(5)->addChild(m_pEye_5_Transform.get());
	}

	size_t iNum = m_pCubeVertArray->size();
	double fCubeMinSize = GM_MIN_GALAXIES_CUBE / m_pKernelData->fUnitArray->at(5);
	m_pGalaxiesInfoUniform = new osg::Uniform("cubeInfo", osg::Vec2f(fCubeMinSize, 1.0f));

	// 第0层星系群，单独处理
	if (!m_pGalaxyGroup_Transform.valid())
	{
		m_pGalaxyGroup_Transform = new osg::PositionAttitudeTransform();
		m_pHierarchyRootVector.at(5)->addChild(m_pGalaxyGroup_Transform.get());

		m_pGeodeGalaxyGroup_5 = new osg::Geode();
		m_pGalaxyGroup_Transform->addChild(m_pGeodeGalaxyGroup_5.get());
		osg::ref_ptr<osg::StateSet> pSSGroup = m_pGeodeGalaxyGroup_5->getOrCreateStateSet();

		pSSGroup->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
		pSSGroup->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
		pSSGroup->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pSSGroup->setMode(GL_BLEND, osg::StateAttribute::ON);
		pSSGroup->setAttributeAndModes(new osg::BlendFunc(
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
		), osg::StateAttribute::ON);
		pSSGroup->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
		pSSGroup->setRenderBinDetails(BIN_GALAXIES, "DepthSortedBin");

		pSSGroup->addUniform(m_pGalaxiesInfoUniform.get());

		int iUnit = 1;
		CGMKit::AddTexture(pSSGroup.get(), m_pGalaxiesTex.get(), "galaxiesTex", iUnit++);

		// 添加shader
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxies_5_Vert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxies_5_Frag.glsl";
		CGMKit::LoadShader(pSSGroup.get(), strVertPath, strFragPath, "Galaxies_5");

		osg::ref_ptr<osg::Vec4Array> vertArray = new osg::Vec4Array;
		vertArray->reserve(iNum);
		for (size_t i = 0; i < iNum; i++)
		{
			osg::Vec4 vert = m_pCubeVertArray->at(i);
			vertArray->push_back(vert*fCubeMinSize);
		}

		osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
		pGeometry->setNodeMask(0);
		pGeometry->setVertexArray(vertArray.get());
		pGeometry->setColorArray(m_pGalaxiesInfo.get());
		pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

		osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
		normal->push_back(osg::Vec3(0, 1, 0));
		pGeometry->setNormalArray(normal.get());
		pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
		pGeometry->addPrimitiveSet(m_pCubeElement.get());

		pGeometry->setUseVertexBufferObjects(true);
		pGeometry->setUseDisplayList(false);
		pGeometry->setDataVariance(osg::Object::DYNAMIC);

		// 注意：这里必须是将最小星系群放在第一个位置，否则显影会出问题
		m_pGalaxiesGeomVector.push_back(pGeometry);
		m_pGeodeGalaxyGroup_5->addDrawable(pGeometry.get());
	}

	if (!m_pGeodeGalaxies_5.valid())
	{
		m_pGeodeGalaxies_5 = new osg::Geode();
		m_pEye_5_Transform->addChild(m_pGeodeGalaxies_5.get());
		osg::ref_ptr<osg::StateSet> pSSG_5 = m_pGeodeGalaxies_5->getOrCreateStateSet();

		pSSG_5->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
		pSSG_5->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
		pSSG_5->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pSSG_5->setMode(GL_BLEND, osg::StateAttribute::ON);
		pSSG_5->setAttributeAndModes(new osg::BlendFunc(
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
		), osg::StateAttribute::ON);
		pSSG_5->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
		pSSG_5->setRenderBinDetails(BIN_GALAXIES, "DepthSortedBin");

		pSSG_5->addUniform(m_pGalaxiesInfoUniform.get());

		int iUnit = 1;
		CGMKit::AddTexture(pSSG_5.get(), m_pGalaxiesTex.get(), "galaxiesTex", iUnit++);

		// 添加shader
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxies_5_Vert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Galaxies_5_Frag.glsl";
		CGMKit::LoadShader(pSSG_5.get(), strVertPath, strFragPath, "Galaxies_5");
	}

	// 在眼点变换节点下，只创建第1、2、3层星系群，第0层星系群，单独处理
	for (int i = 1; i < 4; i++)
	{
		float fCubeSize = fCubeMinSize * std::pow(2, float(i));

		osg::ref_ptr<osg::Vec4Array> vertArray = new osg::Vec4Array;
		vertArray->reserve(iNum);
		for (size_t i = 0; i < iNum; i++)
		{
			osg::Vec4 vert = m_pCubeVertArray->at(i);
			vertArray->push_back(vert*fCubeSize);
		}

		osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
		pGeometry->setNodeMask(0);
		pGeometry->setVertexArray(vertArray.get());
		pGeometry->setColorArray(m_pGalaxiesInfo.get());
		pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

		osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
		normal->push_back(osg::Vec3(0, 1, 0));
		pGeometry->setNormalArray(normal.get());
		pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
		pGeometry->addPrimitiveSet(m_pCubeElement.get());

		pGeometry->setUseVertexBufferObjects(true);
		pGeometry->setUseDisplayList(false);
		pGeometry->setDataVariance(osg::Object::DYNAMIC);

		m_pGalaxiesGeomVector.push_back(pGeometry);
		m_pGeodeGalaxies_5->addDrawable(pGeometry.get());
	}

	return true;
}

bool CGMGalaxy::_CreateSupercluster()
{
	const float fSuperclusterSize = 0.5f;

	m_pGeodeSupercluster = new osg::Geode;
	m_pGeodeSupercluster->setNodeMask(0);// 默认先隐藏
	m_pGeodeSupercluster->addDrawable(_MakeBoxGeometry(fSuperclusterSize, fSuperclusterSize, fSuperclusterSize));
	m_pHierarchyRootVector.at(5)->addChild(m_pGeodeSupercluster.get());

	osg::ref_ptr<osg::StateSet> pSS = m_pGeodeSupercluster->getOrCreateStateSet();
	pSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSS->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSS->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pSS->setAttributeAndModes(new osg::CullFace());
	pSS->setRenderBinDetails(BIN_SUPERCLUSTER, "DepthSortedBin");

	osg::Vec3f vRangeMax = osg::Vec3f(0.5, 0.5, 0.5)*fSuperclusterSize;
	pSS->addUniform(new osg::Uniform("rangeMax", vRangeMax));
	osg::Vec3f vRangeMin = -vRangeMax;
	pSS->addUniform(new osg::Uniform("rangeMin", vRangeMin));

	pSS->addUniform(m_pCommonUniform->GetScreenSize());

	int iUnit = 0;
	CGMKit::AddTexture(pSS.get(), m_3DShapeTex.get(), "shapeNoiseTex", iUnit++);
	//CGMKit::AddTexture(pSS.get(), m_blueNoiseTex.get(), "blueNoiseTex", iUnit++);

	// 添加shader
	std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SuperclusterVert.glsl";
	std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SuperclusterFrag.glsl";
	return CGMKit::LoadShader(pSS.get(), strVertPath, strFragPath, "Supercluster");
}

bool CGMGalaxy::_CreateUltracluster()
{
	const float fUltraclusterSize = 0.5f / 0.11f;

	m_pGeodeUltracluster = new osg::Geode;
	m_pGeodeUltracluster->setNodeMask(0);// 默认先隐藏
	m_pGeodeUltracluster->addDrawable(_MakeBoxGeometry(fUltraclusterSize, fUltraclusterSize, fUltraclusterSize));
	m_pHierarchyRootVector.at(5)->addChild(m_pGeodeUltracluster.get());

	osg::ref_ptr<osg::StateSet> pSS = m_pGeodeUltracluster->getOrCreateStateSet();
	pSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSS->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSS->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pSS->setAttributeAndModes(new osg::CullFace());
	pSS->setRenderBinDetails(BIN_ULTRACLUSTER, "DepthSortedBin");
	pSS->setDefine("ULTRA", osg::StateAttribute::ON);

	osg::Vec3f vRangeMax = osg::Vec3f(0.5, 0.5, 0.5)*fUltraclusterSize;
	pSS->addUniform(new osg::Uniform("rangeMax", vRangeMax));
	osg::Vec3f vRangeMin = -vRangeMax;
	pSS->addUniform(new osg::Uniform("rangeMin", vRangeMin));

	pSS->addUniform(m_pCommonUniform->GetScreenSize());

	int iUnit = 0;
	CGMKit::AddTexture(pSS.get(), m_3DShapeTex.get(), "shapeNoiseTex", iUnit++);
	//CGMKit::AddTexture(pSS.get(), m_blueNoiseTex.get(), "blueNoiseTex", iUnit++);

	// 添加shader
	std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SuperclusterVert.glsl";
	std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SuperclusterFrag.glsl";
	return CGMKit::LoadShader(pSS.get(), strVertPath, strFragPath, "Supercluster");
}

bool CGMGalaxy::_CreateMyWorld()
{
	m_pMyWorld_5_AutoTrans = new osg::AutoTransform();
	m_pMyWorld_5_AutoTrans->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_CAMERA);
	m_pHierarchyRootVector.at(5)->addChild(m_pMyWorld_5_AutoTrans.get());
	m_pGeodeMyWorld_5 = new osg::Geode();
	m_pMyWorld_5_AutoTrans->addChild(m_pGeodeMyWorld_5.get());

	m_pMyWorld_6_AutoTrans = new osg::AutoTransform();
	m_pMyWorld_6_AutoTrans->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_CAMERA);
	m_pHierarchyRootVector.at(6)->addChild(m_pMyWorld_6_AutoTrans.get());
	m_pGeodeMyWorld_6 = new osg::Geode();
	m_pMyWorld_6_AutoTrans->addChild(m_pGeodeMyWorld_6.get());

	// 创建我的世界的点状几何体
	osg::ref_ptr<osg::Geometry> pGeometry_5 = new osg::Geometry();
	osg::ref_ptr<osg::Geometry> pGeometry_6 = new osg::Geometry();

	// 计算世界尺寸和相机参数
	float fWorldSize = 200.0f;
	float fFovy = osg::inDegrees(m_pConfigData->fFovy);
	float fTan = std::tan(fFovy*0.5f);
	// 相机在终极时刻的目标半径（5级空间下）
	float fCameraFinalRadius_5 = fWorldSize * 0.5f / fTan - fWorldSize * 0.5f;

	size_t iNum = 65536;
	osg::ref_ptr<osg::Vec3Array> vertArray_5 = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> vertArray_6 = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> texcoordArray = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array();
	osg::ref_ptr<osg::DrawElementsUInt> el = new osg::DrawElementsUInt(GL_POINTS);
	vertArray_5->reserve(iNum);
	vertArray_6->reserve(iNum);
	texcoordArray->reserve(iNum);
	colorArray->reserve(iNum);
	el->reserve(iNum);

	if (!m_pPhotoImage.valid())
	{
		std::string strPath = m_pConfigData->strCorePath + m_strGalaxyTexPath + "photo.tga";
		m_pPhotoImage = osgDB::readImageFile(strPath);
	}

	std::uniform_int_distribution<> iPseudoNoise(0, 10000);
	int x = 0;
	while (x < iNum)
	{
		float fRandomX = iPseudoNoise(m_iRandom)*1e-4f - 0.5f;
		float fRandomY = iPseudoNoise(m_iRandom)*1e-4f - 0.5f;
		float fRandomZ = iPseudoNoise(m_iRandom)*1e-4f - 0.5f;
		float fRandomAlpha = iPseudoNoise(m_iRandom)*1e-4f;
		float fX = fWorldSize * fRandomX;
		float fY = fWorldSize * fRandomY;
		float fZ = fWorldSize * fRandomZ;

		osg::Vec3 vViewDir = osg::Vec3(fX, fY, fZ - fCameraFinalRadius_5);
		// 相机中轴线方向距离相机1个单位的平面上的向量
		osg::Vec3 vNear = vViewDir / abs(vViewDir.z());
		float fProjU = 0.5f * vNear.x() / fTan + 0.5f;
		float fProjV = 0.5f * vNear.y() / fTan + 0.5f;
		// 如果顶点在画面内
		if (fProjU <= 1.0f && fProjU >= 0.0f && fProjV <= 1.0f && fProjV >= 0.0f)
		{
			float fU = fRandomX + 0.5f;
			float fV = fRandomY + 0.5f;
			float fW = fRandomZ + 0.5f;
			float fHole = _Get3DValue(
				std::fmod(4 * fU, 1.0f),
				std::fmod(4 * fV, 1.0f),
				std::fmod(4 * fW, 1.0f));
			if (fHole < 0.45)
			{
				osg::Vec4f vColor = CGMKit::GetImageColor(m_pPhotoImage.get(), fProjU, fProjV);
				if (vColor.a() > 0.0f)
				{
					vertArray_5->push_back(osg::Vec3(fX, fY, fZ));
					texcoordArray->push_back(osg::Vec3(fU, fV, fW));
					colorArray->push_back(osg::Vec4(vColor.r(), vColor.g(), vColor.b(), fRandomAlpha*(1 - fHole)));
					el->push_back(x);
					x++;
				}
			}
		}
	}

	int y = 0;
	while (y < iNum)
	{
		vertArray_6->push_back(vertArray_5->at(y) / GM_UNIT_SCALE);
		y++;
	}

	// 5
	pGeometry_5->setVertexArray(vertArray_5.get());
	pGeometry_5->setTexCoordArray(0, texcoordArray.get());
	pGeometry_5->setColorArray(colorArray.get());
	pGeometry_5->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 1, 0));
	pGeometry_5->setNormalArray(normal.get());
	pGeometry_5->setNormalBinding(osg::Geometry::BIND_OVERALL);
	pGeometry_5->addPrimitiveSet(el.get());

	pGeometry_5->setUseVertexBufferObjects(true);
	pGeometry_5->setUseDisplayList(false);
	pGeometry_5->setDataVariance(osg::Object::DYNAMIC);

	m_pGeodeMyWorld_5->addDrawable(pGeometry_5.get());

	// 6
	pGeometry_6->setVertexArray(vertArray_6.get());
	pGeometry_6->setTexCoordArray(0, texcoordArray.get());
	pGeometry_6->setColorArray(colorArray.get());
	pGeometry_6->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	pGeometry_6->setNormalArray(normal.get());
	pGeometry_6->setNormalBinding(osg::Geometry::BIND_OVERALL);
	pGeometry_6->addPrimitiveSet(el.get());

	pGeometry_6->setUseVertexBufferObjects(true);
	pGeometry_6->setUseDisplayList(false);
	pGeometry_6->setDataVariance(osg::Object::DYNAMIC);

	m_pGeodeMyWorld_6->addDrawable(pGeometry_6.get());

	// 状态集
	osg::ref_ptr<osg::StateSet> pSS_5 = m_pGeodeMyWorld_5->getOrCreateStateSet();
	osg::ref_ptr<osg::StateSet> pSS_6 = m_pGeodeMyWorld_6->getOrCreateStateSet();

	pSS_5->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
	pSS_5->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	pSS_5->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSS_5->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSS_5->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pSS_5->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pSS_5->setRenderBinDetails(BIN_GALAXIES, "DepthSortedBin");
	pSS_5->addUniform(m_fMyWorldAlphaUniform.get());
	int iUnit_5 = 0;
	//CGMKit::AddTexture(pSS_5.get(), m_2DNoiseTex.get(), "noise2DTex", iUnit_5++);

	pSS_6->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
	pSS_6->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	pSS_6->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSS_6->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSS_6->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pSS_6->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pSS_6->setRenderBinDetails(BIN_GALAXIES, "DepthSortedBin");
	pSS_6->addUniform(m_fMyWorldAlphaUniform.get());
	int iUnit_6 = 0;
	//CGMKit::AddTexture(pSS_6.get(), m_2DNoiseTex.get(), "noise2DTex", iUnit_6++);

	// 添加shader
	std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "MyWorldVert.glsl";
	std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "MyWorldFrag.glsl";
	CGMKit::LoadShader(pSS_5.get(), strVertPath, strFragPath, "MyWorld5");
	CGMKit::LoadShader(pSS_6.get(), strVertPath, strFragPath, "MyWorld6");

	return true;
}

bool CGMGalaxy::_CreateBackgroundGalaxy()
{
	osg::ref_ptr<osg::Geode> pGalaxyBackgroundGeode = new osg::Geode;
	osg::ref_ptr<const osg::EllipsoidModel>	_ellipsoidModel = new osg::EllipsoidModel(1.0, 1.0);
	osg::ref_ptr<osg::Geometry> pDrawable = _MakeEllipsoidGeometry(_ellipsoidModel, 32, 8, 0, true, true, true, -45.0f, 45.0f);
	pGalaxyBackgroundGeode->addDrawable(pDrawable.get());

	m_pBackgroundGalaxyTransform = new osg::PositionAttitudeTransform();
	m_pBackgroundGalaxyTransform->setNodeMask(0);
	m_pBackgroundGalaxyTransform->addChild(pGalaxyBackgroundGeode.get());
	m_pCosmosBoxNode->addChild(m_pBackgroundGalaxyTransform.get());

	osg::ref_ptr<osg::StateSet>	pSSGalaxyBackground = m_pBackgroundGalaxyTransform->getOrCreateStateSet();
	pSSGalaxyBackground->setRenderBinDetails(BIN_BACKGROUND_GALAXY, "RenderBin");

	// 银河系背景纹理
	std::string strTexPath = m_pConfigData->strCorePath + m_strGalaxyTexPath;
	osg::ref_ptr<osg::Texture2D> pGalaxyBackgroundTex = new osg::Texture2D();
	pGalaxyBackgroundTex->setImage(osgDB::readImageFile(strTexPath + "galaxyBackground.dds", m_pDDSOptions.get()));
	pGalaxyBackgroundTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	pGalaxyBackgroundTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	pGalaxyBackgroundTex->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	pGalaxyBackgroundTex->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	pGalaxyBackgroundTex->setInternalFormat(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
	pGalaxyBackgroundTex->setSourceFormat(GL_RGBA);
	pGalaxyBackgroundTex->setSourceType(GL_UNSIGNED_BYTE);

	int iUnit = 0;
	CGMKit::AddTexture(pSSGalaxyBackground.get(), pGalaxyBackgroundTex.get(), "galaxyBackgroundTex", iUnit++);
	CGMKit::AddTexture(pSSGalaxyBackground.get(), _CreateTexture2D(strTexPath + "starNoise.tga", 4), "noiseTex", iUnit++);

	pSSGalaxyBackground->addUniform(m_fStarAlphaUniform.get());
	pSSGalaxyBackground->addUniform(m_pEyePos4Uniform.get());

	// 添加shader
	std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyBackgroundVert.glsl";
	std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyBackgroundFrag.glsl";
	return CGMKit::LoadShader(pSSGalaxyBackground.get(), strVertPath, strFragPath, "GalaxyBackground");
}

bool CGMGalaxy::_CreateCosmosBox()
{
	m_pCosmosBoxGeode = new osg::Geode();
	m_pCosmosBoxGeode->addDrawable(_MakeBoxGeometry(2, 2, 2));
	m_pCosmosBoxNode->addChild(m_pCosmosBoxGeode.get());

	std::string strCubeMapPath = m_pConfigData->strCorePath + m_strGalaxyTexPath + "Skybox/";
	std::string strCubeMapPrefix = "Skybox";
	osg::ref_ptr<osg::Texture> pCubeMapTex = _ReadCubeMap(strCubeMapPath, strCubeMapPrefix);

	osg::ref_ptr<osg::StateSet> pStateSetCosmosBox = m_pCosmosBoxGeode->getOrCreateStateSet();
	pStateSetCosmosBox->setRenderBinDetails(BIN_COSMOS, "RenderBin");

	int iUnit = 0;
	CGMKit::AddTexture(pStateSetCosmosBox.get(), pCubeMapTex.get(), "cubeMapTex", iUnit++);
	CGMKit::AddTexture(pStateSetCosmosBox.get(), m_3DShapeTex.get(), "shapeNoiseTex", iUnit++);

	pStateSetCosmosBox->addUniform(m_fStarAlphaUniform.get());
	pStateSetCosmosBox->addUniform(m_pCommonUniform->GetTime());
	pStateSetCosmosBox->addUniform(m_fMyWorldAlphaUniform.get());

	// 添加shader
	std::string strCosmosBoxVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "CosmosBoxVert.glsl";
	std::string strCosmosBoxFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "CosmosBoxFrag.glsl";
	return CGMKit::LoadShader(pStateSetCosmosBox.get(), strCosmosBoxVertPath, strCosmosBoxFragPath, "CosmosBox");
}

bool CGMGalaxy::_DetachAudioPoints()
{
	m_vPlayingAudioCoord = m_pDataManager->GetAudioCoord(m_strPlayingStarName);
	m_iPlayingAudioUID = m_pDataManager->GetUID(m_strPlayingStarName);

	// 音频数据
	std::map<unsigned int, SGMAudioData> audioDataMap;
	m_pDataManager->GetAudioDataMap(audioDataMap);

	osg::Geometry* pGeometry = dynamic_cast<osg::Geometry*>(m_pGeodeAudio->getDrawable(0));
	if (pGeometry)
	{
		m_pGeodeAudio->removeChild(pGeometry);
	}
	// 从修改过的数据管理模块读取数据，重新创建未激活状态的音频星几何体
	osg::ref_ptr<osg::Geometry> pGeomAudio = _CreateAudioGeometry(audioDataMap, m_iPlayingAudioUID);
	if (!pGeomAudio.valid()) return false;
	m_pGeodeAudio->addDrawable(pGeomAudio.get());

	// 创建激活的音频星
	if (!m_pStarInfoTransform.valid())
	{
		m_pStarInfoTransform = new osg::PositionAttitudeTransform();
		osg::Geode* pGeode = new osg::Geode();
		osg::Geometry* pGeom = _CreateConeGeometry();
		pGeode->addDrawable(pGeom);
		m_pStarInfoTransform->addChild(pGeode);
		m_pHierarchyRootVector.at(4)->addChild(m_pStarInfoTransform.get());

		osg::ref_ptr<osg::StateSet> pSS = m_pStarInfoTransform->getOrCreateStateSet();

		pSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pSS->setMode(GL_BLEND, osg::StateAttribute::ON);
		pSS->setAttributeAndModes(new osg::BlendFunc(
			GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_ALPHA, GL_ONE
		), osg::StateAttribute::ON);
		pSS->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
		pSS->setRenderBinDetails(BIN_STAR_PLAYING, "DepthSortedBin");

		pSS->addUniform(m_pCommonUniform->GetStarColor());

		// 添加shader
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "PlayingStarVert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "PlayingStarFrag.glsl";
		CGMKit::LoadShader(pSS.get(), strVertPath, strFragPath, "PlayingStar");
	}
	osg::Vec3f vPos;
	m_pCommonUniform->SetStarHiePos(vPos);
	m_pStarInfoTransform->asPositionAttitudeTransform()->setPosition(vPos);
	//m_pStarInfoTransform->setNodeMask(~0);
	m_pStarInfoTransform->setNodeMask(0);

	// 创建音频区域
	if (!m_pGeodeRegion.valid())
	{
		m_pGeodeRegion = new osg::Geode();
		osg::Geometry* pRegionGeometry = _CreateRegionGeometry();
		m_pGeodeRegion->addDrawable(pRegionGeometry);
		m_pHierarchyRootVector.at(4)->addChild(m_pGeodeRegion.get());

		osg::ref_ptr<osg::StateSet> pRegionSS = m_pGeodeRegion->getOrCreateStateSet();
		pRegionSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pRegionSS->setMode(GL_BLEND, osg::StateAttribute::ON);
		pRegionSS->setAttributeAndModes(new osg::BlendFunc(
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
		), osg::StateAttribute::ON);
		pRegionSS->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
		pRegionSS->setRenderBinDetails(BIN_REGION, "DepthSortedBin");

		pRegionSS->addUniform(m_pMousePosUniform.get());
		pRegionSS->addUniform(m_pAudioUVUniform.get());
		pRegionSS->addUniform(m_pCommonUniform->GetTime());

		// 添加shader
		std::string strAudioRegionVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "AudioRegionVert.glsl";
		std::string strAudioRegionFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "AudioRegionFrag.glsl";
		CGMKit::LoadShader(pRegionSS.get(), strAudioRegionVertPath, strAudioRegionFragPath, "AudioRegion");
	}
	m_pGeodeRegion->setNodeMask(~0);

	return true;
}

bool CGMGalaxy::_AttachAudioPoints()
{
	SGMAudioData sData;
	sData.UID = m_iPlayingAudioUID;
	sData.name = m_strPlayingStarName;
	sData.audioCoord = m_vPlayingAudioCoord;
	sData.galaxyCoord = m_pDataManager->AudioCoord2GalaxyCoord(m_vPlayingAudioCoord);
	// 设置音频数据，如果位置与已知音频有重合，则自动迁移到合适位置
	m_pDataManager->EditAudioData(sData);
	// 位置迁移后再回传给本模块的音频坐标
	m_vPlayingAudioCoord = sData.audioCoord;

	// 音频数据
	std::map<unsigned int, SGMAudioData> audioDataMap;
	m_pDataManager->GetAudioDataMap(audioDataMap);
	osg::Geometry* pGeometry = dynamic_cast<osg::Geometry*>(m_pGeodeAudio->getDrawable(0));
	if (pGeometry)
	{
		m_pGeodeAudio->removeChild(pGeometry);
	}
	// 从修改过的数据管理模块读取数据，重新创建未激活状态的音频星几何体
	osg::ref_ptr<osg::Geometry> pGeomAudio = _CreateAudioGeometry(audioDataMap);
	if (!pGeomAudio.valid()) return false;
	m_pGeodeAudio->addDrawable(pGeomAudio.get());

	// 隐藏激活的音频星
	if (m_pStarInfoTransform.valid())
	{
		m_pStarInfoTransform->setNodeMask(0);
	}
	if (m_pGeodeRegion.valid())
	{
		m_pGeodeRegion->setNodeMask(0);
	}
	return true;
}

bool CGMGalaxy::_UpdatePlayingStarInformation(const SGMAudioCoord& sAudioCoord)
{
	if (sAudioCoord.angle < 0.0 ||
		sAudioCoord.angle >= (osg::PI*2.0) ||
		sAudioCoord.BPM < m_pConfigData->fMinBPM)
	{
		return false;
	}
	if (4 == m_pKernelData->iHierarchy)
	{
		SGMGalaxyCoord vGC = m_pDataManager->AudioCoord2GalaxyCoord(sAudioCoord);
		m_vPlayingStarWorld4Pos = osg::Vec3d(vGC.x, vGC.y, vGC.z) * m_fGalaxyRadius;

		osg::Vec3f vPosHierarchy = m_vPlayingStarWorld4Pos / m_pKernelData->fUnitArray->at(4);
		m_pSolarSystem->SetSupernovaHiePos(vPosHierarchy);
		m_pCommonUniform->SetStarHiePos(vPosHierarchy);
		// 设置当前音频星颜色
		m_pCommonUniform->SetStarColor(m_pDataManager->GetAudioColor(sAudioCoord));

		// 更新音频的UV
		m_pAudioUVUniform->set(_AudioCoord2UV(sAudioCoord));

		osg::Quat quat = osg::Quat(m_fArrowAngle, osg::Vec3d(0, 0, 1));
		m_pStar_4_Transform->asPositionAttitudeTransform()->setAttitude(quat);
		m_pStar_4_Transform->asPositionAttitudeTransform()->setPosition(vPosHierarchy);
	}
	return true;
}

osg::Geometry* CGMGalaxy::_CreateAudioGeometry(
	std::map<unsigned int, SGMAudioData>& audioMap,
	const unsigned int iDiscardUID)
{
	// 4级空间下的星系半径
	double fGalaxyRadius4 = m_fGalaxyRadius / m_pKernelData->fUnitArray->at(4);

	osg::Geometry* pGeomAudio = new osg::Geometry();

	size_t iNum = audioMap.size();
	if (0 != iDiscardUID) iNum--;
	if (0 >= iNum) return nullptr;

	osg::ref_ptr<osg::Vec3Array> vertArray = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array();
	osg::ref_ptr<osg::DrawElementsUShort> el = new osg::DrawElementsUShort(GL_POINTS);
	vertArray->reserve(iNum);
	colorArray->reserve(iNum);
	el->reserve(iNum);

	int x = 0;
	for (auto& itr : audioMap)
	{
		if (iDiscardUID == itr.first) continue;

		float fX = itr.second.galaxyCoord.x;
		float fY = itr.second.galaxyCoord.y;
		float fZ = itr.second.galaxyCoord.z;

		osg::Vec4f vColor = m_pDataManager->GetAudioColor(itr.first);

		fX *= fGalaxyRadius4;
		fY *= fGalaxyRadius4;
		fZ *= 0.001f * fGalaxyRadius4;

		vertArray->push_back(osg::Vec3f(fX, fY, fZ));
		colorArray->push_back(vColor);
		el->push_back(x);
		x++;
	}

	pGeomAudio->setVertexArray(vertArray.get());
	pGeomAudio->setColorArray(colorArray.get());
	pGeomAudio->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3f(0, 1, 0));
	pGeomAudio->setNormalArray(normal.get());
	pGeomAudio->setNormalBinding(osg::Geometry::BIND_OVERALL);
	pGeomAudio->addPrimitiveSet(el.get());

	pGeomAudio->setUseVertexBufferObjects(true);
	pGeomAudio->setUseDisplayList(false);
	pGeomAudio->setDataVariance(osg::Object::DYNAMIC);

	return pGeomAudio;
}

osg::Geometry* CGMGalaxy::_CreateConeGeometry()
{
	float fRadius = 1.5f;
	float fHeight = 2.0f;
	int segments = 32;
	float deltaAngle = osg::PI * 2 / (float)segments;

	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);

	osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
	verts->reserve(2 + segments);
	geom->setVertexArray(verts.get());

	osg::ref_ptr<osg::Vec3Array> textArray = new osg::Vec3Array;
	textArray->reserve(2 + segments);
	geom->setTexCoordArray(0, textArray.get());

	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->reserve(2 + segments);
	geom->setNormalArray(normal.get());
	geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

	osg::ref_ptr <osg::DrawElementsUShort> el = new osg::DrawElementsUShort(GL_TRIANGLE_FAN);
	el->reserve(2 + segments);
	geom->addPrimitiveSet(el.get());

	verts->push_back(osg::Vec3(0, 0, 0)); // center point
	verts->push_back(osg::Vec3(fRadius, 0, fHeight)); // first point
	textArray->push_back(osg::Vec3(0, 0, 0));
	textArray->push_back(osg::Vec3(1, 0, 1));
	normal->push_back(osg::Vec3(0, 0, -1));
	normal->push_back(osg::Vec3(1, 0, 0));
	el->push_back(0);
	el->push_back(1);

	for (int i = 0; i < segments; ++i)
	{
		double angle = deltaAngle * float(i + 1);
		double cosA = cos(angle);
		double sinA = sin(angle);
		double x = fRadius * cosA;
		double y = fRadius * sinA;
		verts->push_back(osg::Vec3f(x, y, fHeight));
		textArray->push_back(osg::Vec3(cosA, sinA, 1));
		normal->push_back(osg::Vec3(cosA, sinA, 0));
		el->push_back(2 + i);
	}

	return geom;
}

osg::Geometry* CGMGalaxy::_CreateSquareGeometry(const float fWidth, const bool bCorner)
{
	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
	osg::ref_ptr<osg::Vec3Array> verArray = new osg::Vec3Array;

	if (bCorner)
	{
		const float fSqrt2 = std::sqrt(2.0f);
		float fRadius = fWidth / fSqrt2;
		verArray->push_back(osg::Vec3(0, 0, 0));
		verArray->push_back(osg::Vec3(fRadius, -fRadius, 0));
		verArray->push_back(osg::Vec3(fWidth * fSqrt2, 0, 0));
		verArray->push_back(osg::Vec3(fRadius, fRadius, 0));
	}
	else
	{
		float fRadius = fWidth * 0.5f;
		verArray->push_back(osg::Vec3(-fRadius, fRadius, 0));
		verArray->push_back(osg::Vec3(-fRadius, -fRadius, 0));
		verArray->push_back(osg::Vec3(fRadius, -fRadius, 0));
		verArray->push_back(osg::Vec3(fRadius, fRadius, 0));
	}
	pGeometry->setVertexArray(verArray);

	osg::ref_ptr<osg::Vec2Array> textArray = new osg::Vec2Array;
	textArray->push_back(osg::Vec2(0, 0));
	textArray->push_back(osg::Vec2(1, 0));
	textArray->push_back(osg::Vec2(1, 1));
	textArray->push_back(osg::Vec2(0, 1));
	pGeometry->setTexCoordArray(0, textArray);

	osg::ref_ptr< osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 0, 1));
	pGeometry->setNormalArray(normal);
	pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

	pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

	pGeometry->setUseVertexBufferObjects(true);
	pGeometry->setUseDisplayList(false);
	pGeometry->setDataVariance(osg::Object::DYNAMIC);

	return pGeometry.release();
}

osg::Geometry* CGMGalaxy::_CreateRegionGeometry()
{
	// 4级空间下的星系半径
	double fGalaxyRadius4 = m_fGalaxyRadius / m_pKernelData->fUnitArray->at(4);

	// 角度分段数
	int iAngleSegments = 32;
	// 半径分段数
	int iRadiusSegments = 256;
	int iVertexNum = 2 * (iAngleSegments+1)*(iRadiusSegments+1);

	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);
	geom->setUseDisplayList(false);
	geom->setDataVariance(osg::Object::DYNAMIC);

	osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
	osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array();
	osg::ref_ptr<osg::Vec3Array> coordArray = new osg::Vec3Array;
	osg::ref_ptr<osg::DrawElementsUInt> el = new osg::DrawElementsUInt(GL_TRIANGLES);

	verts->reserve(iVertexNum);
	colorArray->reserve(iVertexNum);
	coordArray->reserve(iVertexNum);
	el->reserve(12 * iAngleSegments * iRadiusSegments);

	geom->setVertexArray(verts.get());
	geom->setColorArray(colorArray.get());
	geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	geom->setTexCoordArray(0, coordArray.get());
	geom->addPrimitiveSet(el.get());

	for (int i = 0; i < 2; i++)
	{
		for (int x = 0; x <= iAngleSegments; x++)
		{
			double fU = double(x) / double(iAngleSegments);
			// 边缘位置会出错，所以缩小 0.01%（万分之一 ）绘制
			double fAngle = (osg::PI * fU - osg::PI_4)*0.9999 + i*osg::PI;
			if (fAngle < 0)
			{
				fAngle += osg::PI * 2;
			}
			for (int y = 0; y <= iRadiusSegments; y++)
			{
				double fV = double(y) / double(iRadiusSegments);
				// 不能让BPM太大，否则在中心位置会变形，所以从最小BPM的25倍，开始绘制
				double fBPM = 1 / ((1-fV) / (m_pConfigData->fMinBPM * MAX_BPM_RATIO) + fV / m_pConfigData->fMinBPM);
				SGMAudioCoord vAudioCoord = SGMAudioCoord(fBPM, fAngle);
				SGMGalaxyCoord vGC = m_pDataManager->AudioCoord2GalaxyCoord(vAudioCoord);
				osg::Vec3d vWorldPos = osg::Vec3d(vGC.x, vGC.y, vGC.z) * fGalaxyRadius4;
				osg::Vec4f vColor = m_pDataManager->GetAudioColor(vAudioCoord);

				verts->push_back(vWorldPos);
				colorArray->push_back(vColor);
				coordArray->push_back(osg::Vec3(fU, fV, i));

				if (x < iAngleSegments && y < iRadiusSegments)
				{
					el->push_back(iVertexNum * 0.5 * i + x * (iRadiusSegments + 1) + y);
					el->push_back(iVertexNum * 0.5 * i + (x + 1) * (iRadiusSegments + 1) + y);
					el->push_back(iVertexNum * 0.5 * i + x * (iRadiusSegments + 1) + y + 1);
					el->push_back(iVertexNum * 0.5 * i + x * (iRadiusSegments + 1) + y + 1);
					el->push_back(iVertexNum * 0.5 * i + (x + 1) * (iRadiusSegments + 1) + y);
					el->push_back(iVertexNum * 0.5 * i + (x + 1) * (iRadiusSegments + 1) + y + 1);
				}
			}
		}
	}

	return geom;
}

osg::Geometry* CGMGalaxy::_MakeEllipsoidGeometry(
	const osg::EllipsoidModel * ellipsoid,
	int iLonSegments, int iLatSegments,
	float fHae, bool bGenTexCoords,
	bool bWholeMap, bool bFlipNormal,
	float fLatStart, float fLatEnd) const
{
	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);

	if (fLatEnd < fLatStart)
	{
		float tmp = fLatEnd;
		fLatEnd = fLatStart;
		fLatStart = tmp;
	}

	fLatStart = (fLatStart < -90.0) ? -90.0 : fLatStart;
	fLatEnd = (fLatEnd > 90.0) ? 90.0 : fLatEnd;

	double latSegmentSize = (fLatEnd - fLatStart) / (double)iLatSegments; // degrees
	double lonSegmentSize = 360.0 / (double)iLonSegments; // degrees

	osg::Vec3Array* verts = new osg::Vec3Array();
	verts->reserve(iLatSegments * iLonSegments);

	osg::Vec2Array* texCoords = 0;
	osg::Vec3Array* normals = 0;
	if (bGenTexCoords)
	{
		texCoords = new osg::Vec2Array();
		texCoords->reserve(iLatSegments * iLonSegments);
		geom->setTexCoordArray(0, texCoords);

		normals = new osg::Vec3Array();
		normals->reserve(iLatSegments * iLonSegments);
		geom->setNormalArray(normals);
		geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	}

	osg::DrawElementsUShort* el = new osg::DrawElementsUShort(GL_TRIANGLES);
	el->reserve(iLatSegments * iLonSegments * 6);

	for (int y = 0; y <= iLatSegments; ++y)
	{
		double lat = fLatStart + latSegmentSize * (double)y;
		for (int x = 0; x <= iLonSegments; ++x)
		{
			double lon = lonSegmentSize * (double)x - 180.0;
			double gx, gy, gz;
			ellipsoid->convertLatLongHeightToXYZ(osg::DegreesToRadians(lat), osg::DegreesToRadians(lon), fHae, gx, gy, gz);
			verts->push_back(osg::Vec3(gx, gy, gz));

			if (bGenTexCoords)
			{
				double s, t;
				if (bWholeMap)
				{
					s = 1 - (double)x / (double)iLonSegments;
					t = (double)y / (double)iLatSegments;
				}
				else
				{
					s = (lon + 180) / 360.0;
					t = (lat + 90.0) / 180.0;
				}
				texCoords->push_back(osg::Vec2(s, t));
			}

			if (normals)
			{
				osg::Vec3 normal(gx, gy, gz);
				if (bFlipNormal)
				{
					normal *= -1;
				}
				normal.normalize();
				normals->push_back(normal);
			}

			if ((y < iLatSegments) && (x < iLonSegments))
			{
				//int x_plus_1 = x < iLonSegments - 1 ? x + 1 : 0;
				int x_plus_1 = x + 1;
				int y_plus_1 = y + 1;
				if (bFlipNormal)
				{
					el->push_back(y*(iLonSegments + 1) + x);
					el->push_back(y_plus_1*(iLonSegments + 1) + x);
					el->push_back(y*(iLonSegments + 1) + x_plus_1);
					el->push_back(y*(iLonSegments + 1) + x_plus_1);
					el->push_back(y_plus_1*(iLonSegments + 1) + x);
					el->push_back(y_plus_1*(iLonSegments + 1) + x_plus_1);
				}
				else
				{
					el->push_back(y*(iLonSegments + 1) + x);
					el->push_back(y*(iLonSegments + 1) + x_plus_1);
					el->push_back(y_plus_1*(iLonSegments + 1) + x);
					el->push_back(y*(iLonSegments + 1) + x_plus_1);
					el->push_back(y_plus_1*(iLonSegments + 1) + x_plus_1);
					el->push_back(y_plus_1*(iLonSegments + 1) + x);
				}
			}
		}
	}

	geom->setVertexArray(verts);
	geom->addPrimitiveSet(el);

	return geom;
}

osg::Geometry* CGMGalaxy::_MakeBoxGeometry(
	const float fLength,
	const float fWidth,
	const float fHeight) const
{
	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);

	osg::Vec3Array* verts = new osg::Vec3Array();
	verts->reserve(8);

	osg::Vec3Array* texCoords = new osg::Vec3Array();
	texCoords->reserve(8);

	osg::DrawElementsUShort* el = new osg::DrawElementsUShort(GL_TRIANGLES);
	el->reserve(36);

	float fHalfLength = 0.5 * fLength;
	float fHalfWidth = 0.5 * fWidth;
	float fHalfHeight = 0.5 * fHeight;

	verts->push_back(osg::Vec3(-fHalfLength, -fHalfWidth, -fHalfHeight)); // 0
	verts->push_back(osg::Vec3(fHalfLength, -fHalfWidth, -fHalfHeight)); // 1
	verts->push_back(osg::Vec3(fHalfLength, fHalfWidth, -fHalfHeight)); // 2
	verts->push_back(osg::Vec3(-fHalfLength, fHalfWidth, -fHalfHeight)); // 3
	verts->push_back(osg::Vec3(-fHalfLength, -fHalfWidth, fHalfHeight)); // 4
	verts->push_back(osg::Vec3(fHalfLength, -fHalfWidth, fHalfHeight)); // 5
	verts->push_back(osg::Vec3(fHalfLength, fHalfWidth, fHalfHeight)); // 6
	verts->push_back(osg::Vec3(-fHalfLength, fHalfWidth, fHalfHeight)); // 7


	texCoords->push_back(osg::Vec3(0, 0, 0)); // 0
	texCoords->push_back(osg::Vec3(1, 0, 0)); // 1
	texCoords->push_back(osg::Vec3(1, 1, 0)); // 2
	texCoords->push_back(osg::Vec3(0, 1, 0)); // 3
	texCoords->push_back(osg::Vec3(0, 0, 1)); // 4
	texCoords->push_back(osg::Vec3(1, 0, 1)); // 5
	texCoords->push_back(osg::Vec3(1, 1, 1)); // 6
	texCoords->push_back(osg::Vec3(0, 1, 1)); // 7

	// bottom z = -1
	el->push_back(0);
	el->push_back(1);
	el->push_back(2);
	el->push_back(0);
	el->push_back(2);
	el->push_back(3);
	// front x = 1
	el->push_back(1);
	el->push_back(6);
	el->push_back(2);
	el->push_back(1);
	el->push_back(5);
	el->push_back(6);
	// top z = 1
	el->push_back(4);
	el->push_back(6);
	el->push_back(5);
	el->push_back(4);
	el->push_back(7);
	el->push_back(6);
	// back x = -1
	el->push_back(0);
	el->push_back(3);
	el->push_back(7);
	el->push_back(0);
	el->push_back(7);
	el->push_back(4);
	// left y = 1
	el->push_back(2);
	el->push_back(6);
	el->push_back(7);
	el->push_back(2);
	el->push_back(7);
	el->push_back(3);
	// right y = -1
	el->push_back(0);
	el->push_back(4);
	el->push_back(5);
	el->push_back(0);
	el->push_back(5);
	el->push_back(1);

	geom->setVertexArray(verts);
	geom->setTexCoordArray(0, texCoords);
	geom->addPrimitiveSet(el);

	return geom;
}

float CGMGalaxy::_Get3DValue(float fX, float fY, float fZ)
{
	if (!m_shapeImg.valid())
	{
		std::string strTexturePath = m_pConfigData->strCorePath + m_strGalaxyTexPath + "noiseShape128.tga";
		m_shapeImg = osgDB::readImageFile(strTexturePath);
	}
	if (m_shapeImg.valid())
	{
		unsigned int iWidth = 128;
		unsigned int iHeight = 128;
		unsigned int iDepth = 128;

		float fS = std::fmodf(fX, 1.0f) * iWidth;
		float fT = std::fmodf(fY, 1.0f) * iHeight;
		float fR = std::fmodf(fZ, 1.0f) * iDepth;

		float fDeltaS = fS - (int)fS;
		float fDeltaT = fT - (int)fT;
		float fDeltaR = fR - (int)fR;
		unsigned int s = (unsigned int)fS;
		unsigned int t = (unsigned int)fT;
		unsigned int r = (unsigned int)fR;
		unsigned int s_next = (s == iWidth - 1) ? 0 : (s + 1);
		unsigned int t_next = (t == iHeight - 1) ? 0 : (t + 1);
		unsigned int r_next = (r == iDepth - 1) ? 0 : (r + 1);

		float fValue_000 = _Get3DValue(s, t, r);
		float fValue_100 = _Get3DValue(s_next, t, r);
		float fValue_010 = _Get3DValue(s, t_next, r);
		float fValue_110 = _Get3DValue(s_next, t_next, r);
		float fValue_001 = _Get3DValue(s, t, r_next);
		float fValue_101 = _Get3DValue(s_next, t, r_next);
		float fValue_011 = _Get3DValue(s, t_next, r_next);
		float fValue_111 = _Get3DValue(s_next, t_next, r_next);

		float fValue =
			(fValue_000 * (1 - fDeltaS) * (1 - fDeltaT)
				+ fValue_100 * fDeltaS * (1 - fDeltaT)
				+ fValue_010 * (1 - fDeltaS) * fDeltaT
				+ fValue_110 * fDeltaS * fDeltaT) * (1 - fDeltaR)
			+ (fValue_001 * (1 - fDeltaS) * (1 - fDeltaT)
				+ fValue_101 * fDeltaS * (1 - fDeltaT)
				+ fValue_011 * (1 - fDeltaS) * fDeltaT
				+ fValue_111 * fDeltaS * fDeltaT) * fDeltaR;
		return fValue;
	}
	return 0.0f;
}

float CGMGalaxy::_Get3DValue(unsigned int iX, unsigned int iY, unsigned int iZ)
{
	if (m_shapeImg.valid())
	{
		unsigned int iChannel = iZ / 32;
		unsigned int iSubLayer = iZ % 32;
		switch (iChannel)
		{
		case 0:
		{
			return m_shapeImg->getColor(iX, iY + iSubLayer * 128).r();
		}
		break;
		case 1:
		{
			return m_shapeImg->getColor(iX, iY + iSubLayer * 128).g();
		}
		break;
		case 2:
		{
			return m_shapeImg->getColor(iX, iY + iSubLayer * 128).b();
		}
		break;
		case 3:
		{
			return m_shapeImg->getColor(iX, iY + iSubLayer * 128).a();
		}
		break;
		default:
			break;
		}
	}
	return 0.0f;
}

osg::Vec2f CGMGalaxy::_AudioCoord2UV(const SGMAudioCoord & sAudioCoord) const
{
	float fU = fmod((sAudioCoord.angle + osg::PI_4) / osg::PI, 2.0);
	float fV = 0.0f;
	if (0.0 != sAudioCoord.BPM)
	{
		fV = (m_pConfigData->fMinBPM * MAX_BPM_RATIO / sAudioCoord.BPM - 1) / (MAX_BPM_RATIO - 1);
	}
	return osg::Vec2f(fU, fV);
}

osg::Texture* CGMGalaxy::_CreateTexture2D(const std::string & fileName, const int iChannelNum) const
{
	return _CreateTexture2D(osgDB::readImageFile(fileName), iChannelNum);
}

osg::Texture* CGMGalaxy::_CreateTexture2D(osg::Image * pImg, const int iChannelNum) const
{
	if (!pImg) return nullptr;

	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	texture->setImage(pImg);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	switch (iChannelNum)
	{
	case 1:
	{
		texture->setInternalFormat(GL_R8);
		texture->setSourceFormat(GL_RED);
	}
	break;
	case 2:
	{
		texture->setInternalFormat(GL_RG8);
		texture->setSourceFormat(GL_RG);
	}
	break;
	case 3:
	{
		texture->setInternalFormat(GL_RGB8);
		texture->setSourceFormat(GL_RGB);
	}
	break;
	case 4:
	{
		texture->setInternalFormat(GL_RGBA8);
		texture->setSourceFormat(GL_RGBA);
	}
	break;
	default:
	{
		texture->setInternalFormat(GL_RGBA8);
		texture->setSourceFormat(GL_RGBA);
	}
	}

	texture->setSourceType(GL_UNSIGNED_BYTE);
	return texture.release();
}

osg::Texture* CGMGalaxy::_CreateDDSTexture(const std::string& fileName,
	osg::Texture::WrapMode eWrap_S, osg::Texture::WrapMode eWrap_T, bool bFlip) const
{
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	if (bFlip)
	{
		texture->setImage(osgDB::readImageFile(fileName, m_pDDSOptions.get()));
	}
	else
	{
		texture->setImage(osgDB::readImageFile(fileName));
	}
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, eWrap_S);
	texture->setWrap(osg::Texture::WRAP_T, eWrap_T);
	texture->setBorderColor(osg::Vec4(0,0,0,0));
	texture->setSourceType(GL_UNSIGNED_BYTE);
	return texture.release();
}

osg::Texture* CGMGalaxy::_ReadCubeMap(const std::string& strFolder, const std::string& strFilePrefix) const
{
	osg::TextureCubeMap* cubemap = new osg::TextureCubeMap;

	std::string strFile = strFolder + strFilePrefix;
	osg::Image* imagePosX = osgDB::readImageFile(strFile + "X+.jpg");
	osg::Image* imageNegX = osgDB::readImageFile(strFile + "X-.jpg");
	osg::Image* imagePosY = osgDB::readImageFile(strFile + "Y+.jpg");
	osg::Image* imageNegY = osgDB::readImageFile(strFile + "Y-.jpg");
	osg::Image* imagePosZ = osgDB::readImageFile(strFile + "Z+.jpg");
	osg::Image* imageNegZ = osgDB::readImageFile(strFile + "Z-.jpg");

	if (imagePosX && imageNegX && imagePosY && imageNegY && imagePosZ && imageNegZ)
	{
		cubemap->setImage(osg::TextureCubeMap::POSITIVE_X, imagePosX);
		cubemap->setImage(osg::TextureCubeMap::NEGATIVE_X, imageNegX);
		cubemap->setImage(osg::TextureCubeMap::POSITIVE_Y, imagePosY);
		cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Y, imageNegY);
		cubemap->setImage(osg::TextureCubeMap::POSITIVE_Z, imagePosZ);
		cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Z, imageNegZ);

		cubemap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		cubemap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
		cubemap->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);

		cubemap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		cubemap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

		cubemap->setInternalFormat(GL_RGB8);
		cubemap->setSourceFormat(GL_RGB);
		cubemap->setSourceType(GL_UNSIGNED_BYTE);
	}

	return cubemap;
}

osg::Vec3f CGMGalaxy::_GetRandomStarColor()
{
	std::uniform_int_distribution<> iPseudoNoise(0, 10000);

	// 0.0-4.0
	float fCenter = iPseudoNoise(m_iRandom)*4e-4f;
	// 0.5-1.0，用来随机色相
	float fScale = iPseudoNoise(m_iRandom)*5e-5f + 0.5f;

	float fR = osg::clampBetween(fScale * (2.0f - abs(1.0f - fCenter)), 0.0f, 1.0f);
	float fG = osg::clampBetween(fScale * (2.0f - abs(2.0f - fCenter)), 0.0f, 1.0f);
	float fB = osg::clampBetween(fScale * (2.0f - abs(3.0f - fCenter)), 0.0f, 1.0f);
	return osg::Vec3f(fR, fG, fB);
}

double CGMGalaxy::_IncludedAngle(const double fA, const double fB) const
{
	double fC = fA - fB;
	if (fC > osg::PI)
	{
		return fC - osg::PI * 2;
	}
	else if (fC <= -osg::PI)
	{
		return fC + osg::PI * 2;
	}
	return fC;
}

void CGMGalaxy::_MakePseudoNoise()
{
	int iW = 256;
	int iH = 256;
	int iSize = iW * iH * 4;
	unsigned char* data = new unsigned char[iSize + 1];
	data[iSize] = 0;

	std::uniform_int_distribution<> iPseudoNoise(0,255);

	for (int s = 0; s < iW; s++)
	{
		for (int t = 0; t < iH; t++)
		{
			unsigned int iAddress = 4 * (s*iH + t);

			float fX = iPseudoNoise(m_iRandom);
			float fY = iPseudoNoise(m_iRandom);
			float fZ = iPseudoNoise(m_iRandom);
			float fIntensity = iPseudoNoise(m_iRandom);

			data[iAddress] = (unsigned char)(fX);
			data[iAddress+1] = (unsigned char)(fY);
			data[iAddress+2] = (unsigned char)(fZ);
			data[iAddress+3] = (unsigned char)(fIntensity);
		}
	}

	osg::ref_ptr<osg::Image> pPseudoNoiseImage = new osg::Image();
	pPseudoNoiseImage->setImage(iW, iH, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, data, osg::Image::USE_NEW_DELETE);
	std::string strGalaxyTexPath = m_pConfigData->strCorePath + m_strGalaxyTexPath;
	//osgDB::writeImageFile(*(pPseudoNoiseImage.get()), strGalaxyTexPath + "PseudoNoise.tga");
}

osg::Texture* CGMGalaxy::_Load3DShapeNoise() const
{
	std::string strTexturePath = m_pConfigData->strCorePath + "Textures/Volume/noiseShape.raw";
	osg::ref_ptr<osg::Image> shapeImg = osgDB::readImageFile(strTexturePath);
	shapeImg->setImage(128, 128, 128, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, shapeImg->data(), osg::Image::NO_DELETE);
	osg::Texture3D* tex3d = new osg::Texture3D;
	tex3d->setImage(shapeImg.get());
	tex3d->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	tex3d->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	tex3d->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	tex3d->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	tex3d->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
	tex3d->setInternalFormat(GL_RGBA8);
	tex3d->setSourceFormat(GL_RGBA);
	tex3d->setSourceType(GL_UNSIGNED_BYTE);
	tex3d->allocateMipmapLevels();
	return tex3d;
}