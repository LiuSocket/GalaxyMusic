#include "GMWanderingEarthWidget.h"
#include "../Engine/GMEngine.h"

using namespace GM;

CGMWanderingEarthWidget::CGMWanderingEarthWidget(QWidget *parent)
	: QWidget(parent), m_bInit(false)
{
	ui.setupUi(this);

	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
	setAttribute(Qt::WA_TranslucentBackground);
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
