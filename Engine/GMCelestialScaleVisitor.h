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

			osg::Vec2Array* pV2Coord0 = dynamic_cast<osg::Vec2Array*>(geom.getTexCoordArray(0));
			osg::Vec3Array* pV3Coord0 = nullptr;
			bool bQuator = false;// 默认不是四分之一地形块，而是全球地形块
			if (!pV2Coord0)
			{
				// 如果0号纹理单元不是二维纹理坐标，那么就是三维纹理坐标
				pV3Coord0 = dynamic_cast<osg::Vec3Array*>(geom.getTexCoordArray(0));
				bQuator = true;
			}

			if (!pVert || !pNorm || (!pV2Coord0 && !pV3Coord0)) return;

			for (int i = 0; i < pVert->size(); i++)
			{
				// 原始球面上的位置转经纬度
				double fLat,fLon;
				if (bQuator) // 四分之一地形块，分为极地和赤道两种情况
				{
					// 0层纹理单元 xy = 六面体贴图UV，[0.0, 1.0]；z= 纬度，[-PI/2, PI/2];
					fLat = pV3Coord0->at(i).z();
					osg::Vec2d v2 = osg::Vec2d(pV3Coord0->at(i).x(), pV3Coord0->at(i).y());
					v2.normalize();
					fLon = atan2(v2.y(), v2.x());
				}
				else // 全球地形块
				{
					fLat = (pV2Coord0->at(i).y() - 0.5) * osg::PI;
					fLon = (pV2Coord0->at(i).x() - 0.5) * osg::PI * 2;
				}

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