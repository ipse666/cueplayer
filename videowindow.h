#ifndef VIDEOWINDOW_H
#define VIDEOWINDOW_H

#include <QWidget>
#include "ui_videowindow.h"

class VideoWindow : public QWidget, public Ui::VideoWin
{
	Q_OBJECT
protected:
	void mouseDoubleClickEvent(QMouseEvent * event);
public:
	VideoWindow(QWidget *parent = 0);
};

#endif // VIDEOWINDOW_H
