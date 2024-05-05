//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMAtmosphere.cpp
/// @brief		Galaxy-Music Engine - GMAtmosphere
/// @version	1.0
/// @author		LiuTao
/// @date		2023.11.30
//////////////////////////////////////////////////////////////////////////

#include "GMAtmosphere.h"
#include "GMEngine.h"
#include "GMKit.h"
#include <osg/Texture3D>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <ppl.h>
using namespace concurrency;

using namespace GM;
/*************************************************************************
Macro Defines
*************************************************************************/

#define TRANS_ALT_NUM			(128)			// ͸����ͼ�ĸ߶Ȳ����� [0,fAtmosThick]m
#define TRANS_PITCH_NUM			(256)			// ͸����ͼ��̫������������ֵ������ [��ƽ������ֵ,1]

#define IRRA_ALT_NUM			(128)			// ���նȵĸ߶Ȳ����� [0,fAtmosThick]m
#define IRRA_UP_NUM				(128)			// ���նȵ�̫���������Ϸ���ĵ�˲����� [-1,1]

/*************************************************************************
constexpr
*************************************************************************/

constexpr int ATMOS_NUM = 4;					// ������ȷ�����
constexpr int RADIUS_NUM = 4;					// ����뾶������
constexpr double ATMOS_2_RADIUS = 0.02;			// �������ת����뾶ʱ��ת��ϵ��

/*************************************************************************
Class
*************************************************************************/

/*************************************************************************
CGMAtmosphere Methods
*************************************************************************/

/** @brief ���� */
CGMAtmosphere::CGMAtmosphere(): m_strCoreModelPath("Models/")
{
	m_iRandom.seed(0);
}

/** @brief ���� */
CGMAtmosphere::~CGMAtmosphere()
{
}

/** @brief ��ʼ�� */
bool CGMAtmosphere::Init(SGMConfigData * pConfigData)
{
	m_pConfigData = pConfigData;

	////��͸���ʡ���������
	//_MakeAtmosTransmittance();
	////�����նȡ���������
	//_MakeAtmosIrradiance();
	//����ɢ�䡱��������
	//_MakeAtmosInscattering();

	m_pInscatteringTexVector.reserve(ATMOS_NUM * RADIUS_NUM);
	for (int h = 0; h < ATMOS_NUM; h++)
	{
		int iAtmosThick = ATMOS_MIN * exp2(h);	// ������ȣ���λ��km
		std::string strAtmosH = std::to_string(iAtmosThick);
		for (int r = 0; r < RADIUS_NUM; r++)
		{
			double fSphereR = iAtmosThick * exp2(r) / ATMOS_2_RADIUS; //����뾶����λ��km
			std::string strSphereR = std::to_string(int(fSphereR));

			osg::ref_ptr <osg::Image> pImg = osgDB::readImageFile(m_pConfigData->strCorePath
				+ "Textures/Sphere/Inscattering/Inscattering_"
				+ strAtmosH + "_" + strSphereR + ".raw");
			if (!pImg.valid()) break;
			pImg->setImage(SCAT_PITCH_NUM, SCAT_LIGHT_NUM, SCAT_COS_NUM * SCAT_ALT_NUM,
				GL_RGBA32F, GL_RGBA, GL_FLOAT, pImg->data(), osg::Image::NO_DELETE);
			osg::ref_ptr<osg::Texture3D> pInscatteringTex = new osg::Texture3D;
			pInscatteringTex->setImage(pImg.get());
			pInscatteringTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
			pInscatteringTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
			pInscatteringTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
			pInscatteringTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
			pInscatteringTex->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
			pInscatteringTex->setInternalFormat(GL_RGBA32F);
			pInscatteringTex->setSourceFormat(GL_RGBA);
			pInscatteringTex->setSourceType(GL_FLOAT);

			m_pInscatteringTexVector.emplace_back(pInscatteringTex);
		}
	}
	
	return true;
}

osg::Texture3D* CGMAtmosphere::GetInscattering(const EGMAtmosHeight eAtmosH, const double& fRadius)
{
	float fAtmosH = GetAtmosHeight(eAtmosH);
	int iSphereR = RADIUS_NUM - 1;
	for (int r = 0; r < RADIUS_NUM; r++) //����뾶
	{
		double fR_exp = (fAtmosH / ATMOS_2_RADIUS) * exp2(r); //���������뾶����λ����
		if (fR_exp * 1.5 > fRadius)
		{
			iSphereR = r;
			break;
		}
	}
	int iInscatteringID = RADIUS_NUM * (int(eAtmosH) - 1) + iSphereR;

	if (iInscatteringID < m_pInscatteringTexVector.size())
		return m_pInscatteringTexVector.at(iInscatteringID).get();
	else
		return nullptr;
}

