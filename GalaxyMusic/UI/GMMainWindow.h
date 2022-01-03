#pragma once

#include <QMainWindow>
#include "ui_GMMainWindow.h"

/*************************************************************************
Class
*************************************************************************/
class CGMVolumeWidget;

class CGMMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	CGMMainWindow(QWidget *parent = Q_NULLPTR);
	~CGMMainWindow();

	/** @brief 初始化 */
	bool Init();

	/**
	* SetFullScreen
	* @brief 界面全屏切换
	* @author LiuTao
	* @since 2021.09.05
	* @param bFull true为全屏，false 为非全屏
	* @return void
	*/
	void SetFullScreen(const bool bFull);
	/**
	* GetFullScreen
	* @brief 获取界面全屏状态
	* @author LiuTao
	* @since 2021.10.23
	* @return bool		是否处于全屏状态
	*/
	bool GetFullScreen();

	/**
	* UpdateAudioInfo
	* 更新音频的所有信息
	* @author LiuTao
	* @since 2021.09.05
	* @return void
	*/
	void UpdateAudioInfo();

public slots:

	/**
	* _slotLast
	* @brief 上一首
	* @param void
	* @return void
	*/
	void _slotLast();

	/**
	* _slotPlayOrPause
	* @brief 播放/暂停
	* @param void
	* @return void
	*/
	void _slotPlayOrPause();

	/**
	* _slotNext
	* @brief 下一首
	* @param void
	* @return void
	*/
	void _slotNext();

	/**
	* _slotMinimum
	* @brief 最小化/还原
	* @param void
	* @return void
	*/
	void _slotMinimum();
	/**
	* _slotMaximum
	* @brief 最大化/还原
	* @param void
	* @return void
	*/
	void _slotMaximum();
	/**
	* _slotClose
	* @brief 关闭
	* @param void
	* @return void
	*/
	void _slotClose();

	/** 
	* _slotSetAudioTime
	* @brief 设置音频当前的时间
	* @param iTimeRatio: 当前时间与总时间的比值
	* @return void
	*/
	void _slotSetAudioTime(int iTimeRatio);

	/**
	* _slotSetMute
	* @brief 正常/静音
	* @param void
	* @return void
	*/
	void _slotSetMute();
	/**
	* _slotSetVolume
	* @brief 设置音量
	* @param iVolume	音量，[0, 100]
	* @return void
	*/
	void _slotSetVolume(int iVolume);

	/**
	* _slotSetFullScreen
	* @brief 启用全屏
	* @param void
	* @return void
	*/
	void _slotFullScreen();

protected:
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

private:
	Ui::GMMainWindow					ui;
	CGMVolumeWidget*					m_pVolumeWidget;

	bool								m_bInit;
	bool								m_bFull;				//!< 是否全屏
	bool								m_bPressed;				//!< 是否按下鼠标
	QPoint								m_vPos;					//!< 窗口的位置
	int									m_iAudioDuration;		//!< 音频总时长，单位：ms
	QString								m_strName;				//!< 音频文件名称，包含后缀名
};