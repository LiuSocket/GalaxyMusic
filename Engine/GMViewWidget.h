//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMViewWidget.h
/// @brief		Galaxy-Music Engine - GMViewWidget.h
/// @version	1.0
/// @author		LiuTao
/// @date		2021.08.15
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <osgViewer/CompositeViewer>
#include <osgQt/GraphicsWindowQt>
#include <QtCore/QTimer>

class CGMViewWidget : public QWidget, public osgViewer::CompositeViewer
{
	Q_OBJECT;

public:
	CGMViewWidget(osgViewer::View* pView,
		QWidget* parent = 0, Qt::WindowFlags f = 0,
		osgViewer::CompositeViewer::ThreadingModel threadingModel = osgViewer::CompositeViewer::SingleThreaded);
	~CGMViewWidget();

protected:
	osgQt::GraphicsWindowQt* createGraphicsWindow(
		int x, int y, int w, int h,
		const std::string& name = "",
		bool windowDecoration = false);

	void enterEvent(QEvent* event);

signals:

	/**
	* _signalEnter3D
	* @brief 进入三维界面
	* @param void
	* @return void
	*/
	void _signalEnter3D();

protected:
	QTimer _timer;
};