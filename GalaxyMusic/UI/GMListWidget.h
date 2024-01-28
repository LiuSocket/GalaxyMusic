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

	/** @brief 初始化 */
	bool Init();
	/**
	* AddAudio
	* @brief  在列表的最后添加音频
	* @param strAudioTitle：		音频标题
	* @param strArtistName：		艺术家名
	* @param fBPM：					音频BPM，每分钟节拍数
	* @param fAngle：				音频角度
	* @return bool:				成功返回true， strAudioName为空或者失败返回false
	*/
	bool AddAudio(
		const QString& strAudioTitle,
		const QString& strArtistName,
		const double fBPM,
		const double fAngle);

	/** @brief 确保最后一首歌显示 */
	void EnsureLastAudioVisible();

private:
	Ui::CGMListWidget					ui;

	bool								m_bInit;
	std::vector<CGMAudioWidget*>		m_vAudioWidgetVector;
};
