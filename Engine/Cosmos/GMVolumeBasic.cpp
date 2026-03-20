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
	//m_fPixelLengthUniform(new osg::Uniform("pixelLength", 0.01f)),
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

	m_fResolutionScale = (EGMRENDER_LOW == m_pConfigData->eRenderQuality) ? 0.25f : 0.5f;

	std::string strTexturePath = m_pConfigData->strCorePath + m_strCoreTexturePath;
	m_3DShapeTex = _Load3DShapeNoise();
	m_3DErosionTex = _Load3DErosionNoise();
	m_3DCurlTex = _Load3DCurlNoise();
	m_blueNoiseTex = _CreateTexture2D(strTexturePath + "BlueNoise_RGBA.tga", 4);

	// 初始化TAA
	_InitRayMarching();	
}

void CGMVolumeBasic::Update(double dDeltaTime)
{
}

void CGMVolumeBasic::UpdateLater(double dDeltaTime)
{
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

	//double fFovy, fAspectRatio, fZNear, fZFar;
	//GM_View->getCamera()->getProjectionMatrixAsPerspective(fFovy, fAspectRatio, fZNear, fZFar);
	//_SetPixelLength(fFovy, iH);
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

void CGMVolumeBasic::CreateFootballSolids(osg::Geometry** pFaceGeom, osg::Geometry** pEdgeGeom, osg::Geometry** pVertGeom) const
{
	if (*pFaceGeom || *pEdgeGeom || *pVertGeom) return;

	const float fBevel = 0.333f;   // 切角比例
	const float fOffset = 0.15f;    // 扩张距离
	const float fScale = 2.0f;     // 缩放因子

	// 正二十面体顶点（归一化到单位球面，立起来：顶点4在北极，顶点7在南极）
	const float X = 0.525731112119133606f;
	const float Z = 0.850650808352039932f;
	osg::ref_ptr<osg::Vec3Array> rawVerts = new osg::Vec3Array();
	rawVerts->reserve(12);
	rawVerts->push_back(osg::Vec3(-X, -Z * Z, X * Z) * fScale); // 0
	rawVerts->push_back(osg::Vec3(X, -Z * Z, X * Z) * fScale); // 1
	rawVerts->push_back(osg::Vec3(-X, Z * Z, -X * Z) * fScale); // 2
	rawVerts->push_back(osg::Vec3(X, Z * Z, -X * Z) * fScale); // 3
	rawVerts->push_back(osg::Vec3(0, 0, 1) * fScale); // 4 北极
	rawVerts->push_back(osg::Vec3(0, 2 * X * Z, Z * Z - X * X) * fScale); // 5
	rawVerts->push_back(osg::Vec3(0, -2 * X * Z, X * X - Z * Z) * fScale); // 6
	rawVerts->push_back(osg::Vec3(0, 0, -1) * fScale); // 7 南极
	rawVerts->push_back(osg::Vec3(Z, X * X, X * Z) * fScale); // 8
	rawVerts->push_back(osg::Vec3(-Z, X * X, X * Z) * fScale); // 9
	rawVerts->push_back(osg::Vec3(Z, -X * X, -X * Z) * fScale); // 10
	rawVerts->push_back(osg::Vec3(-Z, -X * X, -X * Z) * fScale); // 11

	// 二十面体的三角形面（20个面，每个面3个顶点，顺序与原始一致，保证逆时针从外部看）
	std::vector<std::vector<int>> inputFaces = {
		{1,4,0}, {4,9,0}, {4,5,9}, {8,5,4}, {1,8,4},
		{1,10,8}, {10,3,8}, {8,3,5}, {3,2,5}, {3,7,2},
		{3,10,7}, {10,6,7}, {6,11,7}, {6,0,11}, {6,1,0},
		{10,1,6}, {11,0,9}, {2,11,9}, {5,2,9}, {11,2,7}
	};

	// 第一步：切角得到截角二十面体（输出普通面）
	osg::ref_ptr<osg::Vec3Array> interVerts;
	std::vector<std::vector<int>> interFaces;
	std::vector<std::pair<int, int>> interEdges;
	_TruncatePolyhedron(rawVerts, inputFaces, fBevel, interVerts, interFaces, interEdges);

	// 第二步：扩张得到小斜方截角二十面体（输出带类型的面）
	osg::ref_ptr<osg::Vec3Array> finalVerts;
	std::vector<FaceWithType> finalFacesWithType;
	std::vector<std::pair<int, int>> finalEdges;
	_ExpandPolyhedron(interVerts, interFaces, fOffset, finalVerts, finalFacesWithType, finalEdges);

	// 预计算：interFaces 的面法线 & 每个 interFace 在 interVerts 中的起始索引（用于从 finalVerts 索引反推来源 face）
	std::vector<osg::Vec3> interFaceNormals(interFaces.size());
	std::vector<int> interFaceStart(interFaces.size());
	{
		int acc = 0;
		for (size_t fi = 0; fi < interFaces.size(); ++fi)
		{
			const auto& face = interFaces[fi];
			interFaceStart[fi] = acc;
			acc += (int)face.size();

			// 面法线（与 _ExpandPolyhedron 中计算一致）
			osg::Vec3 a = (*interVerts)[face[0]];
			osg::Vec3 b = (*interVerts)[face[1]];
			osg::Vec3 c = (*interVerts)[face[2]];
			osg::Vec3 n = (b - a) ^ (c - a);
			n.normalize();
			interFaceNormals[fi] = n;
		}
	}

	auto getOriginalFaceIndexOfOutputVertex = [&](int outIdx) -> int {
		// finalVerts (outputVerts) 在 _ExpandPolyhedron 中按 interFaces 顺序 push_back
		for (size_t fi = 0; fi < interFaces.size(); ++fi)
		{
			int start = interFaceStart[fi];
			int len = (int)interFaces[fi].size();
			if (outIdx >= start && outIdx < start + len) return (int)fi;
		}
		return -1;
		};

	// 分别构建三个几何体（原面面、棱面、顶点面）
	osg::ref_ptr<osg::Vec3Array> faceVerts[3];
	osg::ref_ptr<osg::Vec3Array> faceNormals[3];
	osg::ref_ptr<osg::Vec4Array> faceColors[3];
	osg::ref_ptr<osg::Vec3Array> faceUV0[3];
	osg::ref_ptr<osg::Vec3Array> faceUV1[3];
	osg::ref_ptr<osg::Vec3Array> faceUV2[3];
	osg::ref_ptr<osg::DrawElementsUShort> faceTris[3];

	for (int i = 0; i < 3; ++i)
	{
		faceVerts[i] = new osg::Vec3Array;
		faceNormals[i] = new osg::Vec3Array;
		faceColors[i] = new osg::Vec4Array;
		faceUV0[i] = new osg::Vec3Array;
		faceUV1[i] = new osg::Vec3Array;
		faceUV2[i] = new osg::Vec3Array;
		faceTris[i] = new osg::DrawElementsUShort(GL_TRIANGLES);
	}

	// 颜色定义（用于 shader 混合权重）
	const osg::Vec4 cRed = osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f); // 单一面 -> R=1
	const osg::Vec4 cGreen = osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f); // 第二权重 -> G=1
	const osg::Vec4 cBlue = osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f); // 第三权重 -> B=1

	// 遍历所有带类型的面，分类填充
	for (const auto& fwt : finalFacesWithType)
	{
		const auto& face = fwt.indices;
		int typeIdx = fwt.type; // 0:顶点面, 1:原面面, 2:棱面
		int nVerts = (int)face.size();
		if (nVerts < 3) continue; // 安全保护

		// 计算该面的法线
		osg::Vec3 p0 = (*finalVerts)[face[0]];
		osg::Vec3 p1 = (*finalVerts)[face[1]];
		osg::Vec3 p2 = (*finalVerts)[face[2]];
		osg::Vec3 normal = (p1 - p0) ^ (p2 - p0);
		normal.normalize();

		// 为了双重保险，依然可以用模型中心(0,0,0)做个朝外检测。
		osg::Vec3 faceCenter(0, 0, 0);
		for (int i = 0; i < nVerts; ++i) faceCenter += (*finalVerts)[face[i]];
		faceCenter /= (float)nVerts;
		osg::Vec3 outward = faceCenter;
		outward.normalize();
		if (normal * outward < 0.0f) normal = -normal;

		// 为三层 UV 计算采样方向
		osg::Vec3 samp0 = osg::Vec3(0, 0, 0);
		osg::Vec3 samp1 = osg::Vec3(0, 0, 0);
		osg::Vec3 samp2 = osg::Vec3(0, 0, 0);

		if (typeIdx == 1) // original face：单一采样方向
		{
			samp0 = normal;
		}
		else if (typeIdx == 2) // edge face：之前的逻辑保持（已正确）
		{
			// 收集 quad 每个顶点对应的原始 interFace 索引
			std::vector<int> origIdxPerVertex(nVerts, -1);
			std::set<int> uniqueOrigFaces;
			for (int vi = 0; vi < nVerts; ++vi)
			{
				int outIdx = face[vi];
				int origFi = getOriginalFaceIndexOfOutputVertex(outIdx);
				origIdxPerVertex[vi] = origFi;
				if (origFi >= 0) uniqueOrigFaces.insert(origFi);
			}

			if (uniqueOrigFaces.size() >= 2)
			{
				int fL = -1, fR = -1;
				if (nVerts >= 4 && origIdxPerVertex[0] == origIdxPerVertex[3] && origIdxPerVertex[1] == origIdxPerVertex[2])
				{
					fL = origIdxPerVertex[0];
					fR = origIdxPerVertex[1];
				}
				else
				{
					std::map<int, int> cnt;
					for (int id : origIdxPerVertex) if (id >= 0) ++cnt[id];
					std::vector<std::pair<int, int>> vec;
					for (auto& kv : cnt) vec.push_back({ kv.second, kv.first });
					std::sort(vec.begin(), vec.end(), std::greater<>());
					if (vec.size() >= 2)
					{
						int a = vec[0].second;
						int b = vec[1].second;
						if (origIdxPerVertex[0] == a) { fL = a; fR = b; }
						else if (origIdxPerVertex[0] == b) { fL = b; fR = a; }
						else { fL = a; fR = b; }
					}
				}

				if (fL >= 0 && fR >= 0)
				{
					osg::Vec3 nA = interFaceNormals[fL];
					osg::Vec3 nB = interFaceNormals[fR];
					nA.normalize();
					nB.normalize();
					if (nA * outward < 0.0f) nA = -nA;
					if (nB * outward < 0.0f) nB = -nB;
					samp0 = nA;
					samp1 = nB;

					int base = faceVerts[typeIdx]->size();
					for (int i = 0; i < nVerts; ++i)
					{
						faceVerts[typeIdx]->push_back((*finalVerts)[face[i]]);
						faceNormals[typeIdx]->push_back(normal);

						faceUV0[typeIdx]->push_back(samp0);
						faceUV1[typeIdx]->push_back(samp1);
						faceUV2[typeIdx]->push_back(osg::Vec3(0, 0, 0));

						int src = origIdxPerVertex[i];
						if (src == fL) faceColors[typeIdx]->push_back(cRed);
						else if (src == fR) faceColors[typeIdx]->push_back(cGreen);
						else faceColors[typeIdx]->push_back(cGreen);
					}

					for (int i = 1; i < nVerts - 1; ++i)
					{
						faceTris[typeIdx]->push_back(base);
						faceTris[typeIdx]->push_back(base + i);
						faceTris[typeIdx]->push_back(base + i + 1);
					}

					continue;
				}
			}

			// 兜底：基于几何退化方案
			if (nVerts >= 2)
			{
				osg::Vec3 v0 = (*finalVerts)[face[0]];
				osg::Vec3 v1 = (*finalVerts)[face[1]];
				osg::Vec3 v2 = (*finalVerts)[(nVerts > 2) ? 2 : 0];
				osg::Vec3 nA = (v1 - v0) ^ (faceCenter - v0);
				osg::Vec3 nB = (v2 - v1) ^ (faceCenter - v1);
				nA.normalize();
				nB.normalize();
				if (nA * outward < 0.0f) nA = -nA;
				if (nB * outward < 0.0f) nB = -nB;
				samp0 = nA;
				samp1 = nB;
			}
			else
			{
				samp0 = normal;
			}
		}
		else if (typeIdx == 0) // vertex face：以来源原始面（interFaces）的法线构造三组采样方向，并按来源填色
		{
			// 对于顶点面，face 的每个索引对应于 finalVerts 中某个条目；该条目的来源原始面通过 getOriginalFaceIndexOfOutputVertex 可反推
			std::vector<int> origIdxPerVertex(nVerts, -1);
			std::vector<int> uniqOrder;
			for (int vi = 0; vi < nVerts; ++vi)
			{
				int outIdx = face[vi];
				int origFi = getOriginalFaceIndexOfOutputVertex(outIdx);
				origIdxPerVertex[vi] = origFi;
				// 记录第一次出现的顺序
				bool found = false;
				for (int x : uniqOrder) if (x == origFi) { found = true; break; }
				if (!found && origFi >= 0) uniqOrder.push_back(origFi);
			}

			if (uniqOrder.size() >= 3)
			{
				// 使用前三个出现的原始面作为 m1/m2/m3（三侧采样方向）
				int fA = uniqOrder[0];
				int fB = uniqOrder[1];
				int fC = uniqOrder[2];
				osg::Vec3 mA = interFaceNormals[fA];
				osg::Vec3 mB = interFaceNormals[fB];
				osg::Vec3 mC = interFaceNormals[fC];
				mA.normalize();
				mB.normalize();
				mC.normalize();
				if (mA * outward < 0.0f) mA = -mA;
				if (mB * outward < 0.0f) mB = -mB;
				if (mC * outward < 0.0f) mC = -mC;

				int base = faceVerts[typeIdx]->size();
				for (int i = 0; i < nVerts; ++i)
				{
					faceVerts[typeIdx]->push_back((*finalVerts)[face[i]]);
					faceNormals[typeIdx]->push_back(normal);

					faceUV0[typeIdx]->push_back(mA);
					faceUV1[typeIdx]->push_back(mB);
					faceUV2[typeIdx]->push_back(mC);

					int src = origIdxPerVertex[i];
					if (src == fA) faceColors[typeIdx]->push_back(cRed);
					else if (src == fB) faceColors[typeIdx]->push_back(cGreen);
					else if (src == fC) faceColors[typeIdx]->push_back(cBlue);
					else faceColors[typeIdx]->push_back(cBlue);
				}

				for (int i = 1; i < nVerts - 1; ++i)
				{
					faceTris[typeIdx]->push_back(base);
					faceTris[typeIdx]->push_back(base + i);
					faceTris[typeIdx]->push_back(base + i + 1);
				}

				continue;
			}
			// 若无法可靠推断三侧原始面则回退到几何法线构造（与之前逻辑兼容）
			if (nVerts >= 3)
			{
				int i0 = 0;
				int i1 = nVerts / 3;
				int i2 = (2 * nVerts) / 3;
				osg::Vec3 a0 = (*finalVerts)[face[i0]];
				osg::Vec3 a1 = (*finalVerts)[face[i1]];
				osg::Vec3 a2 = (*finalVerts)[face[i2]];
				osg::Vec3 m1 = (a1 - a0) ^ (faceCenter - a0);
				osg::Vec3 m2 = (a2 - a1) ^ (faceCenter - a1);
				osg::Vec3 m3 = (a0 - a2) ^ (faceCenter - a2);
				m1.normalize();
				m2.normalize();
				m3.normalize();
				if (m1 * outward < 0.0f) m1 = -m1;
				if (m2 * outward < 0.0f) m2 = -m2;
				if (m3 * outward < 0.0f) m3 = -m3;
				samp0 = m1;
				samp1 = m2;
				samp2 = m3;
			}
			else
			{
				samp0 = normal;
			}
		}

		// 若到这里仍未提前填充（非 edge/vertex 的快速路径），按通用路径填充并根据 typeIdx 决定颜色
		int base = faceVerts[typeIdx]->size();
		for (int i = 0; i < nVerts; ++i)
		{
			faceVerts[typeIdx]->push_back((*finalVerts)[face[i]]);
			faceNormals[typeIdx]->push_back(normal);

			faceUV0[typeIdx]->push_back(samp0);
			faceUV1[typeIdx]->push_back(samp1);
			faceUV2[typeIdx]->push_back(samp2);

			if (typeIdx == 1) // 原面：R=1
			{
				faceColors[typeIdx]->push_back(cRed);
			}
			else if (typeIdx == 2) // 棱面：兜底颜色策略（交替）
			{
				if ((i & 1) == 0) faceColors[typeIdx]->push_back(cRed);
				else faceColors[typeIdx]->push_back(cGreen);
			}
			else // 顶点面：轮询 R,G,B（兜底）
			{
				int mod = i % 3;
				if (mod == 0) faceColors[typeIdx]->push_back(cRed);
				else if (mod == 1) faceColors[typeIdx]->push_back(cGreen);
				else faceColors[typeIdx]->push_back(cBlue);
			}
		}

		for (int i = 1; i < nVerts - 1; ++i)
		{
			faceTris[typeIdx]->push_back(base);
			faceTris[typeIdx]->push_back(base + i);
			faceTris[typeIdx]->push_back(base + i + 1);
		}
	}

	// 创建三个几何体

	*pFaceGeom = new osg::Geometry();   // 原面面
	(*pFaceGeom)->setUseVertexBufferObjects(true);
	(*pFaceGeom)->setUseDisplayList(false);
	(*pFaceGeom)->setVertexArray(faceVerts[1].get());
	(*pFaceGeom)->setNormalArray(faceNormals[1].get(), osg::Array::BIND_PER_VERTEX);
	(*pFaceGeom)->setColorArray(faceColors[1].get(), osg::Array::BIND_PER_VERTEX);
	(*pFaceGeom)->setTexCoordArray(0, faceUV0[1].get());
	(*pFaceGeom)->setTexCoordArray(1, faceUV1[1].get());
	(*pFaceGeom)->setTexCoordArray(2, faceUV2[1].get());
	(*pFaceGeom)->addPrimitiveSet(faceTris[1].get());
	(*pFaceGeom)->setName("Football_Face");

	*pEdgeGeom = new osg::Geometry();   // 棱面
	(*pEdgeGeom)->setUseVertexBufferObjects(true);
	(*pEdgeGeom)->setUseDisplayList(false);
	(*pEdgeGeom)->setVertexArray(faceVerts[2].get());
	(*pEdgeGeom)->setNormalArray(faceNormals[2].get(), osg::Array::BIND_PER_VERTEX);
	(*pEdgeGeom)->setColorArray(faceColors[2].get(), osg::Array::BIND_PER_VERTEX);
	(*pEdgeGeom)->setTexCoordArray(0, faceUV0[2].get());
	(*pEdgeGeom)->setTexCoordArray(1, faceUV1[2].get());
	(*pEdgeGeom)->setTexCoordArray(2, faceUV2[2].get());
	(*pEdgeGeom)->addPrimitiveSet(faceTris[2].get());
	(*pEdgeGeom)->setName("Football_Edge");

	*pVertGeom = new osg::Geometry();   // 顶点面
	(*pVertGeom)->setUseVertexBufferObjects(true);
	(*pVertGeom)->setUseDisplayList(false);
	(*pVertGeom)->setVertexArray(faceVerts[0].get());
	(*pVertGeom)->setNormalArray(faceNormals[0].get(), osg::Array::BIND_PER_VERTEX);
	(*pVertGeom)->setColorArray(faceColors[0].get(), osg::Array::BIND_PER_VERTEX);
	(*pVertGeom)->setTexCoordArray(0, faceUV0[0].get());
	(*pVertGeom)->setTexCoordArray(1, faceUV1[0].get());
	(*pVertGeom)->setTexCoordArray(2, faceUV2[0].get());
	(*pVertGeom)->addPrimitiveSet(faceTris[0].get());
	(*pVertGeom)->setName("Football_Vertex");
}

