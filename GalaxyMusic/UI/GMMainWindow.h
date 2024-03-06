#pragma once

#include <QMainWindow>
#include "ui_GMMainWindow.h"

/*************************************************************************
Class
*************************************************************************/
class CGMVolumeWidget;
class CGMListWidget;
class CGMWanderingEarthWidget;
class CGMViewWidget;

class CGMMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	CGMMainWindow(QWidget *parent = Q_NULLPTR);
	~CGMMainWindow();

	/** @brief 初始化 */
	bool Init();
	/** @brief 更新 */
	void Update();

	/**
	* @brief 界面全屏切换
	* @param bFull true为全屏，false 为非全屏
	*/
	void SetFullScreen(const bool bFull);
	/**
	* @brief 获取界面全屏状态
	* @return bool		是否处于全屏状态
	*/
	bool GetFullScreen();

	/**
	* @brief 更新音频的所有信息
	*/
	void UpdateAudioInfo();

	/**
	* @brief 设置是否显示实时变化的音量
	* @param bVisible 是否显示实时变化的音量
	*/
	void SetVolumeVisible(const bool bVisible);

public slots:

	/**
	* @brief 上一首
	*/
	void _slotLast();

	/**
	* @brief 播放/暂停
	*/
	void _slotPlayOrPause();

	/**
	* @brief 下一首
	*/
	void _slotNext();

	/**
	* @brief 最小化/还原
	*/
	void _slotMinimum();
	/**
	* @brief 最大化/还原
	*/
	void _slotMaximum();
	/**
	* @brief 关闭
	*/
	void _slotClose();

	/** 
	* @brief 设置音频当前的时间
	* @param iTimeRatio: 当前时间与总时间的比值,[0,100]
	*/
	void _slotSetAudioTime(int iTimeRatio);

	/**
	* @brief 正常/静音
	*/
	void _slotSetMute();
	/**
	* @brief 设置音量
	* @param iVolume	音量，[0, 100]
	*/
	void _slotSetVolume(int iVolume);

	/**
	* @brief 设置播放列表的显示、隐藏
	*/
	void _slotListVisible();

	/**
	* @brief 启用全屏
	*/
	void _slotFullScreen();

	/**
	* @brief 进入三维界面
	*/
	void _slotEnter3D();

protected:
	void changeEvent(QEvent* event);
	void resizeEvent(QResizeEvent* event);
	void closeEvent(QCloseEvent* event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent* event);
	/** @brief 界面上的键盘事件 */
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);

private:
	/**
	* _Million2MinutesSeconds
	* @brief 毫秒数 转 分钟:秒数
	* @param ms: 输入毫秒数
	* @param minutes: 输出分钟数
	* @param seconds: 输出秒数
	* @return void
	*/
	void _Million2MinutesSeconds(const int ms, int& minutes, int& seconds);

	/**
	* _SetPlayingListGeometry
	* @brief 设置播放列表的位置、尺寸
	* @param void
	* @return void
	*/
	void _SetPlayingListGeometry();

private:
	Ui::GMMainWindow					ui;
	CGMVolumeWidget*					m_pVolumeWidget;
	CGMListWidget*						m_pListWidget;
	CGMWanderingEarthWidget*			m_pWanderingEarthWidget;
	CGMViewWidget*						m_pSceneWidget;

	bool								m_bInit;
	bool								m_bFull;				//!< 是否全屏
	bool								m_bPressed;				//!< 是否按下鼠标
	bool								m_bShowVolume;			//!< 是否显示实时变化的音量
	QPoint								m_vPos;					//!< 窗口的位置
	int									m_iAudioDuration;		//!< 音频总时长，单位：ms
	QString								m_strName;				//!< 音频文件名称，包含后缀名
};