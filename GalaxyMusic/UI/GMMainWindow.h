#pragma once

#include <QMainWindow>
#include "ui_GMMainWindow.h"

/*************************************************************************
Class
*************************************************************************/
class CGMVolumeWidget;
class CGMListWidget;
class CGMWanderingEarthWidget;
class CGMViewWidget;

class CGMMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	CGMMainWindow(QWidget *parent = Q_NULLPTR);
	~CGMMainWindow();

	/** @brief ��ʼ�� */
	bool Init();
	/** @brief ���� */
	void Update();

	/**
	* @brief ����ȫ���л�
	* @param bFull trueΪȫ����false Ϊ��ȫ��
	*/
	void SetFullScreen(const bool bFull);
	/**
	* @brief ��ȡ����ȫ��״̬
	* @return bool		�Ƿ���ȫ��״̬
	*/
	bool GetFullScreen();

	/**
	* @brief ������Ƶ��������Ϣ
	*/
	void UpdateAudioInfo();

	/**
	* @brief �����Ƿ���ʾʵʱ�仯������
	* @param bVisible �Ƿ���ʾʵʱ�仯������
	*/
	void SetVolumeVisible(const bool bVisible);

public slots:

	/**
	* @brief ��һ��
	*/
	void _slotLast();

	/**
	* @brief ����/��ͣ
	*/
	void _slotPlayOrPause();

	/**
	* @brief ��һ��
	*/
	void _slotNext();

	/**
	* @brief ��С��/��ԭ
	*/
	void _slotMinimum();
	/**
	* @brief ���/��ԭ
	*/
	void _slotMaximum();
	/**
	* @brief �ر�
	*/
	void _slotClose();

	/** 
	* @brief ������Ƶ��ǰ��ʱ��
	* @param iTimeRatio: ��ǰʱ������ʱ��ı�ֵ,[0,100]
	*/
	void _slotSetAudioTime(int iTimeRatio);

	/**
	* @brief ����/����
	*/
	void _slotSetMute();
	/**
	* @brief ��������
	* @param iVolume	������[0, 100]
	*/
	void _slotSetVolume(int iVolume);

	/**
	* @brief ���ò����б����ʾ������
	*/
	void _slotListVisible();

	/**
	* @brief ����ȫ��
	*/
	void _slotFullScreen();

	/**
	* @brief ������ά����
	*/
	void _slotEnter3D();

protected:
	void changeEvent(QEvent* event);
	void resizeEvent(QResizeEvent* event);
	void closeEvent(QCloseEvent* event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent* event);
	/** @brief �����ϵļ����¼� */
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);

private:
	/**
	* _Million2MinutesSeconds
	* @brief ������ ת ����:����
	* @param ms: ���������
	* @param minutes: ���������
	* @param seconds: �������
	* @return void
	*/
	void _Million2MinutesSeconds(const int ms, int& minutes, int& seconds);

	/**
	* _SetPlayingListGeometry
	* @brief ���ò����б��λ�á��ߴ�
	* @param void
	* @return void
	*/
	void _SetPlayingListGeometry();

private:
	Ui::GMMainWindow					ui;
	CGMVolumeWidget*					m_pVolumeWidget;
	CGMListWidget*						m_pListWidget;
	CGMWanderingEarthWidget*			m_pWanderingEarthWidget;
	CGMViewWidget*						m_pSceneWidget;

	bool								m_bInit;
	bool								m_bFull;				//!< �Ƿ�ȫ��
	bool								m_bPressed;				//!< �Ƿ������
	bool								m_bShowVolume;			//!< �Ƿ���ʾʵʱ�仯������
	QPoint								m_vPos;					//!< ���ڵ�λ��
	int									m_iAudioDuration;		//!< ��Ƶ��ʱ������λ��ms
	QString								m_strName;				//!< ��Ƶ�ļ����ƣ�������׺��
};