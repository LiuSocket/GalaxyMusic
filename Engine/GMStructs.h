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

	#define LIGHTYEAR					9.461e15	// 1光年

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
		/** @brief 构造 */
		SGMVector2f(float _x = 0, float _y = 0);
		/** @brief == 重载 */
		bool operator == (const SGMVector2f& _sVec) const;
		/** @brief != 重载 */
		bool operator != (const SGMVector2f& _sVec) const;
		/** @brief < 重载 */
		bool operator < (const SGMVector2f& _sVec) const;
		/** @brief > 重载 */
		bool operator > (const SGMVector2f& _sVec) const;
		/** @brief <= 重载 */
		bool operator <= (const SGMVector2f& _sVec) const;
		/** @brief >= 重载 */
		bool operator >= (const SGMVector2f& _sVec) const;
		/** @brief 指针 */
		float * ptr();
		/** @brief 指针 */
		const float * ptr() const;
		/** @brief [] 重载 */
		float& operator [] (int i);
		/** @brief [] 重载 */
		float operator [] (int i) const;
		/** @brief * 重载 */
		float operator * (const SGMVector2f& _sVec) const;
		/** @brief * 重载 */
		const SGMVector2f operator * (float _fValue) const;
		/** @brief *= 重载 */
		SGMVector2f& operator *= (float _fValue);
		/** @brief / 重载 */
		const SGMVector2f operator / (float _fValue) const;
		/** @brief /= 重载 */
		SGMVector2f& operator /= (float _fValue);
		/** @brief + 重载 */
		const SGMVector2f operator + (const SGMVector2f& _sVec) const;
		/** @brief += 重载 */
		SGMVector2f& operator += (const SGMVector2f& _sVec);
		/** @brief - 重载 */
		const SGMVector2f operator - (const SGMVector2f& _sVec) const;
		/** @brief -= 重载 */
		SGMVector2f& operator -= (const SGMVector2f& _sVec);
		/** @brief 取反 重载 */
		const SGMVector2f operator - () const;
		/** @brief 长度 */
		float Length() const;
		/** @brief 长度平方 */
		float SquaredLength() const;
		/** @brief 直线距离 */
		float Distance(const SGMVector2f& _sVec) const;
		/** @brief 直线距离平方 */
		float SquaredDistance(const SGMVector2f& _sVec) const;
		/** @brief 规范化 */
		float Normalize();
		/** @brief 相乘 */
		SGMVector2f ComponentMultiply(const SGMVector2f& _sVecL, const SGMVector2f& _sVecR);
		/** @brief 相除 */
		SGMVector2f ComponentDivide(const SGMVector2f& _sVecL, const SGMVector2f& _sVecR);
		/** @brief 转Int */
		SGMVector2i ToInt();
		/** @brief 转Double */
		SGMVector2 ToDouble();

		// 变量
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
				float				lon;		//!< 经度
				float				lat;		//!< 纬度
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
		/** @brief 构造 */
		SGMVector2(double _x = 0, double _y = 0);
		/** @brief 构造 */
		SGMVector2(const SGMVector2f vec);
		/** @brief 返回Vector2f */
		operator SGMVector2f() const;
		/** @brief == 重载 */
		bool operator == (const SGMVector2& _sVec) const;
		/** @brief != 重载 */
		bool operator != (const SGMVector2& _sVec) const;
		/** @brief < 重载 */
		bool operator < (const SGMVector2& _sVec) const;
		/** @brief > 重载 */
		bool operator > (const SGMVector2& _sVec) const;
		/** @brief <= 重载 */
		bool operator <= (const SGMVector2& _sVec) const;
		/** @brief >= 重载 */
		bool operator >= (const SGMVector2& _sVec) const;
		/** @brief 指针 */
		double * ptr();
		/** @brief 指针 */
		const double * ptr() const;
		/** @brief [] 重载 */
		double& operator [] (int i);
		/** @brief [] 重载 */
		double operator [] (int i) const;
		/** @brief * 重载 */
		double operator * (const SGMVector2& _sVec) const;
		/** @brief * 重载 */
		const SGMVector2 operator * (double _fValue) const;
		/** @brief *= 重载 */
		SGMVector2& operator *= (double _fValue);
		/** @brief / 重载 */
		const SGMVector2 operator / (double _fValue) const;
		/** @brief /= 重载 */
		SGMVector2& operator /= (double _fValue);
		/** @brief + 重载 */
		const SGMVector2 operator + (const SGMVector2& _sVec) const;
		/** @brief += 重载 */
		SGMVector2& operator += (const SGMVector2& _sVec);
		/** @brief - 重载 */
		const SGMVector2 operator - (const SGMVector2& _sVec) const;
		/** @brief -= 重载 */
		SGMVector2& operator -= (const SGMVector2& _sVec);
		/** @brief 取反 重载 */
		const SGMVector2 operator - () const;
		/** @brief 长度 */
		double Length() const;
		/** @brief 长度平方 */
		double SquaredLength() const;
		/** @brief 直线距离 */
		double Distance(const SGMVector2& _sVec) const;
		/** @brief 直线距离平方 */
		double SquaredDistance(const SGMVector2& _sVec) const;
		/** @brief 规范化 */
		double Normalize();
		/** @brief 相乘 */
		SGMVector2 ComponentMultiply(const SGMVector2& _sVecL, const SGMVector2& _sVecR);
		/** @brief 相除 */
		SGMVector2 ComponentDivide(const SGMVector2& _sVecL, const SGMVector2& _sVecR);
		/** @brief 转Int */
		SGMVector2i ToInt();
		/** @brief 转Float */
		SGMVector2f ToFloat();

		// 变量
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
				double				lon;		//!< 经度
				double				lat;		//!< 纬度
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
		/** @brief 构造 */
		SGMVector2i(int _x = 0, int _y = 0);
		/** @brief == 重载 */
		bool operator == (const SGMVector2i& _sVec) const;
		/** @brief != 重载 */
		bool operator != (const SGMVector2i& _sVec) const;
		/** @brief < 重载 */
		bool operator < (const SGMVector2i& _sVec) const;
		/** @brief > 重载 */
		bool operator > (const SGMVector2i& _sVec) const;
		/** @brief <= 重载 */
		bool operator <= (const SGMVector2i& _sVec) const;
		/** @brief >= 重载 */
		bool operator >= (const SGMVector2i& _sVec) const;
		/** @brief 指针 */
		int * ptr();
		/** @brief 指针 */
		const int * ptr() const;
		/** @brief [] 重载 */
		int& operator [] (int i);
		/** @brief [] 重载 */
		int operator [] (int i) const;
		/** @brief * 重载 */
		int operator * (const SGMVector2i& _sVec) const;
		/** @brief * 重载 */
		const SGMVector2i operator * (int _fValue) const;
		/** @brief *= 重载 */
		SGMVector2i& operator *= (int _fValue);
		/** @brief / 重载 */
		const SGMVector2i operator / (int _fValue) const;
		/** @brief /= 重载 */
		SGMVector2i& operator /= (int _fValue);
		/** @brief + 重载 */
		const SGMVector2i operator + (const SGMVector2i& _sVec) const;
		/** @brief += 重载 */
		SGMVector2i& operator += (const SGMVector2i& _sVec);
		/** @brief - 重载 */
		const SGMVector2i operator - (const SGMVector2i& _sVec) const;
		/** @brief -= 重载 */
		SGMVector2i& operator -= (const SGMVector2i& _sVec);
		/** @brief 取反 重载 */
		const SGMVector2i operator - () const;
		/** @brief 长度 */
		double Length() const;
		/** @brief 长度平方 */
		int SquaredLength() const;
		/** @brief 直线距离 */
		double Distance(const SGMVector2i& _sVec) const;
		/** @brief 直线距离平方 */
		int SquaredDistance(const SGMVector2i& _sVec) const;
		/** @brief 相乘 */
		SGMVector2i ComponentMultiply(const SGMVector2i& _sVecL, const SGMVector2i& _sVecR);
		/** @brief 相除 */
		SGMVector2i ComponentDivide(const SGMVector2i& _sVecL, const SGMVector2i& _sVecR);
		/** @brief 转Float */
		SGMVector2f ToFloat();
		/** @brief 转Double */
		SGMVector2 ToDouble();

		// 变量
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
		/** @brief 构造 */
		SGMVector3f(float _x = 0, float _y = 0, float _z = 0);
		/** @brief 构造 */
		SGMVector3f(SGMVector2f _sVec2, float _z = 0);
		/** @brief ==重载 */
		bool operator == (const SGMVector3f& _sVec) const;
		/** @brief !=重载 */
		bool operator != (const SGMVector3f& _sVec) const;
		/** @brief < 重载 */
		bool operator < (const SGMVector3f& _sVec) const;
		/** @brief > 重载 */
		bool operator > (const SGMVector3f& _sVec) const;
		/** @brief <= 重载 */
		bool operator <= (const SGMVector3f& _sVec) const;
		/** @brief >= 重载 */
		bool operator >= (const SGMVector3f& _sVec) const;
		/** @brief 指针 */
		float * ptr();
		/** @brief 指针 */
		const float * ptr() const;
		/** @brief [] 重载 */
		float& operator [] (int i);
		/** @brief [] 重载 */
		float operator [] (int i) const;
		/** @brief * 重载 */
		float operator * (const SGMVector3f& _sVec) const;
		/** @brief ^ 重载 */
		const SGMVector3f operator ^ (const SGMVector3f& _sVec) const;
		/** @brief * 重载 */
		const SGMVector3f operator * (float _fValue) const;
		/** @brief *= 重载 */
		SGMVector3f& operator *= (float _fValue);
		/** @brief / 重载 */
		const SGMVector3f operator / (float _fValue) const;
		/** @brief /= 重载 */
		SGMVector3f& operator /= (float _fValue);
		/** @brief + 重载 */
		const SGMVector3f operator + (const SGMVector3f& _sVec) const;
		/** @brief += 重载 */
		SGMVector3f& operator += (const SGMVector3f& _sVec);
		/** @brief - 重载 */
		const SGMVector3f operator - (const SGMVector3f& _sVec) const;
		/** @brief -= 重载 */
		SGMVector3f& operator -= (const SGMVector3f& _sVec);
		/** @brief 取反 重载 */
		const SGMVector3f operator - () const;
		/** @brief 长度 */
		float Length() const;
		/** @brief 长度平方 */
		float SquaredLength() const;
		/** @brief 直线距离 */
		float Distance(const SGMVector3f& _sVec) const;
		/** @brief 直线距离平方 */
		float SquaredDistance(const SGMVector3f& _sVec) const;
		/** @brief 规范化 */
		float Normalize();
		/** @brief 相乘 */
		SGMVector3f ComponentMultiply(const SGMVector3f& _sVecL, const SGMVector3f& _sVecR);
		/** @brief 相除 */
		SGMVector3f ComponentDivide(const SGMVector3f& _sVecL, const SGMVector3f& _sVecR);
		/** @brief 转Int */
		SGMVector3i ToInt();
		/** @brief 转Double */
		SGMVector3 ToDouble();

		// 变量
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
				float				lon;		//!< 经度
				float				lat;		//!< 纬度
				float				alt;		//!< 高度
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
		/** @brief 构造 */
		SGMVector3(double _x = 0, double _y = 0, double _z = 0);
		/** @brief 构造 */
		SGMVector3(SGMVector2 _sVec2, double _z = 0);
		/** @brief 构造 */
		SGMVector3(const SGMVector3f vec);
		/** @brief 返回Vector3f */
		operator SGMVector3f() const;
		/** @brief ==重载 */
		bool operator == (const SGMVector3& _sVec) const;
		/** @brief !=重载 */
		bool operator != (const SGMVector3& _sVec) const;
		/** @brief < 重载 */
		bool operator < (const SGMVector3& _sVec) const;
		/** @brief > 重载 */
		bool operator > (const SGMVector3& _sVec) const;
		/** @brief <= 重载 */
		bool operator <= (const SGMVector3& _sVec) const;
		/** @brief >= 重载 */
		bool operator >= (const SGMVector3& _sVec) const;
		/** @brief 指针 */
		double * ptr();
		/** @brief 指针 */
		const double * ptr() const;
		/** @brief [] 重载 */
		double& operator [] (int i);
		/** @brief [] 重载 */
		double operator [] (int i) const;
		/** @brief * 重载 */
		double operator * (const SGMVector3& _sVec) const;
		/** @brief ^ 重载 */
		const SGMVector3 operator ^ (const SGMVector3& _sVec) const;
		/** @brief * 重载 */
		const SGMVector3 operator * (double _fValue) const;
		/** @brief *= 重载 */
		SGMVector3& operator *= (double _fValue);
		/** @brief / 重载 */
		const SGMVector3 operator / (double _fValue) const;
		/** @brief /= 重载 */
		SGMVector3& operator /= (double _fValue);
		/** @brief + 重载 */
		const SGMVector3 operator + (const SGMVector3& _sVec) const;
		/** @brief += 重载 */
		SGMVector3& operator += (const SGMVector3& _sVec);
		/** @brief - 重载 */
		const SGMVector3 operator - (const SGMVector3& _sVec) const;
		/** @brief -= 重载 */
		SGMVector3& operator -= (const SGMVector3& _sVec);
		/** @brief 取反 重载 */
		const SGMVector3 operator - () const;
		/** @brief 长度 */
		double Length() const;
		/** @brief 长度平方 */
		double SquaredLength() const;
		/** @brief 直线距离 */
		double Distance(const SGMVector3& _sVec) const;
		/** @brief 直线距离平方 */
		double SquaredDistance(const SGMVector3& _sVec) const;
		/** @brief 规范化 */
		double Normalize();
		/** @brief 相乘 */
		SGMVector3 ComponentMultiply(const SGMVector3& _sVecL, const SGMVector3& _sVecR);
		/** @brief 相除 */
		SGMVector3 ComponentDivide(const SGMVector3& _sVecL, const SGMVector3& _sVecR);
		/** @brief 转Int */
		SGMVector3i ToInt();
		/** @brief 转Float */
		SGMVector3f ToFloat();

		// 变量
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
				double				lon;		//!< 经度
				double				lat;		//!< 纬度
				double				alt;		//!< 高度
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
		/** @brief 构造 */
		SGMVector3i(int _x = 0, int _y = 0, int _z = 0);
		/** @brief 构造 */
		SGMVector3i(SGMVector2i _sVec2, int _z = 0);
		/** @brief ==重载 */
		bool operator == (const SGMVector3i& _sVec) const;
		/** @brief !=重载 */
		bool operator != (const SGMVector3i& _sVec) const;
		/** @brief < 重载 */
		bool operator < (const SGMVector3i& _sVec) const;
		/** @brief > 重载 */
		bool operator > (const SGMVector3i& _sVec) const;
		/** @brief <= 重载 */
		bool operator <= (const SGMVector3i& _sVec) const;
		/** @brief >= 重载 */
		bool operator >= (const SGMVector3i& _sVec) const;
		/** @brief 指针 */
		int * ptr();
		/** @brief 指针 */
		const int * ptr() const;
		/** @brief [] 重载 */
		int& operator [] (int i);
		/** @brief [] 重载 */
		int operator [] (int i) const;
		/** @brief * 重载 */
		int operator * (const SGMVector3i& _sVec) const;
		/** @brief ^ 重载 */
		const SGMVector3i operator ^ (const SGMVector3i& _sVec) const;
		/** @brief * 重载 */
		const SGMVector3i operator * (int _fValue) const;
		/** @brief *= 重载 */
		SGMVector3i& operator *= (int _fValue);
		/** @brief / 重载 */
		const SGMVector3i operator / (int _fValue) const;
		/** @brief /= 重载 */
		SGMVector3i& operator /= (int _fValue);
		/** @brief + 重载 */
		const SGMVector3i operator + (const SGMVector3i& _sVec) const;
		/** @brief += 重载 */
		SGMVector3i& operator += (const SGMVector3i& _sVec);
		/** @brief - 重载 */
		const SGMVector3i operator - (const SGMVector3i& _sVec) const;
		/** @brief -= 重载 */
		SGMVector3i& operator -= (const SGMVector3i& _sVec);
		/** @brief 取反 重载 */
		const SGMVector3i operator - () const;
		/** @brief 长度 */
		double Length() const;
		/** @brief 长度平方 */
		int SquaredLength() const;
		/** @brief 直线距离 */
		double Distance(const SGMVector3i& _sVec) const;
		/** @brief 直线距离平方 */
		int SquaredDistance(const SGMVector3i& _sVec) const;
		/** @brief 相乘 */
		SGMVector3i ComponentMultiply(const SGMVector3i& _sVecL, const SGMVector3i& _sVecR);
		/** @brief 相除 */
		SGMVector3i ComponentDivide(const SGMVector3i& _sVecL, const SGMVector3i& _sVecR);
		/** @brief 转Float */
		SGMVector3f ToFloat();
		/** @brief 转Double */
		SGMVector3 ToDouble();

		// 变量
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
				int				lon;		//!< 经度
				int				lat;		//!< 纬度
				int				alt;		//!< 高度
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
		/** @brief 构造 */
		SGMVector4f(float _x = 0, float _y = 0, float _z = 0, float _w = 0);
		/** @brief 构造 */
		SGMVector4f(unsigned nColorValue);
		/** @brief 构造 */
		SGMVector4f(SGMVector3f _sVec3, float _w = 0);
		/** @brief ==重载 */
		bool operator == (const SGMVector4f& _sVec) const;
		/** @brief !=重载 */
		bool operator != (const SGMVector4f& _sVec) const;
		/** @brief < 重载 */
		bool operator < (const SGMVector4f& _sVec) const;
		/** @brief > 重载 */
		bool operator > (const SGMVector4f& _sVec) const;
		/** @brief <= 重载 */
		bool operator <= (const SGMVector4f& _sVec) const;
		/** @brief >= 重载 */
		bool operator >= (const SGMVector4f& _sVec) const;
		/** @brief 指针 */
		float * ptr();
		/** @brief 指针 */
		const float * ptr() const;
		/** @brief [] 重载 */
		float& operator [] (int i);
		/** @brief [] 重载 */
		float operator [] (int i) const;
		/** @brief * 重载 */
		float operator * (const SGMVector4f& _sVec) const;
		/** @brief * 重载 */
		const SGMVector4f operator * (float _fValue) const;
		/** @brief *= 重载 */
		SGMVector4f& operator *= (float _fValue);
		/** @brief / 重载 */
		const SGMVector4f operator / (float _fValue) const;
		/** @brief /= 重载 */
		SGMVector4f& operator /= (float _fValue);
		/** @brief + 重载 */
		const SGMVector4f operator + (const SGMVector4f& _sVec) const;
		/** @brief += 重载 */
		SGMVector4f& operator += (const SGMVector4f& _sVec);
		/** @brief - 重载 */
		const SGMVector4f operator - (const SGMVector4f& _sVec) const;
		/** @brief -= 重载 */
		SGMVector4f& operator -= (const SGMVector4f& _sVec);
		/** @brief 取反 重载 */
		const SGMVector4f operator - () const;
		/** @brief 长度 */
		float Length() const;
		/** @brief 长度平方 */
		float SquaredLength() const;
		/** @brief 直线距离 */
		float Distance(const SGMVector4f& _sVec) const;
		/** @brief 直线距离平方 */
		float SquaredDistance(const SGMVector4f& _sVec) const;
		/** @brief 规范化 */
		float Normalize();
		/** @brief 相乘 */
		SGMVector4f ComponentMultiply(const SGMVector4f& _sVecL, const SGMVector4f& _sVecR);
		/** @brief 相除 */
		SGMVector4f ComponentDivide(const SGMVector4f& _sVecL, const SGMVector4f& _sVecR);
		/** @brief 转Int */
		SGMVector4i ToInt();
		/** @brief 转Double */
		SGMVector4 ToDouble();

		// 变量
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
		/** @brief 构造 */
		SGMVector4(double _x = 0, double _y = 0, double _z = 0, double _w = 0);
		/** @brief 构造 */
		SGMVector4(unsigned nColorValue);
		/** @brief 构造 */
		SGMVector4(SGMVector3 _sVec3, double _w = 0);
		/** @brief 构造 */
		SGMVector4(const SGMVector4f vec);
		/** @brief 返回Vector4f */
		operator SGMVector4f() const;
		/** @brief ==重载 */
		bool operator == (const SGMVector4& _sVec) const;
		/** @brief !=重载 */
		bool operator != (const SGMVector4& _sVec) const;
		/** @brief < 重载 */
		bool operator < (const SGMVector4& _sVec) const;
		/** @brief > 重载 */
		bool operator > (const SGMVector4& _sVec) const;
		/** @brief <= 重载 */
		bool operator <= (const SGMVector4& _sVec) const;
		/** @brief >= 重载 */
		bool operator >= (const SGMVector4& _sVec) const;
		/** @brief 指针 */
		double * ptr();
		/** @brief 指针 */
		const double * ptr() const;
		/** @brief [] 重载 */
		double& operator [] (int i);
		/** @brief [] 重载 */
		double operator [] (int i) const;
		/** @brief * 重载 */
		double operator * (const SGMVector4& _sVec) const;
		/** @brief * 重载 */
		const SGMVector4 operator * (double _fValue) const;
		/** @brief *= 重载 */
		SGMVector4& operator *= (double _fValue);
		/** @brief / 重载 */
		const SGMVector4 operator / (double _fValue) const;
		/** @brief /= 重载 */
		SGMVector4& operator /= (double _fValue);
		/** @brief + 重载 */
		const SGMVector4 operator + (const SGMVector4& _sVec) const;
		/** @brief += 重载 */
		SGMVector4& operator += (const SGMVector4& _sVec);
		/** @brief - 重载 */
		const SGMVector4 operator - (const SGMVector4& _sVec) const;
		/** @brief -= 重载 */
		SGMVector4& operator -= (const SGMVector4& _sVec);
		/** @brief 取反 重载 */
		const SGMVector4 operator - () const;
		/** @brief 长度 */
		double Length() const;
		/** @brief 长度平方 */
		double SquaredLength() const;
		/** @brief 直线距离 */
		double Distance(const SGMVector4& _sVec) const;
		/** @brief 直线距离平方 */
		double SquaredDistance(const SGMVector4& _sVec) const;
		/** @brief 规范化 */
		double Normalize();
		/** @brief 相乘 */
		SGMVector4 ComponentMultiply(const SGMVector4& _sVecL, const SGMVector4& _sVecR);
		/** @brief 相除 */
		SGMVector4 ComponentDivide(const SGMVector4& _sVecL, const SGMVector4& _sVecR);
		/** @brief 转Int */
		SGMVector4i ToInt();
		/** @brief 转Float */
		SGMVector4f ToFloat();

		// 变量
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
		/** @brief 构造 */
		SGMVector4i(int _x = 0, int _y = 0, int _z = 0, int _w = 0);
		/** @brief 构造 */
		SGMVector4i(unsigned nColorValue);
		/** @brief 构造 */
		SGMVector4i(SGMVector3i _sVec3, int _w = 0);
		/** @brief ==重载 */
		bool operator == (const SGMVector4i& _sVec) const;
		/** @brief !=重载 */
		bool operator != (const SGMVector4i& _sVec) const;
		/** @brief < 重载 */
		bool operator < (const SGMVector4i& _sVec) const;
		/** @brief > 重载 */
		bool operator > (const SGMVector4i& _sVec) const;
		/** @brief <= 重载 */
		bool operator <= (const SGMVector4i& _sVec) const;
		/** @brief >= 重载 */
		bool operator >= (const SGMVector4i& _sVec) const;
		/** @brief 指针 */
		int * ptr();
		/** @brief 指针 */
		const int * ptr() const;
		/** @brief [] 重载 */
		int& operator [] (int i);
		/** @brief [] 重载 */
		int operator [] (int i) const;
		/** @brief * 重载 */
		int operator * (const SGMVector4i& _sVec) const;
		/** @brief * 重载 */
		const SGMVector4i operator * (int _fValue) const;
		/** @brief *= 重载 */
		SGMVector4i& operator *= (int _fValue);
		/** @brief / 重载 */
		const SGMVector4i operator / (int _fValue) const;
		/** @brief /= 重载 */
		SGMVector4i& operator /= (int _fValue);
		/** @brief + 重载 */
		const SGMVector4i operator + (const SGMVector4i& _sVec) const;
		/** @brief += 重载 */
		SGMVector4i& operator += (const SGMVector4i& _sVec);
		/** @brief - 重载 */
		const SGMVector4i operator - (const SGMVector4i& _sVec) const;
		/** @brief -= 重载 */
		SGMVector4i& operator -= (const SGMVector4i& _sVec);
		/** @brief 取反 重载 */
		const SGMVector4i operator - () const;
		/** @brief 长度 */
		double Length() const;
		/** @brief 长度平方 */
		int SquaredLength() const;
		/** @brief 直线距离 */
		double Distance(const SGMVector4i& _sVec) const;
		/** @brief 直线距离平方 */
		int SquaredDistance(const SGMVector4i& _sVec) const;
		/** @brief 相乘 */
		SGMVector4i ComponentMultiply(const SGMVector4i& _sVecL, const SGMVector4i& _sVecR);
		/** @brief 相除 */
		SGMVector4i ComponentDivide(const SGMVector4i& _sVecL, const SGMVector4i& _sVecR);
		/** @brief 转Double */
		SGMVector4 ToDouble();
		/** @brief 转Float */
		SGMVector4f ToFloat();

		// 变量
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
	* 银河坐标结构体
	* @author LiuTao
	* @since 2021.06.20
	* @param fX, fY, fZ			银河空间坐标[-1.0, 1.0]^3,银心位置为(0,0,0)
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
	* 音频空间坐标结构体
	* @author LiuTao
	* @since 2021.07.04
	* @param BPM:		音频的BPM，Beat Per Minute，每分钟节拍数
						没有节奏的音频BPM == 0.0
	* @param angle:		情绪角度 == 音频的类型 [0.0,2*PI)
	* @param rank:		在上述坐标完全相同的情况下设置的音频编号，默认0
	*	angle == 0 或 (2*PI)		兴奋，怒, 红
	*	angle == 0.5*PI				快乐，喜，黄
	*	angle == PI					放松，乐，绿
	*	angle == 1.5*PI				悲伤，哀，蓝
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
	* 音频数据结构体
	* @author LiuTao
	* @since 2021.07.04
	* @param iUID:				音频UID，0为非法，范围[1,16777216]
	* @param name:				音频文件名称
	* @param audioCoord:		音频空间坐标
	* @param galaxyCoord:		银河坐标，[-1,1]
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
