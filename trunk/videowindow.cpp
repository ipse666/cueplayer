#include "videowindow.h"

VideoWindow::VideoWindow(QWidget *parent)
{
	setupUi(this);

}

void VideoWindow::mouseDoubleClickEvent(QMouseEvent * event)
{
	if (isFullScreen())
		showNormal();
	else
		showFullScreen();
}
