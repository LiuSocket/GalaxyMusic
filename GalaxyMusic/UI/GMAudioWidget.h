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
	* @brief 传入音频标题
	* @param strAudio:		音频名称
	* @return bool:			成功true， 失败false
	*/
	bool SetTitle(const QString& strTitle);

	/**
	* SetArtist
	* @brief 传入艺术家名
	* @param strArtist:		艺术家名
	* @return bool:			成功true， 失败false
	*/
	bool SetArtist(const QString& strArtist);

	/**
	* SetBPM
	* @brief 传入音频BPM
	* @param fBPM:			音频BPM，节拍每分钟
	* @return bool:			成功true， 失败false
	*/
	bool SetBPM(const double fBPM);

	/**
	* SetColor
	* @brief 传入音频情绪颜色
	* @param color:			音频情绪颜色
	* @return bool:			成功true， 失败false
	*/
	bool SetColor(const QColor color);

	/**
	* GetTitle
	* @brief 获取音频标题
	* @param void:				无
	* @return QString:			音频名称（不带作者）
	*/
	QString GetTitle();

	/**
	* GetArtist
	* @brief 获取艺术家名
	* @param void:				无
	* @return QString:			艺术家名
	*/
	QString GetArtist();

private:
	Ui::GMAudioWidget ui;
};
