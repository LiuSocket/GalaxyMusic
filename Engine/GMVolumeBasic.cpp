//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMVolumeBasic.cpp
/// @brief		Galaxy-Music Engine - Volume basic
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.09
//////////////////////////////////////////////////////////////////////////
#include "GMVolumeBasic.h"
#include "GMCommonUniform.h"
#include "GMKit.h"

#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osgDB/ReadFile>

using namespace GM;

/** @brief 构造 */
CGMVolumeBasic::CGMVolumeBasic():
	m_pKernelData(nullptr), m_pConfigData(nullptr), m_pCommonUniform(nullptr),
	m_iScreenWidth(1920), m_iScreenHeight(1080),
	m_strVolumeShaderPath("Shaders/VolumeShader/"), m_strCoreTexturePath("Textures/Volume/"), m_strMediaTexturePath("Volume/"),
	m_fCountUniform(new osg::Uniform("countNum", 0.0f)),
	m_vNoiseUniform(new osg::Uniform("noiseVec4", osg::Vec4f(0.0f, 0.0f, 0.0f, 0.0f))),
	m_fPixelLengthUniform(new osg::Uniform("pixelLength", 0.01f)),
	m_vShakeVectorUniform(new osg::Uniform("shakeVec", osg::Vec2f(0.5f, 0.5f))),
	m_vDeltaShakeUniform(new osg::Uniform("deltaShakeVec", osg::Vec2f(0.0f, 0.0f))),
	m_fShakeU(0.0f), m_fShakeV(0.0f), m_vLastShakeVec(osg::Vec2f(0.0f, 0.0f)),
	m_dTimeLastFrame(0.0), m_iUnitTAA(0), m_iShakeCount(0)
{
	m_iRandom.seed(0);
}

/** @brief 更新(在主相机更新姿态之前) */
void CGMVolumeBasic::Update(double dDeltaTime)
{
	// 4 帧一个循环，4个像素之间的抖动
	m_fShakeU = ((m_iShakeCount / 2) % 2) - 0.5f;
	m_fShakeV = (((m_iShakeCount + 1) / 2) % 2) - 0.5f;

	m_iShakeCount++;
}

/** @brief 析构 */
CGMVolumeBasic::~CGMVolumeBasic()
{
}

/** @brief 初始化 */
void CGMVolumeBasic::Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform)
{
	m_pKernelData = pKernelData;
	m_pConfigData = pConfigData;
	m_pCommonUniform = pCommonUniform;

	m_iScreenWidth = pConfigData->iScreenWidth;
	m_iScreenHeight = pConfigData->iScreenHeight;

	std::string strTexturePath = pConfigData->strCorePath + m_strCoreTexturePath;
	m_3DShapeTex = _Load3DShapeNoise();
	m_3DErosionTex = _Load3DErosionNoise();
	m_3DCurlTex = _Load3DCurlNoise();
	m_blueNoiseTex = _CreateTexture2D(strTexturePath + "BlueNoise.jpg", 1);

	// 传入屏幕像素尺寸，用于相机抖动
	SetPixelLength(pConfigData->fFovy, m_iScreenHeight);

	_InitTAA(pConfigData->strCorePath);
}

/** @brief 更新(在主相机更新姿态之后) */
void CGMVolumeBasic::UpdateLater(double dDeltaTime)
{
	std::uniform_int_distribution<> iPseudoNoise(0, 10000);

	if (m_vNoiseUniform.valid())
	{
		m_vNoiseUniform->set(osg::Vec4f(
			(iPseudoNoise(m_iRandom)) / 1e4f,
			(iPseudoNoise(m_iRandom)) / 1e4f,
			(iPseudoNoise(m_iRandom)) / 1e4f,
			(iPseudoNoise(m_iRandom)) / 1e4f));
	}	
	if (m_vShakeVectorUniform.valid())
		m_vShakeVectorUniform->set(osg::Vec2f(m_fShakeU, m_fShakeV));
	if (m_fCountUniform.valid())
		m_fCountUniform->set(float(m_iShakeCount % 100000));
	osg::Vec2f vShakeVec = osg::Vec2f(m_fShakeU, m_fShakeV);
	if (m_vDeltaShakeUniform.valid())
	{
		osg::Vec2f vDeltaShake = vShakeVec - m_vLastShakeVec;
		m_vDeltaShakeUniform->set(vDeltaShake);
	}

	m_vLastShakeVec = vShakeVec;
}

