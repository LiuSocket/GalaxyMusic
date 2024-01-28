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
		CGMCelestialScaleVisitor(): NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

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
			osg::Vec2Array* pCoord0 = dynamic_cast<osg::Vec2Array*>(geom.getTexCoordArray(0));
			if (!pVert || !pNorm || !pCoord0) return;

			for (int i = 0; i < pVert->size(); i++)
			{
				// 原始球面上的位置转经纬度
				double fLat = (pCoord0->at(i).y() - 0.5) * osg::PI;
				double fLon = (pCoord0->at(i).x() - 0.5) * osg::PI * 2;
				double fCosLat = cos(fLat);
				// 经纬度转椭球面上的位置
				double fX, fY, fZ;
				ellipsoid.convertLatLongHeightToXYZ(fLat, fLon, 0, fX, fY, fZ);

				pVert->at(i) = osg::Vec3(fX, fY, fZ);
				pNorm->at(i) = osg::Vec3d(cos(fLon) * fCosLat, sin(fLon) * fCosLat, sin(fLat));
			}

			geom.setUseVertexBufferObjects(true);
			geom.setUseDisplayList(false);
			geom.setDataVariance(osg::Object::STATIC);

			pVert->dirty();
			pNorm->dirty();
			geom.dirtyBound();

			traverse(geom);
		}

	private:
		osg::EllipsoidModel ellipsoid;
	};

}	// GM