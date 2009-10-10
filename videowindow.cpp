#include <QtGui>
#include "videowindow.h"

#define TIMEOUT 3000

VideoWindow::VideoWindow(QWidget *parent)
{
	setupUi(this);
	streamGroup = new QActionGroup(this);
	titleGroup = new QActionGroup(this);
	timer = new QTimer(this);

	shortcutpause = new QShortcut(this);
	shortcutpause->setKey(trUtf8(" "));
	shortcutstop = new QShortcut(this);
	shortcutstop->setKey(trUtf8("Ctrl+s"));
	shortcutfs = new QShortcut(this);
	shortcutfs->setKey(trUtf8("f"));
	shortcutesc = new QShortcut(this);
	shortcutesc->setKey(Qt::Key_Escape);
	shortcutquit = new QShortcut(this);
	shortcutquit->setKey(trUtf8("q"));

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

	setMouseTracking(true);
	createMenu();

	connect(shortcutpause, SIGNAL(activated()), this, SIGNAL(pauseEvent()));
	connect(shortcutstop, SIGNAL(activated()), this, SIGNAL(stopEvent()));
	connect(shortcutfs, SIGNAL(activated()), this, SLOT(fullScreen()));
	connect(shortcutesc, SIGNAL(activated()), this, SLOT(showNormal()));
	connect(shortcutesc, SIGNAL(activated()), this, SLOT(normCursor()));
	connect(shortcutquit, SIGNAL(activated()), qApp, SLOT(quit()));

	connect(shortcutnmin, SIGNAL(activated()), this, SIGNAL(pressKeyRight()));
	connect(shortcutnmid, SIGNAL(activated()), this, SIGNAL(pressKeyUp()));
	connect(shortcutnmax, SIGNAL(activated()), this, SIGNAL(pressKeyPgUp()));
	connect(shortcutpmin, SIGNAL(activated()), this, SIGNAL(pressKeyLeft()));
	connect(shortcutpmid, SIGNAL(activated()), this, SIGNAL(pressKeyDown()));
	connect(shortcutpmax, SIGNAL(activated()), this, SIGNAL(pressKeyPgDown()));
	connect(streamGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeAid(QAction*)));
	connect(titleGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeTid(QAction*)));
	connect(timer, SIGNAL(timeout()), this, SLOT(hideMouseTO()));

	(void) *parent;
}

void VideoWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
	fullScreen();
	(void) *event;
}

void VideoWindow::mouseMoveEvent(QMouseEvent *event)
{
	if (isFullScreen())
	{
		setCursor(Qt::ArrowCursor);
		timer->start(TIMEOUT);
	}
	(void) *event;
}

void VideoWindow::closeEvent(QCloseEvent *event)
{
	event->ignore();
}

void VideoWindow::fullScreen()
{
	if (isFullScreen())
	{
		setCursor(Qt::ArrowCursor);
		showNormal();
	}
	else
	{
		setCursor(Qt::BlankCursor);
		showFullScreen();
	}
}

void VideoWindow::normCursor()
{
	setCursor(Qt::ArrowCursor);
}

void VideoWindow::createMenu()
{
	quitAction = new QAction(trUtf8("&Выход"), this);
	quitAction->setIcon(QIcon(":/images/application-exit.png"));
	connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

	aboutAction = new QAction(trUtf8("&О программе"), this);
	aboutAction->setIcon(QIcon(":/images/help-about.png"));
	connect(aboutAction, SIGNAL(triggered()), this, SIGNAL(aboutSig()));

	audioAction = new QAction(trUtf8("&Аудио"), this);
	audioAction->setIcon(QIcon(":/images/speaker.png"));
	audioMenu = new QMenu(this);
	audioAction->setMenu(audioMenu);

	subtitleAction = new QAction(trUtf8("&Субтитры"), this);
	subtitleAction->setIcon(QIcon(":/images/text-field.png"));
	subtitleMenu = new QMenu(this);
	subtitleAction->setMenu(subtitleMenu);

	addAction(subtitleAction);
	addAction(audioAction);
	addAction(aboutAction);
	addAction(quitAction);
	setContextMenuPolicy(Qt::ActionsContextMenu);
}

void VideoWindow::createAudioMenu(int count, int current)
{
	audioMenu->clear();
	for(; count > -1; count--)
	{
		nstreamAction = new QAction(this);
		nstreamAction->setCheckable(true);
		nstreamAction->setText(QString::number(count));
		streamGroup->addAction(nstreamAction);
		audioMenu->addAction(nstreamAction);
		if (count == current)
			nstreamAction->setChecked(true);
	}
}

void VideoWindow::createTitleMenu(int count, int current)
{
	subtitleMenu->clear();
	for(; count > -1; count--)
	{
		ntitleAction = new QAction(this);
		ntitleAction->setCheckable(true);
		ntitleAction->setText(QString::number(count));
		titleGroup->addAction(ntitleAction);
		subtitleMenu->addAction(ntitleAction);
		if (count == current)
			ntitleAction->setChecked(true);
	}
	ntitleAction = new QAction(this);
	ntitleAction->setCheckable(true);
	ntitleAction->setText(trUtf8("откл."));
	titleGroup->addAction(ntitleAction);
	subtitleMenu->addAction(ntitleAction);
}

void VideoWindow::changeAid(QAction* a)
{
	emit sendAid(a->text().toInt());
}

void VideoWindow::changeTid(QAction* a)
{
	if(a->text() == trUtf8("откл."))
		emit sendTid(20);
	else
		emit sendTid(a->text().toInt());
}

void VideoWindow::hideMouseTO()
{
	if (isFullScreen())
	{
		setCursor(Qt::BlankCursor);
		timer->stop();
	}
}