void CGMAtmosphere::_MakeAtmosTransmittance()
{
	const int iW = TRANS_PITCH_NUM;
	const int iH = TRANS_ALT_NUM;
	const int iTransmittanceBytes = iW * iH * sizeof(float) * 3;

	//int h = 3; //�������
	for (int h = 0; h < ATMOS_NUM; h++) //�������	
	{
		double fAtmosThick = ATMOS_MIN * 1e3 * exp2(h); // ������ȣ���λ����
		double fDensAtmosBottom = _GetAtmosBottomDens(fAtmosThick); // ��������������ܶ�

		//int r = 0; //����뾶
		for (int r = 0; r < RADIUS_NUM; r++) //����뾶
		{
			double fSphereR = (fAtmosThick / ATMOS_2_RADIUS) * exp2(r); //����뾶����λ����
			double fTopR = fSphereR + fAtmosThick;
			float* data = new float[iTransmittanceBytes];

			parallel_for(int(0), int(TRANS_ALT_NUM), [&](int t) // ���߳�
			//for (int t = 0; t < TRANS_ALT_NUM; t++) // ���θ߶�
			{
				// ��ƽ����Զ����
				double fHorizonDisMax = sqrt(fAtmosThick * fAtmosThick + 2 * fAtmosThick * fSphereR);
				// ���ݺ��θ߶�ƽ���ֶ�		
				double fEyeR = CGMKit::Mix(fSphereR + 1, fTopR - 1, t / double(TRANS_ALT_NUM));
				// �۾�λ�õ�
				osg::Vec2d vEyePos = osg::Vec2d(0, fEyeR);
				for (int s = 0; s < TRANS_PITCH_NUM; s++) // �Ϸ�����̫������н�����ֵ
				{
					// �����ƽ�ߵ�����ֵ
					double fSinHoriz = fSphereR / fEyeR;
					// �����ƽ�ߵ�����ֵ
					double fCosHoriz = -sqrt(max(0, 1 - fSinHoriz * fSinHoriz));
					// �Ϸ�����̫������н����ң��ڵ�ƽ������ֵ��1.0֮��ı���
					double fCosUL = CGMKit::Mix(fCosHoriz, 1.0, double(s) / double(TRANS_PITCH_NUM));
					double fSinUL = sqrt(1 - fCosUL * fCosUL);

					double fTmp = fEyeR * fSinUL;
					// vEyePos��vTopPos�ľ���
					double fLen = sqrt(fTopR * fTopR - fTmp * fTmp) - fEyeR * fCosUL;
					// �����㶥��λ�õ�
					osg::Vec2d vTopPos = osg::Vec2d(fLen * fSinUL, fEyeR + fLen * fCosUL);
					// ����ֱ����͸����
					osg::Vec3d vTransmittance = _Transmittance(fDensAtmosBottom, fSphereR, fAtmosThick, vEyePos, vTopPos);
					int iAddress = TRANS_PITCH_NUM * t + s;
					data[3 * iAddress] = float(vTransmittance.x());
					data[3 * iAddress + 1] = float(vTransmittance.y());
					data[3 * iAddress + 2] = float(vTransmittance.z());
				}
			}
			); // end parallel_for

			osg::ref_ptr<osg::Image> pAtmosTransmittanceImage = new osg::Image();
			pAtmosTransmittanceImage->setImage(iW, iH, 1, GL_RGB32F_ARB, GL_RGB, GL_FLOAT, (unsigned char*)data, osg::Image::USE_NEW_DELETE);
			std::string strTexPath = m_pConfigData->strCorePath + "Textures/Sphere/Transmittance/Transmittance_";
			osgDB::writeImageFile(*(pAtmosTransmittanceImage.get()),
				strTexPath + std::to_string(int(fAtmosThick*1e-3)) + "_" + std::to_string(int(fSphereR*1e-3)) + ".tif");
		}
	}
}

