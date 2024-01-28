#include "GMSlider.h"
#include <QMouseEvent>

CGMSlider::CGMSlider(QWidget *parent)
	: QSlider(parent)
{
}

CGMSlider::~CGMSlider()
{
}

void CGMSlider::mousePressEvent(QMouseEvent* event)
{
	if (Qt::Horizontal == orientation())
	{
		double pos = event->pos().x() / (double)width();
		int value = pos * (maximum() - minimum()) + minimum();
		setValue(value);
	}
	else
	{	
		double pos = event->pos().y() / (double)height();
		int value = maximum() - pos * (maximum() - minimum());
		setValue(value);
	}

	QSlider::mousePressEvent(event);//���ø��������������¼����������Բ�Ӱ���϶������
}
