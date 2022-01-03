#include "GMMainWindow.h"
#include "GMVolumeWidget.h"
#include "UI/GMUIManager.h"
#include "../Engine/GMEngine.h"
#include <QKeyEvent>
#include <QtPlatformHeaders\QWindowsWindowFunctions>

using namespace GM;

CGMMainWindow::CGMMainWindow(QWidget *parent)
	: QMainWindow(parent),
	m_pVolumeWidget(nullptr),
	m_bInit(false), m_bFull(false), m_bPressed(false), m_vPos(QPoint(0,0)),
	m_iAudioDuration(5000),m_strName(QString())
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);

	ui.centralWidget->setLayout(ui.centralVLayout);
	ui.titleWidget->setLayout(ui.titleHLayout);
	ui.toolWidget->setLayout(ui.toolGLayout);

	connect(ui.lastBtn, SIGNAL(clicked()), this, SLOT(_slotLast()));
	connect(ui.playBtn, SIGNAL(clicked()), this, SLOT(_slotPlayOrPause()));
	connect(ui.nextBtn, SIGNAL(clicked()), this, SLOT(_slotNext()));

	connect(ui.minBtn, SIGNAL(clicked()), this, SLOT(_slotMinimum()));
	connect(ui.maxBtn, SIGNAL(clicked()), this, SLOT(_slotMaximum()));
	connect(ui.closeBtn, SIGNAL(clicked()), this, SLOT(_slotClose()));

	connect(ui.timeSlider, SIGNAL(valueChanged(int)), this, SLOT(_slotSetAudioTime(int)));
	connect(ui.volumeBtn, SIGNAL(clicked()), this, SLOT(_slotSetMute()));
	connect(ui.fullScreenBtn, SIGNAL(clicked()), this, SLOT(_slotFullScreen()));

	m_pVolumeWidget = new CGMVolumeWidget(this);
	m_pVolumeWidget->raise();
	m_pVolumeWidget->hide();
	connect(m_pVolumeWidget, SIGNAL(_signalSetVolume(int)), this, SLOT(_slotSetVolume(int)));

	// ����QSS
	QFile qssFile(":/Resources/MainWindow.qss");
	if (qssFile.open(QFile::ReadOnly))
	{
		QString style = QLatin1String(qssFile.readAll());
		setStyleSheet(style);
		m_pVolumeWidget->setStyleSheet(style);
		qssFile.close();
	}
}

CGMMainWindow::~CGMMainWindow()
{
	GM_UI_MANAGER_PTR->Release();
}

/** @brief ��ʼ�� */
bool CGMMainWindow::Init()
{
	if (m_bInit)
		return true;

	osg::ref_ptr<CGMViewWidget> pSceneWidget = GM_ENGINE_PTR->CreateViewWidget(this);
	ui.centralVLayout->insertWidget(2,(QWidget*)pSceneWidget.get());

	QImage* pAudioImg = new QImage;
	pAudioImg->load(":/Resources/default_Image.png");
	ui.audioImgLab->setPixmap(QPixmap::fromImage(*pAudioImg));

	m_bInit = true;

	return m_bInit;
}

void CGMMainWindow::SetFullScreen(const bool bFull)
{
	if (m_bFull != bFull)
	{
		m_bFull = bFull;

		// ȫ���л�
		if (m_bFull)
		{
			// ����ȫ������ǰ�����ȼ�������
			QWindowsWindowFunctions::setHasBorderInFullScreen(windowHandle(), true);
			showFullScreen();
			ui.titleWidget->hide();
			ui.titleEdgeLab->hide();
			ui.toolWidget->hide();
			ui.toolEdgeLab->hide();
		}
		else
		{
			if (ui.maxBtn->isChecked())
			{
				showNormal();
			}
			else
			{
				showMaximized();
			}

			ui.titleWidget->show();
			ui.titleEdgeLab->show();
			ui.toolWidget->show();
			ui.toolEdgeLab->show();
		}
	}
}

bool CGMMainWindow::GetFullScreen()
{
	return m_bFull;
}