bool CGMVolumeBasic::ActiveTAA(osg::Texture* pTex, osg::Texture* pVectorTex)
{
	if (!m_statesetTAA.valid()) return false;

	CGMKit::AddTexture(m_statesetTAA.get(), pTex, "currentTex", m_iUnitTAA++);
	CGMKit::AddTexture(m_statesetTAA.get(), pVectorTex, "velocityTex", m_iUnitTAA++);

	GM_Root->addChild(m_TAACamera.get());
	return true;
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
osg::Texture* CGMVolumeBasic::_CreateTexture2D(const std::string & fileName, const int iChannelNum)
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

/**
* 修改屏幕尺寸时,子类调用此函数
* @author LiuTao
* @since 2020.12.07
* @param width: 屏幕宽度
* @param height: 屏幕高度
* @return void
*/
void CGMVolumeBasic::ResizeScreen(const int width, const int height)
{
	m_iScreenWidth = width;
	m_iScreenHeight = height;

	int iW = std::ceil(0.5 * width);
	int iH = std::ceil(0.5 * height);
	if (m_rayMarchCamera.valid())
	{
		m_rayMarchCamera->resize(iW, iH);

		m_vectorMap_0->setTextureSize(iW, iH);
		m_vectorMap_0->dirtyTextureObject();
		m_vectorMap_1->setTextureSize(iW, iH);
		m_vectorMap_1->dirtyTextureObject();

		m_rayMarchTex->setTextureSize(iW, iH);
		m_rayMarchTex->dirtyTextureObject();
	}

	_ResizeScreenTriangle(width, height);

	if (m_TAACamera.valid())
	{
		m_TAACamera->resize(width, height);
		m_TAACamera->setProjectionMatrixAsOrtho2D(0, width, 0, height);
	}
	if (m_TAATex_0.valid() && m_TAATex_1.valid())
	{
		m_TAATex_0->setTextureSize(width, height);
		m_TAATex_0->dirtyTextureObject();
		m_TAATex_1->setTextureSize(width, height);
		m_TAATex_1->dirtyTextureObject();
	}
}

void CGMVolumeBasic::CreatePlatonicSolids(osg::Geometry ** pFaceGeom, osg::Geometry ** pEdgeGeom, osg::Geometry ** pVertGeom) const
{
	if (*pFaceGeom || *pEdgeGeom || *pVertGeom) return;

	const float fBevel = 0.122f;
	const float fScale = 1.0f;
	// Vertex positions
	osg::ref_ptr<osg::Vec3Array> rawVerts = new osg::Vec3Array();
	rawVerts->reserveArray(20);
	rawVerts->push_back(osg::Vec3(0.607f, 0.000f, -0.795f)*fScale);
	rawVerts->push_back(osg::Vec3(0.188f, 0.577f, -0.795f)*fScale);
	rawVerts->push_back(osg::Vec3(-0.491f, 0.357f, -0.795f)*fScale);
	rawVerts->push_back(osg::Vec3(-0.491f, -0.357f, -0.795f)*fScale);
	rawVerts->push_back(osg::Vec3(0.188f, -0.577f, -0.795f)*fScale);
	rawVerts->push_back(osg::Vec3(0.982f, 0.000f, -0.188f)*fScale);
	rawVerts->push_back(osg::Vec3(0.304f, 0.934f, -0.188f)*fScale);
	rawVerts->push_back(osg::Vec3(-0.795f, 0.577f, -0.188f)*fScale);
	rawVerts->push_back(osg::Vec3(-0.795f, -0.577f, -0.188f)*fScale);
	rawVerts->push_back(osg::Vec3(0.304f, -0.934f, -0.188f)*fScale);
	rawVerts->push_back(osg::Vec3(0.795f, 0.577f, 0.188f)*fScale);
	rawVerts->push_back(osg::Vec3(-0.304f, 0.934f, 0.188f)*fScale);
	rawVerts->push_back(osg::Vec3(-0.982f, 0.000f, 0.188f)*fScale);
	rawVerts->push_back(osg::Vec3(-0.304f, -0.934f, 0.188f)*fScale);
	rawVerts->push_back(osg::Vec3(0.795f, -0.577f, 0.188f)*fScale);
	rawVerts->push_back(osg::Vec3(0.491f, 0.357f, 0.795f)*fScale);
	rawVerts->push_back(osg::Vec3(-0.188f, 0.577f, 0.795f)*fScale);
	rawVerts->push_back(osg::Vec3(-0.607f, 0.000f, 0.795f)*fScale);
	rawVerts->push_back(osg::Vec3(-0.188f, -0.577f, 0.795f)*fScale);
	rawVerts->push_back(osg::Vec3(0.491f, -0.357f, 0.795f)*fScale);

	// (faceIndex, vertIndex) => index into rawVerts
	int faceVertIndices[12][5] =
	{
		{ 0, 1, 2, 3, 4 },      // 0
		{ 0, 5, 10, 6, 1 },     // 1
		{ 1, 6, 11, 7, 2 },     // 2
		{ 2, 7, 12, 8, 3 },     // 3
		{ 3, 8, 13, 9, 4 },     // 4
		{ 4, 9, 14, 5, 0 },     // 5
		{ 10, 15, 16, 11, 6 },  // 6
		{ 11, 16, 17, 12, 7 },  // 7
		{ 12, 17, 18, 13, 8 },  // 8
		{ 13, 18, 19, 14, 9 },  // 9
		{ 5, 14, 19, 15, 10 },  // 10
		{ 15, 19, 18, 17, 16 }, // 11
	};

	// For each vert, shows which three faces it's a part of
	int vertFaceIndices[20][3];
	for (int i = 0; i<20; i++)
	{
		int idx = 0;
		for (int j = 0; j<12; j++)
		{
			for (int k = 0; k<5; k++)
			{
				if (faceVertIndices[j][k] == i)
				{
					vertFaceIndices[i][idx++] = j;
				}
			}
		}
	}

	// Edges: (tri0, tri1, vert0, vert1)
	int edges[][4] =
	{
		{ 0, 1, 0, 1 },
		{ 0, 2, 1, 2 },
		{ 0, 3, 2, 3 },
		{ 0, 4, 3, 4 },
		{ 0, 5, 4, 0 },
		{ 1, 10, 5, 10 },
		{ 1, 6, 10, 6 },
		{ 1, 2, 1, 6 },
		{ 2, 6, 6, 11 },
		{ 2, 7, 11, 7 },
		{ 2, 3, 7, 2 },
		{ 3, 7, 7, 12 },
		{ 3, 8, 12, 8 },
		{ 3, 4, 8, 3 },
		{ 4, 8, 8, 13 },
		{ 4, 9, 13, 9 },
		{ 4, 5, 9, 4 },
		{ 5, 9, 9, 14 },
		{ 5, 10, 14, 5 },
		{ 5, 1, 5, 0 },
		{ 6, 11, 15, 16 },
		{ 6, 7, 16, 11 },
		{ 7, 11, 16, 17 },
		{ 7, 8, 17, 12 },
		{ 8, 11, 17, 18 },
		{ 8, 9, 18, 13 },
		{ 9, 11, 18, 19 },
		{ 9, 10, 19, 14 },
		{ 10, 11, 19, 15 },
		{ 10, 6, 15, 10 }
	};

	// Mid-point for each face
	osg::ref_ptr<osg::Vec3Array> faceMidPoints = new osg::Vec3Array(12);
	for (int i = 0; i<12; i++)
	{
		osg::Vec3 midPoint = osg::Vec3(0, 0, 0);
		for (int j = 0; j<5; j++)
		{
			midPoint += (*rawVerts)[faceVertIndices[i][j]];
		}

		(*faceMidPoints)[i] = midPoint / 5.0f;
	}

	int nVertsTotal = 12 * 5 + 20 * 4 + 20 * 3;
	// All vertex positions
	osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array(); verts->reserveArray(nVertsTotal);
	// All normals
	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(); normals->reserveArray(nVertsTotal);
	// Three sets of uvs encode the three normals needed to blend
	osg::ref_ptr<osg::Vec3Array> uvNormal0 = new osg::Vec3Array(); uvNormal0->reserveArray(nVertsTotal);
	osg::ref_ptr<osg::Vec3Array> uvNormal1 = new osg::Vec3Array(); uvNormal1->reserveArray(nVertsTotal);
	osg::ref_ptr<osg::Vec3Array> uvNormal2 = new osg::Vec3Array(); uvNormal2->reserveArray(nVertsTotal);
	// The vertex colors contain the blend weights in rgb
	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(); colors->reserveArray(nVertsTotal);

	// The triangle indices for the faces
	osg::ref_ptr<osg::DrawElementsUShort> faceTris = new osg::DrawElementsUShort(GL_TRIANGLES);
	faceTris->reserve(12 * 9);

	// Compute the faces.
	// Those have only one blend weight.
	for (int i = 0; i<12; i++)
	{
		osg::Vec3 faceMidPoint = (*faceMidPoints)[i];
		osg::Vec3 faceNormal = faceMidPoint;
		faceNormal.normalize();
		for (int j = 0; j < 5; j++)
		{
			osg::Vec3 beveledVertexPosition = faceMidPoint + ((*rawVerts)[faceVertIndices[i][j]] - faceMidPoint) * (1.0f - fBevel);
			verts->push_back(beveledVertexPosition);

			normals->push_back(faceNormal);

			uvNormal0->push_back(faceNormal);//uv0
			uvNormal1->push_back(osg::Vec3(0, 0, 0));
			uvNormal2->push_back(osg::Vec3(0, 0, 0));

			colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
		}

		faceTris->push_back(5 * i + 0);
		faceTris->push_back(5 * i + 1);
		faceTris->push_back(5 * i + 2);
		faceTris->push_back(5 * i + 0);
		faceTris->push_back(5 * i + 2);
		faceTris->push_back(5 * i + 3);
		faceTris->push_back(5 * i + 0);
		faceTris->push_back(5 * i + 3);
		faceTris->push_back(5 * i + 4);
	}

	// Compute the bevelled edges.
	// Those have two blend weights.
	osg::ref_ptr<osg::DrawElementsUShort> edgeTris = new osg::DrawElementsUShort(GL_TRIANGLES);
	edgeTris->reserve(20 * 6);

	for (int i = 0; i< sizeof(edges) / sizeof(edges[0]); i++)
	{
		int f0 = edges[i][0];
		int f1 = edges[i][1];
		int v0 = edges[i][2];
		int v1 = edges[i][3];

		osg::Vec3 midPoint0 = (*faceMidPoints)[f0];
		osg::Vec3 midPoint1 = (*faceMidPoints)[f1];

		osg::Vec3 vert0 = (*rawVerts)[v0];
		osg::Vec3 vert1 = (*rawVerts)[v1];

		// Build a quad with beveled vertex positions
		verts->push_back(midPoint0 + (vert0 - midPoint0) * (1.0f - fBevel));
		verts->push_back(midPoint1 + (vert0 - midPoint1) * (1.0f - fBevel));
		verts->push_back(midPoint1 + (vert1 - midPoint1) * (1.0f - fBevel));
		verts->push_back(midPoint0 + (vert1 - midPoint0) * (1.0f - fBevel));

		int i0 = verts->size() - 4;
		int i1 = verts->size() - 3;
		int i2 = verts->size() - 2;
		int i3 = verts->size() - 1;

		// Ensure ordering
		osg::Vec3 _v0 = (*verts)[i1] - (*verts)[i0];
		osg::Vec3 _v1 = (*verts)[i2] - (*verts)[i0];
		osg::Vec3 _vCross = _v0 ^ _v1;
		if ((_vCross * vert0) > 0.0f)
		{
			int tmp = i1;
			i1 = i3;
			i3 = tmp;
		}

		edgeTris->push_back(i0);
		edgeTris->push_back(i1);
		edgeTris->push_back(i2);
		edgeTris->push_back(i0);
		edgeTris->push_back(i2);
		edgeTris->push_back(i3);

		// Normal
		osg::Vec3 normal = (vert0 + vert1) * 0.5f;
		normal.normalize();
		normals->push_back(normal);
		normals->push_back(normal);
		normals->push_back(normal);
		normals->push_back(normal);

		// The UVs encode the sampling plane normals.
		midPoint0.normalize();
		midPoint1.normalize();

		uvNormal0->push_back(midPoint0);//uv0
		uvNormal0->push_back(midPoint0);//uv0
		uvNormal0->push_back(midPoint0);//uv0
		uvNormal0->push_back(midPoint0);//uv0

		uvNormal1->push_back(midPoint1);//uv1
		uvNormal1->push_back(midPoint1);//uv1
		uvNormal1->push_back(midPoint1);//uv1
		uvNormal1->push_back(midPoint1);//uv1

		uvNormal2->push_back(osg::Vec3(0, 0, 0));
		uvNormal2->push_back(osg::Vec3(0, 0, 0));
		uvNormal2->push_back(osg::Vec3(0, 0, 0));
		uvNormal2->push_back(osg::Vec3(0, 0, 0));

		// Blend weights are stored in the color channel
		osg::Vec4 c0 = osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f);
		osg::Vec4 c1 = osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f);
		colors->push_back(c0);
		colors->push_back(c1);
		colors->push_back(c1);
		colors->push_back(c0);
	}

	// Compute the bevelled vertices.
	// Those have three blend weights.
	osg::ref_ptr<osg::DrawElementsUShort> vertTris = new osg::DrawElementsUShort(GL_TRIANGLES);
	vertTris->reserve(20 * 3);

	for (int i = 0; i<20; i++)
	{
		osg::Vec3 normal = (*rawVerts)[i];
		normal.normalize();

		// The UVs encode the sampling plane normals.
		osg::Vec3 m1 = (*faceMidPoints)[vertFaceIndices[i][0]]; m1.normalize();
		osg::Vec3 m2 = (*faceMidPoints)[vertFaceIndices[i][1]]; m2.normalize();
		osg::Vec3 m3 = (*faceMidPoints)[vertFaceIndices[i][2]]; m3.normalize();

		for (int j = 0; j<3; j++)
		{
			int faceIdx = vertFaceIndices[i][j];
			osg::Vec3 faceMidPoint = (*faceMidPoints)[faceIdx];

			osg::Vec3 beveledVertexPosition = faceMidPoint + ((*rawVerts)[i] - faceMidPoint) * (1.0f - fBevel);
			verts->push_back(beveledVertexPosition);

			normals->push_back(normal);

			uvNormal0->push_back(m1);//uv0
			uvNormal1->push_back(m2);//uv1
			uvNormal2->push_back(m3);//uv2
		}

		// Blend weights are stored in the color channel.
		colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
		colors->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
		colors->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));

		// Indices
		int i0 = verts->size() - 3;
		int i1 = verts->size() - 2;
		int i2 = verts->size() - 1;

		// Ensure ordering
		osg::Vec3 _v0 = (*verts)[i1] - (*verts)[i0];
		osg::Vec3 _v1 = (*verts)[i2] - (*verts)[i0];
		osg::Vec3 _vCross = _v0 ^ _v1;
		if ((_vCross * (*rawVerts)[i]) > 0.0f)
		{
			int tmp = i1;
			i1 = i2;
			i2 = tmp;
		}

		vertTris->push_back(i0);
		vertTris->push_back(i1);
		vertTris->push_back(i2);
	}

	// Face Geom
	*pFaceGeom = new osg::Geometry();
	(*pFaceGeom)->setUseVertexBufferObjects(true);
	(*pFaceGeom)->setUseDisplayList(false);
	(*pFaceGeom)->setDataVariance(osg::Object::DYNAMIC);

	// Array
	(*pFaceGeom)->setVertexArray(verts.get());
	(*pFaceGeom)->setNormalArray(normals.get());
	(*pFaceGeom)->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	(*pFaceGeom)->setColorArray(colors.get());
	(*pFaceGeom)->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	(*pFaceGeom)->setTexCoordArray(0, uvNormal0);
	(*pFaceGeom)->setTexCoordArray(1, uvNormal1);
	(*pFaceGeom)->setTexCoordArray(2, uvNormal2);

	// Edge Geom
	*pEdgeGeom = new osg::Geometry(**pFaceGeom, osg::CopyOp::DEEP_COPY_ARRAYS);
	(*pEdgeGeom)->setUseVertexBufferObjects(true);
	(*pEdgeGeom)->setUseDisplayList(false);
	(*pEdgeGeom)->setDataVariance(osg::Object::DYNAMIC);

	// Vert Geom
	*pVertGeom = new osg::Geometry(**pFaceGeom, osg::CopyOp::DEEP_COPY_ARRAYS);
	(*pVertGeom)->setUseVertexBufferObjects(true);
	(*pVertGeom)->setUseDisplayList(false);
	(*pVertGeom)->setDataVariance(osg::Object::DYNAMIC);

	// Name
	(*pFaceGeom)->setName("Dodecahedron_Face");
	(*pEdgeGeom)->setName("Dodecahedron_Edge");
	(*pVertGeom)->setName("Dodecahedron_Vert");

	// PrimitiveSet
	(*pFaceGeom)->addPrimitiveSet(faceTris.get());
	(*pEdgeGeom)->addPrimitiveSet(edgeTris.get());
	(*pVertGeom)->addPrimitiveSet(vertTris.get());
}

