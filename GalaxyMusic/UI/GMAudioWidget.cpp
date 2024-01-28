#include "GMAudioWidget.h"

CGMAudioWidget::CGMAudioWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

CGMAudioWidget::~CGMAudioWidget()
{
}

bool CGMAudioWidget::SetTitle(const QString & strTitle)
{
	ui.audioTitleLab->setText(strTitle);
	ui.audioTitleLab->setToolTip(strTitle);
	return true;
}

bool CGMAudioWidget::SetArtist(const QString & strArtist)
{
	ui.artistNameLab->setText(strArtist);
	ui.artistNameLab->setToolTip(strArtist);
	return true;
}

bool CGMAudioWidget::SetBPM(const double fBPM)
{
	ui.BPMLab->setText(QString::number(fBPM,'f',1));
	return true;
}

bool CGMAudioWidget::SetColor(const QColor color)
{
	ui.emotionLab->SetColor(color);
	return true;
}

QString CGMAudioWidget::GetTitle()
{
	return ui.audioTitleLab->text();
}

QString CGMAudioWidget::GetArtist()
{
	return ui.artistNameLab->text();
}