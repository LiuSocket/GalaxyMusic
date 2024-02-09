//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMSolar.cpp
/// @brief		Galaxy-Music Engine - GMSolar
/// @version	1.0
/// @author		LiuTao
/// @date		2020.11.27
//////////////////////////////////////////////////////////////////////////

#include "GMSolar.h"
#include "GMAtmosphere.h"
#include "GMEarth.h"
#include "GMPlanet.h"
#include "GMOort.h"
#include "GMTerrain.h"
#include "GMEngine.h"
#include "GMXml.h"
#include "GMKit.h"
#include <osg/PointSprite>
#include <osg/LineWidth>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/PositionAttitudeTransform>
#include <osg/PolygonOffset>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

using namespace GM;

/*************************************************************************
constexpr
*************************************************************************/
constexpr auto RING_SEGMENT			= 128;				// ���ǹ⻷����Ȧ���ж��ٶ�����;
constexpr auto ASTEROID_NUM			= 16384;			// С���Ǵ��ϵĵ㾫������
constexpr auto GM_YEAR				= 1800.0;			// ��ϵͳ�ġ����򡱹�ת���ڡ���λ����
constexpr auto GM_YEAR_REAL			= 31558150.0; 		// һ�����ꡣ��λ���루365.25636*24*60*60��

constexpr auto SUN_RADIUS			= 6.963e8;			// ̫���뾶����λ����;
constexpr auto SUN_BLOOM_SCALE		= 10.0;				// ̫���Թ��ķŴ�������뾶��̫���뾶�ı�ֵ��

constexpr auto GM_AU				= 1.495978707e11; 		// ���ĵ�λ����λ����
constexpr auto GM_R_MERCURY			= GM_AU * 0.3871; 		// ˮ�ǹ���뾶����λ����
constexpr auto GM_R_VENUS			= GM_AU * 0.7233; 		// ���ǹ���뾶����λ����
constexpr auto GM_R_MARS			= GM_AU * 1.52;			// ���ǹ���뾶����λ����
constexpr auto GM_R_JUPITER			= GM_AU * 5.20; 		// ľ�ǹ���뾶����λ����
constexpr auto GM_R_SATURN			= GM_AU * 9.55;			// ���ǹ���뾶����λ����
constexpr auto GM_R_URANUS			= GM_AU * 19.22; 		// �����ǹ���뾶����λ����
constexpr auto GM_R_NEPTUNE			= GM_AU * 30.11; 		// �����ǹ���뾶����λ����

/*************************************************************************
Macro Defines
*************************************************************************/
#define GROUND_BASE_UNIT		(0) 		// �������Ļ�������Ԫ
#define GROUND_INSCAT_UNIT		(2) 		// ��������ϵĴ�������ɢ�䡱����Ԫ

#define CLOUD_BASE_UNIT			(0) 		// �����Ʋ�Ļ�������Ԫ
#define CLOUD_RING_UNIT			(1) 		// �����Ʋ���յ��Ĺ⻷��Ӱ������Ԫ
#define CLOUD_INSCAT_UNIT		(2) 		// �����Ʋ�λ�õĴ����ġ���ɢ�䡱����Ԫ

#define ATMOS_RING_UNIT			(0) 		// ��������ġ��ǻ�������Ԫ
#define ATMOS_INSCAT_UNIT		(1) 		// ��������ġ���ɢ�䡱����Ԫ

/*************************************************************************
constexpr
*************************************************************************/

constexpr double ATMOS_R = 0.2; 		// ���������ɫ�ĺ��ǿ��
constexpr double ATMOS_G = 0.5; 		// ���������ɫ���̹�ǿ��
constexpr double ATMOS_B = 1.0; 		// ���������ɫ������ǿ��

/*************************************************************************
Class
*************************************************************************/
namespace GM
{
class SwitchTexCallback : public osg::Camera::DrawCallback
{
public:
	SwitchTexCallback(osg::Texture *dst, osg::Texture *src) :
		_dstTexture(dst), _srcTexture(src)
	{
		_fbo = new osg::FrameBufferObject();
		_width = dynamic_cast<osg::Texture2D*>(_srcTexture.get())->getTextureWidth();
		_height = dynamic_cast<osg::Texture2D*>(_srcTexture.get())->getTextureHeight();
	}

	virtual void operator() (osg::RenderInfo& renderInfo) const
	{
		osg::GLExtensions* ext = renderInfo.getState()->get<osg::GLExtensions>();
		bool fbo_supported = ext && ext->isFrameBufferObjectSupported;

		_fbo->setAttachment(osg::Camera::COLOR_BUFFER0, osg::FrameBufferAttachment((osg::Texture2D*)(_srcTexture.get())));
		_fbo->setAttachment(osg::Camera::COLOR_BUFFER1, osg::FrameBufferAttachment((osg::Texture2D*)(_dstTexture.get())));
		_srcTexture->apply(*renderInfo.getState());
		_dstTexture->apply(*renderInfo.getState());

		if (fbo_supported && ext->glBlitFramebuffer)
		{
			(_fbo.get())->apply(*renderInfo.getState());
			ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, _fbo->getHandle(renderInfo.getContextID()));
			ext->glFramebufferTexture2D(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, _srcTexture->getTextureObject(renderInfo.getContextID())->id(), 0);
			ext->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, _dstTexture->getTextureObject(renderInfo.getContextID())->id(), 0);

			glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);

			ext->glBlitFramebuffer(
				0, 0, static_cast<GLint>(_width), static_cast<GLint>(_height),
				0, 0, static_cast<GLint>(_width), static_cast<GLint>(_height),
				GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}

		if(ext)
			ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
	}

private:
	osg::ref_ptr<osg::FrameBufferObject> _fbo;
	osg::ref_ptr<osg::Texture> _dstTexture;
	osg::ref_ptr<osg::Texture> _srcTexture;
	unsigned int _width = 8192;
	unsigned int _height = 2;
};

}	// GM

/*************************************************************************
CGMSolar Methods
*************************************************************************/

/** @brief ���� */
CGMSolar::CGMSolar() : m_iCenterCelestialBody(0),
	m_strGalaxyShaderPath("Shaders/GalaxyShader/"), m_strGalaxyTexPath("Textures/Galaxy/"), 
	m_strCoreModelPath("Models/"), m_mView2ECEFMatrix(osg::Matrixd()), m_mWorld2Shadow(osg::Matrixd()),
	m_vPlanetNorth(0.0, 0.0, 1.0), m_vPlanetAxisX(1.0, 0.0, 0.0),
	m_qSolarRotate(osg::Quat(osg::PI*0.3, osg::Vec3d(1, 0, 0), osg::PI*0.2, osg::Vec3d(0, 1, 0), osg::PI*0.1, osg::Vec3d(0, 0, 1))),
	m_qPlanetRotate(osg::Quat()),
	m_vSolarPos_Hie1(0.0, 0.0, 0.0), m_vSolarPos_Hie2(0.0, 0.0, 0.0), m_fEyeAltitude(1e10f), m_fWanderingEarthProgress(0.0f),
	m_fBackgroundSunScaleUniform(new osg::Uniform("backgroundSunScale", 1.0f)),
	m_fBackgroundSunAlphaUniform(new osg::Uniform("backgroundSunAlpha", 1.0f)),
	m_fSunEdgeUniform(new osg::Uniform("sunEdgePos", 0.1f)),
	m_fSupernovaLightUniform(new osg::Uniform("supernovaLight", 1.0f)),
	m_fSupernovaBloomScaleUniform(new osg::Uniform("supernovaBloomScale", 1.0f)),
	m_fSupernovaBloomAlphaUniform(new osg::Uniform("supernovaBloomAlpha", 1.0f)),
	m_fSupernovaAlphaUniform(new osg::Uniform("supernovaAlpha", osg::Vec2f(1.0f, 1.0f))),
	m_fPlanetNumUniform(new osg::Uniform("planetNum", 0.0f)),
	m_fPlanetTailAlphaUniform(new osg::Uniform("planetTailAlpha", 1.0f)),
	m_fPlanetLineAlphaUniform(new osg::Uniform("planetLineAlpha", 1.0f)),
	m_vPlanetRadiusUniform(new osg::Uniform("planetRadius", osg::Vec2f(0.001f, 0.001f))),
	m_fPlanetPointAlphaUniform(new osg::Uniform("planetPointAlpha", 1.0f)),
	m_vViewLightUniform(new osg::Uniform("viewLight", osg::Vec3f(1.0f, 0.0f, 0.0f))),
	m_mRingShadowMatrixUniform(new osg::Uniform("ringShadowMatrix", osg::Matrixf())),
	m_mPlanetShadowMatrixUniform(new osg::Uniform("planetShadowMatrix", osg::Matrixf())),
	m_fNorthDotLightUniform(new osg::Uniform("cosNorthLight", 0.0f)),
	m_fCloudTopUniform(new osg::Uniform("cloudTop", 0.0f)),
	m_fGroundTopUniform(new osg::Uniform("groundTop", 0.01f)),
	m_fAtmosHeightUniform(new osg::Uniform("atmosHeight", 0.1f)),
	m_fMinDotULUniform(new osg::Uniform("minDotUL", -0.2f)),
	m_fEyeAltitudeUniform(new osg::Uniform("eyeAltitude", 1.0f)),
	m_mAtmosColorTransUniform(new osg::Uniform("atmosColorMatrix", osg::Matrixf())),
	m_mView2ECEFUniform(new osg::Uniform("view2ECEFMatrix", osg::Matrixf())),
	m_fStepTimeUniform(new osg::Uniform("stepTime", 525600.0f)),// ����ֵ��Ҫÿ֡��������
	m_vJupiterPosUniform(new osg::Uniform("jupiterPos", osg::Vec2f(0.0f, 1e12f))),// ����ֵ��Ҫÿ֡��������
	m_vCoordScaleUniform(new osg::Uniform("coordScale", osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f))),//UV���ţ��Ĳ㹻����
	m_iPlanetCount(0), m_pCelestialScaleVisitor(nullptr),
	m_pTerrain(nullptr), m_pAtmos(nullptr), m_pEarth(nullptr), m_pPlanet(nullptr), m_pOort(nullptr)
{
	m_pTerrain = new CGMTerrain();
	m_pAtmos = new CGMAtmosphere();
	m_pEarth = new CGMEarth();
	m_pPlanet = new CGMPlanet();
	m_pOort = new CGMOort();

	m_iRandom.seed(0);
	const double fSecondPerHour = 60 * 60;
	// ̫��
	m_sCelestialBodyVector.push_back(SGMCelestialBody(0, 1e9, 0, SUN_RADIUS, SUN_RADIUS, 0, 1, 0.0f, 0.0f, EGMAH_0,
		osg::Vec4f(1,1,1,1)));
	// ˮ��
	// https://nssdc.gsfc.nasa.gov/planetary/factsheet/mercuryfact.html
	m_sCelestialBodyVector.push_back(SGMCelestialBody(GM_R_MERCURY, 0.241*GM_YEAR, 11, 2.4405e6, 2.4383e6,
		osg::DegreesToRadians(0.034), 1407.6*fSecondPerHour, 10000.0f, 0.0f, EGMAH_0,
		osg::Vec4f(1, 1, 1, 0)));
	// ����
	// https://nssdc.gsfc.nasa.gov/planetary/factsheet/venusfact.html
	m_sCelestialBodyVector.push_back(SGMCelestialBody(GM_R_VENUS, 0.616*GM_YEAR, 1, 6.0518e6, 6.0518e6,
		osg::DegreesToRadians(177.36), -5832.6*fSecondPerHour, 10000.0f, 1e4f, EGMAH_128,
		osg::Vec4f(1.0, 0.6, 0.4, 1.0)));
	// ����
	m_sCelestialBodyVector.push_back(SGMCelestialBody(GM_AU, GM_YEAR, 0, 6378137, 6356752,
		osg::DegreesToRadians(23.44), 23.9345*fSecondPerHour, 8848.0f, m_pEarth->GetCloudTopHeight(), EGMAH_64,
		osg::Vec4f(ATMOS_R, ATMOS_G, ATMOS_B, 1.0)));
	// ����
	// https://nssdc.gsfc.nasa.gov/planetary/factsheet/marsfact.html
	m_sCelestialBodyVector.push_back(SGMCelestialBody(GM_R_MARS, 1.882*GM_YEAR, 5, 3.3962e6, 3.3762e6, 
		osg::DegreesToRadians(25.19), 24.6229*fSecondPerHour, 25000.0f, 2e3f, EGMAH_16,
		osg::Vec4f(0.3, 0.25, 0.2, 1.0)));
	// ľ��
	// https://nssdc.gsfc.nasa.gov/planetary/factsheet/jupiterfact.html
	m_sCelestialBodyVector.push_back(SGMCelestialBody(GM_R_JUPITER, 11.86*GM_YEAR, 7, 7.1492e7, 6.6854e7, 
		osg::DegreesToRadians(3.13), 9.9250*fSecondPerHour, 0.0f, 0.0f, EGMAH_128,
		osg::Vec4f(0.4, 0.3, 0.2, 1.0), 8.94e7, 2.8e8));
	// ����
	// https://nssdc.gsfc.nasa.gov/planetary/factsheet/saturnfact.html
	m_sCelestialBodyVector.push_back(SGMCelestialBody(GM_R_SATURN, 29.46*GM_YEAR, 2.5, 6.0268e7, 5.4364e7, 
		osg::DegreesToRadians(26.73), 10.656*fSecondPerHour, 0.0f, 0.0f, EGMAH_128,
		osg::Vec4f(0.36, 0.28, 0.2, 1.0), 7.4658e7, 1.39826e8));
	// ������
	// https://nssdc.gsfc.nasa.gov/planetary/factsheet/uranusfact.html
	m_sCelestialBodyVector.push_back(SGMCelestialBody(GM_R_URANUS, 84.01*GM_YEAR, 13, 2.5559e7, 2.4973e7, 
		osg::DegreesToRadians(97.77), -17.24*fSecondPerHour, 0.0f, 0.0f, EGMAH_128,
		osg::Vec4f(0.05, 0.3, 0.8, 1.0), 4.1837e7, 5.1149e7));
	// ������
	// https://nssdc.gsfc.nasa.gov/planetary/factsheet/neptunefact.html
	m_sCelestialBodyVector.push_back(SGMCelestialBody(GM_R_NEPTUNE, 164.82*GM_YEAR, 17, 2.4764e7, 2.4341e7, 
		osg::DegreesToRadians(28.32), 16.11*fSecondPerHour, 0.0f, 0.0f, EGMAH_128,
		osg::Vec4f(0.02, 0.2, 1.0, 1.0), 4.19e7, 6.2933e7));

	m_pCelestialScaleVisitor = new CGMCelestialScaleVisitor();
}

/** @brief ���� */
CGMSolar::~CGMSolar()
{
}

/** @brief ��ʼ�� */
bool CGMSolar::Init(SGMKernelData * pKernelData, SGMConfigData * pConfigData,
	CGMCommonUniform* pCommonUniform, CGMDataManager* pDataManager)
{
	m_pKernelData = pKernelData;
	m_pConfigData = pConfigData;
	m_pCommonUniform = pCommonUniform;

	m_pOort->Init(pKernelData, pConfigData, pCommonUniform);
	m_pTerrain->Init(pKernelData, pConfigData, pCommonUniform);
	m_pAtmos->Init(pConfigData);
	m_pEarth->Init(pKernelData, pConfigData, pCommonUniform);
	m_pPlanet->Init(pConfigData);

	m_pEarth->SetUniform(
		m_vPlanetRadiusUniform,
		m_vViewLightUniform,
		m_fCloudTopUniform,
		m_fGroundTopUniform,
		m_fAtmosHeightUniform,
		m_fMinDotULUniform,
		m_fEyeAltitudeUniform,
		m_mView2ECEFUniform);

	m_3DShapeTex = _Load3DShapeNoise();

	for (int i = 0; i <= 3; i++)
	{
		osg::ref_ptr<osg::Group> _pRoot = new osg::Group();
		m_pHieSolarRootVector.push_back(_pRoot);
	}

	// ��ȡddsʱ��Ҫ��ֱ��ת
	m_pDDSOptions = new osgDB::Options("dds_flip");
	std::string strGalaxyTexPath = pConfigData->strCorePath + m_strGalaxyTexPath;

	// ��ʼ����һ�㼶�����ĺ��Ǳ任�ڵ㣬Ҳ�ǵڶ��㼶���ǵĹ�ת������ı任�ڵ�
	if (!m_pStar_1_Transform.valid())
	{
		m_pStar_1_Transform = new osg::PositionAttitudeTransform();
		m_pHieSolarRootVector.at(1)->addChild(m_pStar_1_Transform);
	}
	if (!m_pPlanet_1_Transform.valid())
	{
		m_pPlanet_1_Transform = new osg::PositionAttitudeTransform();
		m_pHieSolarRootVector.at(1)->addChild(m_pPlanet_1_Transform);

		// ����1�ռ�㼶�ĵ�����ڵ�ӵ����������Ǹ��ڵ���
		m_pPlanet_1_Transform->addChild(m_pEarth->GetEarthRoot(1));
	}
	// ��ʼ���ڶ��㼶
	if (!m_pStar_2_Transform.valid())
	{
		m_pStar_2_Transform = new osg::PositionAttitudeTransform();
		m_pHieSolarRootVector.at(2)->addChild(m_pStar_2_Transform);
	}
	if (!m_pPlanet_2_Transform.valid())
	{
		m_pPlanet_2_Transform = new osg::PositionAttitudeTransform();
		m_pHieSolarRootVector.at(2)->addChild(m_pPlanet_2_Transform);

		// ����2�ռ�㼶�ĵ�����ڵ�ӵ����������Ǹ��ڵ���
		m_pPlanet_2_Transform->addChild(m_pEarth->GetEarthRoot(2));
	}
	if (!m_pRing_2_Transform.valid())
	{
		m_pRing_2_Transform = new osg::PositionAttitudeTransform();
		m_pHieSolarRootVector.at(2)->addChild(m_pRing_2_Transform);
	}
	// ��ʼ�����Ǻ����ǵ���ͼ
	std::string strSphereTexPath = m_pConfigData->strCorePath + "Textures/Sphere/";
	// ��ʼ��ˮ��
	m_aMercuryBaseTex = _CreateDDSTex2DArray(strSphereTexPath + "Mercury/Mercury_base_");
	// ��ʼ������
	m_aVenusBaseTex = _CreateDDSTex2DArray(strSphereTexPath + "Venus/Venus_base_");
	m_aVenusCloudTex = _CreateDDSTex2DArray(strSphereTexPath + "Venus/Venus_cloud_");
	// ��ʼ�����򣬸������Ͽ��Բο���https://svs.gsfc.nasa.gov/4720
	m_aMoonBaseTex = _CreateDDSTex2DArray(strSphereTexPath + "Moon/Moon_base_");
	// ��ʼ������
	m_aMarsBaseTex = _CreateDDSTex2DArray(strSphereTexPath + "Mars/Mars_base_");
	// ��ʼ��ľ��
	m_aJupiterCloudTex = _CreateDDSTex2DArray(strSphereTexPath + "Jupiter/Jupiter_cloud_");
	m_pJupiterRingTex = _CreateDDSTexture(strSphereTexPath + "Jupiter/Jupiter_ring.dds",osg::Texture::CLAMP_TO_BORDER, osg::Texture::REPEAT);
	// ��ʼ������
	m_aSaturnCloudTex = _CreateDDSTex2DArray(strSphereTexPath + "Saturn/Saturn_cloud_");
	m_pSaturnRingTex = _CreateDDSTexture(strSphereTexPath + "Saturn/Saturn_ring.dds",osg::Texture::CLAMP_TO_BORDER, osg::Texture::REPEAT);
	// ��ʼ��������
	m_aUranusCloudTex = _CreateDDSTex2DArray(strSphereTexPath + "Uranus/Uranus_cloud_");
	m_pUranusRingTex = _CreateDDSTexture(strSphereTexPath + "Uranus/Uranus_ring.dds", osg::Texture::CLAMP_TO_BORDER, osg::Texture::REPEAT);
	// ��ʼ��������
	m_aNeptuneCloudTex = _CreateDDSTex2DArray(strSphereTexPath + "Neptune/Neptune_cloud_");
	m_pNeptuneRingTex = _CreateDDSTexture(strSphereTexPath + "Neptune/Neptune_ring.dds",osg::Texture::CLAMP_TO_BORDER, osg::Texture::REPEAT);
	// ��ʼ��ڤ����
	m_aPlutoBaseTex = _CreateDDSTex2DArray(strSphereTexPath + "Pluto/Pluto_base_");
	// ��ʼ������
	m_aCharonBaseTex = _CreateDDSTex2DArray(strSphereTexPath + "Charon/Charon_base_");
	//// ����
	//m_aAuroraTex = _CreateDDSTex2DArray(strSphereTexPath + "aurora.dds");

	// ��ʼ�������㼶

	// ����̫��ϵ��ʼ��Ϣ
	CGMXml aXML;
	if (aXML.Load(m_pConfigData->strCorePath + "Users/SolarData.cfg", "SolarSystem"))
	{
		CGMXmlNode sJupiter = aXML.GetChild("Jupiter");
		double fTrueAnomaly = sJupiter.GetPropDouble("trueAnomaly");
		m_sCelestialBodyVector.at(5).fStartTrueAnomaly = fTrueAnomaly;
	}

	// ��ϴС���Ǵ�����
	//_WashAsteroidBeltData();

	// ��ʼ����ʹ��compute shader����С���Ǵ��˶���Ϣ���Լ�С���Ǵ���ʾģ��
	_InitAsteroidBelt();

	return true;
}

