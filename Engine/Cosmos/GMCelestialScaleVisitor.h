//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMCelestialScaleVisitor.h
/// @brief		Galaxy-Music Engine - CGMCelestialScaleVisitor
/// @version	1.0
/// @author		LiuTao
/// @date		2023.12.02
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <osg/CoordinateSystemNode>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/NodeVisitor>

namespace GM
{
	class CGMCelestialScaleVisitor : public osg::NodeVisitor
	{
	public:
		CGMCelestialScaleVisitor(): NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN){}

		void SetTileLevel(const float fTileLevel)
		{
			_fTileLevel = fTileLevel;
		}
		void SetQuatorFace(const int iQuatorFaceID)
		{
			_iQuatorFaceID = iQuatorFaceID;
		}
		void SetRadius(const double fEquator, const double fPolar)
		{
			ellipsoid.setRadiusEquator(fEquator);
			ellipsoid.setRadiusPolar(fPolar);
		}

		void apply(osg::Node& node) { traverse(node); }
		void apply(osg::Geode& node) { traverse(node); }
		void apply(osg::Geometry& geom)
		{
			osg::Vec3Array* pVert = dynamic_cast<osg::Vec3Array*>(geom.getVertexArray());
			osg::Vec3Array* pNorm = dynamic_cast<osg::Vec3Array*>(geom.getNormalArray());
			// 0层纹理单元 xy = WGS84对应的UV，[0.0, 1.0]
			// 1层纹理单元 xy = 瓦片体贴图UV，[0.0, 1.0]; z = 面对应的编号0-23
			osg::Vec2Array* pCoord0 = dynamic_cast<osg::Vec2Array*>(geom.getTexCoordArray(0));
			osg::Vec3Array* pCoord1 = dynamic_cast<osg::Vec3Array*>(geom.getTexCoordArray(1));

			if (!pVert || !pNorm || !pCoord0 || !pCoord1) return;

			// 每条边上的顶点数
			int iVertEdgeNum = sqrt(float(pVert->size()));
			for (int i = 0; i < pVert->size(); i++)
			{
				if (0.0f < _fTileLevel) 
					pCoord1->at(i).z() = _iQuatorFaceID;

				double fLon = (pCoord0->at(i).x() - 0.5) * osg::PI * 2;
				double fLat = (pCoord0->at(i).y() - 0.5) * osg::PI;
				double fCosLat = cos(fLat);
				// 经纬度转椭球面上的位置
				double fX, fY, fZ;
				ellipsoid.convertLatLongHeightToXYZ(fLat, fLon, 0, fX, fY, fZ);

				pVert->at(i) = osg::Vec3(fX, fY, fZ);
				pNorm->at(i) = osg::Vec3(cos(fLon) * fCosLat, sin(fLon) * fCosLat, sin(fLat));
			}

			geom.setUseVertexBufferObjects(true);
			geom.setUseDisplayList(false);
			geom.setDataVariance(osg::Object::STATIC);

			pVert->dirty();
			pNorm->dirty();
			if (0.0f < _fTileLevel) pCoord1->dirty();
			geom.dirtyBound();

			traverse(geom);
		}

	private:
		osg::EllipsoidModel ellipsoid;
		// 瓦片层级（0.5、1.0、1.5、2.0、2.5...）
		// 0.0级瓦片是六面体细分后的球体的一个面
		// 0.5级瓦片是0.0级瓦片的四分之一，1.0级瓦片是0.5级瓦片细分一次
		// 1.5级瓦片是1.0级瓦片的四分之一，2.0级瓦片是1.5级瓦片细分一次
		float _fTileLevel = 0.0f;
		// 瓦片体对应的编号 0-23
		int _iQuatorFaceID = 0;
	};

}	// GM