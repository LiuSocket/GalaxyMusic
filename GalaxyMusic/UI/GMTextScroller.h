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
	int m_iCurrentIndex;			// �ı���ʼx�㣬��λ��px
	int m_iTextWidth;
	int m_iPauseCount;				// �ı���ͣ������ʱ�䣬��λ��֡
	QString m_strSourceText;		// ԭ�ı�
	QString m_strShowText;			// ��ʾ���ı�
};