/** @brief ���� */
bool CGMSolar::Update(double dDeltaTime)
{
	double fTimes = osg::Timer::instance()->time_s();
	int iHie = m_pKernelData->iHierarchy;
	if (0 == iHie || 1 == iHie || 2 == iHie)
	{
		_UpdatePlanetSpin(dDeltaTime);
		osg::Quat qPlanetSpin = _GetPlanetSpin();
		osg::Quat qPlanetInclination = _GetPlanetInclination(fTimes);
		osg::Quat qPlanetTurn = _GetPlanetTurn(fTimes);
		m_qPlanetRotate = qPlanetSpin * qPlanetInclination * qPlanetTurn;
	}

	switch (iHie)
	{
	case 0:
	{
	}
	break;
	case 1:
	{
		// ���幫ת�뾶
		double fHieR = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fOrbitalRadius
			/ m_pKernelData->fUnitArray->at(1);
		// ����������
		double fSunTheta = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fStartTrueAnomaly
			+ fTimes * osg::PI * 2 / m_sCelestialBodyVector.at(m_iCenterCelestialBody).fOrbitalPeriod;
		osg::Vec3d vSolarPos = osg::Vec3d(-fHieR * cos(fSunTheta), -fHieR * sin(fSunTheta), 0);
		osg::Matrix mSolarRotate = osg::Matrix::identity();
		mSolarRotate.setRotate(m_qPlanetRotate.inverse());
		m_vSolarPos_Hie1 = mSolarRotate.preMult(vSolarPos);

		m_pStar_1_Transform->asPositionAttitudeTransform()->setPosition(m_vSolarPos_Hie1);
		m_pSunBloomTransform->setPosition(m_vSolarPos_Hie1);
		m_pSupernovaTransform->setPosition(m_vSolarPos_Hie1);
	}
	break;
	case 2:
	{
		// ���µڶ��㼶�ĺ��Ǻ����Ǳ任����

		// ���幫ת�뾶
		double fHieR = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fOrbitalRadius
			/ m_pKernelData->fUnitArray->at(2);
		// ����������
		double fSunTheta = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fStartTrueAnomaly
			+ fTimes * osg::PI * 2 / m_sCelestialBodyVector.at(m_iCenterCelestialBody).fOrbitalPeriod;
		m_vSolarPos_Hie2 = osg::Vec3d(-fHieR * cos(fSunTheta), -fHieR * sin(fSunTheta), 0);
		m_pStar_2_Transform->asPositionAttitudeTransform()->setPosition(m_vSolarPos_Hie2);
		m_pSunBloomTransform->setPosition(m_vSolarPos_Hie2);
		m_pSupernovaTransform->setPosition(m_vSolarPos_Hie2);

		//��������β������ת
		osg::Quat qTailRotate = osg::Quat(fSunTheta, osg::Vec3d(0, 0, 1));
		m_pPlanetTailTransform->asPositionAttitudeTransform()->setAttitude(qTailRotate);
		double fScale = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fOrbitalRadius / GM_AU;
		m_pPlanetTailTransform->asPositionAttitudeTransform()->setScale(osg::Vec3d(fScale, fScale, fScale));

		// ����������ת
		m_pPlanet_2_Transform->asPositionAttitudeTransform()->setAttitude(m_qPlanetRotate);

		// ��2�㼶�ռ��£����ǵı�����/��ת��
		//���˵���ı�������Ҫʵʱ�仯���Ա�֤������ٶ�
		m_vPlanetNorth = m_qPlanetRotate * osg::Vec3d(0, 0, 1);
		m_vPlanetNorth.normalize();
		// ��2�㼶�ռ��£����ǵ�X��(��γ0��)
		m_vPlanetAxisX = m_qPlanetRotate * osg::Vec3d(1, 0, 0);
		m_vPlanetAxisX.normalize();

		// ����ģ���ڲ���Ҫ������ת�ڵ�
		m_pEarth->SetEarthRotate(m_qPlanetRotate);
	}
	break;
	default:
		break;
	}

	// ���С���Ǵ�������������ﵽԤ�ں�رռ��㲢������洢��ͼƬ��
	if (m_pAsteroidComputeNode->getDirty())
	{
		m_pAsteroidComputeNode->setDirty(false);
		m_pReadPixelFinishCallback->SetReady(true);
		m_pReadPixelFinishCallback->SetWritten(false);
	}

	// ���·��沽��
	float fStepTime = dDeltaTime * GM_YEAR_REAL / GM_YEAR;
	m_fStepTimeUniform->set(fStepTime);

	// ����ľ��λ��
	// ľ�ǹ�ת�뾶
	double fJupiterR = m_sCelestialBodyVector.at(5).fOrbitalRadius;
	// ľ�ǵ�������
	double fPlanetTheta = m_sCelestialBodyVector.at(5).fStartTrueAnomaly
		+ fTimes * osg::PI * 2 / m_sCelestialBodyVector.at(5).fOrbitalPeriod;
	fPlanetTheta = fmod(fPlanetTheta, osg::PI * 2);
	// ľ��λ��
	m_vJupiterPosUniform->set(osg::Vec2f(fJupiterR*cos(fPlanetTheta), fJupiterR*sin(fPlanetTheta)));

	m_pEarth->Update(dDeltaTime);
	m_pOort->Update(dDeltaTime);

	return true;
}

/** @brief ����(�������������̬֮��) */
bool CGMSolar::UpdateLater(double dDeltaTime)
{
	osg::Matrixd mViewMatrix = GM_View->getCamera()->getViewMatrix();
	osg::Vec3d vEye, vCenter, vUp;
	GM_View->getCamera()->getViewMatrixAsLookAt(vEye, vCenter, vUp);
	double fDistance = GM_ENGINE.GetHierarchyTargetDistance();

	int iHie = GM_ENGINE.GetHierarchy();
	double fUnit = m_pKernelData->fUnitArray->at(iHie);

	// �ڶ��㼶�ռ��£��������ǵı���������ƽ��ķ�����
	// Ϊ������㣬������������ı����ᶼ������ᴹֱ
	osg::Vec3d vPlanetAxisNorm(1.0, 0.0, 0.0);

	if (2 >= iHie)
	{
		// �����۵�߶�
		osg::Vec3d vWorldSky = vEye;
		double fEye2Center = vWorldSky.normalize();
		double cosTheta = m_vPlanetNorth * vWorldSky;
		double a = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fEquatorRadius;
		double b = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fPolarRadius;
		double fSurface2Center = a * b / sqrt(b * b + cosTheta * cosTheta * (a * a - b * b));
		// ע��������۵�߶ȵ�λ�ǣ���
		m_fEyeAltitude = fEye2Center * fUnit - fSurface2Center;
		// ע��������۾��߶����ڵ�ǰ�ռ�㼶��λ�µ�ֵ
		m_fEyeAltitudeUniform->set(float(m_fEyeAltitude / fUnit));

		// ������ռ䡱�����ǵ����ĵ�����
		osg::Vec3d vWorldPlanetPos = osg::Vec3d(0,0,0);
		if (1 >= iHie)
		{
			//vWorldPlanetPos = to do
		}
		// ���㡰����ռ䡱ת��ECEF�ռ䡱�ľ���
		// ����ġ�����ռ䡱��ָ����1/2�㼶�ռ䡱����ϵ
		osg::Vec3d vPlanetAxisY = m_vPlanetNorth ^ m_vPlanetAxisX;
		vPlanetAxisY.normalize();
		osg::Matrixd mWorld2ECEF = osg::Matrixd(
			m_vPlanetAxisX.x(), vPlanetAxisY.x(), m_vPlanetNorth.x(), 0.0,
			m_vPlanetAxisX.y(), vPlanetAxisY.y(), m_vPlanetNorth.y(), 0.0,
			m_vPlanetAxisX.z(), vPlanetAxisY.z(), m_vPlanetNorth.z(), 0.0,
			-m_vPlanetAxisX * vWorldPlanetPos, -vPlanetAxisY * vWorldPlanetPos, -m_vPlanetNorth * vWorldPlanetPos, 1.0
		);
		osg::Matrixd mViewMatrixInverse = osg::Matrixd::inverse(mViewMatrix);
		m_mView2ECEFMatrix = mWorld2ECEF;
		m_mView2ECEFMatrix.preMult(mViewMatrixInverse);
		m_mView2ECEFUniform->set(osg::Matrixf(m_mView2ECEFMatrix));
	}

	// ���ƺ���̫��Bloom����Ӱ
	if (3 >= iHie)
	{
		if (!m_pGeodeSupernovaBloom->getNodeMask())
		{
			m_pGeodeSupernovaBloom->setNodeMask(~0);
		}

		// �����۵���̫���ľ���
		osg::Vec3d vEye2Solar = vEye * fUnit;
		if (2 == iHie)
		{
			vEye2Solar = (m_vSolarPos_Hie2 - vEye) * fUnit;
		}
		else if (1 == iHie)
		{
			vEye2Solar = (m_vSolarPos_Hie1 - vEye) * fUnit;
		}
		else if (0 == iHie)
		{
			// to do
		}
		else{}

		double fEye2Solar = vEye2Solar.length();
		//��������ǵľ���������ڵ�̫��bloom�Ĵ�С
		float fBloomScale = std::exp2(-fEye2Solar * 1.5e-12);
		m_fSupernovaBloomScaleUniform->set(fBloomScale);
	}
	else
	{
		if (m_pGeodeSupernovaBloom->getNodeMask())
		{
			m_pGeodeSupernovaBloom->setNodeMask(0);
		}
	}

	if (2 == iHie)
	{
		double fDistance2Sun = (vEye - m_pStar_2_Transform->asPositionAttitudeTransform()->getPosition()).length();
		float fLight = 1 + 3*std::exp2(-fDistance2Sun * 0.1); // ����������
		m_fSupernovaLightUniform->set(fLight);
	}
	else
	{
		float fLight = std::exp2(-fDistance * fUnit / 5e22); // ��߶�˥��
		m_fSupernovaLightUniform->set(fLight);
	}

	switch (iHie)
	{
	case 0:
	{

	}
	break;
	case 1:
	{
		// ���¹�Դ����
		osg::Vec3d vStarPos = m_pStar_1_Transform->asPositionAttitudeTransform()->getPosition();
		osg::Vec3f vViewLight = mViewMatrix.preMult(vStarPos);
		vViewLight.normalize();
		m_vViewLightUniform->set(vViewLight);
	}
	break;
	case 2:
	{
		// ���¹⻷��ת����Ϊ���������͡��Ƴཻ����б����������	
		osg::Quat qInclination = osg::Quat(
			m_sCelestialBodyVector.at(m_iCenterCelestialBody).fObliquity,
			vPlanetAxisNorm);// ����Ƴཻ��
		osg::Vec3d vWorldBack = vEye - vCenter;
		vWorldBack.normalize();
		osg::Vec3d vWorldEast = m_vPlanetNorth ^ vWorldBack;
		vWorldEast.normalize();
		// �⻷����ת����billboard����֤һ����ǰһ���ں�
		double fRingAngle = acos(vPlanetAxisNorm*vWorldEast);
		if (m_vPlanetNorth.z()*vWorldEast.y() < 0)
			fRingAngle *= -1;
		osg::Quat qRingSpin = osg::Quat(fRingAngle, osg::Vec3d(0, 0, 1));
		m_pRing_2_Transform->asPositionAttitudeTransform()->setAttitude(qRingSpin*qInclination);

		// ѣ�⿿������ʱ˥��
		double fDistance2Celestial= vEye.length();
		double fMinDistance = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fEquatorRadius * 20 / fUnit;
		bool bNearCelestial = fDistance2Celestial < fMinDistance;
		if (0 == m_iCenterCelestialBody) // �Խ����Ǻ���
		{
			fMinDistance = m_sCelestialBodyVector.at(0).fEquatorRadius * 100 / fUnit;
			// ����ѣ�������
			bNearCelestial = fDistance2Celestial < fMinDistance;
			m_pSupernovaTransform->setNodeMask(bNearCelestial ? 0 : ~0);

			float fAlpha = osg::clampBetween((fDistance2Celestial - fMinDistance) / fmax(0.1f, fMinDistance), 0.0, 1.0);
			// ����ѣ���alpha,x = ����ѣ���alpha��y = ��״ѣ���alpha
			m_fSupernovaAlphaUniform->set(osg::Vec2f(fAlpha, fAlpha));
			// ���ƹ���bloom��alpha
			m_fSupernovaBloomAlphaUniform->set(fAlpha);
		}
		else // �Խ���������
		{
			osg::Vec3d vSun2EyeDir = m_pStar_2_Transform->asPositionAttitudeTransform()->getPosition() - vEye;
			double fSun2EyeLength = vSun2EyeDir.normalize();
			// �����۾���̫���������������������ĵ㣨�������ģ�����������۾��ľ��룬ע�⣺��������
			double fEye2NearPointLength = vSun2EyeDir * vEye;
			// �����۾���̫���������������������ĵ㣨�������ģ������������
			osg::Vec3d vNearPointUp = vEye - vSun2EyeDir * fEye2NearPointLength;
			double fNearLen = vNearPointUp.normalize();
			// �����������λ�õİ뾶
			double cosTheta = m_vPlanetNorth * vNearPointUp;
			double a = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fEquatorRadius / fUnit;
			double b = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fPolarRadius / fUnit;
			double fPlanetRadius = a * b / sqrt(b * b + cosTheta * cosTheta * (a * a - b * b));
			double fSunRadiusAtPlanet = (m_sCelestialBodyVector.at(0).fEquatorRadius / fUnit) * abs(fEye2NearPointLength) / fSun2EyeLength;
			// ���Ƕ�glare���ڵ�
			float fCull = osg::clampBetween((fNearLen - fPlanetRadius) / fSunRadiusAtPlanet, 0.0, 1.0);
			// glareδ���ڵ�ʱ��alpha
			float fGlareAlpha = osg::clampBetween((fDistance2Celestial - fMinDistance) / fmax(0.1f, fMinDistance), 0.0, 1.0);
			// ����ѣ���alpha,x = ����ѣ���alpha��y = ��״ѣ���alpha
			m_fSupernovaAlphaUniform->set(osg::Vec2f(fGlareAlpha*fCull, fCull));

			// ���ǶԹ���bloom���ڵ�
			float fBloomCull = osg::clampBetween((fNearLen - fPlanetRadius + fSunRadiusAtPlanet) / (2 * fSunRadiusAtPlanet), 0.0, 1.0);
			m_fSupernovaBloomAlphaUniform->set(fBloomCull);
		}

		// ���Ʊ���̫������Ӱ
		if (bNearCelestial)
		{
			if (!m_pBackgroundSunTransform->getNodeMask())
			{
				m_pBackgroundSunTransform->setNodeMask(~0);
			}

			if (0 != m_iCenterCelestialBody)
			{
				// ���Ʊ���̫����alpha�����뵭��
				float fSunAlpha = osg::clampBetween((fMinDistance - fDistance2Celestial) / (fMinDistance*0.5), 0.0, 1.0);
				m_fBackgroundSunAlphaUniform->set(fSunAlpha);
			}
			else
			{
				m_fBackgroundSunAlphaUniform->set(0.0f);
			}
		}
		else
		{
			if (m_pBackgroundSunTransform->getNodeMask())
			{
				m_pBackgroundSunTransform->setNodeMask(0);
			}
		}
		// ���ñ���̫����λ��
		osg::Vec3d vEye2Solar = m_vSolarPos_Hie2 - vEye;
		osg::Vec3d vSolarDir = vEye2Solar;
		double fEye2Solar = vSolarDir.normalize();
		m_pBackgroundSunTransform->setPosition(vSolarDir);
		// �޸�ǰ�ڵ�̫���Թ��
		double fSunRadius2 = SUN_RADIUS / fUnit;
		float fSunEdge = fEye2Solar / (SUN_BLOOM_SCALE*sqrt(max(1e-10,fEye2Solar * fEye2Solar - fSunRadius2 * fSunRadius2)));
		m_fSunEdgeUniform->set(fSunEdge);

		// ���Ƶڶ��㼶���ǹ켣����Ӱ
		unsigned int iLineMask = m_pGeodePlanetsLine_2->getNodeMask();
		float fLineVisMinDis = 3;
		if ((fDistance < fLineVisMinDis) && iLineMask)
		{
			m_pGeodePlanetsLine_2->setNodeMask(0);
		}
		else if((fDistance > fLineVisMinDis) && (0 == iLineMask))
		{
			m_pGeodePlanetsLine_2->setNodeMask(~0);
		}

		// ��������β��������
		unsigned int iTailMask = m_pPlanetTailTransform->getNodeMask();
		float fTailVisMinDis = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fPolarRadius * 10 / fUnit;
		if ((fDistance < fTailVisMinDis) && iTailMask)
		{
			m_pPlanetTailTransform->setNodeMask(0);
		}
		else if ((fDistance > fTailVisMinDis) && (0 == iTailMask))
		{
			m_pPlanetTailTransform->setNodeMask(~0);
		}
		float fTailAlpha = osg::clampBetween((float(fDistance) - fTailVisMinDis) / (9 * fTailVisMinDis), 0.0f, 1.0f);
		m_fPlanetTailAlphaUniform->set(fTailAlpha);

		// �������ǹ켣��alpha
		float fLineAlpha = osg::clampBetween((float(fDistance) - fLineVisMinDis)/(9 * fLineVisMinDis),0.0f,1.0f);
		m_fPlanetLineAlphaUniform->set(fLineAlpha);
		// �������ǵ㾫��alpha
		float fPlanetPointAlpha = 1 - exp2(-fDistance*2);
		m_fPlanetPointAlphaUniform->set(fPlanetPointAlpha);

		// ���¹�Դ����
		osg::Vec3d vStarPos = m_pStar_2_Transform->asPositionAttitudeTransform()->getPosition();
		osg::Vec3f vViewLight = mViewMatrix.preMult(vStarPos);
		vViewLight.normalize();
		m_vViewLightUniform->set(vViewLight);

		// ��Ŀ��������ǻ�ʱ���Ÿ�����Ӱ��ر���
		if ((4 < m_iCenterCelestialBody) && (9 > m_iCenterCelestialBody))
		{
			// ����Ӱ�ռ䡱Z�᣺����ǰ������
			osg::Vec3d vWorldLightDir = -vStarPos;
			vWorldLightDir.normalize(); // Z��
			// ����Ӱ�ռ䡱X�᣺�����ǵı������й�
			osg::Vec3d vWorldShadowX = m_vPlanetNorth ^ vWorldLightDir; // X��
			vWorldShadowX.normalize();
			// ����Ӱ�ռ䡱Y�᣺���Ǳ����ᡢY�ᡢZ�Ṳ��
			osg::Vec3d vWorldShadowY = vWorldLightDir ^ vWorldShadowX; // Y��
			vWorldShadowY.normalize();
			// ����Ӱ�ռ䡱ԭ�㣺��ǰ�������ĵ㣬���ڶ��㼶�ռ�ԭ��
			osg::Vec3d vWorldPlanetPos = osg::Vec3d(0, 0, 0);
			// ���¡�����ռ䡱ת����Ӱ�ռ䡱�ľ���
			// ����ġ�����ռ䡱��ָ���ڶ��㼶�ռ䡱����ϵ
			// ����ġ���Ӱ�ռ䡱��ָһ������ϵ���Ե�ǰ����Ϊԭ�㣬����ǰ������ΪZ�ᣬ���Ǳ����ᡢY�ᡢZ�Ṳ��
			m_mWorld2Shadow = osg::Matrixd(
				vWorldShadowX.x(), vWorldShadowY.x(), vWorldLightDir.x(), 0.0,
				vWorldShadowX.y(), vWorldShadowY.y(), vWorldLightDir.y(), 0.0,
				vWorldShadowX.z(), vWorldShadowY.z(), vWorldLightDir.z(), 0.0,
				-vWorldShadowX * vWorldPlanetPos, -vWorldShadowY * vWorldPlanetPos, -vWorldLightDir * vWorldPlanetPos, 1.0
			);

			// ���¡��⻷�ռ䡱ת����Ӱ�ռ䡱�ľ���
			osg::Matrixd mRing2Shadow = m_mWorld2Shadow;
			mRing2Shadow.preMult(m_pRing_2_Transform->getWorldMatrices().at(0));
			m_mRingShadowMatrixUniform->set(osg::Matrixf(mRing2Shadow));
			// ���¡����ǿռ䡱ת����Ӱ�ռ䡱�ľ���
			osg::Matrixd mPlanet2Shadow = m_mWorld2Shadow;
			mPlanet2Shadow.preMult(m_pPlanet_2_Transform->getWorldMatrices().at(0));
			m_mPlanetShadowMatrixUniform->set(osg::Matrixf(mPlanet2Shadow));

			// ������ת��������ǰ������ĵ��
			float fNorthDotLight = m_vPlanetNorth * vWorldLightDir;
			m_fNorthDotLightUniform->set(fNorthDotLight);
		}
	}
	break;
	case 3:
	{
	}
	break;
	default:
		break;
	}

	m_pEarth->UpdateLater(dDeltaTime);
	m_pOort->UpdateLater(dDeltaTime);

	return true;
}