void CGMVolumeBasic::SetPixelLength(const float fFovy, const int iHeight)
{
	if (!m_fPixelLengthUniform.valid()) return;

	float fFovyRadian = osg::DegreesToRadians(fFovy);
	float fPixelLength = tan(fFovyRadian*0.5) / (iHeight*0.5);
	m_fPixelLengthUniform->set(fPixelLength);
}

osg::Texture2D* CGMVolumeBasic::CreateTexture(
	const osg::Vec2i & size,
	GLint internalFormat,
	GLenum sourceFormat,
	GLenum sourceType)
{
	osg::Texture2D* texture = new osg::Texture2D();
	texture->setTextureSize(size.x(), size.y());
	texture->setInternalFormat(internalFormat);
	texture->setSourceFormat(sourceFormat);
	texture->setSourceType(sourceType);

	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	texture->setBorderColor(osg::Vec4d(0, 0, 0, 0));
	texture->setDataVariance(osg::Object::DYNAMIC);
	texture->setResizeNonPowerOfTwoHint(false);
	return texture;
}

void CGMVolumeBasic::_InitTAA(std::string strCorePath)
{
	int iW = m_pConfigData->iScreenWidth;
	int iH = m_pConfigData->iScreenHeight;

	m_TAATex_0 = CreateTexture(osg::Vec2i(iW, iH));
	m_TAATex_1 = CreateTexture(osg::Vec2i(iW, iH));

	m_TAACamera = new osg::Camera;
	m_TAACamera->setName("TAACamera");
	m_TAACamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT);
	m_TAACamera->setClearMask(GL_COLOR_BUFFER_BIT);
	m_TAACamera->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
	m_TAACamera->setViewport(0, 0, iW, iH);
	m_TAACamera->setRenderOrder(osg::Camera::PRE_RENDER,2);
	m_TAACamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	m_TAACamera->attach(osg::Camera::COLOR_BUFFER, m_TAATex_0.get());
	m_TAACamera->setAllowEventFocus(false);
	m_TAACamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	m_TAACamera->setViewMatrix(osg::Matrix::identity());
	m_TAACamera->setProjectionMatrixAsOrtho2D(0, iW, 0, iH);
	m_TAACamera->setProjectionResizePolicy(osg::Camera::FIXED);

	// TAA交换buffer的回调函数指针
	SwitchFBOCallback* pTAAFBOCallback = new SwitchFBOCallback(m_TAATex_1.get(), m_TAATex_0.get());
	m_TAACamera->setPostDrawCallback(pTAAFBOCallback);

	m_pTAAGeode = new osg::Geode();
	m_pTAAGeode->addDrawable(_CreateScreenTriangle(iW, iH));
	m_TAACamera->addChild(m_pTAAGeode.get());

	m_statesetTAA = m_pTAAGeode->getOrCreateStateSet();
	m_statesetTAA->addUniform(m_pCommonUniform->GetScreenSize());
	m_statesetTAA->addUniform(m_vShakeVectorUniform.get());

	CGMKit::AddTexture(m_statesetTAA.get(), m_TAATex_1.get(), "lastTex", m_iUnitTAA++);

	std::string strTAAVertPath = m_pConfigData->strCorePath + m_strVolumeShaderPath + "TAAVert.glsl";
	std::string strTAAFragPath = m_pConfigData->strCorePath + m_strVolumeShaderPath + "TAAFrag.glsl";
	CGMKit::LoadShader(m_statesetTAA.get(), strTAAVertPath, strTAAFragPath, "TAA");
}

