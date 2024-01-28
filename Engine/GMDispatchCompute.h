//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMDispatchCompute.h
/// @brief		Galaxy-Music Engine - GMDispatchCompute
/// @version	1.0
/// @author		LiuTao
/// @date		2023.06.20
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <osg/Drawable>
#include <osg/Image>
#include <osg/Camera>
#include <osgDB/WriteFile>

namespace GM
{
	/*************************************************************************
	Enums
	*************************************************************************/

	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/

	/** Wrapper around glDispatchCompute.*/
	class CGMDispatchCompute : public osg::Drawable
	{
	public:
		CGMDispatchCompute(GLint numGroupsX = 0, GLint numGroupsY = 0, GLint numGroupsZ = 0) :
			Drawable(),
			_numGroupsX(numGroupsX), _numGroupsY(numGroupsY), _numGroupsZ(numGroupsZ),
			_bDispatch(false), _bDirty(false), _bOnce(true)
		{
			setUseDisplayList(false);
			setUseVertexBufferObjects(true);
		}

		CGMDispatchCompute(const CGMDispatchCompute&o, const osg::CopyOp& copyop) :
			Drawable(o, copyop),
			_numGroupsX(o._numGroupsX), _numGroupsY(o._numGroupsY), _numGroupsZ(o._numGroupsZ),
			_bDispatch(o._bDispatch), _bDirty(false)
		{
			setUseDisplayList(false);
			setUseVertexBufferObjects(true);
		}

		virtual void compileGLObjects(osg::RenderInfo&) const {}

		virtual void drawImplementation(osg::RenderInfo& renderInfo) const
		{
			if (_bDispatch)
			{
				renderInfo.getState()->get<osg::GLExtensions>()->glDispatchCompute(_numGroupsX, _numGroupsY, _numGroupsZ);
				if (_bOnce)
				{
					_bDispatch = false;//保证只执行一次
					_bDirty = true;
				}
			}
		}

		/** Set compute shader work groups */
		void setComputeGroups(const GLint numGroupsX, const GLint numGroupsY, const GLint numGroupsZ)
		{
			_numGroupsX = numGroupsX;
			_numGroupsY = numGroupsY;
			_numGroupsZ = numGroupsZ;
		}

		/** Get compute shader work groups */
		void getComputeGroups(GLint& numGroupsX, GLint& numGroupsY, GLint& numGroupsZ) const
		{
			numGroupsX = _numGroupsX;
			numGroupsY = _numGroupsY;
			numGroupsZ = _numGroupsZ;
		}

		void setDispatch(bool bEnabled)
		{
			_bDispatch = bEnabled;
		}

		void setDirty(bool bDirty)
		{
			_bDirty = bDirty;
		}
		bool getDirty()
		{
			return _bDirty;
		}

		inline void setOnce(bool bOnce)
		{
			_bOnce = bOnce;
		}

	protected:
		GLint _numGroupsX, _numGroupsY, _numGroupsZ;
		mutable bool _bDispatch;
		mutable bool _bDirty;
		bool _bOnce;// 只计算一次
	};

	class CReadPixelFinishCallback : public osg::Camera::DrawCallback
	{
	public:
		CReadPixelFinishCallback(osg::Image* pImg)
			:_pImage(pImg), _bWritten(false), _bReady(false), _path("")
		{}

		virtual void operator() (osg::RenderInfo& renderInfo) const
		{
			if (_bReady && !_bWritten)
			{
				osgDB::writeImageFile(*(_pImage.get()), _path);
				_bWritten = true;
			}
		}

		void SetWritten(bool bWritten)
		{
			_bWritten = bWritten;
		}
		bool GetWritten()
		{
			return _bWritten;
		}
		void SetReady(bool bReady)
		{
			_bReady = bReady;
		}
		void SetPath(const std::string &path)
		{
			_path = path;
		}

	private:
		mutable bool					_bWritten;				// 贴图已经写入磁盘
		bool							_bReady;				// 贴图已经准备好，可以写入磁盘
		osg::ref_ptr<osg::Image>		_pImage;
		std::string						_path;
	};
}	// GM