/** @brief ���� */
bool CGMSolar::Load()
{
	std::string strGalaxyShader = m_pConfigData->strCorePath + m_strGalaxyShaderPath;

	if (m_pGeodeSun_2.valid())
	{
		CGMKit::LoadShader(m_pGeodeSun_2->getStateSet(),
			strGalaxyShader + "SunVert.glsl",
			strGalaxyShader + "SunFrag.glsl",
			"Sun");
	}
	if (m_pSunBloomTransform.valid())
	{
		CGMKit::LoadShader(m_pSunBloomTransform->getStateSet(),
			strGalaxyShader + "SunBloomVert.glsl",
			strGalaxyShader + "SunBloomFrag.glsl",
			"SunBloom");
	}
	if (m_pSSPlanetGround.valid())
	{
		CGMKit::LoadShaderWithCommonFrag(m_pSSPlanetGround,
			strGalaxyShader + "CelestialGround.vert",
			strGalaxyShader + "CelestialGround.frag",
			strGalaxyShader + "CelestialCommon.frag",
			"CelestialGround");
	}
	if (m_pSSPlanetCloud.valid())
	{
		CGMKit::LoadShaderWithCommonFrag(m_pSSPlanetCloud,
			strGalaxyShader + "CelestialCloud.vert",
			strGalaxyShader + "CelestialCloud.frag",
			strGalaxyShader + "CelestialCommon.frag",
			"CelestialCloud");
	}
	if (m_pSSPlanetAtmos.valid())
	{
		CGMKit::LoadShader(m_pSSPlanetAtmos,
			strGalaxyShader + "CelestialAtmosphere.vert",
			strGalaxyShader + "CelestialAtmosphere.frag",
			"CelestialAtmosphere");
	}
	if (m_pRingGeode_2.valid())
	{
		CGMKit::LoadShader(m_pRingGeode_2->getStateSet(),
			strGalaxyShader + "Ring.vert",
			strGalaxyShader + "Ring.frag",
			"Ring");
	}
	if (m_pPlanetTailTransform.valid())
	{
		CGMKit::LoadShader(m_pPlanetTailTransform->getStateSet(),
			strGalaxyShader + "PlanetTail.vert",
			strGalaxyShader + "PlanetTail.frag",
			"PlanetTail");
	}
	if (m_pGeodePlanets_2.valid() && m_pGeodePlanets_3.valid())
	{
		CGMKit::LoadShader(m_pGeodePlanets_2->getStateSet(),
			strGalaxyShader + "Planets_Vert.glsl", strGalaxyShader + "Planets_Frag.glsl",
			"Planets_2");
		CGMKit::LoadShader(m_pGeodePlanets_3->getStateSet(),
			strGalaxyShader + "Planets_Vert.glsl", strGalaxyShader + "Planets_Frag.glsl",
			"Planets_3");
	}
	if (m_pGeodePlanetsLine_2.valid() && m_pGeodePlanetsLine_3.valid())
	{
		CGMKit::LoadShader(m_pGeodePlanetsLine_2->getStateSet(),
			strGalaxyShader + "PlanetLineVert.glsl", strGalaxyShader + "PlanetLineFrag.glsl",
			"PlanetLine");
		CGMKit::LoadShader(m_pGeodePlanetsLine_3->getStateSet(),
			strGalaxyShader + "PlanetLineVert.glsl", strGalaxyShader + "PlanetLineFrag.glsl",
			"PlanetLine");
	}
	if (m_pGeodeAsteroid_2.valid() && m_pGeodeAsteroid_3.valid())
	{
		CGMKit::LoadShader(m_pGeodeAsteroid_2->getStateSet(),
			strGalaxyShader + "AsteroidBelt.vert", strGalaxyShader + "AsteroidBelt.frag",
			"AsteroidBelt2");
		CGMKit::LoadShader(m_pGeodeAsteroid_3->getStateSet(),
			strGalaxyShader + "AsteroidBelt.vert", strGalaxyShader + "AsteroidBelt.frag",
			"AsteroidBelt3");
	}
	if (m_pBackgroundSunTransform.valid())
	{
		CGMKit::LoadShader(m_pBackgroundSunTransform->getStateSet(),
			strGalaxyShader + "BackgroundSun.vert",
			strGalaxyShader + "BackgroundSun.frag",
			"BackgroundSun");
	}

	if (m_pGeodeSupernovaX.valid())
	{
		CGMKit::LoadShader(m_pGeodeSupernovaX->getStateSet(),
			strGalaxyShader + "SupernovaVert.glsl",
			strGalaxyShader + "SupernovaFrag.glsl",
			"Supernova");
	}
	if (m_pGeodeSupernovaBloom.valid())
	{
		CGMKit::LoadShader(m_pGeodeSupernovaBloom->getStateSet(),
			strGalaxyShader + "SupernovaBloom.vert",
			strGalaxyShader + "SupernovaBloom.frag",
			"SupernovaBloom");
	}

	if (m_pAsteroidComputeNode.valid())
	{
		CGMKit::LoadComputeShader(m_pAsteroidComputeNode->getStateSet(), strGalaxyShader + "AsteroidData.comp", "AsteroidDataCS");
	}

	m_pEarth->Load();
	m_pOort->Load();

	return true;
}

bool CGMSolar::SaveSolarData()
{
	float fTimes;
	m_pCommonUniform->GetTime()->get(fTimes);

	m_pAsteroidComputeNode->setDirty(true);

	// ����˿�̫��ϵ��Ϣ
	CGMXml aXML;
	aXML.Create(m_pConfigData->strCorePath + "Users/SolarData.cfg", "SolarSystem");
	CGMXmlNode sNode = aXML.AddChild("Jupiter");
	// ���������ǣ���λ������
	double fTrueAnomaly = m_sCelestialBodyVector.at(5).fStartTrueAnomaly
		+ fTimes * osg::PI * 2 / m_sCelestialBodyVector.at(5).fOrbitalPeriod;
	fTrueAnomaly = fmod(fTrueAnomaly, osg::PI * 2);
	sNode.SetPropDouble("trueAnomaly", fTrueAnomaly);
	aXML.Save();

	return true;
}

void CGMSolar::ResizeScreen(const int iW, const int iH)
{
	m_pEarth->ResizeScreen(iW, iH);
	m_pOort->ResizeScreen(iW, iH);
}

bool CGMSolar::CreateSolarSystem()
{
	// ����̫����ֻ��Ҫ��2���ռ䴴��
	_CreateSun();

	// ��������ϵͳ������1���ռ�
	_CreatePlanetSystem_1();
	// ��������ϵͳ������2���ռ�
	_CreatePlanetSystem_2();

	// �������ǣ�����2��3���ռ�
	_CreatePlanets();

	// 3�㼶�ռ��µ�����

	// ����������
	_CreateSupernova();

	// ����̫��
	_CreateBackgroundSun();

	// ��������
	m_pEarth->CreateEarth();

	if (EGMRENDER_LOW != m_pConfigData->eRenderQuality)
	{
		m_pOort->MakeOort();
		m_pOort->SetOortAttitude(m_qSolarRotate);
	}

	return true;
}

osg::Matrix CGMSolar::HierarchyAddMatrix() const
{
	osg::Vec3d vZeroPos = osg::Vec3d(0, 0, 0); // �������ԭ���ڽ���ǰ�ռ��е�����
	osg::Quat qRotate = osg::Quat(); // ��ת��Ԫ��

	switch (m_pKernelData->iHierarchy)
	{
	case 1:	// 0->1
	{
	}
	break;
	case 2:	// 1->2
	{
		qRotate = m_qPlanetRotate;
	}
	break;
	default:
		break;
	}

	osg::Matrix mHierarchyMatrix = osg::Matrix();
	mHierarchyMatrix.preMultTranslate(vZeroPos);
	mHierarchyMatrix.preMultRotate(qRotate);
	mHierarchyMatrix.preMultScale(osg::Vec3d(1.0, 1.0, 1.0) / GM_UNIT_SCALE);
	return mHierarchyMatrix;
}

osg::Matrix CGMSolar::HierarchySubMatrix() const
{
	osg::Vec3d vZeroPos = osg::Vec3d(0, 0, 0); // �������ԭ���ڽ���ǰ�ռ��е�����
	osg::Quat qRotate = osg::Quat(); // ��ת��Ԫ��
	switch (m_pKernelData->iHierarchy)
	{
	case 0:	// 1->0
	{
	}
	break;
	case 1:	// 2->1
	{
		qRotate = m_qPlanetRotate;
	}
	break;
	default:
		break;
	}

	osg::Matrix mHierarchyMatrix = osg::Matrix();
	mHierarchyMatrix.preMultScale(osg::Vec3d(GM_UNIT_SCALE, GM_UNIT_SCALE, GM_UNIT_SCALE));
	mHierarchyMatrix.preMultRotate(qRotate.inverse());
	mHierarchyMatrix.preMultTranslate(-vZeroPos);
	return mHierarchyMatrix;
}

bool CGMSolar::GetNearestCelestialBody(const SGMVector3& vSearchHiePos,
	SGMVector3& vPlanetHiePos, double& fOrbitalPeriod)
{
	int iHie = m_pKernelData->iHierarchy;
	if(iHie != 3) return false;
	// 3 == iHie
	vPlanetHiePos = SGMVector3(0,0,0);
	fOrbitalPeriod = m_sCelestialBodyVector.at(0).fOrbitalPeriod;

	bool bGetCelestialBody = false;
	double fNearest3 = 0.0001;
	SGMVector2 vPlanePos(vSearchHiePos.x, vSearchHiePos.y);
	double fSearchWorldRadius = vPlanePos.Length() * m_pKernelData->fUnitArray->at(iHie);
	for (auto& itr : m_sCelestialBodyVector)
	{
		float fTimes;
		m_pCommonUniform->GetTime()->get(fTimes);
		// ����Ĺ�ת�뾶
		double fHieR = itr.fOrbitalRadius / m_pKernelData->fUnitArray->at(iHie);
		// �����������
		double fTheta = itr.fStartTrueAnomaly + fTimes * osg::PI * 2 / itr.fOrbitalPeriod;
		// ��ǰ�����������λ��
		SGMVector3 vItrHiePos = SGMVector3(fHieR*cos(fTheta), fHieR*sin(fTheta), 0);
		// ��ǰ�����������ѯλ�õľ���
		double fDis = (vItrHiePos - vSearchHiePos).Length();
		if (fDis < fNearest3)
		{
			fNearest3 = fDis;
			vPlanetHiePos = vItrHiePos;
			fOrbitalPeriod = itr.fOrbitalPeriod;
			bGetCelestialBody = true;
		}
	}
	return bGetCelestialBody;
}

void CGMSolar::GetCelestialBody(SGMVector3& vPlanetPos, double & fOrbitalPeriod)
{
	float fTimes;
	m_pCommonUniform->GetTime()->get(fTimes);
	double fHieR = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fOrbitalRadius;
	// ���ǵ�������
	double fTheta = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fStartTrueAnomaly
		+ fTimes * osg::PI * 2 / m_sCelestialBodyVector.at(m_iCenterCelestialBody).fOrbitalPeriod;

	vPlanetPos = SGMVector3(fHieR*cos(fTheta), fHieR*sin(fTheta), 0);
	fOrbitalPeriod = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fOrbitalPeriod;
}

double CGMSolar::GetCelestialMeanRadius() const
{
	return (m_sCelestialBodyVector.at(m_iCenterCelestialBody).fPolarRadius + 
		m_sCelestialBodyVector.at(m_iCenterCelestialBody).fEquatorRadius) * 0.5;
}

double CGMSolar::GetCelestialRadius(const double fLatitude) const
{
	double cosTheta = cos(osg::PI_2 - osg::DegreesToRadians(abs(fLatitude)));
	double a = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fEquatorRadius;
	double b = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fPolarRadius;
	return a * b / sqrt(b * b + cosTheta * cosTheta * (a * a - b * b));
}

SGMVector3 CGMSolar::GetCelestialNorth() const
{
	if (1 == m_pKernelData->iHierarchy)
	{
		return SGMVector3(0, 0, 1);
	}
	else if (2 == m_pKernelData->iHierarchy)
	{
		return SGMVector3(m_vPlanetNorth.x(), m_vPlanetNorth.y(), m_vPlanetNorth.z());
	}

	return SGMVector3(0,0,1);
}

unsigned int CGMSolar::GetCelestialIndex() const
{
	return m_iCenterCelestialBody;
}

double CGMSolar::GetEyeAltitude() const
{
	return m_fEyeAltitude;
}

void CGMSolar::SetSupernovaHiePos(const osg::Vec3f& vHiePos)
{
	if (m_pSupernovaTransform.valid())
		m_pSupernovaTransform->setPosition(vHiePos);

	if (4 == m_pKernelData->iHierarchy)
		m_pOort->SetOortHiePos4(vHiePos);
}

void CGMSolar::SetWanderingEarthProgress(const float fProgress)
{
	m_fWanderingEarthProgress = fProgress;
	m_pEarth->SetWanderingEarthProgress(fProgress);
}

bool CGMSolar::UpdateHierarchy(int iHieNew)
{
	if (iHieNew <= 4)
	{
		// �Ƴ���һ�ռ�㼶
		if (iHieNew > 0 && GM_Root->containsNode(m_pHieSolarRootVector.at(iHieNew - 1).get()))
		{
			GM_Root->removeChild(m_pHieSolarRootVector.at(iHieNew - 1));
		}
		// �Ƴ���һ�ռ�㼶
		if (iHieNew < 3 && GM_Root->containsNode(m_pHieSolarRootVector.at(iHieNew + 1).get()))
		{
			GM_Root->removeChild(m_pHieSolarRootVector.at(iHieNew + 1));
		}
		// ��ӵ�ǰ�ռ�㼶
		if (iHieNew < 4 && !(GM_Root->containsNode(m_pHieSolarRootVector.at(iHieNew).get())))
		{
			GM_Root->addChild(m_pHieSolarRootVector.at(iHieNew));
		}
	}

	switch (iHieNew)
	{
	case 0:
	{
		float fUnit0 = m_pKernelData->fUnitArray->at(0);
		// �޸�����뾶Uniform
		m_vPlanetRadiusUniform->set(osg::Vec2f(
			m_sCelestialBodyVector.at(m_iCenterCelestialBody).fEquatorRadius / fUnit0,
			m_sCelestialBodyVector.at(m_iCenterCelestialBody).fPolarRadius / fUnit0));
		// �޸ĵ������ɽ�ĺ��θ߶�Uniform
		m_fGroundTopUniform->set(float(m_sCelestialBodyVector.at(m_iCenterCelestialBody).fGroundTop / fUnit0));
		// ������ȣ���λ����
		float fAtmosH = m_pAtmos->GetAtmosHeight(m_sCelestialBodyVector.at(m_iCenterCelestialBody).eAtmosHeight);
		m_fAtmosHeightUniform->set(float(fAtmosH / fUnit0));
	}
	break;
	case 1:
	{
		// to do m_pStar_1_Transform
		osg::Vec3d vStarHiePos = m_pStar_2_Transform->asPositionAttitudeTransform()->getPosition() * GM_UNIT_SCALE;
		if (m_pStar_1_Transform.valid())
		{
			m_pStar_1_Transform->asPositionAttitudeTransform()->setPosition(vStarHiePos);
		}

		float fUnit1 = m_pKernelData->fUnitArray->at(1);
		double fEquatorR1 = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fEquatorRadius / fUnit1;
		double fPolarR1 = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fPolarRadius / fUnit1;

		// �޸�����뾶Uniform
		m_vPlanetRadiusUniform->set(osg::Vec2f(fEquatorR1, fPolarR1));
		// �޸ĵ������ɽ�ĺ��θ߶�Uniform
		m_fGroundTopUniform->set(float(m_sCelestialBodyVector.at(m_iCenterCelestialBody).fGroundTop / fUnit1));
		// ������ȣ���λ����
		float fAtmosH = m_pAtmos->GetAtmosHeight(m_sCelestialBodyVector.at(m_iCenterCelestialBody).eAtmosHeight);
		m_fAtmosHeightUniform->set(float(fAtmosH / fUnit1));

		m_fCloudTopUniform->set(m_sCelestialBodyVector.at(m_iCenterCelestialBody).fCloudTop / fUnit1);

		// ��1�㼶�ռ��£����ǵı�����/��ת��	
		m_vPlanetNorth = osg::Vec3d(0, 0, 1);
		// ��1�㼶�ռ��£����ǵ�X��(��γ0��)
		m_vPlanetAxisX = osg::Vec3d(1, 0, 0);
	}
	break;
	case 2:
	{
		float fUnit2 = m_pKernelData->fUnitArray->at(2);

		// �޸�����뾶Uniform
		m_vPlanetRadiusUniform->set(osg::Vec2f(
			m_sCelestialBodyVector.at(m_iCenterCelestialBody).fEquatorRadius / fUnit2,
			m_sCelestialBodyVector.at(m_iCenterCelestialBody).fPolarRadius / fUnit2));
		// �޸ĵ������ɽ�ĺ��θ߶�Uniform
		m_fGroundTopUniform->set(float(m_sCelestialBodyVector.at(m_iCenterCelestialBody).fGroundTop / fUnit2));
		// ������ȣ���λ����
		float fAtmosH = m_pAtmos->GetAtmosHeight(m_sCelestialBodyVector.at(m_iCenterCelestialBody).eAtmosHeight); // ��λ����
		m_fAtmosHeightUniform->set(float(fAtmosH / fUnit2));

		m_fCloudTopUniform->set(m_sCelestialBodyVector.at(m_iCenterCelestialBody).fCloudTop / fUnit2);
	}
	break;
	case 3:
	{
		if (m_pSupernovaTransform.valid())
		{
			m_pSupernovaTransform->setPosition(osg::Vec3d(0, 0, 0));
		}
	}
	break;
	case 4:
	{
	}
	break;
	case 5:
	{
	}
	break;
	case 6:
	{
	}
	break;
	default:
		break;
	}

	m_pEarth->UpdateHierarchy(iHieNew);
	m_pOort->UpdateHierarchy(iHieNew);

	return true;
}

