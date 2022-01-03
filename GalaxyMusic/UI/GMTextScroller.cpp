#include "GMTextScroller.h"
#include <QPainter>

/*************************************************************************
Macro Defines
*************************************************************************/
#define PAUSE_FRAME					100

CGMTextScroller::CGMTextScroller(QWidget *parent)
	: QLabel(parent), m_iCharWidth(0), m_iCurrentIndex(0), m_iTextWidth(0), m_iPauseCount(0),
	m_strSourceText(""), m_strShowText("")
{
	startTimer(30);
}

CGMTextScroller::~CGMTextScroller()
{
}

void CGMTextScroller::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);
	QRectF rectText = rect();
	rectText.setWidth(m_iTextWidth);
	rectText.setX(-m_iCurrentIndex);
	painter.drawText(rectText, alignment(), m_strShowText);
}

void CGMTextScroller::timerEvent(QTimerEvent *event)
{
	if (text().isEmpty())
	{
		return;
	}

	if (m_strSourceText != text())
	{
		m_strSourceText = text();

		if (fontMetrics().width(m_strSourceText) > width())
		{
			QString strTmp = m_strSourceText + "                    ";
			m_strShowText = strTmp + m_strSourceText.mid(0, width());
			m_iCurrentIndex = 0;
			m_iTextWidth = fontMetrics().width(strTmp) + width();
		}
		else
		{
			m_strShowText = m_strSourceText;
			m_iCurrentIndex = 0;
			m_iTextWidth = fontMetrics().width(m_strShowText);
		}
	}

	if (m_iTextWidth > width())
	{
		if(m_iPauseCount > PAUSE_FRAME)
			m_iCurrentIndex++;

		if (m_iCurrentIndex == (m_iTextWidth - width()))
		{
			m_iCurrentIndex = 0;
			m_iPauseCount = 0;
		}
	}
	m_iPauseCount++;

	update();
}
