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
#include "GMEngine.h"
#include "GMDataManager.h"
#include "GMKit.h"
#include <osg/PointSprite>
#include <osgDB/ReadFile>
#include <osg/LineWidth>
#include <osg/BlendFunc>
#include <osg/BlendEquation>
#include <osg/Texture2D>
#include <osg/Depth>
#include <osg/PositionAttitudeTransform>

using namespace GM;

/*************************************************************************
Macro Defines
*************************************************************************/

#define PULSE_NUM	128 	// max number of audio pulse

/*************************************************************************
CGMGalaxy Methods
*************************************************************************/

/** @brief 构造 */
CGMGalaxy::CGMGalaxy():CGMNebula(), m_fGalaxyRadius(5.0f),
	m_strGalaxyShaderPath("Shaders/GalaxyShader/"), m_strGalaxyTexPath("Textures/Galaxy/"),
	m_strPlayingStarName(L""), m_vPlayingAudioCoord(SGMAudioCoord(0.5,0.0)),
	m_vPlayingStarWorldPos(osg::Vec3f(0.0f, 0.0f, 0.0f)),
	m_pMousePosUniform(new osg::Uniform("mouseWorldPos", osg::Vec3f(0.0f, 0.0f, 0.0f))),
	m_pStarWorldPosUniform(new osg::Uniform("starWorldPos", osg::Vec3f(0.0f, 0.0f, 0.0f))),
	m_pStarAudioPosUniform(new osg::Uniform("starAudioPos", osg::Vec2f(0.5f, 0.0f))),
	m_pTimesUniform(new osg::Uniform("times", 0.0f)),
	m_pStarColorUniform(new osg::Uniform("playingStarColor", osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f))),
	m_pLevelArrayUniform(new osg::Uniform(osg::Uniform::Type::FLOAT, "level", PULSE_NUM)),
	m_bCapture(false), m_bWelcome(false), m_fWelcomeTime(0.0)
{
	for (int i = 0; i < PULSE_NUM; i++)
	{
		m_pLevelArrayUniform->setElement(i, 0.0f);
	}
}

/** @brief 析构 */
CGMGalaxy::~CGMGalaxy()
{
}

/** @brief 初始化 */
bool CGMGalaxy::Init(SGMKernelData * pKernelData, SGMConfigData * pConfigData)
{
	CGMNebula::Init(pKernelData, pConfigData);

	return true;
}

/** @brief 更新 */
bool CGMGalaxy::Update(double dDeltaTime)
{
	double dTime = osg::Timer::instance()->time_s();
	float fTimes = std::fmod((float)dTime, 1000.0f);
	m_pTimesUniform->set(fTimes);

	if (m_bWelcome)
	{
		if (GM_WELCOME_MAX_TIME > m_fWelcomeTime)
		{
			m_fWelcomeTime += dDeltaTime;


		}
		else
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
			m_pStarAudioPosUniform->set(osg::Vec2f(m_vPlayingAudioCoord.radius, m_vPlayingAudioCoord.angle));
		}
	}

	CGMNebula::Update(dDeltaTime);
	return true;
}

/** @brief 更新(在主相机更新姿态之后) */
bool CGMGalaxy::UpdateLater(double dDeltaTime)
{
	if (m_pStateSetGalaxy.valid())
	{
		unsigned int iShakeCount = GetShakeCount();
		if (iShakeCount % 2)
		{
			m_pStateSetGalaxy->setTextureAttributeAndModes(1, m_distanceMap_0.get());
		}
		else
		{
			m_pStateSetGalaxy->setTextureAttributeAndModes(1, m_distanceMap_1.get());
		}
	}

	CGMNebula::UpdateLater(dDeltaTime);
	return true;
}