SGMVector3 CGMSolar::UpdateCelestialBody(const SGMVector3& vTargetHiePos)
{
	double fTimes = osg::Timer::instance()->time_s();

	// �޸�ǰĿ�������������е�˳��
	unsigned int iLastCelestialBody = m_iCenterCelestialBody;
	SGMVector3 vBodyDeltaPos = SGMVector3();
	double fUnit = m_pKernelData->fUnitArray->at(m_pKernelData->iHierarchy);
	if (3 == m_pKernelData->iHierarchy)
	{
		// Ŀ�����ǹ�ת�뾶
		double fPlanetR = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fOrbitalRadius;
		// Ŀ�����ǵ�������
		double fPlanetTheta = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fStartTrueAnomaly
			+ fTimes * osg::PI * 2 / m_sCelestialBodyVector.at(m_iCenterCelestialBody).fOrbitalPeriod;
		fPlanetTheta = fmod(fPlanetTheta, osg::PI * 2);
		// Ŀ������λ��
		SGMVector3 vPlanetPos(fPlanetR*cos(fPlanetTheta),fPlanetR*sin(fPlanetTheta),0);

		unsigned int iPlanetID = 0;
		double fNearest = (vPlanetPos - vTargetHiePos * fUnit).Length();
		for (auto& itr : m_sCelestialBodyVector)
		{
			// ���ǵ�������
			double fTheta = itr.fStartTrueAnomaly + fTimes * osg::PI * 2 / itr.fOrbitalPeriod;
			// ������3���ռ��λ��
			SGMVector3 vPos(itr.fOrbitalRadius*cos(fTheta), itr.fOrbitalRadius*sin(fTheta), 0);
			double fDistance = (vPos - vTargetHiePos * fUnit).Length();
			if (fDistance < fNearest)
			{
				fNearest = fDistance;
				// �޸ĵ�ǰ���Ǳ��
				if (iPlanetID != m_iCenterCelestialBody)
				{
					vBodyDeltaPos = vPos - vPlanetPos;
					m_iCenterCelestialBody = iPlanetID;
				}
			}
			iPlanetID++;
		}
	}
	else if (2 == m_pKernelData->iHierarchy)
	{
		const double fNearRange = 2e10;
		// ���Ŀ���Զ�뵱ǰ���򣬾�Ѱ����Ŀ������������
		if ((vTargetHiePos.Length() * fUnit) > fNearRange)
		{
			// �������ǹ�ת�뾶
			double fCenterPlanetR = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fOrbitalRadius;
			// �������ǵ�������
			double fCenterPlanetTheta = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fStartTrueAnomaly
				+ fTimes * osg::PI * 2 / m_sCelestialBodyVector.at(m_iCenterCelestialBody).fOrbitalPeriod;
			fCenterPlanetTheta = fmod(fCenterPlanetTheta, osg::PI * 2);
			// ����������2���ռ�ĺ�������ϵ�µ�λ�ã�ע�⣺����������������2���ռ��µ�λ��
			SGMVector3 vCenterPlanetPos(
				fCenterPlanetR*cos(fCenterPlanetTheta),
				fCenterPlanetR*sin(fCenterPlanetTheta),
				0);
			SGMVector3 vTargetSolarHiePos = vTargetHiePos + vCenterPlanetPos / fUnit;

			unsigned int iPlanetID = 0;
			double fHieNearest = vTargetHiePos.Length();
			for (auto& itr : m_sCelestialBodyVector)
			{
				// ���ǵ�������
				double fTheta = itr.fStartTrueAnomaly + fTimes * osg::PI * 2 / itr.fOrbitalPeriod;
				// �����ں�������ϵ�µ�λ��
				SGMVector3 vPos(itr.fOrbitalRadius*cos(fTheta), itr.fOrbitalRadius*sin(fTheta), 0);

				double fHieDistance = (vTargetSolarHiePos - vPos / fUnit).Length();
				if (fHieDistance < fHieNearest)
				{
					fHieNearest = fHieDistance;
					// �޸ĵ�ǰ���Ǳ��
					if (iPlanetID != m_iCenterCelestialBody)
					{
						vBodyDeltaPos = vPos - vCenterPlanetPos;
						m_iCenterCelestialBody = iPlanetID;
					}
				}
				iPlanetID++;
			}
		}
	}
	else
	{
		// to do
	}

	unsigned int iOnOverride = osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE;
	// �ж����������Ƿ����޸�
	if (iLastCelestialBody == m_iCenterCelestialBody)
	{
		// ���û���޸ģ���ƫ��λ������Ϊ0
		vBodyDeltaPos = SGMVector3();
	}
	else if(0 != m_iCenterCelestialBody)
	{
		float fUnit2 = m_pKernelData->fUnitArray->at(2);

		// ��ȡ��ǰ������ͼ�ĳߴ磬�����޸�UV����ϵ��
		float fBaseTexSize = m_aVenusBaseTex->getTextureWidth();
		float fCloudTexSize = m_aVenusCloudTex->getTextureWidth();

		switch (m_iCenterCelestialBody)
		{
		case 1:// ˮ��
		{
			m_pGroundRoot->setNodeMask(~0);
			m_pCloudRoot->setNodeMask(0);
			m_pAtmosRoot->setNodeMask(0);
			// ����
			m_pSSPlanetGround->setTextureAttributeAndModes(GROUND_BASE_UNIT, m_aMercuryBaseTex, iOnOverride);
			m_pSSPlanetGround->setDefine("EARTH", osg::StateAttribute::OFF);

			fBaseTexSize = m_aMercuryBaseTex->getTextureWidth();
		}
		break;
		case 2:// ����
		{
			m_pGroundRoot->setNodeMask(~0);
			m_pCloudRoot->setNodeMask(~0);
			m_pAtmosRoot->setNodeMask(~0);
			// ����
			m_pSSPlanetGround->setTextureAttributeAndModes(GROUND_BASE_UNIT, m_aVenusBaseTex, iOnOverride);
			m_pSSPlanetGround->setDefine("EARTH", osg::StateAttribute::OFF);
			// �Ʋ�
			m_pSSPlanetCloud->setTextureAttributeAndModes(CLOUD_BASE_UNIT, m_aVenusCloudTex, iOnOverride);
			m_pSSPlanetCloud->setDefine("EARTH", osg::StateAttribute::OFF);
			m_pSSPlanetCloud->setDefine("SATURN", osg::StateAttribute::OFF);
			// ����
			m_pSSPlanetAtmos->setDefine("SATURN", osg::StateAttribute::OFF);

			fBaseTexSize = m_aVenusBaseTex->getTextureWidth();
			fCloudTexSize = m_aVenusCloudTex->getTextureWidth();
		}
		break;
		case 3:// ����
		{
			m_pGroundRoot->setNodeMask(0);
			m_pCloudRoot->setNodeMask(0);
			m_pAtmosRoot->setNodeMask(0);
		}
		break;
		case 4:// ����
		{
			m_pGroundRoot->setNodeMask(~0);
			m_pCloudRoot->setNodeMask(0);
			m_pAtmosRoot->setNodeMask(~0);
			// ����
			m_pSSPlanetGround->setTextureAttributeAndModes(GROUND_BASE_UNIT, m_aMarsBaseTex, iOnOverride);
			m_pSSPlanetGround->setDefine("EARTH", osg::StateAttribute::OFF);
			// ����
			m_pSSPlanetAtmos->setDefine("SATURN", osg::StateAttribute::OFF);

			fBaseTexSize = m_aMarsBaseTex->getTextureWidth();
		}
		break;
		case 5:// ľ��
		{
			m_pGroundRoot->setNodeMask(0);
			m_pCloudRoot->setNodeMask(~0);
			m_pAtmosRoot->setNodeMask(~0);
			// �Ʋ�
			m_pSSPlanetCloud->setDefine("EARTH", osg::StateAttribute::OFF);
			m_pSSPlanetCloud->setDefine("SATURN", osg::StateAttribute::OFF);
			m_pSSPlanetCloud->setTextureAttributeAndModes(CLOUD_BASE_UNIT, m_aJupiterCloudTex, iOnOverride);
			// ����
			m_pSSPlanetAtmos->setDefine("SATURN", osg::StateAttribute::OFF);
			// �⻷
			m_pRingGeode_2->getStateSet()->setTextureAttributeAndModes(0, m_pJupiterRingTex, osg::StateAttribute::ON);

			fCloudTexSize = m_aJupiterCloudTex->getTextureWidth();
		}
		break;
		case 6:// ����
		{
			m_pGroundRoot->setNodeMask(0);
			m_pCloudRoot->setNodeMask(~0);
			m_pAtmosRoot->setNodeMask(~0);
			// �Ʋ�
			m_pSSPlanetCloud->setDefine("EARTH", osg::StateAttribute::OFF);
			m_pSSPlanetCloud->setDefine("SATURN", osg::StateAttribute::ON);
			m_pSSPlanetCloud->setTextureAttributeAndModes(CLOUD_BASE_UNIT, m_aSaturnCloudTex, iOnOverride);
			// ����
			m_pSSPlanetAtmos->setDefine("SATURN", osg::StateAttribute::ON);
			// �⻷
			m_pRingGeode_2->getStateSet()->setTextureAttributeAndModes(0, m_pSaturnRingTex, osg::StateAttribute::ON);

			fCloudTexSize = m_aSaturnCloudTex->getTextureWidth();
		}
		break;
		case 7:// ������
		{
			m_pGroundRoot->setNodeMask(0);
			m_pCloudRoot->setNodeMask(~0);
			m_pAtmosRoot->setNodeMask(~0);
			// �Ʋ�
			m_pSSPlanetCloud->setDefine("EARTH", osg::StateAttribute::OFF);
			m_pSSPlanetCloud->setDefine("SATURN", osg::StateAttribute::OFF);
			m_pSSPlanetCloud->setTextureAttributeAndModes(CLOUD_BASE_UNIT, m_aUranusCloudTex, iOnOverride);
			// ����
			m_pSSPlanetAtmos->setDefine("SATURN", osg::StateAttribute::OFF);
			// �⻷
			m_pRingGeode_2->getStateSet()->setTextureAttributeAndModes(0, m_pUranusRingTex, osg::StateAttribute::ON);

			fCloudTexSize = m_aUranusCloudTex->getTextureWidth();
		}
		break;
		case 8:// ������
		{
			m_pGroundRoot->setNodeMask(0);
			m_pCloudRoot->setNodeMask(~0);
			m_pAtmosRoot->setNodeMask(~0);
			// �Ʋ�
			m_pSSPlanetCloud->setDefine("EARTH", osg::StateAttribute::OFF);
			m_pSSPlanetCloud->setDefine("SATURN", osg::StateAttribute::OFF);
			m_pSSPlanetCloud->setTextureAttributeAndModes(CLOUD_BASE_UNIT, m_aNeptuneCloudTex, iOnOverride);
			// ����
			m_pSSPlanetAtmos->setDefine("SATURN", osg::StateAttribute::OFF);
			// �⻷
			m_pRingGeode_2->getStateSet()->setTextureAttributeAndModes(0, m_pNeptuneRingTex, osg::StateAttribute::ON);

			fCloudTexSize = m_aNeptuneCloudTex->getTextureWidth();
		}
		break;
		default:
			break;
		}

		// �޸ı���̫���ĳߴ�
		float fBackGroundSunScale = std::fmax(0.05f, std::tan(m_sCelestialBodyVector.at(0).fEquatorRadius * 60 /
			m_sCelestialBodyVector.at(m_iCenterCelestialBody).fOrbitalRadius));
		m_fBackgroundSunScaleUniform->set(fBackGroundSunScale);

		// �޸�����뾶
		double fEquatorR2 = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fEquatorRadius / fUnit2;
		double fPolarR2 = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fPolarRadius / fUnit2;
		m_pCelestialScaleVisitor->SetRadius(fEquatorR2, fPolarR2);
		// ��Ҫ��nodemask������ɺ���ܵ���accept
		m_pPlanetGeom_2->accept(*m_pCelestialScaleVisitor);

		float fUnit1 = m_pKernelData->fUnitArray->at(1);
		double fEquatorR1 = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fEquatorRadius / fUnit1;
		double fPolarR1 = m_sCelestialBodyVector.at(m_iCenterCelestialBody).fPolarRadius / fUnit1;
		m_pCelestialScaleVisitor->SetRadius(fEquatorR1, fPolarR1);
		// ��Ҫ��nodemask������ɺ���ܵ���accept

		// ������ͼ��С���޸�m_vCoordScaleUniform
		// Ϊ�˱��������α�Ե����ʱ�з죬������ÿ�������ͼ��Ե����һ�����صı������أ��͸��������غ�
		// ���Բ���ʱ��Ҫ������ͼ��С����ÿ������Ե�������Сһ������
		osg::Vec4f vCoordScale;
		m_vCoordScaleUniform->get(vCoordScale);
		// x = baseTex; y = cloudTex; z = illumTex;
		vCoordScale.x() = (fBaseTexSize - 2.0f) / fBaseTexSize;
		vCoordScale.y() = (fCloudTexSize - 2.0f) / fCloudTexSize;
		m_vCoordScaleUniform->set(vCoordScale);

		// �޸�����뾶Uniform
		m_vPlanetRadiusUniform->set(osg::Vec2f(
			m_sCelestialBodyVector.at(m_iCenterCelestialBody).fEquatorRadius / fUnit2,
			m_sCelestialBodyVector.at(m_iCenterCelestialBody).fPolarRadius / fUnit2));
		// �޸ĵ������ɽ�ĺ��θ߶�Uniform
		m_fGroundTopUniform->set(float(m_sCelestialBodyVector.at(m_iCenterCelestialBody).fGroundTop / fUnit2));
		// �޸��ƶ��߶�
		m_fCloudTopUniform->set(float(m_sCelestialBodyVector.at(m_iCenterCelestialBody).fCloudTop / fUnit2));

		//�޸�����Ĵ������
		if (m_sCelestialBodyVector.at(m_iCenterCelestialBody).eAtmosHeight != EGMAH_0)
		{
			// ���ݴ�����Ⱥ�����뾶����������š���ɢ�䡱����
			osg::Texture3D* pInscatTex = m_pAtmos->GetInscattering(
				m_sCelestialBodyVector.at(m_iCenterCelestialBody).eAtmosHeight,
				m_sCelestialBodyVector.at(m_iCenterCelestialBody).fEquatorRadius);
			m_pSSPlanetGround->setTextureAttributeAndModes(GROUND_INSCAT_UNIT, pInscatTex, iOnOverride);
			m_pSSPlanetCloud->setTextureAttributeAndModes(CLOUD_INSCAT_UNIT, pInscatTex, iOnOverride);
			m_pSSPlanetAtmos->setTextureAttributeAndModes(ATMOS_INSCAT_UNIT, pInscatTex, iOnOverride);
			// ������ȣ���λ����
			double fAtmosH = m_pAtmos->GetAtmosHeight(m_sCelestialBodyVector.at(m_iCenterCelestialBody).eAtmosHeight);
			m_fAtmosHeightUniform->set(float(fAtmosH / fUnit2));
			// ������С�й�����
			m_fMinDotULUniform->set(m_pAtmos->GetMinDotUL(fAtmosH, m_sCelestialBodyVector.at(m_iCenterCelestialBody).fPolarRadius));
			// �޸����������ɫת������
			m_mAtmosColorTransUniform->set(_ColorTransMatrix(m_sCelestialBodyVector.at(m_iCenterCelestialBody).vAtmosColor));

			m_pSSPlanetGround->setDefine("ATMOS", osg::StateAttribute::ON);
		}
		else
		{
			m_pSSPlanetGround->setDefine("ATMOS", osg::StateAttribute::OFF);
		}

		// �޸Ĺ⻷�뾶
		if (m_iCenterCelestialBody > 4)
		{
			if (0 == m_pRingGeode_2->getNodeMask())
			{
				m_pRingGeode_2->setNodeMask(~0);
			}
			_SetRingRadius(
				m_sCelestialBodyVector.at(m_iCenterCelestialBody).fRingMinRadius,
				m_sCelestialBodyVector.at(m_iCenterCelestialBody).fRingMaxRadius);
		}
		else
		{
			if (0 != m_pRingGeode_2->getNodeMask())
			{
				m_pRingGeode_2->setNodeMask(0);
			}
		}
	}
	else
	{
		// do nothing
	}

	// ����
	if (3 == m_iCenterCelestialBody)
	{
		m_pEarth->SetVisible(true);
	}
	else
	{
		m_pEarth->SetVisible(false);
	}
	return vBodyDeltaPos;
}