void CGMMainWindow::UpdateAudioInfo()
{
	QString strFileName = QString::fromStdWString(GM_ENGINE_PTR->GetAudioName());
	if ("" == strFileName) return;
	if (m_strName != strFileName)
	{
		m_strName = strFileName;
		strFileName.chop(strFileName.size() - strFileName.lastIndexOf("."));
		QStringList strList = strFileName.split(" - ");
		// ����гɵĶ�������1����һ�ξ������������ڶ��ξ��Ǹ�������
		if (1 < strList.size())
		{
			ui.audioNameLab->setText(strList[1]);
			ui.audioInfoLab->setText(strList[0]);
		}
		else
		{
			ui.audioNameLab->setText(strFileName);
			ui.audioInfoLab->setText("");
		}

		m_iAudioDuration = GM_ENGINE_PTR->GetAudioDuration();

		// ������/��ͣ��ť���óɲ���״̬
		ui.playBtn->setChecked(true);

		// ���㲢��ʾ��ǰ��Ƶ��ʱ��
		int iMinutesAll = 0;
		int iSecondsAll = 0;
		_Million2MinutesSeconds(m_iAudioDuration, iMinutesAll, iSecondsAll);
		QString strAll = QString::number(iMinutesAll);
		if (iMinutesAll < 10) strAll = "0" + strAll;
		strAll += ":";
		if (iSecondsAll < 10) strAll += "0";
		strAll += QString::number(iSecondsAll);
		ui.timeAllLab->setText(strAll);
	}
	// ��ȡ��ǰ��Ƶ����λ�ã���λ��ms
	int iCurrentTime = GM_ENGINE_PTR->GetAudioCurrentTime();
	float fTimeRatio = 100 * float(iCurrentTime) / float(m_iAudioDuration);
	ui.timeSlider->setValue(int(fTimeRatio));

	// ���㲢��ʾ�Ѳ���ʱ��
	int iMinutesPassed = 0;
	int iSecondsPassed = 0;
	_Million2MinutesSeconds(iCurrentTime, iMinutesPassed, iSecondsPassed);
	QString strPassed = QString::number(iMinutesPassed);
	if (iMinutesPassed < 10) strPassed = "0" + strPassed;
	strPassed += ":";
	if (iSecondsPassed < 10) strPassed += "0";
	strPassed += QString::number(iSecondsPassed);
	ui.timePassedLab->setText(strPassed);
}

void CGMMainWindow::_slotLast()
{
	GM_ENGINE_PTR->Last();
	ui.playBtn->setChecked(true);
}

void CGMMainWindow::_slotPlayOrPause()
{
	if (ui.playBtn->isChecked())
	{
		GM_ENGINE_PTR->Play();
	}
	else
	{
		GM_ENGINE_PTR->Pause();
	}
}

void CGMMainWindow::_slotNext()
{
	GM_ENGINE_PTR->Next();
	ui.playBtn->setChecked(true);
}

void CGMMainWindow::_slotMinimum()
{
	showMinimized();
}

void CGMMainWindow::_slotMaximum()
{
	// ��������ۺ���ʱ����ť��״̬�Ѿ��ı�
	if (ui.maxBtn->isChecked())
	{
		showNormal();
	}
	else
	{
		showMaximized();
	}
}

void CGMMainWindow::_slotClose()
{
	exit(0);
}

void CGMMainWindow::_slotSetAudioTime(int iTimeRatio)
{
	// ��ǰ��Ƶ���ŵ�ʱ��
	int iAudioCurrentTime = float(iTimeRatio)*0.01*m_iAudioDuration;
	int iAudioCurrentPreciseTime = GM_ENGINE_PTR->GetAudioCurrentTime();
	if (std::abs(iAudioCurrentPreciseTime - iAudioCurrentTime) > 1000)
	{
		GM_ENGINE_PTR->SetAudioCurrentTime(iAudioCurrentTime);
	}
}

void CGMMainWindow::_slotSetMute()
{
	if (ui.volumeBtn->isChecked())
	{
		GM_ENGINE_PTR->SetVolume(0.0f);
	}
	else
	{
		GM_ENGINE_PTR->SetVolume(m_pVolumeWidget->GetVolume()*0.01f);
	}
}

void CGMMainWindow::_slotSetVolume(int iVolume)
{
	if (0 == iVolume)
	{
		if(!ui.volumeBtn->isChecked())
			ui.volumeBtn->setChecked(true);
	}
	else 
	{
		if (ui.volumeBtn->isChecked())
			ui.volumeBtn->setChecked(false);
	}

	GM_ENGINE_PTR->SetVolume(iVolume*0.01f);
}

