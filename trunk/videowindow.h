#ifndef VIDEOWINDOW_H
#define VIDEOWINDOW_H

#include <QShortcut>
#include "ui_videowindow.h"
#include "videoslider.h"

class VideoWindow : public QWidget, public Ui::VideoWin
{
	Q_OBJECT

protected:
	void mouseDoubleClickEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent * event);
	void closeEvent(QCloseEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
public:
	VideoWindow(QWidget *parent = 0);
	void createAudioMenu(int, int);
	void createTitleMenu(int, int);
	void setSliderMaximum(int);
	void newTrack();
	void setSliderPos(int);
	int getSliderPos();
	void setVolumePos(int);
private:
	VideoSlider *slider;
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
	QTimer *timer;
	QWidget *desktop;
private slots:
	void fullScreen();
	void normCursor();
	void createMenu();
	void changeAid(QAction*);
	void changeTid(QAction*);
	void hideMouseTO();
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
	void sliderRelease();
	void volumeChan(int);
	void videoExit();
	void draganddrop(QStringList);
	void newTime(int);
};

#endif // VIDEOWINDOW_H
