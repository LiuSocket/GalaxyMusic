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
#include "GMEnums.h"
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
	class CGMDataManager;
	class CGMGalaxy;
	class CGMAudio;
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

		/** @brief ����/�رձ༭ģʽ */
		void SetEditMode(const bool bEnable);
		/** @brief ����/�رղ����� */
		void SetCapture(const bool bEnable);

		/**
		* SetAudio
		* @brief ��������ѡ����Ҫ��������Ƶ
		* @author LiuTao
		* @since 2021.05.30
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
		/** @brief ��һ�� */
		bool Last();
		/** @brief ��һ�� */
		bool Next();
		/** @brief ����������0.0-1.0 */
		bool SetVolume(float fVolume);
		/** @brief ��ȡ������0.0-1.0 */
		float GetVolume();
		/**
		* SetPlayMode
		* @brief ���ò���ģʽ
		* @author LiuTao
		* @since 2021.10.24
		* @param eMode:			����ģʽ������ѭ����������š��б�ѭ���ȣ�
		* @return bool��		�ɹ�true�� ʧ��false
		*/
		bool SetPlayMode(EGMA_MODE eMode);
		/**
		* GetPlayMode
		* @brief ��ȡ����ģʽ
		* @author LiuTao
		* @since 2021.10.24
		* @param void		
		* @return EGMA_MODE��	����ģʽ������ѭ����������š��б�ѭ���ȣ�
		*/
		EGMA_MODE GetPlayMode();

		/**
		* GetAudioName
		* ��ȡ��ǰ��Ƶ�ļ�����
		* @author LiuTao
		* @since 2021.09.11
		* @param void
		* @return std::wstring ��ǰ���ŵ���Ƶ�ļ����ƣ�����׺����δ�����򷵻� L""
		*/
		std::wstring GetAudioName();
		/**
		* SetAudioCurrentTime
		* @brief ������Ƶ�Ĳ���λ�ã���λ��ms
		* @author LiuTao
		* @since 2021.09.05
		* @param iTime: ��Ƶ�Ĳ���λ��
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool SetAudioCurrentTime(int iTime);
		/**
		* GetAudioCurrentTime
		* @brief ��ȡ��Ƶ�Ĳ���λ�ã���λ��ms
		* @author LiuTao
		* @since 2021.09.05
		* @param void
		* @return int: ��Ƶ�Ĳ���λ��
		*/
		int GetAudioCurrentTime();
		/**
		* GetAudioDuration
		* @brief ��ȡ��Ƶ����ʱ������λ��ms
		* @author LiuTao
		* @since 2021.09.05
		* @param void
		* @return int: ��Ƶ����ʱ��
		*/
		int GetAudioDuration();

		/**
		* Welcome
		* @brief ��������ӭЧ����
		* @brief ÿ�ο�����������ô˺�����ʵ�֡���ӭ���ܡ�
		* @author LiuTao
		* @since 2021.08.28
		* @param void
		* @return void
		*/
		void Welcome();
		/**
		* WelcomeFinished
		* @brief ����ӭЧ�����Ƿ����
		* @author LiuTao
		* @since 2021.09.11
		* @param void
		* @return bool �����򷵻�true������false
		*/
		bool IsWelcomeFinished();

		/**
		* SetMousePosition
		* ����ǰ���ռ�㼶���괫��
		* @author LiuTao
		* @since 2021.07.04
		* @param vPos:			��ǰ֡����ڵ�ǰ�ռ�㼶�µ�����
		* @return void
		*/
		void SetMousePosition(const osg::Vec3f& vHierarchyPos);

		/**
		* GetCurrentStarWorldPos
		* ��ȡ��ǰ���ŵ���Ƶ������ռ�����
		* ���ڱ���Ʒ�������ԣ���Ҫ������ϵ������Ϊ���������
		* �����Ͳ��ÿ�����double��ʾ������6���ռ��е�λ�õ��������
		* ����Ϊ�˷�����㣬ֻ������ϵ�л������ǣ�������ϵ������һ���ʵ�
		* �涨�����ǵ��������С��λ����GM_STAR_POS_MIN = 1e10�ף���3����Ծ����2��ʱ������뾶��5e11��
		* ����������ȿ���������ϵ�з��������ڿź��Ƕ�û���ظ��У������˲�Ʒ����
		* @author LiuTao
		* @since 2021.07.30
		* @param void
		* @return osg::Vec3d	��ǰ���ŵ���Ƶ������ռ�����,��λ����
		*/
		osg::Vec3d GetCurrentStarWorldPos();

		/**
		* GetGalaxyRadius
		* ��ȡ��ϵ�뾶
		* @author LiuTao
		* @since 2021.08.22
		* @param void
		* @return double		��ϵ�뾶����λ����
		*/
		double GetGalaxyRadius();

		/**
		* GetHierarchyTargetDistance
		* ��ȡ��ǰ�㼶�µ�Ŀ������
		* @author LiuTao
		* @since 2021.10.05
		* @param void
		* @return double		Ŀ�����룬��λ����ǰ�㼶��λ
		*/
		double GetHierarchyTargetDistance();

		/**
		* GetUnit
		* ��ȡ��ǰ���ռ�㼶��λ������һ����λ���������,��ʼֵΪ1e20
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
		* @author LiuTao
		* @since 2021.09.11
		* @param void					�������Ϊ�գ�����Ϊ������ǵ�ǰ�ռ�㼶���
		* @param iHierarchy				�ռ�㼶�Ĳ㼶���
		* @return double				����Ŀռ�㼶�ġ��ռ�㼶��λ��
		*/
		inline double GetUnit()
		{
			return m_pKernelData->fUnitArray->at(m_pKernelData->iHierarchy);
		}
		double GetUnit(int iHierarchy);

		/**
		* GetHierarchyLastEyePos
		* ��ȡ��ǰ�ռ�㼶��Ծǰ�����ʱ�̵��۵�ռ�����
		* Ϊ��ʵ�ִ� ��1.0���׵���1e30���׵��޷촩�󣬱�����14��Vec3d��¼�۵��Ŀ�����7���ռ�㼶�н���ʱ�Ŀռ�����
		* Ȼ������Щ����������Ƶ�ǰ�ռ�㼶���������ƽ��
		* ÿ��5��������������һ�Σ����ı�һ��ϵͳԭ�㣻ֻҪ6�����䣬�Ϳ���������������
		* �ڱ�ϵͳ�У������Ϊ6���ռ䣬0��-6����0��������߶ȣ�6�����������棬�������Խ�󣬼�����λԽ��
		* @author LiuTao
		* @since 2021.09.25
		* @param iHierarchy:			�㼶��ţ�-1���ʾ��ǰ�㼶
		* @return Vec3d��				�۵��ڸÿռ�㼶��Ծǰ�����ʱ�̵Ŀռ����꣬��ʼʱȫΪ(1,1,1)
		*/
		osg::Vec3d GetHierarchyLastEyePos(int iHierarchy = -1);

		/**
		* GetHierarchyLastTargetPos
		* ��ȡ��ǰ�ռ�㼶��Ծǰ�����ʱ�̵�Ŀ���ռ�����
		* @author LiuTao
		* @since 2021.09.25
		* @param iHierarchy:			�㼶��ţ�-1���ʾ��ǰ�㼶
		* @return Vec3d:				Ŀ����ڸÿռ�㼶��Ծǰ�����ʱ�̵Ŀռ����꣬��ʼʱȫΪ0
		*/
		osg::Vec3d GetHierarchyLastTargetPos(int iHierarchy = -1);

		/**
		* GetHierarchy
		* ��ȡ��ǰ�ռ�㼶�Ĳ㼶���
		* �ڱ�ϵͳ�У������Ϊ6���ռ䣬0��-6����0��������߶ȣ�6������������
		* @author LiuTao
		* @since 2021.09.12
		* @param void
		* @return int				��ǰ�ռ�㼶�Ĳ㼶���
		*/
		inline int GetHierarchy()
		{
			return m_pKernelData->iHierarchy;
		}
		/**
		* AddHierarchy
		* ����ǰ�ռ�㼶�Ĳ㼶���+1��ֻ���� 0-6 ��Χ��
		* ͬʱ�����봫���۵��ڵ�ǰ�ռ�㼶�µĿռ�����
		* ����������һ�㼶�ռ��λ�ã��Լ���¼�Ժ�ԭ·���ظÿռ�ʱ��λ��
		* �ڱ�ϵͳ�У������Ϊ6���ռ䣬0��-6����0��������߶ȣ�6������������
		* @author LiuTao
		* @since 2021.09.12
		* @param vHierarchyEyePos:		��ǰ�ռ�㼶�£��۵��ڲ㼶ԾǨʱ�Ŀռ�����
		* @param vHierarchyTargetPos:	��ǰ�ռ�㼶�£�Ŀ����ڲ㼶ԾǨʱ�Ŀռ�����
		* @return bool					�ɹ�true������ѵ�6��false
		*/
		bool AddHierarchy(const osg::Vec3d& vHierarchyEyePos, const osg::Vec3d& vHierarchyTargetPos);
		/**
		* SubHierarchy
		* ����ǰ�ռ�㼶�Ĳ㼶���-1��ֻ���� 0-6 ��Χ��
		* ͬʱ�����봫���۵��Ŀ����ڵ�ǰ�ռ�㼶�µĿռ�����
		* ����������һ�㼶�ռ��λ�ã��Լ���¼�Ժ�ԭ·���ظÿռ�ʱ��λ��
		* Ŀ���ռ����������������ǣ�
		* 1.��ǰ�ռ�㼶Ϊ0��1��2��3��4ʱ���ӿռ�ԭ����ǵ�ǰ�ռ��Ŀ���
		* 2.��ǰ�ռ�㼶Ϊ5��6ʱ���ӿռ�ԭ�� == ��ǰ�ռ�ԭ�� == (0,0,0)
		* �ڱ�ϵͳ�У������Ϊ6���ռ䣬0��-6����0��������߶ȣ�6������������
		* @author LiuTao
		* @since 2021.09.12
		* @param vHierarchyEyePos:		��ǰ�ռ�㼶�£��۵��ڲ㼶ԾǨʱ�Ŀռ�����
		* @param vHierarchyTargetPos:	��ǰ�ռ�㼶�£�Ŀ����ڲ㼶ԾǨʱ�Ŀռ�����
		* @return bool					�ɹ�true������ѵ�0��false
		*/
		bool SubHierarchy(const osg::Vec3d& vHierarchyEyePos, const osg::Vec3d& vHierarchyTargetPos);

		/**
		* Hierarchy2World
		* �㼶�ռ�����ת����ռ����꣬����double���Ȳ������������
		* @author LiuTao
		* @since 2021.09.19
		* @param vHierarchy��		�㼶�ռ�����
		* @return osg::Vec3d��		����ռ�����
		*/
		osg::Vec3d Hierarchy2World(osg::Vec3d vHierarchy);

		/**
		* StarWorld2Hierarchy
		* ���ǵ�����ռ�����ת�㼶�ռ����ֻ꣬�����ں����������
		* ���ǵ�����ռ�������С����Ϊ1e10,��ֻ��������ϵ�ڵĺ���
		* ������double���Ծ�ȷ��ʾÿ�ź��ǵ�����ռ�����
		* @author LiuTao
		* @since 2021.09.19
		* @param vStarWorldPos��	���ǵ�����ռ�����
		* @return osg::Vec3d��		�㼶�ռ�����
		*/
		osg::Vec3d StarWorld2Hierarchy(osg::Vec3d vStarWorldPos);

		/** @brief �����ӿ�(QT:QWidget) */
		CGMViewWidget* CreateViewWidget(QWidget* parent);

		/** @brief ��ȡ��Ƶ���ݹ����� */
		inline CGMDataManager* GetDataManager() { return m_pDataManager; }

	private:
		/**
		* _Next
		* ������һ��
		* @author LiuTao
		* @since 2021.07.23
		* @param void
		* @return void
		*/
		void _Next(EGMA_MODE eMode);

		/**
		* _InnerUpdate
		* �������
		* @author LiuTao
		* @since 2021.07.04
		* @param updateStep ���μ�����µ�ʱ����λs
		* @return void
		*/
		void _InnerUpdate(float updateStep);

		/**
		* _World2GalaxyCoord
		* @brief ����ռ����� ת ���ӿռ�����
		* @author LiuTao
		* @since 2021.06.22
		* @param fX, fY: ����ռ�����
		* @param &fGalaxyX, &fGalaxyY: ���ӿռ�����[-1,1]
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool _World2GalaxyCoord(const double fX, const double fY, double& fGalaxyX, double& fGalaxyY);

		/**
		* _World2GalaxyCoord
		* @brief ���ӿռ����� ת ����ռ�����
		* @author LiuTao
		* @since 2021.07.04
		* @param fGalaxyX, fGalaxyY: ���ӿռ�����[-1,1]
		* @param &fX, &fY: ����ɹ������������ռ�����
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool _GalaxyCoord2World(const double fGalaxyX, const double fGalaxyY, double& fX, double& fY);

		/**
		* _StarCoord2World
		* @brief �ǳ����� ת ����ռ�����
		* @author LiuTao
		* @since 2021.07.04
		* @param starCoord: �ǳ�����
		* @param &fX, &fY: ����ɹ������������ռ�����
		* @return bool �ɹ�true�� ʧ��false
		*/
		bool _StarCoord2World(const SGMStarCoord& starCoord, double& fX, double& fY);

		/**
		* _UpdateScenes
		* @brief ���ڿռ�㼶�仯�����³���
		* @author LiuTao
		* @since 2021.09.15
		* @param void
		* @return void
		*/
		void _UpdateScenes();

		// ����
	private:

		SGMKernelData*						m_pKernelData;				//!< �ں�����
		SGMConfigData*						m_pConfigData;				//!< ��������
		CGMDataManager*						m_pDataManager;				//!< ���ݹ���ģ��
		CGMCameraManipulator*				m_pManipulator;				//!< ���������

		bool								m_bInit;					//!< ��ʼ����־
		bool								m_bDirty;					//!< ����
		double								m_dTimeLastFrame;			//!< ��һ֡ʱ��
		float								m_fDeltaStep;				//!< ��λs
		float								m_fConstantStep;			//!< �ȼ�����µ�ʱ��,��λs
		double								m_fGalaxyDiameter;			//!< ��ϵֱ������λ����
		CGMGalaxy*							m_pGalaxy;					//!< ��ϵģ��
		CGMAudio*							m_pAudio;					//!< ��Ƶģ��
		EGMA_MODE							m_ePlayMode;				//!< ��ǰ����ģʽ

		std::default_random_engine			m_iRandom;
	};
}	// GM