// ---------- 截角操作（输出普通面） ----------
void CGMVolumeBasic::_TruncatePolyhedron(
	const osg::ref_ptr<osg::Vec3Array>& inputVerts,
	const std::vector<std::vector<int>>& inputFaces,
	float fBevel,
	osg::ref_ptr<osg::Vec3Array>& outputVerts,
	std::vector<std::vector<int>>& outputFaces,
	std::vector<std::pair<int, int>>& outputEdges) const
{
	if (fBevel <= 0.0f || fBevel >= 0.5f) return;  // 无效切角比例
	size_t numVerts = inputVerts->size();
	size_t numFaces = inputFaces.size();

	// 1. 构建边到面的映射
	std::map<std::pair<int, int>, std::vector<int>> edgeToFaces;
	for (size_t i = 0; i < numFaces; ++i) {
		const auto& face = inputFaces[i];
		int k = (int)face.size();
		for (int j = 0; j < k; ++j) {
			int a = face[j];
			int b = face[(j + 1) % k];
			if (a > b) std::swap(a, b);
			edgeToFaces[{a, b}].push_back((int)i);
		}
	}

	// 2. 构建每个顶点的无序邻点列表
	std::vector<std::vector<int>> neighbors(numVerts);
	for (const auto& kv : edgeToFaces) {
		int a = kv.first.first;
		int b = kv.first.second;
		neighbors[a].push_back(b);
		neighbors[b].push_back(a);
	}

	// 3. 为每个顶点建立有序邻点列表（逆时针环绕）
	std::vector<std::vector<int>> orderedNeighbors(numVerts);
	for (size_t v = 0; v < numVerts; ++v) {
		if (neighbors[v].empty()) continue;
		int start = neighbors[v][0];
		std::vector<int> order;
		order.push_back(start);

		auto it = edgeToFaces.find({ min((int)v, start), max((int)v, start) });
		if (it == edgeToFaces.end()) continue;
		int currentFace = it->second[0];
		int prevFace = currentFace;
		int current = start;

		while (true) {
			const auto& face = inputFaces[prevFace];
			int k = (int)face.size();
			int next = -1;
			for (int i = 0; i < k; ++i) {
				if (face[i] == v) {
					int prevIdx = (i - 1 + k) % k;
					int nextIdx = (i + 1) % k;
					if (face[prevIdx] != current && face[prevIdx] != v)
						next = face[prevIdx];
					else if (face[nextIdx] != current && face[nextIdx] != v)
						next = face[nextIdx];
					break;
				}
			}
			if (next == -1 || next == start) break;
			order.push_back(next);

			auto it2 = edgeToFaces.find({ min((int)v, next), max((int)v, next) });
			if (it2 == edgeToFaces.end()) break;
			const auto& faces = it2->second;
			int nextFace = (faces[0] == prevFace) ? faces[1] : faces[0];
			prevFace = nextFace;
			current = next;
		}
		if ((int)order.size() == (int)neighbors[v].size())
			orderedNeighbors[v] = order;
		else
			orderedNeighbors[v] = neighbors[v];
	}

	// 3.5 核心注入：纯拓扑方向校正（保证所有邻点环绕为严格逆时针 CCW）
	for (size_t v = 0; v < numVerts; ++v) {
		if (orderedNeighbors[v].size() < 3) continue;
		int n0 = orderedNeighbors[v][0];
		int n1 = orderedNeighbors[v][1];
		bool is_ccw = false;

		auto it0 = edgeToFaces.find({ min((int)v, n0), max((int)v, n0) });
		auto it1 = edgeToFaces.find({ min((int)v, n1), max((int)v, n1) });
		if (it0 != edgeToFaces.end() && it1 != edgeToFaces.end()) {
			int sharedFace = -1;
			for (int f0 : it0->second) {
				for (int f1 : it1->second) {
					if (f0 == f1) { sharedFace = f0; break; }
				}
				if (sharedFace != -1) break;
			}
			if (sharedFace != -1) {
				const auto& face = inputFaces[sharedFace];
				int k = (int)face.size();
				for (int i = 0; i < k; ++i) {
					if (face[i] == v) {
						int prev = face[(i - 1 + k) % k];
						int next = face[(i + 1) % k];
						// 若在原面中，经过 v 的走向是 n1 -> v -> n0，则顶点环绕 n0 -> n1 为正确的逆时针
						if (prev == n1 && next == n0) {
							is_ccw = true;
						}
						break;
					}
				}
			}
		}
		// 如果拓扑方向相反，直接反转整个邻点数组
		if (!is_ccw) {
			std::reverse(orderedNeighbors[v].begin(), orderedNeighbors[v].end());
		}
	}

	// 4. 为每个顶点建立邻点到索引的映射
	std::vector<std::map<int, int>> neighborIndex(numVerts);
	for (size_t v = 0; v < numVerts; ++v) {
		for (size_t i = 0; i < orderedNeighbors[v].size(); ++i) {
			neighborIndex[v][orderedNeighbors[v][i]] = (int)i;
		}
	}

	// 5. 生成全局新顶点（每条边两个）
	outputVerts = new osg::Vec3Array;
	outputVerts->reserve(edgeToFaces.size() * 2);

	std::vector<std::vector<int>> pointIndex(numVerts);
	for (size_t v = 0; v < numVerts; ++v) {
		pointIndex[v].assign(orderedNeighbors[v].size(), -1);
	}

	for (const auto& kv : edgeToFaces) {
		int a = kv.first.first;
		int b = kv.first.second;
		if (neighborIndex[a].count(b) == 0 || neighborIndex[b].count(a) == 0)
			continue;
		int idx_a_in_b = neighborIndex[b][a];
		int idx_b_in_a = neighborIndex[a][b];

		osg::Vec3 va = (*inputVerts)[a];
		osg::Vec3 vb = (*inputVerts)[b];
		osg::Vec3 pa = va + (vb - va) * fBevel;
		osg::Vec3 pb = vb + (va - vb) * fBevel;

		int idx_a = (int)outputVerts->size();
		outputVerts->push_back(pa);
		int idx_b = (int)outputVerts->size();
		outputVerts->push_back(pb);

		pointIndex[a][idx_b_in_a] = idx_a;
		pointIndex[b][idx_a_in_b] = idx_b;
	}

	auto getPointIdx = [&](int v, int neighbor) -> int {
		int i = neighborIndex[v][neighbor];
		return pointIndex[v][i];
		};

	// 6. 生成新面（普通面）
	outputFaces.clear();
	std::set<std::pair<int, int>> edgeSet;

	// 6.1 顶点面
	for (size_t v = 0; v < numVerts; ++v) {
		if (orderedNeighbors[v].empty()) continue;
		std::vector<int> newFace;
		for (int nb : orderedNeighbors[v]) {
			newFace.push_back(getPointIdx((int)v, nb));
		}
		outputFaces.push_back(newFace);
		for (size_t i = 0; i < newFace.size(); ++i) {
			int a = newFace[i];
			int b = newFace[(i + 1) % newFace.size()];
			if (a > b) std::swap(a, b);
			edgeSet.insert({ a, b });
		}
	}

	// 6.2 原始面
	for (const auto& face : inputFaces) {
		int k = (int)face.size();
		std::vector<int> newFace;
		for (int i = 0; i < k; ++i) {
			int v0 = face[i];
			int v1 = face[(i + 1) % k];
			newFace.push_back(getPointIdx(v0, v1));
			newFace.push_back(getPointIdx(v1, v0));
		}
		outputFaces.push_back(newFace);
		for (size_t i = 0; i < newFace.size(); ++i) {
			int a = newFace[i];
			int b = newFace[(i + 1) % newFace.size()];
			if (a > b) std::swap(a, b);
			edgeSet.insert({ a, b });
		}
	}

	// 7. 输出边集合
	outputEdges.clear();
	outputEdges.reserve(edgeSet.size());
	for (const auto& p : edgeSet) {
		outputEdges.push_back(p);
	}
}

