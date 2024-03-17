#include "GMWanderingEarthWidget.h"
#include "../Engine/GMEngine.h"

using namespace GM;

CGMWanderingEarthWidget::CGMWanderingEarthWidget(QWidget *parent)
	: QWidget(parent), m_bInit(false)
{
	ui.setupUi(this);

	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
	setAttribute(Qt::WA_TranslucentBackground);

	connect(ui.WEProgressSlider, SIGNAL(valueChanged(int)), this, SLOT(_slotSetProgress(int)));
}

CGMWanderingEarthWidget::~CGMWanderingEarthWidget()
{}

bool CGMWanderingEarthWidget::Init()
{
	if (m_bInit)
		return true;

	m_bInit = true;
	return m_bInit;
}

void CGMWanderingEarthWidget::Update()
{
	// 避免循环修改进图条
	float fProgress = GM_ENGINE.GetWanderingEarthProgress();
	int iProgressLast = ui.WEProgressSlider->value();
	if (std::abs(fProgress*1000 - iProgressLast) > 1)
		ui.WEProgressSlider->setValue(fProgress * 1000);
}

void CGMWanderingEarthWidget::_slotSetProgress(int iProgress)
{
	float fProgress = GM_ENGINE.GetWanderingEarthProgress();
	if (std::abs(fProgress - 0.001f*iProgress) > 0.002f)
	{
		GM_ENGINE.SetWanderingEarthProgress(0.001f * iProgress);
	}
}
