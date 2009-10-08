#include <QtGui>
#include "videowindow.h"

VideoWindow::VideoWindow(QWidget *parent)
{
	setupUi(this);
	shortcutpause = new QShortcut(this);
	shortcutpause->setKey(trUtf8(" "));
	shortcutstop = new QShortcut(this);
	shortcutstop->setKey(trUtf8("Ctrl+s"));
	shortcutfs = new QShortcut(this);
	shortcutfs->setKey(trUtf8("f"));
	shortcutesc = new QShortcut(this);
	shortcutesc->setKey(Qt::Key_Escape);

	shortcutnmin = new QShortcut(this);
	shortcutnmin->setKey(Qt::Key_Right);
	shortcutnmid = new QShortcut(this);
	shortcutnmid->setKey(Qt::Key_Up);
	shortcutnmax = new QShortcut(this);
	shortcutnmax->setKey(Qt::Key_PageUp);
	shortcutpmin = new QShortcut(this);
	shortcutpmin->setKey(Qt::Key_Left);
	shortcutpmid = new QShortcut(this);
	shortcutpmid->setKey(Qt::Key_Down);
	shortcutpmax = new QShortcut(this);
	shortcutpmax->setKey(Qt::Key_PageDown);

	connect(shortcutpause, SIGNAL(activated()), this, SIGNAL(pauseEvent()));
	connect(shortcutstop, SIGNAL(activated()), this, SIGNAL(stopEvent()));
	connect(shortcutfs, SIGNAL(activated()), this, SLOT(fullScreen()));
	connect(shortcutesc, SIGNAL(activated()), this, SLOT(showNormal()));

	connect(shortcutnmin, SIGNAL(activated()), this, SIGNAL(pressKeyRight()));
	connect(shortcutnmid, SIGNAL(activated()), this, SIGNAL(pressKeyUp()));
	connect(shortcutnmax, SIGNAL(activated()), this, SIGNAL(pressKeyPgUp()));
	connect(shortcutpmin, SIGNAL(activated()), this, SIGNAL(pressKeyLeft()));
	connect(shortcutpmid, SIGNAL(activated()), this, SIGNAL(pressKeyDown()));
	connect(shortcutpmax, SIGNAL(activated()), this, SIGNAL(pressKeyPgDown()));

	(void) *parent;
}

void VideoWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
	fullScreen();
	(void) *event;
}

void VideoWindow::closeEvent(QCloseEvent *event)
{
	event->ignore();
}

void VideoWindow::fullScreen()
{
	if (isFullScreen())
		showNormal();
	else
		showFullScreen();
}
