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

	#define LIGHTYEAR					9.461e15	// 1����

	/*************************************************************************
	 Enums
	*************************************************************************/

	/*************************************************************************
	 Classes
	*************************************************************************/

	/*************************************************************************
	 Structs
	*************************************************************************/
	struct SGMVector2;
	struct SGMVector2i;
	struct SGMVector2f;

	struct SGMVector3;
	struct SGMVector3i;
	struct SGMVector3f;

	struct SGMVector4;
	struct SGMVector4i;
	struct SGMVector4f;

	/*!
	*  @struct SGMVector2f
	*  @brief VrEarth Vector2f
	*/
	struct SGMVector2f
	{
		/** @brief ���� */
		SGMVector2f(float _x = 0, float _y = 0);
		/** @brief == ���� */
		bool operator == (const SGMVector2f& _sVec) const;
		/** @brief != ���� */
		bool operator != (const SGMVector2f& _sVec) const;
		/** @brief < ���� */
		bool operator < (const SGMVector2f& _sVec) const;
		/** @brief > ���� */
		bool operator > (const SGMVector2f& _sVec) const;
		/** @brief <= ���� */
		bool operator <= (const SGMVector2f& _sVec) const;
		/** @brief >= ���� */
		bool operator >= (const SGMVector2f& _sVec) const;
		/** @brief ָ�� */
		float * ptr();
		/** @brief ָ�� */
		const float * ptr() const;
		/** @brief [] ���� */
		float& operator [] (int i);
		/** @brief [] ���� */
		float operator [] (int i) const;
		/** @brief * ���� */
		float operator * (const SGMVector2f& _sVec) const;
		/** @brief * ���� */
		const SGMVector2f operator * (float _fValue) const;
		/** @brief *= ���� */
		SGMVector2f& operator *= (float _fValue);
		/** @brief / ���� */
		const SGMVector2f operator / (float _fValue) const;
		/** @brief /= ���� */
		SGMVector2f& operator /= (float _fValue);
		/** @brief + ���� */
		const SGMVector2f operator + (const SGMVector2f& _sVec) const;
		/** @brief += ���� */
		SGMVector2f& operator += (const SGMVector2f& _sVec);
		/** @brief - ���� */
		const SGMVector2f operator - (const SGMVector2f& _sVec) const;
		/** @brief -= ���� */
		SGMVector2f& operator -= (const SGMVector2f& _sVec);
		/** @brief ȡ�� ���� */
		const SGMVector2f operator - () const;
		/** @brief ���� */
		float Length() const;
		/** @brief ����ƽ�� */
		float SquaredLength() const;
		/** @brief ֱ�߾��� */
		float Distance(const SGMVector2f& _sVec) const;
		/** @brief ֱ�߾���ƽ�� */
		float SquaredDistance(const SGMVector2f& _sVec) const;
		/** @brief �淶�� */
		float Normalize();
		/** @brief ��� */
		SGMVector2f ComponentMultiply(const SGMVector2f& _sVecL, const SGMVector2f& _sVecR);
		/** @brief ��� */
		SGMVector2f ComponentDivide(const SGMVector2f& _sVecL, const SGMVector2f& _sVecR);
		/** @brief תInt */
		SGMVector2i ToInt();
		/** @brief תDouble */
		SGMVector2 ToDouble();

		// ����
		union
		{
			struct
			{
				float				x;
				float				y;
			};
			struct
			{
				float				w;
				float				h;
			};
			struct
			{
				float				lon;		//!< ����
				float				lat;		//!< γ��
			};
			struct
			{
				float				_v[2];
			};
		};
	};

	/*!
	 *  @struct SGMVector2
	 *  @brief VrEarth Vector2
	 */
	struct SGMVector2
	{
		/** @brief ���� */
		SGMVector2(double _x = 0, double _y = 0);
		/** @brief ���� */
		SGMVector2(const SGMVector2f vec);
		/** @brief ����Vector2f */
		operator SGMVector2f() const;
		/** @brief == ���� */
		bool operator == (const SGMVector2& _sVec) const;
		/** @brief != ���� */
		bool operator != (const SGMVector2& _sVec) const;
		/** @brief < ���� */
		bool operator < (const SGMVector2& _sVec) const;
		/** @brief > ���� */
		bool operator > (const SGMVector2& _sVec) const;
		/** @brief <= ���� */
		bool operator <= (const SGMVector2& _sVec) const;
		/** @brief >= ���� */
		bool operator >= (const SGMVector2& _sVec) const;
		/** @brief ָ�� */
		double * ptr();
		/** @brief ָ�� */
		const double * ptr() const;
		/** @brief [] ���� */
		double& operator [] (int i);
		/** @brief [] ���� */
		double operator [] (int i) const;
		/** @brief * ���� */
		double operator * (const SGMVector2& _sVec) const;
		/** @brief * ���� */
		const SGMVector2 operator * (double _fValue) const;
		/** @brief *= ���� */
		SGMVector2& operator *= (double _fValue);
		/** @brief / ���� */
		const SGMVector2 operator / (double _fValue) const;
		/** @brief /= ���� */
		SGMVector2& operator /= (double _fValue);
		/** @brief + ���� */
		const SGMVector2 operator + (const SGMVector2& _sVec) const;
		/** @brief += ���� */
		SGMVector2& operator += (const SGMVector2& _sVec);
		/** @brief - ���� */
		const SGMVector2 operator - (const SGMVector2& _sVec) const;
		/** @brief -= ���� */
		SGMVector2& operator -= (const SGMVector2& _sVec);
		/** @brief ȡ�� ���� */
		const SGMVector2 operator - () const;
		/** @brief ���� */
		double Length() const;
		/** @brief ����ƽ�� */
		double SquaredLength() const;
		/** @brief ֱ�߾��� */
		double Distance(const SGMVector2& _sVec) const;
		/** @brief ֱ�߾���ƽ�� */
		double SquaredDistance(const SGMVector2& _sVec) const;
		/** @brief �淶�� */
		double Normalize();
		/** @brief ��� */
		SGMVector2 ComponentMultiply(const SGMVector2& _sVecL, const SGMVector2& _sVecR);
		/** @brief ��� */
		SGMVector2 ComponentDivide(const SGMVector2& _sVecL, const SGMVector2& _sVecR);
		/** @brief תInt */
		SGMVector2i ToInt();
		/** @brief תFloat */
		SGMVector2f ToFloat();

		// ����
		union
		{
			struct
			{
				double				x;
				double				y;
			};
			struct
			{
				double				w;
				double				h;
			};
			struct
			{
				double				lon;		//!< ����
				double				lat;		//!< γ��
			};
			struct
			{
				double				_v[2];
			};
		};
	};

	/*!
	*  @struct SGMVector2i
	*  @brief VrEarth Vector2i
	*/
	struct SGMVector2i
	{
		/** @brief ���� */
		SGMVector2i(int _x = 0, int _y = 0);
		/** @brief == ���� */
		bool operator == (const SGMVector2i& _sVec) const;
		/** @brief != ���� */
		bool operator != (const SGMVector2i& _sVec) const;
		/** @brief < ���� */
		bool operator < (const SGMVector2i& _sVec) const;
		/** @brief > ���� */
		bool operator > (const SGMVector2i& _sVec) const;
		/** @brief <= ���� */
		bool operator <= (const SGMVector2i& _sVec) const;
		/** @brief >= ���� */
		bool operator >= (const SGMVector2i& _sVec) const;
		/** @brief ָ�� */
		int * ptr();
		/** @brief ָ�� */
		const int * ptr() const;
		/** @brief [] ���� */
		int& operator [] (int i);
		/** @brief [] ���� */
		int operator [] (int i) const;
		/** @brief * ���� */
		int operator * (const SGMVector2i& _sVec) const;
		/** @brief * ���� */
		const SGMVector2i operator * (int _fValue) const;
		/** @brief *= ���� */
		SGMVector2i& operator *= (int _fValue);
		/** @brief / ���� */
		const SGMVector2i operator / (int _fValue) const;
		/** @brief /= ���� */
		SGMVector2i& operator /= (int _fValue);
		/** @brief + ���� */
		const SGMVector2i operator + (const SGMVector2i& _sVec) const;
		/** @brief += ���� */
		SGMVector2i& operator += (const SGMVector2i& _sVec);
		/** @brief - ���� */
		const SGMVector2i operator - (const SGMVector2i& _sVec) const;
		/** @brief -= ���� */
		SGMVector2i& operator -= (const SGMVector2i& _sVec);
		/** @brief ȡ�� ���� */
		const SGMVector2i operator - () const;
		/** @brief ���� */
		double Length() const;
		/** @brief ����ƽ�� */
		int SquaredLength() const;
		/** @brief ֱ�߾��� */
		double Distance(const SGMVector2i& _sVec) const;
		/** @brief ֱ�߾���ƽ�� */
		int SquaredDistance(const SGMVector2i& _sVec) const;
		/** @brief ��� */
		SGMVector2i ComponentMultiply(const SGMVector2i& _sVecL, const SGMVector2i& _sVecR);
		/** @brief ��� */
		SGMVector2i ComponentDivide(const SGMVector2i& _sVecL, const SGMVector2i& _sVecR);
		/** @brief תFloat */
		SGMVector2f ToFloat();
		/** @brief תDouble */
		SGMVector2 ToDouble();

		// ����
		union
		{
			struct
			{
				int				x;
				int				y;
			};
			struct
			{
				int				w;
				int				h;
			};
			struct
			{
				int				_v[2];
			};
		};
	};

	/*!
	*  @struct SGMVector3f
	*  @brief VrEarth Vector3f
	*/
	struct SGMVector3f
	{
		/** @brief ���� */
		SGMVector3f(float _x = 0, float _y = 0, float _z = 0);
		/** @brief ���� */
		SGMVector3f(SGMVector2f _sVec2, float _z = 0);
		/** @brief ==���� */
		bool operator == (const SGMVector3f& _sVec) const;
		/** @brief !=���� */
		bool operator != (const SGMVector3f& _sVec) const;
		/** @brief < ���� */
		bool operator < (const SGMVector3f& _sVec) const;
		/** @brief > ���� */
		bool operator > (const SGMVector3f& _sVec) const;
		/** @brief <= ���� */
		bool operator <= (const SGMVector3f& _sVec) const;
		/** @brief >= ���� */
		bool operator >= (const SGMVector3f& _sVec) const;
		/** @brief ָ�� */
		float * ptr();
		/** @brief ָ�� */
		const float * ptr() const;
		/** @brief [] ���� */
		float& operator [] (int i);
		/** @brief [] ���� */
		float operator [] (int i) const;
		/** @brief * ���� */
		float operator * (const SGMVector3f& _sVec) const;
		/** @brief ^ ���� */
		const SGMVector3f operator ^ (const SGMVector3f& _sVec) const;
		/** @brief * ���� */
		const SGMVector3f operator * (float _fValue) const;
		/** @brief *= ���� */
		SGMVector3f& operator *= (float _fValue);
		/** @brief / ���� */
		const SGMVector3f operator / (float _fValue) const;
		/** @brief /= ���� */
		SGMVector3f& operator /= (float _fValue);
		/** @brief + ���� */
		const SGMVector3f operator + (const SGMVector3f& _sVec) const;
		/** @brief += ���� */
		SGMVector3f& operator += (const SGMVector3f& _sVec);
		/** @brief - ���� */
		const SGMVector3f operator - (const SGMVector3f& _sVec) const;
		/** @brief -= ���� */
		SGMVector3f& operator -= (const SGMVector3f& _sVec);
		/** @brief ȡ�� ���� */
		const SGMVector3f operator - () const;
		/** @brief ���� */
		float Length() const;
		/** @brief ����ƽ�� */
		float SquaredLength() const;
		/** @brief ֱ�߾��� */
		float Distance(const SGMVector3f& _sVec) const;
		/** @brief ֱ�߾���ƽ�� */
		float SquaredDistance(const SGMVector3f& _sVec) const;
		/** @brief �淶�� */
		float Normalize();
		/** @brief ��� */
		SGMVector3f ComponentMultiply(const SGMVector3f& _sVecL, const SGMVector3f& _sVecR);
		/** @brief ��� */
		SGMVector3f ComponentDivide(const SGMVector3f& _sVecL, const SGMVector3f& _sVecR);
		/** @brief תInt */
		SGMVector3i ToInt();
		/** @brief תDouble */
		SGMVector3 ToDouble();

		// ����
		union
		{
			struct
			{
				float				x;
				float				y;
				float				z;
			};
			struct
			{
				float				lon;		//!< ����
				float				lat;		//!< γ��
				float				alt;		//!< �߶�
			};
			struct
			{
				float				_v[3];
			};
		};
	};

	/*!
	 *  @struct SGMVector3
	 *  @brief VrEarth Vector3
	 */
	struct SGMVector3
	{
		/** @brief ���� */
		SGMVector3(double _x = 0, double _y = 0, double _z = 0);
		/** @brief ���� */
		SGMVector3(SGMVector2 _sVec2, double _z = 0);
		/** @brief ���� */
		SGMVector3(const SGMVector3f vec);
		/** @brief ����Vector3f */
		operator SGMVector3f() const;
		/** @brief ==���� */
		bool operator == (const SGMVector3& _sVec) const;
		/** @brief !=���� */
		bool operator != (const SGMVector3& _sVec) const;
		/** @brief < ���� */
		bool operator < (const SGMVector3& _sVec) const;
		/** @brief > ���� */
		bool operator > (const SGMVector3& _sVec) const;
		/** @brief <= ���� */
		bool operator <= (const SGMVector3& _sVec) const;
		/** @brief >= ���� */
		bool operator >= (const SGMVector3& _sVec) const;
		/** @brief ָ�� */
		double * ptr();
		/** @brief ָ�� */
		const double * ptr() const;
		/** @brief [] ���� */
		double& operator [] (int i);
		/** @brief [] ���� */
		double operator [] (int i) const;
		/** @brief * ���� */
		double operator * (const SGMVector3& _sVec) const;
		/** @brief ^ ���� */
		const SGMVector3 operator ^ (const SGMVector3& _sVec) const;
		/** @brief * ���� */
		const SGMVector3 operator * (double _fValue) const;
		/** @brief *= ���� */
		SGMVector3& operator *= (double _fValue);
		/** @brief / ���� */
		const SGMVector3 operator / (double _fValue) const;
		/** @brief /= ���� */
		SGMVector3& operator /= (double _fValue);
		/** @brief + ���� */
		const SGMVector3 operator + (const SGMVector3& _sVec) const;
		/** @brief += ���� */
		SGMVector3& operator += (const SGMVector3& _sVec);
		/** @brief - ���� */
		const SGMVector3 operator - (const SGMVector3& _sVec) const;
		/** @brief -= ���� */
		SGMVector3& operator -= (const SGMVector3& _sVec);
		/** @brief ȡ�� ���� */
		const SGMVector3 operator - () const;
		/** @brief ���� */
		double Length() const;
		/** @brief ����ƽ�� */
		double SquaredLength() const;
		/** @brief ֱ�߾��� */
		double Distance(const SGMVector3& _sVec) const;
		/** @brief ֱ�߾���ƽ�� */
		double SquaredDistance(const SGMVector3& _sVec) const;
		/** @brief �淶�� */
		double Normalize();
		/** @brief ��� */
		SGMVector3 ComponentMultiply(const SGMVector3& _sVecL, const SGMVector3& _sVecR);
		/** @brief ��� */
		SGMVector3 ComponentDivide(const SGMVector3& _sVecL, const SGMVector3& _sVecR);
		/** @brief תInt */
		SGMVector3i ToInt();
		/** @brief תFloat */
		SGMVector3f ToFloat();

		// ����
		union
		{
			struct
			{
				double				x;
				double				y;
				double				z;
			};
			struct
			{
				double				lon;		//!< ����
				double				lat;		//!< γ��
				double				alt;		//!< �߶�
			};
			struct
			{
				double				_v[3];
			};
		};
	};

	/*!
	*  @struct SGMVector3i
	*  @brief VrEarth Vector3i
	*/
	struct SGMVector3i
	{
		/** @brief ���� */
		SGMVector3i(int _x = 0, int _y = 0, int _z = 0);
		/** @brief ���� */
		SGMVector3i(SGMVector2i _sVec2, int _z = 0);
		/** @brief ==���� */
		bool operator == (const SGMVector3i& _sVec) const;
		/** @brief !=���� */
		bool operator != (const SGMVector3i& _sVec) const;
		/** @brief < ���� */
		bool operator < (const SGMVector3i& _sVec) const;
		/** @brief > ���� */
		bool operator > (const SGMVector3i& _sVec) const;
		/** @brief <= ���� */
		bool operator <= (const SGMVector3i& _sVec) const;
		/** @brief >= ���� */
		bool operator >= (const SGMVector3i& _sVec) const;
		/** @brief ָ�� */
		int * ptr();
		/** @brief ָ�� */
		const int * ptr() const;
		/** @brief [] ���� */
		int& operator [] (int i);
		/** @brief [] ���� */
		int operator [] (int i) const;
		/** @brief * ���� */
		int operator * (const SGMVector3i& _sVec) const;
		/** @brief ^ ���� */
		const SGMVector3i operator ^ (const SGMVector3i& _sVec) const;
		/** @brief * ���� */
		const SGMVector3i operator * (int _fValue) const;
		/** @brief *= ���� */
		SGMVector3i& operator *= (int _fValue);
		/** @brief / ���� */
		const SGMVector3i operator / (int _fValue) const;
		/** @brief /= ���� */
		SGMVector3i& operator /= (int _fValue);
		/** @brief + ���� */
		const SGMVector3i operator + (const SGMVector3i& _sVec) const;
		/** @brief += ���� */
		SGMVector3i& operator += (const SGMVector3i& _sVec);
		/** @brief - ���� */
		const SGMVector3i operator - (const SGMVector3i& _sVec) const;
		/** @brief -= ���� */
		SGMVector3i& operator -= (const SGMVector3i& _sVec);
		/** @brief ȡ�� ���� */
		const SGMVector3i operator - () const;
		/** @brief ���� */
		double Length() const;
		/** @brief ����ƽ�� */
		int SquaredLength() const;
		/** @brief ֱ�߾��� */
		double Distance(const SGMVector3i& _sVec) const;
		/** @brief ֱ�߾���ƽ�� */
		int SquaredDistance(const SGMVector3i& _sVec) const;
		/** @brief ��� */
		SGMVector3i ComponentMultiply(const SGMVector3i& _sVecL, const SGMVector3i& _sVecR);
		/** @brief ��� */
		SGMVector3i ComponentDivide(const SGMVector3i& _sVecL, const SGMVector3i& _sVecR);
		/** @brief תFloat */
		SGMVector3f ToFloat();
		/** @brief תDouble */
		SGMVector3 ToDouble();

		// ����
		union
		{
			struct
			{
				int				x;
				int				y;
				int				z;
			};
			struct
			{
				int				lon;		//!< ����
				int				lat;		//!< γ��
				int				alt;		//!< �߶�
			};
			struct
			{
				int				_v[3];
			};
		};
	};

	/*!
	*  @struct SGMVector4f
	*  @brief VrEarth Vector4
	*/
	struct SGMVector4f
	{
		/** @brief ���� */
		SGMVector4f(float _x = 0, float _y = 0, float _z = 0, float _w = 0);
		/** @brief ���� */
		SGMVector4f(unsigned nColorValue);
		/** @brief ���� */
		SGMVector4f(SGMVector3f _sVec3, float _w = 0);
		/** @brief ==���� */
		bool operator == (const SGMVector4f& _sVec) const;
		/** @brief !=���� */
		bool operator != (const SGMVector4f& _sVec) const;
		/** @brief < ���� */
		bool operator < (const SGMVector4f& _sVec) const;
		/** @brief > ���� */
		bool operator > (const SGMVector4f& _sVec) const;
		/** @brief <= ���� */
		bool operator <= (const SGMVector4f& _sVec) const;
		/** @brief >= ���� */
		bool operator >= (const SGMVector4f& _sVec) const;
		/** @brief ָ�� */
		float * ptr();
		/** @brief ָ�� */
		const float * ptr() const;
		/** @brief [] ���� */
		float& operator [] (int i);
		/** @brief [] ���� */
		float operator [] (int i) const;
		/** @brief * ���� */
		float operator * (const SGMVector4f& _sVec) const;
		/** @brief * ���� */
		const SGMVector4f operator * (float _fValue) const;
		/** @brief *= ���� */
		SGMVector4f& operator *= (float _fValue);
		/** @brief / ���� */
		const SGMVector4f operator / (float _fValue) const;
		/** @brief /= ���� */
		SGMVector4f& operator /= (float _fValue);
		/** @brief + ���� */
		const SGMVector4f operator + (const SGMVector4f& _sVec) const;
		/** @brief += ���� */
		SGMVector4f& operator += (const SGMVector4f& _sVec);
		/** @brief - ���� */
		const SGMVector4f operator - (const SGMVector4f& _sVec) const;
		/** @brief -= ���� */
		SGMVector4f& operator -= (const SGMVector4f& _sVec);
		/** @brief ȡ�� ���� */
		const SGMVector4f operator - () const;
		/** @brief ���� */
		float Length() const;
		/** @brief ����ƽ�� */
		float SquaredLength() const;
		/** @brief ֱ�߾��� */
		float Distance(const SGMVector4f& _sVec) const;
		/** @brief ֱ�߾���ƽ�� */
		float SquaredDistance(const SGMVector4f& _sVec) const;
		/** @brief �淶�� */
		float Normalize();
		/** @brief ��� */
		SGMVector4f ComponentMultiply(const SGMVector4f& _sVecL, const SGMVector4f& _sVecR);
		/** @brief ��� */
		SGMVector4f ComponentDivide(const SGMVector4f& _sVecL, const SGMVector4f& _sVecR);
		/** @brief תInt */
		SGMVector4i ToInt();
		/** @brief תDouble */
		SGMVector4 ToDouble();

		// ����
		union
		{
			struct
			{
				float				x;
				float				y;
				float				z;
				float				w;
			};
			struct
			{
				float				r;
				float				g;
				float				b;
				float				a;
			};
			struct
			{
				float				_v[4];
			};
		};
		
	};

	struct SGMVector4
	{
		/** @brief ���� */
		SGMVector4(double _x = 0, double _y = 0, double _z = 0, double _w = 0);
		/** @brief ���� */
		SGMVector4(unsigned nColorValue);
		/** @brief ���� */
		SGMVector4(SGMVector3 _sVec3, double _w = 0);
		/** @brief ���� */
		SGMVector4(const SGMVector4f vec);
		/** @brief ����Vector4f */
		operator SGMVector4f() const;
		/** @brief ==���� */
		bool operator == (const SGMVector4& _sVec) const;
		/** @brief !=���� */
		bool operator != (const SGMVector4& _sVec) const;
		/** @brief < ���� */
		bool operator < (const SGMVector4& _sVec) const;
		/** @brief > ���� */
		bool operator > (const SGMVector4& _sVec) const;
		/** @brief <= ���� */
		bool operator <= (const SGMVector4& _sVec) const;
		/** @brief >= ���� */
		bool operator >= (const SGMVector4& _sVec) const;
		/** @brief ָ�� */
		double * ptr();
		/** @brief ָ�� */
		const double * ptr() const;
		/** @brief [] ���� */
		double& operator [] (int i);
		/** @brief [] ���� */
		double operator [] (int i) const;
		/** @brief * ���� */
		double operator * (const SGMVector4& _sVec) const;
		/** @brief * ���� */
		const SGMVector4 operator * (double _fValue) const;
		/** @brief *= ���� */
		SGMVector4& operator *= (double _fValue);
		/** @brief / ���� */
		const SGMVector4 operator / (double _fValue) const;
		/** @brief /= ���� */
		SGMVector4& operator /= (double _fValue);
		/** @brief + ���� */
		const SGMVector4 operator + (const SGMVector4& _sVec) const;
		/** @brief += ���� */
		SGMVector4& operator += (const SGMVector4& _sVec);
		/** @brief - ���� */
		const SGMVector4 operator - (const SGMVector4& _sVec) const;
		/** @brief -= ���� */
		SGMVector4& operator -= (const SGMVector4& _sVec);
		/** @brief ȡ�� ���� */
		const SGMVector4 operator - () const;
		/** @brief ���� */
		double Length() const;
		/** @brief ����ƽ�� */
		double SquaredLength() const;
		/** @brief ֱ�߾��� */
		double Distance(const SGMVector4& _sVec) const;
		/** @brief ֱ�߾���ƽ�� */
		double SquaredDistance(const SGMVector4& _sVec) const;
		/** @brief �淶�� */
		double Normalize();
		/** @brief ��� */
		SGMVector4 ComponentMultiply(const SGMVector4& _sVecL, const SGMVector4& _sVecR);
		/** @brief ��� */
		SGMVector4 ComponentDivide(const SGMVector4& _sVecL, const SGMVector4& _sVecR);
		/** @brief תInt */
		SGMVector4i ToInt();
		/** @brief תFloat */
		SGMVector4f ToFloat();

		// ����
		union
		{
			struct
			{
				double				x;
				double				y;
				double				z;
				double				w;
			};
			struct
			{
				double				r;
				double				g;
				double				b;
				double				a;
			};
			struct
			{
				double				_v[4];
			};
		};
	};

	/*!
	*  @struct SGMVector4i
	*  @brief VrEarth Vector4
	*/
	struct SGMVector4i
	{
		/** @brief ���� */
		SGMVector4i(int _x = 0, int _y = 0, int _z = 0, int _w = 0);
		/** @brief ���� */
		SGMVector4i(unsigned nColorValue);
		/** @brief ���� */
		SGMVector4i(SGMVector3i _sVec3, int _w = 0);
		/** @brief ==���� */
		bool operator == (const SGMVector4i& _sVec) const;
		/** @brief !=���� */
		bool operator != (const SGMVector4i& _sVec) const;
		/** @brief < ���� */
		bool operator < (const SGMVector4i& _sVec) const;
		/** @brief > ���� */
		bool operator > (const SGMVector4i& _sVec) const;
		/** @brief <= ���� */
		bool operator <= (const SGMVector4i& _sVec) const;
		/** @brief >= ���� */
		bool operator >= (const SGMVector4i& _sVec) const;
		/** @brief ָ�� */
		int * ptr();
		/** @brief ָ�� */
		const int * ptr() const;
		/** @brief [] ���� */
		int& operator [] (int i);
		/** @brief [] ���� */
		int operator [] (int i) const;
		/** @brief * ���� */
		int operator * (const SGMVector4i& _sVec) const;
		/** @brief * ���� */
		const SGMVector4i operator * (int _fValue) const;
		/** @brief *= ���� */
		SGMVector4i& operator *= (int _fValue);
		/** @brief / ���� */
		const SGMVector4i operator / (int _fValue) const;
		/** @brief /= ���� */
		SGMVector4i& operator /= (int _fValue);
		/** @brief + ���� */
		const SGMVector4i operator + (const SGMVector4i& _sVec) const;
		/** @brief += ���� */
		SGMVector4i& operator += (const SGMVector4i& _sVec);
		/** @brief - ���� */
		const SGMVector4i operator - (const SGMVector4i& _sVec) const;
		/** @brief -= ���� */
		SGMVector4i& operator -= (const SGMVector4i& _sVec);
		/** @brief ȡ�� ���� */
		const SGMVector4i operator - () const;
		/** @brief ���� */
		double Length() const;
		/** @brief ����ƽ�� */
		int SquaredLength() const;
		/** @brief ֱ�߾��� */
		double Distance(const SGMVector4i& _sVec) const;
		/** @brief ֱ�߾���ƽ�� */
		int SquaredDistance(const SGMVector4i& _sVec) const;
		/** @brief ��� */
		SGMVector4i ComponentMultiply(const SGMVector4i& _sVecL, const SGMVector4i& _sVecR);
		/** @brief ��� */
		SGMVector4i ComponentDivide(const SGMVector4i& _sVecL, const SGMVector4i& _sVecR);
		/** @brief תDouble */
		SGMVector4 ToDouble();
		/** @brief תFloat */
		SGMVector4f ToFloat();

		// ����
		union
		{
			struct
			{
				int				x;
				int				y;
				int				z;
				int				w;
			};
			struct
			{
				int				r;
				int				g;
				int				b;
				int				a;
			};
			struct
			{
				int				_v[4];
			};
		};
	};

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
	* ��������ṹ��
	* @author LiuTao
	* @since 2021.06.20
	* @param fX, fY, fZ			���ӿռ�����[-1.0, 1.0]^3,����λ��Ϊ(0,0,0)
	 */
	struct SGMGalaxyCoord
	{
		SGMGalaxyCoord() : x(0.0f), y(0.0f), z(0.0f) {}
		SGMGalaxyCoord(const float fX, const float fY, const float fZ = 0.0f) : x(fX), y(fY), z(fZ) {}
		friend bool operator == (const SGMGalaxyCoord &A, const SGMGalaxyCoord &B);
		friend bool operator != (const SGMGalaxyCoord &A, const SGMGalaxyCoord &B);
		friend bool operator < (const SGMGalaxyCoord &A, const SGMGalaxyCoord &B);

		float x;
		float y;
		float z;
	};

	inline bool operator == (const SGMGalaxyCoord& A, const SGMGalaxyCoord& B)
	{
		return A.x == B.x && A.y == B.y && A.z == B.z;
	}

	inline bool operator != (const SGMGalaxyCoord& A, const SGMGalaxyCoord& B)
	{
		return A.x != B.x || A.y != B.y || A.z != B.z;
	}

	inline bool operator < (const SGMGalaxyCoord& A, const SGMGalaxyCoord& B)
	{
		return A.x < B.x || (A.x == B.x && A.y < B.y) || (A.x == B.x && A.y == B.y && A.z < B.z);
	}

	/**
	* ��Ƶ�ռ�����ṹ��
	* @author LiuTao
	* @since 2021.07.04
	* @param BPM:		��Ƶ��BPM��Beat Per Minute��ÿ���ӽ�����
						û�н������ƵBPM == 0.0
	* @param angle:		�����Ƕ� == ��Ƶ������ [0.0,2*PI)
	* @param rank:		������������ȫ��ͬ����������õ���Ƶ��ţ�Ĭ��0
	*	angle == 0 �� (2*PI)		�˷ܣ�ŭ, ��
	*	angle == 0.5*PI				���֣�ϲ����
	*	angle == PI					���ɣ��֣���
	*	angle == 1.5*PI				���ˣ�������
	*/
	struct SGMAudioCoord
	{
		SGMAudioCoord() : BPM(0.0), angle(0.0), rank(0){}
		SGMAudioCoord(const double fBPM, const double fAngle, const int iID = 0) :
			BPM(fBPM), angle(fAngle), rank(iID)
		{}
		friend bool operator == (const SGMAudioCoord &A, const SGMAudioCoord &B);
		friend bool operator != (const SGMAudioCoord &A, const SGMAudioCoord &B);
		friend bool operator < (const SGMAudioCoord &A, const SGMAudioCoord &B);

		double BPM;
		double angle;
		int rank;
	};

	inline bool operator == (const SGMAudioCoord& A, const SGMAudioCoord& B)
	{
		return A.BPM == B.BPM && A.angle == B.angle && A.rank == B.rank;
	}

	inline bool operator != (const SGMAudioCoord& A, const SGMAudioCoord& B)
	{
		return A.BPM != B.BPM || A.angle != B.angle || A.rank != B.rank;
	}

	inline bool operator < (const SGMAudioCoord& A, const SGMAudioCoord& B)
	{
		return A.BPM < B.BPM
			|| (A.BPM == B.BPM && A.angle < B.angle)
			|| (A.BPM == B.BPM && A.angle == B.angle && A.rank < B.rank);
	}

	/**
	* ��Ƶ���ݽṹ��
	* @author LiuTao
	* @since 2021.07.04
	* @param iUID:				��ƵUID��0Ϊ�Ƿ�����Χ[1,16777216]
	* @param name:				��Ƶ�ļ�����
	* @param audioCoord:		��Ƶ�ռ�����
	* @param galaxyCoord:		�������꣬[-1,1]
	*/
	struct SGMAudioData
	{
		SGMAudioData() :
			UID(0),
			name(L""),
			audioCoord(SGMAudioCoord()),
			galaxyCoord(SGMGalaxyCoord()) {}

		SGMAudioData(
			unsigned int iUID,
			const std::wstring& wstrName,
			const SGMAudioCoord& sAudioCoord,
			const SGMGalaxyCoord& sGalaxyCoord) :
			UID(iUID),
			name(wstrName),
			audioCoord(sAudioCoord),
			galaxyCoord(sGalaxyCoord) {}

		unsigned int UID;
		std::wstring name;
		SGMAudioCoord audioCoord;
		SGMGalaxyCoord galaxyCoord;
	};

}	// GM
