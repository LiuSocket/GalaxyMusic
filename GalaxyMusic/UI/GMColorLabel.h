#ifndef VRCOLORLABEL_H
#define VRCOLORLABEL_H

#include <QLabel>

class CGMColorLabel : public QLabel
{
	Q_OBJECT

public:
	CGMColorLabel(QWidget *parent);
	~CGMColorLabel();

	/* ����ɫ���Ƿ�����Alphaͨ�� */
	void	SetAlphaEnabled(bool bEnabled);
	/* ����ɫ����ɫ */
	void	SetColor(const QColor& color);
	/* ��ȡɫ����ɫ */
	QColor	GetColor();

protected:
	void paintEvent(QPaintEvent* event);
	void mousePressEvent(QMouseEvent *ev);
private:
	QColor			m_color;		// ��ǰ��ɫ
	bool			m_bAlpha;		// �Ƿ�����Alphaͨ��
signals:
	void clicked();
};

#endif // VRCOLORLABEL_H
