//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMVolumeBasic.h
/// @brief		Galaxy-Music Engine - GMEngine.h
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.10
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "GMCommon.h"
#include "GMKernel.h"
#include <random>

/*************************************************************************
Class
*************************************************************************/
class QWidget;

namespace GM
{
	/*************************************************************************
	Macro Defines
	*************************************************************************/
	#define GM_ENGINE					CGMEngine::getSingleton()
	#define GM_ENGINE_PTR				CGMEngine::getSingletonPtr()

	/*************************************************************************
	Enums
	*************************************************************************/

	// ��ǰ��Ƶ����ģʽ
	enum EGMA_MODE
	{
		EGMA_MOD_SINGLE,			// ����ѭ��
		EGMA_MOD_CIRCLE,			// �б�ѭ��
		EGMA_MOD_RANDOM,			// �������
		EGMA_MOD_ORDER				// ˳�򲥷�
	};

	/*************************************************************************
	Structs
	*************************************************************************/

	/*************************************************************************
	Class
	*************************************************************************/
	class CGMCommonUniform;
	class CGMDataManager;
	class CGMGalaxy;
	class CGMAudio;
	class CGMPost;
	class CGMCameraManipulator;

	/*!
	*  @class CGMEngine
	*  @brief Galaxy-Music GMEngine
	*/
	class CGMEngine : public CGMSingleton<CGMEngine>
	{
		// ����
	protected:
		/** @brief ���� */
		CGMEngine();
		/** @brief ���� */
		~CGMEngine();

	public:
		/** @brief ��ȡ���� */
		static CGMEngine& getSingleton(void);
		static CGMEngine* getSingletonPtr(void);

		/** @brief ��ʼ�� */
		bool Init();
		/** @brief �ͷ� */
		void Release();
		/** @brief ���� */
		bool Update();
		/** @brief ���� */
		bool Load();
		/** @brief ���� */
		bool Save();
		/** @brief ����̫��ϵ�˿̵���Ϣ */
		bool SaveSolarData();
		/**
		* �޸���Ļ�ߴ�ʱ���ô˺���
		* @param iW: ��Ļ���
		* @param iH: ��Ļ�߶�
		*/
		void ResizeScreen(const int iW, const int iH);

		/** @brief ����/�رձ༭ģʽ */
		void SetEditMode(const bool bEnable);
		/** @brief ��ȡ�༭ģʽ���ǻ�� */
		bool GetEditMode() const;
		/** @brief ����/�ر�ץ�ֵ�����״̬ */
		void SetHandleHover(const bool bHover);
		/** @brief ����/�رղ����� */
		void SetCapture(const bool bEnable);

		/**
		* @brief ��������ѡ����Ҫ��������Ƶ
		* @param fX, fY: �����λ�õ�����ռ�����,��λ����
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool SetAudio(const double fX, const double fY);
		/** @brief ���� */
		bool Play();
		/** @brief ��ͣ */
		bool Pause();
		/** @brief ֹͣ */
		bool Stop();
		/** @brief ��ѯ��һ�ף����д��ۣ��ص���ȥ�۲���ʷ��Ϣ��δ�����ı� */
		bool Last();
		/** @brief ��һ�� */
		bool Next();
		/** @brief ����������0.0-1.0 */
		bool SetVolume(const float fVolume);
		/** @brief ��ȡ������0.0-1.0 */
		float GetVolume() const;
		/**
		* @brief ���ò���ģʽ
		* @param eMode:			����ģʽ������ѭ����������š��б�ѭ���ȣ�
		* @return bool��		�ɹ�true�� ʧ��false
		*/
		bool SetPlayMode(EGMA_MODE eMode);
		/**
		* @brief ��ȡ����ģʽ	
		* @return EGMA_MODE��	����ģʽ������ѭ����������š��б�ѭ���ȣ�
		*/
		inline EGMA_MODE GetPlayMode() const
		{
			return m_ePlayMode;
		}

		/**
		* @brief ��ȡ��ǰ��Ƶ�ļ�����
		* @return std::wstring ��ǰ���ŵ���Ƶ�ļ����ƣ�����׺����δ�����򷵻� L""
		*/
		std::wstring GetAudioName() const;

		/**
		* @brief ������Ƶ�Ĳ���λ�ã���λ��ms
		* @param iTime: ��Ƶ�Ĳ���λ��
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool SetAudioCurrentTime(const int iTime);
		/**
		* @brief ��ȡ��Ƶ�Ĳ���λ�ã���λ��ms
		* @return int: ��Ƶ�Ĳ���λ��
		*/
		int GetAudioCurrentTime() const;

