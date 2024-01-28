//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMStructs.cpp
/// @brief		Galaxy-Music Engine -Structs
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.09
//////////////////////////////////////////////////////////////////////////
#include "GMStructs.h"
#include "tinyxml.h"
using namespace GM;

/*************************************************************************
 SGMVector2 Methods
*************************************************************************/
/** @brief ���� */
SGMVector2::SGMVector2(double _x, double _y)
	: x(_x), y(_y)
{}

/** @brief ���� */
SGMVector2::SGMVector2(const SGMVector2f vec)
{
	_v[0] = vec._v[0];
	_v[1] = vec._v[1];
}

/** @brief ����Vector3f */
SGMVector2::operator SGMVector2f() const
{
	return SGMVector2f(static_cast<float>(_v[0]), static_cast<float>(_v[1]));
}

/** @brief ==���� */
bool SGMVector2::operator == (const SGMVector2& _sVec) const
{
	return (_v[0] == _sVec._v[0]) && (_v[1] == _sVec._v[1]);
}

/** @brief !=���� */
bool SGMVector2::operator != (const SGMVector2& _sVec) const
{
	return (_v[0] != _sVec._v[0]) || (_v[1] != _sVec._v[1]);
}

/** @brief < ���� */
bool SGMVector2::operator < (const SGMVector2& _sVec) const
{
	if (_v[0] < _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else
		return (_v[1] < _sVec._v[1]);
}

/** @brief > ���� */
bool SGMVector2::operator > (const SGMVector2& _sVec) const
{
	if (_v[0] > _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else
		return (_v[1] > _sVec._v[1]);

}

/** @brief <= ���� */
bool SGMVector2::operator <= (const SGMVector2& _sVec) const
{
	if (_v[0] <= _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else
		return (_v[1] <= _sVec._v[1]);
}

/** @brief > ���� */
bool SGMVector2::operator >= (const SGMVector2& _sVec) const
{
	if (_v[0] >= _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else
		return (_v[1] >= _sVec._v[1]);
}

/** @brief ָ�� */
double * SGMVector2::ptr()
{
	return _v;
}

/** @brief ָ�� */
const double * SGMVector2::ptr() const
{
	return _v;
}

/** @brief [] ���� */
double& SGMVector2::operator [] (int i)
{
	return _v[i];
}

/** @brief [] ���� */
double SGMVector2::operator [] (int i) const
{
	return _v[i];
}

/** @brief * ���� */
double SGMVector2::operator * (const SGMVector2& _sVec) const
{
	return _v[0] * _sVec._v[0] + _v[1] * _sVec._v[1];
}

/** @brief * ���� */
const SGMVector2 SGMVector2::operator * (double _fValue) const
{
	return SGMVector2(_v[0] * _fValue, _v[1] * _fValue);
}

/** @brief *= ���� */
SGMVector2& SGMVector2::operator *= (double _fValue)
{
	_v[0] *= _fValue;
	_v[1] *= _fValue;
	return *this;
}

/** @brief / ���� */
const SGMVector2 SGMVector2::operator / (double _fValue) const
{
	return SGMVector2(_v[0] / _fValue, _v[1] / _fValue);
}

/** @brief /= ���� */
SGMVector2& SGMVector2::operator /= (double _fValue)
{
	_v[0] /= _fValue;
	_v[1] /= _fValue;
	return *this;
}

/** @brief + ���� */
const SGMVector2 SGMVector2::operator + (const SGMVector2& _sVec) const
{
	return SGMVector2(_v[0] + _sVec._v[0], _v[1] + _sVec._v[1]);
}

/** @brief += ���� */
SGMVector2& SGMVector2::operator += (const SGMVector2& _sVec)
{
	_v[0] += _sVec._v[0];
	_v[1] += _sVec._v[1];
	return *this;
}

/** @brief - ���� */
const SGMVector2 SGMVector2::operator - (const SGMVector2& _sVec) const
{
	return SGMVector2(_v[0] - _sVec._v[0], _v[1] - _sVec._v[1]);
}

/** @brief -= ���� */
SGMVector2& SGMVector2::operator -= (const SGMVector2& _sVec)
{
	_v[0] -= _sVec._v[0];
	_v[1] -= _sVec._v[1];
	return *this;
}

/** @brief ȡ�� ���� */
const SGMVector2 SGMVector2::operator - () const
{
	return SGMVector2(-_v[0], -_v[1]);
}

/** @brief ���� */
double SGMVector2::Length() const
{
	return sqrt(_v[0] * _v[0] + _v[1] * _v[1]);
}

/** @brief ����ƽ�� */
double SGMVector2::SquaredLength() const
{
	return _v[0] * _v[0] + _v[1] * _v[1];
}

/** @brief ֱ�߾��� */
double SGMVector2::Distance(const SGMVector2& _sVec) const
{
	return (*this - _sVec).Length();
}

/** @brief ֱ�߾���ƽ�� */
double SGMVector2::SquaredDistance(const SGMVector2& _sVec) const
{
	return (*this - _sVec).SquaredLength();
}

/** @brief �淶�� */
double SGMVector2::Normalize()
{
	double fNorm = SGMVector2::Length();
	if (fNorm > 0.0)
	{
		double fInv = 1.0f / fNorm;
		_v[0] *= fInv;
		_v[1] *= fInv;
	}
	return fNorm;
}

/** @brief ��� */
SGMVector2 SGMVector2::ComponentMultiply(const SGMVector2& _sVecL, const SGMVector2& _sVecR)
{
	return SGMVector2(_sVecL[0] * _sVecR[0], _sVecL[1] * _sVecR[1]);
}

/** @brief ��� */
SGMVector2 SGMVector2::ComponentDivide(const SGMVector2& _sVecL, const SGMVector2& _sVecR)
{
	return SGMVector2(_sVecL[0] / _sVecR[0], _sVecL[1] / _sVecR[1]);
}

/** @brief ��� */
SGMVector2i SGMVector2::ToInt()
{
	return SGMVector2i((int)_v[0], (int)_v[1]);
}

/** @brief ��� */
SGMVector2f SGMVector2::ToFloat()
{
	return SGMVector2f((float)_v[0], (float)_v[1]);
}

/*************************************************************************
SGMVector2i Methods
*************************************************************************/
/** @brief ���� */
SGMVector2i::SGMVector2i(int _x, int _y)
	: x(_x), y(_y)
{}

/** @brief ==���� */
bool SGMVector2i::operator == (const SGMVector2i& _sVec) const
{
	return (_v[0] == _sVec._v[0]) && (_v[1] == _sVec._v[1]);
}

/** @brief !=���� */
bool SGMVector2i::operator != (const SGMVector2i& _sVec) const
{
	return (_v[0] != _sVec._v[0]) || (_v[1] != _sVec._v[1]);
}

/** @brief < ���� */
bool SGMVector2i::operator < (const SGMVector2i& _sVec) const
{
	if (_v[0] < _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else
		return (_v[1] < _sVec._v[1]);
}

/** @brief > ���� */
bool SGMVector2i::operator > (const SGMVector2i& _sVec) const
{
	if (_v[0] > _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else
		return (_v[1] > _sVec._v[1]);

}

/** @brief <= ���� */
bool SGMVector2i::operator <= (const SGMVector2i& _sVec) const
{
	if (_v[0] <= _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else
		return (_v[1] <= _sVec._v[1]);
}

/** @brief > ���� */
bool SGMVector2i::operator >= (const SGMVector2i& _sVec) const
{
	if (_v[0] >= _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else
		return (_v[1] >= _sVec._v[1]);
}

/** @brief ָ�� */
int * SGMVector2i::ptr()
{
	return _v;
}

/** @brief ָ�� */
const int * SGMVector2i::ptr() const
{
	return _v;
}

/** @brief [] ���� */
int& SGMVector2i::operator [] (int i)
{
	return _v[i];
}

/** @brief [] ���� */
int SGMVector2i::operator [] (int i) const
{
	return _v[i];
}

/** @brief * ���� */
int SGMVector2i::operator * (const SGMVector2i& _sVec) const
{
	return _v[0] * _sVec._v[0] + _v[1] * _sVec._v[1];
}

/** @brief * ���� */
const SGMVector2i SGMVector2i::operator * (int _fValue) const
{
	return SGMVector2i(_v[0] * _fValue, _v[1] * _fValue);
}

/** @brief *= ���� */
SGMVector2i& SGMVector2i::operator *= (int _fValue)
{
	_v[0] *= _fValue;
	_v[1] *= _fValue;
	return *this;
}

/** @brief / ���� */
const SGMVector2i SGMVector2i::operator / (int _fValue) const
{
	return SGMVector2i(_v[0] / _fValue, _v[1] / _fValue);
}

/** @brief /= ���� */
SGMVector2i& SGMVector2i::operator /= (int _fValue)
{
	_v[0] /= _fValue;
	_v[1] /= _fValue;
	return *this;
}

/** @brief + ���� */
const SGMVector2i SGMVector2i::operator + (const SGMVector2i& _sVec) const
{
	return SGMVector2i(_v[0] + _sVec._v[0], _v[1] + _sVec._v[1]);
}

/** @brief += ���� */
SGMVector2i& SGMVector2i::operator += (const SGMVector2i& _sVec)
{
	_v[0] += _sVec._v[0];
	_v[1] += _sVec._v[1];
	return *this;
}

/** @brief - ���� */
const SGMVector2i SGMVector2i::operator - (const SGMVector2i& _sVec) const
{
	return SGMVector2i(_v[0] - _sVec._v[0], _v[1] - _sVec._v[1]);
}

/** @brief -= ���� */
SGMVector2i& SGMVector2i::operator -= (const SGMVector2i& _sVec)
{
	_v[0] -= _sVec._v[0];
	_v[1] -= _sVec._v[1];
	return *this;
}

/** @brief ȡ�� ���� */
const SGMVector2i SGMVector2i::operator - () const
{
	return SGMVector2i(-_v[0], -_v[1]);
}

/** @brief ���� */
double SGMVector2i::Length() const
{
	return sqrt(_v[0] * _v[0] + _v[1] * _v[1]);
}

/** @brief ����ƽ�� */
int SGMVector2i::SquaredLength() const
{
	return _v[0] * _v[0] + _v[1] * _v[1];
}

/** @brief ֱ�߾��� */
double SGMVector2i::Distance(const SGMVector2i& _sVec) const
{
	return (*this - _sVec).Length();
}

/** @brief ֱ�߾���ƽ�� */
int SGMVector2i::SquaredDistance(const SGMVector2i& _sVec) const
{
	return (*this - _sVec).SquaredLength();
}

/** @brief ��� */
SGMVector2i SGMVector2i::ComponentMultiply(const SGMVector2i& _sVecL, const SGMVector2i& _sVecR)
{
	return SGMVector2i(_sVecL[0] * _sVecR[0], _sVecL[1] * _sVecR[1]);
}

/** @brief ��� */
SGMVector2i SGMVector2i::ComponentDivide(const SGMVector2i& _sVecL, const SGMVector2i& _sVecR)
{
	return SGMVector2i(_sVecL[0] / _sVecR[0], _sVecL[1] / _sVecR[1]);
}

/** @brief תDouble */
SGMVector2 SGMVector2i::ToDouble()
{
	return SGMVector2((double)_v[0], (double)_v[1]);
}

/** @brief תFloat */
SGMVector2f SGMVector2i::ToFloat()
{
	return SGMVector2f((float)_v[0], (float)_v[1]);
}

/*************************************************************************
SGMVector2f Methods
*************************************************************************/
/** @brief ���� */
SGMVector2f::SGMVector2f(float _x, float _y)
	: x(_x), y(_y)
{}

/** @brief ==���� */
bool SGMVector2f::operator == (const SGMVector2f& _sVec) const
{
	return (_v[0] == _sVec._v[0]) && (_v[1] == _sVec._v[1]);
}

/** @brief !=���� */
bool SGMVector2f::operator != (const SGMVector2f& _sVec) const
{
	return (_v[0] != _sVec._v[0]) || (_v[1] != _sVec._v[1]);
}

/** @brief < ���� */
bool SGMVector2f::operator < (const SGMVector2f& _sVec) const
{
	if (_v[0] < _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else
		return (_v[1] < _sVec._v[1]);
}

/** @brief > ���� */
bool SGMVector2f::operator > (const SGMVector2f& _sVec) const
{
	if (_v[0] > _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else
		return (_v[1] > _sVec._v[1]);

}

/** @brief <= ���� */
bool SGMVector2f::operator <= (const SGMVector2f& _sVec) const
{
	if (_v[0] <= _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else
		return (_v[1] <= _sVec._v[1]);
}

/** @brief > ���� */
bool SGMVector2f::operator >= (const SGMVector2f& _sVec) const
{
	if (_v[0] >= _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else
		return (_v[1] >= _sVec._v[1]);
}

/** @brief ָ�� */
float * SGMVector2f::ptr()
{
	return _v;
}

/** @brief ָ�� */
const float * SGMVector2f::ptr() const
{
	return _v;
}

/** @brief [] ���� */
float& SGMVector2f::operator [] (int i)
{
	return _v[i];
}

/** @brief [] ���� */
float SGMVector2f::operator [] (int i) const
{
	return _v[i];
}

/** @brief * ���� */
float SGMVector2f::operator * (const SGMVector2f& _sVec) const
{
	return _v[0] * _sVec._v[0] + _v[1] * _sVec._v[1];
}

/** @brief * ���� */
const SGMVector2f SGMVector2f::operator * (float _fValue) const
{
	return SGMVector2f(_v[0] * _fValue, _v[1] * _fValue);
}

/** @brief *= ���� */
SGMVector2f& SGMVector2f::operator *= (float _fValue)
{
	_v[0] *= _fValue;
	_v[1] *= _fValue;
	return *this;
}

/** @brief / ���� */
const SGMVector2f SGMVector2f::operator / (float _fValue) const
{
	return SGMVector2f(_v[0] / _fValue, _v[1] / _fValue);
}

/** @brief /= ���� */
SGMVector2f& SGMVector2f::operator /= (float _fValue)
{
	_v[0] /= _fValue;
	_v[1] /= _fValue;
	return *this;
}

/** @brief + ���� */
const SGMVector2f SGMVector2f::operator + (const SGMVector2f& _sVec) const
{
	return SGMVector2f(_v[0] + _sVec._v[0], _v[1] + _sVec._v[1]);
}

/** @brief += ���� */
SGMVector2f& SGMVector2f::operator += (const SGMVector2f& _sVec)
{
	_v[0] += _sVec._v[0];
	_v[1] += _sVec._v[1];
	return *this;
}

/** @brief - ���� */
const SGMVector2f SGMVector2f::operator - (const SGMVector2f& _sVec) const
{
	return SGMVector2f(_v[0] - _sVec._v[0], _v[1] - _sVec._v[1]);
}

/** @brief -= ���� */
SGMVector2f& SGMVector2f::operator -= (const SGMVector2f& _sVec)
{
	_v[0] -= _sVec._v[0];
	_v[1] -= _sVec._v[1];
	return *this;
}

/** @brief ȡ�� ���� */
const SGMVector2f SGMVector2f::operator - () const
{
	return SGMVector2f(-_v[0], -_v[1]);
}

/** @brief ���� */
float SGMVector2f::Length() const
{
	return sqrt(_v[0] * _v[0] + _v[1] * _v[1]);
}

/** @brief ����ƽ�� */
float SGMVector2f::SquaredLength() const
{
	return _v[0] * _v[0] + _v[1] * _v[1];
}

/** @brief ֱ�߾��� */
float SGMVector2f::Distance(const SGMVector2f& _sVec) const
{
	return (*this - _sVec).Length();
}

/** @brief ֱ�߾���ƽ�� */
float SGMVector2f::SquaredDistance(const SGMVector2f& _sVec) const
{
	return (*this - _sVec).SquaredLength();
}

/** @brief �淶�� */
float SGMVector2f::Normalize()
{
	float fNorm = SGMVector2f::Length();
	if (fNorm > 0.0)
	{
		float fInv = 1.0f / fNorm;
		_v[0] *= fInv;
		_v[1] *= fInv;
	}
	return fNorm;
}

/** @brief ��� */
SGMVector2f SGMVector2f::ComponentMultiply(const SGMVector2f& _sVecL, const SGMVector2f& _sVecR)
{
	return SGMVector2f(_sVecL[0] * _sVecR[0], _sVecL[1] * _sVecR[1]);
}

/** @brief ��� */
SGMVector2f SGMVector2f::ComponentDivide(const SGMVector2f& _sVecL, const SGMVector2f& _sVecR)
{
	return SGMVector2f(_sVecL[0] / _sVecR[0], _sVecL[1] / _sVecR[1]);
}

/** @brief תDouble */
SGMVector2 SGMVector2f::ToDouble()
{
	return SGMVector2((double)_v[0], (double)_v[1]);
}

/** @brief תInt */
SGMVector2i SGMVector2f::ToInt()
{
	return SGMVector2i((int)_v[0], (int)_v[1]);
}

/*************************************************************************
 SGMVector3 Methods
*************************************************************************/
/** @brief ���� */
SGMVector3::SGMVector3(double _x, double _y, double _z)
	: x(_x), y(_y), z(_z)
{}

/** @brief ���� */
SGMVector3::SGMVector3(SGMVector2 _sVec2, double _z)
{
	_v[0] = _sVec2[0];
	_v[1] = _sVec2[1];
	_v[2] = _z;
}

/** @brief ���� */
SGMVector3::SGMVector3(const SGMVector3f vec)
{
	_v[0] = vec[0];
	_v[1] = vec[1];
	_v[2] = vec[2];
}

/** @brief ����Vector3f */
SGMVector3::operator SGMVector3f() const
{ 
	return SGMVector3f(static_cast<float>(_v[0]), static_cast<float>(_v[1]), static_cast<float>(_v[2]));
}

/** @brief ==���� */
bool SGMVector3::operator == (const SGMVector3& _sVec) const
{
	return (x == _sVec.x) && (y == _sVec.y) && (z == _sVec.z);
}

/** @brief !=���� */
bool SGMVector3::operator != (const SGMVector3& _sVec) const
{
	return (x != _sVec.x) || (y != _sVec.y) || (z != _sVec.z);
}

/** @brief < ���� */
bool SGMVector3::operator < (const SGMVector3& _sVec) const
{
	if (_v[0] < _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else if (_v[1] < _sVec._v[1])
		return true;
	else if (_v[1] > _sVec._v[1])
		return false;
	else
		return (_v[2] < _sVec._v[2]);
}

/** @brief > ���� */
bool SGMVector3::operator > (const SGMVector3& _sVec) const
{
	if (_v[0] > _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else if (_v[1] > _sVec._v[1])
		return true;
	else if (_v[1] < _sVec._v[1])
		return false;
	else
		return (_v[2] > _sVec._v[2]);
}

/** @brief <= ���� */
bool SGMVector3::operator <= (const SGMVector3& _sVec) const
{
	if (_v[0] <= _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else if (_v[1] <= _sVec._v[1])
		return true;
	else if (_v[1] > _sVec._v[1])
		return false;
	else
		return (_v[2] <= _sVec._v[2]);
}

/** @brief >= ���� */
bool SGMVector3::operator >= (const SGMVector3& _sVec) const
{
	if (_v[0] >= _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else if (_v[1] >= _sVec._v[1])
		return true;
	else if (_v[1] < _sVec._v[1])
		return false;
	else
		return (_v[2] >= _sVec._v[2]);
}

/** @brief ָ�� */
double * SGMVector3::ptr()
{
	return _v;
}

/** @brief ָ�� */
const double * SGMVector3::ptr() const
{
	return _v;
}

/** @brief [] ���� */
double& SGMVector3::operator [] (int i)
{
	return _v[i];
}

/** @brief [] ���� */
double SGMVector3::operator [] (int i) const
{
	return _v[i];
}

/** @brief * ���� */
double SGMVector3::operator * (const SGMVector3& _sVec) const
{
	return _v[0] * _sVec._v[0] + _v[1] * _sVec._v[1] + _v[2] * _sVec._v[2];
}

/** @brief ^ ���� */
const SGMVector3 SGMVector3::operator ^ (const SGMVector3& _sVec) const
{
	return SGMVector3(_v[1] * _sVec._v[2] - _v[2] * _sVec._v[1],
		_v[2] * _sVec._v[0] - _v[0] * _sVec._v[2],
		_v[0] * _sVec._v[1] - _v[1] * _sVec._v[0]);
}

/** @brief * ���� */
const SGMVector3 SGMVector3::operator * (double _fValue) const
{
	return SGMVector3(_v[0] * _fValue, _v[1] * _fValue, _v[2] * _fValue);
}

/** @brief *= ���� */
SGMVector3& SGMVector3::operator *= (double _fValue)
{
	_v[0] *= _fValue;
	_v[1] *= _fValue;
	_v[2] *= _fValue;
	return *this;
}

/** @brief / ���� */
const SGMVector3 SGMVector3::operator / (double _fValue) const
{
	return SGMVector3(_v[0] / _fValue, _v[1] / _fValue, _v[2] / _fValue);
}

/** @brief /= ���� */
SGMVector3& SGMVector3::operator /= (double _fValue)
{
	_v[0] /= _fValue;
	_v[1] /= _fValue;
	_v[2] /= _fValue;
	return *this;
}

/** @brief + ���� */
const SGMVector3 SGMVector3::operator + (const SGMVector3& _sVec) const
{
	return SGMVector3(_v[0] + _sVec._v[0], _v[1] + _sVec._v[1], _v[2] + _sVec._v[2]);
}

/** @brief += ���� */
SGMVector3& SGMVector3::operator += (const SGMVector3& _sVec)
{
	_v[0] += _sVec._v[0];
	_v[1] += _sVec._v[1];
	_v[2] += _sVec._v[2];
	return *this;
}

/** @brief - ���� */
const SGMVector3 SGMVector3::operator - (const SGMVector3& _sVec) const
{
	return SGMVector3(_v[0] - _sVec._v[0], _v[1] - _sVec._v[1], _v[2] - _sVec._v[2]);
}

/** @brief -= ���� */
SGMVector3& SGMVector3::operator -= (const SGMVector3& _sVec)
{
	_v[0] -= _sVec._v[0];
	_v[1] -= _sVec._v[1];
	_v[2] -= _sVec._v[2];
	return *this;
}

/** @brief ȡ�� ���� */
const SGMVector3 SGMVector3::operator - () const
{
	return SGMVector3(-_v[0], -_v[1], -_v[2]);
}

/** @brief ���� */
double SGMVector3::Length() const
{
	return sqrt(_v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2]);
}

/** @brief ����ƽ�� */
double SGMVector3::SquaredLength() const
{
	return _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2];
}

/** @brief ֱ�߾��� */
double SGMVector3::Distance(const SGMVector3& _sVec) const
{
	return (*this - _sVec).Length();
}

/** @brief ֱ�߾���ƽ�� */
double SGMVector3::SquaredDistance(const SGMVector3& _sVec) const
{
	return (*this - _sVec).SquaredLength();
}

/** @brief �淶�� */
double SGMVector3::Normalize()
{
	double fNorm = SGMVector3::Length();
	if (fNorm > 0.0)
	{
		double fInv = 1.0f / fNorm;
		_v[0] *= fInv;
		_v[1] *= fInv;
		_v[2] *= fInv;
	}
	return fNorm;
}

/** @brief ��� */
SGMVector3 SGMVector3::ComponentMultiply(const SGMVector3& _sVecL, const SGMVector3& _sVecR)
{
	return SGMVector3(_sVecL[0] * _sVecR[0], _sVecL[1] * _sVecR[1], _sVecL[2] * _sVecR[2]);
}

/** @brief ��� */
SGMVector3 SGMVector3::ComponentDivide(const SGMVector3& _sVecL, const SGMVector3& _sVecR)
{
	return SGMVector3(_sVecL[0] / _sVecR[0], _sVecL[1] / _sVecR[1], _sVecL[2] / _sVecR[2]);
}

/** @brief תInt */
SGMVector3i SGMVector3::ToInt()
{
	return SGMVector3i((int)_v[0], (int)_v[1], (int)_v[2]);
}

/** @brief תFloat */
SGMVector3f SGMVector3::ToFloat()
{
	return SGMVector3f((float)_v[0], (float)_v[1], (float)_v[2]);
}

/*************************************************************************
SGMVector3i Methods
*************************************************************************/
/** @brief ���� */
SGMVector3i::SGMVector3i(int _x, int _y, int _z)
	: x(_x), y(_y), z(_z)
{}

/** @brief ���� */
SGMVector3i::SGMVector3i(SGMVector2i _sVec2, int _z)
{
	_v[0] = _sVec2[0];
	_v[1] = _sVec2[1];
	_v[2] = _z;
}

/** @brief ==���� */
bool SGMVector3i::operator == (const SGMVector3i& _sVec) const
{
	return (x == _sVec.x) && (y == _sVec.y) && (z == _sVec.z);
}

/** @brief !=���� */
bool SGMVector3i::operator != (const SGMVector3i& _sVec) const
{
	return (x != _sVec.x) || (y != _sVec.y) || (z != _sVec.z);
}

/** @brief < ���� */
bool SGMVector3i::operator < (const SGMVector3i& _sVec) const
{
	if (_v[0] < _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else if (_v[1] < _sVec._v[1])
		return true;
	else if (_v[1] > _sVec._v[1])
		return false;
	else
		return (_v[2] < _sVec._v[2]);
}

/** @brief > ���� */
bool SGMVector3i::operator > (const SGMVector3i& _sVec) const
{
	if (_v[0] > _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else if (_v[1] > _sVec._v[1])
		return true;
	else if (_v[1] < _sVec._v[1])
		return false;
	else
		return (_v[2] > _sVec._v[2]);
}

/** @brief <= ���� */
bool SGMVector3i::operator <= (const SGMVector3i& _sVec) const
{
	if (_v[0] <= _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else if (_v[1] <= _sVec._v[1])
		return true;
	else if (_v[1] > _sVec._v[1])
		return false;
	else
		return (_v[2] <= _sVec._v[2]);
}

/** @brief >= ���� */
bool SGMVector3i::operator >= (const SGMVector3i& _sVec) const
{
	if (_v[0] >= _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else if (_v[1] >= _sVec._v[1])
		return true;
	else if (_v[1] < _sVec._v[1])
		return false;
	else
		return (_v[2] >= _sVec._v[2]);
}

/** @brief ָ�� */
int * SGMVector3i::ptr()
{
	return _v;
}

/** @brief ָ�� */
const int * SGMVector3i::ptr() const
{
	return _v;
}

/** @brief [] ���� */
int& SGMVector3i::operator [] (int i)
{
	return _v[i];
}

/** @brief [] ���� */
int SGMVector3i::operator [] (int i) const
{
	return _v[i];
}

/** @brief * ���� */
int SGMVector3i::operator * (const SGMVector3i& _sVec) const
{
	return _v[0] * _sVec._v[0] + _v[1] * _sVec._v[1] + _v[2] * _sVec._v[2];
}

/** @brief ^ ���� */
const SGMVector3i SGMVector3i::operator ^ (const SGMVector3i& _sVec) const
{
	return SGMVector3i(_v[1] * _sVec._v[2] - _v[2] * _sVec._v[1],
		_v[2] * _sVec._v[0] - _v[0] * _sVec._v[2],
		_v[0] * _sVec._v[1] - _v[1] * _sVec._v[0]);
}

/** @brief * ���� */
const SGMVector3i SGMVector3i::operator * (int _fValue) const
{
	return SGMVector3i(_v[0] * _fValue, _v[1] * _fValue, _v[2] * _fValue);
}

/** @brief *= ���� */
SGMVector3i& SGMVector3i::operator *= (int _fValue)
{
	_v[0] *= _fValue;
	_v[1] *= _fValue;
	_v[2] *= _fValue;
	return *this;
}

/** @brief / ���� */
const SGMVector3i SGMVector3i::operator / (int _fValue) const
{
	return SGMVector3i(_v[0] / _fValue, _v[1] / _fValue, _v[2] / _fValue);
}

/** @brief /= ���� */
SGMVector3i& SGMVector3i::operator /= (int _fValue)
{
	_v[0] /= _fValue;
	_v[1] /= _fValue;
	_v[2] /= _fValue;
	return *this;
}

/** @brief + ���� */
const SGMVector3i SGMVector3i::operator + (const SGMVector3i& _sVec) const
{
	return SGMVector3i(_v[0] + _sVec._v[0], _v[1] + _sVec._v[1], _v[2] + _sVec._v[2]);
}

/** @brief += ���� */
SGMVector3i& SGMVector3i::operator += (const SGMVector3i& _sVec)
{
	_v[0] += _sVec._v[0];
	_v[1] += _sVec._v[1];
	_v[2] += _sVec._v[2];
	return *this;
}

/** @brief - ���� */
const SGMVector3i SGMVector3i::operator - (const SGMVector3i& _sVec) const
{
	return SGMVector3i(_v[0] - _sVec._v[0], _v[1] - _sVec._v[1], _v[2] - _sVec._v[2]);
}

/** @brief -= ���� */
SGMVector3i& SGMVector3i::operator -= (const SGMVector3i& _sVec)
{
	_v[0] -= _sVec._v[0];
	_v[1] -= _sVec._v[1];
	_v[2] -= _sVec._v[2];
	return *this;
}

/** @brief ȡ�� ���� */
const SGMVector3i SGMVector3i::operator - () const
{
	return SGMVector3i(-_v[0], -_v[1], -_v[2]);
}

/** @brief ���� */
double SGMVector3i::Length() const
{
	return sqrt(_v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2]);
}

/** @brief ����ƽ�� */
int SGMVector3i::SquaredLength() const
{
	return _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2];
}

/** @brief ֱ�߾��� */
double SGMVector3i::Distance(const SGMVector3i& _sVec) const
{
	return (*this - _sVec).Length();
}

/** @brief ֱ�߾���ƽ�� */
int SGMVector3i::SquaredDistance(const SGMVector3i& _sVec) const
{
	return (*this - _sVec).SquaredLength();
}

/** @brief ��� */
SGMVector3i SGMVector3i::ComponentMultiply(const SGMVector3i& _sVecL, const SGMVector3i& _sVecR)
{
	return SGMVector3i(_sVecL[0] * _sVecR[0], _sVecL[1] * _sVecR[1], _sVecL[2] * _sVecR[2]);
}

/** @brief ��� */
SGMVector3i SGMVector3i::ComponentDivide(const SGMVector3i& _sVecL, const SGMVector3i& _sVecR)
{
	return SGMVector3i(_sVecL[0] / _sVecR[0], _sVecL[1] / _sVecR[1], _sVecL[2] / _sVecR[2]);
}

/** @brief תDouble */
SGMVector3 SGMVector3i::ToDouble()
{
	return SGMVector3((double)_v[0], (double)_v[1], (double)_v[2]);
}

/** @brief תFloat */
SGMVector3f SGMVector3i::ToFloat()
{
	return SGMVector3f((float)_v[0], (float)_v[1], (float)_v[2]);
}

/*************************************************************************
SGMVector3f Methods
*************************************************************************/
/** @brief ���� */
SGMVector3f::SGMVector3f(float _x, float _y, float _z)
	: x(_x), y(_y), z(_z)
{}

/** @brief ���� */
SGMVector3f::SGMVector3f(SGMVector2f _sVec2, float _z)
{
	_v[0] = _sVec2[0];
	_v[1] = _sVec2[1];
	_v[2] = _z;
}

/** @brief ==���� */
bool SGMVector3f::operator == (const SGMVector3f& _sVec) const
{
	return (x == _sVec.x) && (y == _sVec.y) && (z == _sVec.z);
}

/** @brief !=���� */
bool SGMVector3f::operator != (const SGMVector3f& _sVec) const
{
	return (x != _sVec.x) || (y != _sVec.y) || (z != _sVec.z);
}

/** @brief < ���� */
bool SGMVector3f::operator < (const SGMVector3f& _sVec) const
{
	if (_v[0] < _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else if (_v[1] < _sVec._v[1])
		return true;
	else if (_v[1] > _sVec._v[1])
		return false;
	else
		return (_v[2] < _sVec._v[2]);
}

/** @brief > ���� */
bool SGMVector3f::operator > (const SGMVector3f& _sVec) const
{
	if (_v[0] > _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else if (_v[1] > _sVec._v[1])
		return true;
	else if (_v[1] < _sVec._v[1])
		return false;
	else
		return (_v[2] > _sVec._v[2]);
}

/** @brief <= ���� */
bool SGMVector3f::operator <= (const SGMVector3f& _sVec) const
{
	if (_v[0] <= _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else if (_v[1] <= _sVec._v[1])
		return true;
	else if (_v[1] > _sVec._v[1])
		return false;
	else
		return (_v[2] <= _sVec._v[2]);
}

/** @brief >= ���� */
bool SGMVector3f::operator >= (const SGMVector3f& _sVec) const
{
	if (_v[0] >= _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else if (_v[1] >= _sVec._v[1])
		return true;
	else if (_v[1] < _sVec._v[1])
		return false;
	else
		return (_v[2] >= _sVec._v[2]);
}

/** @brief ָ�� */
float * SGMVector3f::ptr()
{
	return _v;
}

/** @brief ָ�� */
const float * SGMVector3f::ptr() const
{
	return _v;
}

/** @brief [] ���� */
float& SGMVector3f::operator [] (int i)
{
	return _v[i];
}

/** @brief [] ���� */
float SGMVector3f::operator [] (int i) const
{
	return _v[i];
}

/** @brief * ���� */
float SGMVector3f::operator * (const SGMVector3f& _sVec) const
{
	return _v[0] * _sVec._v[0] + _v[1] * _sVec._v[1] + _v[2] * _sVec._v[2];
}

/** @brief ^ ���� */
const SGMVector3f SGMVector3f::operator ^ (const SGMVector3f& _sVec) const
{
	return SGMVector3f(_v[1] * _sVec._v[2] - _v[2] * _sVec._v[1],
		_v[2] * _sVec._v[0] - _v[0] * _sVec._v[2],
		_v[0] * _sVec._v[1] - _v[1] * _sVec._v[0]);
}

/** @brief * ���� */
const SGMVector3f SGMVector3f::operator * (float _fValue) const
{
	return SGMVector3f(_v[0] * _fValue, _v[1] * _fValue, _v[2] * _fValue);
}

/** @brief *= ���� */
SGMVector3f& SGMVector3f::operator *= (float _fValue)
{
	_v[0] *= _fValue;
	_v[1] *= _fValue;
	_v[2] *= _fValue;
	return *this;
}

/** @brief / ���� */
const SGMVector3f SGMVector3f::operator / (float _fValue) const
{
	return SGMVector3f(_v[0] / _fValue, _v[1] / _fValue, _v[2] / _fValue);
}

/** @brief /= ���� */
SGMVector3f& SGMVector3f::operator /= (float _fValue)
{
	_v[0] /= _fValue;
	_v[1] /= _fValue;
	_v[2] /= _fValue;
	return *this;
}

/** @brief + ���� */
const SGMVector3f SGMVector3f::operator + (const SGMVector3f& _sVec) const
{
	return SGMVector3f(_v[0] + _sVec._v[0], _v[1] + _sVec._v[1], _v[2] + _sVec._v[2]);
}

/** @brief += ���� */
SGMVector3f& SGMVector3f::operator += (const SGMVector3f& _sVec)
{
	_v[0] += _sVec._v[0];
	_v[1] += _sVec._v[1];
	_v[2] += _sVec._v[2];
	return *this;
}

/** @brief - ���� */
const SGMVector3f SGMVector3f::operator - (const SGMVector3f& _sVec) const
{
	return SGMVector3f(_v[0] - _sVec._v[0], _v[1] - _sVec._v[1], _v[2] - _sVec._v[2]);
}

/** @brief -= ���� */
SGMVector3f& SGMVector3f::operator -= (const SGMVector3f& _sVec)
{
	_v[0] -= _sVec._v[0];
	_v[1] -= _sVec._v[1];
	_v[2] -= _sVec._v[2];
	return *this;
}

/** @brief ȡ�� ���� */
const SGMVector3f SGMVector3f::operator - () const
{
	return SGMVector3f(-_v[0], -_v[1], -_v[2]);
}

/** @brief ���� */
float SGMVector3f::Length() const
{
	return sqrt(_v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2]);
}

/** @brief ����ƽ�� */
float SGMVector3f::SquaredLength() const
{
	return _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2];
}

/** @brief ֱ�߾��� */
float SGMVector3f::Distance(const SGMVector3f& _sVec) const
{
	return (*this - _sVec).Length();
}

/** @brief ֱ�߾���ƽ�� */
float SGMVector3f::SquaredDistance(const SGMVector3f& _sVec) const
{
	return (*this - _sVec).SquaredLength();
}

/** @brief �淶�� */
float SGMVector3f::Normalize()
{
	float fNorm = SGMVector3f::Length();
	if (fNorm > 0.0)
	{
		float fInv = 1.0f / fNorm;
		_v[0] *= fInv;
		_v[1] *= fInv;
		_v[2] *= fInv;
	}
	return fNorm;
}

/** @brief ��� */
SGMVector3f SGMVector3f::ComponentMultiply(const SGMVector3f& _sVecL, const SGMVector3f& _sVecR)
{
	return SGMVector3f(_sVecL[0] * _sVecR[0], _sVecL[1] * _sVecR[1], _sVecL[2] * _sVecR[2]);
}

/** @brief ��� */
SGMVector3f SGMVector3f::ComponentDivide(const SGMVector3f& _sVecL, const SGMVector3f& _sVecR)
{
	return SGMVector3f(_sVecL[0] / _sVecR[0], _sVecL[1] / _sVecR[1], _sVecL[2] / _sVecR[2]);
}

/** @brief תInt */
SGMVector3i SGMVector3f::ToInt()
{
	return SGMVector3i((int)_v[0], (int)_v[1], (int)_v[2]);
}

/** @brief תFloat */
SGMVector3 SGMVector3f::ToDouble()
{
	return SGMVector3((double)_v[0], (double)_v[1], (double)_v[2]);
}

/*************************************************************************
 SGMVector4 Methods
*************************************************************************/
/** @brief ���� */
SGMVector4::SGMVector4(double _x, double _y, double _z, double _w)
	: x(_x), y(_y), z(_z), w(_w)
{}

/** @brief ���� */
SGMVector4::SGMVector4(unsigned nColorValue)
{
	r = (double)(nColorValue >> 24) / 255.0f;
	g = (double)((nColorValue & 0xFF0000) >> 16) / 255.0f;
	b = (double)((nColorValue & 0xFF00) >> 8) / 255.0f;
	a = (double)(nColorValue & 0xFF) / 255.0f;
}

/** @brief ���� */
SGMVector4::SGMVector4(SGMVector3 _sVec3, double _w)
{
	_v[0] = _sVec3[0];
	_v[1] = _sVec3[1];
	_v[2] = _sVec3[2];
	_v[3] = _w;
}

/** @brief ���� */
SGMVector4::SGMVector4(const SGMVector4f vec)
{
	_v[0] = vec[0];
	_v[1] = vec[1];
	_v[2] = vec[2];
	_v[3] = vec[3];
}

/** @brief ����Vector4f */
SGMVector4::operator SGMVector4f() const
{
	return SGMVector4f(static_cast<float>(_v[0]), static_cast<float>(_v[1]), static_cast<float>(_v[2]), static_cast<float>(_v[3]));
}

/** @brief ==���� */
bool SGMVector4::operator == (const SGMVector4& _sVec) const
{
	return (x == _sVec.x) && (y == _sVec.y) && (z == _sVec.z) && (w == _sVec.w);
}

/** @brief !=���� */
bool SGMVector4::operator != (const SGMVector4& _sVec) const
{
	return (x != _sVec.x) || (y != _sVec.y) || (z != _sVec.z) || (w != _sVec.w);
}

/** @brief < ���� */
bool SGMVector4::operator < (const SGMVector4& _sVec) const
{
	if (_v[0] < _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else if (_v[1] < _sVec._v[1])
		return true;
	else if (_v[1] > _sVec._v[1])
		return false;
	else if (_v[2] < _sVec._v[2])
		return true;
	else if (_v[2] > _sVec._v[2])
		return false;
	else
		return (_v[3] < _sVec._v[3]);
}

/** @brief > ���� */
bool SGMVector4::operator > (const SGMVector4& _sVec) const
{
	if (_v[0] > _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else if (_v[1] > _sVec._v[1])
		return true;
	else if (_v[1] < _sVec._v[1])
		return false;
	else if (_v[2] > _sVec._v[2])
		return true;
	else if (_v[2] < _sVec._v[2])
		return false;
	else
		return (_v[3] > _sVec._v[3]);
}

/** @brief <= ���� */
bool SGMVector4::operator <= (const SGMVector4& _sVec) const
{
	if (_v[0] <= _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else if (_v[1] <= _sVec._v[1])
		return true;
	else if (_v[1] > _sVec._v[1])
		return false;
	else if (_v[2] <= _sVec._v[2])
		return true;
	else if (_v[2] > _sVec._v[2])
		return false;
	else
		return (_v[3] <= _sVec._v[3]);
}

/** @brief >= ���� */
bool SGMVector4::operator >= (const SGMVector4& _sVec) const
{
	if (_v[0] >= _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else if (_v[1] >= _sVec._v[1])
		return true;
	else if (_v[1] < _sVec._v[1])
		return false;
	else if (_v[2] >= _sVec._v[2])
		return true;
	else if (_v[2] < _sVec._v[2])
		return false;
	else
		return (_v[3] >= _sVec._v[3]);
}

/** @brief ָ�� */
double * SGMVector4::ptr()
{
	return _v;
}

/** @brief ָ�� */
const double * SGMVector4::ptr() const
{
	return _v;
}

/** @brief [] ���� */
double& SGMVector4::operator [] (int i)
{
	return _v[i];
}

/** @brief [] ���� */
double SGMVector4::operator [] (int i) const
{
	return _v[i];
}

/** @brief * ���� */
double SGMVector4::operator * (const SGMVector4& _sVec) const
{
	return _v[0] * _sVec._v[0] + _v[1] * _sVec._v[1] + _v[2] * _sVec._v[2] + _v[3] * _sVec._v[3];
}

/** @brief * ���� */
const SGMVector4 SGMVector4::operator * (double _fValue) const
{
	return SGMVector4(_v[0] * _fValue, _v[1] * _fValue, _v[2] * _fValue, _v[3] * _fValue);
}

/** @brief *= ���� */
SGMVector4& SGMVector4::operator *= (double _fValue)
{
	_v[0] *= _fValue;
	_v[1] *= _fValue;
	_v[2] *= _fValue;
	_v[3] *= _fValue;
	return *this;
}

/** @brief / ���� */
const SGMVector4 SGMVector4::operator / (double _fValue) const
{
	return SGMVector4(_v[0] / _fValue, _v[1] / _fValue, _v[2] / _fValue, _v[3] / _fValue);
}

/** @brief /= ���� */
SGMVector4& SGMVector4::operator /= (double _fValue)
{
	_v[0] /= _fValue;
	_v[1] /= _fValue;
	_v[2] /= _fValue;
	_v[3] /= _fValue;
	return *this;
}

/** @brief + ���� */
const SGMVector4 SGMVector4::operator + (const SGMVector4& _sVec) const
{
	return SGMVector4(_v[0] + _sVec._v[0], _v[1] + _sVec._v[1], _v[2] + _sVec._v[2], _v[3] + _sVec._v[3]);
}

/** @brief += ���� */
SGMVector4& SGMVector4::operator += (const SGMVector4& _sVec)
{
	_v[0] += _sVec._v[0];
	_v[1] += _sVec._v[1];
	_v[2] += _sVec._v[2];
	_v[3] += _sVec._v[3];
	return *this;
}

/** @brief - ���� */
const SGMVector4 SGMVector4::operator - (const SGMVector4& _sVec) const
{
	return SGMVector4(_v[0] - _sVec._v[0], _v[1] - _sVec._v[1], _v[2] - _sVec._v[2], _v[3] - _sVec._v[3]);
}

/** @brief -= ���� */
SGMVector4& SGMVector4::operator -= (const SGMVector4& _sVec)
{
	_v[0] -= _sVec._v[0];
	_v[1] -= _sVec._v[1];
	_v[2] -= _sVec._v[2];
	_v[3] -= _sVec._v[3];
	return *this;
}

/** @brief ȡ�� ���� */
const SGMVector4 SGMVector4::operator - () const
{
	return SGMVector4(-_v[0], -_v[1], -_v[2], -_v[3]);
}

/** @brief ���� */
double SGMVector4::Length() const
{
	return sqrt(_v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2] + _v[3] * _v[3]);
}

/** @brief ����ƽ�� */
double SGMVector4::SquaredLength() const
{
	return _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2] + _v[3] * _v[3];
}

/** @brief ֱ�߾��� */
double SGMVector4::Distance(const SGMVector4& _sVec) const
{
	return (*this - _sVec).Length();
}

/** @brief ֱ�߾���ƽ�� */
double SGMVector4::SquaredDistance(const SGMVector4& _sVec) const
{
	return (*this - _sVec).SquaredLength();
}

/** @brief �淶�� */
double SGMVector4::Normalize()
{
	double fNorm = SGMVector4::Length();
	if (fNorm > 0.0)
	{
		double fInv = 1.0f / fNorm;
		_v[0] *= fInv;
		_v[1] *= fInv;
		_v[2] *= fInv;
		_v[3] *= fInv;
	}
	return fNorm;
}

/** @brief ��� */
SGMVector4 SGMVector4::ComponentMultiply(const SGMVector4& _sVecL, const SGMVector4& _sVecR)
{
	return SGMVector4(_sVecL[0] * _sVecR[0], _sVecL[1] * _sVecR[1], _sVecL[2] * _sVecR[2], _sVecL[3] * _sVecR[3]);
}

/** @brief ��� */
SGMVector4 SGMVector4::ComponentDivide(const SGMVector4& _sVecL, const SGMVector4& _sVecR)
{
	return SGMVector4(_sVecL[0] / _sVecR[0], _sVecL[1] / _sVecR[1], _sVecL[2] / _sVecR[2], _sVecL[3] / _sVecR[3]);
}

/** @brief ��� */
SGMVector4i SGMVector4::ToInt()
{
	return SGMVector4i((int)_v[0], (int)_v[1], (int)_v[2], (int)_v[3]);
}

/** @brief ��� */
SGMVector4f SGMVector4::ToFloat()
{
	return SGMVector4f((float)_v[0], (float)_v[1], (float)_v[2], (float)_v[3]);
}

/*************************************************************************
SGMVector4i Methods
*************************************************************************/
/** @brief ���� */
SGMVector4i::SGMVector4i(int _x, int _y, int _z, int _w)
	: x(_x), y(_y), z(_z), w(_w)
{}

/** @brief ���� */
SGMVector4i::SGMVector4i(unsigned nColorValue)
{
	r = (int)(nColorValue >> 24) / 255;
	g = (int)((nColorValue & 0xFF0000) >> 16) / 255;
	b = (int)((nColorValue & 0xFF00) >> 8) / 255;
	a = (int)(nColorValue & 0xFF) / 255;
}

/** @brief ���� */
SGMVector4i::SGMVector4i(SGMVector3i _sVec3, int _w)
{
	_v[0] = _sVec3[0];
	_v[1] = _sVec3[1];
	_v[2] = _sVec3[2];
	_v[3] = _w;
}

/** @brief ==���� */
bool SGMVector4i::operator == (const SGMVector4i& _sVec) const
{
	return (x == _sVec.x) && (y == _sVec.y) && (z == _sVec.z) && (w == _sVec.w);
}

/** @brief !=���� */
bool SGMVector4i::operator != (const SGMVector4i& _sVec) const
{
	return (x != _sVec.x) || (y != _sVec.y) || (z != _sVec.z) || (w != _sVec.w);
}

/** @brief < ���� */
bool SGMVector4i::operator < (const SGMVector4i& _sVec) const
{
	if (_v[0] < _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else if (_v[1] < _sVec._v[1])
		return true;
	else if (_v[1] > _sVec._v[1])
		return false;
	else if (_v[2] < _sVec._v[2])
		return true;
	else if (_v[2] > _sVec._v[2])
		return false;
	else
		return (_v[3] < _sVec._v[3]);
}

/** @brief > ���� */
bool SGMVector4i::operator > (const SGMVector4i& _sVec) const
{
	if (_v[0] > _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else if (_v[1] > _sVec._v[1])
		return true;
	else if (_v[1] < _sVec._v[1])
		return false;
	else if (_v[2] > _sVec._v[2])
		return true;
	else if (_v[2] < _sVec._v[2])
		return false;
	else
		return (_v[3] > _sVec._v[3]);
}

/** @brief <= ���� */
bool SGMVector4i::operator <= (const SGMVector4i& _sVec) const
{
	if (_v[0] <= _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else if (_v[1] <= _sVec._v[1])
		return true;
	else if (_v[1] > _sVec._v[1])
		return false;
	else if (_v[2] <= _sVec._v[2])
		return true;
	else if (_v[2] > _sVec._v[2])
		return false;
	else
		return (_v[3] <= _sVec._v[3]);
}

/** @brief >= ���� */
bool SGMVector4i::operator >= (const SGMVector4i& _sVec) const
{
	if (_v[0] >= _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else if (_v[1] >= _sVec._v[1])
		return true;
	else if (_v[1] < _sVec._v[1])
		return false;
	else if (_v[2] >= _sVec._v[2])
		return true;
	else if (_v[2] < _sVec._v[2])
		return false;
	else
		return (_v[3] >= _sVec._v[3]);
}

/** @brief ָ�� */
int * SGMVector4i::ptr()
{
	return _v;
}

/** @brief ָ�� */
const int * SGMVector4i::ptr() const
{
	return _v;
}

/** @brief [] ���� */
int& SGMVector4i::operator [] (int i)
{
	return _v[i];
}

/** @brief [] ���� */
int SGMVector4i::operator [] (int i) const
{
	return _v[i];
}

/** @brief * ���� */
int SGMVector4i::operator * (const SGMVector4i& _sVec) const
{
	return _v[0] * _sVec._v[0] + _v[1] * _sVec._v[1] + _v[2] * _sVec._v[2] + _v[3] * _sVec._v[3];
}

/** @brief * ���� */
const SGMVector4i SGMVector4i::operator * (int _fValue) const
{
	return SGMVector4i(_v[0] * _fValue, _v[1] * _fValue, _v[2] * _fValue, _v[3] * _fValue);
}

/** @brief *= ���� */
SGMVector4i& SGMVector4i::operator *= (int _fValue)
{
	_v[0] *= _fValue;
	_v[1] *= _fValue;
	_v[2] *= _fValue;
	_v[3] *= _fValue;
	return *this;
}

/** @brief / ���� */
const SGMVector4i SGMVector4i::operator / (int _fValue) const
{
	return SGMVector4i(_v[0] / _fValue, _v[1] / _fValue, _v[2] / _fValue, _v[3] / _fValue);
}

/** @brief /= ���� */
SGMVector4i& SGMVector4i::operator /= (int _fValue)
{
	_v[0] /= _fValue;
	_v[1] /= _fValue;
	_v[2] /= _fValue;
	_v[3] /= _fValue;
	return *this;
}

/** @brief + ���� */
const SGMVector4i SGMVector4i::operator + (const SGMVector4i& _sVec) const
{
	return SGMVector4i(_v[0] + _sVec._v[0], _v[1] + _sVec._v[1], _v[2] + _sVec._v[2], _v[3] + _sVec._v[3]);
}

/** @brief += ���� */
SGMVector4i& SGMVector4i::operator += (const SGMVector4i& _sVec)
{
	_v[0] += _sVec._v[0];
	_v[1] += _sVec._v[1];
	_v[2] += _sVec._v[2];
	_v[3] += _sVec._v[3];
	return *this;
}

/** @brief - ���� */
const SGMVector4i SGMVector4i::operator - (const SGMVector4i& _sVec) const
{
	return SGMVector4i(_v[0] - _sVec._v[0], _v[1] - _sVec._v[1], _v[2] - _sVec._v[2], _v[3] - _sVec._v[3]);
}

/** @brief -= ���� */
SGMVector4i& SGMVector4i::operator -= (const SGMVector4i& _sVec)
{
	_v[0] -= _sVec._v[0];
	_v[1] -= _sVec._v[1];
	_v[2] -= _sVec._v[2];
	_v[3] -= _sVec._v[3];
	return *this;
}

/** @brief ȡ�� ���� */
const SGMVector4i SGMVector4i::operator - () const
{
	return SGMVector4i(-_v[0], -_v[1], -_v[2], -_v[3]);
}

/** @brief ���� */
double SGMVector4i::Length() const
{
	return sqrt(_v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2] + _v[3] * _v[3]);
}

/** @brief ����ƽ�� */
int SGMVector4i::SquaredLength() const
{
	return _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2] + _v[3] * _v[3];
}

/** @brief ֱ�߾��� */
double SGMVector4i::Distance(const SGMVector4i& _sVec) const
{
	return (*this - _sVec).Length();
}

/** @brief ֱ�߾���ƽ�� */
int SGMVector4i::SquaredDistance(const SGMVector4i& _sVec) const
{
	return (*this - _sVec).SquaredLength();
}

/** @brief ��� */
SGMVector4i SGMVector4i::ComponentMultiply(const SGMVector4i& _sVecL, const SGMVector4i& _sVecR)
{
	return SGMVector4i(_sVecL[0] * _sVecR[0], _sVecL[1] * _sVecR[1], _sVecL[2] * _sVecR[2], _sVecL[3] * _sVecR[3]);
}

/** @brief ��� */
SGMVector4i SGMVector4i::ComponentDivide(const SGMVector4i& _sVecL, const SGMVector4i& _sVecR)
{
	return SGMVector4i(_sVecL[0] / _sVecR[0], _sVecL[1] / _sVecR[1], _sVecL[2] / _sVecR[2], _sVecL[3] / _sVecR[3]);
}

/** @brief תDouble */
SGMVector4 SGMVector4i::ToDouble()
{
	return SGMVector4((double)_v[0], (double)_v[1], (double)_v[2], (double)_v[3]);
}

/** @brief תFloat */
SGMVector4f SGMVector4i::ToFloat()
{
	return SGMVector4f((float)_v[0], (float)_v[1], (float)_v[2], (float)_v[3]);
}

/*************************************************************************
SGMVector4f Methods
*************************************************************************/
/** @brief ���� */
SGMVector4f::SGMVector4f(float _x, float _y, float _z, float _w)
	: x(_x), y(_y), z(_z), w(_w)
{}

/** @brief ���� */
SGMVector4f::SGMVector4f(unsigned nColorValue)
{
	r = (float)(nColorValue >> 24) / 255.0f;
	g = (float)((nColorValue & 0xFF0000) >> 16) / 255.0f;
	b = (float)((nColorValue & 0xFF00) >> 8) / 255.0f;
	a = (float)(nColorValue & 0xFF) / 255.0f;
}

/** @brief ���� */
SGMVector4f::SGMVector4f(SGMVector3f _sVec3, float _w)
{
	_v[0] = _sVec3[0];
	_v[1] = _sVec3[1];
	_v[2] = _sVec3[2];
	_v[3] = _w;
}

/** @brief ==���� */
bool SGMVector4f::operator == (const SGMVector4f& _sVec) const
{
	return (x == _sVec.x) && (y == _sVec.y) && (z == _sVec.z) && (w == _sVec.w);
}

/** @brief !=���� */
bool SGMVector4f::operator != (const SGMVector4f& _sVec) const
{
	return (x != _sVec.x) || (y != _sVec.y) || (z != _sVec.z) || (w != _sVec.w);
}

/** @brief < ���� */
bool SGMVector4f::operator < (const SGMVector4f& _sVec) const
{
	if (_v[0] < _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else if (_v[1] < _sVec._v[1])
		return true;
	else if (_v[1] > _sVec._v[1])
		return false;
	else if (_v[2] < _sVec._v[2])
		return true;
	else if (_v[2] > _sVec._v[2])
		return false;
	else
		return (_v[3] < _sVec._v[3]);
}

/** @brief > ���� */
bool SGMVector4f::operator > (const SGMVector4f& _sVec) const
{
	if (_v[0] > _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else if (_v[1] > _sVec._v[1])
		return true;
	else if (_v[1] < _sVec._v[1])
		return false;
	else if (_v[2] > _sVec._v[2])
		return true;
	else if (_v[2] < _sVec._v[2])
		return false;
	else
		return (_v[3] > _sVec._v[3]);
}

/** @brief <= ���� */
bool SGMVector4f::operator <= (const SGMVector4f& _sVec) const
{
	if (_v[0] <= _sVec._v[0])
		return true;
	else if (_v[0] > _sVec._v[0])
		return false;
	else if (_v[1] <= _sVec._v[1])
		return true;
	else if (_v[1] > _sVec._v[1])
		return false;
	else if (_v[2] <= _sVec._v[2])
		return true;
	else if (_v[2] > _sVec._v[2])
		return false;
	else
		return (_v[3] <= _sVec._v[3]);
}

/** @brief >= ���� */
bool SGMVector4f::operator >= (const SGMVector4f& _sVec) const
{
	if (_v[0] >= _sVec._v[0])
		return true;
	else if (_v[0] < _sVec._v[0])
		return false;
	else if (_v[1] >= _sVec._v[1])
		return true;
	else if (_v[1] < _sVec._v[1])
		return false;
	else if (_v[2] >= _sVec._v[2])
		return true;
	else if (_v[2] < _sVec._v[2])
		return false;
	else
		return (_v[3] >= _sVec._v[3]);
}

/** @brief ָ�� */
float * SGMVector4f::ptr()
{
	return _v;
}

/** @brief ָ�� */
const float * SGMVector4f::ptr() const
{
	return _v;
}

/** @brief [] ���� */
float& SGMVector4f::operator [] (int i)
{
	return _v[i];
}

/** @brief [] ���� */
float SGMVector4f::operator [] (int i) const
{
	return _v[i];
}

/** @brief * ���� */
float SGMVector4f::operator * (const SGMVector4f& _sVec) const
{
	return _v[0] * _sVec._v[0] + _v[1] * _sVec._v[1] + _v[2] * _sVec._v[2] + _v[3] * _sVec._v[3];
}

/** @brief * ���� */
const SGMVector4f SGMVector4f::operator * (float _fValue) const
{
	return SGMVector4f(_v[0] * _fValue, _v[1] * _fValue, _v[2] * _fValue, _v[3] * _fValue);
}

/** @brief *= ���� */
SGMVector4f& SGMVector4f::operator *= (float _fValue)
{
	_v[0] *= _fValue;
	_v[1] *= _fValue;
	_v[2] *= _fValue;
	_v[3] *= _fValue;
	return *this;
}

/** @brief / ���� */
const SGMVector4f SGMVector4f::operator / (float _fValue) const
{
	return SGMVector4f(_v[0] / _fValue, _v[1] / _fValue, _v[2] / _fValue, _v[3] / _fValue);
}

/** @brief /= ���� */
SGMVector4f& SGMVector4f::operator /= (float _fValue)
{
	_v[0] /= _fValue;
	_v[1] /= _fValue;
	_v[2] /= _fValue;
	_v[3] /= _fValue;
	return *this;
}

/** @brief + ���� */
const SGMVector4f SGMVector4f::operator + (const SGMVector4f& _sVec) const
{
	return SGMVector4f(_v[0] + _sVec._v[0], _v[1] + _sVec._v[1], _v[2] + _sVec._v[2], _v[3] + _sVec._v[3]);
}

/** @brief += ���� */
SGMVector4f& SGMVector4f::operator += (const SGMVector4f& _sVec)
{
	_v[0] += _sVec._v[0];
	_v[1] += _sVec._v[1];
	_v[2] += _sVec._v[2];
	_v[3] += _sVec._v[3];
	return *this;
}

/** @brief - ���� */
const SGMVector4f SGMVector4f::operator - (const SGMVector4f& _sVec) const
{
	return SGMVector4f(_v[0] - _sVec._v[0], _v[1] - _sVec._v[1], _v[2] - _sVec._v[2], _v[3] - _sVec._v[3]);
}

/** @brief -= ���� */
SGMVector4f& SGMVector4f::operator -= (const SGMVector4f& _sVec)
{
	_v[0] -= _sVec._v[0];
	_v[1] -= _sVec._v[1];
	_v[2] -= _sVec._v[2];
	_v[3] -= _sVec._v[3];
	return *this;
}

/** @brief ȡ�� ���� */
const SGMVector4f SGMVector4f::operator - () const
{
	return SGMVector4f(-_v[0], -_v[1], -_v[2], -_v[3]);
}

/** @brief ���� */
float SGMVector4f::Length() const
{
	return sqrt(_v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2] + _v[3] * _v[3]);
}

/** @brief ����ƽ�� */
float SGMVector4f::SquaredLength() const
{
	return _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2] + _v[3] * _v[3];
}

/** @brief ֱ�߾��� */
float SGMVector4f::Distance(const SGMVector4f& _sVec) const
{
	return (*this - _sVec).Length();
}

/** @brief ֱ�߾���ƽ�� */
float SGMVector4f::SquaredDistance(const SGMVector4f& _sVec) const
{
	return (*this - _sVec).SquaredLength();
}

/** @brief �淶�� */
float SGMVector4f::Normalize()
{
	float fNorm = SGMVector4f::Length();
	if (fNorm > 0.0)
	{
		float fInv = 1.0f / fNorm;
		_v[0] *= fInv;
		_v[1] *= fInv;
		_v[2] *= fInv;
		_v[3] *= fInv;
	}
	return fNorm;
}

/** @brief ��� */
SGMVector4f SGMVector4f::ComponentMultiply(const SGMVector4f& _sVecL, const SGMVector4f& _sVecR)
{
	return SGMVector4f(_sVecL[0] * _sVecR[0], _sVecL[1] * _sVecR[1], _sVecL[2] * _sVecR[2], _sVecL[3] * _sVecR[3]);
}

/** @brief ��� */
SGMVector4f SGMVector4f::ComponentDivide(const SGMVector4f& _sVecL, const SGMVector4f& _sVecR)
{
	return SGMVector4f(_sVecL[0] / _sVecR[0], _sVecL[1] / _sVecR[1], _sVecL[2] / _sVecR[2], _sVecL[3] / _sVecR[3]);
}

/** @brief תInt */
SGMVector4i SGMVector4f::ToInt()
{
	return SGMVector4i((int)_v[0], (int)_v[1], (int)_v[2], (int)_v[3]);
}

/** @brief תDouble */
SGMVector4 SGMVector4f::ToDouble()
{
	return SGMVector4((double)_v[0], (double)_v[1], (double)_v[2], (double)_v[3]);
}