// ---------- 扩张操作（输出带类型的面） ----------
void CGMVolumeBasic::_ExpandPolyhedron(
	const osg::ref_ptr<osg::Vec3Array>& inputVerts,
	const std::vector<std::vector<int>>& inputFaces,
	float fOffset,
	osg::ref_ptr<osg::Vec3Array>& outputVerts,
	std::vector<FaceWithType>& outputFaces,
	std::vector<std::pair<int, int>>& outputEdges) const
{
	size_t numVerts = inputVerts->size();
	size_t numFaces = inputFaces.size();

	// 1. 构建边到面的映射
	std::map<std::pair<int, int>, std::vector<int>> edgeToFaces;
	for (size_t i = 0; i < numFaces; ++i) {
		const auto& face = inputFaces[i];
		int k = (int)face.size();
		for (int j = 0; j < k; ++j) {
			int a = face[j];
			int b = face[(j + 1) % k];
			if (a > b) std::swap(a, b);
			edgeToFaces[{a, b}].push_back((int)i);
		}
	}

	// 2 & 3. 构建无序和有序邻点列表
	std::vector<std::vector<int>> neighbors(numVerts);
	for (const auto& kv : edgeToFaces) {
		int a = kv.first.first;
		int b = kv.first.second;
		neighbors[a].push_back(b);
		neighbors[b].push_back(a);
	}

	std::vector<std::vector<int>> orderedNeighbors(numVerts);
	for (size_t v = 0; v < numVerts; ++v) {
		if (neighbors[v].empty()) continue;
		int start = neighbors[v][0];
		std::vector<int> order;
		order.push_back(start);

		auto it = edgeToFaces.find({ min((int)v, start), max((int)v, start) });
		if (it == edgeToFaces.end()) continue;
		int currentFace = it->second[0];
		int prevFace = currentFace;
		int current = start;

		while (true) {
			const auto& face = inputFaces[prevFace];
			int k = (int)face.size();
			int next = -1;
			for (int i = 0; i < k; ++i) {
				if (face[i] == v) {
					int prevIdx = (i - 1 + k) % k;
					int nextIdx = (i + 1) % k;
					if (face[prevIdx] != current && face[prevIdx] != v)
						next = face[prevIdx];
					else if (face[nextIdx] != current && face[nextIdx] != v)
						next = face[nextIdx];
					break;
				}
			}
			if (next == -1 || next == start) break;
			order.push_back(next);

			auto it2 = edgeToFaces.find({ min((int)v, next), max((int)v, next) });
			if (it2 == edgeToFaces.end()) break;
			const auto& faces = it2->second;
			int nextFace = (faces[0] == prevFace) ? faces[1] : faces[0];
			prevFace = nextFace;
			current = next;
		}
		if ((int)order.size() == (int)neighbors[v].size()) orderedNeighbors[v] = order;
		else orderedNeighbors[v] = neighbors[v];
	}

	// 3.5 核心注入：纯拓扑方向校正（保证所有邻点环绕为严格逆时针 CCW）
	for (size_t v = 0; v < numVerts; ++v) {
		if (orderedNeighbors[v].size() < 3) continue;
		int n0 = orderedNeighbors[v][0];
		int n1 = orderedNeighbors[v][1];
		bool is_ccw = false;

		auto it0 = edgeToFaces.find({ min((int)v, n0), max((int)v, n0) });
		auto it1 = edgeToFaces.find({ min((int)v, n1), max((int)v, n1) });
		if (it0 != edgeToFaces.end() && it1 != edgeToFaces.end()) {
			int sharedFace = -1;
			for (int f0 : it0->second) {
				for (int f1 : it1->second) {
					if (f0 == f1) { sharedFace = f0; break; }
				}
				if (sharedFace != -1) break;
			}
			if (sharedFace != -1) {
				const auto& face = inputFaces[sharedFace];
				int k = (int)face.size();
				for (int i = 0; i < k; ++i) {
					if (face[i] == v) {
						int prev = face[(i - 1 + k) % k];
						int next = face[(i + 1) % k];
						if (prev == n1 && next == n0) is_ccw = true;
						break;
					}
				}
			}
		}
		if (!is_ccw) {
			std::reverse(orderedNeighbors[v].begin(), orderedNeighbors[v].end());
		}
	}

	// 4. 计算每个面的法线（摒弃质心判断，直接信任拓扑！）
	std::vector<osg::Vec3> faceNormals(numFaces);
	for (size_t i = 0; i < numFaces; ++i) {
		const auto& face = inputFaces[i];
		osg::Vec3 a = (*inputVerts)[face[0]];
		osg::Vec3 b = (*inputVerts)[face[1]];
		osg::Vec3 c = (*inputVerts)[face[2]];
		osg::Vec3 n = (b - a) ^ (c - a);
		n.normalize();
		faceNormals[i] = n;
	}

	// 5. 建立顶点到面的映射
	std::vector<std::vector<int>> vertFaces(numVerts);
	for (size_t i = 0; i < numFaces; ++i) {
		for (int v : inputFaces[i])
			vertFaces[v].push_back((int)i);
	}

	// 6. 为每个顶点建立有序面列表
	std::vector<std::vector<int>> orderedFaces(numVerts);
	for (size_t v = 0; v < numVerts; ++v) {
		if (orderedNeighbors[v].empty()) continue;
		const auto& nb = orderedNeighbors[v];
		int m = (int)nb.size();
		std::vector<int> faces;
		for (int i = 0; i < m; ++i) {
			int nb1 = nb[i];
			int nb2 = nb[(i + 1) % m];
			auto it1 = edgeToFaces.find({ min((int)v, nb1), max((int)v, nb1) });
			auto it2 = edgeToFaces.find({ min((int)v, nb2), max((int)v, nb2) });
			if (it1 == edgeToFaces.end() || it2 == edgeToFaces.end()) continue;
			const auto& faces1 = it1->second;
			const auto& faces2 = it2->second;
			int commonFace = -1;
			for (int f1 : faces1) {
				for (int f2 : faces2) {
					if (f1 == f2) { commonFace = f1; break; }
				}
				if (commonFace != -1) break;
			}
			if (commonFace != -1) faces.push_back(commonFace);
		}
		if ((int)faces.size() == m) orderedFaces[v] = faces;
		else orderedFaces[v] = vertFaces[v];
	}

	// 7. 生成新顶点
	outputVerts = new osg::Vec3Array;
	int totalNewPoints = 0;
	for (const auto& face : inputFaces)
		totalNewPoints += (int)face.size();
	outputVerts->reserve(totalNewPoints);

	std::vector<std::vector<int>> faceVertexIdx(numFaces);
	std::vector<std::map<int, int>> vertFaceIdx(numVerts);

	for (size_t fi = 0; fi < numFaces; ++fi) {
		const auto& face = inputFaces[fi];
		int k = (int)face.size();
		faceVertexIdx[fi].resize(k, -1);
		for (int j = 0; j < k; ++j) {
			int v = face[j];
			osg::Vec3 p = (*inputVerts)[v] + faceNormals[fi] * fOffset;
			int idx = (int)outputVerts->size();
			outputVerts->push_back(p);
			faceVertexIdx[fi][j] = idx;
			vertFaceIdx[v][fi] = idx;
		}
	}

	// 8. 生成新面（直接依赖拓扑生成）
	outputFaces.clear();
	std::set<std::pair<int, int>> edgeSet;

	// 8.1 顶点面
	for (size_t v = 0; v < numVerts; ++v) {
		if (orderedFaces[v].empty()) continue;
		std::vector<int> newFace;
		for (int f : orderedFaces[v]) {
			auto it = vertFaceIdx[v].find(f);
			if (it != vertFaceIdx[v].end())
				newFace.push_back(it->second);
		}
		if (newFace.size() < 3) continue;

		outputFaces.push_back({ newFace, FACE_VERTEX });
		for (size_t i = 0; i < newFace.size(); ++i) {
			int a = newFace[i];
			int b = newFace[(i + 1) % newFace.size()];
			if (a > b) std::swap(a, b);
			edgeSet.insert({ a, b });
		}
	}

	// 8.2 原面面
	for (size_t fi = 0; fi < numFaces; ++fi) {
		const auto& idx = faceVertexIdx[fi];
		std::vector<int> newFace = idx;

		outputFaces.push_back({ newFace, FACE_ORIGINAL });
		for (size_t i = 0; i < newFace.size(); ++i) {
			int a = newFace[i];
			int b = newFace[(i + 1) % newFace.size()];
			if (a > b) std::swap(a, b);
			edgeSet.insert({ a, b });
		}
	}

	// 8.3 棱面（使用上一次的完美拓扑解法）
	for (const auto& kv : edgeToFaces) {
		int a = kv.first.first;
		int b = kv.first.second;
		const auto& faceIndices = kv.second;
		if (faceIndices.size() != 2) continue;
		int f0 = faceIndices[0];
		int f1 = faceIndices[1];

		auto it_a_f0 = vertFaceIdx[a].find(f0);
		auto it_a_f1 = vertFaceIdx[a].find(f1);
		auto it_b_f0 = vertFaceIdx[b].find(f0);
		auto it_b_f1 = vertFaceIdx[b].find(f1);
		if (it_a_f0 == vertFaceIdx[a].end() || it_a_f1 == vertFaceIdx[a].end() ||
			it_b_f0 == vertFaceIdx[b].end() || it_b_f1 == vertFaceIdx[b].end())
			continue;

		int p0 = it_a_f0->second;
		int p1 = it_a_f1->second;
		int p2 = it_b_f1->second;
		int p3 = it_b_f0->second;

		bool f0_has_ab = false;
		const auto& origFace0 = inputFaces[f0];
		int k = (int)origFace0.size();
		for (int i = 0; i < k; ++i) {
			if (origFace0[i] == a && origFace0[(i + 1) % k] == b) {
				f0_has_ab = true;
				break;
			}
		}

		std::vector<int> quad;
		if (f0_has_ab) quad = { p0, p1, p2, p3 };
		else quad = { p0, p3, p2, p1 };

		outputFaces.push_back({ quad, FACE_EDGE });
		for (int i = 0; i < 4; ++i) {
			int x = quad[i];
			int y = quad[(i + 1) % 4];
			if (x > y) std::swap(x, y);
			edgeSet.insert({ x, y });
		}
	}

	// 9. 输出边集合
	outputEdges.clear();
	outputEdges.reserve(edgeSet.size());
	for (const auto& p : edgeSet)
		outputEdges.push_back(p);
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
	m_rayMarchAlphaTex->setInternalFormat(GL_R8);
	m_rayMarchAlphaTex->setSourceFormat(GL_RED);
	m_rayMarchAlphaTex->setSourceType(GL_UNSIGNED_BYTE);
	m_rayMarchAlphaTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	m_rayMarchAlphaTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
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

//void CGMVolumeBasic::_SetPixelLength(const float fFovy, const int iHeight)
//{
//	if (!m_fPixelLengthUniform.valid()) return;
//
//	float fFovyRadian = osg::DegreesToRadians(fFovy);
//	float fPixelLength = tan(fFovyRadian * 0.5) / (iHeight * 0.5);
//	m_fPixelLengthUniform->set(fPixelLength);
//}

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