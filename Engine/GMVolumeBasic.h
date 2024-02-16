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

#include "GMCommon.h"
#include "GMKernel.h"
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

		/** @brief Get the current switching texture from TAA */
		inline osg::Texture2D* GetTAATex() const
		{
			return m_TAATex_0.get();
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
		* @brief 向TAA节点中添加Texture，并启动TAA
		* @param pTex 需要TAA操作的纹理指针
		* @param pVectorTex 速度矢量纹理指针
		* @return bool 成功为true，失败为false
		*/
		bool ActiveTAA(osg::Texture* pTex, osg::Texture* pVectorTex);

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

		/**
		* @brief 设置屏幕上的像素在1米近平面位置的长度,用于TAA等功能
		* @param fFovy : 相机垂直FOV，单位：°
		* @param iHeight : 屏幕高度上的像素数量
		*/
		void SetPixelLength(const float fFovy, const int iHeight);

		static osg::Texture2D* CreateTexture(
			const osg::Vec2i& size,
			GLint internalFormat = GL_RGBA8,
			GLenum sourceFormat = GL_RGBA,
			GLenum sourceType = GL_UNSIGNED_BYTE
		);

	private:
		/**
		* @brief TAA初始化，用于体渲染
		* @param strCorePath 核心路径
		*/
		void _InitTAA(std::string strCorePath);
		/**
		* @brief 创建渲染面，屏幕两倍大小的三角面，比矩形效率要高一些
		* @param width: 等效矩形的宽度
		* @param height: 等效矩形的高度
		* @return osg::Geometry* 返回几何节点指针
		*/
		osg::Geometry* _CreateScreenTriangle(const int width, const int height);
		/**
		* @brief 给渲染面重设尺寸，屏幕两倍大小的三角面，比矩形效率要高一些
		* @param width: 等效矩形的宽度
		* @param height: 等效矩形的高度
		*/
		void _ResizeScreenTriangle(const int width, const int height);

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
		osg::ref_ptr<osg::Geode>							m_pTAAGeode;					//!< TAA节点
		osg::ref_ptr<osg::StateSet>							m_statesetTAA;					//!< TAA节点的状态集
		osg::ref_ptr<osg::Texture>							m_3DShapeTex;					//!< 三维无缝shape噪声，RGBA，128^3
		osg::ref_ptr<osg::Texture>							m_3DErosionTex;					//!< 三维无缝erosion噪声，RGB，32^3
		osg::ref_ptr<osg::Texture>							m_3DCurlTex;					//!< 三维无缝curl噪声，R，128*128*4
		osg::ref_ptr<osg::Texture>							m_blueNoiseTex;					//!< 蓝噪声
		osg::ref_ptr<osg::Texture2D>						m_vectorMap_0;					//!< the No.0 vector texture
		osg::ref_ptr<osg::Texture2D>						m_vectorMap_1;					//!< the No.1 vector texture
		osg::ref_ptr<osg::Texture2D>						m_rayMarchTex;					//!< the texture by rayMarch
		osg::ref_ptr<osg::Texture2D>						m_TAATex_0;						//!< the No.0 switching texture for TAA
		osg::ref_ptr<osg::Texture2D>						m_TAATex_1;						//!< the No.1 switching texture for TAA
		osg::ref_ptr<osg::Camera>							m_rayMarchCamera;				//!< the camera for raymarch
		osg::ref_ptr<osg::Camera>							m_TAACamera;					//!< the camera for TAA
		
		osg::ref_ptr<osg::Uniform>							m_fCountUniform;				//!< 每帧+1 计数器
		osg::ref_ptr<osg::Uniform>							m_vNoiseUniform;				//!< vec4随机数噪声Uniform：0-1
		osg::ref_ptr<osg::Uniform>							m_fPixelLengthUniform;			//!< 1米近截面处的像素长度Uniform
		osg::ref_ptr<osg::Uniform>							m_vShakeVectorUniform;			//!< TAA相机抖动向量
		osg::ref_ptr<osg::Uniform>							m_vDeltaShakeUniform;			//!< 相机插值抖动向量Uniform

		float												m_fShakeU;						//!< U方向像素抖动， 单位：屏幕像素
		float												m_fShakeV;						//!< V方向像素抖动， 单位：屏幕像素
		osg::Vec2f											m_vLastShakeVec;				//!< 上一帧抖动向量
		std::default_random_engine							m_iRandom;						//!< 随机值
		double												m_dTimeLastFrame;				//!< 上一帧时间

	private:
		int													m_iUnitTAA;						//!< TAA面板纹理单元
		unsigned int										m_iShakeCount;					//!< TAA相机抖动的计数
	};
}	// GM