		/**
		* @brief ��ȡ��Ƶ����ʱ������λ��ms
		* @return int: ��Ƶ����ʱ��
		*/
		int GetAudioDuration() const;

		/**
		* @brief ��������ӭЧ����
		* ÿ�ο�����������ô˺�����ʵ�֡���ӭ���ܡ�
		*/
		void Welcome();
		/**
		* @brief ����ӭЧ�����Ƿ����
		* @return bool �����򷵻�true������false
		*/
		bool IsWelcomeFinished() const;

		/**
		* @brief ��ȡ��Ƶ���ŵ�˳���б�backλ��Ϊ���µ���Ƶ
		* @return std::vector<std::wstring>��	��Ƶ���ŵ�˳���б�
		*/
		const std::vector<std::wstring> GetPlayingOrder() const;

		/**
		* @brief ����ǰ���ռ�㼶���괫��
		* @param vHiePos:		��ǰ֡����ڵ�ǰ�ռ�㼶�µ�����
		* @return void
		*/
		void SetMousePosition(const SGMVector3& vHiePos);

		/**
		* @brief ���õ�ǰ���ŵĺ�����ʾ����Ƶ�ռ�����
		* ע�⣺������ ���ú������ݿ��е���������
		* @param vAudioCoord:	��ǰ���ŵĺ��ǵ���Ƶ�ռ�����
		* @return void
		*/
		void SetCurrentStarAudioCoord(const SGMAudioCoord& vAudioCoord);

		/**
		* @brief ��ȡ��ǰ���ŵĺ�����ʾ����Ƶ�ռ�����
		* ע�⣺��һ�����ں������ݿ��е��������ݣ��п��������޸�
		* @return SGMAudioCoord:	��ǰ���ŵĺ��ǵ���Ƶ�ռ�����
		*/
		SGMAudioCoord GetCurrentStarAudioCoord() const;

		/**
		* @brief ��ȡ��ǰ���ŵ���Ƶ������ռ�����
		* ���ڱ���Ʒ�������ԣ���Ҫ������ϵ������Ϊ���������
		* �����Ͳ��ÿ��Ǻ�����6���ռ��е�λ�õ��������(��double���Ȳ��㵼��)
		* ����Ϊ�˷�����㣬ֻ������ϵ�л���ƺ��ǣ�������ϵ������һ���ʵ�
		* @return SGMVector3	��ǰ���ŵ���Ƶ������ռ�����,��λ����
		*/
		SGMVector3 GetCurrentStarWorldPos() const;

		/**
		* @brief ��ȡָ����Ƶ����Ƶ�ռ�����
		* ע�⣺��һ�����ں������ݿ��е��������ݣ��п��������޸�
		* @param strName��			��Ƶ�ļ���
		* @return SGMAudioCoord:	��Ƶ�ռ�����
		*/
		SGMAudioCoord GetAudioCoord(const std::wstring& strName) const;

		/**
		* @brief ��ȡ��ϵ�뾶
		* @return double		��ϵ�뾶����λ����
		*/
		inline double GetGalaxyRadius() const
		{
			return m_fGalaxyDiameter * 0.5;
		}

		/**
		* @brief ��ȡ��ǰ�㼶�µ�Ŀ������
		* @return double		Ŀ�����룬��λ����ǰ�㼶��λ
		*/
		double GetHierarchyTargetDistance() const;

