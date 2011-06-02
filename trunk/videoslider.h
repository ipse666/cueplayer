#ifndef VIDEOSLIDER_H
#define VIDEOSLIDER_H

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
        void initPlayer();
        void externalTimeRevers(bool);

protected:
        void changeEvent(QEvent *e);
	void mouseDoubleClickEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent * event);
	void leaveEvent(QEvent *event);
        void mousePressEvent(QMouseEvent * event);

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
        bool reverseTime;

private slots:
	void setNumLCDs(int);
	void sliderTrigger(int);
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
	void newTime(int);
        void timeRevers(bool);
};

#endif // VIDEOSLIDER_H
