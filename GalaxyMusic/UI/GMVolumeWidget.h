#pragma once

#include <QWidget>
#include "ui_GMVolumeWidget.h"

class CGMVolumeWidget : public QWidget
{
	Q_OBJECT

public:
	CGMVolumeWidget(QWidget *parent = Q_NULLPTR);
	~CGMVolumeWidget();

	/** @brief ����������0-100 */
	bool SetVolume(const int iVolume);
	/** @brief ��ȡ������0-100 */
	int GetVolume();

protected:
	void leaveEvent(QEvent *);

signals:
	void _signalHideVolume();
	void _signalSetVolume(int iVolume);

private:
	Ui::GMVolumeWidget	ui;
};
