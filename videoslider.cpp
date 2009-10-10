#include "videoslider.h"
#include "ui_videoslider.h"

VideoSlider::VideoSlider(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::VideoSlider)
{
    m_ui->setupUi(this);

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

	connect(shortcutpause, SIGNAL(activated()), this, SIGNAL(pauseEvent()));
	connect(shortcutstop, SIGNAL(activated()), this, SIGNAL(stopEvent()));
	connect(shortcutfs, SIGNAL(activated()), this, SIGNAL(fullScreen()));
	connect(shortcutesc, SIGNAL(activated()), this, SIGNAL(showNormal()));
	connect(shortcutesc, SIGNAL(activated()), this, SIGNAL(normCursor()));
	connect(shortcutquit, SIGNAL(activated()), qApp, SLOT(quit()));

	connect(shortcutnmin, SIGNAL(activated()), this, SIGNAL(pressKeyRight()));
	connect(shortcutnmid, SIGNAL(activated()), this, SIGNAL(pressKeyUp()));
	connect(shortcutnmax, SIGNAL(activated()), this, SIGNAL(pressKeyPgUp()));
	connect(shortcutpmin, SIGNAL(activated()), this, SIGNAL(pressKeyLeft()));
	connect(shortcutpmid, SIGNAL(activated()), this, SIGNAL(pressKeyDown()));
	connect(shortcutpmax, SIGNAL(activated()), this, SIGNAL(pressKeyPgDown()));

	connect(m_ui->timeSlider, SIGNAL(valueChanged(int)), this, SLOT(setNumLCDs(int)));
	connect(m_ui->timeSlider, SIGNAL(sliderReleased()), this, SIGNAL(sliderRelease()));

	connect(m_ui->pauseButton, SIGNAL(clicked()), this, SIGNAL(pauseEvent()));
	connect(m_ui->stopButton, SIGNAL(clicked()), this, SIGNAL(stopEvent()));
	connect(m_ui->volumeSlider, SIGNAL(valueChanged(int)), this, SIGNAL(volumeChan(int)));
}

VideoSlider::~VideoSlider()
{
    delete m_ui;
}

void VideoSlider::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void VideoSlider::mouseDoubleClickEvent(QMouseEvent *event)
{
	emit doubleClick();
	(void) *event;
}

void VideoSlider::leaveEvent(QEvent *event)
{
	hide();
	(void) *event;
}

void VideoSlider::setSliderMax(int time)
{
	m_ui->timeSlider->setMaximum(time/1000);
}

void VideoSlider::setSliderPos(int pos)
{
	if(!m_ui->timeSlider->isSliderDown())
		m_ui->timeSlider->setSliderPosition(pos);
}

void VideoSlider::setNumLCDs(int sec)
{
	int min = sec / 60;
	int hour = min / 60;
	if (sec >= 60)
		sec %= 60;
	if (min >= 60)
		min %= 60;
	if (sec < 10)
	{
		QString newvalue = "0" + QString::number(sec,10);
		m_ui->secNumber->display(newvalue);
	}
	else
		m_ui->secNumber->display(sec);
	if (min < 10)
	{
		QString newvalue = "0" + QString::number(min,10);
		m_ui->minNumber->display(newvalue);
	}
	else
		m_ui->minNumber->display(min);
	m_ui->hourNumber->display(hour);
}

int VideoSlider::getSliderPos()
{
	return m_ui->timeSlider->value();
}

void VideoSlider::setVolumePos(int pos)
{
	m_ui->volumeSlider->setValue(pos);
}
