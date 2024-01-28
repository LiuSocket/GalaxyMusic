#include "GMListWidget.h"
#include "../Engine/GMEngine.h"

/*************************************************************************
 Macro Defines
*************************************************************************/
#define GM_LIST_MAX					(50)

using namespace GM;

CGMListWidget::CGMListWidget(QWidget *parent)
	: QWidget(parent), m_bInit(false)
{
	ui.setupUi(this);
	setLayout(ui.verticalLayout);
	ui.listScrollAreaWidgetContents->setLayout(ui.listVLayout);

	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
	setAttribute(Qt::WA_TranslucentBackground);
}

CGMListWidget::~CGMListWidget()
{
}

bool CGMListWidget::Init()
{
	if (m_bInit)
		return true;

	const std::vector<std::wstring> vStrPlayingOrder = GM_ENGINE.GetPlayingOrder();

	for (int i = (vStrPlayingOrder.size() - 1); i >= 0; i--)
	{
		QString strFileName = QString::fromStdWString(vStrPlayingOrder.at(i));
		if (QString::fromStdWString(L"") != strFileName)
		{
			SGMAudioCoord sAudioCoord = GM_ENGINE_PTR->GetAudioCoord(vStrPlayingOrder.at(i));
			CGMAudioWidget* pAudioWidget = new CGMAudioWidget(this);

			strFileName.chop(strFileName.size() - strFileName.lastIndexOf("."));
			QStringList strList = strFileName.split(" - ");
			// 如果切成的段数大于1，第一段就是作者名，第二段就是音频名称
			if (1 < strList.size())
			{
				pAudioWidget->SetTitle(strList[1]);
				pAudioWidget->SetArtist(strList[0]);
			}
			else
			{
				pAudioWidget->SetTitle(strFileName);
				pAudioWidget->SetArtist("Unknown");
			}
			pAudioWidget->SetBPM(sAudioCoord.BPM);

			SGMVector4f vColor = GM_ENGINE_PTR->Angle2Color(sAudioCoord.angle);
			pAudioWidget->SetColor(QColor(vColor.r * 255, vColor.g * 255, vColor.b * 255, vColor.a * 255));

			ui.listVLayout->insertWidget(0, (QWidget*)pAudioWidget);

			m_vAudioWidgetVector.emplace(m_vAudioWidgetVector.begin(), pAudioWidget);
		}
	}
	int iHeight = m_vAudioWidgetVector.at(0)->height();
	ui.listScrollAreaWidgetContents->setMinimumHeight(vStrPlayingOrder.size() * iHeight);

	m_bInit = true;
	return m_bInit;
}

bool CGMListWidget::AddAudio(
	const QString& strAudioTitle,
	const QString& strArtistName,
	const double fBPM,
	const double fAngle)
{
	if ("" == strAudioTitle) return false;

	if (GM_LIST_MAX <= m_vAudioWidgetVector.size())
	{
		// 删除第一个音频数据
		QLayoutItem *child = ui.listVLayout->takeAt(0);
		//setParent为NULL，防止删除之后界面不消失
		if (child != nullptr && child->widget())
		{
			child->widget()->setParent(NULL);
			delete child;
		}
		m_vAudioWidgetVector.erase(m_vAudioWidgetVector.begin());
	}

	CGMAudioWidget* pAudioWidget = new CGMAudioWidget(this);
	pAudioWidget->SetTitle(strAudioTitle);
	pAudioWidget->SetArtist(strArtistName);
	pAudioWidget->SetBPM(fBPM);

	SGMVector4f vColor = GM_ENGINE_PTR->Angle2Color(fAngle);
	pAudioWidget->SetColor(QColor(vColor.r * 255, vColor.g * 255, vColor.b * 255, vColor.a * 255));

	ui.listVLayout->addWidget((QWidget*)pAudioWidget);
	m_vAudioWidgetVector.push_back(pAudioWidget);

	int iHeight = m_vAudioWidgetVector.at(0)->height();
	ui.listScrollAreaWidgetContents->setMinimumHeight(m_vAudioWidgetVector.size() * iHeight);

	return true;
}

void CGMListWidget::EnsureLastAudioVisible()
{
	ui.listScrollArea->ensureWidgetVisible(m_vAudioWidgetVector.back());
}
