#pragma once

#include <QWidget>
#include "ui_GMWanderingEarthWidget.h"

class CGMWanderingEarthWidget : public QWidget
{
	Q_OBJECT

public:
	CGMWanderingEarthWidget(QWidget* parent = nullptr);
	~CGMWanderingEarthWidget();

	/** @brief ≥ı ºªØ */
	bool Init();

private:
	Ui::GMWanderingEarthWidget ui;

	bool				m_bInit;
};