void CGMSolar::_CreateBackgroundSun()
{
	m_pBackgroundSunTransform = new osg::AutoTransform();
	m_pBackgroundSunTransform->setNodeMask(0);
	m_pBackgroundSunTransform->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
	m_pKernelData->pBackgroundCam->addChild(m_pBackgroundSunTransform);

	osg::ref_ptr<osg::Geode> pGeodeBackgroundSun = new osg::Geode();
	osg::Geometry* pBackgroundSunGeometry = new osg::Geometry();
	double fHalfWidth = 1.0;// ��������С
	// ����
	osg::ref_ptr<osg::Vec3Array> vertArray = new osg::Vec3Array;
	vertArray->reserve(6);
	vertArray->push_back(osg::Vec3(-fHalfWidth, -fHalfWidth, 0.0f));
	vertArray->push_back(osg::Vec3(fHalfWidth, -fHalfWidth, 0.0f));
	vertArray->push_back(osg::Vec3(fHalfWidth, fHalfWidth, 0.0f));
	vertArray->push_back(osg::Vec3(-fHalfWidth, -fHalfWidth, 0.0f));
	vertArray->push_back(osg::Vec3(fHalfWidth, fHalfWidth, 0.0f));
	vertArray->push_back(osg::Vec3(-fHalfWidth, fHalfWidth, 0.0f));
	pBackgroundSunGeometry->setVertexArray(vertArray);
	// UV
	osg::ref_ptr<osg::Vec2Array> texArray = new osg::Vec2Array;
	texArray->reserve(6);
	texArray->push_back(osg::Vec2(-1, -1));
	texArray->push_back(osg::Vec2(1, -1));
	texArray->push_back(osg::Vec2(1, 1));
	texArray->push_back(osg::Vec2(-1, -1));
	texArray->push_back(osg::Vec2(1, 1));
	texArray->push_back(osg::Vec2(-1, 1));
	pBackgroundSunGeometry->setTexCoordArray(0, texArray);
	// ����
	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 0, 1));
	pBackgroundSunGeometry->setNormalArray(normal);
	pBackgroundSunGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
	// Primitive
	pBackgroundSunGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, 6));
	pGeodeBackgroundSun->addDrawable(pBackgroundSunGeometry);
	m_pBackgroundSunTransform->addChild(pGeodeBackgroundSun);
	osg::ref_ptr<osg::StateSet> pBSS = m_pBackgroundSunTransform->getOrCreateStateSet();

	pBSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pBSS->setMode(GL_BLEND, osg::StateAttribute::ON);
	pBSS->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pBSS->setRenderBinDetails(BIN_BACKGROUND_SUN, "DepthSortedBin");

	int iUnitBloom = 0;
	std::string strGalaxyTexPath = m_pConfigData->strCorePath + m_strGalaxyTexPath;
	CGMKit::AddTexture(pBSS, _CreateTexture2D(strGalaxyTexPath + "sunNoise.jpg", 1), "sunNoiseTex", iUnitBloom++);

	pBSS->addUniform(m_pCommonUniform->GetStarColor());
	pBSS->addUniform(m_pCommonUniform->GetLevelArray());
	pBSS->addUniform(m_pCommonUniform->GetTime());
	pBSS->addUniform(m_fBackgroundSunScaleUniform);
	pBSS->addUniform(m_fBackgroundSunAlphaUniform);

	// ���shader
	std::string strShaderPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath;
	CGMKit::LoadShader(pBSS,
		strShaderPath + "BackgroundSun.vert",
		strShaderPath + "BackgroundSun.frag",
		"BackgroundSun");
}

bool CGMSolar::_CreateSupernova()
{
	m_pSupernovaTransform = new osg::AutoTransform();
	m_pSupernovaTransform->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
	m_pSupernovaTransform->setAutoScaleToScreen(true);
	m_pKernelData->pForegroundCam->addChild(m_pSupernovaTransform);

	std::string strGalaxyTexPath = m_pConfigData->strCorePath + m_strGalaxyTexPath;
	std::string strGalaxyShaderPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath;

	/*
	** ����ǰ�������ǵ�ʮ�ֽ��
	*/
	m_pGeodeSupernovaX = new osg::Geode();
	osg::Geometry* pGeometry = new osg::Geometry();

	// ����
	osg::ref_ptr<osg::Vec3Array> vertXArray = new osg::Vec3Array;
	// ��Ȧ
	float fHalfWidth = 32;
	float fHalfHeight = 32;
	vertXArray->push_back(osg::Vec3(-fHalfWidth, -fHalfHeight, 0.1f));
	vertXArray->push_back(osg::Vec3(fHalfWidth, -fHalfHeight, 0.1f));
	vertXArray->push_back(osg::Vec3(fHalfWidth, fHalfHeight, 0.1f));
	vertXArray->push_back(osg::Vec3(-fHalfWidth, -fHalfHeight, 0.1f));
	vertXArray->push_back(osg::Vec3(fHalfWidth, fHalfHeight, 0.1f));
	vertXArray->push_back(osg::Vec3(-fHalfWidth, fHalfHeight, 0.1f));
	// ������
	fHalfWidth = 100;
	fHalfHeight = 2;
	vertXArray->push_back(osg::Vec3(-fHalfWidth, -fHalfHeight, 0.0f));
	vertXArray->push_back(osg::Vec3(fHalfWidth, -fHalfHeight, 0.0f));
	vertXArray->push_back(osg::Vec3(fHalfWidth, fHalfHeight, 0.0f));
	vertXArray->push_back(osg::Vec3(-fHalfWidth, -fHalfHeight, 0.0f));
	vertXArray->push_back(osg::Vec3(fHalfWidth, fHalfHeight, 0.0f));
	vertXArray->push_back(osg::Vec3(-fHalfWidth, fHalfHeight, 0.0f));
	// ������
	fHalfWidth = 2;
	fHalfHeight = 60;
	vertXArray->push_back(osg::Vec3(fHalfWidth, -fHalfHeight, 0.0f));
	vertXArray->push_back(osg::Vec3(fHalfWidth, fHalfHeight, 0.0f));
	vertXArray->push_back(osg::Vec3(-fHalfWidth, fHalfHeight, 0.0f));
	vertXArray->push_back(osg::Vec3(fHalfWidth, -fHalfHeight, 0.0f));
	vertXArray->push_back(osg::Vec3(-fHalfWidth, fHalfHeight, 0.0f));
	vertXArray->push_back(osg::Vec3(-fHalfWidth, -fHalfHeight, 0.0f));
	pGeometry->setVertexArray(vertXArray);

	// UV������ͼƬ128*8���أ����϶ˣ���ȦͼƬ100*100���أ������棬��������10����
	osg::ref_ptr<osg::Vec2Array> texArray = new osg::Vec2Array;
	// ��Ȧ
	osg::Vec2 vLD = osg::Vec2(13.5f / 128.0f, 9.5f / 128.0f);
	float fDeltaU = 101.0f / 128.0f;
	float fDeltaV = 101.0f / 128.0f;
	texArray->push_back(vLD);
	texArray->push_back(vLD + osg::Vec2(fDeltaU, 0));
	texArray->push_back(vLD + osg::Vec2(fDeltaU, fDeltaV));
	texArray->push_back(vLD);
	texArray->push_back(vLD + osg::Vec2(fDeltaU, fDeltaV));
	texArray->push_back(vLD + osg::Vec2(0, fDeltaV));
	// ���ߣ��������ߵ�UV˳����ȫһ��
	vLD = osg::Vec2(0.0f, 120.5f / 128.0f);
	fDeltaU = 1.0f;
	fDeltaV = 7.0f / 128.0f;
	// ������
	texArray->push_back(vLD);
	texArray->push_back(vLD + osg::Vec2(fDeltaU, 0));
	texArray->push_back(vLD + osg::Vec2(fDeltaU, fDeltaV));
	texArray->push_back(vLD);
	texArray->push_back(vLD + osg::Vec2(fDeltaU, fDeltaV));
	texArray->push_back(vLD + osg::Vec2(0, fDeltaV));
	// ������
	texArray->push_back(vLD);
	texArray->push_back(vLD + osg::Vec2(fDeltaU, 0));
	texArray->push_back(vLD + osg::Vec2(fDeltaU, fDeltaV));
	texArray->push_back(vLD);
	texArray->push_back(vLD + osg::Vec2(fDeltaU, fDeltaV));
	texArray->push_back(vLD + osg::Vec2(0, fDeltaV));
	pGeometry->setTexCoordArray(0, texArray);

	// ����
	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	// ��Ȧ���ű���������С
	double fSqrt = 0.5;
	normal->push_back(osg::Vec3(-fSqrt, -fSqrt, 0));
	normal->push_back(osg::Vec3(fSqrt, -fSqrt, 0));
	normal->push_back(osg::Vec3(fSqrt, fSqrt, 0));
	normal->push_back(osg::Vec3(-fSqrt, -fSqrt, 0));
	normal->push_back(osg::Vec3(fSqrt, fSqrt, 0));
	normal->push_back(osg::Vec3(-fSqrt, fSqrt, 0));
	// ������
	normal->push_back(osg::Vec3(-1, -0, 0));
	normal->push_back(osg::Vec3(1, -0, 0));
	normal->push_back(osg::Vec3(1, 0, 0));
	normal->push_back(osg::Vec3(-1, -0, 0));
	normal->push_back(osg::Vec3(1, 0, 0));
	normal->push_back(osg::Vec3(-1, 0, 0));
	// ������
	normal->push_back(osg::Vec3(0, -1, 0));
	normal->push_back(osg::Vec3(0, 1, 0));
	normal->push_back(osg::Vec3(-0, 1, 0));
	normal->push_back(osg::Vec3(0, -1, 0));
	normal->push_back(osg::Vec3(-0, 1, 0));
	normal->push_back(osg::Vec3(-0, -1, 0));
	pGeometry->setNormalArray(normal);
	pGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	// Primitive
	pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, 18));

	pGeometry->setUseVertexBufferObjects(true);
	pGeometry->setUseDisplayList(false);
	pGeometry->setDataVariance(osg::Object::DYNAMIC);

	m_pGeodeSupernovaX->addDrawable(pGeometry);
	m_pSupernovaTransform->addChild(m_pGeodeSupernovaX);
	osg::ref_ptr<osg::StateSet> pSSS = m_pGeodeSupernovaX->getOrCreateStateSet();

	pSSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSS->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSS->setAttributeAndModes(new osg::BlendFunc(
		GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_ALPHA, GL_ONE), osg::StateAttribute::ON);
	pSSS->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pSSS->setRenderBinDetails(0, "DepthSortedBin");

	osg::ref_ptr<osg::Texture> _supernovaTex = _CreateTexture2D(strGalaxyTexPath + "supernova.tga", 4);
	int iUnit = 0;
	CGMKit::AddTexture(pSSS, _supernovaTex, "supernovaTex", iUnit++);

	pSSS->addUniform(m_pCommonUniform->GetStarColor());
	pSSS->addUniform(m_pCommonUniform->GetLevelArray());
	pSSS->addUniform(m_fSupernovaLightUniform);
	pSSS->addUniform(m_fSupernovaAlphaUniform);

	// ���shader
	CGMKit::LoadShader(pSSS,
		strGalaxyShaderPath + "SupernovaVert.glsl",
		strGalaxyShaderPath + "SupernovaFrag.glsl",
		"Supernova");

	/*
	** ����ǰ�������ǵĻԹ���
	*/
	m_pGeodeSupernovaBloom = new osg::Geode();
	m_pGeodeSupernovaBloom->setNodeMask(0);
	osg::Geometry* pBloomGeometry = new osg::Geometry();
	// ����
	float fBloomW = 2000.0f;
	osg::ref_ptr<osg::Vec3Array> vertBloomArray = new osg::Vec3Array;
	vertBloomArray->reserve(6);
	vertBloomArray->push_back(osg::Vec3(-fBloomW, -fBloomW, -0.1f));
	vertBloomArray->push_back(osg::Vec3(fBloomW, -fBloomW, -0.1f));
	vertBloomArray->push_back(osg::Vec3(fBloomW, fBloomW, -0.1f));
	vertBloomArray->push_back(osg::Vec3(-fBloomW, -fBloomW, -0.1f));
	vertBloomArray->push_back(osg::Vec3(fBloomW, fBloomW, -0.1f));
	vertBloomArray->push_back(osg::Vec3(-fBloomW, fBloomW, -0.1f));
	pBloomGeometry->setVertexArray(vertBloomArray);
	// Primitive
	pBloomGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, 6));
	pBloomGeometry->setUseVertexBufferObjects(true);
	pBloomGeometry->setUseDisplayList(false);
	pBloomGeometry->setDataVariance(osg::Object::DYNAMIC);

	m_pGeodeSupernovaBloom->addDrawable(pBloomGeometry);
	m_pSupernovaTransform->addChild(m_pGeodeSupernovaBloom);
	osg::ref_ptr<osg::StateSet> pSSSB = m_pGeodeSupernovaBloom->getOrCreateStateSet();

	pSSSB->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSSB->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSSB->setAttributeAndModes(new osg::BlendFunc(
		GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pSSSB->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pSSSB->setRenderBinDetails(0, "DepthSortedBin");
	// ���Uniform
	pSSSB->addUniform(m_fSupernovaBloomScaleUniform);
	pSSSB->addUniform(m_fSupernovaBloomAlphaUniform);
	pSSSB->addUniform(m_pCommonUniform->GetStarColor());
	// ���shader
	CGMKit::LoadShader(pSSSB,
		strGalaxyShaderPath + "SupernovaBloom.vert",
		strGalaxyShaderPath + "SupernovaBloom.frag",
		"SupernovaBloom");

	return true;
}

bool CGMSolar::_CreateSun()
{
	// ����̫������
	// ̫���뾶
	double fSunRadius2 = SUN_RADIUS / m_pKernelData->fUnitArray->at(2);
	m_pGeodeSun_2 = new osg::Geode;
	osg::ref_ptr<const osg::EllipsoidModel>	_sunModel_2 = new osg::EllipsoidModel(fSunRadius2, fSunRadius2);
	osg::ref_ptr<osg::Geometry> pDrawable_2 = m_pPlanet->MakeEllipsoidGeometry(_sunModel_2, 64, 32, 0, true, true);
	m_pGeodeSun_2->addDrawable(pDrawable_2);
	m_pStar_2_Transform->addChild(m_pGeodeSun_2);

	osg::ref_ptr<osg::StateSet>	pSS_2 = m_pGeodeSun_2->getOrCreateStateSet();
	pSS_2->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSS_2->setMode(GL_BLEND, osg::StateAttribute::OFF);
	pSS_2->setAttributeAndModes(new osg::CullFace());
	pSS_2->setRenderBinDetails(BIN_STAR_PLAYING, "DepthSortedBin");
	pSS_2->addUniform(m_pCommonUniform->GetStarColor());
	pSS_2->addUniform(m_pCommonUniform->GetLevelArray());
	pSS_2->addUniform(m_pCommonUniform->GetTime());
	int iUnit2 = 0;
	CGMKit::AddTexture(pSS_2, m_3DShapeTex, "shapeNoiseTex", iUnit2++);
	// mean color = 181,102,24 (0.710, 0.4, 0.094)
	CGMKit::AddTexture(pSS_2,
		_CreateDDSTexture(m_pConfigData->strCorePath + "Textures/Sphere/Star/Star.dds",
			osg::Texture::REPEAT, osg::Texture::CLAMP_TO_EDGE, true),
		"starTex", iUnit2++);

	// ���shader
	std::string strVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SunVert.glsl";
	std::string strFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SunFrag.glsl";
	CGMKit::LoadShader(pSS_2, strVertPath, strFragPath, "Sun");

	// ����̫���Թ�
	m_pSunBloomTransform = new osg::AutoTransform();
	m_pSunBloomTransform->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_CAMERA);
	m_pHieSolarRootVector.at(2)->addChild(m_pSunBloomTransform);

	osg::ref_ptr<osg::Geode> pGeodeSunBloom = new osg::Geode();
	osg::Geometry* pBloomGeometry = new osg::Geometry();
	double fHalfWidth = SUN_BLOOM_SCALE * fSunRadius2;
	// ����
	osg::ref_ptr<osg::Vec3Array> vertArray = new osg::Vec3Array;
	vertArray->reserve(6);
	vertArray->push_back(osg::Vec3(-fHalfWidth, -fHalfWidth, 0.0f));
	vertArray->push_back(osg::Vec3(fHalfWidth, -fHalfWidth, 0.0f));
	vertArray->push_back(osg::Vec3(fHalfWidth, fHalfWidth, 0.0f));
	vertArray->push_back(osg::Vec3(-fHalfWidth, -fHalfWidth, 0.0f));
	vertArray->push_back(osg::Vec3(fHalfWidth, fHalfWidth, 0.0f));
	vertArray->push_back(osg::Vec3(-fHalfWidth, fHalfWidth, 0.0f));
	pBloomGeometry->setVertexArray(vertArray);
	// UV
	osg::ref_ptr<osg::Vec2Array> texArray = new osg::Vec2Array;
	texArray->reserve(6);
	texArray->push_back(osg::Vec2(-1, -1));
	texArray->push_back(osg::Vec2(1, -1));
	texArray->push_back(osg::Vec2(1, 1));
	texArray->push_back(osg::Vec2(-1, -1));
	texArray->push_back(osg::Vec2(1, 1));
	texArray->push_back(osg::Vec2(-1, 1));
	pBloomGeometry->setTexCoordArray(0, texArray);
	// ����
	osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
	normal->push_back(osg::Vec3(0, 0, 1));
	pBloomGeometry->setNormalArray(normal);
	pBloomGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
	// Primitive
	pBloomGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, 6));
	pGeodeSunBloom->addDrawable(pBloomGeometry);
	m_pSunBloomTransform->addChild(pGeodeSunBloom);
	osg::ref_ptr<osg::StateSet> pSSS = m_pSunBloomTransform->getOrCreateStateSet();

	pSSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSS->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSS->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pSSS->setRenderBinDetails(BIN_SUN_BLOOM, "DepthSortedBin");

	int iUnitBloom = 0;
	std::string strGalaxyTexPath = m_pConfigData->strCorePath + m_strGalaxyTexPath;
	CGMKit::AddTexture(pSSS, _CreateTexture2D(strGalaxyTexPath + "sunNoise.jpg", 1), "sunNoiseTex", iUnitBloom++);

	pSSS->addUniform(m_pCommonUniform->GetStarColor());
	pSSS->addUniform(m_pCommonUniform->GetLevelArray());
	pSSS->addUniform(m_pCommonUniform->GetTime());
	pSSS->addUniform(m_fBackgroundSunAlphaUniform);
	pSSS->addUniform(m_fSunEdgeUniform);

	// ���shader
	std::string strBloomVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SunBloomVert.glsl";
	std::string strBloomFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "SunBloomFrag.glsl";
	return CGMKit::LoadShader(pSSS, strBloomVertPath, strBloomFragPath, "SunBloom");
}

