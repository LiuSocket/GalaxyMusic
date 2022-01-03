//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMMain.cpp
/// @brief		Galaxy-Music Engine - GMMain.cpp
/// @version	1.0
/// @author		LiuTao
/// @date		2020.12.10
//////////////////////////////////////////////////////////////////////////

#include "GMSystemManager.h"
#include <QTextCodec>
#include <QFileInfo>
#include <QtWidgets/QApplication>
#include <QFont>
#include <QIcon>
#include <QTranslator>

using namespace GM;

int main(int argc, char **argv)
{
	QTextCodec *xcodec = QTextCodec::codecForLocale();
	QString exeDir = xcodec->toUnicode(QByteArray(argv[0]));
	QString BKE_CURRENT_DIR = QFileInfo(exeDir).path();
	QStringList  libpath;
	libpath << BKE_CURRENT_DIR + QString::fromLocal8Bit("/plugins/platforms");
	libpath << BKE_CURRENT_DIR << BKE_CURRENT_DIR + QString::fromLocal8Bit("/plugins/imageformats");
	libpath << BKE_CURRENT_DIR + QString::fromLocal8Bit("/plugins");
	libpath << QApplication::libraryPaths();
	QApplication::setLibraryPaths(libpath);

	QApplication a(argc, argv);
	a.setWindowIcon(QIcon(":/Resources/GM_logo.png"));
	a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

	QFont textFont = a.font();
	textFont.setFamily("Microsoft YaHei");
	textFont.setStyleStrategy(QFont::PreferAntialias);
	a.setFont(textFont);

	//加载Qt标准对话框的中文翻译文件
	QTranslator tran;
	tran.load(":/Resources/qt_zh_CN.qm");
	a.installTranslator(&tran);

	GM_SYSTEM_MANAGER_PTR->Init();

	return a.exec();
}