/** @brief 加载 */
bool CGMGalaxy::Load()
{
	if (m_pStateSetGalaxy.valid())
	{
		std::string strStarVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarVert.glsl";
		std::string strStarFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarFrag.glsl";
		CGMKit::LoadShader(m_pStateSetGalaxy.get(), strStarVertPath, strStarFragPath);
	}
	if (m_pGeodeAudio.valid())
	{
		std::string strAudioVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "AudioVert.glsl";
		std::string strAudioFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "AudioFrag.glsl";
		CGMKit::LoadShader(m_pGeodeAudio->getOrCreateStateSet(), strAudioVertPath, strAudioFragPath);
	}
	if (m_pStateSetPlane.valid())
	{
		std::string strGalaxyPlaneVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyPlaneVert.glsl";
		std::string strGalaxyPlaneFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyPlaneFrag.glsl";
		CGMKit::LoadShader(m_pStateSetPlane.get(), strGalaxyPlaneVertPath, strGalaxyPlaneFragPath);
	}
	if(m_pPlayingStarTransform.valid())
	{
		osg::ref_ptr<osg::StateSet> pSS = m_pPlayingStarTransform->getOrCreateStateSet();
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "PlayingStarVert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "PlayingStarFrag.glsl";
		CGMKit::LoadShader(pSS.get(), strVertPath, strFragPath);
	}
	if (m_pGeodeHelpLine.valid())
	{
		osg::ref_ptr<osg::StateSet> pLineSS = m_pGeodeHelpLine->getOrCreateStateSet();
		std::string strLineVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "HelpLineVert.glsl";
		std::string strLineFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "HelpLineFrag.glsl";
		CGMKit::LoadShader(pLineSS.get(), strLineVertPath, strLineFragPath);
	}

	return CGMNebula::Load();
}

