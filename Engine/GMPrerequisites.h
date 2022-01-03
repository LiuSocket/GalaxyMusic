//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMPrerequisite.h
/// @brief		Galaxy-Music Engine -Prerequisite
/// @version	1.0
/// @author		LiuTao
/// @date		2021.06.20
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <assert.h>
#include <string>
#include <vector>

namespace GM
{
	/*************************************************************************
	 Macro Defines
	*************************************************************************/
	#define GM_NEW(T)				(new T)
	#define GM_DELETE(p)			{ if(p) { delete (p); (p)=NULL; } }
	#define GM_DELETE_ARRAY(p)		{ if(p) { delete[] (p); (p)=NULL; } }
	#define GM_RELEASE(p)			{ if(p) { (p)->Release(); (p)=NULL; } }

	/*************************************************************************
	 Type Defines
	*************************************************************************/
	typedef std::vector<std::string>		VStringVec;

	/*************************************************************************
	 Template class for creating single-instance global classes.
	*************************************************************************/
	template <typename T> class CGMSingleton
	{
	private:
		/** @brief Explicit private copy constructor. This is a forbidden operation.*/
		CGMSingleton(const CGMSingleton<T> &);
		/** @brief Private operator= . This is a forbidden operation. */
		CGMSingleton& operator=(const CGMSingleton<T> &);
    
	protected:
		static T* msSingleton;

	public:
		CGMSingleton( void )
		{
			assert( !msSingleton );
	#if defined( _MSC_VER ) && _MSC_VER < 1200	 
			int offset = (int)(T*)1 - (int)(CrisSingleton <T>*)(T*)1;
			msSingleton = (T*)((int)this + offset);
	#else
			msSingleton = static_cast< T* >( this );
	#endif
		}
		~CGMSingleton( void )
		{ assert( msSingleton );  msSingleton = 0;  }
		static T& getSingleton( void )
		{ assert( msSingleton );  return ( *msSingleton ); }
		static T* getSingletonPtr( void )
		{ return msSingleton; }
	};

}	// GM
