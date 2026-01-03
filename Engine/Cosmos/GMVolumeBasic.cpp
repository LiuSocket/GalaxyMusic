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
#include "../GMCommonUniform.h"
#include "../GMKit.h"

#include <osg/Texture3D>
#include <osgDB/ReadFile>

using namespace GM;

CGMVolumeBasic::CGMVolumeBasic():
	m_pKernelData(nullptr), m_pConfigData(nullptr), m_pCommonUniform(nullptr),
	m_fResolutionScale(0.25f),
	m_fPixelLengthUniform(new osg::Uniform("pixelLength", 0.01f)),
	m_strVolumeShaderPath("Shaders/VolumeShader/"), m_strCoreTexturePath("Textures/Volume/"),
	m_strMediaTexturePath("Volume/")
{
}

CGMVolumeBasic::~CGMVolumeBasic()
{
}

void CGMVolumeBasic::Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform)
{
	m_pKernelData = pKernelData;
	m_pConfigData = pConfigData;
	m_pCommonUniform = pCommonUniform;

	m_pVolumeRoot = new osg::Group();
	GM_Root->addChild(m_pVolumeRoot.get());
	osg::ref_ptr<osg::StateSet> pSS = m_pVolumeRoot->getOrCreateStateSet();
	if (EGMRENDER_LOW == m_pConfigData->eRenderQuality)
	{
		pSS->setDefine("RESOLUTION_QUARTER", osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		m_fResolutionScale = 0.25f;
	}
	else if (EGMRENDER_NORMAL == m_pConfigData->eRenderQuality)
	{
		m_fResolutionScale = 0.5f;
	}
	else{}

	std::string strTexturePath = m_pConfigData->strCorePath + m_strCoreTexturePath;
	m_3DShapeTex = _Load3DShapeNoise();
	m_3DErosionTex = _Load3DErosionNoise();
	m_3DCurlTex = _Load3DCurlNoise();
	m_blueNoiseTex = _CreateTexture2D(strTexturePath + "BlueNoise.jpg", 1);

	// ³õÊ¼»¯TAA
	_InitRayMarching();	
}

void CGMVolumeBasic::Update(double dDeltaTime)
{
}

void CGMVolumeBasic::UpdateLater(double dDeltaTime)
{
	double fFovy, fAspectRatio, fZNear, fZFar;
	GM_View->getCamera()->getProjectionMatrixAsPerspective(fFovy, fAspectRatio, fZNear, fZFar);
	_SetPixelLength(fFovy, m_pConfigData->iScreenHeight);
}

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

void CGMVolumeBasic::ResizeScreen(const int width, const int height)
{
	int iW = width * m_fResolutionScale;
	int iH = height * m_fResolutionScale;
	if (m_rayMarchCamera.valid())
	{
		m_rayMarchCamera->resize(iW, iH);
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

void CGMVolumeBasic::_InitRayMarching()
{
	int iW = m_pConfigData->iScreenWidth * m_fResolutionScale;
	int iH = m_pConfigData->iScreenHeight * m_fResolutionScale;

	m_rayMarchColorTex = new osg::Texture2D;
	m_rayMarchColorTex->setName("rayMarchColorTex");
	m_rayMarchColorTex->setTextureSize(iW, iH);
	m_rayMarchColorTex->setInternalFormat(GL_RGBA8);
	m_rayMarchColorTex->setSourceFormat(GL_RGBA);
	m_rayMarchColorTex->setSourceType(GL_UNSIGNED_BYTE);
	m_rayMarchColorTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_rayMarchColorTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	m_rayMarchColorTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_rayMarchColorTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_rayMarchColorTex->setDataVariance(osg::Object::DYNAMIC);
	m_rayMarchColorTex->setResizeNonPowerOfTwoHint(false);

	m_rayMarchAlphaTex = new osg::Texture2D;
	m_rayMarchAlphaTex->setName("rayMarchAlphaTex");
	m_rayMarchAlphaTex->setTextureSize(iW, iH);
	m_rayMarchAlphaTex->setInternalFormat(GL_RGBA8);
	m_rayMarchAlphaTex->setSourceFormat(GL_RGBA);
	m_rayMarchAlphaTex->setSourceType(GL_UNSIGNED_BYTE);
	m_rayMarchAlphaTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
	m_rayMarchAlphaTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
	m_rayMarchAlphaTex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_rayMarchAlphaTex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_rayMarchAlphaTex->setDataVariance(osg::Object::DYNAMIC);
	m_rayMarchAlphaTex->setResizeNonPowerOfTwoHint(false);

	// Create its camera and render to it
	m_rayMarchCamera = new osg::Camera;
	m_rayMarchCamera->setName("rayMarchCamera");
	m_rayMarchCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT);
	m_rayMarchCamera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	m_rayMarchCamera->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
	m_rayMarchCamera->setViewport(0, 0, iW, iH);
	m_rayMarchCamera->setRenderOrder(osg::Camera::PRE_RENDER, 1);
	m_rayMarchCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	m_rayMarchCamera->attach(osg::Camera::COLOR_BUFFER0, m_rayMarchColorTex.get());
	m_rayMarchCamera->attach(osg::Camera::COLOR_BUFFER1, m_rayMarchAlphaTex.get());
	m_rayMarchCamera->setAllowEventFocus(false);
	m_rayMarchCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

	m_pVolumeRoot->addChild(m_rayMarchCamera.get());
}

void CGMVolumeBasic::_SetPixelLength(const float fFovy, const int iHeight)
{
	if (!m_fPixelLengthUniform.valid()) return;

	float fFovyRadian = osg::DegreesToRadians(fFovy);
	float fPixelLength = tan(fFovyRadian * 0.5) / (iHeight * 0.5);
	m_fPixelLengthUniform->set(fPixelLength);
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