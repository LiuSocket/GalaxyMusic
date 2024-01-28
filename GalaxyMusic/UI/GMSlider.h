#pragma once

#include <QSlider>

class CGMSlider : public QSlider
{
	Q_OBJECT

public:
	CGMSlider(QWidget *parent);
	~CGMSlider();

protected:
	void mousePressEvent(QMouseEvent *event);
};
