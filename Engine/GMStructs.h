//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMStructs.h
/// @brief		Galaxy-Music Engine -Structs
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.09
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GMPrerequisites.h"

namespace GM
{
	/*************************************************************************
	 Macro Defines
	*************************************************************************/
	#define GM_COORD_MAX					(1024)
	#define GM_COORD_MAX_2					(2048)
	#define GM_COORD_MAX_16					(1024*16)
	/*************************************************************************
	 Enums
	*************************************************************************/

	/*************************************************************************
	 Classes
	*************************************************************************/

	/*************************************************************************
	 Structs
	*************************************************************************/

	/**
	* 体积数据范围
	* @author LiuTao
	* @since 2020.12.13
	* @param fXMin, fXMax			长度
	* @param fYMin, fYMax			宽度
	* @param fZMin, fZMax			高度
	*/
	struct SGMVolumeRange
	{
		SGMVolumeRange()
			:fXMin(-5.0f), fXMax(5.0f), fYMin(-5.0f), fYMax(5.0f), fZMin(-0.5f), fZMax(0.5f)
		{
		}
		float					fXMin;
		float					fXMax;
		float					fYMin;
		float					fYMax;
		float					fZMin;
		float					fZMax;
	};

	/**
	* 星辰坐标结构体
	* @author LiuTao
	* @since 2021.06.20
	* @param iX, iY, iZ			星辰空间坐标[-GM_COORD_MAX, GM_COORD_MAX]
	*/
	struct SGMStarCoord
	{
		SGMStarCoord() : x(0), y(0), z(0) {}
		SGMStarCoord(const int iX, const int iY, const int iZ = 0) : x(iX), y(iY), z(iZ) {}
		friend bool operator == (const SGMStarCoord &A, const SGMStarCoord &B);
		friend bool operator != (const SGMStarCoord &A, const SGMStarCoord &B);
		friend bool operator < (const SGMStarCoord &A, const SGMStarCoord &B);

		int x;
		int y;
		int z;
	};

	inline bool operator == (const SGMStarCoord& A, const SGMStarCoord& B)
	{
		return A.x == B.x && A.y == B.y && A.z == B.z;
	}

	inline bool operator != (const SGMStarCoord& A, const SGMStarCoord& B)
	{
		return A.x != B.x || A.y != B.y || A.z != B.z;
	}

	inline bool operator < (const SGMStarCoord& A, const SGMStarCoord& B)
	{
		return A.x < B.x || (A.x == B.x && A.y < B.y) || (A.x == B.x && A.y == B.y && A.z < B.z);
	}

	/**
	* 音频空间坐标结构体
	* @author LiuTao
	* @since 2021.07.04
	* @param radius:	音频半径 == 音频的节拍，半径越大，节拍越慢 (0,1)
	* @param angle:		旋转角度 == 音频的类型 [0,2*PI)
	*	angle == 0 或 (2*PI)		兴奋
	*	angle == 0.5*PI				快乐
	*	angle == PI					放松
	*	angle == 1.5*PI				悲伤
	*/
	struct SGMAudioCoord
	{
		SGMAudioCoord() : radius(0.5), angle(0.0) {}
		SGMAudioCoord(const double fRadius, const double fAngle) : radius(fRadius), angle(fAngle) {}
		friend bool operator < (const SGMAudioCoord &A, const SGMAudioCoord &B);

		double radius;
		double angle;
	};

	inline bool operator < (const SGMAudioCoord& A, const SGMAudioCoord& B)
	{
		return A.angle < B.angle || (A.angle == B.angle && A.radius < B.radius);
	}

	/**
	* 音频数据结构体
	* @author LiuTao
	* @since 2021.07.04
	* @param name:				音频文件名称
	* @param audioCoord:		音频空间坐标
	*/
	struct SGMAudioData
	{
		SGMAudioData() : name(L""), audioCoord(SGMAudioCoord()) {}
		SGMAudioData(const std::wstring& wstrName, const SGMAudioCoord& sAudioCoord) :
			name(wstrName), audioCoord(sAudioCoord) {}

		std::wstring name;
		SGMAudioCoord audioCoord;
	};

}	// GM
