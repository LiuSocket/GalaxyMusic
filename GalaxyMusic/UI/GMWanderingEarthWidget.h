#pragma once

#include <QWidget>
#include "ui_GMWanderingEarthWidget.h"

class CGMWanderingEarthWidget : public QWidget
{
	Q_OBJECT

public:
	CGMWanderingEarthWidget(QWidget* parent = nullptr);
	~CGMWanderingEarthWidget();

	/** @brief ��ʼ�� */
	bool Init();
	/** @brief ���� */
	void Update();

public slots:
	void _slotSetProgress(int iProgress);

private:
	Ui::GMWanderingEarthWidget ui;

	bool				m_bInit;
};
