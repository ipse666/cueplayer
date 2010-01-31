#include <QtGui>
#include "videowindow.h"

#define TIMEOUT 3000

VideoWindow::VideoWindow(QWidget *parent)
{
	setupUi(this);
	streamGroup = new QActionGroup(this);
	titleGroup = new QActionGroup(this);
	timer = new QTimer(this);
	slider = new VideoSlider(this);
	slider->hide();
	desktop = QApplication::desktop();
	slider->resize(desktop->width(), slider->height());
	QPoint p(0, desktop->height() - slider->height());
	slider->move(p);
	slider->setWindowFlags(Qt::Popup);

	// Drag and Drop
	setAcceptDrops(true);

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
	connect(slider, SIGNAL(pauseEvent()), this, SIGNAL(pauseEvent()));
	connect(shortcutstop, SIGNAL(activated()), this, SIGNAL(stopEvent()));
	connect(slider, SIGNAL(stopEvent()), this, SIGNAL(stopEvent()));
	connect(shortcutfs, SIGNAL(activated()), this, SLOT(fullScreen()));
	connect(slider, SIGNAL(fullScreen()), this, SLOT(fullScreen()));
	connect(shortcutesc, SIGNAL(activated()), this, SLOT(showNormal()));
	connect(slider, SIGNAL(showNormal()), this, SLOT(showNormal()));
	connect(shortcutesc, SIGNAL(activated()), this, SLOT(normCursor()));
	connect(slider, SIGNAL(normCursor()), this, SLOT(normCursor()));
	connect(shortcutquit, SIGNAL(activated()), this, SIGNAL(videoExit()));

	connect(shortcutnmin, SIGNAL(activated()), this, SIGNAL(pressKeyRight()));
	connect(slider, SIGNAL(pressKeyRight()), this, SIGNAL(pressKeyRight()));
	connect(shortcutnmid, SIGNAL(activated()), this, SIGNAL(pressKeyUp()));
	connect(slider, SIGNAL(pressKeyUp()), this, SIGNAL(pressKeyUp()));
	connect(shortcutnmax, SIGNAL(activated()), this, SIGNAL(pressKeyPgUp()));
	connect(slider, SIGNAL(pressKeyPgUp()), this, SIGNAL(pressKeyPgUp()));
	connect(shortcutpmin, SIGNAL(activated()), this, SIGNAL(pressKeyLeft()));
	connect(slider, SIGNAL(pressKeyLeft()), this, SIGNAL(pressKeyLeft()));
	connect(shortcutpmid, SIGNAL(activated()), this, SIGNAL(pressKeyDown()));
	connect(slider, SIGNAL(pressKeyDown()), this, SIGNAL(pressKeyDown()));
	connect(shortcutpmax, SIGNAL(activated()), this, SIGNAL(pressKeyPgDown()));
	connect(slider, SIGNAL(pressKeyPgDown()), this, SIGNAL(pressKeyPgDown()));
	connect(streamGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeAid(QAction*)));
	connect(titleGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeTid(QAction*)));
	connect(timer, SIGNAL(timeout()), this, SLOT(hideMouseTO()));
	connect(slider, SIGNAL(doubleClick()), this, SLOT(fullScreen()));
	connect(slider, SIGNAL(sliderRelease()), this, SIGNAL(sliderRelease()));
	connect(slider, SIGNAL(volumeChan(int)), this, SIGNAL(volumeChan(int)));
	connect(slider, SIGNAL(newTime(int)), this, SIGNAL(newTime(int)));

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
		if (event->y() > desktop->height() - slider->height())
			slider->show();
		timer->start(TIMEOUT);
	}
}

void VideoWindow::closeEvent(QCloseEvent *event)
{
	event->ignore();
	emit stopEvent();
}

void VideoWindow::fullScreen()
{
	if (this->parentWidget() != 0)
		emit doubleClick(true);
	else
		emit doubleClick(false);

	if (isFullScreen())
	{
		setCursor(Qt::ArrowCursor);
		showNormal();
		slider->hide();
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
	connect(quitAction, SIGNAL(triggered()), this, SIGNAL(videoExit()));

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
	nstreamAction = new QAction(this);
	nstreamAction->setCheckable(true);
	nstreamAction->setText(trUtf8("файл"));
	streamGroup->addAction(nstreamAction);
	audioMenu->addAction(nstreamAction);
	nstreamAction->setEnabled(false); // до фикса
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
	if (current < 0) ntitleAction->setChecked(true);
}

void VideoWindow::changeAid(QAction* a)
{
	if(a->text() == trUtf8("файл"))
		emit sendAid(20);
	else
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

void VideoWindow::setSliderMaximum(int time)
{
	slider->setSliderMax(time);
}

void VideoWindow::newTrack()
{
	slider->setSliderPos(0);
	slider->hide();
}

void VideoWindow::setSliderPos(int pos)
{
	slider->setSliderPos(pos);
}

int VideoWindow::getSliderPos()
{
	return slider->getSliderPos();
}

void VideoWindow::setVolumePos(int pos)
{
	slider->setVolumePos(pos);
}

void VideoWindow::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("text/uri-list"))
			event->acceptProposedAction();
}

void VideoWindow::dropEvent(QDropEvent *event)
{
	QRegExp rxFilename("^file://.*");
	QList<QUrl> urls = event->mimeData()->urls();
	if (urls.isEmpty())
		return;
	if (rxFilename.indexIn(urls.first().toString()) != -1)
		emit draganddrop(QStringList() << urls.first().toLocalFile());
	else
		emit draganddrop(QStringList() << urls.first().toString());
}

bool VideoWindow::event(QEvent *event)
{
	if (event->type() == QEvent::Show)
		emit showWin(true);
	else if (event->type() == QEvent::Hide)
		emit showWin(false);
	return QWidget::event(event);
}
