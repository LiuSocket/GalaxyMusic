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

#define TRANS_ALT_NUM			(128)			// 透过率图的高度采样数 [0,fAtmosThick]m
#define TRANS_PITCH_NUM			(256)			// 透过率图的太阳俯仰角余弦值采样数 [地平线余弦值,1]

#define IRRA_ALT_NUM			(128)			// 辐照度的高度采样数 [0,fAtmosThick]m
#define IRRA_UP_NUM				(128)			// 辐照度的太阳方向与上方向的点乘采样数 [-1,1]

/*************************************************************************
constexpr
*************************************************************************/

constexpr int ATMOS_NUM = 4;					// 大气厚度分类数
constexpr int RADIUS_NUM = 4;					// 星球半径分类数
constexpr double ATMOS_2_RADIUS = 0.02;			// 大气厚度转星球半径时的转换系数

/*************************************************************************
Class
*************************************************************************/

/*************************************************************************
CGMAtmosphere Methods
*************************************************************************/

/** @brief 构造 */
CGMAtmosphere::CGMAtmosphere(): m_strCoreModelPath("Models/")
{
	m_iRandom.seed(0);
}

/** @brief 析构 */
CGMAtmosphere::~CGMAtmosphere()
{
}

/** @brief 初始化 */
bool CGMAtmosphere::Init(SGMConfigData * pConfigData)
{
	m_pConfigData = pConfigData;

	////“透过率”纹理数组
	//_MakeAtmosTransmittance();
	////“辐照度”纹理数组
	//_MakeAtmosIrradiance();
	////“内散射”纹理数组
	//_MakeAtmosInscattering();

	m_pInscatteringTexVector.reserve(ATMOS_NUM * RADIUS_NUM);
	for (int h = 0; h < ATMOS_NUM; h++)
	{
		int iAtmosThick = ATMOS_MIN * exp2(h);	// 大气厚度，单位：km
		std::string strAtmosH = std::to_string(iAtmosThick);
		for (int r = 0; r < RADIUS_NUM; r++)
		{
			double fSphereR = iAtmosThick * exp2(r) / ATMOS_2_RADIUS; //星球半径，单位：km
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
	for (int r = 0; r < RADIUS_NUM; r++) //星球半径
	{
		double fR_exp = (fAtmosH / ATMOS_2_RADIUS) * exp2(r); //星球量化半径，单位：米
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

	//int h = 3; //大气厚度
	for (int h = 0; h < ATMOS_NUM; h++) //大气厚度	
	{
		double fAtmosThick = ATMOS_MIN * 1e3 * exp2(h); // 大气厚度，单位：米
		double fDensAtmosBottom = _GetAtmosBottomDens(fAtmosThick); // 星球表面大气相对密度

		//int r = 0; //星球半径
		for (int r = 0; r < RADIUS_NUM; r++) //星球半径
		{
			double fSphereR = (fAtmosThick / ATMOS_2_RADIUS) * exp2(r); //星球半径，单位：米
			double fTopR = fSphereR + fAtmosThick;
			float* data = new float[iTransmittanceBytes];

			parallel_for(int(0), int(TRANS_ALT_NUM), [&](int t) // 多线程
			//for (int t = 0; t < TRANS_ALT_NUM; t++) // 海拔高度
			{
				// 地平线最远距离
				//double fHorizonDisMax = sqrt(fAtmosThick * fAtmosThick + 2 * fAtmosThick * fSphereR);
				// 根据海拔高度平均分段		
				double fEyeR = CGMKit::Mix(fSphereR + 1, fTopR - 1, t / double(TRANS_ALT_NUM));
				// 眼睛位置点
				osg::Vec2d vEyePos = osg::Vec2d(0, fEyeR);
				for (int s = 0; s < TRANS_PITCH_NUM; s++) // 上方向与太阳方向夹角余弦值
				{
					// 计算地平线的正弦值
					double fSinHoriz = fSphereR / fEyeR;
					// 计算地平线的余弦值
					double fCosHoriz = -sqrt(max(0, 1 - fSinHoriz * fSinHoriz));
					// 上方向与太阳方向夹角余弦，在地平线余弦值与1.0之间的比例
					double fCosUL = CGMKit::Mix(fCosHoriz, 1.0, double(s) / double(TRANS_PITCH_NUM));
					double fSinUL = sqrt(1 - fCosUL * fCosUL);

					double fTmp = fEyeR * fSinUL;
					// vEyePos到vTopPos的距离
					double fLen = sqrt(fTopR * fTopR - fTmp * fTmp) - fEyeR * fCosUL;
					// 大气层顶部位置点
					osg::Vec2d vTopPos = osg::Vec2d(fLen * fSinUL, fEyeR + fLen * fCosUL);
					// 计算直射光的透过率
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
	const int iSurfaceNum = 512;	// 地表总采样数量
	const int iPitchNum = 256;		// 大气采样俯仰方向数量
	const int iYawNum = 32;			// 大气采样偏航方向数量
	const double fStepUnit = 500;	// 采样步长，单位：米
	const int iW = IRRA_UP_NUM;
	const int iH = IRRA_ALT_NUM;
	const int iIrradianceBytes = iW * iH * sizeof(float) * 3;
	std::uniform_int_distribution<> iPseudoNoise(0, 9999);
	// 假设到达地球的太阳光单位面积上能量为 1
	// 也就是说底面积为1，长度为大气厚度(H)的圆柱体上，每个单位体积内分配的能量只有（1/ H）

	//int h = 2; //大气厚度
	for (int h = 0; h < ATMOS_NUM; h++) //大气厚度
	{
		double fAtmosThick = ATMOS_MIN * 1e3 * exp2(h);				// 大气厚度，单位：米
		double fDensAtmosBottom = _GetAtmosBottomDens(fAtmosThick);		// 星球表面大气密度

		//for (int r = 1; r < 2; r++) //星球半径
		for (int r = 0; r < RADIUS_NUM; r++) //星球半径
		{
			float* data = new float[iIrradianceBytes];
			double fSphereR = (fAtmosThick / ATMOS_2_RADIUS) * exp2(r); //星球半径，单位：米
			double fTopR = fSphereR + fAtmosThick;

			osg::ref_ptr<osg::Image> pTransImg = osgDB::readImageFile(
				m_pConfigData->strCorePath + "Textures/Sphere/Transmittance/Transmittance_"
				+ std::to_string(int(fAtmosThick*1e-3)) + "_" + std::to_string(int(fSphereR*1e-3)) + ".tif");
			if (!pTransImg.valid()) break;

			parallel_for(int(0), int(IRRA_UP_NUM), [&](int s) // 多线程
			//for (int s = 0; s < IRRA_UP_NUM; s++) // 上方向与太阳方向夹角余弦值
			{
				double fCosUL = 2 * double(s) / double(IRRA_UP_NUM) - 1;
				// 太阳方向
				osg::Vec3d vSun = osg::Vec3d(0, sqrt(1 - fCosUL* fCosUL), fCosUL);
				for (int t = 0; t < IRRA_ALT_NUM; t++) // 海拔高度
				{
					//// 地平线最远距离
					//double fHorizonDisMax = sqrt(fAtmosThick * fAtmosThick + 2 * fAtmosThick * fSphereR);
					// 根据海拔高度平均分段		
					double fEyeR = CGMKit::Mix(fSphereR + 1, fTopR - 1, t / double(IRRA_ALT_NUM));
					// 计算眼点看到的地平线的正弦值
					double fSinHoriz = fSphereR / fEyeR;
					// 计算眼点看到的地平线的余弦值
					double fCosHoriz = -sqrt(max(0, 1 - fSinHoriz * fSinHoriz));
					// 眼点到地平线的射线与下方向的夹角
					double fHorizonAngle = std::asin(fSinHoriz);
					// 眼点看到的地面球冠总面积
					double fGroundS = osg::PI * 2 * fSphereR * fSphereR * (1 - fSinHoriz);
					// 眼点位置
					osg::Vec3d vEyePos = osg::Vec3d(0, 0, fEyeR);

					osg::Vec3f vAlbedo(0, 0, 0);
// 开启后会出现不连续的怪异结果，所以决定不加上地面反射因素
//#define SURFACE_ALBEDO 0.3
#ifdef SURFACE_ALBEDO
					// 地面类型		反照率(%)
					// 水面			6~8
					// 阔叶林		13~15
					// 草地			10~18
					// 水稻田		12~18
					// 灌木			16~18
					// 田野			15~20
					// 草原			20~25
					// 沙漠			25~30
					// 雪被			> 50
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
						// 漫反射光线与眼点上方向的余弦值 >0
						double fCosAngle = sqrt(1 - fSinAngle * fSinAngle);
						double fTmp = fEyeR * fSinAngle;
						// 眼点到地面焦点的距离
						double fLen = fEyeR * fCosAngle - sqrt(fSphereR * fSphereR - fTmp * fTmp);

						// 地面坐标
						osg::Vec3d vGroundPos = vEyePos + vDir * fLen;
						// 地面法线
						osg::Vec3d vGroundNorm = vGroundPos;
						vGroundNorm.normalize();
						// 太阳方向与地面法线的余弦值
						double fCosNorm2Sun = vGroundNorm * vSun;
						if (fCosNorm2Sun > 0)
						{
							// 如果有漫反射，则获取地表的漫反射光强度
							osg::Vec4 vD = CGMKit::GetImageColor(pTransImg.get(),
								fCosNorm2Sun,
								0.0f,
								true);
							vD *= fCosNorm2Sun * (1 - fCosAngle);

							// 眼点处的漫反射光线方向衰减
							osg::Vec4 vEyeT = CGMKit::GetImageColor(pTransImg.get(),
								(fCosAngle - fCosHoriz) / max(0, 1 - fCosHoriz),
								float(t) / IRRA_ALT_NUM,
								true);

							// 漫反射光方向
							osg::Vec3d vDiffuseDir = osg::Vec3d(0, fSinAngle, fCosAngle);
							// 漫反射光线与地面法线的余弦值
							double fCosDiffuse2Norm = vDiffuseDir * vGroundNorm;
							// 地表的漫反射光线方向衰减
							osg::Vec4 vGroundT = CGMKit::GetImageColor(pTransImg.get(),
								fCosDiffuse2Norm,
								0.0f,
								true);

							// 漫反射光还会被大气再吸收
							vD.x() *= vGroundT.x() / max(1e-20, vEyeT.x());
							vD.y() *= vGroundT.y() / max(1e-20, vEyeT.y());
							vD.z() *= vGroundT.z() / max(1e-20, vEyeT.z());
							vAlbedo += osg::Vec3(vD.x(), vD.y(), vD.z()) * abs(-vDir * vGroundNorm) / (fLen * fLen);
						}
					}
					vAlbedo *= SURFACE_ALBEDO * 1e-3 * fGroundS / iSurfaceNum;
#endif // SURFACE_ALBEDO

					osg::Vec3d vIrradiance(0, 0, 0);
					for (int iX = 0; iX < iPitchNum; iX++)
					{
						// “上方向”与“散射源方向”的夹角
						double fCosUV = 2.0 * (iX + 0.5) / (double)iPitchNum - 1.0;
						double fSinUV = std::sqrt(1 - fCosUV * fCosUV);

						for (int iY = 0; iY < iYawNum; iY++)
						{
							double fYaw = 2.0 * osg::PI * (iY + (double)iX / (double)iPitchNum) / (double)iYawNum;
							// 散射源方向
							osg::Vec3d vOffsetDir = osg::Vec3d(fSinUV * cos(fYaw), fSinUV * sin(fYaw), fCosUV);
							double fIrraDis = vOffsetDir.normalize();
							// 散射光方向
							osg::Vec3d vIrraDir = -vOffsetDir;

							double fTmp = fEyeR * fSinUV;
							// vEyePos到vTopPos的距离
							double fLenET = sqrt(fTopR * fTopR - fTmp * fTmp) - fEyeR * fCosUV;
							// 计算散射源方向最远距离
							double fLenMax = fLenET;
							// 如果散射源方向余弦值小于地平线余弦值，则最远点为地面
							if (fCosUV < fCosHoriz)
							{
								// vEyePos到vGroundPos的距离	
								double fLenEG = -fEyeR * fCosUV - sqrt(fSphereR * fSphereR - fTmp * fTmp);
								fLenMax = fLenEG;
							}

							fLenMax = min(5e3, fLenMax);
							double fSampleNum = fLenMax / fStepUnit;
							for (int c = 0; c < int(fSampleNum + 1); c++)
							{
								// 注意：这里的步长为了避免锯齿，而做了微小的调整
								double fLenS = fStepUnit * (c + fmod(fSampleNum, 1));
								// 散射点坐标
								osg::Vec3d vIrraPos = vEyePos + vOffsetDir * fLenS;
								osg::Vec3d vIrraUp = vIrraPos;
								double fIrraR = vIrraUp.normalize();
								double fIrraAlt = fIrraR - fSphereR;
								double fIrraAltCoord = fIrraAlt / fAtmosThick;
								// 散射点看到的地平线的正弦值
								double fSinHoriz_Source = fSphereR / fIrraR;
								// 散射点看到的地平线的余弦值
								double fCosHoriz_Source = -sqrt(max(0, 1 - fSinHoriz_Source * fSinHoriz_Source));
								// 在眼点周围随机采样太阳直射光
								osg::Vec4d vSunLight = CGMKit::GetImageColor(pTransImg.get(),
									(vIrraUp * vSun - fCosHoriz_Source) / max(0, 1 - fCosHoriz_Source),
									fIrraAltCoord,
									true);

								double fCosIL = vIrraDir * vSun;
								// 米氏散射
								double fMie = _MieCoefficient(fIrraAlt, fAtmosThick) * _MiePhase(fCosIL);
								// 在眼点周围的散射光
								osg::Vec3d vScattering =
									(_RayleighCoefficient(fIrraAlt, fAtmosThick) * _RayleighPhase(fCosIL)
										+ osg::Vec3d(fMie, fMie, fMie));
								osg::Vec3d vI = osg::Vec3(
									vSunLight.r() * vScattering.x(),
									vSunLight.g() * vScattering.y(),
									vSunLight.b() * vScattering.z());

								// 眼点接收到的散射光方向的余弦
								double fIrraCos_Eye = vIrraDir.z();
								// 散射点的散射光方向的余弦
								double fIrraCos_Source = vIrraUp * vIrraDir;
								// "fIrraCos_Eye < fCosHoriz" 与 "fIrraCos_Source < fCosHoriz_Source"
								// 这两个条件必然同时满足或同时不满足
								if (fIrraCos_Eye >= fCosHoriz) // 散射光从上往下射入，眼睛在下
								{
									// 眼点的散射光方向衰减
									osg::Vec4d vEyeT = CGMKit::GetImageColor(pTransImg.get(),
										max(0, fIrraCos_Eye - fCosHoriz) / (1 - fCosHoriz),
										float(t) / IRRA_ALT_NUM,
										true);
									// 散射点的散射光方向衰减
									osg::Vec4d vIrraT = CGMKit::GetImageColor(pTransImg.get(),
										max(0, fIrraCos_Source - fCosHoriz_Source) / max(0, 1 - fCosHoriz_Source),
										fIrraAltCoord,
										true);
									// 多重散射的光传播到眼点，会被大气再吸收
									// 眼睛位置的透射率小，作为分子
									vI.x() *= vEyeT.x() / max(1e-20, vIrraT.x());
									vI.y() *= vEyeT.y() / max(1e-20, vIrraT.y());
									vI.z() *= vEyeT.z() / max(1e-20, vIrraT.z());
								}
								else // fIrraCos_Eye < fCosHoriz  // 散射光从下往上射入，眼睛在上
								{
									// 眼点位置的散射光“逆向”衰减
									osg::Vec4d vEyeT = CGMKit::GetImageColor(pTransImg.get(),
										max(0, -fIrraCos_Eye - fCosHoriz) / (1 - fCosHoriz),
										float(t) / IRRA_ALT_NUM,
										true);
									// 散射点的散射光“逆向”衰减
									osg::Vec4d vIrraT = CGMKit::GetImageColor(pTransImg.get(),
										max(0, -fIrraCos_Source - fCosHoriz_Source) / max(0, 1 - fCosHoriz_Source),
										fIrraAltCoord,
										true);
									// 多重散射的光传播到眼点，会被大气再吸收
									// 眼睛位置的透射率大，作为分母
									vI.x() *= vIrraT.x() / max(1e-20, vEyeT.x());
									vI.y() *= vIrraT.y() / max(1e-20, vEyeT.y());
									vI.z() *= vIrraT.z() / max(1e-20, vEyeT.z());
								}
								// 避免锯齿
								double fAA = fSampleNum / int(fSampleNum + 1);
								vIrradiance += vI * fAA;
							}
						}
					}
					vIrradiance *= 1e6 / double(iPitchNum * iYawNum);

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
	/*	大气视觉厚度:	适用星球：					天体半径，单位：	100km
		16 km			火星						8,16,32,64			*100km
		32 km			暂无						16,32,64,128		*100km
		64 km			地球、泰坦（土卫六）		32,64,128,256		*100km
		128 km			金、天、海、木、土			64,128,256,512		*100km
	*/
	const double GROUND_STEP_UNIT = 50;				// 地面采样步长
	const double SKY_STEP_UNIT = 100;				// 天空采样步长
	const int iAtmosImageBytes = 4 * sizeof(float)
		* SCAT_PITCH_NUM * SCAT_LIGHT_NUM * SCAT_COS_NUM * SCAT_ALT_NUM;
	std::uniform_int_distribution<> iPseudoNoise(0, 999);

	std::string strTransmittancePath = m_pConfigData->strCorePath + "Textures/Sphere/Transmittance/Transmittance_";
	std::string strIrradiancePath = m_pConfigData->strCorePath + "Textures/Sphere/Irradiance/Irradiance_";

	int h = 2; //大气厚度
	//for (int h = 0; h < ATMOS_NUM; h++) //大气厚度
	{
		double fAtmosThick = ATMOS_MIN * 1e3 * exp2(h);				// 大气厚度，单位：米
		double fDensAtmosBottom = _GetAtmosBottomDens(fAtmosThick);		// 星球表面大气密度

		for (int r = 1; r < 2; r++) //  星球半径
		//for (int r = 0; r < RADIUS_NUM; r++) //星球半径
		{
			double fSphereR = (fAtmosThick / ATMOS_2_RADIUS) * exp2(r); //星球半径，单位：米
			double fTopR = fSphereR + fAtmosThick;
			double fMinDotUL = GetMinDotUL(fAtmosThick, fSphereR);

			osg::ref_ptr<osg::Image> pIrraImg = osgDB::readImageFile(strIrradiancePath
				+ std::to_string(int(fAtmosThick*1e-3)) + "_" + std::to_string(int(fSphereR*1e-3)) + ".tif");
			if (!pIrraImg.valid()) break;

			// 计算内散射值
			float* data = new float[iAtmosImageBytes];

			parallel_for(int(0), int(SCAT_PITCH_NUM), [&](int s) // 多线程
			//for (int s = 0; s < SCAT_PITCH_NUM; s++) // d0/dH 或 d0/dh
			{
				for (int t = 0; t < SCAT_ALT_NUM; t++) // 海拔高度 
				{
					double fEyeAltCoord = (SCAT_ALT_NUM - 1 - t) / double(SCAT_ALT_NUM - 1);
					double fEyeAltRatio = fEyeAltCoord * fEyeAltCoord;
					// 根据海拔高度分段，海拔越低，分段越细
					double fEyeR = CGMKit::Mix(fSphereR + 1, fTopR - 1, fEyeAltRatio);
					// 预计算
					double fEyeR2 = fEyeR * fEyeR;
					double fSphereR2 = fSphereR * fSphereR;
					double fTopR2 = fTopR * fTopR;

					// 计算地平线的正弦值
					double fSinHoriz = fSphereR / fEyeR;
					// 计算地平线的余弦值
					double fCosHoriz = -sqrt(max(0, 1 - fSinHoriz * fSinHoriz));
					// 将每一个s按照俯仰角由高到低排列
					// fRatioS 代表 d0/dH 或 d0/dh
					// 必须取到比1.0小一点点的值，保证天顶位置不突变
					// 必须取到比-1.0大一点点的值，保证地面中心位置不突变
					double fRatioS = 2 * double(s) / double(SCAT_PITCH_NUM - 1) - 1;
					fRatioS = osg::clampBetween(fRatioS, -0.99999, 0.99999);
					// 是否是天空
					bool bSky = fRatioS > 0.0;
					// 计算地平线（或者地平线后面天空的）最远距离
					// 眼点到地平线的距离
					double fDisEye2Horizon = sqrt(fEyeR2 - fSphereR2);
					// 地面上的某点到水平方向大气顶端的距离
					double fDisHorizon2Top = sqrt(fTopR2 - fSphereR2);
					// 眼点到地平线后面的大气顶端的距离
					double fDisEye2Top = fDisEye2Horizon + fDisHorizon2Top;

					// 视线末端距离（默认地面）
					double fDisMax = CGMKit::Mix(fDisEye2Horizon, max(0.0, fEyeR - fSphereR), abs(fRatioS));
					// 视线与上方向夹角余弦值(默认视线朝向地面)
					double fCosUV = -(fEyeR2 + fDisMax * fDisMax - fSphereR2) / (2 * fEyeR * fDisMax);
					if (bSky)// 视线看向大气
					{
						// 视线末端距离（大气顶部）
						 fDisMax = CGMKit::Mix(fDisEye2Top, max(0.0, fTopR - fEyeR), fRatioS);
						// 视线与上方向夹角余弦值（看向大气）
						fCosUV = -(fEyeR2 + fDisMax * fDisMax - fTopR2) / (2 * fEyeR * fDisMax);
					}	
					double fSinUV = sqrt(1 - fCosUV * fCosUV);
					double fSampleNum = fDisMax / GROUND_STEP_UNIT;
					if (bSky)
					{
						fSampleNum = fDisMax / SKY_STEP_UNIT;
					}

					for (int y = 0; y < SCAT_LIGHT_NUM; y++) // 上方向与太阳方向夹角余弦值
					{
						// 小于 fMinDotUL 就不计算，提高纹理利用率
						double fCosUL = 1 - (1 - fMinDotUL) * double(y+1) / double(SCAT_LIGHT_NUM);
						double fSinUL = sqrt(1 - fCosUL * fCosUL);
						for (int x = 0; x < SCAT_COS_NUM; x++) // 视线与太阳夹角的余弦值
						{
							// 首先定义local坐标系：
							// 人朝着太阳站在地平面上，脸正面是Y轴，右手是X轴，头顶是Z轴，地心是原点

							// local空间下的眼点坐标
							osg::Vec3d vEyePos = osg::Vec3d(0, 0, fEyeR);
							// local空间下的太阳方向
							osg::Vec3d vSunDir = osg::Vec3d(0, fSinUL, fCosUL);
							// 当前Pitch线上，视线方向的偏航角
							double fYaw = osg::PI * (1 - double(x) / double(SCAT_COS_NUM-1));
							// local空间下，视线方向
							osg::Vec3d vViewDir = osg::Vec3d(fSinUV * sin(fYaw), fSinUV * cos(fYaw), fCosUV);

							osg::Vec4d vInscatterSum(0,0,0,0);
							for (int j = 0; j < int(fSampleNum + 1); j++)
							{
								osg::Vec3d vNoiseViewDir = vViewDir;
								// 注意：这里的步长为了避免锯齿，而做了微小的调整
								double fLenS = (j + fmod(fSampleNum, 1));
								// 为了避免采样间隔过大引起的不连续，需要在采样方向上做一定的随机扰动
								if (bSky)
								{
									fLenS *= SKY_STEP_UNIT;
								}
								else
								{
									fLenS *= GROUND_STEP_UNIT;
								}
								vNoiseViewDir.normalize();

								osg::Vec3d vStepPos = vEyePos + vNoiseViewDir * fLenS;
								// 每一步的上方向
								osg::Vec3d vStepUp = vStepPos;
								// 每一步与地心的距离
								double fStepR = vStepUp.normalize();
								// 每一步的海拔高度
								double fStepAlt = fStepR - fSphereR;
								// 每一步所在位置的高度坐标
								double fStepAltCoord = fStepAlt / fAtmosThick;

								// 每一步的散射系数
								osg::Vec4d vStepCoef = osg::Vec4d(_RayleighCoefficient(fStepAlt, fAtmosThick), _MieCoefficient(fStepAlt, fAtmosThick));
								// 每一步的上方向与太阳方向夹角余弦值
								float fStepCosUL = vStepUp * vSunDir;
								// 每一步的上方向与视线方向夹角余弦值
								float fStepCosUV = vStepUp * vNoiseViewDir;
								// 每一步所在位置看到的地平线的正弦值
								double fStepSinHoriz = fSphereR / fStepR;
								// 每一步所在位置看到的地平线的余弦值
								double fStepCosHoriz = -sqrt(max(0, 1 - fStepSinHoriz * fStepSinHoriz));

								// 计算辐照度	
								osg::Vec4d vI = CGMKit::GetImageColor(pIrraImg.get(),
									fStepCosUL * 0.5f + 0.5f,
									fStepAltCoord,
									true);

								// 避免锯齿
								vStepCoef *= fSampleNum / int(fSampleNum + 1);

								vInscatterSum += osg::Vec4d(
									vStepCoef.x() * vI.x(),
									vStepCoef.y() * vI.y(),
									vStepCoef.z() * vI.z(),
									vStepCoef.w() * (vI.x() + vI.y() + vI.z()) * 0.33333);
							}

							if (bSky)
							{
								vInscatterSum *= SKY_STEP_UNIT;
							}
							else
							{
								vInscatterSum *= GROUND_STEP_UNIT;
							}
							int iAddress = ((t * SCAT_COS_NUM + x) * SCAT_LIGHT_NUM + y) * SCAT_PITCH_NUM + s;
							data[4 * iAddress] = float(vInscatterSum.x());
							data[4 * iAddress + 1] = float(vInscatterSum.y());
							data[4 * iAddress + 2] = float(vInscatterSum.z());
							data[4 * iAddress + 3] = float(vInscatterSum.w());
						}
					}
				}
			}
			); // end parallel_for

			// 存储data到图片
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
		double fMie = _MieCoefficient(fAlt, fAtmosThick);
		osg::Vec3d vScattering = _RayleighCoefficient(fAlt, fAtmosThick) + osg::Vec3d(fMie, fMie, fMie);
		osg::Vec3d vAbsorption = _MieAbsorption(fAlt, fAtmosThick) + _OzoneAbsorption(fAlt, fAtmosThick);
		osg::Vec3d vExtinction = vScattering + vAbsorption;

		vSum += vExtinction * fStepLen;
		vStepPos += vDir * fStepLen;
	}
	vSum *= fAtmosDens;
	return osg::Vec3d(std::exp(-vSum.x()), std::exp(-vSum.y()), std::exp(-vSum.z()));
}
