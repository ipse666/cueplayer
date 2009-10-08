#ifndef VIDEOWINDOW_H
#define VIDEOWINDOW_H

#include <QWidget>
#include <QShortcut>
#include "ui_videowindow.h"

class VideoWindow : public QWidget, public Ui::VideoWin
{
	Q_OBJECT

protected:
	void mouseDoubleClickEvent(QMouseEvent * event);
	void closeEvent(QCloseEvent *event);
public:
	VideoWindow(QWidget *parent = 0);
private:
	QShortcut *shortcutpause;
	QShortcut *shortcutstop;
	QShortcut *shortcutfs;
	QShortcut *shortcutesc;
	QShortcut *shortcutnmin;
	QShortcut *shortcutnmid;
	QShortcut *shortcutnmax;
	QShortcut *shortcutpmin;
	QShortcut *shortcutpmid;
	QShortcut *shortcutpmax;
private slots:
	void fullScreen();
signals:
	void pauseEvent();
	void stopEvent();
	void pressKeyRight();
	void pressKeyLeft();
	void pressKeyUp();
	void pressKeyDown();
	void pressKeyPgUp();
	void pressKeyPgDown();
};

#endif // VIDEOWINDOW_H