		/**
		* @brief ��ȡ��ǰ���ռ�㼶��λ������һ����λ���������,��ʼֵΪ1e20
		* Ϊ��ʵ�ִ� ��1.0���׵���1e30���׵��޷촩�󣬱�����6��double��������¼ÿ���ռ�㼶��ϵͳ��λ
		* Ȼ��Ϳ�������Щ�����������ϵͳ�������������
		*
		* ÿ��5��������������һ�Σ�������һ��ϵͳ��λ��ֻҪ6�����䣬�Ϳ���������������
		* �ڱ�ϵͳ�У������Ϊ6���ռ䣬0��-6����0��������߶ȣ�6�����������棬�������Խ�󣬼�����λԽ��
		* �ռ�㼶���:	0��		1��		2��		3��		4��		5��		6��
		* �ռ�㼶��λ:	1.0		1e5		1e10	1e15	1e20	1e25	1e30
		*
		* �����оٵ��͵ĳ߶ȣ��Ա��û���������߶ȵĸ���
		* ����ֱ����					1.275e7 ��
		* ̫��ֱ����					1.392e9 ��
		* ���ĵ�λ��AU����				1.496e11 ��
		* ������UY�������ǣ�ֱ����	2.36e12 ��
		* �����ǵĹ���볤�᣺			5.498e12 �ף�30.07 AU
		* ���꣨ly����					9.461e15 ��
		* �����������ǵľ��룺			4e16 �ף�ԼΪ 4.22����
		* ����ϵֱ����					1e21 �ף�ԼΪ 10�����
		* ��Ů��ϵ�����ǵľ��룺		2.5e22 �ף�ԼΪ 254�����
		* ����ϵȺֱ����				1e23 �ף�ԼΪ1000�����
		* ��Ů������ϵ��ֱ����			1e24 �ף�ԼΪ1.1�ڹ���
		* �����ǿ��ǳ���ϵ��ֱ����		5e24 �ף�ԼΪ5.2�ڹ���
		* �ɹ۲����淶Χ��				1e27 �ף�ԼΪ930�ڹ���
		* ����֮�⣬�����ҵ����磺		1e30 ��
		*
		* @param void					�������Ϊ�գ�����Ϊ������ǵ�ǰ�ռ�㼶���
		* @param iHierarchy				�ռ�㼶�Ĳ㼶���
		* @return double				����Ŀռ�㼶�ġ��ռ�㼶��λ��
		*/
		inline double GetUnit() const
		{
			return m_pKernelData->fUnitArray->at(m_pKernelData->iHierarchy);
		}
		double GetUnit(const int iHierarchy) const;

		/**
		* @brief ��ȡСһ���Ŀռ�㼶���Ϸ����������ڵ�ǰ�ռ�㼶����ϵ�µ�����ֵ
		* @param vX,vY,vZ:			��һ����X/Y/Z��λ����
		* @return bool:				�ɹ�true��ʧ��false */
		bool GetSmallerHierarchyCoord(SGMVector3& vX, SGMVector3& vY, SGMVector3& vZ) const;
		/**
		* ��ȡ��һ���Ŀռ�㼶���Ϸ����������ڵ�ǰ�ռ�㼶����ϵ�µ�����ֵ
		* @param vX,vY,vZ:			��һ����X/Y/Z��λ����
		* @return bool:				�ɹ�true��ʧ��false */
		bool GetBiggerHierarchyCoord(SGMVector3& vX, SGMVector3& vY, SGMVector3& vZ) const;
		/**
		* @brief ��ȡ��ǰ�ռ�㼶�Ĳ㼶���
		* �ڱ�ϵͳ�У������Ϊ6���ռ䣬0��-6����0��������߶ȣ�6������������
		* @return int				��ǰ�ռ�㼶�Ĳ㼶���
		*/
		inline int GetHierarchy() const
		{
			return m_pKernelData->iHierarchy;
		}
		/**
		* @brief ����ǰ�ռ�㼶�Ĳ㼶���+1��ֻ���� 0-6 ��Χ��
		* ͬʱ�����봫���۵��ڵ�ǰ�ռ�㼶�µĿռ�����
		* ����������һ���ռ��λ�ã��Լ���¼�Ժ󷵻ظÿռ�ʱ��λ��
		* �ڱ�ϵͳ�У������Ϊ6���ռ䣬0��-6����0��������߶ȣ�6������������
		* @param vHierarchyEyePos:		��ǰ�ռ�㼶�£��۵��ڲ㼶ԾǨʱ�Ŀռ�����
		* @param vHierarchyTargetPos:	��ǰ�ռ�㼶�£�Ŀ����ڲ㼶ԾǨʱ�Ŀռ�����
		* @return bool					�ɹ�true������ѵ�6��false
		*/
		bool AddHierarchy(const SGMVector3& vHierarchyEyePos, const SGMVector3& vHierarchyTargetPos);
		/**
		* @brief ����ǰ�ռ�㼶�Ĳ㼶���-1��ֻ���� 0-6 ��Χ��
		* ͬʱ�����봫���۵��Ŀ����ڵ�ǰ�ռ�㼶�µĿռ�����
		* �������㽵һ���ռ��λ�ã��Լ���¼�Ժ󷵻ظÿռ�ʱ��λ��
		* Ŀ���ռ�����ֶ���������ǣ�
		* 1.��ǰ�ռ�㼶Ϊ0��1ʱ to do
		* 2.��ǰ�ռ�㼶Ϊ2ʱ���ӿռ�ԭ����ǵ�ǰ������������
		* 3.��ǰ�ռ�㼶Ϊ4ʱ���ӿռ�ԭ����ǵ�ǰ������ǵ�����
		* 4.��ǰ�ռ�㼶Ϊ3��5��6ʱ���ӿռ�ԭ�� == ��ǰ�ռ�ԭ�� == (0,0,0)
		* �ڱ�ϵͳ�У������Ϊ6���ռ䣬0��-6����0��������߶ȣ�6������������
		* @param vHierarchyEyePos:		��ǰ�ռ�㼶�£��۵��ڲ㼶ԾǨʱ�Ŀռ�����
		* @param vHierarchyTargetPos:	��ǰ�ռ�㼶�£�Ŀ����ڲ㼶ԾǨʱ�Ŀռ�����
		* @return bool					�ɹ�true������ѵ�0��false
		*/
		bool SubHierarchy(const SGMVector3& vHierarchyEyePos, const SGMVector3& vHierarchyTargetPos);
		/**
		* @brief ��ȡ����������������ռ��µ�����
		* @param vBefore:			������������ǰ�Ŀռ��µ�ֵ
		* @return SGMVector4:		�����������������ռ��µ�ֵ
		*/
		SGMVector3 AfterAddHierarchy(const SGMVector4& vBefore) const;
		/**
		* @brief ��ȡ���������ڽ�����ռ��µ�����
		* @param vBefore:		�������ڽ���ǰ�Ŀռ��µ�ֵ
		* @return SGMVector4:		����������ڽ�����ռ��µ�ֵ
		*/
		SGMVector3 AfterSubHierarchy(const SGMVector4& vBefore) const;
		/**
		* @brief ��ȡ��ѯλ�ø��������һ�����壨���ǻ���ǣ���λ��
		* @param vSearchHiePos ��ѯλ�ã���ǰ�ռ�㼶����ϵ
		* @param vPlanetHiePos �������������λ�ã���ǰ�ռ�㼶����ϵ
		* @param fOrbitalPeriod ���ش����ǹ�ת������ڣ���λ����
		* @return bool ��3�㼶�ռ��£����λ�ø�����������true������false
		*/
		bool GetNearestCelestialBody(const SGMVector3& vSearchHiePos,
			SGMVector3& vPlanetHiePos, double& fOrbitalPeriod) const;
		/**
		* @brief ��ȡĿ��㸽����������壨���ǻ���ǣ���λ��
		* @param vPlanetPos �������������λ�ã���λ����
		* @param fOrbitalPeriod ���ش����ǹ�ת������ڣ���λ����
		*/
		void GetCelestialBody(SGMVector3& vPlanetPos, double& fOrbitalPeriod) const;

