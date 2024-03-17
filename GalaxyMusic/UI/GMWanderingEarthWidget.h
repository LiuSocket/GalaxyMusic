#pragma once

#include <QWidget>
#include "ui_GMWanderingEarthWidget.h"

class CGMWanderingEarthWidget : public QWidget
{
	Q_OBJECT

public:
	CGMWanderingEarthWidget(QWidget* parent = nullptr);
	~CGMWanderingEarthWidget();

	/** @brief 初始化 */
	bool Init();
	/** @brief 更新 */
	void Update();

public slots:
	void _slotSetProgress(int iProgress);

private:
	Ui::GMWanderingEarthWidget ui;

	bool				m_bInit;
};
