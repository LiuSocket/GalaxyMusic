#ifndef VRCOLORLABEL_H
#define VRCOLORLABEL_H

#include <QLabel>

class CGMColorLabel : public QLabel
{
	Q_OBJECT

public:
	CGMColorLabel(QWidget *parent);
	~CGMColorLabel();

	/* 设置色块是否启用Alpha通道 */
	void	SetAlphaEnabled(bool bEnabled);
	/* 设置色块颜色 */
	void	SetColor(const QColor& color);
	/* 获取色块颜色 */
	QColor	GetColor();

protected:
	void paintEvent(QPaintEvent* event);
	void mousePressEvent(QMouseEvent *ev);
private:
	QColor			m_color;		// 当前颜色
	bool			m_bAlpha;		// 是否启用Alpha通道
signals:
	void clicked();
};

#endif // VRCOLORLABEL_H