void CGMAtmosphere::_MakeAtmosIrradiance()
{
	const int iSurfaceNum = 512;	// �ر��ܲ�������
	const int iPitchNum = 256;		// ��������������������
	const int iYawNum = 32;			// ��������ƫ����������
	const double fStepUnit = 500;	// ������������λ����
	const int iW = IRRA_UP_NUM;
	const int iH = IRRA_ALT_NUM;
	const int iIrradianceBytes = iW * iH * sizeof(float) * 3;
	std::uniform_int_distribution<> iPseudoNoise(0, 9999);
	// ���赽������̫���ⵥλ���������Ϊ 1
	// Ҳ����˵�����Ϊ1������Ϊ�������(H)��Բ�����ϣ�ÿ����λ����ڷ��������ֻ�У�1/ H��

	//int h = 2; //�������
	for (int h = 0; h < ATMOS_NUM; h++) //�������
	{
		double fAtmosThick = ATMOS_MIN * 1e3 * exp2(h);				// ������ȣ���λ����
		double fDensAtmosBottom = _GetAtmosBottomDens(fAtmosThick);		// �����������ܶ�

		//for (int r = 1; r < 2; r++) //����뾶
		for (int r = 0; r < RADIUS_NUM; r++) //����뾶
		{
			float* data = new float[iIrradianceBytes];
			double fSphereR = (fAtmosThick / ATMOS_2_RADIUS) * exp2(r); //����뾶����λ����
			double fTopR = fSphereR + fAtmosThick;

			osg::ref_ptr<osg::Image> pTransImg = osgDB::readImageFile(
				m_pConfigData->strCorePath + "Textures/Sphere/Transmittance/Transmittance_"
				+ std::to_string(int(fAtmosThick*1e-3)) + "_" + std::to_string(int(fSphereR*1e-3)) + ".tif");
			if (!pTransImg.valid()) break;

			parallel_for(int(0), int(IRRA_UP_NUM), [&](int s) // ���߳�
			//for (int s = 0; s < IRRA_UP_NUM; s++) // �Ϸ�����̫������н�����ֵ
			{
				double fCosUL = 2 * double(s) / double(IRRA_UP_NUM) - 1;
				// ̫������
				osg::Vec3d vSun = osg::Vec3d(0, sqrt(1 - fCosUL* fCosUL), fCosUL);
				for (int t = 0; t < IRRA_ALT_NUM; t++) // ���θ߶�
				{
					//// ��ƽ����Զ����
					//double fHorizonDisMax = sqrt(fAtmosThick * fAtmosThick + 2 * fAtmosThick * fSphereR);
					// ���ݺ��θ߶�ƽ���ֶ�		
					double fEyeR = CGMKit::Mix(fSphereR + 1, fTopR - 1, t / double(IRRA_ALT_NUM));
					// �����۵㿴���ĵ�ƽ�ߵ�����ֵ
					double fSinHoriz = fSphereR / fEyeR;
					// �����۵㿴���ĵ�ƽ�ߵ�����ֵ
					double fCosHoriz = -sqrt(max(0, 1 - fSinHoriz * fSinHoriz));
					// �۵㵽��ƽ�ߵ��������·���ļн�
					double fHorizonAngle = std::asin(fSinHoriz);
					// �۵㿴���ĵ�����������
					double fGroundS = osg::PI * 2 * fSphereR * fSphereR * (1 - fSinHoriz);
					// �۵�λ��
					osg::Vec3d vEyePos = osg::Vec3d(0, 0, fEyeR);

					osg::Vec3f vAlbedo(0, 0, 0);
//#define SURFACE_ALBEDO 0.3
#ifdef SURFACE_ALBEDO
					// ��������		������(%)
					// ˮ��			6~8
					// ��Ҷ��		13~15
					// �ݵ�			10~18
					// ˮ����		12~18
					// ��ľ			16~18
					// ��Ұ			15~20
					// ��ԭ			20~25
					// ɳĮ			25~30
					// ѩ��			> 50
					// ��֪����ȡ�ĸ������ʵ�ֵ
					for (int j = 0; j < iSurfaceNum; j++)
					{
						float fRandomX = iPseudoNoise(m_iRandom) * 1e-4f;	// 0.0-1.0
						float fRandomY = iPseudoNoise(m_iRandom) * 1e-4f;	// 0.0-1.0
						double fAngle = fRandomX * fHorizonAngle;
						double fSinAngle = sin(fAngle);
						osg::Vec3d vDir(
							fSinAngle * cos(fRandomY * 2 * osg::PI),
							fSinAngle * sin(fRandomY * 2 * osg::PI),
							-cos(fAngle));
						// ������������۵��Ϸ��������ֵ >0
						double fCosAngle = sqrt(1 - fSinAngle * fSinAngle);
						double fTmp = fEyeR * fSinAngle;
						// �۵㵽���潹��ľ���
						double fLen = fEyeR * fCosAngle - sqrt(fSphereR * fSphereR - fTmp * fTmp);

						// ��������
						osg::Vec3d vGroundPos = vEyePos + vDir * fLen;
						// ���淨��
						osg::Vec3d vGroundNorm = vGroundPos;
						vGroundNorm.normalize();
						// ̫����������淨�ߵ�����ֵ
						double fCosNorm2Sun = vGroundNorm * vSun;
						if (fCosNorm2Sun > 0)
						{
							// ����������䣬���ȡ�ر���������ǿ��
							osg::Vec4 vD = CGMKit::GetImageColor(pTransImg.get(),
								fCosNorm2Sun,
								0.0f,
								true);
							vD *= fCosNorm2Sun * (1 - fCosAngle);

							// �۵㴦����������߷���˥��
							osg::Vec4 vEyeT = CGMKit::GetImageColor(pTransImg.get(),
								(fCosAngle - fCosHoriz) / (1 - fCosHoriz),
								float(t) / IRRA_ALT_NUM,
								true);

							// ������ⷽ��
							osg::Vec3d vDiffuseDir = osg::Vec3d(0, fSinAngle, fCosAngle);
							// �������������淨�ߵ�����ֵ
							double fCosDiffuse2Norm = vDiffuseDir * vGroundNorm;
							// �ر����������߷���˥��
							osg::Vec4 vGroundT = CGMKit::GetImageColor(pTransImg.get(),
								fCosDiffuse2Norm,
								0.0f,
								true);

							// ������⻹�ᱻ����������
							vD.x() *= vGroundT.x() / max(1e-20, vEyeT.x());
							vD.y() *= vGroundT.y() / max(1e-20, vEyeT.y());
							vD.z() *= vGroundT.z() / max(1e-20, vEyeT.z());
							vAlbedo += osg::Vec3(vD.x(), vD.y(), vD.z()) * abs(-vDir * vGroundNorm) / (fLen * fLen);
						}
					}
					vAlbedo *= SURFACE_ALBEDO * 2e-4 * fGroundS / iSurfaceNum;
#endif // SURFACE_ALBEDO

					osg::Vec3d vIrradiance(0, 0, 0);
					for (int iX = 0; iX < iPitchNum; iX++)
					{
						// ���Ϸ����롰ɢ��Դ���򡱵ļн�
						double fCosUV = 2.0 * (iX + 0.5) / (double)iPitchNum - 1.0;
						double fSinUV = std::sqrt(1 - fCosUV * fCosUV);

						for (int iY = 0; iY < iYawNum; iY++)
						{
							double fYaw = 2.0 * osg::PI * (iY + (double)iX / (double)iPitchNum) / (double)iYawNum;
							// ɢ��Դ����
							osg::Vec3d vOffsetDir = osg::Vec3d(fSinUV * cos(fYaw), fSinUV * sin(fYaw), fCosUV);
							double fIrraDis = vOffsetDir.normalize();
							// ɢ��ⷽ��
							osg::Vec3d vIrraDir = -vOffsetDir;

							double fTmp = fEyeR * fSinUV;
							// vEyePos��vTopPos�ľ���
							double fLenET = sqrt(fTopR * fTopR - fTmp * fTmp) - fEyeR * fCosUV;
							// ����ɢ��Դ������Զ����
							double fLenMax = fLenET;
							// ���ɢ��Դ��������ֵС�ڵ�ƽ������ֵ������Զ��Ϊ����
							if (fCosUV < fCosHoriz)
							{
								// vEyePos��vGroundPos�ľ���	
								double fLenEG = -fEyeR * fCosUV - sqrt(fSphereR * fSphereR - fTmp * fTmp);
								fLenMax = fLenEG;
							}

							fLenMax = min(5e3, fLenMax);
							double fSampleNum = fLenMax / fStepUnit;
							for (int c = 0; c < int(fSampleNum + 1); c++)
							{
								// ע�⣺����Ĳ���Ϊ�˱����ݣ�������΢С�ĵ���
								double fLenS = fStepUnit * (c + fmod(fSampleNum, 1));
								// ɢ�������
								osg::Vec3d vIrraPos = vEyePos + vOffsetDir * fLenS;
								osg::Vec3d vIrraUp = vIrraPos;
								double fIrraR = vIrraUp.normalize();
								double fIrraAlt = fIrraR - fSphereR;
								double fIrraAltCoord = fIrraAlt / fAtmosThick;
								// ɢ��㿴���ĵ�ƽ�ߵ�����ֵ
								double fSinHoriz_Source = fSphereR / fIrraR;
								// ɢ��㿴���ĵ�ƽ�ߵ�����ֵ
								double fCosHoriz_Source = -sqrt(max(0, 1 - fSinHoriz_Source * fSinHoriz_Source));
								// ���۵���Χ�������̫��ֱ���
								osg::Vec4d vSunLight = CGMKit::GetImageColor(pTransImg.get(),
									(vIrraUp * vSun - fCosHoriz_Source) / (1 - fCosHoriz_Source),
									fIrraAltCoord,
									true);

								double fCosIL = vIrraDir * vSun;
								// ���۵���Χ��ɢ���
								osg::Vec3d vScattering =
									(_RayleighCoefficient(fIrraAlt, fAtmosThick) * _RayleighPhase(fCosIL)
										+ _MieCoefficient(fIrraAlt, fAtmosThick) * _MiePhase(fCosIL));
								osg::Vec3d vI = osg::Vec3(
									vSunLight.r() * vScattering.x(),
									vSunLight.g() * vScattering.y(),
									vSunLight.b() * vScattering.z());

								// �۵���յ���ɢ��ⷽ�������
								double fIrraCos_Eye = vIrraDir.z();
								// ɢ����ɢ��ⷽ�������
								double fIrraCos_Source = vIrraUp * vIrraDir;
								// "fIrraCos_Eye < fCosHoriz" �� "fIrraCos_Source < fCosHoriz_Source"
								// ������������Ȼͬʱ�����ͬʱ������
								if (fIrraCos_Eye >= fCosHoriz) // ɢ�������������룬�۾�����
								{
									// �۵��ɢ��ⷽ��˥��
									osg::Vec4d vEyeT = CGMKit::GetImageColor(pTransImg.get(),
										max(0, fIrraCos_Eye - fCosHoriz) / (1 - fCosHoriz),
										float(t) / IRRA_ALT_NUM,
										true);
									// ɢ����ɢ��ⷽ��˥��
									osg::Vec4d vIrraT = CGMKit::GetImageColor(pTransImg.get(),
										max(0, fIrraCos_Source - fCosHoriz_Source) / (1 - fCosHoriz_Source),
										fIrraAltCoord,
										true);
									// ����ɢ��Ĺ⴫�����۵㣬�ᱻ����������
									// �۾�λ�õ�͸����С����Ϊ����
									vI.x() *= vEyeT.x() / max(1e-20, vIrraT.x());
									vI.y() *= vEyeT.y() / max(1e-20, vIrraT.y());
									vI.z() *= vEyeT.z() / max(1e-20, vIrraT.z());
								}
								else // fIrraCos_Eye < fCosHoriz  // ɢ�������������룬�۾�����
								{
									// �۵�λ�õ�ɢ��⡰����˥��
									osg::Vec4d vEyeT = CGMKit::GetImageColor(pTransImg.get(),
										max(0, -fIrraCos_Eye - fCosHoriz) / (1 - fCosHoriz),
										float(t) / IRRA_ALT_NUM,
										true);
									// ɢ����ɢ��⡰����˥��
									osg::Vec4d vIrraT = CGMKit::GetImageColor(pTransImg.get(),
										max(0, -fIrraCos_Source - fCosHoriz_Source) / (1 - fCosHoriz_Source),
										fIrraAltCoord,
										true);
									// ����ɢ��Ĺ⴫�����۵㣬�ᱻ����������
									// �۾�λ�õ�͸���ʴ���Ϊ��ĸ
									vI.x() *= vIrraT.x() / max(1e-20, vEyeT.x());
									vI.y() *= vIrraT.y() / max(1e-20, vEyeT.y());
									vI.z() *= vIrraT.z() / max(1e-20, vEyeT.z());
								}
								// ������
								double fAA = fSampleNum / int(fSampleNum + 1);
								vIrradiance += vI * fAA;
							}
						}
					}
					vIrradiance *= 2e6 / double(iPitchNum * iYawNum);

					osg::Vec3d vSumColor = (vAlbedo + vIrradiance) * fmin(1.0, fDensAtmosBottom);
					int iAddress = IRRA_UP_NUM * t + s;
					data[3 * iAddress] = float(vSumColor.x());
					data[3 * iAddress + 1] = float(vSumColor.y());
					data[3 * iAddress + 2] = float(vSumColor.z());
				}
			}
			); // end parallel_for

			osg::ref_ptr<osg::Image> pImage = new osg::Image();
			pImage->setImage(iW, iH, 1, GL_RGB32F_ARB, GL_RGB, GL_FLOAT, (unsigned char*)data, osg::Image::USE_NEW_DELETE);
			std::string strTexPath = m_pConfigData->strCorePath + "Textures/Sphere/Irradiance/Irradiance_";
			osgDB::writeImageFile(*(pImage.get()),
				strTexPath + std::to_string(int(fAtmosThick*1e-3)) + "_" + std::to_string(int(fSphereR*1e-3)) + ".tif");
		}
	}
}