void CGMGalaxy::SetEditMode(const bool bEnable)
{
	m_pStateSetPlane->setDefine("EDIT", bEnable ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
	if(bEnable)
	{
		_DetachAudioPoints();
	}
	else
	{
		_AttachAudioPoints();
	}
}

void CGMGalaxy::SetCapture(const bool bEnable)
{
	if (bEnable != m_bCapture)
	{
		m_bCapture = bEnable;
		m_pStateSetPlane->setDefine("CAPTURE", bEnable ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
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

bool CGMGalaxy::CreateGalaxy(float fDiameter)
{
	m_fGalaxyRadius = fDiameter * 0.5f;

	_CreateAudioPoints();
	_CreateGalaxyPoints();
	_CreateGalaxyPlane();
	//MakeNebula(fDiameter, fDiameter, 0.13f*fDiameter);

	return true;
}

void CGMGalaxy::SetMousePosition(const osg::Vec3f& vPos)
{
	if (m_bWelcome) return;

	m_pMousePosUniform->set(vPos);
}

void CGMGalaxy::SetCurrentStar(const osg::Vec3f& vPos, const std::wstring& wstrName)
{
	if (m_bWelcome) return;

	m_vPlayingStarWorldPos = vPos;
	m_pStarWorldPosUniform->set(vPos);
	m_strPlayingStarName = wstrName;

	if(m_pPlayingStarTransform.valid())
	{
		m_pPlayingStarTransform->asPositionAttitudeTransform()->setPosition(vPos);
	}

	// 设置当前颜色
	SGMStarCoord vStarcoord = GM_ENGINE_PTR->GetDataManager()->GetStarCoord(wstrName);
	m_pStarColorUniform->set(GM_ENGINE_PTR->GetDataManager()->GetAudioColor(vStarcoord));
}

void CGMGalaxy::SetAudioLevel(float fLevel)
{
	for (int i = PULSE_NUM-2; i >= 0; i--)
	{
		float fL;
		m_pLevelArrayUniform->getElement(i, fL);
		m_pLevelArrayUniform->setElement(i+1, fL);
	}
	m_pLevelArrayUniform->setElement(0, fLevel);
}

osg::Vec3d CGMGalaxy::GetStarWorldPos()
{
	return m_vPlayingStarWorldPos;
}

bool CGMGalaxy::_CreateAudioPoints()
{
	m_pGeodeAudio = new osg::Geode();
	GM_Root->addChild(m_pGeodeAudio.get());

	// 音频文件的星辰坐标Vector
	std::vector<SGMStarCoord> coordVector;
	GM_ENGINE_PTR->GetDataManager()->GetStarCoordVector(coordVector);

	// 从数据管理模块读取数据，创建未激活状态的音频星几何体
	osg::ref_ptr<osg::Geometry> pGeomAudio = _CreateAudioGeometry(coordVector);
	if (!pGeomAudio.valid()) return false;
	m_pGeodeAudio->addDrawable(pGeomAudio.get());

	osg::ref_ptr<osg::StateSet> pStateSetAudio = m_pGeodeAudio->getOrCreateStateSet();
	pStateSetAudio->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
	pStateSetAudio->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	pStateSetAudio->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pStateSetAudio->setMode(GL_BLEND, osg::StateAttribute::ON);
	pStateSetAudio->setAttributeAndModes(new osg::BlendFunc(), osg::StateAttribute::ON);
	pStateSetAudio->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pStateSetAudio->setRenderBinDetails(BIN_AUDIO_POINT, "DepthSortedBin");

	pStateSetAudio->addUniform(m_pMousePosUniform.get());
	pStateSetAudio->addUniform(m_pStarWorldPosUniform.get());
	pStateSetAudio->addUniform(m_pLevelArrayUniform.get());

	// 添加shader
	std::string strAudioVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "AudioVert.glsl";
	std::string strAudioFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "AudioFrag.glsl";
	return CGMKit::LoadShader(pStateSetAudio.get(), strAudioVertPath, strAudioFragPath);
}

bool CGMGalaxy::_CreateGalaxyPoints()
{
	osg::ref_ptr<osg::Geode> pGalaxyPointsGeode = new osg::Geode();
	GM_Root->addChild(pGalaxyPointsGeode.get());

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();

	size_t iNum = 65536;
	osg::ref_ptr<osg::Vec3Array> vertArray = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec2Array> texcoordArray = new osg::Vec2Array;
	osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array();
	osg::ref_ptr<osg::DrawElementsUShort> el = new osg::DrawElementsUShort(GL_POINTS);
	vertArray->reserve(iNum);
	texcoordArray->reserve(iNum);
	colorArray->reserve(iNum);
	el->reserve(iNum);

	int x = 0;
	while (x < iNum)
	{
		float fRandomX = (m_iRandom() % 10000)*0.0001f - 0.5f;
		float fRandomY = (m_iRandom() % 10000)*0.0001f - 0.5f;
		float fRandomAlpha = (m_iRandom() % 1000)*0.001f;
		float fX = m_fGalaxyRadius * 2.0f * fRandomX;
		float fY = m_fGalaxyRadius * 2.0f * fRandomY;
		float fU = fRandomX + 0.5f;
		float fV = fRandomY + 0.5f;

		float fA = _GetGalaxyValue(fU, fV, 3);
		if (fRandomAlpha < fA)
		{
			float fRandomR = (m_iRandom() % 100)*0.01f - 0.5f;
			float fR = max(0.0f, _GetGalaxyValue(fU, fV, 0) + fRandomR * fRandomR*fRandomR);
			float fG = _GetGalaxyValue(fU, fV, 1);
			float fB = _GetGalaxyValue(fU, fV, 2);

			float fRGBMax = max(max(max(fR, fG), fB), 1e-5);
			fR /= fRGBMax;
			fG /= fRGBMax;
			fB /= fRGBMax;

			float fRandomZ = (m_iRandom() % 1000)*0.002f - 1.0f;
			float fSignZ = (fRandomZ > 0) ? 1.0f : -1.0f;
			float fSmooth = fSignZ * (3 * fRandomZ*fRandomZ - 2 * abs(fRandomZ*fRandomZ*fRandomZ));
			float fZ = (fRandomAlpha + 0.2f)*0.04f*m_fGalaxyRadius*fSmooth;
			float fRandomRadius = (m_iRandom() % 1000)*0.001f;
			fRandomRadius = fRandomAlpha * fRandomRadius*fRandomRadius;
			float fRadiusNow = osg::Vec2(fRandomX, fRandomY).length();
			float fTmp = pow(min(1.0f, 1.03f*(1.0f - fRadiusNow)), 11);
			fZ = fZ * (0.5 + 3 * fTmp*fTmp - 2 * fTmp*fTmp*fTmp);
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

	osg::ref_ptr <osg::Vec3Array> normal = new osg::Vec3Array;
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
	osg::ref_ptr<osg::BlendEquation> blendEqua = new osg::BlendEquation(osg::BlendEquation::RGBA_MAX);
	m_pStateSetGalaxy->setAttributeAndModes(blendEqua.get(), osg::StateAttribute::ON);
	m_pStateSetGalaxy->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	m_pStateSetGalaxy->setRenderBinDetails(BIN_GALAXY_POINT, "DepthSortedBin");

	m_pStateSetGalaxy->setTextureAttributeAndModes(1, m_distanceMap_0.get());
	m_pStateSetGalaxy->addUniform(new osg::Uniform("distanceTex", 1));

	m_pStateSetGalaxy->addUniform(m_pStarWorldPosUniform.get());
	m_pStateSetGalaxy->addUniform(m_pLevelArrayUniform.get());
	m_pStateSetGalaxy->addUniform(m_pStarColorUniform.get());

	// 添加shader
	std::string strStarVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarVert.glsl";
	std::string strStarFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "StarFrag.glsl";
	return CGMKit::LoadShader(m_pStateSetGalaxy.get(), strStarVertPath, strStarFragPath);
}

bool CGMGalaxy::_CreateGalaxyPlane()
{
	osg::ref_ptr<osg::Geode> pGalaxyPlaneGeode = new osg::Geode();
	GM_Root->addChild(pGalaxyPlaneGeode.get());

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
	osg::ref_ptr<osg::Vec3Array> verArray = new osg::Vec3Array;
	verArray->push_back(osg::Vec3(-m_fGalaxyRadius, -m_fGalaxyRadius, 0));
	verArray->push_back(osg::Vec3(m_fGalaxyRadius, -m_fGalaxyRadius, 0));
	verArray->push_back(osg::Vec3(m_fGalaxyRadius, m_fGalaxyRadius, 0));
	verArray->push_back(osg::Vec3(-m_fGalaxyRadius, m_fGalaxyRadius, 0));
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
	m_pStateSetPlane->setAttributeAndModes(new osg::BlendFunc(), osg::StateAttribute::ON);
	m_pStateSetPlane->setRenderBinDetails(BIN_GALAXY_PLANE, "DepthSortedBin");

	std::string strGalaxyTexPath = m_pConfigData->strCorePath + m_strGalaxyTexPath;
	osg::ref_ptr<osg::Texture> _galaxyTex = _CreateTexture2D(strGalaxyTexPath + "milkyWay.tga" , 4);
	int iUnit = 0;
	m_pStateSetPlane->setTextureAttributeAndModes(iUnit, _galaxyTex.get());
	m_pStateSetPlane->addUniform(new osg::Uniform("galaxyTex", iUnit));
	iUnit++;

	m_pStateSetPlane->addUniform(m_pLevelArrayUniform.get());
	m_pStateSetPlane->addUniform(m_pMousePosUniform.get());

	// 添加shader
	std::string strGalaxyPlaneVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyPlaneVert.glsl";
	std::string strGalaxyPlaneFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "GalaxyPlaneFrag.glsl";
	return CGMKit::LoadShader(m_pStateSetPlane.get(), strGalaxyPlaneVertPath, strGalaxyPlaneFragPath);
}

bool CGMGalaxy::_DetachAudioPoints()
{
	SGMStarCoord vStarCoord = GM_ENGINE_PTR->GetDataManager()->GetStarCoord(m_strPlayingStarName);
	m_vPlayingAudioCoord = GM_ENGINE_PTR->GetDataManager()->GetAudioCoord(m_strPlayingStarName);
	m_pStarAudioPosUniform->set(osg::Vec2f(m_vPlayingAudioCoord.radius, m_vPlayingAudioCoord.angle));

	// 音频文件的星辰坐标Vector
	std::vector<SGMStarCoord> coordVector;
	GM_ENGINE_PTR->GetDataManager()->GetStarCoordVector(coordVector);

	std::vector<SGMStarCoord> newCoordVector;
	for (auto itr = coordVector.begin(); itr != coordVector.end(); itr++)
	{
		if (vStarCoord != *itr)
		{
			newCoordVector.push_back(*itr);
		}
	}
	if (newCoordVector.size() != (coordVector.size()-1))
	{
		return false;
	}

	osg::Geometry* pGeometry = dynamic_cast<osg::Geometry*>(m_pGeodeAudio->getDrawable(0));
	if (pGeometry)
	{
		m_pGeodeAudio->removeChild(pGeometry);
	}
	// 从修改过的数据管理模块读取数据，重新创建未激活状态的音频星几何体
	osg::ref_ptr<osg::Geometry> pGeomAudio = _CreateAudioGeometry(newCoordVector);
	if (!pGeomAudio.valid()) return false;
	m_pGeodeAudio->addDrawable(pGeomAudio.get());

	// 创建激活的音频星
	if (!m_pPlayingStarTransform.valid())
	{
		m_pPlayingStarTransform = new osg::PositionAttitudeTransform();
		osg::Geode* pGeode = new osg::Geode();
		osg::Geometry* pGeom = _CreateConeGeometry();
		pGeode->addDrawable(pGeom);
		m_pPlayingStarTransform->addChild(pGeode);
		GM_Root->addChild(m_pPlayingStarTransform.get());

		osg::ref_ptr<osg::StateSet> pSS = m_pPlayingStarTransform->getOrCreateStateSet();
		pSS->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);

		pSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pSS->setMode(GL_BLEND, osg::StateAttribute::ON);
		pSS->setAttributeAndModes(new osg::BlendFunc(), osg::StateAttribute::ON);
		pSS->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
		pSS->setRenderBinDetails(BIN_STAR_PLAYING, "DepthSortedBin");

		pSS->addUniform(m_pStarColorUniform.get());

		// 添加shader
		std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "PlayingStarVert.glsl";
		std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "PlayingStarFrag.glsl";
		CGMKit::LoadShader(pSS.get(), strVertPath, strFragPath);
	}
	osg::Vec3f vPos;
	m_pStarWorldPosUniform->get(vPos);
	m_pPlayingStarTransform->asPositionAttitudeTransform()->setPosition(vPos);
	//m_pPlayingStarTransform->setNodeMask(~0);
	m_pPlayingStarTransform->setNodeMask(0);

	// 创建音频空间坐标指示线
	if (!m_pGeodeHelpLine.valid())
	{
		m_pGeodeHelpLine = new osg::Geode();
		osg::Geometry* pLineGeometry = _CreateHelpLineGeometry();
		m_pGeodeHelpLine->addDrawable(pLineGeometry);
		GM_Root->addChild(m_pGeodeHelpLine.get());

		osg::ref_ptr<osg::StateSet> pLineSS = m_pGeodeHelpLine->getOrCreateStateSet();
		osg::ref_ptr<osg::LineWidth> pLineWidth = new osg::LineWidth;
		pLineWidth->setWidth(1);
		pLineSS->setAttributeAndModes(pLineWidth.get(), osg::StateAttribute::ON);
		pLineSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pLineSS->setMode(GL_BLEND, osg::StateAttribute::ON);
		pLineSS->setAttributeAndModes(new osg::BlendFunc(), osg::StateAttribute::ON);
		pLineSS->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
		pLineSS->setRenderBinDetails(BIN_HELP_LINE, "DepthSortedBin");

		pLineSS->addUniform(m_pStarAudioPosUniform.get());
		pLineSS->addUniform(m_pTimesUniform.get());

		// 添加shader
		std::string strLineVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "HelpLineVert.glsl";
		std::string strLineFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "HelpLineFrag.glsl";
		CGMKit::LoadShader(pLineSS.get(), strLineVertPath, strLineFragPath);
	}
	m_pGeodeHelpLine->setNodeMask(~0);

	return true;
}

bool CGMGalaxy::_AttachAudioPoints()
{
	SGMAudioData sData;
	sData.name = m_strPlayingStarName;
	sData.audioCoord = m_vPlayingAudioCoord;
	GM_ENGINE_PTR->GetDataManager()->SetAudioData(sData);

	// 音频文件的星辰坐标Vector
	std::vector<SGMStarCoord> coordVector;
	GM_ENGINE_PTR->GetDataManager()->GetStarCoordVector(coordVector);
	osg::Geometry* pGeometry = dynamic_cast<osg::Geometry*>(m_pGeodeAudio->getDrawable(0));
	if (pGeometry)
	{
		m_pGeodeAudio->removeChild(pGeometry);
	}
	// 从修改过的数据管理模块读取数据，重新创建未激活状态的音频星几何体
	osg::ref_ptr<osg::Geometry> pGeomAudio = _CreateAudioGeometry(coordVector);
	if (!pGeomAudio.valid()) return false;
	m_pGeodeAudio->addDrawable(pGeomAudio.get());

	// 隐藏激活的音频星
	if (m_pPlayingStarTransform.valid())
	{
		m_pPlayingStarTransform->setNodeMask(0);
	}
	if (m_pGeodeHelpLine.valid())
	{
		m_pGeodeHelpLine->setNodeMask(0);
	}
	return true;
}

osg::Geometry* CGMGalaxy::_CreateAudioGeometry(std::vector<SGMStarCoord>& coordVector)
{
	osg::Geometry* pGeomAudio = new osg::Geometry();

	size_t iNum = coordVector.size();
	if (0 == iNum) return nullptr;

	osg::ref_ptr<osg::Vec3Array> vertArray = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array();
	osg::ref_ptr<osg::DrawElementsUShort> el = new osg::DrawElementsUShort(GL_POINTS);
	vertArray->reserve(iNum);
	colorArray->reserve(iNum);
	el->reserve(iNum);

	int x = 0;
	for (auto itr : coordVector)
	{
		float fX = float(itr.x) / GM_COORD_MAX;
		float fY = float(itr.y) / GM_COORD_MAX;
		float fZ = float(itr.z) / GM_COORD_MAX;

		osg::Vec4f vColor = GM_ENGINE_PTR->GetDataManager()->GetAudioColor(itr);

		fX *= m_fGalaxyRadius;
		fY *= m_fGalaxyRadius;
		fZ *= 0.001f * m_fGalaxyRadius;

		vertArray->push_back(osg::Vec3f(fX, fY, fZ));
		colorArray->push_back(vColor);
		el->push_back(x);
		x++;
	}

	pGeomAudio->setVertexArray(vertArray.get());
	pGeomAudio->setColorArray(colorArray.get());
	pGeomAudio->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	osg::ref_ptr <osg::Vec3Array> normal = new osg::Vec3Array;
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

	osg::ref_ptr <osg::Vec3Array> verts = new osg::Vec3Array();
	verts->reserve(2 + segments);
	geom->setVertexArray(verts.get());

	osg::ref_ptr<osg::Vec3Array> textArray = new osg::Vec3Array;
	textArray->reserve(2 + segments);
	geom->setTexCoordArray(0, textArray.get());

	osg::ref_ptr <osg::Vec3Array> normal = new osg::Vec3Array;
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
		double angle = deltaAngle * float(i+1);
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

osg::Geometry* CGMGalaxy::_CreateHelpLineGeometry()
{
	// 角度分段数
	int iAngleSegments = 128;
	// 半径分段数
	int iRadiusSegments = 128;
	int iVertexNum = 2 * (iAngleSegments + iRadiusSegments);

	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);
	geom->setUseDisplayList(false);
	geom->setDataVariance(osg::Object::DYNAMIC);

	osg::ref_ptr <osg::Vec3Array> verts = new osg::Vec3Array();
	verts->reserve(iVertexNum);
	geom->setVertexArray(verts.get());

	osg::ref_ptr<osg::Vec3Array> textArray = new osg::Vec3Array;
	textArray->reserve(iVertexNum);
	geom->setTexCoordArray(0, textArray.get());

	osg::ref_ptr<osg::DrawElementsUShort> el = new osg::DrawElementsUShort(GL_LINES);
	el->reserve(iVertexNum);
	geom->addPrimitiveSet(el.get());

	// 角度辅助线
	float fAngleEach = osg::PI * 2.0 / float(iAngleSegments);
	for (int x = 0; x < 2 * iAngleSegments; x++)
	{
		float fMod2 = float(x % 2);
		float fX = m_fGalaxyRadius * cos(fAngleEach * x * 0.5);
		float fY = m_fGalaxyRadius * sin(fAngleEach * x * 0.5);
		verts->push_back(osg::Vec3(fX, fY, 0));
		textArray->push_back(osg::Vec3(0.0f, (float(x) - fMod2 * 0.4 + 0.5) / float(2 * iAngleSegments), 1.0f - fMod2));
		el->push_back(x);
	}

	// 半径辅助线
	float fRadiusEach = m_fGalaxyRadius / float(iRadiusSegments);
	for (int y = 0; y < 2 * iRadiusSegments; y++)
	{
		verts->push_back(osg::Vec3(0, m_fGalaxyRadius*0.5*float(y)/float(iRadiusSegments), 0));
		textArray->push_back(osg::Vec3((float(y + y % 2) + 0.5) / float(2 * iRadiusSegments), 0, 0));
		el->push_back(2 * iAngleSegments + y);
	}

	return geom;
}

float CGMGalaxy::_GetGalaxyValue(float fX, float fY, int iChannel, bool bLinear)
{
	if (!m_pGalaxyImage.valid())
	{
		std::string strFile = m_pConfigData->strCorePath + m_strGalaxyTexPath + "milkyWay.tga";
		m_pGalaxyImage = osgDB::readImageFile(strFile);
	}
	if (m_pGalaxyImage.valid())
	{
		unsigned int iWidth = m_pGalaxyImage->s();
		unsigned int iHeight = m_pGalaxyImage->t();

		float fS = fX*iWidth;
		float fT = fY*iHeight;

		float fDeltaS = fS - (int)fS;
		float fDeltaT = fT - (int)fT;
		unsigned int s = (unsigned int)fS;
		unsigned int t = (unsigned int)fT;
		unsigned int s_next = (s == iWidth) ? iWidth : (s + 1);
		unsigned int t_next = (t == iHeight) ? iHeight : (t + 1);

		float fValue_00 = 0;
		float fValue_10 = 0;
		float fValue_01 = 0;
		float fValue_11 = 0;

		switch (iChannel)
		{
		case 0:
		{
			fValue_00 = m_pGalaxyImage->getColor(s, t).r();
		}
		break;
		case 1:
		{
			fValue_00 = m_pGalaxyImage->getColor(s, t).g();
		}
		break;
		case 2:
		{
			fValue_00 = m_pGalaxyImage->getColor(s, t).b();
		}
		break;
		case 3:
		{
			fValue_00 = m_pGalaxyImage->getColor(s, t).a();
		}
		break;
		default: {}
		}

		if (bLinear)
		{
			switch (iChannel)
			{
			case 0:
			{
				fValue_10 = m_pGalaxyImage->getColor(s_next, t).r();
				fValue_01 = m_pGalaxyImage->getColor(s, t_next).r();
				fValue_11 = m_pGalaxyImage->getColor(s_next, t_next).r();
			}
			break;
			case 1:
			{
				fValue_10 = m_pGalaxyImage->getColor(s_next, t).g();
				fValue_01 = m_pGalaxyImage->getColor(s, t_next).g();
				fValue_11 = m_pGalaxyImage->getColor(s_next, t_next).g();
			}
			break;
			case 2:
			{
				fValue_10 = m_pGalaxyImage->getColor(s_next, t).b();
				fValue_01 = m_pGalaxyImage->getColor(s, t_next).b();
				fValue_11 = m_pGalaxyImage->getColor(s_next, t_next).b();
			}
			break;
			case 3:
			{
				fValue_10 = m_pGalaxyImage->getColor(s_next, t).a();
				fValue_01 = m_pGalaxyImage->getColor(s, t_next).a();
				fValue_11 = m_pGalaxyImage->getColor(s_next, t_next).a();
			}
			break;
			default: {}
			}
		}

		float fValue = fValue_00;
		if (bLinear)
		{
			fValue =
				fValue_00 * (1 - fDeltaS) * (1 - fDeltaT)
				+ fValue_10 * fDeltaS * (1 - fDeltaT)
				+ fValue_01 * (1 - fDeltaS) * fDeltaT
				+ fValue_11 * fDeltaS * fDeltaT;
		}
		return fValue;
	}
	return 0.0f;
}

/**
* _CreateTexture2D
* 加载2D纹理
* @author LiuTao
* @since 2020.06.16
* @param fileName: 图片文件路径
* @param iChannelNum: 1、2、3、4分别代表R、RG、RGB、RGBA
* @return osg::Texture* 返回纹理指针
*/
osg::Texture* CGMGalaxy::_CreateTexture2D(const std::string & fileName, const int iChannelNum)
{
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	texture->setImage(osgDB::readImageFile(fileName));
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
