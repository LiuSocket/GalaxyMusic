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
		// ����
	public:
		/** @brief ���� */
		CGMVolumeBasic();
		/**
		* @brief �޸���Ļ�ߴ�ʱ,������ô˺���
		* @param width: ��Ļ���
		* @param height: ��Ļ�߶�
		*/
		void ResizeScreen(const int width, const int height);

		/**
		* ������Ⱦ����
		* @param eQuality:	��Ⱦ����ö��
		*/
		inline void SetRenderQuality(EGMRENDER_QUALITY eQuality)
		{ 
			m_pConfigData->eRenderQuality = eQuality;
		}
		/**
		* ��ȡ��Ⱦ����
		* @return EGMRENDER_QUALITY ��Ⱦ����
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

		// ����
	protected:
		/** @brief ���� */
		virtual ~CGMVolumeBasic();

		/** @brief ��ʼ�� */
		void Init(SGMKernelData* pKernelData, SGMConfigData* pConfigData, CGMCommonUniform* pCommonUniform);
		/** @brief ����(�������������̬֮ǰ) */
		void Update(double dDeltaTime);
		/** @brief ����(�������������̬֮������Ⱦģ��UpdateLater֮ǰ) */
		void UpdateLater(double dDeltaTime);
		/**
		* @brief ��TAA�ڵ������Texture��������TAA
		* @param pTex ��ҪTAA����������ָ��
		* @param pVectorTex �ٶ�ʸ������ָ��
		* @return bool �ɹ�Ϊtrue��ʧ��Ϊfalse
		*/
		bool ActiveTAA(osg::Texture* pTex, osg::Texture* pVectorTex);

		/**
		* @brief ����2D����
		* @param fileName: ͼƬ�ļ�·��
		* @param iChannelNum: 1��2��3��4�ֱ����R��RG��RGB��RGBA
		* @return osg::Texture* ��������ָ��
		*/
		osg::Texture* _CreateTexture2D(const std::string& fileName, const int iChannelNum);

		/**
		* @brief �����������Ҫ�İ���ͼʵ�壨����12�����20����֮���ʵ�壩
		* @param pFaceGeom:		�����İ���ͼʵ���12������ε�ָ��
		* @param pEdgeGeom:		�����İ���ͼʵ���30���ı��ε�ָ��
		* @param pVertGeom:		�����İ���ͼʵ���20�������ε�ָ��
		*/
		void CreatePlatonicSolids(
			osg::Geometry** pFaceGeom,
			osg::Geometry** pEdgeGeom,
			osg::Geometry** pVertGeom) const;

		/**
		* @brief ������Ļ�ϵ�������1�׽�ƽ��λ�õĳ���,����TAA�ȹ���
		* @param fFovy : �����ֱFOV����λ����
		* @param iHeight : ��Ļ�߶��ϵ���������
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
		* @brief TAA��ʼ������������Ⱦ
		* @param strCorePath ����·��
		*/
		void _InitTAA(std::string strCorePath);
		/**
		* @brief ������Ⱦ�棬��Ļ������С�������棬�Ⱦ���Ч��Ҫ��һЩ
		* @param width: ��Ч���εĿ��
		* @param height: ��Ч���εĸ߶�
		* @return osg::Geometry* ���ؼ��νڵ�ָ��
		*/
		osg::Geometry* _CreateScreenTriangle(const int width, const int height);
		/**
		* @brief ����Ⱦ������ߴ磬��Ļ������С�������棬�Ⱦ���Ч��Ҫ��һЩ
		* @param width: ��Ч���εĿ��
		* @param height: ��Ч���εĸ߶�
		*/
		void _ResizeScreenTriangle(const int width, const int height);

		osg::Texture* _Load3DShapeNoise() const;
		osg::Texture* _Load3DErosionNoise() const;
		osg::Texture* _Load3DCurlNoise() const;

		// ����
	protected:
		SGMKernelData*										m_pKernelData;					//!< �ں�����
		SGMConfigData*										m_pConfigData;					//!< ��������
		CGMCommonUniform*									m_pCommonUniform;				//!< ����Uniform
		int													m_iScreenWidth;					//!< ��ǰ��Ļ���
		int													m_iScreenHeight;				//!< ��ǰ��Ļ�߶�
		std::string											m_strVolumeShaderPath;			//!< Volume shader ��ɫ��·��
		std::string											m_strCoreTexturePath;			//!< Volume texture ������ͼ·��
		std::string											m_strMediaTexturePath;			//!< Volume texture �Ǻ�����ͼ·��

		osg::ref_ptr<osg::MatrixTransform>					m_pDodecahedronTrans;			//!< ��12����任�ڵ�
		osg::ref_ptr<osg::Geode>							m_pDodecahedronFace;			//!< 12����ġ��桱���νڵ�
		osg::ref_ptr<osg::Geode>							m_pDodecahedronEdge;			//!< 12����ġ��ߡ����νڵ�
		osg::ref_ptr<osg::Geode>							m_pDodecahedronVert;			//!< 12����ġ��㡱���νڵ�
		osg::ref_ptr<osg::Geode>							m_pTAAGeode;					//!< TAA�ڵ�
		osg::ref_ptr<osg::StateSet>							m_statesetTAA;					//!< TAA�ڵ��״̬��
		osg::ref_ptr<osg::Texture>							m_3DShapeTex;					//!< ��ά�޷�shape������RGBA��128^3
		osg::ref_ptr<osg::Texture>							m_3DErosionTex;					//!< ��ά�޷�erosion������RGB��32^3
		osg::ref_ptr<osg::Texture>							m_3DCurlTex;					//!< ��ά�޷�curl������R��128*128*4
		osg::ref_ptr<osg::Texture>							m_blueNoiseTex;					//!< ������
		osg::ref_ptr<osg::Texture2D>						m_vectorMap_0;					//!< the No.0 vector texture
		osg::ref_ptr<osg::Texture2D>						m_vectorMap_1;					//!< the No.1 vector texture
		osg::ref_ptr<osg::Texture2D>						m_rayMarchTex;					//!< the texture by rayMarch
		osg::ref_ptr<osg::Texture2D>						m_TAATex_0;						//!< the No.0 switching texture for TAA
		osg::ref_ptr<osg::Texture2D>						m_TAATex_1;						//!< the No.1 switching texture for TAA
		osg::ref_ptr<osg::Camera>							m_rayMarchCamera;				//!< the camera for raymarch
		osg::ref_ptr<osg::Camera>							m_TAACamera;					//!< the camera for TAA
		
		osg::ref_ptr<osg::Uniform>							m_fCountUniform;				//!< ÿ֡+1 ������
		osg::ref_ptr<osg::Uniform>							m_vNoiseUniform;				//!< vec4���������Uniform��0-1
		osg::ref_ptr<osg::Uniform>							m_fPixelLengthUniform;			//!< 1�׽����洦�����س���Uniform
		osg::ref_ptr<osg::Uniform>							m_vShakeVectorUniform;			//!< TAA�����������
		osg::ref_ptr<osg::Uniform>							m_vDeltaShakeUniform;			//!< �����ֵ��������Uniform

		float												m_fShakeU;						//!< U�������ض����� ��λ����Ļ����
		float												m_fShakeV;						//!< V�������ض����� ��λ����Ļ����
		osg::Vec2f											m_vLastShakeVec;				//!< ��һ֡��������
		std::default_random_engine							m_iRandom;						//!< ���ֵ
		double												m_dTimeLastFrame;				//!< ��һ֡ʱ��

	private:
		int													m_iUnitTAA;						//!< TAA�������Ԫ
		unsigned int										m_iShakeCount;					//!< TAA��������ļ���
	};
}	// GM
