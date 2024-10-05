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

		void SetTileLevel(const int iTileLevel)
		{
			_iTileLevel = iTileLevel;
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

			// 如果是瓦片体，重新计算顶点位置和纹理坐标
			osg::Vec2Array* pNewCoord0 =  new osg::Vec2Array();
			pNewCoord0->resize(pVert->size());

			// 每条边上的顶点数
			int iVertEdgeNum = sqrt(float(pVert->size()));
			for (int i = 0; i < pVert->size(); i++)
			{
				osg::Vec2d vCoord = pCoord0->at(i);
				if (0 <= _iTileLevel)
				{
					int iX = i % iVertEdgeNum;
					int iY = i / iVertEdgeNum;
					int iAddX = 2 * (iVertEdgeNum / 2 - iX) - 1;
					// X轴对称点的索引
					int iInvX = i + iAddX;
					// Y轴对称点的索引
					int iInvY = (iY + 2 * (iVertEdgeNum / 2 - iY) - 1) * iVertEdgeNum + iX;
					// 中心对称点的索引
					int iInvXY = iInvY + iAddX;

					// 四分之面体的0层纹理单元不是WGS84对应的最终UV，会根据ID变化，所以这里要重新计算
					// 赤道地形块的ID为0-15，北极地形块的ID为16-19，南极地形块的ID为20-23
					// 6个面一共24块地形，每个面内部，按照一二三四象限顺序依次编号
					//	1/5/9/13/17/21		|		0/4/8/12/16/20
					//		第二象限		|			第一象限
					//----------------------+-----------------------
					//		第三象限		|			第四象限
					//	2/6/10/14/18/22		|		3/7/11/15/19/23
					switch (_iQuatorFaceID)
					{
					case 0: // 第一象限 posX 赤道非洲
						vCoord.x() = pCoord0->at(i).x();
						vCoord.y() = pCoord0->at(i).y();
						break;
					case 4: // 第一象限 negX 赤道太平洋
						vCoord.x() = pCoord0->at(i).x() - 0.5;
						vCoord.y() = pCoord0->at(i).y();
						break;
					case 8: // 第一象限 posY 赤道印尼
						vCoord.x() = pCoord0->at(i).x() + 0.25;
						vCoord.y() = pCoord0->at(i).y();
						break;
					case 12: // 第一象限 negY 赤道美洲
						vCoord.x() = pCoord0->at(i).x() - 0.25;
						vCoord.y() = pCoord0->at(i).y();
						break;
					case 16: // 第一象限 posZ 北极
						vCoord.x() = pCoord0->at(i).x();
						vCoord.y() = pCoord0->at(i).y();
						break;
					case 20: // 第一象限 negZ 南极
						vCoord.x() = 1.5 - pCoord0->at(i).x();
						vCoord.y() = 1.0 - pCoord0->at(i).y();
						break;
					///////////////////////////////////////////////////////////
					case 1: // 第二象限 posX 赤道大西洋
						vCoord.x() = 1.0 - pCoord0->at(iInvX).x();
						vCoord.y() = pCoord0->at(iInvX).y();
						break;
					case 5: // 第二象限 negX 赤道太平洋
						vCoord.x() = 1.5 - pCoord0->at(iInvX).x();
						vCoord.y() = pCoord0->at(iInvX).y();
						break;
					case 9: // 第二象限 posY 赤道印尼
						vCoord.x() = 1.25 - pCoord0->at(iInvX).x();
						vCoord.y() = pCoord0->at(iInvX).y();
						break;
					case 13: // 第二象限 negY 赤道美洲
						vCoord.x() = 0.75 - pCoord0->at(iInvX).x();
						vCoord.y() = pCoord0->at(iInvX).y();
						break;
					case 17: // 第二象限 posZ 北极
						vCoord.x() = 1.0 - pCoord0->at(iInvX).x();
						vCoord.y() = pCoord0->at(iInvX).y();
						break;
					case 21: // 第二象限 negZ 南极
						vCoord.x() = pCoord0->at(iInvX).x() - 0.5;
						vCoord.y() = 1.0 - pCoord0->at(iInvX).y();
						break;
					///////////////////////////////////////////////////////////
					case 2: // 第三象限 posX 赤道大西洋
						vCoord.x() = 1.0 - pCoord0->at(iInvXY).x();
						vCoord.y() = 1.0 - pCoord0->at(iInvXY).y();
						break;
					case 6: // 第三象限 negX 赤道太平洋
						vCoord.x() = 1.5 - pCoord0->at(iInvXY).x();
						vCoord.y() = 1.0 - pCoord0->at(iInvXY).y();
						break;
					case 10: // 第三象限 posY 赤道印尼
						vCoord.x() = 1.25 - pCoord0->at(iInvXY).x();
						vCoord.y() = 1.0 - pCoord0->at(iInvXY).y();
						break;
					case 14: // 第三象限 negY 赤道美洲
						vCoord.x() = 0.75 - pCoord0->at(iInvXY).x();
						vCoord.y() = 1.0 - pCoord0->at(iInvXY).y();
						break;
					case 18: // 第三象限 posZ 北极
						vCoord.x() = pCoord0->at(iInvXY).x() - 0.5;
						vCoord.y() = pCoord0->at(iInvXY).y();
						break;
					case 22: // 第三象限 negZ 南极
						vCoord.x() = 1.0 - pCoord0->at(iInvXY).x();
						vCoord.y() = 1.0 - pCoord0->at(iInvXY).y();
						break;
					///////////////////////////////////////////////////////////
					case 3: // 第四象限 posX 赤道大西洋
						vCoord.x() = pCoord0->at(iInvY).x();
						vCoord.y() = 1.0 - pCoord0->at(iInvY).y();
						break;
					case 7: // 第四象限 negX 赤道太平洋
						vCoord.x() = pCoord0->at(iInvY).x() - 0.5;
						vCoord.y() = 1.0 - pCoord0->at(iInvY).y();
						break;
					case 11: // 第四象限 posY 赤道印尼
						vCoord.x() = pCoord0->at(iInvY).x() + 0.25;
						vCoord.y() = 1.0 - pCoord0->at(iInvY).y();
						break;
					case 15: // 第四象限 negY 赤道美洲
						vCoord.x() = pCoord0->at(iInvY).x() - 0.25;
						vCoord.y() = 1.0 - pCoord0->at(iInvY).y();
						break;
					case 19: // 第四象限 posZ 北极
						vCoord.x() = 1.5 - pCoord0->at(iInvY).x();
						vCoord.y() = pCoord0->at(iInvY).y();
						break;
					case 23: // 第四象限 negZ 南极
						vCoord.x() = pCoord0->at(iInvY).x();
						vCoord.y() = 1.0 - pCoord0->at(iInvY).y();
						break;
					///////////////////////////////////////////////////////////
					default:
						break;
					}
					pNewCoord0->at(i) = vCoord;
					pCoord1->at(i).z() = _iQuatorFaceID;
				}

				double fLon = (vCoord.x() - 0.5) * osg::PI * 2;
				double fLat = (vCoord.y() - 0.5) * osg::PI;
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
			if (0 <= _iTileLevel)
			{
				geom.setTexCoordArray(0, pNewCoord0);
				pCoord1->dirty();
			}
			geom.dirtyBound();

			traverse(geom);
		}

	private:
		osg::EllipsoidModel ellipsoid;
		// 瓦片层级，-1表示六面体的一个面
		// 0级瓦片是六面体细分后的球体的一个面的四分之一，1级瓦片是0级瓦片细分一次的结果，以此类推
		int _iTileLevel = -1;
		// 瓦片体对应的编号 0-23
		int _iQuatorFaceID = 0;
	};

}	// GM