#pragma once

#include <QLabel>

class CGMTextScroller : public QLabel
{
	Q_OBJECT

public:
	CGMTextScroller(QWidget *parent = Q_NULLPTR);
	~CGMTextScroller();
	inline void setInterVal(int nInterVal)
	{
		startTimer(nInterVal);
	}

protected:
	void paintEvent(QPaintEvent *event);
	void timerEvent(QTimerEvent *event);

private:
	int m_iCharWidth;
	int m_iCurrentIndex;			// 文本起始x点，单位：px
	int m_iTextWidth;
	int m_iPauseCount;				// 文本暂停滚动的时间，单位：帧
	QString m_strSourceText;		// 原文本
	QString m_strShowText;			// 显示的文本
};