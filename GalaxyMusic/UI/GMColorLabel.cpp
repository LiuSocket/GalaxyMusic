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

/* ����ɫ���Ƿ�����Alphaͨ�� */
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
	painter.setPen(Qt::NoPen);//���ñ߿���ɫ(������ʽ)

	if (m_bAlpha)
	{
		//����RGB��ɫ
		painter.setBrush(QBrush(QColor(m_color.red(), m_color.green(), m_color.blue()), Qt::SolidPattern));//�����ڲ����ɫ����ˢ��ʽ��
		painter.drawRect(0, 0, width() - 1, height() - 3);

		//����Alphaͨ������ֵ
		painter.setBrush(QBrush(Qt::black, Qt::SolidPattern));//�����ڲ����ɫ����ˢ��ʽ��
		painter.drawRect(0, height() - 3, width() - 1, 2);
		painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));//�����ڲ����ɫ����ˢ��ʽ��
		painter.drawRect(0, height() - 3, width()*m_color.alphaF() - 1, 2);
	}
	else
	{
		// ���Ʊ�Ե�Թ�Ч��
		painter.setBrush(QBrush(QColor(m_color.red(), m_color.green(), m_color.blue(), 32), Qt::SolidPattern));//�����ڲ����ɫ����ˢ��ʽ��
		painter.drawRoundRect(0, 0, width() - 1, height() - 1, 80, 80);
		painter.setBrush(QBrush(QColor(m_color.red(), m_color.green(), m_color.blue(), 64), Qt::SolidPattern));//�����ڲ����ɫ����ˢ��ʽ��
		painter.drawRoundRect(1, 1, width() - 3, height() - 3, 80, 80);

		//����RGB��ɫ
		painter.setBrush(QBrush(QColor(m_color.red(), m_color.green(), m_color.blue()), Qt::SolidPattern));//�����ڲ����ɫ����ˢ��ʽ��
		painter.drawRoundRect(2, 2, width() - 5, height() - 5, 90, 90);
	}
}

void CGMColorLabel::mousePressEvent(QMouseEvent *ev)
{
	//QColor color;
	//if (m_bAlpha)
	//{
	//	color = QColorDialog::getColor(m_color, this, QString::fromLocal8Bit(" ��ȡ��ɫ "), QColorDialog::ShowAlphaChannel);
	//}
	//else
	//{
	//	color = QColorDialog::getColor(m_color, this, QString::fromLocal8Bit(" ��ȡ��ɫ "));
	//}

	//if (color.isValid())
	//{
	//	m_color = color;
	//}
	update();
	emit clicked();
}
