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
	void createAudioMenu(int, int);
	void createTitleMenu(int, int);
private:
	QShortcut *shortcutpause;
	QShortcut *shortcutstop;
	QShortcut *shortcutfs;
	QShortcut *shortcutesc;
	QShortcut *shortcutquit;
	QShortcut *shortcutnmin;
	QShortcut *shortcutnmid;
	QShortcut *shortcutnmax;
	QShortcut *shortcutpmin;
	QShortcut *shortcutpmid;
	QShortcut *shortcutpmax;
	QAction *quitAction;
	QAction *aboutAction;
	QAction *subtitleAction;
	QAction *audioAction;
	QActionGroup *streamGroup;
	QActionGroup *titleGroup;
	QAction *nstreamAction;
	QAction *ntitleAction;
	QMenu *audioMenu;
	QMenu *subtitleMenu;
private slots:
	void fullScreen();
	void normCursor();
	void createMenu();
	void changeAid(QAction*);
	void changeTid(QAction*);
signals:
	void pauseEvent();
	void stopEvent();
	void pressKeyRight();
	void pressKeyLeft();
	void pressKeyUp();
	void pressKeyDown();
	void pressKeyPgUp();
	void pressKeyPgDown();
	void aboutSig();
	void sendAid(int);
	void sendTid(int);
};

#endif // VIDEOWINDOW_H