#include "GMVolumeWidget.h"

CGMVolumeWidget::CGMVolumeWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
	setAttribute(Qt::WA_TranslucentBackground);

	connect(ui.volumeSlider, SIGNAL(valueChanged(int)), this, SIGNAL(_signalSetVolume(int)));
}

CGMVolumeWidget::~CGMVolumeWidget()
{
}

bool CGMVolumeWidget::SetVolume(const int iVolume)
{
	if(iVolume == ui.volumeSlider->value()) return true;

	if (iVolume >= 0 && iVolume <= 100)
	{
		ui.volumeSlider->setValue(iVolume);
		return true;
	}
	else
	{
		return false;
	}
}

int CGMVolumeWidget::GetVolume()
{
	return ui.volumeSlider->value();
}

void CGMVolumeWidget::leaveEvent(QEvent* event)
{
	hide();
}