		/**
		* @brief ��ȡĿ��㸽����������壨���ǻ���ǣ���ƽ���뾶
		* @return double ����ƽ���뾶����λ����
		*/
		double GetCelestialMeanRadius() const;
		/**
		* @brief ��ȡĿ��㸽����������壨���ǻ���ǣ���ָ��γ�ȵĺ�ƽ�������ľ���
		* @param fLatitude γ�ȣ���λ���㣬��Χ��[-90.0, 90.0]
		* @return double ָ��γ�ȵĺ�ƽ�������ľ��룬��λ����
		*/
		double GetCelestialRadius(const double fLatitude) const;
		/**
		* @brief ��ȡĿ��㸽����������壨���ǻ���ǣ��ı����ᣨ��ǰ�㼶�ռ��£�
		* @return SGMVector3 �����ᣨ��ǰ�㼶�ռ��£�
		*/
		SGMVector3 GetCelestialNorth() const;

		/**
		* @brief ��ȡĿ��㸽����������壨���ǻ���ǣ��ڱ�����ϵ�е�����
		* @return unsigned int 0������ǣ�1����ת����뾶��С������
		*/
		unsigned int GetCelestialIndex() const;

		/**
		* @brief ��ȡ����ĺ���������ϵ����ϵ��4���ռ����������ϵ����λ��
		* @author LiuTao
		* @since 2023.01.16
		* @param void:
		* @return SGMVector3:		����ĺ���������ϵ����ϵ��4���ռ����������ϵ����λ��
		*/
		SGMVector3 GetNearStarWorldPos() const;

		/**
		* Angle2Color const
		* �����ֵġ������Ƕȡ�ת����ɫ����
		* @author LiuTao
		* @since 2022.09.04
		* @param fEmotionAngle��	�����Ƕȣ�[0.0,2*PI)
		* @return SGMVector4f��		���������Ͷ�Ӧ����ɫ,[0.0,1.0]
		*/
		SGMVector4f Angle2Color(const float fEmotionAngle) const;

