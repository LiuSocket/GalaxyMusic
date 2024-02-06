#include "GMMainWindow.h"
#include "GMVolumeWidget.h"
#include "GMListWidget.h"
#include "UI/GMUIManager.h"
#include "../Engine/GMEngine.h"
#include <QKeyEvent>
#include <QScreen>

using namespace GM;

CGMMainWindow::CGMMainWindow(QWidget *parent)
	: QMainWindow(parent),
	m_pVolumeWidget(nullptr), m_pListWidget(nullptr), m_pSceneWidget(nullptr),
	m_bInit(false), m_bFull(false), m_bPressed(false), m_bShowVolume(false),
	m_vPos(QPoint(0,0)), m_iAudioDuration(5000), m_strName(QString())
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_Mapped);

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
	connect(ui.listBtn, SIGNAL(clicked()), this, SLOT(_slotListVisible()));
	connect(ui.fullScreenBtn, SIGNAL(clicked()), this, SLOT(_slotFullScreen()));

	m_pVolumeWidget = new CGMVolumeWidget(this);
	m_pVolumeWidget->raise();
	m_pVolumeWidget->hide();
	connect(m_pVolumeWidget, SIGNAL(_signalSetVolume(int)), this, SLOT(_slotSetVolume(int)));

	m_pListWidget = new CGMListWidget(this);
	m_pListWidget->Init();
	m_pListWidget->raise();
	m_pListWidget->hide();

	// 加载QSS
	QFile qssFile(":/Resources/MainWindow.qss");
	if (qssFile.open(QFile::ReadOnly))
	{
		QString style = QLatin1String(qssFile.readAll());
		setStyleSheet(style);
		m_pVolumeWidget->setStyleSheet(style);
		m_pListWidget->setStyleSheet(style);
		qssFile.close();
	}
}

CGMMainWindow::~CGMMainWindow()
{
	GM_UI_MANAGER_PTR->Release();
}

/** @brief 初始化 */
bool CGMMainWindow::Init()
{
	if (m_bInit)
		return true;

	m_pSceneWidget = GM_ENGINE.CreateViewWidget(this);
	ui.centralVLayout->insertWidget(2,(QWidget*)m_pSceneWidget);

	connect(m_pSceneWidget, SIGNAL(_signalEnter3D()), this, SLOT(_slotEnter3D()));

	QImage* pAudioImg = new QImage;
	pAudioImg->load(":/Resources/default_Image.png");
	ui.audioImgLab->setPixmap(QPixmap::fromImage(*pAudioImg));

	m_bInit = true;

	return m_bInit;
}

void CGMMainWindow::Update()
{
	// 更新音量
	if (m_bShowVolume)
	{
		m_pVolumeWidget->SetVolume(GM_ENGINE.GetVolume() * 100);
	}
}