void CGMMainWindow::_slotFullScreen()
{
	SetFullScreen(true);
}

void CGMMainWindow::resizeEvent(QResizeEvent* event)
{
	m_pVolumeWidget->hide();

	GM_UI_MANAGER_PTR->Resize();
}

void CGMMainWindow::closeEvent(QCloseEvent* event)
{
	GM_UI_MANAGER_PTR->Release();
}

void CGMMainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (event->pos().y() < ui.titleWidget->height())
	{
		if (ui.maxBtn->isChecked()) // ���ڻ�״̬
		{
			ui.maxBtn->setChecked(false);
			// �л������
			showMaximized();
		}
		else // ���״̬ 
		{
			ui.maxBtn->setChecked(true);
			// �л�������
			showNormal();
		}
	}
	QWidget::mouseDoubleClickEvent(event);
}

void CGMMainWindow::mousePressEvent(QMouseEvent * event)
{
	m_bPressed = true;
	m_vPos = event->globalPos();
	QWidget::mousePressEvent(event);
}

void CGMMainWindow::mouseReleaseEvent(QMouseEvent * event)
{
	m_bPressed = false;
	QWidget::mouseReleaseEvent(event);
}

void CGMMainWindow::mouseMoveEvent(QMouseEvent* event)
{
	int iVolumeMinX = ui.volumeBtn->pos().rx();
	int iVolumeMaxX = iVolumeMinX + ui.volumeBtn->width();
	int iVolumeMinY = ui.toolEdgeLab->pos().ry();
	int iVolumeMaxY = iVolumeMinY + ui.volumeBtn->height();

	if (event->x() < iVolumeMinX || event->x() > iVolumeMaxX ||
		event->y() < iVolumeMinY || event->y() > iVolumeMaxY)
	{
		if (m_pVolumeWidget->isVisible())
		{
			m_pVolumeWidget->hide();
		}
	}
	else if (!m_pVolumeWidget->isVisible())
	{
		int iX = pos().rx() + ui.volumeBtn->pos().rx();
		int iY = pos().ry() + ui.toolEdgeLab->pos().ry() - m_pVolumeWidget->size().height() + 20;

		m_pVolumeWidget->SetVolume(GM_ENGINE_PTR->GetVolume() * 100);
		m_pVolumeWidget->move(iX, iY);
		m_pVolumeWidget->show();
	}

	if (m_bPressed && (event->pos().y() < ui.titleWidget->height()))
	{
		if (ui.maxBtn->isChecked())// ���ڻ�״̬
		{
			QPoint movePoint = event->globalPos() - m_vPos;
			m_vPos = event->globalPos();
			move(x() + movePoint.x(), y() + movePoint.y());
		}
		else // ���״̬
		{
			// �л��ش��ڻ�״̬
			ui.maxBtn->setChecked(true);
			showNormal();

			m_vPos = event->globalPos();
			move(m_vPos.x() - 640, m_vPos.y() - 5);
		}
	}

	QMainWindow::mouseMoveEvent(event);
}

void CGMMainWindow::keyPressEvent(QKeyEvent* event)
{
	if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_S))
	{
		GM_ENGINE_PTR->Save();
	}

	switch (event->key())
	{
	case Qt::Key_F2:
	{
		GM_ENGINE_PTR->SetPlayMode(EGMA_MOD_CIRCLE);
		GM_ENGINE_PTR->Next();
	}
	break;
	case Qt::Key_F3:
	{
		GM_ENGINE_PTR->SetPlayMode(EGMA_MOD_RANDOM);
		GM_ENGINE_PTR->Next();
	}
	break;
	case Qt::Key_F11:
	{
		GM_UI_MANAGER_PTR->SetFullScreen(!GM_UI_MANAGER_PTR->GetFullScreen());
	}
	break;
	default:
		break;
	}
}

void CGMMainWindow::keyReleaseEvent(QKeyEvent* event)
{
}

void CGMMainWindow::_Million2MinutesSeconds(const int ms, int & minutes, int & seconds)
{
	int iAllSeconds = ms / 1000;
	minutes = max(0, min(59, iAllSeconds / 60));
	seconds = max(0, min(59, iAllSeconds % 60));
}