/**
* 创建渲染面
* @author LiuTao
* @since 2020.09.01
* @param width: 面的宽度
* @param height: 面的高度
* @return osg::Geometry* 返回几何节点指针
*/
osg::Geometry* CGMVolumeBasic::_CreateScreenTriangle(const int width, const int height)
{
	osg::Geometry* pGeometry = new osg::Geometry();

	osg::ref_ptr<osg::Vec3Array> verArray = new osg::Vec3Array;
	verArray->push_back(osg::Vec3(0, 0, 0));
	verArray->push_back(osg::Vec3(2*width, 0, 0));
	verArray->push_back(osg::Vec3(0, 2*height, 0));
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
	pGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pGeometry->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

	return pGeometry;
}

void CGMVolumeBasic::_ResizeScreenTriangle(const int width, const int height)
{
	osg::ref_ptr<osg::Geometry> pGeometry = m_pTAAGeode->asGeode()->getDrawable(0)->asGeometry();
	osg::ref_ptr<osg::Vec3Array> verArray = new osg::Vec3Array;
	verArray->push_back(osg::Vec3(0, 0, 0));
	verArray->push_back(osg::Vec3(2 * width, 0, 0));
	verArray->push_back(osg::Vec3(0, 2 * height, 0));
	pGeometry->setVertexArray(verArray);
	pGeometry->dirtyBound();
}

