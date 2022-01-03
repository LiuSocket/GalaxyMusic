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
	* ������ݷ�Χ
	* @author LiuTao
	* @since 2020.12.13
	* @param fXMin, fXMax			����
	* @param fYMin, fYMax			���
	* @param fZMin, fZMax			�߶�
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
	* �ǳ�����ṹ��
	* @author LiuTao
	* @since 2021.06.20
	* @param iX, iY, iZ			�ǳ��ռ�����[-GM_COORD_MAX, GM_COORD_MAX]
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
	* ��Ƶ�ռ�����ṹ��
	* @author LiuTao
	* @since 2021.07.04
	* @param radius:	��Ƶ�뾶 == ��Ƶ�Ľ��ģ��뾶Խ�󣬽���Խ�� (0,1)
	* @param angle:		��ת�Ƕ� == ��Ƶ������ [0,2*PI)
	*	angle == 0 �� (2*PI)		�˷�
	*	angle == 0.5*PI				����
	*	angle == PI					����
	*	angle == 1.5*PI				����
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
	* ��Ƶ���ݽṹ��
	* @author LiuTao
	* @since 2021.07.04
	* @param name:				��Ƶ�ļ�����
	* @param audioCoord:		��Ƶ�ռ�����
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
