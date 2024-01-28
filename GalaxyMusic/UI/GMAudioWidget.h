#pragma once

#include <QWidget>
#include "ui_GMAudioWidget.h"

class CGMAudioWidget : public QWidget
{
	Q_OBJECT

public:
	CGMAudioWidget(QWidget *parent = Q_NULLPTR);
	~CGMAudioWidget();

	/**
	* SetTitle
	* @brief ������Ƶ����
	* @param strAudio:		��Ƶ����
	* @return bool:			�ɹ�true�� ʧ��false
	*/
	bool SetTitle(const QString& strTitle);

	/**
	* SetArtist
	* @brief ������������
	* @param strArtist:		��������
	* @return bool:			�ɹ�true�� ʧ��false
	*/
	bool SetArtist(const QString& strArtist);

	/**
	* SetBPM
	* @brief ������ƵBPM
	* @param fBPM:			��ƵBPM������ÿ����
	* @return bool:			�ɹ�true�� ʧ��false
	*/
	bool SetBPM(const double fBPM);

	/**
	* SetColor
	* @brief ������Ƶ������ɫ
	* @param color:			��Ƶ������ɫ
	* @return bool:			�ɹ�true�� ʧ��false
	*/
	bool SetColor(const QColor color);

	/**
	* GetTitle
	* @brief ��ȡ��Ƶ����
	* @param void:				��
	* @return QString:			��Ƶ���ƣ��������ߣ�
	*/
	QString GetTitle();

	/**
	* GetArtist
	* @brief ��ȡ��������
	* @param void:				��
	* @return QString:			��������
	*/
	QString GetArtist();

private:
	Ui::GMAudioWidget ui;
};