		/** @brief �����ӿ�(QT:QWidget) */
		CGMViewWidget* CreateViewWidget(QWidget* parent);

	private:
		/**
		* @brief ��������
		*/
		bool _LoadConfig();
		/**
		* @brief ��ʼ��������ؽڵ�
		*/
		void _InitBackground();
		/**
		* @brief ��ʼ��ǰ����ؽڵ�
		*/
		void _InitForeground();
		/**
		* @brief ������һ��
		*/
		void _Next(const EGMA_MODE eMode);
		/**
		* @brief ������£�һ���Ӹ���10��
		* @param updateStep ���μ�����µ�ʱ����λs
		*/
		void _InnerUpdate(const float updateStep);
		/** @brief ����(�������������̬֮��) */
		bool _UpdateLater(const double dDeltaTime);
		/**
		* @brief ����ռ����� ת ���ӿռ�����
		* @param fX, fY: ����ռ�����
		* @param &fGalaxyX, &fGalaxyY: ���ӿռ�����[-1,1]
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool _World2GalaxyCoord(const double fX, const double fY, double& fGalaxyX, double& fGalaxyY) const;
		/**
		* @brief ���ӿռ����� ת ����ռ�����
		* @param fGalaxyX, fGalaxyY: ���ӿռ�����[-1,1]
		* @param &fX, &fY: ����ɹ������������ռ�����
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool _GalaxyCoord2World(const double fGalaxyX, const double fGalaxyY, double& fX, double& fY) const;

		/**
		* @brief ��ȡ����������������ռ��µ�����
		* @param vBefore:			������������ǰ�Ŀռ��µ�ֵ
		* @return osg::Vec3d:		����ĵ���������ռ��µ�ֵ
		*/
		osg::Vec3d _AfterAddHierarchy(const osg::Vec4d& vBefore) const;
		/**
		* @brief ��ȡ���������ڽ�����ռ��µ�����
		* @param vBefore:			�������ڽ���ǰ�Ŀռ��µ�ֵ
		* @return osg::Vec3d:		����������ڽ�����ռ��µ�ֵ
		*/
		osg::Vec3d _AfterSubHierarchy(const osg::Vec4d& vBefore) const;
		/**
		* @brief ���ڿռ�㼶�仯�����³���
		*/
		void _UpdateScenes();
		/**
		* @brief �������Զ������
		*/
		void _UpdateNearFar();
		/**
		* @brief ��Ϻ���,�ο� glsl �е� mix(a,b,x)
		* @param fMin, fMax:			��Χ
		* @param fX:					���ϵ��
		* @return double:				��Ϻ��ֵ
		*/
		inline double _Mix(const double fMin, const double fMax, const double fX)
		{
			return fMin * (1 - fX) + fMax * fX;
		}
		/**
		* @brief SGMVector3 ת osg::Vec3d
		* @param vGM:				�����GM����
		* @return osg::Vec3d:		�����osg���� */
		inline SGMVector3 _OSG2GM(const osg::Vec3d& vOSG) const
		{
			return SGMVector3(vOSG.x(), vOSG.y(), vOSG.z());
		}

		// ����
	private:

		SGMKernelData*						m_pKernelData;				//!< �ں�����
		SGMConfigData*						m_pConfigData;				//!< ��������
		CGMCommonUniform*					m_pCommonUniform;			//!< ����Uniform
		CGMDataManager*						m_pDataManager;				//!< ���ݹ���ģ��
		CGMCameraManipulator*				m_pManipulator;				//!< ���������

		bool								m_bInit;					//!< ��ʼ����־
		double								m_dTimeLastFrame;			//!< ��һ֡ʱ��
		float								m_fDeltaStep;				//!< ��λs
		float								m_fConstantStep;			//!< �ȼ�����µ�ʱ��,��λs
		double								m_fGalaxyDiameter;			//!< ��ϵֱ������λ����
		CGMGalaxy*							m_pGalaxy;					//!< ��ϵģ��
		CGMAudio*							m_pAudio;					//!< ��Ƶģ��
		CGMPost*							m_pPost;					//!< ����ģ��

		EGMA_MODE							m_ePlayMode;				//!< ��ǰ����ģʽ
		std::default_random_engine			m_iRandom;

		osg::ref_ptr<osg::Texture2D>		m_pSceneTex;				//!< ��������ɫͼ
		osg::ref_ptr<osg::Texture2D>		m_pBackgroundTex;			//!< ������ɫͼ
		osg::ref_ptr<osg::Texture2D>		m_pForegroundTex;			//!< ǰ����ɫͼ
	};
}	// GM