osg::Texture* CGMVolumeBasic::_Load3DShapeNoise() const
{
	std::string strTexturePath = m_pConfigData->strCorePath + m_strCoreTexturePath + "noiseShape.raw";
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

osg::Texture* CGMVolumeBasic::_Load3DErosionNoise() const
{
	std::string strTexturePath = m_pConfigData->strCorePath + m_strCoreTexturePath + "noiseErosion.raw";
	osg::Image* img = osgDB::readImageFile(strTexturePath);
	img->setImage(32, 32, 32, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, img->data(), osg::Image::NO_DELETE);
	osg::Texture3D* tex3d = new osg::Texture3D;
	tex3d->setImage(img);
	tex3d->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	tex3d->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	tex3d->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	tex3d->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	tex3d->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
	tex3d->setInternalFormat(GL_RGB8);
	tex3d->setSourceFormat(GL_RGB);
	tex3d->setSourceType(GL_UNSIGNED_BYTE);
	tex3d->allocateMipmapLevels();
	return tex3d;
}

osg::Texture* CGMVolumeBasic::_Load3DCurlNoise() const
{
	std::string strTexturePath = m_pConfigData->strCorePath + m_strCoreTexturePath + "noiseCurl.raw";
	osg::Image* img = osgDB::readImageFile(strTexturePath);
	img->setImage(128, 128, 4, GL_R8, GL_RED, GL_UNSIGNED_BYTE, img->data(), osg::Image::NO_DELETE);
	osg::Texture3D* tex3d = new osg::Texture3D;
	tex3d->setImage(img);
	tex3d->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	tex3d->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	tex3d->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	tex3d->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	tex3d->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
	tex3d->setInternalFormat(GL_R8);
	tex3d->setSourceFormat(GL_RED);
	tex3d->setSourceType(GL_UNSIGNED_BYTE);
	tex3d->allocateMipmapLevels();
	return tex3d;
}