bool CGMSolar::_CreatePlanets()
{
	m_iPlanetCount = 0;

	// step_1.0 - ���ǹ켣�߳�ʼ��
	// �����и�Ǳ�ڵ�bug��ÿ�����ǹ켣����1024�����㣬��������������64��ʱ�����������ᳬ��������ֵ
	const size_t iMaxNum = 65536;

	std::string strLineVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "PlanetLineVert.glsl";
	std::string strLineFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "PlanetLineFrag.glsl";

	osg::ref_ptr<osg::LineWidth> pLineWidth = new osg::LineWidth;
	pLineWidth->setWidth(1);

	// ��2�㼶���ǹ켣��
	m_pGeodePlanetsLine_2 = new osg::Geode();
	m_pStar_2_Transform->addChild(m_pGeodePlanetsLine_2);
	osg::ref_ptr<osg::Geometry> pPlanetLineGeom_2 = new osg::Geometry();
	pPlanetLineGeom_2->setUseVertexBufferObjects(true);
	pPlanetLineGeom_2->setUseDisplayList(false);
	pPlanetLineGeom_2->setDataVariance(osg::Object::STATIC);

	m_pPlanetLineVerts_2 = new osg::Vec3Array();
	m_pPlanetLineVerts_2->reserve(iMaxNum);
	m_pPlanetLineCoords_2 = new osg::Vec3Array;
	m_pPlanetLineCoords_2->reserve(iMaxNum);
	m_pPlanetLineElement_2 = new osg::DrawElementsUShort(GL_LINES);
	m_pPlanetLineElement_2->reserve(iMaxNum);

	osg::ref_ptr<osg::StateSet> pLineSS2 = m_pGeodePlanetsLine_2->getOrCreateStateSet();
	pLineSS2->setAttributeAndModes(pLineWidth, osg::StateAttribute::ON);
	pLineSS2->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pLineSS2->setMode(GL_BLEND, osg::StateAttribute::ON);
	pLineSS2->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pLineSS2->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pLineSS2->setRenderBinDetails(BIN_PLANET_LINE, "DepthSortedBin");
	pLineSS2->addUniform(m_pCommonUniform->GetStarColor());
	pLineSS2->addUniform(m_pCommonUniform->GetTime());
	pLineSS2->addUniform(m_fPlanetLineAlphaUniform);
	CGMKit::LoadShader(pLineSS2, strLineVertPath, strLineFragPath, "PlanetLine2");

	// ��3�㼶���ǹ켣��
	m_pGeodePlanetsLine_3 = new osg::Geode();
	m_pHieSolarRootVector.at(3)->addChild(m_pGeodePlanetsLine_3);
	osg::ref_ptr<osg::Geometry> pPlanetLineGeom_3 = new osg::Geometry();
	pPlanetLineGeom_3->setUseVertexBufferObjects(true);
	pPlanetLineGeom_3->setUseDisplayList(false);
	pPlanetLineGeom_3->setDataVariance(osg::Object::STATIC);

	m_pPlanetLineVerts_3 = new osg::Vec3Array();
	m_pPlanetLineVerts_3->reserve(iMaxNum);
	m_pPlanetLineCoords_3 = new osg::Vec3Array;
	m_pPlanetLineCoords_3->reserve(iMaxNum);
	m_pPlanetLineElement_3 = new osg::DrawElementsUShort(GL_LINES);
	m_pPlanetLineElement_3->reserve(iMaxNum);

	osg::ref_ptr<osg::StateSet> pLineSS3 = m_pGeodePlanetsLine_3->getOrCreateStateSet();
	pLineSS3->setAttributeAndModes(pLineWidth, osg::StateAttribute::ON);
	pLineSS3->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pLineSS3->setMode(GL_BLEND, osg::StateAttribute::ON);
	pLineSS3->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pLineSS3->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pLineSS3->setRenderBinDetails(BIN_PLANET_LINE, "DepthSortedBin");
	pLineSS3->addUniform(m_pCommonUniform->GetStarColor());
	pLineSS3->addUniform(m_pCommonUniform->GetTime());
	pLineSS3->addUniform(m_fPlanetLineAlphaUniform);
	CGMKit::LoadShader(pLineSS3, strLineVertPath, strLineFragPath, "PlanetLine3");

	// step_1.5 - ���ǳ�ʼ��
	// ���ǵ㾫����������
	const size_t iMaxPointNum = 65536;
	std::string strPlanetVertPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Planets_Vert.glsl";
	std::string strPlanetFragPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath + "Planets_Frag.glsl";

	// ��2�㼶����
	m_pGeodePlanets_2 = new osg::Geode();
	m_pStar_2_Transform->addChild(m_pGeodePlanets_2);
	osg::ref_ptr<osg::Geometry> pPlanetGeom_2 = new osg::Geometry();
	pPlanetGeom_2->setUseVertexBufferObjects(true);
	pPlanetGeom_2->setUseDisplayList(false);
	pPlanetGeom_2->setDataVariance(osg::Object::STATIC);

	m_pPlanetVertArray_2 = new osg::Vec4Array;
	m_pPlanetVertCoords_2 = new osg::Vec2Array;
	m_pPlanetElement_2 = new osg::DrawElementsUShort(GL_POINTS);
	m_pPlanetVertArray_2->reserve(iMaxPointNum);
	m_pPlanetVertCoords_2->reserve(iMaxPointNum);
	m_pPlanetElement_2->reserve(iMaxPointNum);

	osg::ref_ptr<osg::StateSet> pSSPlanets_2 = m_pGeodePlanets_2->getOrCreateStateSet();
	pSSPlanets_2->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
	pSSPlanets_2->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	pSSPlanets_2->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSPlanets_2->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSPlanets_2->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pSSPlanets_2->setRenderBinDetails(BIN_PLANETS, "DepthSortedBin");
	pSSPlanets_2->addUniform(m_pCommonUniform->GetStarColor());
	pSSPlanets_2->addUniform(m_pCommonUniform->GetTime());
	pSSPlanets_2->addUniform(m_pCommonUniform->GetUnit());
	pSSPlanets_2->addUniform(m_fPlanetNumUniform);
	pSSPlanets_2->addUniform(m_fPlanetPointAlphaUniform);
	CGMKit::LoadShader(pSSPlanets_2, strPlanetVertPath, strPlanetFragPath, "Planets2");

	// ��3�㼶����
	m_pGeodePlanets_3 = new osg::Geode();
	m_pHieSolarRootVector.at(3)->addChild(m_pGeodePlanets_3);
	osg::ref_ptr<osg::Geometry> pPlanetGeom_3 = new osg::Geometry();
	pPlanetGeom_3->setUseVertexBufferObjects(true);
	pPlanetGeom_3->setUseDisplayList(false);
	pPlanetGeom_3->setDataVariance(osg::Object::STATIC);

	m_pPlanetVertArray_3 = new osg::Vec4Array;
	m_pPlanetVertCoords_3 = new osg::Vec2Array;
	m_pPlanetElement_3 = new osg::DrawElementsUShort(GL_POINTS);
	m_pPlanetVertArray_3->reserve(iMaxPointNum);
	m_pPlanetVertCoords_3->reserve(iMaxPointNum);
	m_pPlanetElement_3->reserve(iMaxPointNum);

	osg::ref_ptr<osg::StateSet> pSSPlanets_3 = m_pGeodePlanets_3->getOrCreateStateSet();
	pSSPlanets_3->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
	pSSPlanets_3->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	pSSPlanets_3->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSPlanets_3->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSPlanets_3->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pSSPlanets_3->setRenderBinDetails(BIN_PLANETS, "DepthSortedBin");
	pSSPlanets_3->addUniform(m_pCommonUniform->GetStarColor());
	pSSPlanets_3->addUniform(m_pCommonUniform->GetTime());
	pSSPlanets_3->addUniform(m_pCommonUniform->GetUnit());
	pSSPlanets_3->addUniform(m_fPlanetNumUniform);
	pSSPlanets_3->addUniform(m_fPlanetPointAlphaUniform);
	CGMKit::LoadShader(pSSPlanets_3, strPlanetVertPath, strPlanetFragPath, "Planets3");

	// step_2.0 - �������
	for (auto& itr : m_sCelestialBodyVector)
	{
		_AddPlanet(itr.fOrbitalRadius, itr.fOrbitalPeriod, itr.fStartTrueAnomaly);
	}
	// ���������Ϻ�������������Uniform
	m_fPlanetNumUniform->set(float(m_iPlanetCount));

	// step_2.5 - ��ӿ���������С���Ǵ���״���ӣ����������������ӣ���Ҫ���������˶��켣��
	osg::ref_ptr <osg::Vec4dArray> pBeltArray = new osg::Vec4dArray();
	pBeltArray->reserve(1);// ����֮����Ҫ��array����Ϊ���Ժ�����������ϵʱ��Ϊ��ʵ�ֶ��С���Ǵ���Ԥ���Ľӿ�
	/* ��������
	* �ڰ뾶 = fAU*40
	* ��뾶 = fAU*55
	* ƽ���뾶���ڹ���Ĺ�ת���� = 300��
	* ����С���ǵ��� = iMaxPointNum - m_iPlanetCount */
	pBeltArray->push_back(osg::Vec4d(GM_AU * 40, GM_AU * 55, 300 * GM_YEAR, iMaxPointNum - m_iPlanetCount));
	int iBeltMax = pBeltArray->capacity(); // ���ܸ���
	std::uniform_int_distribution<> iPseudoNoise(0, 10000);
	int iAsteroidCount = 0;
	for (int i = 0; i < iBeltMax; i++)
	{
		int iAsteroidNum = osg::maximum(0, int(pBeltArray->at(i).w()));
		for (int j = (m_iPlanetCount + iAsteroidCount); j < (m_iPlanetCount + iAsteroidNum); j++)
		{
			double fRadIn = pBeltArray->at(i).x();
			double fRadOut = pBeltArray->at(i).y();

			double fRandom = iPseudoNoise(m_iRandom)*1e-4; // 0.0000-1.0000
			double fRandom1 = 0.5 * std::asin(fRandom * 2 - 1) / osg::PI + 0.5;
			double fMix = 2 * std::abs(fRandom - 0.5);
			fMix *= fMix; fMix *= fMix;
			double fRandom2 = CGMKit::Mix(fRandom1, fRandom, fMix);

			double fRadius = fRandom2 * (fRadOut - fRadIn) + fRadIn;
			double fR2 = fRadius / m_pKernelData->fUnitArray->at(2);
			double fR3 = fRadius / m_pKernelData->fUnitArray->at(3);
			double fAngle = iPseudoNoise(m_iRandom) * 1e-4 * osg::PI * 2;
			float fX = cos(fAngle);
			float fY = sin(fAngle);
			float fT = pBeltArray->at(i).z() * std::pow(fRadius * 2 / (fRadOut + fRadIn), 1.5);

			m_pPlanetVertArray_2->push_back(osg::Vec4(fR2 * fX, fR2 * fY, 0, 1));
			m_pPlanetVertCoords_2->push_back(osg::Vec2(fT, 0));
			m_pPlanetElement_2->push_back(j);
			m_pPlanetVertArray_3->push_back(osg::Vec4(fR3 * fX, fR3 * fY, 0, 1));
			m_pPlanetVertCoords_3->push_back(osg::Vec2(fT, 0));
			m_pPlanetElement_3->push_back(j);
		}
		iAsteroidCount += iAsteroidNum;
	}

	// step_3.0 - ��ӽ����󣬽����ǹ켣�߹ҵ���Ӧ�ڵ���
	pPlanetLineGeom_2->setVertexArray(m_pPlanetLineVerts_2);
	pPlanetLineGeom_2->setTexCoordArray(0, m_pPlanetLineCoords_2);
	pPlanetLineGeom_2->addPrimitiveSet(m_pPlanetLineElement_2);
	m_pGeodePlanetsLine_2->addDrawable(pPlanetLineGeom_2);

	pPlanetLineGeom_3->setVertexArray(m_pPlanetLineVerts_3);
	pPlanetLineGeom_3->setTexCoordArray(0, m_pPlanetLineCoords_3);
	pPlanetLineGeom_3->addPrimitiveSet(m_pPlanetLineElement_3);
	m_pGeodePlanetsLine_3->addDrawable(pPlanetLineGeom_3);

	// step_3.5 - ��ӽ����󣬽����ǹҵ���Ӧ�ڵ���
	pPlanetGeom_2->setVertexArray(m_pPlanetVertArray_2);
	pPlanetGeom_2->setTexCoordArray(0, m_pPlanetVertCoords_2);
	pPlanetGeom_2->addPrimitiveSet(m_pPlanetElement_2);
	m_pGeodePlanets_2->addDrawable(pPlanetGeom_2);

	pPlanetGeom_3->setVertexArray(m_pPlanetVertArray_3);
	pPlanetGeom_3->setTexCoordArray(0, m_pPlanetVertCoords_3);
	pPlanetGeom_3->addPrimitiveSet(m_pPlanetElement_3);
	m_pGeodePlanets_3->addDrawable(pPlanetGeom_3);

	return true;
}

bool CGMSolar::_AddPlanet(const double fRadius, const double fOrbitalPeriod, const double fStartTrueAnomaly)
{
	double fR2 = fRadius / m_pKernelData->fUnitArray->at(2);
	double fR3 = fRadius / m_pKernelData->fUnitArray->at(3);

	// ������ǹ켣��
	const int iAngleSegments = 512;	// �Ƕȷֶ���
	const int iVertex = iAngleSegments * 2;	// ��������
	float fAngleEach = osg::PI * 2.0 / float(iAngleSegments);
	for (int x = 0; x < iAngleSegments; x++)
	{
		for (int i = 0; i < 2; i++)
		{
			float f = x + i;
			float fX = cos(fAngleEach * f + fStartTrueAnomaly);
			float fY = sin(fAngleEach * f + fStartTrueAnomaly);

			m_pPlanetLineVerts_2->push_back(osg::Vec3(fR2*fX, fR2*fY, 0));
			m_pPlanetLineCoords_2->push_back(osg::Vec3f(f / float(iAngleSegments), fOrbitalPeriod, float(m_iPlanetCount)));
			m_pPlanetLineElement_2->push_back(m_iPlanetCount * iVertex + 2 * x + i);

			m_pPlanetLineVerts_3->push_back(osg::Vec3(fR3*fX, fR3*fY, 0));
			m_pPlanetLineCoords_3->push_back(osg::Vec3f(f / float(iAngleSegments), fOrbitalPeriod, float(m_iPlanetCount)));
			m_pPlanetLineElement_3->push_back(m_iPlanetCount * iVertex + 2 * x + i);
		}
	}

	// ������ǵ㾫��
	float fStartX = cos(fStartTrueAnomaly);
	float fStartY = sin(fStartTrueAnomaly);
	m_pPlanetVertArray_2->push_back(osg::Vec4(fR2 * fStartX, fR2 * fStartY, 0, 1));
	m_pPlanetVertCoords_2->push_back(osg::Vec2(fOrbitalPeriod, 0));
	m_pPlanetElement_2->push_back(m_iPlanetCount);
	m_pPlanetVertArray_3->push_back(osg::Vec4(fR3 * fStartX, fR3 * fStartY, 0, 1));
	m_pPlanetVertCoords_3->push_back(osg::Vec2(fOrbitalPeriod, 0));
	m_pPlanetElement_3->push_back(m_iPlanetCount);

	m_iPlanetCount++;
	return true;
}

bool CGMSolar::_CreatePlanetSystem_1()
{
	// �����������壬���߾��ȣ�����������λ�ó��ּ��˹��������
	m_pPlanetGeom_1 = m_pPlanet->MakeHexahedronSphereGeometry();
	if (!m_pPlanetGeom_1.valid()) return false;

	return true;
}