void CGMAtmosphere::_MakeAtmosInscattering()
{
	/*	�����Ӿ����:	��������					����뾶����λ��	100km
		16 km			����						8,16,32,64			*100km
		32 km			����						16,32,64,128		*100km
		64 km			����̩̹����������		32,64,128,256		*100km
		128 km			���졢����ľ����			64,128,256,512		*100km
	*/
	const double STEP_UNIT = 100;				// ��������
	const int iAtmosImageBytes = 4 * sizeof(float)
		* SCAT_PITCH_NUM * SCAT_LIGHT_NUM * SCAT_COS_NUM * SCAT_ALT_NUM;

	std::uniform_int_distribution<> iPseudoNoise(0, 999);

	std::string strTransmittancePath = m_pConfigData->strCorePath + "Textures/Sphere/Transmittance/Transmittance_";
	std::string strIrradiancePath = m_pConfigData->strCorePath + "Textures/Sphere/Irradiance/Irradiance_";

	int h = 2; //�������
	//for (int h = 0; h < ATMOS_NUM; h++) //�������
	{
		double fAtmosThick = ATMOS_MIN * 1e3 * exp2(h);				// ������ȣ���λ����
		double fDensAtmosBottom = _GetAtmosBottomDens(fAtmosThick);		// �����������ܶ�

		for (int r = 1; r < 2; r++) //����뾶
		//for (int r = 0; r < RADIUS_NUM; r++) //����뾶
		{
			double fSphereR = (fAtmosThick / ATMOS_2_RADIUS) * exp2(r); //����뾶����λ����
			double fTopR = fSphereR + fAtmosThick;
			double fMinDotUL = GetMinDotUL(fAtmosThick, fSphereR);

			osg::ref_ptr<osg::Image> pTransImg = osgDB::readImageFile(strTransmittancePath
				+ std::to_string(int(fAtmosThick*1e-3)) + "_" + std::to_string(int(fSphereR*1e-3)) + ".tif");
			if (!pTransImg.valid()) break;
			osg::ref_ptr<osg::Image> pIrraImg = osgDB::readImageFile(strIrradiancePath
				+ std::to_string(int(fAtmosThick*1e-3)) + "_" + std::to_string(int(fSphereR*1e-3)) + ".tif");
			if (!pIrraImg.valid()) break;

			// ������ɢ��ֵ
			float* data = new float[iAtmosImageBytes];

			parallel_for(int(0), int(SCAT_PITCH_NUM), [&](int s) // ���߳�
			//for (int s = 0; s < SCAT_PITCH_NUM; s++) // �������Ϸ���н�����ֵ
			{
				for (int t = 0; t < SCAT_ALT_NUM; t++) // ���θ߶� 
				{
					//// ��ƽ����Զ����
					//double fHorizonDisMax = sqrt(fAtmosThick * (fAtmosThick + 2 * fSphereR));
					double fEyeAltCoord = (SCAT_ALT_NUM - 1 - t) / double(SCAT_ALT_NUM - 1);
					// ���ݺ��θ߶ȷֶΣ�����Խ�ͣ��ֶ�Խϸ
					double fEyeR = CGMKit::Mix(fSphereR + 1, fTopR - 1, fEyeAltCoord*fEyeAltCoord);
					// �����ƽ�ߵ�����ֵ
					double fSinHoriz = fSphereR / fEyeR;
					// �����ƽ�ߵ�����ֵ
					double fCosHoriz = -sqrt(max(0, 1 - fSinHoriz * fSinHoriz));
					// ��ÿһ��s���ո������ɸߵ�������
					// fRatioS ���������Ϸ���нǵ�����ֵͬ����
					double fRatioS = 2 * double(s + 0.5) / double(SCAT_PITCH_NUM) - 1;
					// Ϊ����ߵ�ƽ�߸����ľ��ȣ���Ҫ��pitch�ֲ��ز����ȣ�����ƽ�����·ֳ�������
					// �������Ϸ���н�����ֵ
					double fCosUV = -1;
					double fCosUV_1 = fCosHoriz;
					double fDeltaCos = 0;
					if (fRatioS <= 0.0)
					{
						fCosUV = fCosHoriz - fRatioS * fRatioS * (1 + fCosHoriz);
						// �ȡ�fRatioS��Сһ��
						double fRatioS_1 = 2 * double(s + 1.5) / double(SCAT_PITCH_NUM) - 1;
						if (fRatioS_1 <= 0.0)
						{
							fCosUV_1 = fCosHoriz - fRatioS_1 * fRatioS_1 * (1 + fCosHoriz);
						}
						fDeltaCos = fCosUV_1 - fCosUV;
					}
					else
					{
						fCosUV = fCosHoriz + fRatioS * fRatioS * (1 - fCosHoriz);
						// �ȡ�fRatioS��Сһ��
						double fRatioS_1 = 2 * double(s - 0.5) / double(SCAT_PITCH_NUM) - 1;
						if (fRatioS_1 > 0.0)
						{
							fCosUV_1 = fCosHoriz + fRatioS_1 * fRatioS_1 * (1 - fCosHoriz);
						}
						fDeltaCos = fCosUV - fCosUV_1;
					}
					double fSinUV2 = 1 - fCosUV * fCosUV;
					double fSinUV = sqrt(fSinUV2);

					for (int y = 0; y < SCAT_LIGHT_NUM; y++) // �Ϸ�����̫������н�����ֵ
					{
						// С�� fMinDotUL �Ͳ����㣬�������������
						double fCosUL = 1 - (1 - fMinDotUL) * double(y+1) / double(SCAT_LIGHT_NUM);
						double fSinUL = sqrt(1 - fCosUL * fCosUL);
						for (int x = 0; x < SCAT_COS_NUM; x++) // ������̫���нǵ�����ֵ
						{
							// ���ȶ���local����ϵ��
							// �˳���̫��վ�ڵ�ƽ���ϣ���������Y�ᣬ������X�ᣬͷ����Z�ᣬ������ԭ��

							// local�ռ��µ��۵�����
							osg::Vec3d vEyePos = osg::Vec3d(0, 0, fEyeR);
							// local�ռ��µ�̫������
							osg::Vec3d vSunDir = osg::Vec3d(0, fSinUL, fCosUL);
							// local�ռ�YZƽ���ϣ�ӭ���������
							osg::Vec3d vViewFrontDir = osg::Vec3d(0, fSinUV, fCosUV);
							// local�ռ�YZƽ���ϣ������������
							osg::Vec3d vViewBackDir = osg::Vec3d(0, -fSinUV, fCosUV);
							// ���ߺ�̫��������������ֵ����С�нǣ�
							double fMaxCosVL = vViewFrontDir * vSunDir;
							// ���ߺ�̫���������С����ֵ�����нǣ�
							double fMinCosVL = vViewBackDir * vSunDir;
							// ���߷�����̫������ļн�����
							float fCosVL = CGMKit::Mix(fMinCosVL, fMaxCosVL, double(x+0.5) / double(SCAT_COS_NUM));
							// ���߷���
							osg::Vec3d vViewDir = osg::Vec3d(0, 0, 1);
							if (0 == fSinUL)
							{
								vViewDir = osg::Vec3d(fSinUV, 0, fCosUV);
							}
							else
							{
								double fViewDirY = (fCosVL - fCosUV * fCosUL) / fSinUL;
								double fViewDirX = sqrt(fSinUV2 - fViewDirY * fViewDirY);
								vViewDir = osg::Vec3d(fViewDirX, fViewDirY, fCosUV);
							}

							double fTopR = fSphereR + fAtmosThick;
							double fTmp = fEyeR * fSinUV;
							// vEyePos��vTopPos�ľ���
							double fLenET = sqrt(fTopR * fTopR - fTmp * fTmp) - fEyeR * fCosUV;
							// �������߷�����Զ����
							double fLenMax = fLenET;
							// �����������ֵС�ڵ�ƽ������ֵ������Զ��Ϊ����
							if (fCosUV < fCosHoriz)
							{		
								// vEyePos��vGroundPos�ľ���	
								double fLenEG = -fEyeR * fCosUV - sqrt(fSphereR * fSphereR - fTmp * fTmp);
								fLenMax = fLenEG;
							}
							
							double fSampleNum = fLenMax / STEP_UNIT;	
							osg::Vec3d vInscatterSum(0,0,0);
							double fAlphaSum = 0.0;
							for (int j = 0; j < int(fSampleNum + 1); j++)
							{
								// Ϊ�˱�����������������Ĳ���������Ҫ�ڲ�����������һ��������Ŷ�
								vViewDir.z() += fDeltaCos * 0.01 * (iPseudoNoise(m_iRandom) * 0.001 - 0.5);
								vViewDir.normalize();

								// ע�⣺����Ĳ���Ϊ�˱����ݣ�������΢С�ĵ���
								double fLenS = STEP_UNIT * (j + fmod(fSampleNum, 1));
								osg::Vec3d vStepPos = vEyePos + vViewDir * fLenS;
								// ÿһ�����Ϸ���
								osg::Vec3d vStepUp = vStepPos;
								// ÿһ������ĵľ���
								double fStepR = vStepUp.normalize();
								// ÿһ���ĺ��θ߶�
								double fStepAlt = fStepR - fSphereR;
								// ÿһ������λ�õĸ߶�����
								double fStepAltCoord = fStepAlt / fAtmosThick;

								// ÿһ����ɢ��ϵ��
								osg::Vec3d vStepCoef = (_MieCoefficient(fStepAlt, fAtmosThick) + _RayleighCoefficient(fStepAlt, fAtmosThick)) / (4 * osg::PI);
								// ����ÿһ���Ĵ���Alpha����ֵ
								double fAlpha = exp2(-std::fmax(0, fStepAlt) / (fAtmosThick * 0.05));
								// ÿһ�����Ϸ�����̫������н�����ֵ
								float fStepCosUL = vStepUp * vSunDir;
								// ÿһ�����Ϸ��������߷���н�����ֵ
								float fStepCosUV = vStepUp * vViewDir;					
								// ÿһ������λ�ÿ����ĵ�ƽ�ߵ�����ֵ
								double fStepSinHoriz = fSphereR / fStepR;
								// ÿһ������λ�ÿ����ĵ�ƽ�ߵ�����ֵ
								double fStepCosHoriz = -sqrt(max(0, 1 - fStepSinHoriz * fStepSinHoriz));

								// ������ն�	
								osg::Vec4d vI = CGMKit::GetImageColor(pIrraImg.get(),
									fStepCosUL * 0.5f + 0.5f,
									fStepAltCoord,
									true);

								// ����˥��
								osg::Vec3d vT = osg::Vec3d(1, 1, 1);
								// "fCosUV < fCosHoriz" �� "fStepCosUV < fStepCosHoriz"
								// ������������Ȼͬʱ�����ͬʱ������
								if (fCosUV >= fCosHoriz)// ���߳�����գ����ߴ���������۾�
								{		
									// �۵��ɢ��ⷽ��˥��
									osg::Vec4d vEyeT = CGMKit::GetImageColor(pTransImg.get(),
										max(0, fCosUV - fCosHoriz) / (1 - fCosHoriz),
										fEyeAltCoord,
										true);
									// ɢ����ɢ��ⷽ��˥��
									osg::Vec4d vIrraT = CGMKit::GetImageColor(pTransImg.get(),
										max(0, fStepCosUV - fStepCosHoriz) / (1 - fStepCosHoriz),
										fStepAltCoord,
										true);
									// ����ɢ��Ĺ⴫�����۵㣬�ᱻ����������
									// �۾�λ�õ�͸����С����Ϊ����
									vT.x() = vEyeT.x() / max(1e-20, vIrraT.x());
									vT.y() = vEyeT.y() / max(1e-20, vIrraT.y());
									vT.z() = vEyeT.z() / max(1e-20, vIrraT.z());
								}
								else // fCosUV < fCosHoriz ���߳�����棬���ߴӵ��������۾�
								{
									// �۵�λ�õ�ɢ��⡰����˥��
									osg::Vec4d vEyeT = CGMKit::GetImageColor(pTransImg.get(),
										max(0, -fCosUV - fCosHoriz) / (1 - fCosHoriz),
										fEyeAltCoord,
										true);
									// ɢ����ɢ��⡰����˥��
									osg::Vec4d vIrraT = CGMKit::GetImageColor(pTransImg.get(),
										max(0, -fStepCosUV - fStepCosHoriz) / (1 - fStepCosHoriz),
										fStepAltCoord,
										true);
									// ����ɢ��Ĺ⴫�����۵㣬�ᱻ����������
									// �۾�λ�õ�͸���ʴ���Ϊ��ĸ
									vT.x() = vIrraT.x() / max(1e-20, vEyeT.x());
									vT.y() = vIrraT.y() / max(1e-20, vEyeT.y());
									vT.z() = vIrraT.z() / max(1e-20, vEyeT.z());
								}

								// ������
								double fAA = fSampleNum / int(fSampleNum + 1);
								vStepCoef *= fAA;
								fAlpha *= fAA;

								// ���Ӳ���ֵ
								vInscatterSum += osg::Vec3d(
									vStepCoef.x() * vI.x() * vT.x(),
									vStepCoef.y() * vI.y() * vT.y(),
									vStepCoef.z() * vI.z() * vT.z());
								fAlphaSum += fAlpha;
							}
							vInscatterSum *= STEP_UNIT;
							// ������alpha��Ҫ���Ǹ߶Ⱥ�̫��������ǿ��
							fAlphaSum = fmin((vInscatterSum.x() + vInscatterSum.y() + vInscatterSum.z())*10.0, 1.0)
								* (1 - exp(-fAlphaSum * STEP_UNIT * 1e-3));

							int iAddress = ((t * SCAT_COS_NUM + x) * SCAT_LIGHT_NUM + y) * SCAT_PITCH_NUM + s;
							data[4 * iAddress] = float(vInscatterSum.x());
							data[4 * iAddress + 1] = float(vInscatterSum.y());
							data[4 * iAddress + 2] = float(vInscatterSum.z());
							data[4 * iAddress + 3] = float(fAlphaSum);
						}
					}
				}
			}
			); // end parallel_for

			// �洢data��ͼƬ
			osg::ref_ptr<osg::Image> pAtmosScatteringImage = new osg::Image();
			pAtmosScatteringImage->setImage(SCAT_PITCH_NUM, SCAT_LIGHT_NUM * SCAT_COS_NUM * SCAT_ALT_NUM, 1,
				GL_RGBA32F, GL_RGBA, GL_FLOAT, (unsigned char*)data, osg::Image::USE_NEW_DELETE);
			std::string strTexPath = m_pConfigData->strCorePath + "Textures/Sphere/Inscattering/Inscattering_";
			osgDB::writeImageFile(*(pAtmosScatteringImage.get()),
				strTexPath + std::to_string(int(fAtmosThick*1e-3)) + "_" + std::to_string(int(fSphereR*1e-3)) + ".tif");
		}
	}
}

