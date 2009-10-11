#ifndef VIDEOSLIDER_H
#define VIDEOSLIDER_H

#include <QtGui/QWidget>
#include <QShortcut>
#include <QMouseEvent>

namespace Ui {
    class VideoSlider;
}

class VideoSlider : public QWidget {
    Q_OBJECT
public:
    VideoSlider(QWidget *parent = 0);
    ~VideoSlider();

	void setSliderMax(int);
	void setSliderPos(int);
	int getSliderPos();
	void setVolumePos(int);

protected:
    void changeEvent(QEvent *e);
	void mouseDoubleClickEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent * event);
	void leaveEvent(QEvent *event);

private:
    Ui::VideoSlider *m_ui;
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

private slots:
	void setNumLCDs(int);
signals:
	void doubleClick();
	void pauseEvent();
	void stopEvent();
	void fullScreen();
	void showNormal();
	void normCursor();
	void pressKeyRight();
	void pressKeyLeft();
	void pressKeyUp();
	void pressKeyDown();
	void pressKeyPgUp();
	void pressKeyPgDown();
	void sliderRelease();
	void volumeChan(int);
};

#endif // VIDEOSLIDER_H
