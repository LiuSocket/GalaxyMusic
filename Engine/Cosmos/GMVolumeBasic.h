//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMVolumeBasic.h
/// @brief		Galaxy-Music Engine - Volume basic
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.09
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "../GMCommon.h"
#include "../GMKernel.h"
#include <random>
#include <osg/MatrixTransform>
#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/Geometry>

namespace GM
{
	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/
	class CGMCommonUniform;

	class SwitchFBOCallback : public osg::Camera::DrawCallback
	{
	public:
		SwitchFBOCallback(osg::Texture *dst, osg::Texture *src) :
			_dstTexture(dst), _srcTexture(src)
		{
			_fbo = new osg::FrameBufferObject();
			_width = dynamic_cast<osg::Texture2D*>(_srcTexture.get())->getTextureWidth();
			_height = dynamic_cast<osg::Texture2D*>(_srcTexture.get())->getTextureHeight();
		}

		void SetSize(unsigned int width, unsigned int height)
		{
			_width = width;
			_height = height;
		}

		virtual void operator() (osg::RenderInfo& renderInfo) const
		{
			osg::GLExtensions* ext = renderInfo.getState()->get<osg::GLExtensions>();
			bool fbo_supported = ext && ext->isFrameBufferObjectSupported;

			_fbo->setAttachment(osg::Camera::COLOR_BUFFER0, osg::FrameBufferAttachment((osg::Texture2D*)(_srcTexture.get())));
			_fbo->setAttachment(osg::Camera::COLOR_BUFFER1, osg::FrameBufferAttachment((osg::Texture2D*)(_dstTexture.get())));
			_srcTexture->apply(*renderInfo.getState());
			_dstTexture->apply(*renderInfo.getState());

			if (fbo_supported && ext->glBlitFramebuffer)
			{
				(_fbo.get())->apply(*renderInfo.getState());
				ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, _fbo->getHandle(renderInfo.getContextID()));
				ext->glFramebufferTexture2D(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, _srcTexture->getTextureObject(renderInfo.getContextID())->id(), 0);
				ext->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, _dstTexture->getTextureObject(renderInfo.getContextID())->id(), 0);

				glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);

				ext->glBlitFramebuffer(
					0, 0, static_cast<GLint>(_width), static_cast<GLint>(_height),
					0, 0, static_cast<GLint>(_width), static_cast<GLint>(_height),
					GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}

			ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
		}

	private:
		osg::ref_ptr<osg::FrameBufferObject> _fbo;
		osg::ref_ptr<osg::Texture> _dstTexture;
		osg::ref_ptr<osg::Texture> _srcTexture;
		unsigned int _width = 1920;
		unsigned int _height = 1080;
	};

	/*!
	*  @class CGMVolumeBasic
	*  @brief Digital-Planets Volume Basic
	*/
	class CGMVolumeBasic
	{
		// 函数
	public:
		/** @brief 构造 */
		CGMVolumeBasic();
		/**
		* @brief 修改屏幕尺寸时,子类调用此函数
		* @param width: 屏幕宽度
		* @param height: 屏幕高度
		*/
		void ResizeScreen(const int width, const int height);

		/**
		* 设置渲染质量
		* @param eQuality:	渲染质量枚举
		*/
		inline void SetRenderQuality(EGMRENDER_QUALITY eQuality)
		{ 
			m_pConfigData->eRenderQuality = eQuality;
		}
		/**
		* 获取渲染质量
		* @return EGMRENDER_QUALITY 渲染质量
		*/
		inline EGMRENDER_QUALITY GetRenderQuality() const
		{
			return m_pConfigData->eRenderQuality;
		}

		/** @brief 获取体渲染颜色图，不含Alpha */
		inline osg::Texture2D* GetRayMarchColorTex() const
		{
			return m_rayMarchColorTex.get();
		}
		/** @brief 获取体渲染alpha图，RGBA分别输出“左上、右上、左下、右下”4个子像素的alpha */
		inline osg::Texture2D* GetRayMarchAlphaTex() const
		{
			return m_rayMarchAlphaTex.get();
		}

		// 函数
	protected:
		/** @brief 析构 */
		virtual ~CGMVolumeBasic();

		/** @brief 初始化 */
		void Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief 更新(在主相机更新姿态之前) */
		void Update(double dDeltaTime);
		/** @brief 更新(在主相机更新姿态之后，体渲染模块UpdateLater之前) */
		void UpdateLater(double dDeltaTime);

		/**
		* @brief 加载2D纹理
		* @param fileName: 图片文件路径
		* @param iChannelNum: 1、2、3、4分别代表R、RG、RGB、RGBA
		* @return osg::Texture* 返回纹理指针
		*/
		osg::Texture* _CreateTexture2D(const std::string& fileName, const int iChannelNum);

		/**
		* @brief 生成体积云需要的柏拉图实体（介于12面体和20面体之间的实体）
		* @param pFaceGeom:		创建的柏拉图实体的12个五边形的指针
		* @param pEdgeGeom:		创建的柏拉图实体的30个四边形的指针
		* @param pVertGeom:		创建的柏拉图实体的20个三角形的指针
		*/
		void CreatePlatonicSolids(
			osg::Geometry** pFaceGeom,
			osg::Geometry** pEdgeGeom,
			osg::Geometry** pVertGeom) const;

	private:
		/** @brief raymarching初始化，用于体渲染 */
		void _InitRayMarching();

		osg::Texture* _Load3DShapeNoise() const;
		osg::Texture* _Load3DErosionNoise() const;
		osg::Texture* _Load3DCurlNoise() const;

		// 变量
	protected:
		SGMKernelData*										m_pKernelData;					//!< 内核数据
		SGMConfigData*										m_pConfigData;					//!< 配置数据
		CGMCommonUniform*									m_pCommonUniform;				//!< 公共Uniform
		int													m_iScreenWidth;					//!< 当前屏幕宽度
		int													m_iScreenHeight;				//!< 当前屏幕高度
		std::string											m_strVolumeShaderPath;			//!< Volume shader 着色器路径
		std::string											m_strCoreTexturePath;			//!< Volume texture 核心贴图路径
		std::string											m_strMediaTexturePath;			//!< Volume texture 非核心贴图路径

		osg::ref_ptr<osg::MatrixTransform>					m_pDodecahedronTrans;			//!< 正12面体变换节点
		osg::ref_ptr<osg::Geode>							m_pDodecahedronFace;			//!< 12面体的“面”几何节点
		osg::ref_ptr<osg::Geode>							m_pDodecahedronEdge;			//!< 12面体的“边”几何节点
		osg::ref_ptr<osg::Geode>							m_pDodecahedronVert;			//!< 12面体的“点”几何节点
		osg::ref_ptr<osg::Texture>							m_3DShapeTex;					//!< 三维无缝shape噪声，RGBA，128^3
		osg::ref_ptr<osg::Texture>							m_3DErosionTex;					//!< 三维无缝erosion噪声，RGB，32^3
		osg::ref_ptr<osg::Texture>							m_3DCurlTex;					//!< 三维无缝curl噪声，R，128*128*4
		osg::ref_ptr<osg::Texture>							m_blueNoiseTex;					//!< 蓝噪声
		osg::ref_ptr<osg::Texture2D>						m_rayMarchColorTex;				//!< rayMarch生成的颜色图
		osg::ref_ptr<osg::Texture2D>						m_rayMarchAlphaTex;				//!< rayMarch生成的alpha图
		osg::ref_ptr<osg::Camera>							m_rayMarchCamera;				//!< the camera for raymarch
	};
}	// GM