void CGMMainWindow::SetFullScreen(const bool bFull)
{
	if (m_bFull != bFull)
	{
		m_bFull = bFull;

		// 全屏切换
		if (m_bFull)
		{
			QList<QScreen*> mScreen = qApp->screens();
			setGeometry(0, 0, mScreen[0]->geometry().width(), mScreen[0]->geometry().height());
			show();

			ui.titleWidget->hide();
			ui.titleEdgeLab->hide();
			ui.toolWidget->hide();
			ui.toolEdgeLab->hide();

			ui.listBtn->setChecked(false);
			m_pListWidget->hide();
		}
		else
		{
			if (ui.maxBtn->isChecked())
			{
				setGeometry(320, 180, 1280, 720);
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
	const std::wstring wstrAudioName = GM_ENGINE.GetAudioName();
	QString strFileName = QString::fromStdWString(wstrAudioName);
	if ("" == strFileName) return;
	if (m_strName != strFileName)
	{
		m_strName = strFileName;
		strFileName.chop(strFileName.size() - strFileName.lastIndexOf("."));
		QStringList strList = strFileName.split(" - ");
		// 如果切成的段数大于1，第一段就是作者名，第二段就是歌曲名称
		if (1 < strList.size())
		{
			ui.audioNameTextScroller->setText(strList[1]);
			ui.audioInfoTextScroller->setText(strList[0]);
		}
		else
		{
			ui.audioNameTextScroller->setText(strFileName);
			ui.audioInfoTextScroller->setText("Unknown");
		}

		//更新播放列表界面
		SGMAudioCoord sAudioCoord = GM_ENGINE.GetAudioCoord(wstrAudioName);
		if (1 < strList.size())
		{
			m_pListWidget->AddAudio(strList[1], strList[0], sAudioCoord.BPM, sAudioCoord.angle);
		}
		else
		{
			m_pListWidget->AddAudio(strFileName, "", sAudioCoord.BPM, sAudioCoord.angle);
		}
		qApp->processEvents();
		m_pListWidget->EnsureLastAudioVisible();

		m_iAudioDuration = GM_ENGINE.GetAudioDuration();

		// 将播放/暂停按钮设置成播放状态
		ui.playBtn->setChecked(true);

		// 计算并显示当前音频总时长
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
	// 获取当前音频播放位置，单位：ms
	int iCurrentTime = GM_ENGINE.GetAudioCurrentTime();
	float fTimeRatio = 100 * float(iCurrentTime) / float(m_iAudioDuration);
	// 避免循环修改时间
	int iTimeLast = ui.timeSlider->value();
	if(abs(fTimeRatio - iTimeLast) > 0.5f)
		ui.timeSlider->setValue(fTimeRatio);

	// 计算并显示已播放时间
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

void CGMMainWindow::SetVolumeVisible(const bool bVisible)
{
	if (bVisible)
	{
		int iX = pos().x() + ui.volumeBtn->pos().x();
		int iY = pos().y() + ui.toolEdgeLab->pos().y() - m_pVolumeWidget->size().height() + 20;
		m_pVolumeWidget->move(iX, iY);
	}

	m_pVolumeWidget->setVisible(bVisible);
	m_bShowVolume = bVisible;
}

void CGMMainWindow::_slotLast()
{
	GM_ENGINE.Last();
	ui.playBtn->setChecked(true);
}

void CGMMainWindow::_slotPlayOrPause()
{
	if (ui.playBtn->isChecked())
	{
		GM_ENGINE.Play();
	}
	else
	{
		GM_ENGINE.Pause();
	}
}

void CGMMainWindow::_slotNext()
{
	GM_ENGINE.Next();
	ui.playBtn->setChecked(true);
}

void CGMMainWindow::_slotMinimum()
{
	showMinimized();
}

void CGMMainWindow::_slotMaximum()
{
	// 进入这个槽函数时，按钮的状态已经改变
	if (ui.maxBtn->isChecked())
	{
		showNormal();
	}
	else
	{
		showMaximized();
	}

	if (m_pListWidget->isVisible())
	{
		_SetPlayingListGeometry();
	}
}

void CGMMainWindow::_slotClose()
{
	GM_ENGINE.Save();
	exit(0);
}

void CGMMainWindow::_slotSetAudioTime(int iTimeRatio)
{
	// 当前音频播放的时刻
	int iAudioCurrentTime = float(iTimeRatio)*0.01*m_iAudioDuration;
	int iAudioCurrentPreciseTime = GM_ENGINE.GetAudioCurrentTime();
	if (std::abs(iAudioCurrentPreciseTime - iAudioCurrentTime) > 1000)
	{
		GM_ENGINE.SetAudioCurrentTime(iAudioCurrentTime);
	}
}

void CGMMainWindow::_slotSetMute()
{
	if (ui.volumeBtn->isChecked())
	{
		GM_ENGINE.SetVolume(0.0f);
	}
	else
	{
		GM_ENGINE.SetVolume(m_pVolumeWidget->GetVolume()*0.01f);
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

	GM_ENGINE.SetVolume(iVolume*0.01f);
}

void CGMMainWindow::_slotListVisible()
{
	if (ui.listBtn->isChecked())
	{
		_SetPlayingListGeometry();
		m_pListWidget->show();
		m_pListWidget->EnsureLastAudioVisible();
	}
	else
	{
		m_pListWidget->hide();
	}
}

void CGMMainWindow::_slotFullScreen()
{
	SetFullScreen(true);
}

void CGMMainWindow::_slotEnter3D()
{
	m_pVolumeWidget->hide();
}

void CGMMainWindow::changeEvent(QEvent* event)
{
	if (GM_ENGINE.GetRendering() && isMinimized())
	{
		GM_ENGINE.SetRendering(false);
	}
	else if (!GM_ENGINE.GetRendering() && !isMinimized())
	{
		GM_ENGINE.SetRendering(true);
		setAttribute(Qt::WA_Mapped);
	}
	else{}

	QWidget::changeEvent(event);
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
		if (ui.maxBtn->isChecked()) // 窗口化状态
		{
			ui.maxBtn->setChecked(false);
			// 切换到最大化
			showMaximized();
		}
		else // 最大化状态 
		{
			ui.maxBtn->setChecked(true);
			// 切换到窗口
			showNormal();
		}

		if (m_pListWidget->isVisible())
		{
			_SetPlayingListGeometry();
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
	// 设置音量控件显隐
	int iVolumeMinX = ui.volumeBtn->pos().x();
	int iVolumeMaxX = iVolumeMinX + ui.volumeBtn->width();
	int iVolumeMinY = ui.toolEdgeLab->pos().y();
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
		int iX = pos().x() + ui.volumeBtn->pos().x();
		int iY = pos().y() + ui.toolEdgeLab->pos().y() - m_pVolumeWidget->size().height() + 20;

		m_pVolumeWidget->SetVolume(GM_ENGINE.GetVolume() * 100);
		m_pVolumeWidget->move(iX, iY);
		m_pVolumeWidget->show();
	}

	// 鼠标拖动标题栏以移动窗口
	if (m_bPressed && (event->pos().y() < ui.titleWidget->height()))
	{
		if (ui.maxBtn->isChecked())// 窗口化状态
		{
			QPoint movePoint = event->globalPos() - m_vPos;
			m_vPos = event->globalPos();
			move(x() + movePoint.x(), y() + movePoint.y());

			m_pListWidget->move(m_pListWidget->pos().x() + movePoint.x(), m_pListWidget->pos().y() + movePoint.y());
		}
		else // 最大化状态
		{
			// 切换回窗口化状态
			ui.maxBtn->setChecked(true);
			showNormal();

			m_vPos = event->globalPos();
			move(m_vPos.x() - 640, m_vPos.y() - 5);

			if (m_pListWidget->isVisible())
			{
				_SetPlayingListGeometry();
			}
		}
	}

	QWidget::mouseMoveEvent(event);
}

void CGMMainWindow::keyPressEvent(QKeyEvent* event)
{
	if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_S))
	{
		GM_ENGINE.Save();
		// 记录太阳系此刻的信息，保证重启时太阳系行星的同步
		GM_ENGINE.SaveSolarData();
	}

	switch (event->key())
	{
	case Qt::Key_F2:
	{
		GM_ENGINE.SetPlayMode(EGMA_MOD_CIRCLE);
		GM_ENGINE.Next();
	}
	break;
	case Qt::Key_F3:
	{
		GM_ENGINE.SetPlayMode(EGMA_MOD_RANDOM);
		GM_ENGINE.Next();
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

	QWidget::keyPressEvent(event);
}

void CGMMainWindow::keyReleaseEvent(QKeyEvent* event)
{
	QWidget::keyReleaseEvent(event);
}

void CGMMainWindow::_Million2MinutesSeconds(const int ms, int & minutes, int & seconds)
{
	int iAllSeconds = ms / 1000;
	minutes = max(0, min(59, iAllSeconds / 60));
	seconds = max(0, min(59, iAllSeconds % 60));
}

void CGMMainWindow::_SetPlayingListGeometry()
{
	int iX = pos().x() + ui.titleEdgeLab->pos().x() + ui.titleEdgeLab->width() - m_pListWidget->width();
	int iY = pos().y() + ui.titleEdgeLab->pos().y() + ui.titleEdgeLab->height();
	int iWidth = m_pListWidget->width();
	int iHeight = ui.toolEdgeLab->y() - ui.titleEdgeLab->pos().y() - ui.titleEdgeLab->height();

	m_pListWidget->setGeometry(iX, iY, iWidth, iHeight);
}
