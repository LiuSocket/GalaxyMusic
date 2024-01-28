#pragma once

#include <QWidget>
#include "GMAudioWidget.h"
#include "ui_GMListWidget.h"

class CGMListWidget : public QWidget
{
	Q_OBJECT

public:
	CGMListWidget(QWidget *parent = Q_NULLPTR);
	~CGMListWidget();

	/** @brief ��ʼ�� */
	bool Init();
	/**
	* AddAudio
	* @brief  ���б����������Ƶ
	* @param strAudioTitle��		��Ƶ����
	* @param strArtistName��		��������
	* @param fBPM��					��ƵBPM��ÿ���ӽ�����
	* @param fAngle��				��Ƶ�Ƕ�
	* @return bool:				�ɹ�����true�� strAudioNameΪ�ջ���ʧ�ܷ���false
	*/
	bool AddAudio(
		const QString& strAudioTitle,
		const QString& strArtistName,
		const double fBPM,
		const double fAngle);

	/** @brief ȷ�����һ�׸���ʾ */
	void EnsureLastAudioVisible();

private:
	Ui::CGMListWidget					ui;

	bool								m_bInit;
	std::vector<CGMAudioWidget*>		m_vAudioWidgetVector;
};
