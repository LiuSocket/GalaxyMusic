#pragma once

#include <QMainWindow>
#include "ui_GMMainWindow.h"

/*************************************************************************
Class
*************************************************************************/
class CGMVolumeWidget;

class CGMMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	CGMMainWindow(QWidget *parent = Q_NULLPTR);
	~CGMMainWindow();

	/** @brief ��ʼ�� */
	bool Init();

	/**
	* SetFullScreen
	* @brief ����ȫ���л�
	* @author LiuTao
	* @since 2021.09.05
	* @param bFull trueΪȫ����false Ϊ��ȫ��
	* @return void
	*/
	void SetFullScreen(const bool bFull);
	/**
	* GetFullScreen
	* @brief ��ȡ����ȫ��״̬
	* @author LiuTao
	* @since 2021.10.23
	* @return bool		�Ƿ���ȫ��״̬
	*/
	bool GetFullScreen();

	/**
	* UpdateAudioInfo
	* ������Ƶ��������Ϣ
	* @author LiuTao
	* @since 2021.09.05
	* @return void
	*/
	void UpdateAudioInfo();

public slots:

	/**
	* _slotLast
	* @brief ��һ��
	* @param void
	* @return void
	*/
	void _slotLast();

	/**
	* _slotPlayOrPause
	* @brief ����/��ͣ
	* @param void
	* @return void
	*/
	void _slotPlayOrPause();

	/**
	* _slotNext
	* @brief ��һ��
	* @param void
	* @return void
	*/
	void _slotNext();

	/**
	* _slotMinimum
	* @brief ��С��/��ԭ
	* @param void
	* @return void
	*/
	void _slotMinimum();
	/**
	* _slotMaximum
	* @brief ���/��ԭ
	* @param void
	* @return void
	*/
	void _slotMaximum();
	/**
	* _slotClose
	* @brief �ر�
	* @param void
	* @return void
	*/
	void _slotClose();

	/** 
	* _slotSetAudioTime
	* @brief ������Ƶ��ǰ��ʱ��
	* @param iTimeRatio: ��ǰʱ������ʱ��ı�ֵ
	* @return void
	*/
	void _slotSetAudioTime(int iTimeRatio);

	/**
	* _slotSetMute
	* @brief ����/����
	* @param void
	* @return void
	*/
	void _slotSetMute();
	/**
	* _slotSetVolume
	* @brief ��������
	* @param iVolume	������[0, 100]
	* @return void
	*/
	void _slotSetVolume(int iVolume);

	/**
	* _slotSetFullScreen
	* @brief ����ȫ��
	* @param void
	* @return void
	*/
	void _slotFullScreen();

protected:
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

private:
	Ui::GMMainWindow					ui;
	CGMVolumeWidget*					m_pVolumeWidget;

	bool								m_bInit;
	bool								m_bFull;				//!< �Ƿ�ȫ��
	bool								m_bPressed;				//!< �Ƿ������
	QPoint								m_vPos;					//!< ���ڵ�λ��
	int									m_iAudioDuration;		//!< ��Ƶ��ʱ������λ��ms
	QString								m_strName;				//!< ��Ƶ�ļ����ƣ�������׺��
};