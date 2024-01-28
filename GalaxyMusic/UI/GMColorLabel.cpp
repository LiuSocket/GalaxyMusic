#include "GMColorLabel.h"
#include <QFile>
#include <QPainter>
// #include <QColorDialog>

CGMColorLabel::CGMColorLabel(QWidget *parent)
	: QLabel(parent), m_color(QColor(255,255,255,255)), m_bAlpha(false)
{
	QString qss;
	QFile DPditorFile(":/Resources/Empty.qss");
	DPditorFile.open(QFile::ReadOnly);
	if (DPditorFile.isOpen())
	{
		qss = QLatin1String(DPditorFile.readAll());
		setStyleSheet(qss);
		DPditorFile.close();
	}
}

CGMColorLabel::~CGMColorLabel()
{

}

QColor CGMColorLabel::GetColor()
{
	return m_color;
}

/* 设置色块是否启用Alpha通道 */
void CGMColorLabel::SetAlphaEnabled(bool bEnabled)
{
	m_bAlpha = bEnabled;
	if (!m_bAlpha)
	{
		m_color.setAlpha(255);

		QPalette palette;
		palette.setColor(QPalette::Base, QColor(m_color.red(), m_color.green(), m_color.blue()));
		setPalette(palette);
		update();
	}
}

void CGMColorLabel::SetColor(const QColor& color)
{
	m_color = color;

	QPalette palette;
	palette.setColor(QPalette::Base, QColor(m_color.red(), m_color.green(), m_color.blue()));
	setPalette(palette);
	update();
}

void CGMColorLabel::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	QTransform transform;
	painter.setTransform(transform);
	painter.setPen(Qt::NoPen);//设置边框颜色(画笔形式)

	if (m_bAlpha)
	{
		//设置RGB颜色
		painter.setBrush(QBrush(QColor(m_color.red(), m_color.green(), m_color.blue()), Qt::SolidPattern));//设置内部填充色（画刷形式）
		painter.drawRect(0, 0, width() - 1, height() - 3);

		//设置Alpha通道的数值
		painter.setBrush(QBrush(Qt::black, Qt::SolidPattern));//设置内部填充色（画刷形式）
		painter.drawRect(0, height() - 3, width() - 1, 2);
		painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));//设置内部填充色（画刷形式）
		painter.drawRect(0, height() - 3, width()*m_color.alphaF() - 1, 2);
	}
	else
	{
		// 绘制边缘辉光效果
		painter.setBrush(QBrush(QColor(m_color.red(), m_color.green(), m_color.blue(), 32), Qt::SolidPattern));//设置内部填充色（画刷形式）
		painter.drawRoundRect(0, 0, width() - 1, height() - 1, 80, 80);
		painter.setBrush(QBrush(QColor(m_color.red(), m_color.green(), m_color.blue(), 64), Qt::SolidPattern));//设置内部填充色（画刷形式）
		painter.drawRoundRect(1, 1, width() - 3, height() - 3, 80, 80);

		//设置RGB颜色
		painter.setBrush(QBrush(QColor(m_color.red(), m_color.green(), m_color.blue()), Qt::SolidPattern));//设置内部填充色（画刷形式）
		painter.drawRoundRect(2, 2, width() - 5, height() - 5, 90, 90);
	}
}

void CGMColorLabel::mousePressEvent(QMouseEvent *ev)
{
	//QColor color;
	//if (m_bAlpha)
	//{
	//	color = QColorDialog::getColor(m_color, this, QString::fromLocal8Bit(" 获取颜色 "), QColorDialog::ShowAlphaChannel);
	//}
	//else
	//{
	//	color = QColorDialog::getColor(m_color, this, QString::fromLocal8Bit(" 获取颜色 "));
	//}

	//if (color.isValid())
	//{
	//	m_color = color;
	//}
	update();
	emit clicked();
}