osg::Vec3d CGMAtmosphere::_Transmittance(const double& fAtmosDens,
	const double& fR, const double& fAtmosThick,
	const osg::Vec2d& vP0, const osg::Vec2d& vP1)
{
	const int iLoop = 1024;
	osg::Vec2d vDir = vP1 - vP0;
	double fLen = vDir.normalize();
	double fStepLen = fLen / iLoop;
	osg::Vec3d vSum = osg::Vec3d(0, 0, 0);
	osg::Vec2d vStepPos = vP0 + vDir * fStepLen * 0.5;

	for (int i = 0; i < iLoop; i++)
	{
		double fAlt = vStepPos.length() - fR;
		osg::Vec3d vScattering = _RayleighCoefficient(fAlt, fAtmosThick) + _MieCoefficient(fAlt, fAtmosThick);
		osg::Vec3d vAbsorption = _MieAbsorption(fAlt, fAtmosThick) + _OzoneAbsorption(fAlt, fAtmosThick);
		osg::Vec3d vExtinction = vScattering + vAbsorption;

		vSum += vExtinction * fStepLen;
		vStepPos += vDir * fStepLen;
	}
	vSum *= fAtmosDens;
	return osg::Vec3d(std::exp(-vSum.x()), std::exp(-vSum.y()), std::exp(-vSum.z()));
}