bool CGMSolar::_CreatePlanetSystem_2()
{
	m_pPlanetGeom_2 = m_pPlanet->MakeHexahedronSphereGeometry();
	if (!m_pPlanetGeom_2.valid()) return false;

	osg::Texture3D* pSaturnInscatTex = m_pAtmos->GetInscattering(
		m_sCelestialBodyVector.at(6).eAtmosHeight,
		m_sCelestialBodyVector.at(6).fEquatorRadius);

	std::string strShaderPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath;
	unsigned int iOnOverride = osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE;

	////////////////////////////////////
	// ������ʯ����
	m_pGroundRoot = new osg::Geode();
	m_pGroundRoot->addDrawable(m_pPlanetGeom_2);
	m_pPlanet_2_Transform->addChild(m_pGroundRoot);

	m_pSSPlanetGround = m_pGroundRoot->getOrCreateStateSet();
	m_pSSPlanetGround->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	m_pSSPlanetGround->setMode(GL_BLEND, osg::StateAttribute::OFF);
	m_pSSPlanetGround->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	m_pSSPlanetGround->setAttributeAndModes(new osg::CullFace());
	m_pSSPlanetGround->setRenderBinDetails(BIN_ROCKSPHERE, "DepthSortedBin");

	// ������ͼ
	m_pSSPlanetGround->setTextureAttributeAndModes(GROUND_BASE_UNIT, m_aVenusBaseTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGrundBaseUniform = new osg::Uniform("baseTex", GROUND_BASE_UNIT);
	m_pSSPlanetGround->addUniform(pGrundBaseUniform);
	// �����ϵĴ�������ɢ�䡱����
	m_pSSPlanetGround->setTextureAttributeAndModes(GROUND_INSCAT_UNIT, pSaturnInscatTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pGroundInscatteringUniform = new osg::Uniform("inscatteringTex", GROUND_INSCAT_UNIT);
	m_pSSPlanetGround->addUniform(pGroundInscatteringUniform);

	m_pSSPlanetGround->addUniform(m_pCommonUniform->GetViewUp());
	m_pSSPlanetGround->addUniform(m_vViewLightUniform);
	m_pSSPlanetGround->addUniform(m_fAtmosHeightUniform);
	m_pSSPlanetGround->addUniform(m_fEyeAltitudeUniform);
	m_pSSPlanetGround->addUniform(m_fGroundTopUniform);
	m_pSSPlanetGround->addUniform(m_vPlanetRadiusUniform);
	m_pSSPlanetGround->addUniform(m_fMinDotULUniform);
	m_pSSPlanetGround->addUniform(m_mAtmosColorTransUniform);
	m_pSSPlanetGround->addUniform(m_pCommonUniform->GetScreenSize());
	m_pSSPlanetGround->addUniform(m_vCoordScaleUniform);

	// ���shader
	CGMKit::LoadShaderWithCommonFrag(m_pSSPlanetGround,
		strShaderPath + "CelestialGround.vert",
		strShaderPath + "CelestialGround.frag",
		strShaderPath + "CelestialCommon.frag",
		"CelestialGround");

	////////////////////////////////////
	// �����Ʋ�
	m_pCloudRoot = new osg::Geode();
	m_pCloudRoot->addDrawable(m_pPlanetGeom_2);
	m_pPlanet_2_Transform->addChild(m_pCloudRoot);

	m_pSSPlanetCloud = m_pCloudRoot->getOrCreateStateSet();
	m_pSSPlanetCloud->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	m_pSSPlanetCloud->setMode(GL_BLEND, osg::StateAttribute::ON);
	m_pSSPlanetCloud->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	m_pSSPlanetCloud->setAttributeAndModes(new osg::CullFace());
	m_pSSPlanetCloud->setRenderBinDetails(BIN_CLOUD, "DepthSortedBin");
	// �Ʋ�ƫ�ƣ�����͵�����
	osg::ref_ptr<osg::PolygonOffset> pCloudPO = new osg::PolygonOffset(-1, -1);
	m_pSSPlanetCloud->setAttributeAndModes(pCloudPO, iOnOverride);
	m_pSSPlanetCloud->setDefine("ATMOS", osg::StateAttribute::ON);
	m_pSSPlanetCloud->setDefine("SATURN", osg::StateAttribute::ON);

	// ������ͼ
	m_pSSPlanetCloud->setTextureAttributeAndModes(CLOUD_BASE_UNIT, m_aVenusBaseTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pCloudBaseUniform = new osg::Uniform("cloudTex", CLOUD_BASE_UNIT);
	m_pSSPlanetCloud->addUniform(pCloudBaseUniform);
	// ���ｫ�⻷�������ó����ǵģ�����Ϊֻ�����ǹ⻷������Ҫ�Ӹ����ǣ���ʵ����Ӱ
	m_pSSPlanetCloud->setTextureAttributeAndModes(CLOUD_RING_UNIT, m_pSaturnRingTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pCloudRingUniform = new osg::Uniform("ringTex", CLOUD_RING_UNIT);
	m_pSSPlanetCloud->addUniform(pCloudRingUniform);
	// ɢ������
	m_pSSPlanetCloud->setTextureAttributeAndModes(CLOUD_INSCAT_UNIT, pSaturnInscatTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pCloudInscatteringUniform = new osg::Uniform("inscatteringTex", CLOUD_INSCAT_UNIT);
	m_pSSPlanetCloud->addUniform(pCloudInscatteringUniform);

	m_pSSPlanetCloud->addUniform(m_pCommonUniform->GetViewUp());
	m_pSSPlanetCloud->addUniform(m_vViewLightUniform);
	m_pSSPlanetCloud->addUniform(m_fCloudTopUniform);
	m_pSSPlanetCloud->addUniform(m_fGroundTopUniform);
	m_pSSPlanetCloud->addUniform(m_fEyeAltitudeUniform);
	m_pSSPlanetCloud->addUniform(m_fAtmosHeightUniform);
	m_pSSPlanetCloud->addUniform(m_mPlanetShadowMatrixUniform);
	m_pSSPlanetCloud->addUniform(m_fNorthDotLightUniform);
	m_pSSPlanetCloud->addUniform(m_vPlanetRadiusUniform);
	m_pSSPlanetCloud->addUniform(m_fMinDotULUniform);
	m_pSSPlanetCloud->addUniform(m_mAtmosColorTransUniform);
	m_pSSPlanetCloud->addUniform(m_pCommonUniform->GetScreenSize());
	m_pSSPlanetCloud->addUniform(m_vCoordScaleUniform);

	// ���shader
	CGMKit::LoadShaderWithCommonFrag(m_pSSPlanetCloud,
		strShaderPath + "CelestialCloud.vert",
		strShaderPath + "CelestialCloud.frag",
		strShaderPath + "CelestialCommon.frag",
		"CelestialCloud");

	// ���Ǵ���
	m_pAtmosRoot = new osg::Geode();
	m_pAtmosRoot->addDrawable(m_pPlanetGeom_2);
	m_pPlanet_2_Transform->addChild(m_pAtmosRoot);

	m_pSSPlanetAtmos = m_pAtmosRoot->getOrCreateStateSet();
	m_pSSPlanetAtmos->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	m_pSSPlanetAtmos->setMode(GL_BLEND, osg::StateAttribute::ON);
	m_pSSPlanetAtmos->setAttributeAndModes(new osg::BlendFunc(
		GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_ALPHA, GL_ONE), osg::StateAttribute::ON);
	m_pSSPlanetAtmos->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false)); // no zbuffer
	m_pSSPlanetAtmos->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT));
	m_pSSPlanetAtmos->setRenderBinDetails(BIN_ATMOSPHERE, "DepthSortedBin");
	m_pSSPlanetAtmos->setDefine("SATURN", osg::StateAttribute::ON);

	// ���ｫ�⻷�������ó����ǵģ�����Ϊֻ�����ǹ⻷������Ҫ�Ӹ����ǣ���ʵ����Ӱ
	m_pSSPlanetAtmos->setTextureAttributeAndModes(ATMOS_RING_UNIT, m_pSaturnRingTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pAtmosRingUniform = new osg::Uniform("ringTex", ATMOS_RING_UNIT);
	m_pSSPlanetAtmos->addUniform(pAtmosRingUniform);
	// ����ɢ�䡱��ͼ
	m_pSSPlanetAtmos->setTextureAttributeAndModes(ATMOS_INSCAT_UNIT, pSaturnInscatTex, iOnOverride);
	osg::ref_ptr<osg::Uniform> pInscatteringUniform = new osg::Uniform("inscatteringTex", ATMOS_INSCAT_UNIT);
	m_pSSPlanetAtmos->addUniform(pInscatteringUniform);

	int iAtmosUnit = 2;
	//// �Է�����ͼ
	//m_pSSPlanetAtmos->setTextureAttributeAndModes(iAtmosUnit, m_aAuroraTex, iOnOverride);
	//osg::ref_ptr<osg::Uniform> pAtmosIllumUniform = new osg::Uniform("auroraTex", iAtmosUnit);
	//m_pSSPlanetAtmos->addUniform(pAtmosIllumUniform);
	//iAtmosUnit++;

	m_pSSPlanetAtmos->addUniform(m_pCommonUniform->GetViewUp());
	m_pSSPlanetAtmos->addUniform(m_vViewLightUniform);
	m_pSSPlanetAtmos->addUniform(m_fAtmosHeightUniform);
	m_pSSPlanetAtmos->addUniform(m_fEyeAltitudeUniform);
	m_pSSPlanetAtmos->addUniform(m_vPlanetRadiusUniform);
	m_pSSPlanetAtmos->addUniform(m_fMinDotULUniform);
	m_pSSPlanetAtmos->addUniform(m_mAtmosColorTransUniform);
	m_pSSPlanetAtmos->addUniform(m_mView2ECEFUniform);
	m_pSSPlanetAtmos->addUniform(m_mPlanetShadowMatrixUniform);
	m_pSSPlanetAtmos->addUniform(m_fNorthDotLightUniform);

	// ���shader
	CGMKit::LoadShader(m_pSSPlanetAtmos,
		strShaderPath + "CelestialAtmosphere.vert",
		strShaderPath + "CelestialAtmosphere.frag",
		"CelestialAtmosphere");

	////////////////////////////////////
	// ���ǹ⻷���г�2�ȷ��Ա����͸������
	m_pRingGeode_2 = new osg::Geode();
	osg::ref_ptr <osg::Geometry> pRingGeoBack = _MakeRingGeometry(0);
	osg::ref_ptr <osg::Geometry> pRingGeoFront = _MakeRingGeometry(1);
	m_pRingGeode_2->addDrawable(pRingGeoBack);
	m_pRingGeode_2->addDrawable(pRingGeoFront);
	m_pRing_2_Transform->addChild(m_pRingGeode_2);

	osg::ref_ptr<osg::StateSet>	pSSRing = m_pRingGeode_2->getOrCreateStateSet();
	pSSRing->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSRing->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSRing->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	// ��������������Բ������ţ������͸������
	pRingGeoBack->getOrCreateStateSet()->setRenderBinDetails(BIN_RING_BACK, "DepthSortedBin");
	pRingGeoFront->getOrCreateStateSet()->setRenderBinDetails(BIN_RING_FRONT, "DepthSortedBin");

	// �����baseTex����Ԫ����Ϊ0����Ϊ��ʱ���޸�
	pSSRing->setTextureAttributeAndModes(0, m_pJupiterRingTex, osg::StateAttribute::ON);
	osg::ref_ptr<osg::Uniform> pRingUniform = new osg::Uniform("ringColorTex", 0);
	pSSRing->addUniform(pRingUniform);

	pSSRing->addUniform(m_mRingShadowMatrixUniform);
	pSSRing->addUniform(m_vViewLightUniform);
	pSSRing->addUniform(m_vPlanetRadiusUniform);

	// ���shader
	CGMKit::LoadShader(pSSRing,
		strShaderPath + "Ring.vert",
		strShaderPath + "Ring.frag",
		"Ring");

	////////////////////////////////////
	// ����β�������ڽ����۲�����ʱ��ʾ�������ǹ켣��ͬ
	if (!m_pPlanetTailTransform.valid())
	{
		m_pPlanetTailTransform = new osg::PositionAttitudeTransform();
		m_pHieSolarRootVector.at(2)->addChild(m_pPlanetTailTransform);
	}

	// �������β�����߶μ�����
	osg::ref_ptr<osg::Geometry> pTailDrawable = new osg::Geometry();
	pTailDrawable->setUseVertexBufferObjects(true);
	pTailDrawable->setUseDisplayList(false);
	pTailDrawable->setDataVariance(osg::Object::STATIC);

	const int iVertex = 512;	// ������
	const int iSegments = iVertex - 1;	// �ֶ���
	const double fAngleSeg = 0.003;// ÿ�ε�ƫת�Ƕȣ���λ������
	const double fR = GM_AU / m_pKernelData->fUnitArray->at(2);
	osg::ref_ptr<osg::Vec3Array> pTailVertArray = new osg::Vec3Array();
	osg::ref_ptr<osg::Vec2Array> pTailTexcoordArray = new osg::Vec2Array();
	osg::ref_ptr<osg::DrawElementsUShort> pTailElement = new osg::DrawElementsUShort(GL_LINE_STRIP);
	pTailVertArray->reserve(iVertex);
	pTailTexcoordArray->reserve(iVertex);
	pTailElement->reserve(iVertex);
	// ��ʼ��β��Բ��
	for (int i = 0; i < iVertex; i++)
	{
		pTailVertArray->push_back(osg::Vec3(fR*(cos(i*fAngleSeg) - 1), fR*sin(-i*fAngleSeg), 0));
		pTailTexcoordArray->push_back(osg::Vec2f(1 - float(i) / float(iVertex), 0));
		pTailElement->push_back(i);
	}
	pTailDrawable->setVertexArray(pTailVertArray);
	pTailDrawable->setTexCoordArray(0, pTailTexcoordArray);
	pTailDrawable->addPrimitiveSet(pTailElement);

	osg::ref_ptr<osg::Geode> pTailGeode = new osg::Geode();
	pTailGeode->addDrawable(pTailDrawable);
	m_pPlanetTailTransform->addChild(pTailGeode);

	osg::ref_ptr<osg::LineWidth> pLineWidth = new osg::LineWidth;
	pLineWidth->setWidth(1);
	osg::ref_ptr<osg::StateSet> pTailSS = m_pPlanetTailTransform->getOrCreateStateSet();
	pTailSS->setAttributeAndModes(pLineWidth, osg::StateAttribute::ON);
	pTailSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pTailSS->setMode(GL_BLEND, osg::StateAttribute::ON);
	pTailSS->setAttributeAndModes(new osg::BlendFunc(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE
	), osg::StateAttribute::ON);
	pTailSS->setRenderBinDetails(BIN_PLANET_TAIL, "DepthSortedBin");

	pTailSS->addUniform(m_pCommonUniform->GetStarColor());
	pTailSS->addUniform(m_fPlanetTailAlphaUniform);
	pTailSS->addUniform(m_fPlanetLineAlphaUniform);
	pTailSS->addUniform(m_vPlanetRadiusUniform);

	// ���shader
	CGMKit::LoadShader(pTailSS,
		strShaderPath + "PlanetTail.vert",
		strShaderPath + "PlanetTail.frag",
		"PlanetTail");

	return true;
}

osg::Geometry* CGMSolar::_CreateSquareGeometry(const float fWidth, const bool bCorner)
{
	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
	osg::ref_ptr<osg::Vec3Array> vertArray = new osg::Vec3Array;

	if (bCorner)
	{
		const float fSqrt2 = std::sqrt(2.0f);
		float fRadius = fWidth / fSqrt2;
		vertArray->push_back(osg::Vec3(0, 0, 0));
		vertArray->push_back(osg::Vec3(fRadius, -fRadius, 0));
		vertArray->push_back(osg::Vec3(fWidth * fSqrt2, 0, 0));
		vertArray->push_back(osg::Vec3(fRadius, fRadius, 0));
	}
	else
	{
		float fRadius = fWidth * 0.5f;
		vertArray->push_back(osg::Vec3(-fRadius, fRadius, 0));
		vertArray->push_back(osg::Vec3(-fRadius, -fRadius, 0));
		vertArray->push_back(osg::Vec3(fRadius, -fRadius, 0));
		vertArray->push_back(osg::Vec3(fRadius, fRadius, 0));
	}
	pGeometry->setVertexArray(vertArray);

	osg::ref_ptr<osg::Vec2Array> texArray = new osg::Vec2Array;
	texArray->push_back(osg::Vec2(0, 0));
	texArray->push_back(osg::Vec2(1, 0));
	texArray->push_back(osg::Vec2(1, 1));
	texArray->push_back(osg::Vec2(0, 1));
	pGeometry->setTexCoordArray(0, texArray);

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

osg::Geometry* CGMSolar::_MakeRingGeometry(unsigned int iID)
{
	if (iID >= 2) return nullptr;

	osg::Geometry* geom = new osg::Geometry();
	geom->setUseVertexBufferObjects(true);

	double fDegreeEach = 180;
	double fRadiusMin = 0.01;
	double fRadiusMax = 0.02;
	float fLonStart = iID * fDegreeEach; // degrees
	double lonSegmentSize = fDegreeEach / double(RING_SEGMENT); // degrees

	osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
	verts->reserve(RING_SEGMENT);
	m_ringVertVector.push_back(verts);

	osg::Vec2Array* texCoords = new osg::Vec2Array();
	texCoords->reserve(RING_SEGMENT);
	geom->setTexCoordArray(0, texCoords);

	osg::DrawElementsUShort* el = new osg::DrawElementsUShort(GL_TRIANGLES);
	el->reserve(RING_SEGMENT * 6);

	for (int y = 0; y < 2; ++y)
	{
		double fR = y ? fRadiusMax : fRadiusMin;
		for (int x = 0; x <= RING_SEGMENT; ++x)
		{
			double lonDegree = fLonStart + lonSegmentSize * (double)x;// �Ƕ�
			double lonRadians = osg::DegreesToRadians(lonDegree);// ����
			verts->push_back(osg::Vec3(fR*cos(lonRadians),fR*sin(lonRadians),0));

			texCoords->push_back(osg::Vec2(y, lonDegree / 360.0));

			if ((y < 1) && (x < RING_SEGMENT))
			{
				int x_plus_1 = x + 1;
				int y_plus_1 = y + 1;

				el->push_back(y*(RING_SEGMENT + 1) + x);
				el->push_back(y_plus_1*(RING_SEGMENT + 1) + x);
				el->push_back(y*(RING_SEGMENT + 1) + x_plus_1);
				el->push_back(y*(RING_SEGMENT + 1) + x_plus_1);
				el->push_back(y_plus_1*(RING_SEGMENT + 1) + x);
				el->push_back(y_plus_1*(RING_SEGMENT + 1) + x_plus_1);
			}
		}
	}

	geom->setVertexArray(verts);
	geom->addPrimitiveSet(el);

	return geom;
}

void CGMSolar::_SetRingRadius(double fRadiusMin, double fRadiusMax)
{
	double fUnit2 = m_pKernelData->fUnitArray->at(2);
	double fDegreeEach = 180;
	double lonSegmentSize = fDegreeEach / double(RING_SEGMENT); // degrees
	int iID = 0;
	for (auto& itr : m_ringVertVector)
	{
		float fLonStart = iID * fDegreeEach; // degrees
		int i = 0;
		for (int y = 0; y < 2; ++y)
		{
			double fR = (y ? fRadiusMax : fRadiusMin) / fUnit2;
			for (int x = 0; x <= RING_SEGMENT; ++x)
			{
				double lon = osg::DegreesToRadians(fLonStart + lonSegmentSize * (double)x);// ����
				itr.get()->at(i) = osg::Vec3(fR*cos(lon), fR*sin(lon), 0);
				i++;
			}
		}
		itr.get()->dirty();
		iID++;
	}
	m_pRingGeode_2->dirtyBound();
}

osg::Texture* CGMSolar::_CreateTexture2D(const std::string & fileName, const int iChannelNum) const
{
	return _CreateTexture2D(osgDB::readImageFile(fileName), iChannelNum);
}

osg::Texture* CGMSolar::_CreateTexture2D(osg::Image * pImg, const int iChannelNum) const
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

osg::Texture* CGMSolar::_CreateDDSTexture(const std::string& fileName,
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

osg::Texture2DArray* CGMSolar::_CreateDDSTex2DArray(const std::string& filePreName, bool bFlip) const
{
	osg::Image* pImg = osgDB::readImageFile(filePreName + "0.dds");
	if (!pImg) return nullptr;

	osg::ref_ptr<osg::Texture2DArray> texture = new osg::Texture2DArray;
	texture->setTextureSize(pImg->s(), pImg->t(), 6);
	for (int i = 0; i < 6; i++)
	{
		std::string fileName = filePreName + std::to_string(i) + ".dds";
		if (bFlip)
		{
			texture->setImage(i, osgDB::readImageFile(fileName, m_pDDSOptions.get()));
		}
		else
		{
			texture->setImage(i, osgDB::readImageFile(fileName));
		}
	}

	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
	texture->setBorderColor(osg::Vec4(0, 0, 0, 0));
	texture->setSourceType(GL_UNSIGNED_BYTE);
	return texture.release();
}

osg::Texture* CGMSolar::_Load3DShapeNoise() const
{
	std::string strTexturePath = m_pConfigData->strCorePath + "Textures/Volume/noiseShape.raw";
	osg::ref_ptr<osg::Image> shapeImg = osgDB::readImageFile(strTexturePath);
	shapeImg->setImage(128, 128, 128, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, shapeImg->data(), osg::Image::NO_DELETE);
	osg::Texture3D* tex3d = new osg::Texture3D;
	tex3d->setImage(shapeImg);
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

osg::Matrixf CGMSolar::_ColorTransMatrix(const osg::Vec4f& vColor) const
{
	// ���������ɫ
	osg::Vec4f vEarth(ATMOS_R, ATMOS_G, ATMOS_B, 1.0f);

	if (vColor.b() <= vColor.g() && vColor.g() <= vColor.r())// b<g<r
	{
		return osg::Matrixf(
			0, 0, vColor.b() / vEarth.r(), 0,
			0, vColor.g() / vEarth.g(), 0, 0,
			vColor.r() / vEarth.b(), 0, 0, 0,
			0, 0, 0, vColor.a() / vEarth.a());
	}
	else if (vColor.g() <= vColor.b() && vColor.b() <= vColor.r())// g<b<r
	{
		return osg::Matrixf(
			0, vColor.g() / vEarth.r(), 0, 0,
			0, 0, vColor.b() / vEarth.g(), 0,
			vColor.r() / vEarth.b(), 0, 0, 0,
			0, 0, 0, vColor.a() / vEarth.a());
	}
	else if (vColor.b() <= vColor.r() && vColor.r() <= vColor.g())// b<r<g
	{
		return osg::Matrixf(
			0, 0, vColor.b() / vEarth.r(), 0,
			vColor.r() / vEarth.g(), 0, 0, 0,
			0, vColor.g() / vEarth.b(), 0, 0,
			0, 0, 0, vColor.a() / vEarth.a());
	}
	else if (vColor.r() <= vColor.b() && vColor.b() <= vColor.g())// r<b<g
	{
		return osg::Matrixf(
			vColor.r() / vEarth.r(), 0, 0, 0,
			0, 0, vColor.b() / vEarth.g(), 0,
			0, vColor.g() / vEarth.b(), 0, 0,
			0, 0, 0, vColor.a() / vEarth.a());
	}
	else if (vColor.g() <= vColor.r() && vColor.r() <= vColor.b())// g<r<b
	{
		return osg::Matrixf(
			0, vColor.g() / vEarth.r(), 0, 0,
			vColor.r() / vEarth.g(), 0, 0, 0,
			0, 0, vColor.b() / vEarth.b(), 0,
			0, 0, 0, vColor.a() / vEarth.a());
	}
	else // r<g<b
	{
		return osg::Matrixf(
			vColor.r() / vEarth.r(), 0, 0, 0,
			0, vColor.g() / vEarth.g(), 0, 0,
			0, 0, vColor.b() / vEarth.b(), 0,
			0, 0, 0, vColor.a() / vEarth.a());
	}
}

void CGMSolar::_InitAsteroidBelt()
{
	std::string strShaderPath = m_pConfigData->strCorePath + m_strGalaxyShaderPath;
	// ����С���Ǵ���λ�ú��ٶȼ���ģ��
	int iWidth = 8192;	// С��������ͼ�Ŀ�
	int iHeight = ASTEROID_NUM / iWidth;	// С��������ͼ�ĸ�
	// ʹ�� compute shader С��������ͼ
	m_pAsteroidComputeNode = new CGMDispatchCompute(iWidth / 256, iHeight, 1);
	// ����С��������
	m_pAsteroidComputeNode->setDispatch(true);
	// ִ�ж��
	m_pAsteroidComputeNode->setOnce(false);
	// ����Ϊfalse����ʾ����Ҫ��������浽ͼƬ�ļ�
	m_pAsteroidComputeNode->setDirty(false);
	osg::ref_ptr<osg::Geode> pAsteroidCSGeode = new osg::Geode();
	pAsteroidCSGeode->setCullingActive(false);
	pAsteroidCSGeode->addDrawable(m_pAsteroidComputeNode);
	GM_Root->addChild(pAsteroidCSGeode);

	const std::string strImgPath = m_pConfigData->strCorePath + "Textures/Sphere/AsteroidData.tif";
	osg::ref_ptr<osg::Image> pLastAsteroidImage = osgDB::readImageFile(strImgPath);
	unsigned int iPixelSize = iWidth * iHeight;
	unsigned int iDataSize = iPixelSize * 4;
	// �����һ֡��Դ��ͼƬ�����ڣ��򴴽�ͼƬ
	if (!pLastAsteroidImage.valid())
	{
		pLastAsteroidImage = new osg::Image;
		float* pLastAsteroidData = new float[iDataSize];
		std::uniform_int_distribution<> iPseudoNoise(0, 1000000);
		double fRadIn = GM_AU * 2.1;
		double fRadOut = GM_AU * 5.2;
		double fG = 6.67349e-11;	// ������������
		double fMassSun = 1.98855e30;	// kg
		for (int j = 0; j < iPixelSize; j++)
		{
			//double fRadius = iPseudoNoise(m_iRandom) * 1e-6 * (fRadOut - fRadIn) + fRadIn;
			double fRadius = GM_R_JUPITER;
			double fAngle = iPseudoNoise(m_iRandom) * 1e-6 * osg::PI * 2;
			float fX = cos(fAngle);
			float fY = sin(fAngle);
			// �ʵ��ʼ�ٶ�
			float fV = sqrt(fG * fMassSun / fRadius);

			pLastAsteroidData[4 * j] = fRadius * fX;
			pLastAsteroidData[4 * j + 1] = fRadius * fY;
			pLastAsteroidData[4 * j + 2] = - fV * fY;
			pLastAsteroidData[4 * j + 3] = fV * fX;
		}
		pLastAsteroidImage->setImage(iWidth, iHeight, 1, GL_RGBA32F, GL_RGBA, GL_FLOAT, (unsigned char*)pLastAsteroidData, osg::Image::USE_NEW_DELETE);
	}
	// ����Ŀ��ͼƬ
	osg::ref_ptr<osg::Image> pTargetAsteroidImage = new osg::Image;
	float* pTargetAsteroidData = new float[iDataSize];
	for (int j = 0; j < iDataSize; j++)
	{
		pTargetAsteroidData[j] = 0;
	}
	pTargetAsteroidImage->setImage(iWidth, iHeight, 1, GL_RGBA32F, GL_RGBA, GL_FLOAT, (unsigned char*)pTargetAsteroidData, osg::Image::USE_NEW_DELETE);

	/** ����Դ����һ֡�������洢С���Ǵ���λ�ú��ٶȣ�RGBA32F��RG = λ�ã�BA = �ٶ� */
	osg::ref_ptr<osg::Texture2D> pLastAsteroidTex = new osg::Texture2D;
	pLastAsteroidTex->setImage(pLastAsteroidImage);
	pLastAsteroidTex->setName("LastAsteroidTex");
	pLastAsteroidTex->setTextureSize(iWidth, iHeight);
	pLastAsteroidTex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
	pLastAsteroidTex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
	pLastAsteroidTex->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER);
	pLastAsteroidTex->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER);
	pLastAsteroidTex->setBorderColor(osg::Vec4(0, 0, 0, 0));
	pLastAsteroidTex->setInternalFormat(GL_RGBA32F);
	pLastAsteroidTex->setSourceFormat(GL_RGBA);
	pLastAsteroidTex->setSourceType(GL_FLOAT);
	pLastAsteroidTex->setDataVariance(osg::Object::DYNAMIC);

	/** ����Ŀ�������洢С���Ǵ���λ�ú��ٶȣ�RGBA32F��RG = λ�ã�BA = �ٶ� */
	osg::ref_ptr<osg::Texture2D> pTargetAsteroidTex = new osg::Texture2D();
	pTargetAsteroidTex->setName("TargetAsteroidTex");
	pTargetAsteroidTex->setTextureSize(iWidth, iHeight);
	pTargetAsteroidTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
	pTargetAsteroidTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
	pTargetAsteroidTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
	pTargetAsteroidTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
	pTargetAsteroidTex->setBorderColor(osg::Vec4(0, 0, 0, 0));
	pTargetAsteroidTex->setInternalFormat(GL_RGBA32F);
	pTargetAsteroidTex->setSourceFormat(GL_RGBA);
	pTargetAsteroidTex->setSourceType(GL_FLOAT);
	pTargetAsteroidTex->setDataVariance(osg::Object::DYNAMIC);

	osg::ref_ptr<osg::StateSet> pAsteroidDataSS = m_pAsteroidComputeNode->getOrCreateStateSet();
	pAsteroidDataSS->addUniform(m_fStepTimeUniform);
	pAsteroidDataSS->addUniform(m_vJupiterPosUniform);
	pAsteroidDataSS->addUniform(m_pCommonUniform->GetLevelArray());

	int iUnit = 0;

	pLastAsteroidTex->bindToImageUnit(iUnit, osg::Texture::READ_ONLY);
	CGMKit::AddTexture(pAsteroidDataSS, pLastAsteroidTex, "lastAsteroidDataImg", iUnit++);
	pLastAsteroidTex->setUnRefImageDataAfterApply(false);

	pTargetAsteroidTex->bindToImageUnit(iUnit, osg::Texture::READ_WRITE);
	CGMKit::AddTexture(pAsteroidDataSS, pTargetAsteroidTex, "targetAsteroidDataImg", iUnit++);
	pTargetAsteroidTex->setUnRefImageDataAfterApply(false);

	CGMKit::LoadComputeShader(pAsteroidDataSS, strShaderPath + "AsteroidData.comp", "AsteroidDataCS");

	m_pReadAsteroidCam = new osg::Camera;
	m_pReadAsteroidCam->setName("ReadAsteroidCamera");
	m_pReadAsteroidCam->setReferenceFrame(osg::Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT);
	m_pReadAsteroidCam->setClearMask(GL_COLOR_BUFFER_BIT);
	m_pReadAsteroidCam->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
	m_pReadAsteroidCam->setViewport(0, 0, iWidth, iHeight);
	m_pReadAsteroidCam->setRenderOrder(osg::Camera::POST_RENDER);
	m_pReadAsteroidCam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	m_pReadAsteroidCam->attach(osg::Camera::COLOR_BUFFER, pTargetAsteroidImage);
	m_pReadAsteroidCam->setAllowEventFocus(false);
	m_pReadAsteroidCam->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	m_pReadAsteroidCam->setViewMatrix(osg::Matrix::identity());
	m_pReadAsteroidCam->setProjectionMatrixAsOrtho2D(0, iWidth, 0, iHeight);
	m_pReadAsteroidCam->setProjectionResizePolicy(osg::Camera::FIXED);
	GM_Root->addChild(m_pReadAsteroidCam);

	// ����buffer�Ļص�����ָ��
	m_pReadAsteroidCam->setPostDrawCallback(new SwitchTexCallback(pLastAsteroidTex, pTargetAsteroidTex.get()));
	// �����ɵ���Ϣͼд����̵Ļص�����ָ��
	m_pReadPixelFinishCallback = new CReadPixelFinishCallback(pTargetAsteroidImage);
	m_pReadPixelFinishCallback->SetPath(strImgPath);
	m_pReadAsteroidCam->setFinalDrawCallback(m_pReadPixelFinishCallback);

	osg::ref_ptr<osg::Geode> pReadAsteroidGeode = new osg::Geode();
	pReadAsteroidGeode->addDrawable(_CreateScreenTriangle(iWidth, iHeight));
	m_pReadAsteroidCam->addChild(pReadAsteroidGeode);

	osg::ref_ptr<osg::StateSet> pReadAsteroidStateset = pReadAsteroidGeode->getOrCreateStateSet();
	pReadAsteroidStateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pReadAsteroidStateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

	CGMKit::AddTexture(pReadAsteroidStateset, pTargetAsteroidTex, "inputTex", 0);
	CGMKit::LoadShader(pReadAsteroidStateset,
		strShaderPath + "ReadPixel.vert",
		strShaderPath + "ReadPixel.frag",
		"ReadPixel");

	//////////// ����С���Ǵ�����ʾģ��
	std::string strAsteroidVertPath = strShaderPath + "AsteroidBelt.vert";
	std::string strAsteroidFragPath = strShaderPath + "AsteroidBelt.frag";

	// ��2�㼶С���Ǵ�
	m_pGeodeAsteroid_2 = new osg::Geode();
	m_pStar_2_Transform->addChild(m_pGeodeAsteroid_2);
	osg::ref_ptr<osg::Geometry> pAsteroidGeom_2 = new osg::Geometry();
	pAsteroidGeom_2->setUseVertexBufferObjects(true);
	pAsteroidGeom_2->setUseDisplayList(false);
	pAsteroidGeom_2->setDataVariance(osg::Object::DYNAMIC);

	osg::ref_ptr<osg::StateSet> pSSAsteroid_2 = m_pGeodeAsteroid_2->getOrCreateStateSet();
	pSSAsteroid_2->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
	pSSAsteroid_2->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	pSSAsteroid_2->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSAsteroid_2->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSAsteroid_2->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pSSAsteroid_2->setRenderBinDetails(BIN_PLANETS, "DepthSortedBin");
	pSSAsteroid_2->addUniform(m_pCommonUniform->GetStarColor());
	pSSAsteroid_2->addUniform(m_pCommonUniform->GetUnit());
	pSSAsteroid_2->addUniform(m_fPlanetPointAlphaUniform);
	CGMKit::AddTexture(pSSAsteroid_2, pTargetAsteroidTex, "asteroidDataTex", 0);
	CGMKit::LoadShader(pSSAsteroid_2, strAsteroidVertPath, strAsteroidFragPath, "AsteroidBelt2");

	// ��3�㼶С���Ǵ�
	m_pGeodeAsteroid_3 = new osg::Geode();
	m_pHieSolarRootVector.at(3)->addChild(m_pGeodeAsteroid_3);
	osg::ref_ptr<osg::Geometry> pAsteroidGeom_3 = new osg::Geometry();
	pAsteroidGeom_3->setUseVertexBufferObjects(true);
	pAsteroidGeom_3->setUseDisplayList(false);
	pAsteroidGeom_3->setDataVariance(osg::Object::DYNAMIC);

	osg::ref_ptr<osg::StateSet> pSSAsteroid_3 = m_pGeodeAsteroid_3->getOrCreateStateSet();
	pSSAsteroid_3->setTextureAttributeAndModes(0, new osg::PointSprite(), osg::StateAttribute::ON);
	pSSAsteroid_3->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	pSSAsteroid_3->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pSSAsteroid_3->setMode(GL_BLEND, osg::StateAttribute::ON);
	pSSAsteroid_3->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false)); // no zbuffer
	pSSAsteroid_3->setRenderBinDetails(BIN_PLANETS, "DepthSortedBin");
	pSSAsteroid_3->addUniform(m_pCommonUniform->GetStarColor());
	pSSAsteroid_3->addUniform(m_pCommonUniform->GetUnit());
	pSSAsteroid_3->addUniform(m_fPlanetPointAlphaUniform);
	CGMKit::AddTexture(pSSAsteroid_3, pTargetAsteroidTex, "asteroidDataTex", 0);
	CGMKit::LoadShader(pSSAsteroid_3, strAsteroidVertPath, strAsteroidFragPath, "AsteroidBelt3");

	// 2��3�㼶С���Ǵ����ö����Ԫ������

	//!< ��2�㼶С���Ǵ���������
	osg::ref_ptr<osg::Vec4Array> pAsteroidVertArray_2 = new osg::Vec4Array;
	pAsteroidVertArray_2->reserve(ASTEROID_NUM);
	//!< ��2�㼶С���Ǵ�Ԫ��
	osg::ref_ptr<osg::DrawElementsUShort> pAsteroidElement_2 = new osg::DrawElementsUShort(GL_POINTS);
	pAsteroidElement_2->reserve(ASTEROID_NUM);

	//!< ��3�㼶С���Ǵ���������
	osg::ref_ptr<osg::Vec4Array> pAsteroidVertArray_3 = new osg::Vec4Array;
	pAsteroidVertArray_3->reserve(ASTEROID_NUM);
	//!< ��3�㼶С���Ǵ�Ԫ��
	osg::ref_ptr<osg::DrawElementsUShort> pAsteroidElement_3 = new osg::DrawElementsUShort(GL_POINTS);
	pAsteroidElement_3->reserve(ASTEROID_NUM);

	for (int j = 0; j < int(ASTEROID_NUM); j++)
	{
		double fRadius = GM_AU * 6;
		double fR2 = fRadius / m_pKernelData->fUnitArray->at(2);
		double fR3 = fRadius / m_pKernelData->fUnitArray->at(3);
		double fAngle = (j / double(ASTEROID_NUM)) * osg::PI * 2;
		float fX = cos(fAngle);
		float fY = sin(fAngle);

		pAsteroidVertArray_2->push_back(osg::Vec4(fR2 * fX, fR2 * fY, 0, 1));
		pAsteroidElement_2->push_back(j);
		pAsteroidVertArray_3->push_back(osg::Vec4(fR3 * fX, fR3 * fY, 0, 1));
		pAsteroidElement_3->push_back(j);
	}

	pAsteroidGeom_2->setVertexArray(pAsteroidVertArray_2);
	pAsteroidGeom_2->addPrimitiveSet(pAsteroidElement_2);
	m_pGeodeAsteroid_2->addDrawable(pAsteroidGeom_2);

	pAsteroidGeom_3->setVertexArray(pAsteroidVertArray_3);
	pAsteroidGeom_3->addPrimitiveSet(pAsteroidElement_3);
	m_pGeodeAsteroid_3->addDrawable(pAsteroidGeom_3);
}

void CGMSolar::_WashAsteroidBeltData()
{
	const std::string strImgPath = m_pConfigData->strCorePath + "Textures/Sphere/AsteroidData.tif";
	osg::ref_ptr<osg::Image> pAsteroidImage = osgDB::readImageFile(strImgPath);
	unsigned int iWidth = pAsteroidImage->s();
	unsigned int iHeight = pAsteroidImage->t();

	std::uniform_int_distribution<> iPseudoNoise(0, 1000000);

	// ����Ŀ��ͼƬ
	osg::ref_ptr<osg::Image> pTargetAsteroidImage = new osg::Image;
	int iDataSize = iWidth * iHeight * 4;
	float* pTargetAsteroidData = new float[iDataSize];
	for (int j = 0; j < iDataSize; j++)
	{
		pTargetAsteroidData[j] = 0;
	}
	pTargetAsteroidImage->setImage(iWidth, iHeight, 1, GL_RGBA32F, GL_RGBA, GL_FLOAT, (unsigned char*)pTargetAsteroidData, osg::Image::USE_NEW_DELETE);

	for (unsigned int i = 0; i < iWidth; i++)
	{
		for (unsigned int j = 0; j < iHeight; j++)
		{
			// ��ǰλ�õ�����
			osg::Vec4 vValueNow = pAsteroidImage->getColor(i, j);
			// ����֮ǰλ�õ��������ݣ�������ͬ�ĵ��ֹͣ����
			bool bSame = false;
			for (unsigned int _i = 0; _i <= i && !bSame; _i++)
			{
				for (unsigned int _j = 0; _j <= j && !bSame; _j++)
				{
					// �����ȣ������������
					if (_i == i && _j == j) continue;

					osg::Vec4 vValueFront = pAsteroidImage->getColor(_i, _j);
					bSame = (vValueNow == vValueFront) ? true : bSame;
				}
			}

			if (bSame)
			{
				pTargetAsteroidImage->setColor(osg::Vec4(
					vValueNow.x() + iPseudoNoise(m_iRandom),
					vValueNow.y() + iPseudoNoise(m_iRandom),
					3e6f + iPseudoNoise(m_iRandom),
					iPseudoNoise(m_iRandom)), i, j);
			}
			else
			{
				pTargetAsteroidImage->setColor(vValueNow, i, j);
			}
		}
	}

	osgDB::writeImageFile(*(pTargetAsteroidImage.get()), strImgPath);
}

osg::Geometry* CGMSolar::_CreateScreenTriangle(const int width, const int height)
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

void CGMSolar::_UpdatePlanetSpin(double dDeltaTime)
{
	int iCenterCelestialBody = 0;
	for (auto& itr : m_sCelestialBodyVector)
	{
		// �������������ת�ٶȶ�����240��(����6����תһȦ)���Ա�֤�Ӿ�Ч��
		// ��ʵ����ת�ٶ�Ӧ�ø��ݱ�ϵͳ�Ĺ�ת�ٶȱ���������
		double fDeltaSpin = dDeltaTime * (GM_YEAR_REAL / GM_YEAR) * osg::PI * 2 / itr.fSpinPeriod;
		//���˵���ģʽ����ֹͣ�Դ�
		if (m_pConfigData->bWanderingEarth && (3 == iCenterCelestialBody))
			fDeltaSpin *= 1 - osg::clampBetween((m_fWanderingEarthProgress - 0.1) * 10, 0.0, 1.0);

		itr.fSpin += fDeltaSpin;
		if (itr.fSpin > osg::PI * 2) itr.fSpin -= osg::PI * 2;

		iCenterCelestialBody++;
	}
}

osg::Quat CGMSolar::_GetPlanetSpin() const
{
	return osg::Quat(m_sCelestialBodyVector.at(m_iCenterCelestialBody).fSpin, osg::Vec3d(0, 0, 1));
}

double CGMSolar::_GetPlanetObliquityAngle(const double fTime) const
{
	//�����ǰ�����˵��򣬾ͽ���������
	if (m_pConfigData->bWanderingEarth && (3 == m_iCenterCelestialBody))
	{
		return osg::PI_2;
	}
	else
	{
		return m_sCelestialBodyVector.at(m_iCenterCelestialBody).fObliquity;
	}
}

osg::Quat CGMSolar::_GetPlanetInclination(const double fTime) const
{
	double fObliquity = _GetPlanetObliquityAngle(fTime);
	return osg::Quat(fObliquity, osg::Vec3d(1, 0, 0));
}

osg::Quat CGMSolar::_GetPlanetTurn(const double fTime) const
{
	if (m_pConfigData->bWanderingEarth && (3 == m_iCenterCelestialBody))
	{
		// �����������
		double fSunTheta = m_sCelestialBodyVector.at(3).fStartTrueAnomaly
			+ fTime * osg::PI * 2 / m_sCelestialBodyVector.at(3).fOrbitalPeriod;
		return osg::Quat(fSunTheta, osg::Vec3d(0, 0, 1));
	}
	else
	{
		return osg::Quat(0.0, osg::Vec3d(0, 0, 1));
	}
}
